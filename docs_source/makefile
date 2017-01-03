.PHONY: all
all: wayverb.pdf

wayverb.pdf: $(SOURCES) pdfs
	pandoc $(PANDOC_OPTIONS) -o $@ $(SOURCES)

.PHONY: pdfs
pdfs: sketches $(patsubst %.svg,%.pdf,$(wildcard images/*))

.PHONY: sketches
sketches:
	$(MAKE) -C images/sketch
	cp images/sketch/*.svg images

%.pdf: %.svg
	inkscape --export-pdf=$@ --without-gui $<

PANDOC_OPTIONS = \
	--smart \
	--chapter \
	--variable=colorlinks:true \
	--variable=geometry:margin=2cm \
	--variable=geometry:a4paper \
	--variable=documentclass:scrreprt \
	--metadata=link-citations:true \
	--latex-engine=lualatex \
	--bibliography=wayverb.bib \
	--csl=ieee-with-url.csl \
	--default-image-extension=pdf

SOURCES := \
	title_page.md \
	acknowledgements.md \
	introduction.md \
	context.md \
	image_source.md \
	ray_tracer.md \
	waveguide.md \
	hybrid.md \
	microphone.md \
	boundary.md \
	evaluation.md \
	bibliography.md

.PHONY: check
check:
	- echo "weasel"
	- ./writegood/weasel.sh $(SOURCES)
	- echo
	- echo "passive"
	- ./writegood/passive.sh $(SOURCES)
	- echo
	- echo "duplicates"
	- ./writegood/dups.pl $(SOURCES)

.PHONY: clean
clean:
	- rm wayverb.pdf
