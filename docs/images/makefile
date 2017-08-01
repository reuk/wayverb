.PHONY: all
all: pdfs

.PHONY: pdfs
pdfs: sketches $(patsubst %.svg,%.pdf,$(wildcard *))

.PHONY: sketches
sketches:
	$(MAKE) -C sketch
	cp sketch/*.svg .

%.pdf: %.svg
	inkscape --export-pdf=$@ --without-gui $<
