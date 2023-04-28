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

echo "Creating html files"
for i in about assembly config docu esp12-schematic esp32-schematic leds server usage
do
	for j in de en
	do
		if [[ ${i}_$j.adoc -nt ${i}_$j.html ]] || [[ version_$j.adoc -nt ${i}_$j.html ]]
		then
			asciidoctor -b html5 ${i}_$j.adoc
		fi
	done
done

echo "Creating pdf files"
for j in de en
do
	for i in about assembly config esp12-schematic esp32-schematic leds server usage pdfmaster version
	do
		if [[ ${i}_$j.adoc -nt pdfmaster_$j.pdf ]]
		then
			asciidoctor -b docbook5 pdfmaster_$j.adoc
			dblatex -P figure.note="/etc/asciidoc/images/icons/note" -P figure.important="/etc/asciidoc/images/icons/important" -P figure.warning="/etc/asciidoc/images/icons/warning" -P doc.collab.show=0 -P paper.type=a4paper -P page.margin.top=15mm -P page.margin.bottom=15mm -P page.margin.inner=20mm -P page.margin.outer=20mm pdfmaster_$j.xml
			break
		fi
	done
done

exit
