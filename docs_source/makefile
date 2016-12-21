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
	--csl=ieee.csl \
	--default-image-extension=pdf

SOURCES := \
	title_page.md \
	acknowledgements.md \
	introduction.md \
	context.md \
	image_source.md \
	ray_tracer.md \
	waveguide.md \
	microphone.md \
	boundary.md \
	evaluation.md \
	bibliography.md

%.pdf : %.svg
	inkscape --export-pdf=$@ --without-gui $<

PDF_FILES := $(patsubst %.svg,%.pdf,$(wildcard images/*))

wayverb.pdf : $(SOURCES) $(PDF_FILES)
	pandoc $(PANDOC_OPTIONS) -o $@ $(SOURCES)

.PHONY: all clean

all : wayverb.pdf

clean:
	- rm wayverb.pdf
