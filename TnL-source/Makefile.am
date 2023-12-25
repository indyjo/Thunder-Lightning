SUBDIRS = src scripts

if WITH_TESTS
SUBDIRS += tests
endif

dist-hook:
	mkdir $(distdir)/autopackage; \
		cp -pr $(srcdir)/autopackage/* $(distdir)/autopackage
	mkdir $(distdir)/extras; \
		cp -p $(srcdir)/extras/* $(distdir)/extras
	
