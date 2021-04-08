include prorab.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lpapki -lutki
this_ldflags += -L$(d)../../src/out/$(c)

this_no_install := true

$(eval $(prorab-build-app))

# add dependency on libpapki.so
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libpapki$(dot_so))

include $(d)../test_target.mk

# include makefile for building papki
$(eval $(call prorab-include, ../../src/makefile))
