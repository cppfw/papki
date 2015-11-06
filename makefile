include prorab.mk


$(eval $(prorab-build-subdirs))


install::
#install pkg-config files
	$(prorab_echo)prorab-apply-version.sh `prorab-deb-version.sh $(prorab_this_dir)debian/changelog` $(prorab_this_dir)pkg-config/*.pc.in
	$(prorab_echo)install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	$(prorab_echo)install $(prorab_this_dir)pkg-config/*.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig


$(prorab-clear-this-vars)

this_soname_dependency := $(prorab_this_dir)src/soname.txt

this_soname := $(shell cat $(this_soname_dependency))

$(eval $(prorab-build-deb))
