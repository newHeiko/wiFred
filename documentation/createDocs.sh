#!/bin/bash

echo "Creating png files from pdf files"
cd largeImages
for i in *pdf
do
    PNGFILE=../images/`basename $i .pdf`.png
    if [ $i -nt $PNGFILE ]
    then
	convert -density 150 $i $PNGFILE
    fi
done
cd ..

echo "Reducing jpg images in size"
cd largeImages
for i in *jpg
do
    if [ $i -nt ../images/$i ]
    then
	convert $i -resize 1920x1920 -quality 50 -strip ../images/$i
    fi
done
cd ..

echo "Creating html and pdf files"
asciidoctor -b docbook docu_en.adoc
asciidoctor -b docbook docu_de.adoc
asciidoctor -b html5 docu_en.adoc
asciidoctor -b html5 docu_de.adoc
dblatex -P figure.note="/etc/asciidoc/images/icons/note" -P figure.important="/etc/asciidoc/images/icons/important" -P figure.warning="/etc/asciidoc/images/icons/warning" -P doc.collab.show=0 -P paper.type=a4paper -P page.margin.top=15mm -P page.margin.bottom=15mm -P page.margin.inner=20mm -P page.margin.outer=20mm docu_en.xml 
dblatex -P figure.note="/etc/asciidoc/images/icons/note" -P figure.important="/etc/asciidoc/images/icons/important" -P figure.warning="/etc/asciidoc/images/icons/warning" -P doc.collab.show=0 -P paper.type=a4paper -P page.margin.top=15mm -P page.margin.bottom=15mm -P page.margin.inner=20mm -P page.margin.outer=20mm docu_de.xml


exit
