#!/bin/bash

rm -rf docs/devel/phpdoc
mkdir docs/devel/phpdoc
phpdoc -d ./ -t docs/devel/phpdoc --title "LDAP Account Manager - Documentation" --defaultpackagename main -e php,inc
 
