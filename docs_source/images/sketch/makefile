%.tex : %.sk
	sketch -t template.tex $< -o $@

%.pdf : %.tex
	xelatex $< -o $@

%.svg : %.pdf
	pdf2svg $< $@

# probably not needed ever

%.mps : %.mp
	mpost $<

all : house.svg topology.svg
