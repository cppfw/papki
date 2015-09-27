include prorab.mk


this_name := tests


#compiler flags
this_cflags += -std=c++11
this_cflags += -Wall
this_cflags += -DDEBUG
this_cflags += -fstrict-aliasing #strict aliasing!!!

this_srcs += main.cpp tests.cpp


this_ldlibs += -lpapki

this_ldflags += -L$(prorab_this_dir)../../src/

ifeq ($(prorab_os),macosx)
    this_cflags += -stdlib=libc++ #this is needed to be able to use c++11 std lib
    this_ldlibs += -lc++
endif


$(eval $(prorab-build-app))

include $(prorab_this_dir)../test_target.mk


#add dependency on libpapki
this_target_name := $(prorab_this_name): $(abspath $(prorab_this_dir)../../src/libpapki$(prorab_lib_extension))


#include makefile for building papki
$(eval $(call prorab-include,$(prorab_this_dir)../../src/makefile))
