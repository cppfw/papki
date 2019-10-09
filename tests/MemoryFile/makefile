include prorab.mk


this_name := tests


#compiler flags
this_cxxflags += -std=c++11
this_cxxflags += -Wall
this_cxxflags += -DDEBUG
this_cxxflags += -fstrict-aliasing #strict aliasing!!!

this_srcs += main.cpp tests.cpp

this_ldlibs += -lpapki -lstdc++

this_ldflags += -L$(prorab_this_dir)../../src/

ifeq ($(prorab_os),macosx)
    this_cxxflags += -stdlib=libc++ #this is needed to be able to use c++11 std lib
    this_ldlibs += -lc++
else ifeq ($(prorab_os),windows)
endif

this_no_install := true

$(eval $(prorab-build-app))

#add dependency on libpapki.so
$(prorab_this_name): $(abspath $(d)../../src/libpapki$(soext))


include $(d)../test_target.mk


#include makefile for building papki
$(eval $(call prorab-try-include,$(d)../../src/makefile))
