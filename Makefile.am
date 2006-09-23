SUBDIRS = src scripts

dist-hook:
	mkdir $(distdir)/autopackage; \
		cp -p $(srcdir)/autopackage/* $(distdir)/autopackage
	mkdir $(distdir)/projects; \
		cp -p $(srcdir)/projects/*.vcproj $(distdir)/projects
	
