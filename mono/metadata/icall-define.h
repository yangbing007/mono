/*
 * icall-define.h: Convenience macro for definining icall implementations.
 *
 * Authors:
 *  - Aleksey Kliger <aleksey@xamarin.com>
 *
 * Copyright 2015 Xamarin, Inc. (www.xamarin.com)
 */
#ifndef _MONO_METADATA_ICALL_DEFINE_H
#define _MONO_METADATA_ICALL_DEFINE_H

#include <mono/utils/mono-pp-foreach.h>
#include <mono/utils/mono-pp-bool.h>
#include <mono/metadata/handle.h>

/**
   MONO_ICALL_DEFINE(ReturnSpec, fname, ParametersSpec, Body)
     - Define implementation for icall named 'fname'.

     The method will return a result of type specified by 'ReturnSpec'
     and take arguments given by the 'ParametersSpec' with the given
     'Body' (a C block).

     The ParametersSpec is (SpecArg1, ..., SpecArgN) where each SpecArg is a Spec or LocalSpec.
     The ReturnSpec is a Spec or void.

     A Spec is either VAL(type, name) where type is some value (non-MonoObject) type,
     or REF(type, name) (where type is a MonoObject or its subtype).

     A LocalSpec is LOCAL(type, name) which declares not a parameter
     but a new local handle (initially pointing to NULL).

     In the case of VAL spec, the value is available in the Body with
     its provided type. (for return specs, you should assign to it to return a value from the icall).

     In the case of a REF or LOCAL spec, the value is available in the Body in
     a MonoHandle and cannot be used directly.

 */
#define MONO_ICALL_DEFINE(ReturnSpec, name, ParametersSpec, Body)	\
	MONO_ICALL_IMPL_DECL(ReturnSpec, name, ParametersSpec)		\
	{								\
		MONO_ICALL_IMPL_DECLARE_RETURN_LCL(ReturnSpec);		\
		/*ICALL_ENTRY();*/					\
		MONO_ICALL_IMPL_DECLARE_PARAMETERS_LCL(ParametersSpec);	\
		/*MONO_HANDLE_ARENA_PUSH(MONO_ICALL_IMPL_ARENA_SIZE(ReturnSpec,ParametersSpec));*/ \
		MONO_ICALL_IMPL_INIT_RETURN_LCL(ReturnSpec);		\
		MONO_ICALL_IMPL_TAKE_PARAMETERS(ParametersSpec);	\
		Body ;							\
		MONO_ICALL_IMPL_RELEASE_RETURN(ReturnSpec);		\
		/*MONO_HANDLE_ARENA_POP;*/				\
		/*ICALL_EXIT();*/					\
		MONO_ICALL_IMPL_RETURN(ReturnSpec);			\
	}

/* expands to (0 +1 +1 +1) where the number of +1's equals to the number of REF specs in the
 * return and parameter specs
 */
#define MONO_ICALL_IMPL_ARENA_SIZE(ReturnSpec,ParametersSpec)		\
	(0 MONO_ICALL_IMPL_IN_ARENA_INC(ReturnSpec) MONO_ICALL_IMPL_IN_ARENA_PARAMS(ParametersSpec) )

/* expands to +1 +1 +1 where the number of +1's is equal to the number
 * of REF specs in the parameter pack */
#define MONO_ICALL_IMPL_IN_ARENA_PARAMS(ParametersSpec) MONO_ICALL_IMPL_IN_ARENA_PARAMS_ ParametersSpec
#define MONO_ICALL_IMPL_IN_ARENA_PARAMS_(...) MONO_PP_VA_FOREACH(MONO_ICALL_IMPL_IN_ARENA_INC, __VA_ARGS__)

#define MONO_ICALL_IMPL_IN_ARENA_INC(Spec)	\
	MONO_PP_IF(MONO_ICALL_IMPL_IS_REF(Spec))(+1)(/*empty*/)

#define MONO_ICALL_IMPL_DECL(ReturnSpec, name, ParametersSpec)	\
	MONO_ICALL_IMPL_SPEC_RETURN_TYPE(ReturnSpec)		\
	name (MONO_ICALL_IMPL_SPEC_PARAMETERS(ParametersSpec) )

#define MONO_ICALL_IMPL_SPEC_RETURN_TYPE(ReturnSpec)	\
	MONO_ICALL_IMPL_SPEC_CASE(ReturnSpec,MONO_ICALL_IMPL_SPEC_RETURN)

#define MONO_ICALL_IMPL_SPEC_RETURN_void() void
#define MONO_ICALL_IMPL_SPEC_RETURN_VAL(Typ, name) Typ
#define MONO_ICALL_IMPL_SPEC_RETURN_REF(Typ, name) Typ*

#define MONO_ICALL_IMPL_SPEC_PARAMETERS(Parameters) MONO_ICALL_IMPL_SPEC_PARAMETERS_ Parameters
#define MONO_ICALL_IMPL_SPEC_PARAMETERS_(...) MONO_PP_VA_FOREACH_CS(MONO_ICALL_IMPL_SPEC_PARAMETER, __VA_ARGS__)

#define MONO_ICALL_IMPL_SPEC_PARAMETER(Spec)				\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_SPEC_PARAMETER)

#define MONO_ICALL_IMPL_SPEC_PARAMETER_VAL(Typ, name) Typ name
#define MONO_ICALL_IMPL_SPEC_PARAMETER_REF(Typ, name) Typ* MONO_ICALL_IMPL_MANGLED_REF_NAME(name)
/* undefined MONO_ICALL_IMPL_SPEC_PARAMETER_void() */

/* REF parameters have their names mangled so that it's slightly
 * inconvenient for icall implementers to use them directly.
  */
#define MONO_ICALL_IMPL_MANGLED_REF_NAME(name) __icall_hidden_ ## name

#define MONO_ICALL_IMPL_HANDLE_TYPE(Typ) MONO_HANDLE_TYPE(Typ)


/* Create variable declarations for the REF parameters in the current scope. */
#define MONO_ICALL_IMPL_DECLARE_PARAMETERS_LCL(ParametersSpec) MONO_ICALL_IMPL_DECLARE_PARAMETERS_LCL_ ParametersSpec
#define MONO_ICALL_IMPL_DECLARE_PARAMETERS_LCL_(...)			\
	MONO_PP_VA_FOREACH(MONO_ICALL_IMPL_DECLARE_PARAMETER_LCL, __VA_ARGS__)


#define MONO_ICALL_IMPL_DECLARE_PARAMETER_LCL(Spec)			\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_DECLARE_PARAMETER_LCL)

#define MONO_ICALL_IMPL_DECLARE_PARAMETER_LCL_VAL(Typ, name) /* empty */
#define MONO_ICALL_IMPL_DECLARE_PARAMETER_LCL_REF(Typ, name) MONO_ICALL_IMPL_HANDLE_TYPE(Typ) name;
/* undefined MONO_ICALL_IMPL_DECLARE_PARAMETER_LCL_void */

#define MONO_ICALL_IMPL_ADDR_OF_HANDLE_COMMA(Spec)		\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_ADDR_OF_HANDLE_COMMA)

#define MONO_ICALL_IMPL_ADDR_OF_HANDLE_COMMA_VAL(Typ, name) /* empty */
#define MONO_ICALL_IMPL_ADDR_OF_HANDLE_COMMA_REF(Typ, name) (MonoHandle)name,
/* undefined MONO_ICALL_IMPL_ADDR_OF_HANDLE_COMMA_void() */


/* For each REF parameter, assign from the raw pointer to the local handle. */
#define MONO_ICALL_IMPL_TAKE_PARAMETERS(ParametersSpec) MONO_ICALL_IMPL_TAKE_PARAMETERS_ ParametersSpec
#define MONO_ICALL_IMPL_TAKE_PARAMETERS_(...) MONO_PP_VA_FOREACH(MONO_ICALL_IMPL_TAKE_PARAMETER, __VA_ARGS__)

#define MONO_ICALL_IMPL_TAKE_PARAMETER(Spec)			\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_TAKE_PARAMETER)

#define MONO_ICALL_IMPL_TAKE_PARAMETER_VAL(Typ, name) /* empty */
#define MONO_ICALL_IMPL_TAKE_PARAMETER_REF(Typ, name) name = MONO_LH_TAKE_FROM_UNSAFE(Typ, MONO_ICALL_IMPL_MANGLED_REF_NAME(name));
/* undefined MONO_ICALL_IMPL_TAKE_PARAMETER_void() */


/* For a VAL(Typ, name) spec, just expands to
 *    Typ name;
 *
 * For a REF(Typ, name) spec, expands to
 *    Typ *mangled_name;
 *    TypHandle *name;
 *
 * For a void spec, expands to nothing
 */
#define MONO_ICALL_IMPL_DECLARE_RETURN_LCL(Spec)			\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_DECLARE_RETURN_LCL)

#define MONO_ICALL_IMPL_DECLARE_RETURN_LCL_void() /* void return, no variable */
#define MONO_ICALL_IMPL_DECLARE_RETURN_LCL_VAL(Typ, name) Typ name;
#define MONO_ICALL_IMPL_DECLARE_RETURN_LCL_REF(Typ, name) \
	Typ *MONO_ICALL_IMPL_MANGLED_REF_NAME(name);	  \
        MONO_ICALL_IMPL_HANDLE_TYPE(Typ) name;

/* For a VAL spec or a void spec, expands to nothing
 *
 * For a REF(Typ, name) spec, expands to
 * name = (Typ ## Handle) mono_handle_new(NULL)
 */
#define MONO_ICALL_IMPL_INIT_RETURN_LCL(Spec)	\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_INIT_RETURN_LCL)

#define MONO_ICALL_IMPL_INIT_RETURN_LCL_void() /* empty */
#define MONO_ICALL_IMPL_INIT_RETURN_LCL_VAL(Typ, name) /* empty */
#define MONO_ICALL_IMPL_INIT_RETURN_LCL_REF(Typ, name) \
        name = (MONO_ICALL_IMPL_HANDLE_TYPE(Typ)) mono_handle_new(NULL)


/* FOR a VAL or a void spec, expands to nothing
 * 
 * For a REF(Typ, name) spec, expands to
 * name_raw = (Typ*) mono_lh_release((MonoHandle) name)
 */
#define MONO_ICALL_IMPL_RELEASE_RETURN(Spec)				\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_RELEASE_RETURN)

#define MONO_ICALL_IMPL_RELEASE_RETURN_void() /* empty */
#define MONO_ICALL_IMPL_RELEASE_RETURN_VAL(Typ, name) /*empty*/
#define MONO_ICALL_IMPL_RELEASE_RETURN_REF(Typ, name) MONO_ICALL_IMPL_MANGLED_REF_NAME(name) = (Typ*) mono_lh_release((MonoHandle)name)

/* For a VAL(Typ, name) spec expands to
 *   return name
 * For a void spec expands to
 *   return
 * For a REF(Typ, name) spec expands to
 *   return name_raw
 */
#define MONO_ICALL_IMPL_RETURN(Spec)					\
	MONO_ICALL_IMPL_SPEC_CASE(Spec,MONO_ICALL_IMPL_RETURN)

#define MONO_ICALL_IMPL_RETURN_void() return
#define MONO_ICALL_IMPL_RETURN_VAL(Typ, name) return name
#define MONO_ICALL_IMPL_RETURN_REF(Typ, name) return MONO_ICALL_IMPL_MANGLED_REF_NAME(name)

static MONO_ALWAYS_INLINE MonoObject*
mono_lh_release (MonoHandle h)
{
	MonoObject *obj = h->obj;
	h->obj = NULL;
	return obj;
}

#define MONO_LH_TAKE_FROM_UNSAFE(Typ, obj) (MONO_ICALL_IMPL_HANDLE_TYPE(Typ)) mono_handle_new ((MonoObject*)obj)


/* Implementation Utility macros */

#define MONO_ICALL_IMPL_IS_VAL(Spec) MONO_PP_IS_CHECK(MONO_PP_CONCATENATE(MONO_ICALL_IMPL_IS_VAL_, Spec))
#define MONO_ICALL_IMPL_IS_VAL_VAL(Typ, name) MONO_PP_PROBE(~)

#define MONO_ICALL_IMPL_ARGS_OF_VAL(valspec) MONO_PP_RESCAN_MACRO0(MONO_PP_CONCATENATE(MONO_ICALL_IMPL_ARGS_OF_VAL_, valspec))
#define MONO_ICALL_IMPL_ARGS_OF_VAL_VAL(...) __VA_ARGS__

#define MONO_ICALL_IMPL_IS_REF(Spec) MONO_PP_IS_CHECK(MONO_PP_CONCATENATE(MONO_ICALL_IMPL_IS_REF_, Spec))
#define MONO_ICALL_IMPL_IS_REF_REF(Typ, name) MONO_PP_PROBE(~)

#define MONO_ICALL_IMPL_ARGS_OF_REF(refspec) MONO_PP_RESCAN_MACRO0(MONO_PP_CONCATENATE(MONO_ICALL_IMPL_ARGS_OF_REF_, refspec))
#define MONO_ICALL_IMPL_ARGS_OF_REF_REF(...) __VA_ARGS__

#define MONO_ICALL_IMPL_IS_void(Spec) MONO_PP_IS_CHECK(MONO_PP_CONCATENATE(MONO_ICALL_IMPL_IS_void_, Spec))
#define MONO_ICALL_IMPL_IS_void_void MONO_PP_PROBE(~)


#define MONO_ICALL_IMPL_IS_OUTREF(Spec) MONO_PP_IS_CHECK(MONO_PP_CONCATENATE(MONO_ICALL_IMPL_IS_OUTREF_, Spec))
#define MONO_ICALL_IMPL_IS_OUTREF_OUTREF(Typ, name) MONO_PP_PROBE(~)

#define MONO_ICALL_IMPL_ARGS_OF_OUTREF(valspec) MONO_PP_RESCAN_MACRO0(MONO_PP_CONCATENATE(MONO_ICALL_IMPL_ARGS_OF_OUTREF_, valspec))
#define MONO_ICALL_IMPL_ARGS_OF_OUTREF_OUTREF(...) __VA_ARGS__

#define MONO_ICALL_IMPL_SPEC_CASE(Spec,Prefix)				\
	MONO_PP_IF(MONO_ICALL_IMPL_IS_VAL(Spec))(			\
		MONO_PP_VA_APPLY_PACK(MONO_PP_CONCATENATE(Prefix,_VAL), (MONO_ICALL_IMPL_ARGS_OF_VAL(Spec))) \
	)(								\
		MONO_PP_IF(MONO_ICALL_IMPL_IS_REF(Spec))(		\
			MONO_PP_VA_APPLY_PACK(MONO_PP_CONCATENATE(Prefix,_REF), (MONO_ICALL_IMPL_ARGS_OF_REF(Spec))) \
		)(							\
			MONO_PP_IF(MONO_ICALL_IMPL_IS_void(Spec))(	\
				MONO_PP_VA_APPLY_PACK(MONO_PP_CONCATENATE(Prefix,_void), ()) \
			)(						\
				MONO_PP_CONCATENATE(MONO_PP_CONCATENATE(_error_in_,Prefix),_wanted_REF_or_VAL_or_void_spec))))



#endif
