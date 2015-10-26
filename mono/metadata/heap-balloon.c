/*
 * metadata/heap-balloon.h: Heap Balloon interface
 *
 * Author: Aleksey Kliger <aleksey@xamarin.com>
 *
 * Copyright (C) 2015 Xamarin, Inc (http://www.xamarin.com)
 *
 */

/* The Heap Balloon is a new GC root that points to a large heap
 * object.  The idea is that the balloon imposes pressure on the
 * amount of memory available to the clients (so it only makes sense
 * with a fixed heap size).
 * 
 * This is different from the System.GC.AddMemoryPressure() API which
 * is about informing the GC about memory pressure outside of its
 * control.  By contrast, this API is about dropping some large
 * allocations into the managed heap.
 *
 * A couple of tricky points (and why this needs to be an API and not
 * just some allocations in the mutator thread):
 * 
 * 1. We do not want to be relegated to the large object store, so we
 * allocate a bunch of small balloons.
 */

#include <config.h>
#include <glib.h>
#include <mono/metadata/gc-internal.h>
#include <mono/metadata/heap-balloon.h>
#include <mono/metadata/object.h>
#if HAVE_SGEN_GC
#  include <mono/sgen/sgen-gc.h>
#  include <mono/sgen/sgen-memory-governor.h>
#endif


void
mono_heap_balloon_inflate (guint64 nbytes)
{
#if HAVE_SGEN_GC
	sgen_memgov_balloon_inflate (nbytes);
#endif

#if HAVE_BOEHM_GC
#endif

	g_assert_not_reached ();
}

void
mono_heap_balloon_deflate (guint64 nbytes)
{
#if HAVE_SGEN_GC
	sgen_memgov_balloon_deflate (nbytes);
#endif

#if HAVE_BOEHM_GC
#endif

	g_assert_not_reached();
}

