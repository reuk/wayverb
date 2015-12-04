CUSTOM_FILE_0 := mic_modelling/mic_modelling.md.pdf
CUSTOM_FLAGS_0 := --csl=apa.csl --bibliography=bib.bib -H config.sty -s --highlight-style pygments

PDFS := $(patsubst %.md,%.md.pdf,$(wildcard **/*.md))

all : $(PDFS)

$(CUSTOM_FILE_0) : CUSTOM_FLAGS := $(CUSTOM_FLAGS_0)

%.md.pdf : %.md
	pandoc $(CUSTOM_FLAGS) --latex-engine=xelatex $< -o $@

clean :
	rm $(PDFS)

rebuild : clean all
