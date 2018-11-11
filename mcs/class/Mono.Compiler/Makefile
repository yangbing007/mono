thisdir = class/Mono.Compiler
include ../../build/rules.make

LIBRARY = Mono.Compiler.dll
LIBRARY_WARN_AS_ERROR = yes

KEYFILE = $(LIBRARY_SNK)

API_BUILD := $(filter build, $(PROFILE))

ifdef API_BUILD
TARGET_NET_REFERENCE = $(BOOTSTRAP_BIN_PROFILE)
LIBRARY_USE_INTERMEDIATE_FILE = 1
endif

LIB_REFS = System
LIB_MCS_FLAGS = /publicsign /nowarn:0618 /unsafe

LIB_MCS_FLAGS += /d:MONO_DATACONVERTER_STATIC_METHODS

XTEST_LIB_REFS =

TEST_MCS_FLAGS += -debug -unsafe

EXTRA_DISTFILES =

CLEAN_FILES =

include ../../build/library.make
