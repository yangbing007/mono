/*
 * handle.c: Handle to object in native code
 *
 * Authors:
 *  - Ludovic Henry <ludovic@xamarin.com>
 *
 * Copyright 2015 Xamarin, Inc. (www.xamarin.com)
 */

#include <config.h>
#include <glib.h>

#include "handle.h"
#include "object-internals.h"
#include "gc-internals.h"
#include "utils/atomic.h"
#include "utils/mono-lazy-init.h"

#define HANDLES_PER_CHUNK (16 - 3)

typedef struct _MonoHandleArenaChunk MonoHandleArenaChunk;
struct _MonoHandleArenaChunk {
	MonoHandleArenaChunk *next;
	gsize handles_capacity;
	gsize handles_size;
	MonoHandle handles [MONO_ZERO_LEN_ARRAY];
};

struct _MonoHandleArena {
	MonoHandleArenaChunk *chunk;
	MonoHandleArenaChunk *chunk_last;
	MonoHandleArena *prev;
};

static mono_lazy_init_t arena_status = MONO_LAZY_INIT_STATUS_NOT_INITIALIZED;

#ifdef HAVE_SGEN_GC
static MonoGCDescriptor arena_desc = MONO_GC_DESCRIPTOR_NULL;
#endif

static MonoHandleArenaChunk *chunk_free_list = NULL;

static void
arena_scan (gpointer addr, MonoGCMarkFunc mark_func, gpointer gc_data);

static void
initialize (void)
{
#ifdef HAVE_SGEN_GC
	arena_desc = mono_gc_make_root_descr_user (arena_scan);
#endif
}

static inline MonoHandleArenaChunk*
chunk_alloc (void)
{
	MonoHandleArenaChunk *old, *new;

	do {
		old = chunk_free_list;
		if (!old) {
			MonoHandleArenaChunk *chunk;

			chunk = g_malloc0 (sizeof (MonoHandleArenaChunk) + sizeof (MonoHandle) * (HANDLES_PER_CHUNK - MONO_ZERO_LEN_ARRAY));
			chunk->handles_capacity = HANDLES_PER_CHUNK;

			return chunk;
		}

		new = old->next;
	} while (InterlockedCompareExchangePointer ((gpointer*) &chunk_free_list, new, old) != old);

	memset (old, 0, sizeof (MonoHandleArenaChunk));
	return old;
}

static inline void
chunk_free (MonoHandleArenaChunk *chunk)
{
	do {
		chunk->next = chunk_free_list;
	} while (InterlockedCompareExchangePointer ((gpointer*) &chunk_free_list, chunk, chunk->next) != chunk->next);
}

static MonoHandle*
handle_new (MonoHandleArena *arena, MonoObject *obj)
{
	MonoHandleArenaChunk *chunk;

	g_assert (arena->chunk);
	g_assert (arena->chunk_last);

	chunk = arena->chunk_last;

	if (chunk->handles_size < chunk->handles_capacity) {
		chunk->handles [chunk->handles_size].obj = obj;
		chunk->handles_size += 1;

		return &chunk->handles [chunk->handles_size - 1];
	}

	chunk = chunk->next = chunk_alloc ();

	chunk->handles [0].obj = obj;
	chunk->handles_size = 1;

	arena->chunk_last = chunk;

	return &chunk->handles [0];
}

MonoHandle*
mono_handle_new (MonoObject *obj)
{
	MonoInternalThread *thread;
	MonoHandleArena *arena;

	thread = mono_thread_internal_current ();
	g_assert (thread);

	arena = thread->handle_arena;
	g_assert (arena);

	return handle_new (arena, obj);
}

/*
 * Elevate the handle to the parent arena
 */
MonoHandle*
mono_handle_elevate (MonoHandle *handle)
{
	MonoInternalThread *thread;
	MonoHandleArena *arena;

	thread = mono_thread_internal_current ();
	g_assert (thread);

	arena = thread->handle_arena;
	g_assert (arena);
	g_assert (arena->prev);

	return handle_new (arena->prev, handle->obj);
}

gsize
mono_handle_arena_size (gsize nb_handles)
{
	g_assert (nb_handles > 0);
	return sizeof (MonoHandleArena) + sizeof (MonoHandleArenaChunk) + sizeof (MonoHandle) * (MAX (nb_handles, HANDLES_PER_CHUNK) - MONO_ZERO_LEN_ARRAY);
}

void
mono_handle_arena_push (MonoHandleArena *arena, gsize nb_handles)
{
	MonoInternalThread *thread;

	thread = mono_thread_internal_current ();
	g_assert (thread);

	g_assert (arena);

	arena->prev = thread->handle_arena;
	arena->chunk = arena->chunk_last = (MonoHandleArenaChunk*) (((char*) arena) + sizeof (MonoHandleArena));

	arena->chunk->next = NULL;
	arena->chunk->handles_capacity = MAX (nb_handles, HANDLES_PER_CHUNK);
	arena->chunk->handles_size = 0;
	memset (&arena->chunk->handles [0], 0, sizeof (MonoHandle) * arena->chunk->handles_capacity);

	thread->handle_arena = arena;
}

void
mono_handle_arena_pop (MonoHandleArena *arena, gsize nb_handles)
{
	MonoInternalThread *thread;
	MonoHandleArenaChunk *chunk, *next;

	g_assert (arena);
	g_assert (arena->chunk);
	g_assert (arena->chunk_last);

	thread = mono_thread_internal_current ();
	g_assert (thread);

	thread->handle_arena = arena->prev;

	for (chunk = arena->chunk; chunk; chunk = next) {
		next = chunk->next;
		memset (&chunk->handles [0], 0, sizeof (MonoHandle) * chunk->handles_capacity);
		if (chunk != arena->chunk)
			chunk_free (chunk);
	}
}

MonoHandle*
mono_handle_arena_pop_and_ret (MonoHandleArena *arena, gsize nb_handles, MonoHandle *handle)
{
	MonoInternalThread *thread;
	MonoHandleArenaChunk *chunk, *next;
	MonoHandle *handle_prev;

	g_assert (arena);
	g_assert (arena->prev);

	handle_prev = handle_new (arena->prev, handle->obj);

	g_assert (arena->chunk);
	g_assert (arena->chunk_last);

	thread = mono_thread_internal_current ();
	g_assert (thread);

	thread->handle_arena = arena->prev;

	for (chunk = arena->chunk; chunk; chunk = next) {
		next = chunk->next;
		memset (&chunk->handles [0], 0, sizeof (MonoHandle) * chunk->handles_capacity);
		if (chunk != arena->chunk)
			chunk_free (chunk);
	}

	return handle_prev;
}

static void
arena_scan (gpointer addr, MonoGCMarkFunc mark_func, gpointer gc_data)
{
	MonoHandleArena *arena;
	MonoHandleArenaChunk *chunk;
	int i;

	for (arena = *(MonoHandleArena**) addr; arena; arena = arena->prev) {
		for (chunk = arena->chunk; chunk; chunk = chunk->next) {
			for (i = 0; i < chunk->handles_size; ++i) {
				if (chunk->handles [i].obj != NULL)
					mark_func (&chunk->handles [i].obj, gc_data);
			}
		}
	}
}

void
mono_handle_arena_init_thread (MonoInternalThread *thread)
{
#ifdef HAVE_SGEN_GC
	mono_lazy_initialize (&arena_status, initialize);
	mono_gc_register_root ((char*) &thread->handle_arena, sizeof (MonoHandleArena*), arena_desc, MONO_ROOT_SOURCE_HANDLE, "runtime threads handle arena");
#endif
}

void
mono_handle_arena_deinit_thread (MonoInternalThread *thread)
{
#ifdef HAVE_SGEN_GC
	mono_gc_deregister_root ((char*) &thread->handle_arena);
#endif
}
