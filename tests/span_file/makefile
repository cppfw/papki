include prorab.mk

this_name := tests

this_cxxflags += -std=c++11
this_cxxflags += -Wall
this_cxxflags += -DDEBUG
this_cxxflags += -fstrict-aliasing

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lpapki -lstdc++

this_ldflags += -L$(d)../../src/

ifeq ($(os),macosx)
    this_cxxflags += -stdlib=libc++ #this is needed to be able to use c++11 std lib
    this_ldlibs += -lc++
else ifeq ($(os),windows)
endif

this_no_install := true

$(eval $(prorab-build-app))

# add dependency on libpapki.so
$(prorab_this_name): $(abspath $(d)../../src/libpapki$(dot_so))

include $(d)../test_target.mk

# include makefile for building papki
$(eval $(call prorab-include,../../src/makefile))
