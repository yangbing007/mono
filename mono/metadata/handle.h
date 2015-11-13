/*
 * handle.h: Handle to object in native code
 *
 * Authors:
 *  - Ludovic Henry <ludovic@xamarin.com>
 *
 * Copyright 2015 Xamarin, Inc. (www.xamarin.com)
 */

#ifndef __MONO_HANDLE_H__
#define __MONO_HANDLE_H__

#include <config.h>
#include <glib.h>

#include "object.h"
#include "class.h"
#include "class-internals.h"
#include "threads-types.h"

#include "mono/utils/mono-threads-coop.h"

G_BEGIN_DECLS

typedef struct _MonoHandle MonoHandle;
struct _MonoHandle {
	MonoObject *obj;
};

MonoHandle*
mono_handle_new (MonoObject *obj);

MonoHandle*
mono_handle_elevate (MonoHandle *handle);

#define mono_handle_obj(handle) ((handle)->obj)

static inline MonoClass*
mono_handle_class (MonoHandle *handle)
{
	return mono_handle_obj (handle)->vtable->klass;
}

static inline MonoDomain*
mono_handle_domain (MonoHandle *handle)
{
	return mono_handle_obj (handle)->vtable->domain;
}

#define MONO_HANDLE_TYPE_DECL(type)      typedef struct { type *obj; } type ## Handle
#define MONO_HANDLE_TYPE(type)           type ## Handle
#define MONO_HANDLE_NEW(type,obj)        ((type ## Handle*) mono_handle_new ((MonoObject*) (obj)))
#define MONO_HANDLE_ELEVATE(type,handle) ((type ## Handle*) mono_handle_elevate ((MonoObject*) mono_handle_obj ((handle))))

#define MONO_HANDLE_SETREF(handle,fieldname,value)	\
	do {	\
		gpointer __value = (value);	\
		MONO_PREPARE_CRITICAL_SECTION;	\
		MONO_OBJECT_SETREF (mono_handle_obj ((handle)), fieldname, __value);	\
		MONO_FINISH_CRITICAL_SECTION;	\
	} while (0)

#define MONO_HANDLE_SET(handle,fieldname,value)	\
	do {	\
		MONO_PREPARE_CRITICAL_SECTION;	\
		mono_handle_obj ((handle))->fieldname = (value);	\
		MONO_FINISH_CRITICAL_SECTION;	\
	} while (0)

/* handle arena specific functions */

typedef struct _MonoHandleArena MonoHandleArena;

gsize
mono_handle_arena_size (gsize nb_handles);

void
mono_handle_arena_push (MonoHandleArena *arena, gsize nb_handles);

void
mono_handle_arena_pop (MonoHandleArena *arena, gsize nb_handles);

MonoHandle*
mono_handle_arena_pop_and_ret (MonoHandleArena *arena, gsize nb_handles, MonoHandle *handle);

void
mono_handle_arena_init_thread (MonoInternalThread *thread);

void
mono_handle_arena_deinit_thread (MonoInternalThread *thread);

#define MONO_HANDLE_ARENA_PUSH(NB_HANDLES)	\
	do {	\
		gsize __arena_nb_handles = (NB_HANDLES);	\
		MonoHandleArena *__arena = (MonoHandleArena*) g_alloca (mono_handle_arena_size (__arena_nb_handles));	\
		mono_handle_arena_push (__arena, __arena_nb_handles)

#define MONO_HANDLE_ARENA_POP	\
		mono_handle_arena_pop (__arena, __arena_nb_handles);	\
	} while (0)

/* Some common handle types */

MONO_HANDLE_TYPE_DECL (MonoArray);
MONO_HANDLE_TYPE_DECL (MonoString);

G_END_DECLS

#endif /* __MONO_HANDLE_H__ */
