#!/bin/sh

cd `dirname $0`;
perl extract.pl | xgettext --keyword=_ -C --no-location -
