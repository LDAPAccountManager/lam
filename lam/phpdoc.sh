#!/bin/bash

rm -rf docs/phpdoc
mkdir docs/phpdoc
phpdoc -d ./ -t docs/phpdoc --title "LDAP Account Manager" --template clean --defaultpackagename main -e php,inc --ignore lib/en*,lib/env.inc,lib/3rdParty*,templates/3rdParty*
