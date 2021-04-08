include prorab.mk

$(eval $(call prorab-config, ../config))

this_name := papki

this_soname := $(shell cat $(prorab_this_dir)soname.txt)

this_srcs := $(call prorab-src-dir, .)

this_ldlibs += -lz

$(eval $(prorab-build-lib))
