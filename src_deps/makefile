include prorab.mk

# =======================================
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
