#!/bin/bash

echo "Creating eps files from image files and reducing sizes"

for i in images/*jpg
do
	EPSFILE=`dirname $i`/`basename $i .jpg`.eps
	if [ $i -nt $EPSFILE ]
	then
		convert $i -resize 1506x1506\> -quality 50 -density 72 $EPSFILE
		ps2ps -dFitPage $EPSFILE $EPSFILE.temp
		mv $EPSFILE.temp $EPSFILE
	fi
done

for i in images/*png
do
	EPSFILE=`dirname $i`/`basename $i .png`.eps
        if [ $i -nt $EPSFILE ]
	then    
                convert $i -resize 1506x1506\> -quality 50 -density 72 $EPSFILE
                ps2ps -dFitPage $EPSFILE $EPSFILE.temp
                mv $EPSFILE.temp $EPSFILE
	fi      
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
