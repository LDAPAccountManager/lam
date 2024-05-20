#!/bin/sh 

dir=`dirname $0`

if [ -x /usr/bin/php ]; then
  # delimiter must be added to support arguments starting with "--"
	/usr/bin/php -f $dir/cronGlobal.inc delimiter $*
	exit $?
fi

echo "No PHP executable found"

exit 1