#! /usr/bin/perl

# $Id$
#
#  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
#  Copyright (C) 2003 - 2006  Tilo Lutz
#  Copyright (C) 2006 - 2007  Roland Gruber
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

# set a known path
my $path = "";
if (-d "/sbin") {
	if ($path eq "") { $path = "/sbin"; }
	else { $path = "$path:/sbin"; }
}
if (-d "/usr/sbin") {
	if ($path eq "") { $path = "/usr/sbin"; }
	else { $path = "$path:/usr/sbin"; }
}
if (-l "/bin") {
	if ($path eq "") { $path = "/usr/bin"; }
	else { $path = "$path:/usr/bin"; }
}
else {
	if ($path eq "") { $path = "/bin:/usr/bin"; }
	else { $path = "$path:/bin:/usr/bin"; }
}
if (-d "/opt/sbin") { $path = "$path:/opt/sbin"; }
if (-d "/opt/bin") { $path = "$path:/opt/bin"; }
$ENV{"PATH"} = $path;

# get hostname
$hostname = `hostname`;
chop($hostname);

#use strict; # Use strict for security reasons

@quota_grp;
@quota_usr; # Filesystems with enabled userquotas
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
if ($< == 0 ) { # we are root
	# Drop root Previleges
	($<, $>) = ($>, $<);
	# loop for every transmitted user
	while (1) {
		my $input = <STDIN>;
		chop($input);
		$return = "";
		@vals = split (' ', $input);
		switch: {
			# test if lamdaemon can be run
			if (($vals[1] eq 'test')) {
				# basic test
				if ($vals[2] eq 'basic') {
					$return = "Ok";
				}
				# quota test
				elsif ($vals[2] eq 'quota') {
					require Quota;
					$return = "Ok";
				}
				last switch;
			}
			# Get user information
			if (($vals[3] eq 'user') || ($vals[1] eq 'home')) { @user = getpwnam($vals[0]); }
			else { @user = getgrnam($vals[0]); }
			$vals[1] eq 'home' && do {
				switch2: {
					$vals[2] eq 'add' && do {
						# split homedir to set all directories below the last dir. to 0755
						my $path = $user[7];
						$path =~ s,/(?:[^/]*)$,,;
						($<, $>) = ($>, $<); # Get root privileges
						if (! -e $path) {
							system 'mkdir', '-m', '0755', '-p', $path; # Create paths to homedir
						}
						if (! -e $user[7]) {
							system 'mkdir', '-m', $vals[3], $user[7]; # Create homedir itself
							system ("(cd /etc/skel && tar cf - .) | (cd $user[7] && tar xmf -)"); # Copy /etc/sekl into homedir
							system 'chown', '-hR', "$user[2]:$user[3]" , $user[7]; # Change owner to new user
							if (-e '/usr/sbin/useradd.local') {
								system '/usr/sbin/useradd.local', $user[0]; # run useradd-script
								system 'chmod', '-R', $vals[3], $user[7];     # Edit chmod rights
							}
							system 'chmod', $vals[3], $user[7];     # Edit chmod rights
							$return = "INFO,Lamdaemon ($hostname),Home directory created (" . $user[7] . ").";
						}
						else {
							$return = "ERROR,Lamdaemon ($hostname),Home directory already exists (" . $user[7] . ").";
						}
						($<, $>) = ($>, $<); # Give up root previleges
						last switch2;
					};
					$vals[2] eq 'rem' && do {
						($<, $>) = ($>, $<); # Get root previliges
						if (-d $user[7] && $user[7] ne '/') {
							if ((stat($user[7]))[4] eq $user[2]) {
								system 'rm', '-R', $user[7]; # Delete Homedirectory
								if (-e '/usr/sbin/userdel.local') {
									system '/usr/sbin/userdel.local', $user[0];
								}
								$return = "Ok";
							}
							else {
								$return = "ERROR,Lamdaemon ($hostname),Home directory not owned by $user[2].";
							}
						}
						else {
							$return = "INFO,Lamdaemon ($hostname),The directory which should be deleted was not found (skipped).";
							}
						($<, $>) = ($>, $<); # Give up root previleges
						last switch2;
						};
						# Show error if undfined command is used
						$return = "ERROR,Lamdaemon ($hostname),Unknown command $vals[2].";
					}
				last switch;
				};
			$vals[1] eq 'quota' && do {
				require Quota; # Needed to get and set quotas
				get_fs(); # Load list of devices with enabled quotas
				# Store quota information in array
				@quota_temp1 = split (':', $vals[4]);
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
				if ($vals[3] eq 'user') { $group=false; }
				else {
					$group=1;
					@quota_usr = @quota_grp;
					}
				switch2: {
					$vals[2] eq 'rem' && do {
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
					$vals[2] eq 'set' && do {
						$i=0;
						($<, $>) = ($>, $<); # Get root privileges
						while ($quota[$i][0]) {
							$dev = Quota::getqcarg($quota[$i][0]);
							$return = Quota::setqlim($dev,$user[2],$quota[$i][1],$quota[$i][2],$quota[$i][3],$quota[$i][4],1,$group);
							if ($return == -1) {
									$return = "ERROR,Lamdaemon ($hostname),Unable to set quota!";
							}
							$i++;
							}
						($<, $>) = ($>, $<); # Give up root previleges
						last switch2;
						};
					$vals[2] eq 'get' && do {
						$i=0;
						($<, $>) = ($>, $<); # Get root privileges
						while ($quota_usr[$i][0]) {
							if ($vals[0]ne'+') {
								$dev = Quota::getqcarg($quota_usr[$i][1]);
								@temp = Quota::query($dev,$user[2],$group);
								if ($temp[0]ne'') {
									if ($temp == -1) {
											$return = "ERROR,Lamdaemon ($hostname),Unable to read quota!";
										}
									else {
										$return = "$quota_usr[$i][1],$temp[0],$temp[1],$temp[2],$temp[3],$temp[4],$temp[5],$temp[6],$temp[7]:$return";
										}
									}
								else { $return = "$quota_usr[$i][1],0,0,0,0,0,0,0,0:$return"; }
								}
							else { $return = "$quota_usr[$i][1],0,0,0,0,0,0,0,0:$return"; }
							$i++;
							}
						($<, $>) = ($>, $<); # Give up root previleges
						last switch2;
						};
					$return = "ERROR,Lamdaemon ($hostname),Unknown command $vals[2].";
					}
				};
				last switch;
			$return = "ERROR,Lamdaemon ($hostname),Unknown command $vals[1].";
			};
			print "$return\n";
		}
	}
else {
	print "ERROR,Lamdaemon ($hostname),Not called as root!\n";
}
