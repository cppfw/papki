include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := tests

this_srcs += $(call prorab-src-dir, .)

this_ldlibs += -lpapki -lutki
this_ldflags += -L$(d)../../src/out/$(c)

this_no_install := true

$(eval $(prorab-build-app))

# add dependency on libpapki.so
$(prorab_this_name): $(abspath $(d)../../src/out/$(c)/libpapki$(dot_so))

this_test_cmd := $(prorab_this_name)
this_test_deps := $(prorab_this_name)
this_test_ld_path := ../../src/out/$(c)
# for conan
this_test_ld_path += $(patsubst -L%,%,$(filter -L%,$(LDFLAGS)))

$(eval $(prorab-test))

# include makefile for building papki
$(eval $(call prorab-include, ../../src/makefile))
