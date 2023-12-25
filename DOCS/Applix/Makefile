# Makefile for Applix documentation.
# You need Applix to use this (obviously)

APPLIX := /mnt/cdrom/ax_live/applix

SRCS := preface prog-guide development user-guide

PSTARGETS := $(foreach i,$(SRCS),../PS/$i.ps)
HTMLTARGETS := $(foreach i,$(SRCS),../HTML/$i.html)

.PHONY: all
all:	psdocs htmldocs

.PHONY: psdocs,htmldocs
psdocs: $(PSTARGETS)

htmldocs: $(HTMLTARGETS)

.PHONY: clean
clean:
	$(RM) $(PSTARGETS) $(HTMLTARGETS) ../HTML/*.gif

../PS/%.ps:%.aw
	@$(APPLIX) -macro applix2ps -pass $$PWD/$< | cat

../HTML/%.html:%.aw
	@$(APPLIX) -call applix2html ./$< $@ ; \
	echo "============================>Generated $@"

