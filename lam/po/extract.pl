#!/usr/bin/perl
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   original from Horde,
#   modified for LAM by Roland Gruber
#
# Perl script to extract strings from all the files and print
# to stdout for use with xgettext.

use FileHandle;
use File::Basename;
use File::Find;
use Cwd;

use strict;
use vars qw($exts @dirs $dirs %strings);

chdir(dirname($0));

@dirs = qw($ /help /templates /lib);

$exts = '(\.php$|\.inc$)';
$dirs = '^' . cwd() . '/..(' . join('|', @dirs) . ')';

find(\&extract, cwd() . '/..');
print join("\n", sort keys %strings), "\n";

sub extract
{
  my $file = $File::Find::name;
  my $dir  = $File::Find::dir;
  my $fd   = new FileHandle;

  if ($dir !~ /$dirs/s) {
    $File::Find::prune = 1;
    return;
  }

  if ($file =~ /$exts/) {
    open($fd, basename($file));
    my $data = join('', <$fd>);
    while ($data =~ s/_\("(.*?)"\)//s) {
      $strings{"_(\"$1\")"}++;
    }
    close($fd);
  }
}
