CUSTOM_FLAGS_ALL := --csl=apa.csl --bibliography=bib.bib -H config.sty -s --highlight-style pygments

PDFS := $(patsubst %.md,%.md.pdf,$(wildcard **/*.md))

all : $(PDFS)

%.md.pdf : %.md
	pandoc $(CUSTOM_FLAGS_ALL) $(CUSTOM_FLAGS) --latex-engine=xelatex $< -o $@

clean :
	rm $(PDFS)

rebuild : clean all
