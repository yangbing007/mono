/*
 * mono-pp-filter.h: Macros to filter variadic arguments
 *
 * Author:
 *     Aleksey Kliger (aleksey@xamarin.com)
 *
 * Copyright 2015 Xamarin Inc (http://www.xamarin.com)
 */
#ifndef _MONO_UTIL_MONO_PP_FILTER_H
#define _MONO_UTIL_MONO_PP_FILTER_H
#ifdef _MSC_VER
#pragma once
#endif

#include <mono/utils/mono-pp-core.h>
#include <mono/utils/mono-pp-bool.h>
#include <mono/utils/mono-pp-foreach.h>

/* MONO_PP_VA_FILTER(Predicate, e1, ..., eN) expands to the comma-separated e's for which Predicate returns non-0. */
#define MONO_PP_VA_FILTER(Predicate,...) MONO_PP_RESCAN_MACRO(MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE(MONO_PP_VA_FILTER_n_, MONO_PP_VA_COUNT_ARGS(__VA_ARGS__)), Predicate, (__VA_ARGS__)))

#define MONO_PP_VA_FILTER_n_10(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _9), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_9(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _8), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_8(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _7), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_7(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _6), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_6(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _5), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_5(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _4), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_4(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _3), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_3(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _2), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_2(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_CONCATENATE3(MONO_PP_VA_FILTER_, MONO_PP_IF(Predicate(Arg))(y)(n), _1), Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_n_1(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(Arg)(/*empty*/)
#define MONO_PP_VA_FILTER_n_0(...) /* empty */

#define MONO_PP_VA_FILTER_y_10(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_9, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_9(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_8, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_8(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_7, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_7(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_6, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_6(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_5, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_5(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_4, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_4(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_3, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_3(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_2, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_2(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)() MONO_PP_VA_MS_WORKAROUND(MONO_PP_VA_FILTER_y_1, Predicate, (__VA_ARGS__))
#define MONO_PP_VA_FILTER_y_1(Predicate, Arg, ...) MONO_PP_IF(Predicate(Arg))(COMMA Arg)(/*empty*/)
#define MONO_PP_VA_FILTER_y_0(...) /* empty */


#endif/*_MONO_UTIL_MONO_PP_FILTER_H*/
