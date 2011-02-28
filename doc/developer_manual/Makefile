#
# http://www.maxwit.com
#

BOOK_TEX := $(wildcard g-bios_*_manual.tex)
BOOK_PDF := $(BOOK_TEX:%.tex=%.pdf)

all: $(BOOK_PDF)
	@echo
	@ls -lh *.pdf
	@echo

%.pdf: %.tex ../style.tex
	latex $<
	latex $<
	dvipdfmx $(<:%.tex=%.dvi)
	@rm $(<:.tex=.aux)
	@rm $(<:.tex=.dvi)
	@rm $(<:.tex=.log)
	@rm $(<:.tex=.out)
	@rm $(<:.tex=.toc)

clean:
	@for set in $(wildcard *.tex); \
	do \
		rm -rvf $${set%.tex}; \
		for fn in `ls $${set%tex}*`; \
		do \
			if [ $${fn##*.} != tex ]; then rm -rvf $$fn; fi \
		done \
	done
