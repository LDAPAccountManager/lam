#! /usr/bin/perl

#  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
#  Copyright (C) 2003 - 2006  Tilo Lutz
#  Copyright (C) 2006 - 2023  Roland Gruber
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

use Sys::Syslog;

# Defines the protocol version of the lamdaemon script.
# This will only be changed when additional commands are added etc.
my $LAMDAEMON_PROTOCOL_VERSION = 5;

my $SPLIT_DELIMITER = "###x##y##x###";

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
@quota_usr; # Filesystems with enabled user quotas
	# vals = DN, PAssword, user, home, (add|rem),
	#                            quota, (set|get),(u|g), (mountpoint,blocksoft,blockhard,filesoft,filehard)+
	#                            chown  options
$|=1; # Disable buffering

sub get_fs { # Load mount points from mtab if enabled quotas
	Quota::setmntent();
	my $i=0;
	my @args;
	my %deviceList;
	while (my @temp = Quota::getmntent()) {
		if (exists($deviceList{$temp[0]})) {
			# skip duplicate devices
			next;
		}
		$args[$i][0] = $temp[0];
		$args[$i][1] = $temp[1];
		$args[$i][2] = $temp[2];
		$args[$i][3] = $temp[3];
		$deviceList{$temp[0]} = 1;
		$i++;
	}
	Quota::endmntent();
	my $j=0; my $k=0; $i=0;
	while ($args[$i][0]) {
		if ( $args[$i][3] =~ m/usr[j]?quota/ ) {
			$quota_usr[$j][0] = $args[$i][0];
			$quota_usr[$j][1] = $args[$i][1];
			$quota_usr[$j][2] = $args[$i][2];
			$quota_usr[$j][3] = $args[$i][3];
			$j++;
			}
		if ( $args[$i][3] =~ m/grp[j]?quota/ ) {
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

# check if script runs as root
if ($< != 0 ) {
	print "ERROR,Lamdaemon ($hostname),Not called as root (user id " . $< . ").\n";
	logMessage(LOG_ERR, "Not called as root (user id " . $< . ").");
	exit 1;
}

# Drop root privileges
($<, $>) = ($>, $<);
my $input = $ARGV[0];
$return = "";
@vals = split ($SPLIT_DELIMITER, $input);
# Get user information
if (($vals[3] eq 'user') || ($vals[1] eq 'home')) { @user = getpwnam($vals[0]); }
else { @user = getgrnam($vals[0]); }
if ($vals[1] eq '') {
	# empty line, nothing to do
}
elsif (($vals[1] eq 'test')) {
	# run tests
	runTest();
}
elsif ($vals[1] eq 'home') {
	manageHomedirs();
}
elsif ($vals[1] eq 'directory') {
	manageDirectories();
}
elsif ($vals[1] eq 'quota') {
	manageQuotas();
}
else {
	$return = "ERROR,Lamdaemon ($hostname),Unknown command $vals[1].";
	logMessage(LOG_ERR, "Unknown command $vals[1].");
}
print "$return\n";

#
# Runs tests to check the environment
#
sub runTest {
	# protocol version check
	if ($vals[2] eq 'version') {
		if ($vals[3] eq $LAMDAEMON_PROTOCOL_VERSION) {
			$return = "INFO,Version check ok";
		}
		else {
			$return = "ERROR,Version check failed. Please upgrade the lamdaemon script to the same version as your main LAM installation.";
		}
	}
	# basic test
	elsif ($vals[2] eq 'basic') {
		$return = "INFO,Basic test ok";
	}
	# quota test
	elsif ($vals[2] eq 'quota') {
		require Quota;
		$return = "INFO,Quota test ok";
	}
	# NSS LDAP
	elsif ($vals[2] eq 'nss') {
		$userName = $vals[3];
		# check if the user exists in /etc/passwd
		system("grep", "-q", "^" . $userName . ":", "/etc/passwd");
		if ( $? == 0 ) {
			$error = "User $userName is a local user (/etc/passwd) but should be LDAP only.";
			$return = "ERROR,$error";
			logMessage(LOG_ERR, $error);
		}
		else {
			# check if home directory is readable
			@user = getpwnam($userName);
			if ($user[7] eq '') {
				$return = "ERROR,Unable to determine home directory of user $userName. Please check that NSS LDAP is correctly configured.";
				logMessage(LOG_ERR, "Unable to determine home directory of user $userName. Please check that NSS LDAP is correctly configured.");
			}
			else {
				$return = "INFO,NSS test ok";
			}
		}
	}
	else {
		$return = "ERROR,Unknown test: $vals[2]";
	}
}

#
# Handles all homedir related commands
#
sub manageHomedirs {
	if ($vals[2] eq 'add') {
		createHomedir();
	}
	elsif ($vals[2] eq 'rem') {
		removeHomedir();
	}
	elsif ($vals[2] eq 'chgrp') {
		chgrpHomedir();
	}
	elsif ($vals[2] eq 'move') {
		moveHomedir();
	}
	elsif ($vals[2] eq 'check') {
		checkHomedir();
	}
	else {
		# Show error if undefined command is used
		$return = "ERROR,Lamdaemon ($hostname),Unknown home command $vals[2].";
		logMessage(LOG_ERR, "Unknown command $vals[2]");
	}
}

#
# Creates the home directory of the user
#
sub createHomedir {
	my $homedir = $vals[3];
	if ($homedir eq '') {
		$return = "ERROR,Lamdaemon ($hostname),No home directory specified.";
		logMessage(LOG_ERR, "No home directory specified to create.");
		return;
	}
	my $path = $homedir;
	# split homedir to set all directories below the last dir. to 0755
	$path =~ s,/(?:[^/]*)$,,;
	($<, $>) = ($>, $<); # Get root privileges
	if (! -e $path) {
		system 'mkdir', '-m', '0755', '-p', $path; # Create paths to homedir
	}
	if (! -e $homedir) {
		system 'mkdir', '-m', $vals[4], $homedir; # Create homedir itself
		system ("(cd /etc/skel && tar cf - .) | (cd $homedir && tar xmf -)"); # Copy /etc/skel into homedir
		system 'chown', '-hR', "$vals[5]:$vals[6]" , $homedir; # Change owner to new user
		if (-e '/usr/sbin/useradd.local') {
			system '/usr/sbin/useradd.local', $vals[0]; # run useradd-script
			system 'chmod', '-R', $vals[4], $homedir;     # Edit chmod rights
		}
		system 'chmod', $vals[4], $homedir;     # Edit chmod rights
		$return = "INFO,Lamdaemon ($hostname),Home directory created (" . $homedir . ").";
		logMessage(LOG_INFO, "Home directory created (" . $homedir . ")");
	}
	else {
		$return = "ERROR,Lamdaemon ($hostname),Home directory already exists (" . $homedir . ").";
		logMessage(LOG_ERR, "Home directory already exists (" . $homedir . ")");
	}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Removes the home directory of the user
#
sub removeHomedir {
	if ($vals[3] eq '') {
		$return = "ERROR,Lamdaemon ($hostname),No home directory specified to delete.";
		logMessage(LOG_ERR, "No home directory specified to delete.");
		return;
	}
	($<, $>) = ($>, $<); # Get root privileges
	if (-d $vals[3] && $vals[3] ne '/') {
		if ((stat($vals[3]))[4] eq $vals[4]) {
			system 'rm', '-Rf', $vals[3]; # delete home directory
			if (-e '/usr/sbin/userdel.local') {
				system '/usr/sbin/userdel.local', $vals[0];
			}
			$return = "Ok";
			logMessage(LOG_INFO, "Home directory removed (" . $vals[3] . ")");
		}
		else {
			$return = "ERROR,Lamdaemon ($hostname),Home directory not owned by $vals[4].";
			logMessage(LOG_ERR, "Home directory owned by wrong user (" . $vals[4] . ")");
		}
	}
	else {
		$return = "Ok";
		logMessage(LOG_INFO, "The directory " . $vals[3] . " which should be deleted was not found (skipped).");
	}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Moves the home directory of the user
#
sub moveHomedir {
	my $homedir = $vals[3];
	my $owner = $vals[4];
	my $homedirNew = $vals[5];
	if ($homedir eq '') {
		$return = "ERROR,Lamdaemon ($hostname),No home directory specified to move.";
		logMessage(LOG_ERR, "No home directory specified to move.");
		return;
	}
	if (-d $homedirNew) {
		$return = "ERROR,Lamdaemon ($hostname),Directory $homedirNew already exists.";
		logMessage(LOG_ERR, "Directory $homedirNew already exists.");
		return;
	}
	($<, $>) = ($>, $<); # Get root privileges
	if (-d $homedir && $homedir ne '/') {
		if ((stat($homedir))[4] eq $owner) {
			system 'mv', $homedir, $homedirNew; # move home directory
			$return = "Ok";
			logMessage(LOG_INFO, "Home directory moved ($homedir - $homedirNew)");
		}
		else {
			$return = "ERROR,Lamdaemon ($hostname),Home directory not owned by $owner.";
			logMessage(LOG_ERR, "Home directory owned by wrong user (" . $owner . ")");
		}
	}
	else {
		$return = "Ok";
		logMessage(LOG_INFO, "The directory " . $homedir . " which should be moved was not found (skipped).");
	}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Changes the group of the home directory of the user.
#
sub chgrpHomedir {
	my $homedir = $vals[3];
	my $owner = $vals[4];
	my $group = $vals[5];
	if ($homedir eq '') {
		$return = "ERROR,Lamdaemon ($hostname),No home directory specified to move.";
		logMessage(LOG_ERR, "No home directory specified to move.");
		return;
	}
	($<, $>) = ($>, $<); # Get root privileges
	if (-d $homedir && $homedir ne '/') {
		if ((stat($homedir))[4] eq $owner) {
			system 'chgrp', $group, $homedir; # change group
			$return = "Ok";
			logMessage(LOG_INFO, "Home directory changed to new group ($homedir - $group)");
		}
		else {
			$return = "ERROR,Lamdaemon ($hostname),Home directory not owned by $owner.";
			logMessage(LOG_ERR, "Home directory owned by wrong user (" . $owner . ")");
		}
	}
	else {
		$return = "Ok";
		logMessage(LOG_INFO, "The directory " . $homedir . " which should be changed was not found (skipped).");
	}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Checks if the home directory of the user already exists.
#
sub checkHomedir {
	my $homedir = $vals[3];
	if ($homedir eq '') {
		$return = "ERROR,Lamdaemon ($hostname),No home directory specified to check.";
		logMessage(LOG_ERR, "No home directory specified to check.");
		return;
	}
	if (-d $homedir) {
		$return = "ok";
	}
	else {
		$return = "missing";
	}
}

#
# Handles all directory related commands
#
sub manageDirectories {
	if ($vals[2] eq 'add') {
		createDirectory();
	}
	else {
		# Show error if undefined command is used
		$return = "ERROR,Lamdaemon ($hostname),Unknown home command $vals[2].";
		logMessage(LOG_ERR, "Unknown command $vals[2]");
	}
}

#
# Creates a directory of the user
#
sub createDirectory {
	my $homedir = $vals[3];
	if ($homedir eq '') {
		$return = "ERROR,Lamdaemon ($hostname),No directory specified.";
		logMessage(LOG_ERR, "No directory specified to create.");
		return;
	}
	my $path = $homedir;
	# split homedir to set all directories below the last dir. to 0755
	$path =~ s,/(?:[^/]*)$,,;
	($<, $>) = ($>, $<); # Get root privileges
	if (! -e $path) {
		system 'mkdir', '-m', '0755', '-p', $path; # Create paths to homedir
	}
	if (! -e $homedir) {
		system 'mkdir', '-m', $vals[4], $homedir; # Create homedir itself
		system 'chown', '-hR', "$vals[5]:$vals[6]" , $homedir; # Change owner to new user
		system 'chmod', $vals[4], $homedir;     # Edit chmod rights
		$return = "INFO,Lamdaemon ($hostname),Directory created (" . $homedir . ").";
		logMessage(LOG_INFO, "Directory created (" . $homedir . ")");
	}
	else {
		$return = "ERROR,Lamdaemon ($hostname),Directory already exists (" . $homedir . ").";
		logMessage(LOG_ERR, "Directory already exists (" . $homedir . ")");
	}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Handles all quota related commands
#
sub manageQuotas {
	require Quota; # Needed to get and set quotas
	get_fs(); # Load list of devices with enabled quotas
	# Store quota information in array
	@quota_temp1 = split (':', $vals[4]);
	$group=0;
	$i=0;
	foreach my $quota_element (@quota_temp1) {
		if (length ($quota_element) >= 2) {
			@temp = split (',', $quota_element);
			$j=0;
			foreach my $temp_element (@temp) {
				$quota[$i][$j] = $temp_element;
				$j++;
			}
			$i++;
		}
	}
	if ($vals[3] eq 'user') { $group=false; }
	else {
		$group=1;
		@quota_usr = @quota_grp;
	}
	if ($vals[2] eq 'rem') {
		remQuotas();
	}
	elsif ($vals[2] eq 'set') {
		setQuotas();
	}
	elsif ($vals[2] eq 'get') {
		getQuotas();
	}
	else {
		$return = "ERROR,Lamdaemon ($hostname),Unknown quota command $vals[2].";
		logMessage(LOG_ERR, "Unknown command $vals[2].");
	}
}

#
# Removes the quotas of a user or group
#
sub remQuotas {
	$i=0;
	($<, $>) = ($>, $<); # Get root privileges
	while ($quota_usr[$i][0]) {
		$dev = Quota::getqcarg($quota_usr[$i][1]);
		$return = Quota::setqlim($dev,$user[2],0,0,0,0,1,$group);
		$i++;
		}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Sets the quota values
#
sub setQuotas {
	$i=0;
	($<, $>) = ($>, $<); # Get root privileges
	while ($quota[$i][0]) {
		$dev = Quota::getqcarg($quota[$i][0]);
		last if ($dev eq '');
		$return = Quota::setqlim($dev,$user[2],$quota[$i][1],$quota[$i][2],$quota[$i][3],$quota[$i][4],1,$group);
		if ($return == -1) {
				$return = "ERROR,Lamdaemon ($hostname),Unable to set quota!";
				logMessage(LOG_ERR, "Unable to set quota for $user[0] on " . $quota[$i][0] . ".");
		}
		else {
			logMessage(LOG_INFO, "Set quota for $user[0].");
		}
		$i++;
		}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Reads the quota values
#
sub getQuotas {
	$i=0;
	($<, $>) = ($>, $<); # Get root privileges
	while ($quota_usr[$i][0]) {
		if ($vals[0]ne'+') {
			$dev = Quota::getqcarg($quota_usr[$i][1]);
			@temp = Quota::query($dev,$user[2],$group);
			if ($temp[0]ne'') {
				if ($temp == -1) {
						$return = "ERROR,Lamdaemon ($hostname),Unable to read quota!";
						logMessage(LOG_ERR, "Unable to read quota for $user[0].");
					}
				else {
					$return = "QUOTA_ENTRY $quota_usr[$i][1],$temp[0],$temp[1],$temp[2],$temp[3],$temp[4],$temp[5],$temp[6],$temp[7]:$return";
					}
				}
			else { $return = "QUOTA_ENTRY $quota_usr[$i][1],0,0,0,0,0,0,0,0:$return"; }
			}
		else { $return = "QUOTA_ENTRY $quota_usr[$i][1],0,0,0,0,0,0,0,0:$return"; }
		$i++;
		}
	($<, $>) = ($>, $<); # Give up root privileges
}

#
# Logs a message to the syslog.
#
# Parameters: level message
#
#             level: error level
#             message: message text
#
sub logMessage {
	my $level = $_[0];
	my $message = $_[1];
	openlog('LAM - lamdaemon','','user');
	syslog($level, $message);
	closelog;
}

