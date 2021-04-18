#!/bin/bash

set -e

wget -q http://localhost/lam/templates/login.php

grep -v -i "ldap" login.php > /dev/null

set +e

grep -i "error" login.php
if [ $? -ne 1 ]; then
  echo "Error found"
  exit 1
fi

set -e

rm login.php

exit 0
