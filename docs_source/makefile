PANDOC_OPTIONS = \
	--smart \
	--chapter \
	--variable=colorlinks:true \
	--variable=geometry:margin=2cm \
	--variable=geometry:a4paper \
	--variable=documentclass:scrreprt \
	--metadata=link-citations:true \
	--bibliography=wayverb.bib \
	--csl=ieee.csl

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
	the_future.md \
	bibliography.md

wayverb.pdf : $(SOURCES)
	pandoc $(PANDOC_OPTIONS) -o $@ $^

%.png : %.svg
	inkscape --export-png=$@ --export-dpi=300 --export-background-opacity=0 --without-gui $<

PNG_FILES := $(patsubst %.svg,%.png,$(wildcard images/*))

png : $(PNG_FILES)

.PHONY: all clean

all : wayverb.pdf

clean:
	- rm wayverb.pdf
