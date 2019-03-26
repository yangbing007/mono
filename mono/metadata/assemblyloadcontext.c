/**
 * \file
 * AssemblyLoadContext functions
 * 
 */

#include <config.h>
#include <glib.h>

#include <mono/metadata/assemblyloadcontext.h>
#include <mono/metadata/assembly-internals.h>
#include <mono/metadata/exception-internals.h>
#include <mono/metadata/icall-decl.h>
#include <mono/metadata/object-internals.h>
#include <mono/utils/mono-error-internals.h>
#include <mono/utils/mono-logger-internals.h>

#ifdef ENABLE_NETCORE

gpointer
ves_icall_System_Runtime_Loader_AssemblyLoadContext_InitializeAssemblyLoadContext (gpointer netcore_gchandle, MonoBoolean isTPA, MonoBoolean collectible, MonoError *error)
{
	MonoDomain *domain = mono_domain_get ();
	guint32 gchandle = GPOINTER_TO_UINT (netcore_gchandle) >> 1;
	MonoReflectionAssemblyLoadContextHandle alc_obj = MONO_HANDLE_CAST (MonoReflectionAssemblyLoadContext, mono_gchandle_get_target_handle (gchandle));
	g_assert (!MONO_HANDLE_IS_NULL (alc_obj));
	gpointer alc = MONO_HANDLE_GETVAL (alc_obj, native_asmctx);
	g_assert (alc == NULL);

	g_assert (!collectible); /*TODO: needs mono_gchandle_weak_from_handle once we want to support collectible ALCs */

	/* native assembly load context struct will own this gchandle */
	guint32 new_handle;
	MonoAssemblyContextKind kind;

	new_handle = mono_gchandle_from_handle (MONO_HANDLE_CAST (MonoObject, alc_obj), FALSE);
	kind = isTPA ? MONO_ASMCTX_DEFAULT : MONO_ASMCTX_INDIVIDUAL;
	
	alc = mono_domain_create_assembly_load_context (domain, new_handle, kind, collectible, error);
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_ASSEMBLY, "Created MonoAssemblyLoadContext %p (collectible = %s, isTPA = %s)",
		    alc, collectible ? "true" : "false", isTPA ? "true" : "false");
	return alc; /* managed sets alc_obj->native_asmctx to point to alc */
}

MonoReflectionAssemblyHandle
ves_icall_System_Runtime_Loader_AssemblyLoadContext_InternalLoadFile (MonoStringHandle fname, gpointer alc_ptr, MonoError *error)
{
	MonoDomain *domain = mono_domain_get ();
	MonoReflectionAssemblyHandle result = MONO_HANDLE_CAST (MonoReflectionAssembly, NULL_HANDLE);
	char *filename = NULL;
	if (MONO_HANDLE_IS_NULL (fname)) {
		mono_error_set_argument_null (error, "assemblyFile", "");
		goto leave;
	}

	filename = mono_string_handle_to_utf8 (fname, error);
	goto_if_nok (error, leave);

	if (!g_path_is_absolute (filename)) {
		mono_error_set_argument (error, "assemblyFile", "Absolute path information is required.");
		goto leave;
	}

	MonoAssemblyLoadContext *alc = (MonoAssemblyLoadContext *)alc_ptr;
	g_assert (alc);

	MonoImageOpenStatus status;
	MonoAssembly *ass;
	MonoAssemblyOpenRequest req;
	mono_assembly_request_prepare (&req.request, sizeof (req), alc->kind);
	ass = mono_assembly_request_open (filename, &req, &status);
	if (!ass) {
		if (status == MONO_IMAGE_IMAGE_INVALID)
			mono_error_set_bad_image_by_name (error, filename, "Invalid Image");
		else
			mono_error_set_file_not_found (error, filename, "Invalid Image");
		goto leave;
	}

	result = mono_assembly_get_object_handle (domain, ass, error);
leave:
	g_free (filename);
	return result;
}

MonoReflectionAssemblyHandle
ves_icall_System_Reflection_Assembly_InternalLoad (MonoStringHandle name_handle, MonoStackCrawlMark *stack_mark, gpointer load_Context, MonoError *error)
{
	/* XXX TODO: See https://github.com/dotnet/coreclr/blob/2520798548b0c414f513aaaf708399f8ef5a4f6c/src/vm/assemblynative.cpp#L36
	 * AssemblyNative::Load for how this is supposed to function.  In particular, the alc takes precedence over the stack crawl mark.
	 *
	 * The code to load an assembly spec is in
	 * https://github.com/dotnet/coreclr/blob/19394b01ed59d3771c0602ec321ec4520bcd2aa2/src/vm/assemblyspec.cpp#L892
	 * AssemblySpec::LoadDomainAssembly - this is where we get the binding context from the parent assembly.
	 *
	 * And all the nasty low level stuff is in https://github.com/dotnet/coreclr/blob/ea10aaccb09fe30f0444b821fd8d90d9257dd402/src/vm/appdomain.cpp#L5929
	 * AppDomain::BindAssemblySpec.
	 *
	 * In particular it also calls back to AssembySpec::SetBindingContext
	 * in order to stick the AssemblyLoadContext ("Binding Context") onto the load result.
	 *
	 */
	MonoDomain *domain = mono_domain_get ();
	MonoAssemblyLoadContext *alc = (MonoAssemblyLoadContext *)load_Context;

	if (alc)
		mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_ASSEMBLY, "InternalLoad: MonoAssemblyLoadContext %p (kind = %s)", alc, alc->kind == MONO_ASMCTX_DEFAULT ? "default" : "other"); /* TODO: mono_assembly_load_context_get_name () */

	/* TODO: use the stack mark, to pick the ALC from the running assembly, if we're not passed one. */

	char *name;
	name = mono_string_handle_to_utf8 (name_handle, error);
	goto_if_nok (error, fail);

	MonoAssemblyName aname;
	gboolean parsed;
	parsed = mono_assembly_name_parse (name, &aname);
	g_free (name);
	if (!parsed)
		goto fail;

	MonoAssemblyByNameRequest req;
	mono_assembly_request_prepare (&req.request, sizeof (req), alc ? alc->kind : MONO_ASMCTX_DEFAULT);
	req.basedir = NULL;
	req.no_postload_search = TRUE;

	MonoAssembly *ass;
	ass = mono_assembly_load_context_request_byname (alc, &aname, &req, error);
	if (!ass)
		goto fail;

	MonoReflectionAssemblyHandle refass;
	refass = mono_assembly_get_object_handle (domain, ass, error);
	goto_if_nok (error, fail);
	return refass;

fail:
	return MONO_HANDLE_CAST (MonoReflectionAssembly, NULL_HANDLE);
}

gpointer
ves_icall_System_Runtime_AssemblyLoadContext_GetLoadContextForAssembly (MonoReflectionAssemblyHandle refassm, MonoError *error)
{
	MonoAssembly *assm = MONO_HANDLE_GETVAL (refassm, assembly);
	g_assert (assm);

	if (mono_asmctx_get_kind (&assm->context) == MONO_ASMCTX_DEFAULT)
		return NULL; /* null here means managed will return the default. */

	mono_error_set_not_implemented (error, "AssemblyLoadContext.GetLoadContextForAssembly");
	return NULL;
}

#endif /* ENABLE_NETCORE */

#ifdef ENABLE_ASSEMBLY_LOAD_CONTEXT

/**
 * The memory is allocated from the domain's mempool
 * 
 * LOCKING: takes the domain lock
 */
MonoAssemblyLoadContext *
mono_domain_create_assembly_load_context (MonoDomain *domain, guint32 handle, MonoAssemblyContextKind kind, gboolean collectible, MonoError *error)
{
	MonoAssemblyLoadContext *alc = (MonoAssemblyLoadContext*) mono_domain_alloc0 (domain, sizeof (MonoAssemblyLoadContext));
	alc->kind = kind;
	alc->collectible = collectible;
	g_assert (handle);
	alc->handle = handle;
	mono_domain_lock (domain);
	if (!domain->alcs) {
		domain->alcs = mono_domain_alloc0 (domain, sizeof (MonoAssemblyLoadContextOwner));
	}
	if (kind == MONO_ASMCTX_DEFAULT) {
		g_assert (domain->alcs->default_ctx == NULL);
		domain->alcs->default_ctx = alc;
	}
	if (G_UNLIKELY  (collectible))
		g_warning ("Mono does not support collectible AssemblyLoadContexts.  AssemblyLoadContext %p will not be collected.", alc);
	domain->alcs->alcs = g_slist_prepend (domain->alcs->alcs, alc);
	mono_domain_unlock (domain);
	return alc;
}

void
mono_assembly_load_context_free (MonoAssemblyLoadContext *alc)
{
	mono_trace (G_LOG_LEVEL_DEBUG, MONO_TRACE_ASSEMBLY, "Freeing AssemblyLoadContext %p", alc);
	if (alc->handle)
		mono_gchandle_free_internal (alc->handle);
}

#endif /* ENABLE_ASSEMBLY_LOAD_CONTEXT */

void
mono_assembly_load_context_owner_free (MonoAssemblyLoadContextOwner *owner)
{
#ifdef ENABLE_ASSEMBLY_LOAD_CONTEXT
	if (owner->default_ctx)
		owner->default_ctx = NULL;
	GSList *ptr = owner->alcs;
	owner->alcs = NULL;
	while (ptr) {
		mono_assembly_load_context_free ((MonoAssemblyLoadContext *)ptr->data);
		ptr = ptr->next;
	}
#endif
}
