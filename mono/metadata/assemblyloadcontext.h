/**
 * \file
 * Copyright 2019 Microsoft
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#ifndef __MONO_METADATA_ASSEMBLYLOADCONTEXT_H__
#define __MONO_METADATA_ASSEMBLYLOADCONTEXT_H__

#include <config.h>
#include <glib.h>

#include <mono/metadata/metadata-internals.h>

/* If ENABLE_ASSEMBLY_LOAD_CONTEXT is defined, managed code can create ALCs */
#ifdef ENABLE_NETCORE
#define ENABLE_ASSEMBLY_LOAD_CONTEXT 1
#endif

typedef struct _MonoAssemblyLoadContext MonoAssemblyLoadContext;

/* Owns all the assembly load contexts for a domain. */
struct _MonoAssemblyLoadContextOwner {
	MonoAssemblyLoadContext *default_ctx;
	GSList * alcs;
};

struct _MonoAssemblyLoadContext {
	MonoAssemblyContextKind kind;
#ifdef ENABLE_ASSEMBLY_LOAD_CONTEXT
	/* The managed object corresponding to this asmctx.  If it's collectible, the handle is weak */
	guint32 handle;
	guint8 collectible : 1;
#endif /* ENABLE_ASSEMBLY_LOAD_CONTEXT */
};

#ifdef ENABLE_ASSEMBLY_LOAD_CONTEXT
MonoAssemblyLoadContext *
mono_domain_create_assembly_load_context (MonoDomain *domain, guint32 handle, MonoAssemblyContextKind kind, gboolean collectible, MonoError *error);

void
mono_assembly_load_context_free (MonoAssemblyLoadContext *alc);

#endif /* ENABLE_ASSEMBLY_LOAD_CONTEXT */

void
mono_assembly_load_context_owner_free (MonoAssemblyLoadContextOwner *owner);

#endif /* __MONO_METADATA_ASSEMBLYLOADCONTEXT_H__ */
