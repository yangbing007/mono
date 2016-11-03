/*
 * mono-threads-emscripten.c: Stubs
 *
 * Author:
 *	Andi McClure (andi.mcclure@xamarin.com)
 *
 * (C) 2016 Microsoft, Inc
 */

#include "config.h"
#include <glib.h>
#include <mono/utils/mono-threads.h>

#ifdef USE_EMSCRIPTEN_BACKEND

void
mono_threads_suspend_init (void)
{
}

void
mono_threads_suspend_register (MonoThreadInfo *info)
{
}

void
mono_threads_suspend_free (MonoThreadInfo *info)
{
}

void
mono_threads_platform_get_stack_bounds (guint8 **staddr, size_t *stsize)
{
	g_assert_not_reached ();
}

// TODO: Could possibly go into a NO_SIGNALS with TvOS?
void
mono_threads_abort_syscall_init (void)
{
}

gboolean
mono_threads_suspend_needs_abort_syscall (void)
{
	return FALSE;
}

void
mono_threads_suspend_abort_syscall (MonoThreadInfo *info)
{

}

gboolean
mono_threads_suspend_begin_async_suspend (MonoThreadInfo *info, gboolean interrupt_kernel)
{
	g_assert_not_reached ();
}

gboolean
mono_threads_suspend_check_suspend_result (MonoThreadInfo *info)
{
	g_assert_not_reached ();
}

gboolean
mono_threads_suspend_begin_async_resume (MonoThreadInfo *info)
{
	g_assert_not_reached ();
}

#endif