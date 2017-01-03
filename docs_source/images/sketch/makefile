.PHONY: all
all: $(patsubst %.sk,%.svg,$(wildcard ./*))

%.svg: %.pdf
	pdf2svg $< $@

%.pdf: %.tex
	xelatex $<

%.tex: %.sk
	sketch -t template.tex $< -o $@
