#!/bin/bash

echo "Reducing eps image sizes"

for i in images/*eps
do
	ps2ps $i $i.temp
	mv $i.temp $i
done

echo "Creating all docs from latex source file"
pdflatex docu.tex
pdflatex docu.tex
gs -sDEVICE=pdfwrite -dPDFSETTINGS=/ebook -dNOPAUSE -dQUIET -dBATCH -sOutputFile=docu.pdf.temp docu.pdf
mv docu.pdf.temp docu.pdf
latex docu.tex
latex docu.tex
cd html
imagen ../docu.tex
hevea article.hva ../docu.tex
cd ..
