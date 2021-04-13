prorab-lib-path-run = (cd $(d) && \
        $(if $(filter $(os),windows), \
                cmd //C 'set PATH=$1;%PATH% && $2', \
                $(if $(filter $(os),macosx), \
                        DYLD_LIBRARY_PATH=$1 $2, \
                        $(if $(filter $(os),linux), \
                                LD_LIBRARY_PATH=$1 $2, \
                                $(error "unknown OS") \
                            ) \
                    ) \
            ) )

define this_rule
test:: $(prorab_this_name)
$(.RECIPEPREFIX)@myci-running-test.sh $(notdir $(abspath $(d)))
$(.RECIPEPREFIX)$(a)$(call prorab-lib-path-run,../../src/out/$(c),out/$(c)/$$(notdir $$^)) || myci-error.sh "test failed"
$(.RECIPEPREFIX)@myci-passed.sh
endef
$(eval $(this_rule))
