.PHONY: all
all: wayverb.pdf

wayverb.pdf: $(SOURCES) imgs haskell/audioTable
	pandoc $(PANDOC_OPTIONS) -o $@ $(SOURCES)

wayverb.tex: $(SOURCES) imgs haskell/audioTable
	pandoc $(PANDOC_OPTIONS) -o $@ $(SOURCES) --standalone

diff.tex: wayverb.tex wayverb.old.tex
	latexdiff \
		--config "PICTUREENV=(?:picture|equation|DIFnomarkup|enumerate)[\w\d*@]*" \
		wayverb.old.tex wayverb.tex > diff.tex \
		#

diff.pdf: diff.tex
	./fix_latexdiff_output.py -i diff.tex
	lualatex diff.tex
	lualatex diff.tex

.PHONY: imgs
imgs:
	make -C images

PANDOC_OPTIONS = \
	--smart \
	--chapter \
	--variable=colorlinks:true \
	--variable=geometry:margin=2cm \
	--variable=geometry:a4paper \
	--variable=documentclass:scrreprt \
	--filter haskell/audioTable \
	--filter pandoc-fignos \
	--filter pandoc-eqnos \
	--filter pandoc-tablenos \
	--bibliography=wayverb.bib \
	--csl=ieee-with-url.csl \
	--default-image-extension=pdf \
	--metadata=link-citations:true \
	--latex-engine=lualatex \
	#

%.tex: %.md
	pandoc $(PANDOC_OPTIONS) --standalone -o $@ $<

%.pdf: %.md
	pandoc $(PANDOC_OPTIONS) -o $@ $<

haskell/audioTable: haskell/audioTable.hs
	ghc -package pandoc-types-1.16 --make $<

SOURCES := \
	title_page.md \
	acknowledgements.md \
	introduction.md \
	context.md \
	theory.md \
	image_source.md \
	ray_tracer.md \
	waveguide.md \
	hybrid.md \
	microphone.md \
	boundary.md \
	evaluation.md \
	conclusion.md \
	bibliography.md \
	#

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
