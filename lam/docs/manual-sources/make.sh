#!/bin/bash
# $Id$
#
# Copyright (C) 2009 - 2014  Roland Gruber
# This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)

# This script is run to create the LAM manual.

dir=`pwd`

rm -rf ../manual
mkdir ../manual
xsltproc -o ../manual/ --stringparam html.stylesheet.type text/css --stringparam html.stylesheet style.css /usr/share/xml/docbook/stylesheet/nwalsh/html/chunk.xsl howto.xml
mkdir ../manual/images
cp images/*.png ../manual/images
cp images/*.jpg ../manual/images
mkdir ../manual/resources
cp resources/*.* ../manual/resources
cp style.css ../manual

rm -rf ../manual-onePage
mkdir ../manual-onePage
xsltproc -o ../manual-onePage/ --stringparam html.stylesheet.type text/css --stringparam html.stylesheet style.css /usr/share/xml/docbook/stylesheet/nwalsh/html/onechunk.xsl howto.xml
mkdir ../manual-onePage/images
cp images/*.png ../manual-onePage/images
cp images/*.jpg ../manual-onePage/images
mkdir ../manual-onePage/resources
cp resources/*.* ../manual-onePage/resources
cp style.css ../manual-onePage

rm -rf ../manual-pdf
mkdir ../manual-pdf
xsltproc -o ../manual-pdf/out.fo --stringparam paper.type "A4" --stringparam generate.toc "book toc,title,table,figure" /usr/share/xml/docbook/stylesheet/nwalsh/fo/docbook.xsl howto.xml
mkdir ../manual-pdf/images
for img in `ls images/*.png`; do
	convert -density 96 $img ../manual-pdf/$img
done
for img in `ls images/*.jpg`; do
	convert -density 96 $img ../manual-pdf/$img
done
cp images/schema_*.png ../manual-pdf/images/
mkdir ../manual-pdf/resources
cp resources/*.* ../manual-pdf/resources
cd ../manual-pdf
fop out.fo manual.pdf


cd $dir