#!/bin/bash

rm -rf docs/devel/phpdoc
mkdir docs/devel/phpdoc
phpdoc -ue on --output "HTML:Smarty:PHP" -d ./ -t docs/devel/phpdoc -ti "LDAP Account Manager - Documentation" -dc "LDAP Account Manager" -dn main
 
