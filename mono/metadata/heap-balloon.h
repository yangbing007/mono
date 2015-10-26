/*
 * metadata/heap-balloon.h: Heap Balloon interface
 *
 * Author: Aleksey Kliger <aleksey@xamarin.com>
 *
 * Copyright (C) 2015 Xamarin, Inc (http://www.xamarin.com)
 *
 */

#ifndef __MONO_METADATA_HEAP_BALLOON_H__
#define __MONO_METADATA_HEAP_BALLOON_H__

void
mono_heap_balloon_inflate (guint64 nbytes);

void
mono_heap_balloon_deflate (guint64 nbytes);



#endif /* __MONO_METADATA_HEAP_BALLOON_H__ */
