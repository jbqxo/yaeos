define get_test_dependencies
    $(foreach absfile,\
        $(foreach f,\
            $(shell \
                $(GREP) "UNITY_TEST DEPENDS ON:" $(1) | $(SED) 's/\/\/ UNITY_TEST DEPENDS ON: //' | $(TR) "\n" " "),\
            $(abspath $(ROOT)/$(f))),\
        $(if $(wildcard $(absfile)),\
            $(absfile), $(error Can't find $(absfile). Required by $(1))))
endef
