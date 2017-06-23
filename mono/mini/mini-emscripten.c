#include <config.h>
#include "mini.h"
#include "mini-emscripten.h"

// Initialization

void
mono_hwcap_arch_init ()
{
}

void
mono_arch_cpu_init (void)
{
}

void
mono_arch_init (void)
{
}

void
mono_runtime_install_handlers (void)
{
}

// "CPU" spec

guint32
mono_arch_cpu_optimizations (guint32 *exclude_mask)
{
	*exclude_mask = 0;
	return 0;
}
