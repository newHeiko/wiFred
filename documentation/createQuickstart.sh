asciidoctor -b docbook quickstartLaser.adoc
asciidoctor -b docbook quickstartDrilljig.adoc
dblatex -P figure.warning="/etc/asciidoc/images/icons/warning" -P latex.output.revhistory=0 -P paper.type=a6paper -P doc.collab.show=0 -P doc.layout="title mainmatter" -P page.margin.bottom=2mm -P page.margin.top=5mm -P page.margin.inner=5mm -P page.margin.outer=5mm -P latex.class.options=notitlepage quickstartLaser.xml 
dblatex -P figure.warning="/etc/asciidoc/images/icons/warning" -P latex.output.revhistory=0 -P paper.type=a6paper -P doc.collab.show=0 -P doc.layout="title mainmatter" -P page.margin.bottom=2mm -P page.margin.top=5mm -P page.margin.inner=5mm -P page.margin.outer=5mm -P latex.class.options=notitlepage quickstartDrilljig.xml 
