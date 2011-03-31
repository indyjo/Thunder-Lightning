SUBDIRS = src scripts tests

dist-hook:
	mkdir $(distdir)/autopackage; \
		cp -pr $(srcdir)/autopackage/* $(distdir)/autopackage
	mkdir $(distdir)/extras; \
		cp -p $(srcdir)/extras/* $(distdir)/extras
	
