#! /usr/bin/perl

# $Id$
#
#  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
#  Copyright (C) 2003  Tilo Lutz
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
#  LDAP Account Manager daemon to create and delete homedirecotries and quotas
$debug=true; # Show debug messages


use Quota; # Needed to get and set quotas
#use strict; # Use strict for security reasons

@quota_grp;
@quota_usr; # Filesystems with enabled userquotas
@vals = @ARGV;
	# vals = DN, PAssword, user, home, (add|rem),
	#                            quota, (set|get),(u|g), (mountpoint,blocksoft,blockhard,filesoft,filehard)+
	#                            chown  options
$|=1; # Disable buffering

sub get_fs { # Load mountpoints from mtab if enabled quotas
	Quota::setmntent();
	my $i=0;
	my @args;
	while (my @temp = Quota::getmntent()) {
		$args[$i][0] = $temp[0];
		$args[$i][1] = $temp[1];
		$args[$i][2] = $temp[2];
		$args[$i][3] = $temp[3];
		$i++;
		}
	Quota::endmntent();
	my $j=0; my $k=0; $i=0;
	while ($args[$i][0]) {
		if ( $args[$i][3] =~ m/usrquota/ ) {
			$quota_usr[$j][0] = $args[$i][0];
			$quota_usr[$j][1] = $args[$i][1];
			$quota_usr[$j][2] = $args[$i][2];
			$quota_usr[$j][3] = $args[$i][3];
			$j++;
			}
		if ( $args[$i][3] =~ m/grpquota/ ) {
			$quota_grp[$k][0] = $args[$i][0];
			$quota_grp[$k][1] = $args[$i][1];
			$quota_grp[$k][2] = $args[$i][2];
			$quota_grp[$k][3] = $args[$i][3];
			$k++;
			}
		$i++;
		}
	}

# ***************** Check values



if ($( == 0 ) {
	# Drop root Previleges
	($<, $>) = ($>, $<);
	switch: {
		# Get user information
		if (($vals[5] eq 'u') || ($vals[3] eq 'home')) { @user = getpwnam($vals[2]); }
			else { @user = getgrnam($vals[2]); }
		$vals[3] eq 'home' && do {
			switch2: {
				$vals[4] eq 'add' && do {
					# split homedir to set all directories below the last dir. to 755
					my $path = $user[7];
					$path =~ s,/(?:[^/]*)$,,;
					($<, $>) = ($>, $<); # Get root privileges
					if (! -e $path) {
						    system 'mkdir', '-m 755', '-p', $path; # Create paths to homedir
					    }
					if (! -e $user[7]) {
					    system 'mkdir', '-m 755', $user[7]; # Create himdir itself
					    system "cp -a /etc/skel/* /etc/skel/.[^.]* $user[7]"; # Copy /etc/sekl into homedir
				    	    system 'chown', '-R', "$user[2]:$user[3]" , $user[7]; # Change owner to new user
					    if (-e '/usr/sbin/useradd.local') {
						    system '/usr/sbin/useradd.local', $user[0]; # run useradd-script
						    }
					    }
					($<, $>) = ($>, $<); # Give up root previleges
					last switch2;
					};
				$vals[4] eq 'rem' && do {
					($<, $>) = ($>, $<); # Get root previliges
					if (-d $user[7]) {
					    system 'rm', '-R', $user[7]; # Delete Homedirectory
					    if (-e '/usr/sbin/userdel.local') {
						    system '/usr/sbin/userdel.local', $user[0];
						    }
					    }
					($<, $>) = ($>, $<); # Give up root previleges
					last switch2;
					};
				}
			last switch;
			};
		$vals[3] eq 'quota' && do {
			get_fs(); # Load list of devices with enabled quotas
			# Store quota information in array
			@quota_temp1 = split (':', $vals[6]);
			$group=0;
			$i=0;
			while ($quota_temp1[$i]) {
				$j=0;
				@temp = split (',', $quota_temp1[$i]);
				while ($temp[$j]) {
					$quota[$i][$j] = $temp[$j];
					$j++;
					}
				$i++;
				}
			if ($vals[5] eq 'u') { $group=false; } else {
				$group=1;
				@quota_usr = @quota_grp;
				}
			switch2: {
				$vals[4] eq 'rem' && do {
					$i=0;
					($<, $>) = ($>, $<); # Get root privileges
					while ($quota_usr[$i][0]) {
						$dev = Quota::getqcarg($quota_usr[$i][1]);
						$return = Quota::setqlim($dev,$user[2],0,0,0,0,1,$group);
						$i++;
						}
					($<, $>) = ($>, $<); # Give up root previleges
					last switch2;
					};
				$vals[4] eq 'set' && do {
					$i=0;
					($<, $>) = ($>, $<); # Get root privileges
					while ($quota_usr[$i][0]) {
						$dev = Quota::getqcarg($quota[$i][0]);
						$return = Quota::setqlim($dev,$user[2],$quota[$i][1],$quota[$i][2],$quota[$i][3],$quota[$i][4],1,$group);
						$i++;
						}
					($<, $>) = ($>, $<); # Give up root previleges
					last switch2;
					};
				$vals[4] eq 'get' && do {
					$i=0;
					($<, $>) = ($>, $<); # Get root privileges
					while ($quota_usr[$i][0]) {
						if ($vals[2]ne'+') {
							$dev = Quota::getqcarg($quota_usr[$i][1]);
							@temp = Quota::query($dev,$user[2],$group);
							if ($temp[0]ne'') {
								    $return = "$quota_usr[$i][1],$temp[0],$temp[1],$temp[2],$temp[3],$temp[4],$temp[5],$temp[6],$temp[7]:$return";
							    }
							else { $return = "$quota_usr[$i][1],0,0,0,0,0,0,0,0:$return"; }
							}
						else { $return = "$quota_usr[$i][1],0,0,0,0,0,0,0,0:$return"; }
						$i++;
						}
					($<, $>) = ($>, $<); # Give up root previleges
					last switch2;
					};
				}
			last switch;
			};
		last switch;
		};
	print "$return\n";
	}
else {
	$hostname = shift @ARGV;
	$remotepath = shift @ARGV;
	use Net::SSH::Perl;
	@username = split (',', $ARGV[0]);
	$username[0] =~ s/uid=//;
	my $ssh = Net::SSH::Perl->new($hostname, options=>[
		"IdentityFile /var/lib/wwwrun/.ssh/id_dsa",
		"UserKnownHostsFile /dev/null"
		]);
	$ssh->login($username[0], $ARGV[1]);
	($stdout, $stderr, $exit) = $ssh->cmd("sudo $remotepath @ARGV");
	print "$stdout";
	}
