#ifndef __MONO_MINI_EMSCRIPTEN_H__
#define __MONO_MINI_EMSCRIPTEN_H__

#include <glib.h>

// Because there is no JIT on emscripten, most of these defines do nothing.

#ifndef DISABLE_JIT
#error Cannot build for Emscripten with JIT.
#endif

#define MONO_MAX_IREGS 0
#define MONO_MAX_FREGS 0
#define MONO_ARCH_CALLEE_REGS 0
#define MONO_ARCH_CALLEE_FREGS 0
#define MONO_ARCH_CALLEE_SAVED_FREGS 0
#define MONO_ARCH_CALLEE_SAVED_REGS 0
#define MONO_ARCH_VTABLE_REG 0

struct MonoLMF {
	gpointer    previous_lmf;
};

typedef struct {
} MonoCompileArch;

#define MONO_ARCH_INIT_TOP_LMF_ENTRY(...)

void
mono_hwcap_arch_init (void);

#endif /* __MONO_MINI_EMSCRIPTEN_H__ */
