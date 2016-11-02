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

#if 0

#define MONO_SAVED_GREGS 0
#define MONO_SAVED_FREGS 0


#define IREG_SIZE	8
#define FREG_SIZE	8
typedef gfloat		mips_freg;

#endif

#endif /* __MONO_MINI_MIPS_H__ */  
