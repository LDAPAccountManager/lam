#! /usr/bin/perl -T

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

# At the moment communication is using fifos. Later a network socket will be used.

use Cwd; # Needed to get the current path.
use Quota; # Needed to get and set quotas
use Net::LDAP; # Needed to connect to ldap-server
use File::NCopy qw(copy); # Needed to copy recursive
use File::Path; # Nedded to delete recursive and create directories recursive

$path = cwd; # Path of $lam/lib
$server; # URL of ldap-server
$usersuffix; # ldap-suffix of users
@admins; # list of valid admins
@quota_usr; # Filesystems with enabled userquotas
@quota_grp; # Filesystems with enabled userquotas
$debug=true; # Show debug messages
$|=1;

# Check if Fifo was created.
if (!-e 'lamdaemon.fifo') {
	system ('mkfifo', 'lamdaemon.fifo');
	system ('chmod', '777', 'lamdaemon.fifo');
	}

sub loadcfg {
	# Get ldap-server from $lam/config/lam.conf
	# Get valid admins from $lam/config/lam.conf
	open ( CONFIG, "< $path/../config/lam.conf" ) or die ('Can\'t open lam.conf.');
	while (<CONFIG>) {
		my @line = split (':', $_);
		$line[0] =~ s/ //g;
		$line[1] =~ s/ //g;
		if ($line[0] eq 'serverURL') { $server=$line[1]; }
		if ($line[0] eq 'usersuffix') { $usersuffix=$line[1]; }
		if ($line[0] eq 'admins') { @admins = split (';', $line[1]); }
		}
	$time_lam = -M "$path/../config/lam.conf";
	}

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
		elsif ( $args[$i][3] =~ m/grpquota/ ) {
			$quota_grp[$k][0] = $args[$i][0];
			$quota_grp[$k][1] = $args[$i][1];
			$quota_grp[$k][2] = $args[$i][2];
			$quota_grp[$k][3] = $args[$i][3];
			$k++;
			}
		$i++;
		}
	$time_mtab = -M "/etc/mtab";
	}

$host="127.0.0.1";

while (defined (<STDIN>))
	{
	# Reset variables
	$found=false;
	$return='';
	open ( FIFO, '< lamdaemon.fifo' ) or die ('Can\'t open fifo lamdaemon.fifo.'); 	# Open Fifo
	@vals = split (' ', <FIFO>); # read values from fifo
	# vals = DN, PAssword, user, home, (add|rem),
	#                            quota, (set|get),(u|g), (mountpoint:blocksoft:blockhard:filesoft:filehard:timelimit)+
	#                            chown  options
	close FIFO; # Close fifo.
	if ($debug==true) { print "@vals\n"; }
	if ( $time_lam != -M "$path/../config/lam.conf" ) { loadcfg(); } #load config at start and if configfile has changed
	if ( $time_mtab != -M "/etc/mtab" ) { get_fs(); } #load config at start and if configfile has changed

	# Check if DN is listed as admin
	foreach my $admin (@admins) {
		if ($admin eq $vals[0]) { $found=true; }
		}
	if ($found==true) {
		# Connect to ldap-server and check if password is valid.
		$ldap = Net::LDAP->new($host) or die ('Can\'t connect to ldapserver.');
		$result = $ldap->bind (dn => $vals[0], password => $vals[1]) ;
		if (!$result->code) { # password is valid
			switch: {
				# Get user information
				my $isrealadmin=false;
				if (($vals[5] eq 'u') || ($vals[3] eq 'home')) {
					@user = getpwnam($vals[2]);
					my $result = $ldap->search ( base=>$userbase, filter=>"uid=$vals[2]", attrs=>['userPassword', 'uidNumber'] );
					my $href = $result->as_struct;
					my @arrayOfDNs = keys %$href; # use DN hashes
					my $valref = $$href{$arrayOfDNs[0]};
					my @arrayOfAttrs = sort keys %$valref; #use Attr hashes
					if (@$valref{$arrayOfAttrs['uidNumber']}!=$user[2]) { # We've found the wrong user with the right usernmae but wron uidnumber
						$isrealadmin=false;
						if ($debug==true) { print "Found user $user[0] but uidNumber from another user. Please check your settings!!!\n"; }
						}
					else {
						my $userPassword = @$valref{$arrayOfAttrs['userPassword']} ; # Read userPassword.
						my $msg = $ldap->modify (dn => $arrayOfDNs[0], add=>{ 'userPassword'=>$userPassword });
						if (!$result->code) { $isrealadmin=true; }
						}
					}
				else {
					# Check if admin is really an admin
					# If he can modify the password from the user he's one
					@user = getgrnam($vals[2]);
					my $result = $ldap->search ( base=>$userbase, filter=>"gid=$vals[2]", attrs=>['userPassword', 'gidNumber'] );
					my $href = $result->as_struct;
					my @arrayOfDNs = keys %$href; # use DN hashes
					my $valref = $$href{$arrayOfDNs[0]};
					my @arrayOfAttrs = sort keys %$valref; #use Attr hashes
					if (@$valref{$arrayOfAttrs['gidNumber']}!=$user[2]) { # We've found the wrong user with the right usernmae but wron uidnumber
						$isrealadmin=false;
						if ($debug==true) { print "Found user $user[0] but uidNumber from another user. Please check your settings!!!\n"; }
						}
					else {
						my $userPassword = @$valref{$arrayOfAttrs['userPassword']} ; # Read userPassword.
						if (!$userPassword) {
							$roremove=true;
							$userPassword = "*"; # Set invalid Password if Password is not set, e.g. groups
							}
						my $msg = $ldap->modify (dn => $arrayOfDNs[0], replace=>{ 'userPassword'=>$userPassword });
						if (!$result->code) {
							$isrealadmin=true;
							if ($toremove==true) {
								$ldap->modify (dn => $arrayOfDNs[0], delete=>'userPassword');
								}
							}
						}
					}
				if ($isrealadmin==true) {
					$vals[3] eq 'home' && do {
						switch2: {
							$vals[4] eq 'add' && do {
								# split homedir to set all directories below the last dir. to 755
								my $path = $user[7];
								$path =~ s,/(?:[^/]*)$,,;
								eval { mkpath ($patch, 0, '755') };
								if ($@) { $return = 0; }
								if ( $return != 0 ) {
									eval { mkpath ($user[7], 0, '700') };# Create Homedirectory
									if ($@) { $return = 0; }
									}
								if ($return != 0 ) {
									$return =copy \1, '/etc/skel/', $user[7]; # Copy /etc/sekl into homedir
									}
								if ($return > 0) {
									system 'chown', '-R', $user[2], $user[3] , $user[7]; # Change owner to new user
									}
								system '/usr/sbin/useradd.local', $user[0]; # run useradd-script
								last switch2;
								};
							$vals[4] eq 'rem' && do {
								eval { rmtree ($user[7], 0, 0) }; # Delete Homedirectory
								if ($@) { $return = 0; }
								if ($return!=0) { system '/usr/sbin/userdel.local', $user[0]; }
								last switch2;
								};
							}
						last switch;
						};
					$vals[3] eq 'quota' && do {
						# Store quota information in array
						@quota_temp1 = split (';', $vals[6]);
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
						if ($vals[5] eq 'u') { $group=false; }
							else { $group=true; }
							switch2: {
								$vals[4] eq 'set' && do {
									$i=0;
									while ($quota_usr[$i][0]) {
										$dev = Quota::getqcarg($quota[$i][0]);
										$return = Quota::setqlim($dev,$user[2],$quota[$i][1],$quota[$i][2],$quota[$i][3],$quota[$i][4],1,$group);
										$i++;
										}
									last switch2;
									};
								$vals[4] eq 'get' && do {
									$i=0;
									while ($quota_usr[$i][0]) {
										if ($vals[2]!='*') {
											@temp = Quota::query($quota_usr[$i][0],$user[2],$group);
											$return = "$quota_usr[$i][1],$temp[0],$temp[1],$temp[2],$temp[3],$temp[4],$temp[5],$temp[6],$temp[7];$return";
											}
										 else { $return = "$quota_usr[$i][1],0,0,0,0,0,0,0,0;$return"; }
										$i++;
										}
									last switch2;
									};
							}
						last switch;
						};
					}
				else {
					$return = "Got you, stupid hacker.\n";
					}
				}
			}
		else { $return = "Invalid Password"; }
		$ldap->unbind(); # Clode ldap connection.
		}
	else { $return = "Invalid User"; }
	open ( FIFO, '> lamdaemon.fifo' ) or die ('Can\'t open fifo lamdaemon.fifo.'); 	# Open Fifo
	print FIFO $return;
	close FIFO; # Close fifo.
	print "$return\n";
	}
