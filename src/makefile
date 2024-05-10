include prorab.mk
include prorab-license.mk
include prorab-install-dbgsrc.mk
include prorab-clang-format.mk

# =======================++==============
# =============== minizip ===============
$(eval $(prorab-clear-this-vars))

this_name := minizip

$(eval $(call prorab-config, ../config))

this_srcs += $(call prorab-src-dir, minizip)

this_static_lib_only := true
this_no_install := true

# no lint for minizip
this_lint_cmd :=

this_cflags += -D NOUNCRYPT

$(eval $(prorab-build-lib))

# =====================================
# =============== papki ===============
$(eval $(prorab-clear-this-vars))

$(eval $(call prorab-config, ../config))

this_name := papki

this_soname := $(shell cat $(d)soname.txt)

this_srcs := $(call prorab-src-dir, $(this_name))

this_cxxflags += -isystem minizip

this__libminizip := out/$(c)/libminizip.a

this_ldlibs += $(this__libminizip)
this_ldlibs += -lz

ifneq ($(os),macosx)
    this_ldlibs += -lstdc++fs
endif

$(eval $(prorab-build-lib))

$(eval $(call prorab-depend, $(prorab_this_name), $(this__libminizip)))

this_src_dir := $(this_name)

$(eval $(prorab-clang-format))

this_license_file := ../LICENSE
$(eval $(prorab-license))

$(eval $(prorab-install-dbgsrc))
