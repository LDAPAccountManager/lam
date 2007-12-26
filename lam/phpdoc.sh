#!/bin/bash

rm -rf phpdoc
mkdir phpdoc
phpdoc -ue on --output "HTML:Smarty:PHP" -d ./ -t phpdoc -ti "LDAP Account Manager - Documentation" -dc "LDAP Account Manager"
 
