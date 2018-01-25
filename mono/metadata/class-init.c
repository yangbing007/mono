/**
 * \file MonoClass initialization
 * Copyright 2018 Microsoft
 * Licensed under the MIT license. See LICENSE file in the project root for full license information.
 */
#include <config.h>
#include <mono/metadata/class-init.h>
#include <mono/metadata/class-internals.h>
#include <mono/metadata/profiler-private.h>
#include <mono/utils/mono-error-internals.h>
#include <mono/utils/checked-build.h>
#include <mono/utils/unlocked.h>

/* Statistics */
gint32 classes_size;
gint32 inflated_classes_size;
gint32 class_def_count, class_gtd_count, class_ginst_count;

static void mono_generic_class_setup_parent (MonoClass *klass, MonoClass *gtd);


/*
We use gclass recording to allow recursive system f types to be referenced by a parent.

Given the following type hierarchy:

class TextBox : TextBoxBase<TextBox> {}
class TextBoxBase<T> : TextInput<TextBox> where T : TextBoxBase<T> {}
class TextInput<T> : Input<T> where T: TextInput<T> {}
class Input<T> {}

The runtime tries to load TextBoxBase<>.
To load TextBoxBase<> to do so it must resolve the parent which is TextInput<TextBox>.
To instantiate TextInput<TextBox> it must resolve TextInput<> and TextBox.
To load TextBox it must resolve the parent which is TextBoxBase<TextBox>.

At this point the runtime must instantiate TextBoxBase<TextBox>. Both types are partially loaded 
at this point, iow, both are registered in the type map and both and a NULL parent. This means
that the resulting generic instance will have a NULL parent, which is wrong and will cause breakage.

To fix that what we do is to record all generic instantes created while resolving the parent of
any generic type definition and, after resolved, correct the parent field if needed.

*/
static int record_gclass_instantiation;
static GSList *gclass_recorded_list;
typedef gboolean (*gclass_record_func) (MonoClass*, void*);

/* 
 * LOCKING: loader lock must be held until pairing disable_gclass_recording is called.
*/
static void
enable_gclass_recording (void)
{
	++record_gclass_instantiation;
}

/* 
 * LOCKING: loader lock must be held since pairing enable_gclass_recording was called.
*/
static void
disable_gclass_recording (gclass_record_func func, void *user_data)
{
	GSList **head = &gclass_recorded_list;

	g_assert (record_gclass_instantiation > 0);
	--record_gclass_instantiation;

	while (*head) {
		GSList *node = *head;
		if (func ((MonoClass*)node->data, user_data)) {
			*head = node->next;
			g_slist_free_1 (node);
		} else {
			head = &node->next;
		}
	}

	/* We automatically discard all recorded gclasses when disabled. */
	if (!record_gclass_instantiation && gclass_recorded_list) {
		g_slist_free (gclass_recorded_list);
		gclass_recorded_list = NULL;
	}
}

static gboolean
discard_gclass_due_to_failure (MonoClass *gclass, void *user_data)
{
	return mono_class_get_generic_class (gclass)->container_class == user_data;
}

static gboolean
fix_gclass_incomplete_instantiation (MonoClass *gclass, void *user_data)
{
	MonoClass *gtd = (MonoClass*)user_data;
	/* Only try to fix generic instances of @gtd */
	if (mono_class_get_generic_class (gclass)->container_class != gtd)
		return FALSE;

	/* Check if the generic instance has no parent. */
	if (gtd->parent && !gclass->parent)
		mono_generic_class_setup_parent (gclass, gtd);

	return TRUE;
}

static void
mono_class_set_failure_and_error (MonoClass *klass, MonoError *error, const char *msg)
{
	mono_class_set_type_load_failure (klass, "%s", msg);
	mono_error_set_type_load_class (error, klass, "%s", msg);
}

/**
 * mono_class_create_from_typedef:
 * \param image: image where the token is valid
 * \param type_token:  typedef token
 * \param error:  used to return any error found while creating the type
 *
 * Create the MonoClass* representing the specified type token.
 * \p type_token must be a TypeDef token.
 *
 * FIXME: don't return NULL on failure, just let the caller figure it out.
 */
MonoClass *
mono_class_create_from_typedef (MonoImage *image, guint32 type_token, MonoError *error)
{
	MonoTableInfo *tt = &image->tables [MONO_TABLE_TYPEDEF];
	MonoClass *klass, *parent = NULL;
	guint32 cols [MONO_TYPEDEF_SIZE];
	guint32 cols_next [MONO_TYPEDEF_SIZE];
	guint tidx = mono_metadata_token_index (type_token);
	MonoGenericContext *context = NULL;
	const char *name, *nspace;
	guint icount = 0; 
	MonoClass **interfaces;
	guint32 field_last, method_last;
	guint32 nesting_tokeen;

	error_init (error);

	if (mono_metadata_token_table (type_token) != MONO_TABLE_TYPEDEF || tidx > tt->rows) {
		mono_error_set_bad_image (error, image, "Invalid typedef token %x", type_token);
		return NULL;
	}

	mono_loader_lock ();

	if ((klass = (MonoClass *)mono_internal_hash_table_lookup (&image->class_cache, GUINT_TO_POINTER (type_token)))) {
		mono_loader_unlock ();
		return klass;
	}

	mono_metadata_decode_row (tt, tidx - 1, cols, MONO_TYPEDEF_SIZE);
	
	name = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAME]);
	nspace = mono_metadata_string_heap (image, cols [MONO_TYPEDEF_NAMESPACE]);

	if (mono_metadata_has_generic_params (image, type_token)) {
		klass = mono_image_alloc0 (image, sizeof (MonoClassGtd));
		klass->class_kind = MONO_CLASS_GTD;
		UnlockedAdd (&classes_size, sizeof (MonoClassGtd));
		++class_gtd_count;
	} else {
		klass = mono_image_alloc0 (image, sizeof (MonoClassDef));
		klass->class_kind = MONO_CLASS_DEF;
		UnlockedAdd (&classes_size, sizeof (MonoClassDef));
		++class_def_count;
	}

	klass->name = name;
	klass->name_space = nspace;

	MONO_PROFILER_RAISE (class_loading, (klass));

	klass->image = image;
	klass->type_token = type_token;
	mono_class_set_flags (klass, cols [MONO_TYPEDEF_FLAGS]);

	mono_internal_hash_table_insert (&image->class_cache, GUINT_TO_POINTER (type_token), klass);

	/*
	 * Check whether we're a generic type definition.
	 */
	if (mono_class_is_gtd (klass)) {
		MonoGenericContainer *generic_container = mono_metadata_load_generic_params (image, klass->type_token, NULL);
		generic_container->owner.klass = klass;
		generic_container->is_anonymous = FALSE; // Owner class is now known, container is no longer anonymous
		context = &generic_container->context;
		mono_class_set_generic_container (klass, generic_container);
		MonoType *canonical_inst = &((MonoClassGtd*)klass)->canonical_inst;
		canonical_inst->type = MONO_TYPE_GENERICINST;
		canonical_inst->data.generic_class = mono_metadata_lookup_generic_class (klass, context->class_inst, FALSE);
		enable_gclass_recording ();
	}

	if (cols [MONO_TYPEDEF_EXTENDS]) {
		MonoClass *tmp;
		guint32 parent_token = mono_metadata_token_from_dor (cols [MONO_TYPEDEF_EXTENDS]);

		if (mono_metadata_token_table (parent_token) == MONO_TABLE_TYPESPEC) {
			/*WARNING: this must satisfy mono_metadata_type_hash*/
			klass->this_arg.byref = 1;
			klass->this_arg.data.klass = klass;
			klass->this_arg.type = MONO_TYPE_CLASS;
			klass->byval_arg.data.klass = klass;
			klass->byval_arg.type = MONO_TYPE_CLASS;
		}
		parent = mono_class_get_checked (image, parent_token, error);
		if (parent && context) /* Always inflate */
			parent = mono_class_inflate_generic_class_checked (parent, context, error);

		if (parent == NULL) {
			mono_class_set_type_load_failure (klass, "%s", mono_error_get_message (error));
			goto parent_failure;
		}

		for (tmp = parent; tmp; tmp = tmp->parent) {
			if (tmp == klass) {
				mono_class_set_failure_and_error (klass, error, "Cycle found while resolving parent");
				goto parent_failure;
			}
			if (mono_class_is_gtd (klass) && mono_class_is_ginst (tmp) && mono_class_get_generic_class (tmp)->container_class == klass) {
				mono_class_set_failure_and_error (klass, error, "Parent extends generic instance of this type");
				goto parent_failure;
			}
		}
	}

	mono_class_setup_parent (klass, parent);

	/* uses ->valuetype, which is initialized by mono_class_setup_parent above */
	mono_class_setup_mono_type (klass);

	if (mono_class_is_gtd (klass))
		disable_gclass_recording (fix_gclass_incomplete_instantiation, klass);

	/* 
	 * This might access klass->byval_arg for recursion generated by generic constraints,
	 * so it has to come after setup_mono_type ().
	 */
	if ((nesting_tokeen = mono_metadata_nested_in_typedef (image, type_token))) {
		klass->nested_in = mono_class_create_from_typedef (image, nesting_tokeen, error);
		if (!mono_error_ok (error)) {
			/*FIXME implement a mono_class_set_failure_from_mono_error */
			mono_class_set_type_load_failure (klass, "%s",  mono_error_get_message (error));
			mono_loader_unlock ();
			MONO_PROFILER_RAISE (class_failed, (klass));
			return NULL;
		}
	}

	if ((mono_class_get_flags (klass) & TYPE_ATTRIBUTE_STRING_FORMAT_MASK) == TYPE_ATTRIBUTE_UNICODE_CLASS)
		klass->unicode = 1;

#ifdef HOST_WIN32
	if ((mono_class_get_flags (klass) & TYPE_ATTRIBUTE_STRING_FORMAT_MASK) == TYPE_ATTRIBUTE_AUTO_CLASS)
		klass->unicode = 1;
#endif

	klass->cast_class = klass->element_class = klass;
	if (mono_is_corlib_image (klass->image)) {
		switch (klass->byval_arg.type) {
			case MONO_TYPE_I1:
				if (mono_defaults.byte_class)
					klass->cast_class = mono_defaults.byte_class;
				break;
			case MONO_TYPE_U1:
				if (mono_defaults.sbyte_class)
					mono_defaults.sbyte_class = klass;
				break;
			case MONO_TYPE_I2:
				if (mono_defaults.uint16_class)
					mono_defaults.uint16_class = klass;
				break;
			case MONO_TYPE_U2:
				if (mono_defaults.int16_class)
					klass->cast_class = mono_defaults.int16_class;
				break;
			case MONO_TYPE_I4:
				if (mono_defaults.uint32_class)
					mono_defaults.uint32_class = klass;
				break;
			case MONO_TYPE_U4:
				if (mono_defaults.int32_class)
					klass->cast_class = mono_defaults.int32_class;
				break;
			case MONO_TYPE_I8:
				if (mono_defaults.uint64_class)
					mono_defaults.uint64_class = klass;
				break;
			case MONO_TYPE_U8:
				if (mono_defaults.int64_class)
					klass->cast_class = mono_defaults.int64_class;
				break;
		}
	}

	if (!klass->enumtype) {
		if (!mono_metadata_interfaces_from_typedef_full (
			    image, type_token, &interfaces, &icount, FALSE, context, error)){

			mono_class_set_type_load_failure (klass, "%s", mono_error_get_message (error));
			mono_loader_unlock ();
			MONO_PROFILER_RAISE (class_failed, (klass));
			return NULL;
		}

		/* This is required now that it is possible for more than 2^16 interfaces to exist. */
		g_assert(icount <= 65535);

		klass->interfaces = interfaces;
		klass->interface_count = icount;
		klass->interfaces_inited = 1;
	}

	/*g_print ("Load class %s\n", name);*/

	/*
	 * Compute the field and method lists
	 */
	int first_field_idx = cols [MONO_TYPEDEF_FIELD_LIST] - 1;
	mono_class_set_first_field_idx (klass, first_field_idx);
	int first_method_idx = cols [MONO_TYPEDEF_METHOD_LIST] - 1;
	mono_class_set_first_method_idx (klass, first_method_idx);

	if (tt->rows > tidx){		
		mono_metadata_decode_row (tt, tidx, cols_next, MONO_TYPEDEF_SIZE);
		field_last  = cols_next [MONO_TYPEDEF_FIELD_LIST] - 1;
		method_last = cols_next [MONO_TYPEDEF_METHOD_LIST] - 1;
	} else {
		field_last  = image->tables [MONO_TABLE_FIELD].rows;
		method_last = image->tables [MONO_TABLE_METHOD].rows;
	}

	if (cols [MONO_TYPEDEF_FIELD_LIST] && 
	    cols [MONO_TYPEDEF_FIELD_LIST] <= image->tables [MONO_TABLE_FIELD].rows)
		mono_class_set_field_count (klass, field_last - first_field_idx);
	if (cols [MONO_TYPEDEF_METHOD_LIST] <= image->tables [MONO_TABLE_METHOD].rows)
		mono_class_set_method_count (klass, method_last - first_method_idx);

	/* reserve space to store vector pointer in arrays */
	if (mono_is_corlib_image (image) && !strcmp (nspace, "System") && !strcmp (name, "Array")) {
		klass->instance_size += 2 * sizeof (gpointer);
		g_assert (mono_class_get_field_count (klass) == 0);
	}

	if (klass->enumtype) {
		MonoType *enum_basetype = mono_class_find_enum_basetype (klass, error);
		if (!enum_basetype) {
			/*set it to a default value as the whole runtime can't handle this to be null*/
			klass->cast_class = klass->element_class = mono_defaults.int32_class;
			mono_class_set_type_load_failure (klass, "%s", mono_error_get_message (error));
			mono_loader_unlock ();
			MONO_PROFILER_RAISE (class_failed, (klass));
			return NULL;
		}
		klass->cast_class = klass->element_class = mono_class_from_mono_type (enum_basetype);
	}

	/*
	 * If we're a generic type definition, load the constraints.
	 * We must do this after the class has been constructed to make certain recursive scenarios
	 * work.
	 */
	if (mono_class_is_gtd (klass) && !mono_metadata_load_generic_param_constraints_checked (image, type_token, mono_class_get_generic_container (klass), error)) {
		mono_class_set_type_load_failure (klass, "Could not load generic parameter constrains due to %s", mono_error_get_message (error));
		mono_loader_unlock ();
		MONO_PROFILER_RAISE (class_failed, (klass));
		return NULL;
	}

	if (klass->image->assembly_name && !strcmp (klass->image->assembly_name, "Mono.Simd") && !strcmp (nspace, "Mono.Simd")) {
		if (!strncmp (name, "Vector", 6))
			klass->simd_type = !strcmp (name + 6, "2d") || !strcmp (name + 6, "2ul") || !strcmp (name + 6, "2l") || !strcmp (name + 6, "4f") || !strcmp (name + 6, "4ui") || !strcmp (name + 6, "4i") || !strcmp (name + 6, "8s") || !strcmp (name + 6, "8us") || !strcmp (name + 6, "16b") || !strcmp (name + 6, "16sb");
	} else if (klass->image->assembly_name && !strcmp (klass->image->assembly_name, "System.Numerics") && !strcmp (nspace, "System.Numerics")) {
		/* The JIT can't handle SIMD types with != 16 size yet */
		//if (!strcmp (name, "Vector2") || !strcmp (name, "Vector3") || !strcmp (name, "Vector4"))
		if (!strcmp (name, "Vector4"))
			klass->simd_type = 1;
	}

	mono_loader_unlock ();

	MONO_PROFILER_RAISE (class_loaded, (klass));

	return klass;

parent_failure:
	if (mono_class_is_gtd (klass))
		disable_gclass_recording (discard_gclass_due_to_failure, klass);

	mono_class_setup_mono_type (klass);
	mono_loader_unlock ();
	MONO_PROFILER_RAISE (class_failed, (klass));
	return NULL;
}


static void
mono_generic_class_setup_parent (MonoClass *klass, MonoClass *gtd)
{
	if (gtd->parent) {
		ERROR_DECL (error);
		MonoGenericClass *gclass = mono_class_get_generic_class (klass);

		klass->parent = mono_class_inflate_generic_class_checked (gtd->parent, mono_generic_class_get_context (gclass), error);
		if (!mono_error_ok (error)) {
			/*Set parent to something safe as the runtime doesn't handle well this kind of failure.*/
			klass->parent = mono_defaults.object_class;
			mono_class_set_type_load_failure (klass, "Parent is a generic type instantiation that failed due to: %s", mono_error_get_message (error));
			mono_error_cleanup (error);
		}
	}
	mono_loader_lock ();
	if (klass->parent)
		mono_class_setup_parent (klass, klass->parent);

	if (klass->enumtype) {
		klass->cast_class = gtd->cast_class;
		klass->element_class = gtd->element_class;
	}
	mono_loader_unlock ();
}

/*
 * Create the `MonoClass' for an instantiation of a generic type.
 * We only do this if we actually need it.
 */
MonoClass*
mono_class_create_generic_inst (MonoGenericClass *gclass)
{
	MonoClass *klass, *gklass;

	if (gclass->cached_class)
		return gclass->cached_class;

	klass = (MonoClass *)mono_image_set_alloc0 (gclass->owner, sizeof (MonoClassGenericInst));

	gklass = gclass->container_class;

	if (gklass->nested_in) {
		/* The nested_in type should not be inflated since it's possible to produce a nested type with less generic arguments*/
		klass->nested_in = gklass->nested_in;
	}

	klass->name = gklass->name;
	klass->name_space = gklass->name_space;
	
	klass->image = gklass->image;
	klass->type_token = gklass->type_token;

	klass->class_kind = MONO_CLASS_GINST;
	//FIXME add setter
	((MonoClassGenericInst*)klass)->generic_class = gclass;

	klass->byval_arg.type = MONO_TYPE_GENERICINST;
	klass->this_arg.type = klass->byval_arg.type;
	klass->this_arg.data.generic_class = klass->byval_arg.data.generic_class = gclass;
	klass->this_arg.byref = TRUE;
	klass->enumtype = gklass->enumtype;
	klass->valuetype = gklass->valuetype;


	if (gklass->image->assembly_name && !strcmp (gklass->image->assembly_name, "System.Numerics.Vectors") && !strcmp (gklass->name_space, "System.Numerics") && !strcmp (gklass->name, "Vector`1")) {
		g_assert (gclass->context.class_inst);
		g_assert (gclass->context.class_inst->type_argc > 0);
		if (mono_type_is_primitive (gclass->context.class_inst->type_argv [0]))
			klass->simd_type = 1;
	}
	klass->is_array_special_interface = gklass->is_array_special_interface;

	klass->cast_class = klass->element_class = klass;

	if (gclass->is_dynamic) {
		/*
		 * We don't need to do any init workf with unbaked typebuilders. Generic instances created at this point will be later unregistered and/or fixed.
		 * This is to avoid work that would probably give wrong results as fields change as we build the TypeBuilder.
		 * See remove_instantiations_of_and_ensure_contents in reflection.c and its usage in reflection.c to understand the fixup stage of SRE banking.
		*/
		if (!gklass->wastypebuilder)
			klass->inited = 1;

		if (klass->enumtype) {
			/*
			 * For enums, gklass->fields might not been set, but instance_size etc. is 
			 * already set in mono_reflection_create_internal_class (). For non-enums,
			 * these will be computed normally in mono_class_layout_fields ().
			 */
			klass->instance_size = gklass->instance_size;
			klass->sizes.class_size = gklass->sizes.class_size;
			klass->size_inited = 1;
		}
	}

	mono_loader_lock ();

	if (gclass->cached_class) {
		mono_loader_unlock ();
		return gclass->cached_class;
	}

	if (record_gclass_instantiation > 0)
		gclass_recorded_list = g_slist_append (gclass_recorded_list, klass);

	if (mono_class_is_nullable (klass))
		klass->cast_class = klass->element_class = mono_class_get_nullable_param (klass);

	MONO_PROFILER_RAISE (class_loading, (klass));

	mono_generic_class_setup_parent (klass, gklass);

	if (gclass->is_dynamic)
		mono_class_setup_supertypes (klass);

	mono_memory_barrier ();
	gclass->cached_class = klass;

	MONO_PROFILER_RAISE (class_loaded, (klass));

	++class_ginst_count;
	inflated_classes_size += sizeof (MonoClassGenericInst);
	
	mono_loader_unlock ();

	return klass;
}
