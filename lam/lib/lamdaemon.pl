#! /usr/bin/perl

# $Id$
#
#  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
#  Copyright (C) 2003 - 2006  Tilo Lutz
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
	if ($ARGV[0] eq "*test") {
		use Quota; # Needed to get and set quotas
		print "Perl quota module successfully installed.\n";
		print "If you haven't seen any errors lamdaemon.pl was set up successfully.\n";
		}
	else {
		# loop for every transmitted user
		my $string = do {local $/;<STDIN>};
		@input = split ("\n", $string );
		for ($i=0; $i<=$#input; $i++) {
			$return = "";
			@vals = split (' ', $input[$i]);
			switch: {
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
								system 'mkdir', '-m', '0755', $user[7]; # Create homedir itself
								system ("(cd /etc/skel && tar cf - .) | (cd $user[7] && tar xmf -)"); # Copy /etc/sekl into homedir
								system 'chown', '-hR', "$user[2]:$user[3]" , $user[7]; # Change owner to new user
								if (-e '/usr/sbin/useradd.local') {
									system '/usr/sbin/useradd.local', $user[0]; # run useradd-script
									}
								}
							else {
								$return = "ERROR,Lamdaemon,Homedirectory already exists.:$return";
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
							    }
							    else {
								$return = "ERROR,Lamdaemon,Homedirectory not owned by $user[2].:$return";
							    }
								}
							else {
								$return = "ERROR,Lamdaemon,Homedirectory doesn't exists.:$return";
								}
							($<, $>) = ($>, $<); # Give up root previleges
							last switch2;
							};
						}
					# Show error if undfined command is used
					$return = "ERROR,Lamdaemon,Unknown command $vals[2].:$return";
					last switch;
					};
				$vals[1] eq 'quota' && do {
					use Quota; # Needed to get and set quotas
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
							while ($quota_usr[$i][0]) {
								$dev = Quota::getqcarg($quota[$i][0]);
								$return = Quota::setqlim($dev,$user[2],$quota[$i][1],$quota[$i][2],$quota[$i][3],$quota[$i][4],1,$group);
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
						$return = "ERROR,Lamdaemon,Unknown command $vals[2].:$return";
						}
					};
					last switch;
				$return = "ERROR,Lamdaemon,Unknown command $vals[1].:$return";
				};
				print "$return\n";
			}
		}
	}
else {
	$hostname = shift @ARGV;
	$remotepath = shift @ARGV;
	use Net::SSH::Perl;
	if ($ARGV[2] eq "*test") { print "Net::SSH::Perl successfully installed.\n"; }
	if (($ARGV[0] eq "-") and ($ARGV[1] eq "-")) {  # user+passwd are in STDIN
		$username = <STDIN>;
		chop($username);
		@username = split (',', $username);
		$username[0] =~ s/uid=//;
		$username[0] =~ s/cn=//;
		$username = $username[0];
		$password = <STDIN>;
		chop($password);
	}
	else {
		@username = split (',', $ARGV[0]);
		$username[0] =~ s/uid=//;
		$username[0] =~ s/cn=//;
		$username = $username[0];
		$password = $ARGV[1];
	}
	# Put all transfered lines in one string
	if ($ARGV[2] ne "*test") {
		$string = do {local $/;<STDIN>};
		}
	else {
		$argv = "*test\n";
		$string = " \n";
	}
	my $ssh = Net::SSH::Perl->new($hostname, options=>[
		"UserKnownHostsFile /dev/null"],
		protocol => "2,1", debug => 0 );
	$ssh->login($username, $password);
	# Change needed to prevent buffer overrun
	@string2 = split ("\n", $string);
	for ($i=0; $i<=$#string2; $i++) {
		($stdout2, $stderr, $exit) = $ssh->cmd("sudo $remotepath $argv", $string2[$i]);
		$stdout .= $stdout2;
		}
	#($stdout, $stderr, $exit) = $ssh->cmd("sudo $remotepath $argv", $string);
	print $stdout;
	}
