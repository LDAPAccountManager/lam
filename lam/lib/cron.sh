#!/bin/sh 

dir=`dirname $0`

if [ -x /usr/bin/php5 ]; then
	/usr/bin/php5 -f $dir/cron.inc $*
	exit $?
elif [ -x /usr/bin/php ]; then
	/usr/bin/php -f $dir/cron.inc $*
	exit $?
fi

echo "No PHP executable found"

exit 1