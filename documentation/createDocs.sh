#!/bin/bash

echo "Creating eps files from image files"

for i in images/*jpg
do
	convert $i -resize 1506x1506\> -quality 50 -density 72 `dirname $i`/`basename $i .jpg`.eps
done

for i in images/*png
do
	convert  $i -resize 1506x1506\> -quality 50 -density 72 `dirname $i`/`basename $i .png`.eps
done

echo "Reducing eps image sizes"

for i in images/*eps
do
	ps2ps -dEPSFitPage $i $i.temp
	mv $i.temp $i
done

echo "Creating all docs from latex source file"
pdflatex docu.tex
pdflatex docu.tex
gs -sDEVICE=pdfwrite -dPDFSETTINGS=/ebook -dNOPAUSE -dQUIET -dBATCH -sOutputFile=docu.pdf.temp docu.pdf
mv docu.pdf.temp docu.pdf
latex docu.tex
latex docu.tex
cp docu.tex html/
cd html
hevea -fix -s article.hva docu.tex
imagen docu
rm docu.tex
cd ..
