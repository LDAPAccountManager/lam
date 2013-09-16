#!/bin/bash

rm -rf docs/devel/phpdoc
mkdir docs/devel/phpdoc
phpdoc -d ./ -t docs/devel/phpdoc --title "LDAP Account Manager" --template old-ocean --defaultpackagename main -e php,inc
rm phpdoc*.log 
