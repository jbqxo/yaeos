# Extracts rel-files on which unit test depends.
#
#        /* UNITY_TEST DEPENDS ON: kernel/lib/mm/buddy.c
#         * UNITY_TEST DEPENDS ON: kernel/lib/mm/linear.c
#         * UNITY_TEST DEPENDS ON: kernel/lib/ds/bitmap.c
#         */
#
# If such comment would be at the beginning of the file passed to the function,
# then it will return absolute paths to the files if they exist.
define get_test_dependencies
    $(foreach absfile,\
        $(foreach f,\
            $(shell \
                $(GREP) "UNITY_TEST DEPENDS ON:" $(1) |\
                $(SED) -e 's/^.*UNITY_TEST DEPENDS ON: \(.*\)/\1/' -e 's/\*\/$///' |\
                $(TR) "\n" " "\
            ),\
            $(abspath $(ROOT)/$(f))\
        ),\
        $(if $(wildcard $(absfile)),\
            $(absfile), $(error Cannot find $(absfile). Required by $(1)))\
    )
endef
