#!/bin/bash
# $Id$
#
# Copyright (C) 2009 - 2014  Roland Gruber
# This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)

# This script is run to create the LAM manual.

rm -rf ../manual
mkdir ../manual
xsltproc --xinclude -o ../manual/ --stringparam html.stylesheet.type text/css --stringparam html.stylesheet style.css /usr/share/xml/docbook/stylesheet/nwalsh/html/chunk.xsl howto.xml
mkdir ../manual/images
cp images/*.png ../manual/images
mkdir ../manual/resources
cp resources/*.* ../manual/resources
cp style.css ../manual
