# Note: zlib-1.2.2 and RAPID201 are not included in this package.
# 
# You will need to download and install them separately. zlib is already
# installed on many systems. 
#
# See Makefile.definitions file for variables that specify the locations
# of the libraries and header files.

SUBDIRS = zlib-1.2.3 RAPID201 nifti util contrack_gen contrack_score quench

SUBDIR_MAKERULE= \
	if test ! -d $$d; then \
		if test "$(NOSUBMESG)" = "" ; then \
			echo "**** WARNING: $$d: No such directory. **** "; \
		fi \
	else \
		echo "	(cd $$d; $(MAKE) $${RULE:=$@})"; \
		(cd $$d; ${MAKE} $${RULE:=$@}); \
	fi

default:
	@for d in $(SUBDIRS); do $(SUBDIR_MAKERULE); done

clean:
	@for d in $(SUBDIRS); do $(SUBDIR_MAKERULE); done

util:
	@for d in util; do $(SUBDIR_MAKERULE); done

contrack_gen:
	do $(SUBDIR_MAKERULE);
