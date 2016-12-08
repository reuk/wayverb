PANDOC_OPTIONS = \
	--smart \
	--chapter \
	--toc \
	--variable=geometry:margin=3cm \
	--variable=geometry:a4paper \
	--variable=documentclass:scrreprt \
	--metadata=link-citations:true \
	--bibliography=wayverb.bib \
	--csl=apa.csl

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

.PHONY: all clean

all : wayverb.pdf

clean:
	- rm wayverb.pdf
