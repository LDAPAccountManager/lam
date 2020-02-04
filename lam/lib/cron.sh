#!/bin/sh 

dir=`dirname $0`

if [ -x /usr/bin/php ]; then
	/usr/bin/php -f $dir/cron.inc $*
	exit $?
fi

echo "No PHP executable found"

exit 1