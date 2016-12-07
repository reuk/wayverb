PANDOC_OPTIONS = \
	-V geometry:margin=3cm \
	-V geometry:a4paper \
	-V documentclass:scrreprt \
	--smart \
	--chapter \
	--toc \
	--metadata link-citations:true \
	--bibliography=wayverb.bib \
	--csl=apa.csl

SOURCES := \
	title_page.md \
	abstract.md \
	acknowledgements.md \
	introduction.md \
	context.md \
	image_source.md \
	ray_tracer.md \
	waveguide.md \
	microphone.md \
	boundary.md \
	bibliography.md

wayverb.pdf : $(SOURCES)
	pandoc $(PANDOC_OPTIONS) -o $@ $^

.PHONY: all clean

all : wayverb.pdf

clean:
	- rm wayverb.pdf
