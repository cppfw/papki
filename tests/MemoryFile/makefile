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
else ifeq ($(prorab_os),windows)
    this_cflags += -I/usr/include -I/usr/local/include
    this_ldflags += -L/usr/lib -L/usr/local/lib
endif

#add dependency on libpapki.so
$(abspath $(prorab_this_dir)tests): $(abspath $(prorab_this_dir)../../src/libpapki$(prorab_lib_extension))


$(eval $(prorab-build-app))

include $(prorab_this_dir)../test_target.mk


#include makefile for building papki
$(eval $(call prorab-include,$(prorab_this_dir)../../src/makefile))
