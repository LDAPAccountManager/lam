#!/bin/bash

rm -rf docs/phpdoc
mkdir docs/phpdoc
/usr/bin/phpdoc -d ./ -t docs/phpdoc --template=default --title "LDAP Account Manager" --defaultpackagename main --extensions=php --extensions=inc --ignore=config --ignore=tmp --ignore=sess --ignore=lib/env.inc --ignore=lib/3rdParty --ignore=templates/3rdParty --ignore=tests
