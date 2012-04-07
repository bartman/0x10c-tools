SUBDIRS        = lib src
SUBDIR_ACTIONS = all clean distclean
SUBDIR_TARGETS = $(foreach act,${SUBDIR_ACTIONS},$(patsubst %,%/${act},${SUBDIRS}))


-include Makefile.conf

.PHONY: ${SUBDIR_ACTIONS} ${SUBDIR_TARGETS}

.DEFAULT: all

${SUBDIR_ACTIONS}: %: do-%

$(SUBDIR_ACTIONS:%=do-%): do-%:
	${MAKE} $(patsubst %,%/$*,${SUBDIRS})

${SUBDIR_TARGETS}:
	${MAKE} -C $(dir $@) $(filter-out all,$(notdir $@))

all clean distclean: 
	@echo local $@

.PHONY: help

help:
	@echo make all            - build everything
	@echo make clean          - cleanup

# --- ctags and cscope

.PHONY: tags cscope cscope.files

tags: cscope ctags

cscope: cscope.out
cscope.out: cscope.files
	cscope -P`pwd` -b

cscope.files:
	find . -name '*.[ch]' -o -name '*.[ch]pp' | grep -v -e CVS -e SCCS > $@

ctags: cscope.files
	ctags -L $<

