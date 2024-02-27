#!/bin/bash
# $Id: make.sh 5301 2014-03-10 18:46:28Z gruberroland $
#
# Copyright (C) 2014  Roland Gruber
# This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)

# This script is run to create the LAM manual.

dir=`pwd`

rm -rf ../manual-pdf
mkdir ../manual-pdf
xsltproc --xinclude -o ../manual-pdf/out.fo --stringparam paper.type "A4" --stringparam generate.toc "book toc,title,table,figure" --stringparam "body.start.indent" "0pt" /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl howto.xml
mkdir ../manual-pdf/images
for img in `ls images/*.png`; do
	convert -density 96 $img ../manual-pdf/$img
done
cp images/schema_*.png ../manual-pdf/images/
mkdir ../manual-pdf/resources
cp resources/*.* ../manual-pdf/resources
cd ../manual-pdf
fop out.fo manual.pdf


cd $dir