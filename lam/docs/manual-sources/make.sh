#!/bin/bash
# $Id$
#
# Copyright (C) 2009 - 2014  Roland Gruber
# This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)

# This script is run to create the LAM manual.


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
