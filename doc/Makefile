.PHONY: TimeTaggerManual.pdf all clean

all: TimeTaggerManual.pdf

TimeTaggerManual.pdf: TimeTaggerManual.tex
	latexmk -pdf -pdflatex="pdflatex -interaction=nonstopmode" -use-make TimeTaggerManual.tex

clean:
	latexmk -CA

dist-clean:
	latexmk -CA
