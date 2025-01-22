include prorab.mk
include prorab-license.mk
include prorab-install-dbgsrc.mk
include prorab-clang-format.mk

$(eval $(call prorab-config, ../config))

this_name := papki

this_soname := $(shell cat $(d)soname.txt)

this_srcs := $(call prorab-src-dir, $(this_name))

this_cxxflags += -isystem ../src_deps

this__libminizip := ../src_deps/out/$(c)/libminizip.a

this_ldlibs += $(this__libminizip)
this_ldlibs += -lz

ifneq ($(os),macosx)
    this_ldlibs += -lstdc++fs
endif

$(eval $(prorab-build-lib))

this_src_dir := $(this_name)

$(eval $(prorab-clang-format))

this_license_file := ../LICENSE
$(eval $(prorab-license))

$(eval $(prorab-install-dbgsrc))

$(eval $(call prorab-include, ../src_deps/makefile))