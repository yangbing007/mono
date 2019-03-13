/**
 * \file
 * AssemblyLoadContext functions
 * 
 */

#include <config.h>
#include <glib.h>

#include <mono/metadata/icall-decl.h>
#include <mono/metadata/object-internals.h>
#include <mono/utils/mono-error-internals.h>

#ifdef ENABLE_NETCORE

gpointer
ves_icall_System_Runtime_Loader_AssemblyLoadContext_InitializeAssemblyLoadContext (gpointer netcore_gchandle, MonoBoolean isTPA, MonoBoolean collectible, MonoError *error)
{
	guint32 gchandle = GPOINTER_TO_UINT (netcore_gchandle) >> 1;
	g_warning ("registering assembly load context (global handle = %p, tpa = %s, collectible = %s)\n", gchandle, isTPA ? "true" : "false", collectible ? "true" : "false");
	printf ("gchandle points to %p\n", mono_gchandle_get_target_internal (gchandle));
	MonoReflectionAssemblyLoadContextHandle alc_obj = MONO_HANDLE_CAST (MonoReflectionAssemblyLoadContext, mono_gchandle_get_target_handle (gchandle));
	g_assert (!MONO_HANDLE_IS_NULL (alc_obj));
	gpointer asmctx = MONO_HANDLE_GETVAL (alc_obj, native_asmctx);
	g_assert (asmctx == NULL);
	return NULL;
}

#endif /* ENABLE_NETCORE */
