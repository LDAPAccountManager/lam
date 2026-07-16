# makefile
#
# @since       2015-05-14
# @category    Library
# @package     PdfFont
# @author      Nicola Asuni <info@tecnick.com>
# @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
# @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
# @link        https://github.com/tecnickcom/tc-lib-pdf-font
#
# This file is part of tc-lib-pdf-font software library.
#
# Download and convert fonts for tc-lib-pdf-font.
# ----------------------------------------------------------------------------------------------------------------------

# Folder containing the font to install (package)
PKGFONTDIR=.

# License name of the font to be packaged
PKGFONTLICENSE=.

# Project owner
OWNER=tecnickcom

# Project vendor
VENDOR=${OWNER}

# Project name
PROJECT=tc-lib-pdf-font-data-$(PKGFONTDIR)

# Project version
VERSION=$(shell cat ../VERSION)

# Project release number (packaging build number)
RELEASE=$(shell cat ../RELEASE)

# Name of RPM or DEB package
PKGNAME=php-${OWNER}-${PROJECT}

# Data dir
DATADIR=usr/share

# PHP home folder
PHPHOME=${DATADIR}/php/Com/Tecnick

# Default installation path for code
FONTPATH=${PHPHOME}/Pdf/Font/fonts/$(PKGFONTDIR)/

# Default installation path for documentation
DOCPATH=${DATADIR}/doc/$(PKGNAME)/

# Installation path for the code
PATHINSTFONT=$(DESTDIR)/$(FONTPATH)

# Installation path for documentation
PATHINSTDOC=$(DESTDIR)/$(DOCPATH)

# Current directory
CURRENTDIR=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

# RPM Packaging path (where RPMs will be stored)
PATHRPMPKG=$(CURRENTDIR)/../target/RPM_FONTS/$(PKGFONTDIR)

# DEB Packaging path (where DEBs will be stored)
PATHDEBPKG=$(CURRENTDIR)/../target/DEB_FONTS/$(PKGFONTDIR)

# BZ2 Packaging path (where BZ2s will be stored)
PATHBZ2PKG=$(CURRENTDIR)/../target/BZ2_FONTS/$(PKGFONTDIR)


# Composer executable (disable APC to as a work-around of a bug)
COMPOSER=$(shell which php) -d "apc.enable_cli=0" $(shell which composer)

# sed argument for in-place substitutions
SEDINPLACE=-i
ifeq ($(shell uname -s),Darwin)
	SEDINPLACE=-i ''
endif

# Mago version
MAGOVERSION=1.43.0

# --- MAKE TARGETS ---

# Display general help about this command
.PHONY: help
help:
	@echo ""
	@echo "${PROJECT} Makefile."
	@echo "The following commands are available:"
	@echo ""
	@awk '/^## /{desc=substr($$0,4)} /^\.PHONY:/{if(NF>1) {target=$$2; if(desc) printf "  make %-15s: %s\n",target,desc; desc=""}}' Makefile
	@echo ""

# alias for help target
.PHONY: all
all: help

## delete the vendor and target directory
.PHONY: clean
clean:
	rm -rf ./vendor/
	rm -rf ../target/fonts

## Download all dependencies
.PHONY: deps
deps:
	($(COMPOSER) install --no-dev --no-interaction)

## Clean and download the composer dependencies (all TTF font files)
.PHONY: build
build: deps
	./bulk_convert.php

## Update composer dependencies
.PHONY: update
update:
	($(COMPOSER) update --no-interaction)

## Install this application
.PHONY: install
install: uninstall
	mkdir -p $(PATHINSTFONT)
	cp -rf ../target/fonts/$(PKGFONTDIR)/*.json $(PATHINSTFONT)
	cp -rf ../target/fonts/$(PKGFONTDIR)/*.z $(PATHINSTFONT) | true
	find $(PATHINSTFONT) -type d -exec chmod 755 {} \;
	find $(PATHINSTFONT) -type f -exec chmod 644 {} \;
	mkdir -p $(PATHINSTDOC)
	cp -f ../target/fonts/$(PKGFONTDIR)/LICENSE $(PATHINSTDOC)
	cp -f ../target/fonts/$(PKGFONTDIR)/README $(PATHINSTDOC)
	cp -f ../VERSION $(PATHINSTDOC)
	cp -f ../RELEASE $(PATHINSTDOC)
	chmod -R 644 $(PATHINSTDOC)*

## Remove all installed files
.PHONY: uninstall
uninstall:
	rm -rf $(PATHINSTFONT)

# --- PACKAGING ---

## Build the RPM package for RedHat-like Linux distributions
.PHONY: rpm
rpm:
	rm -rf $(PATHRPMPKG)
	rpmbuild --define \
	"_topdir $(PATHRPMPKG)" \
	--define "_vendor $(VENDOR)" \
	--define "_owner $(OWNER)" \
	--define "_project $(PROJECT)" \
	--define "_package $(PKGNAME)" \
	--define "_version $(VERSION)" \
	--define "_release $(RELEASE)" \
	--define "_current_directory $(CURRENTDIR)" \
	--define "_fontpath /$(FONTPATH)" \
	--define "_docpath /$(DOCPATH)" \
	--define "_fontdir $(PKGFONTDIR)" \
	--define "_license $(PKGFONTLICENSE)" \
	-bb ../resources/rpm/fonts.spec

## Build the DEB package for Debian-like Linux distributions
.PHONY: deb
deb:
	echo resources/fonts/control/$(DEBCOPYRIGHT)
	rm -rf $(PATHDEBPKG)
	make install DESTDIR=$(PATHDEBPKG)/$(PKGNAME)-$(VERSION) PKGFONTDIR=$(PKGFONTDIR)
	tar -zcvf $(PATHDEBPKG)/$(PKGNAME)_$(VERSION).orig.tar.gz -C $(PATHDEBPKG)/ $(PKGNAME)-$(VERSION)
	cp -rf ../resources/fonts/debian $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#DATE#~/`date -R`/" {} \;
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#VENDOR#~/$(VENDOR)/" {} \;
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#PROJECT#~/$(PROJECT)/" {} \;
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#PKGNAME#~/$(PKGNAME)/" {} \;
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#VERSION#~/$(VERSION)/" {} \;
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#RELEASE#~/$(RELEASE)/" {} \;
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#FONT#~/$(PKGFONTDIR)/" {} \;
	find $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/ -type f -exec sed $(SEDINPLACE) "s/~#LICENSE#~/$(PKGFONTLICENSE)/" {} \;
	echo $(FONTPATH) > $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/$(PKGNAME).dirs
	echo "$(FONTPATH)* $(FONTPATH)" > $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/install
	echo $(DOCPATH) >> $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/$(PKGNAME).dirs
	echo "$(DOCPATH)* $(DOCPATH)" >> $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/install
	echo "new-package-should-close-itp-bug" > $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/$(PKGNAME).lintian-overrides
	echo "extra-license-file $(DOCPATH)LICENSE.gz" >> $(PATHDEBPKG)/$(PKGNAME)-$(VERSION)/debian/$(PKGNAME).lintian-overrides
	cd $(PATHDEBPKG)/$(PKGNAME)-$(VERSION) && debuild -us -uc

## Build a compressed bz2 archive
.PHONY: bz2
bz2:
	rm -rf $(PATHBZ2PKG)
	make install DESTDIR=$(PATHBZ2PKG) PKGFONTDIR=$(PKGFONTDIR)
	tar -jcvf $(PATHBZ2PKG)/$(PKGNAME)-$(VERSION)-$(RELEASE).tbz2 -C $(PATHBZ2PKG) $(DATADIR)
