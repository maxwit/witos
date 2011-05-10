#
# http://www.maxwit.com
#

DESTDIR := ../../pdf/

ART_TEX  := $(wildcard *.art.tex)
BOOK_TEX := $(wildcard *.tex)
BOOK_TEX := $(filter-out style.tex, $(BOOK_TEX))
BOOK_TEX := $(filter-out $(ART_TEX), $(BOOK_TEX))
BOOK_PDF := $(BOOK_TEX:%.tex=%.pdf)

all: $(BOOK_PDF)
	@echo
	@ls -lh *.pdf
	@echo

%.pdf: %.tex $(ART_TEX)
	xelatex $<
	xelatex $<
	@rm -v $(<:.tex=.aux)
	@rm -v $(<:.tex=.log)
	@rm -v $(<:.tex=.toc)
	@rm -v $(<:.tex=.out)

install:
	@cp -v $(BOOK_PDF) $(DESTDIR)/

clean:
	@for set in $(wildcard *.tex); \
	do \
		for fn in `ls $${set%tex}*`; \
		do \
			if [ $${fn##*.} != tex ]; then rm -rvf $$fn; fi \
		done \
	done
