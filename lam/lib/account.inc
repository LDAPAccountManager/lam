<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


  LDAP Account Manager functions used by account.php
*/

class account { // This class keeps all needed values for any account
	// General Settings
	var $general_username;
	var $general_uidNumber;
	var $general_surname;
	var $general_givenname;
	var $general_dn;
	var $general_group;
	var $general_groupadd;
	var $general_homedir;
	var $general_shell;
	var $general_gecos;
	var $general_memberUid;
	// Unix Password Settings
	var $unix_password;
	var $unix_password_no;
	var $unix_pwdwarn;
	var $unix_pwdallowlogin;
	var $unix_pwdmaxage;
	var $unix_pwdminage;
	var $unix_pwdexpire_day;
	var $unix_pwdexpire_mon;
	var $unix_pwdexpire_yea;
	var $unix_deactivated;
	var $unix_shadowLastChange;
	// Samba Account
	var $smb_password;
	var $smb_password_no;
	var $smb_useunixpwd;
	var $smb_pwdcanchange;
	var $smb_pwdmustchange;
	var $smb_homedrive;
	var $smb_scriptPath;
	var $smb_profilePath;
	var $smb_smbuserworkstations;
	var $smb_smbhome;
	var $smb_domain;
	var $smb_flagsW;
	var $smb_flagsD;
	var $smb_flagsX;
	// Quota Settins
	var $quota;
	// Personal Settings
	var $personal_title;
	var $personal_mail;
	var $personal_telephoneNumber;
	var $personal_mobileTelephoneNumber;
	var $personal_facsimileTelephoneNumber;
	var $personal_street;
	var $personal_postalCode;
	var $personal_postalAddress;
	var $personal_employeeType;
	}

function initvars($type=false,$DN=false) { // This function registers all needes session-varibales needed by account.php
	// if session was started previos, the existing session will be continued
	session_save_path('../sess');
	@session_start();
	if ($type) {
		if (session_is_registered("type2")) session_unregister("type2");
		 else session_register("type2"); // $type2 stores the kind of account (User|Group|Host)
		$_SESSION['type2'] = $type;
		if (session_is_registered("shelllist")) session_unregister("shelllist");
		 else session_register("shelllist"); // $shelllist contains all shells defined in /etc/shells
		$_SESSION['shelllist'] = getshells(); // Write List of all valid shells in variable
		if (session_is_registered("account")) session_unregister("account");
		 else session_register("account"); // The new Accout properties are stored here
		if ($DN) {
			if (session_is_registered("account_old")) session_unregister("account_old");
			else session_register("account_old"); // Only valid if an account should be modified. It'll contains the existing account properties
			$DN = str_replace("\'", '',$DN);
				switch ($type) {
					case 'user':
						$_SESSION['account'] = loaduser($DN);
						$_SESSION['account_old'] = $_SESSION['account'];
						$_SESSION['account']->unix_password='';
						$_SESSION['account']->smb_password='';
						break;
					case 'group':
						$_SESSION['account'] = loadgroup($DN);
						$_SESSION['account_old'] = $_SESSION['account'];
						if (!session_is_registered('final_changegids')) session_register('final_changegids');
							else $_SESSION['final_changegids'] = '';
						break;
					case 'host':
						$_SESSION['account'] = loadhost($DN);
						$_SESSION['account_old'] = $_SESSION['account'];
						$_SESSION['account']->unix_password='';
						$_SESSION['account']->smb_password='';
						break;
					}
			}
		 else {
			if (session_is_registered("account_old")) session_unregister("account_old");
			switch ($type) {
				case 'user':
						$_SESSION['account'] = loadUserProfile('default');
					break;
				case 'group':
						$_SESSION['account'] = loadGroupProfile('default');
					break;
				case 'host':
						$_SESSION['account'] = loadHostProfile('default');
					break;
				}
			if ( (($type=='user')||($type=='group')) && ($_SESSION['config']->scriptServer)) {
				$values = getquotas($type);
				if (is_object($values)) {
					while (list($key, $val) = each($values)) // Set only defined values
						if ($val) $_SESSION['account']->$key = $val;
					}
				}
			}
		}
	}

function getshells() { // Return a list of all shells listed in /etc/shells
	$shells =  file('../config/shells');
	$i=0;
	while ($shells[$i]) {
		chop($shells[$i]);
		trim($shells[$i]);
		$shells[$i] = substr($shells[$i], 0, strpos($shells[$i], '#'));
		if ($shells[$i]=='') unset ($shells[$i]);
		 else $i++;
		}
	return $shells;
	}

function checkglobal($values, $type, $values_old=false) { // This functions checks all global account parameters $values is class account(), $type=user|host|group
	// If all values are OK an array of class account is returned. Else an error-string is returned
	$return = new account();
	switch ($type) {
		case 'user' :
			// Check if Homedir is valid
			$return->general_homedir = str_replace('$group', $values->general_group, $values->general_homedir);
			if ($values->general_username != '')
				$return->general_homedir = str_replace('$user', $values->general_username, $values->general_homedir);
			if ( !ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $return->general_homedir ))
				return _('Homedirectory contents invalid characters.');
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z])+$', $values->general_givenname)) return _('Givenname contents invalid characters');
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z])+$', $values->general_surname)) return _('Surname contents invalid characters');
			if ( ($values->general_gecos=='') || ($values->general_gecos==' '))
				$return->general_gecos = $values->general_givenname . " " . $values->general_surname ;
			// Check if Username contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $values->general_username))
				return _('Username contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if user already exists
			break;
		case 'group' :
			// Check if Groupname contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $values->general_username))
				return _('Groupname contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if group already exists
			if ($values->general_gecos=='') $return->general_gecos = $values->general_username ;
			break;
		case 'host' :
			if ( substr($values->general_username, strlen($values->general_username)-1, strlen($values->general_username)) != '$' )
				$return->general_username = $values->general_username . '$';
			// Check if Hostname contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[$])*$', $values->general_username))
				return _('Hostname contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if Hostname already exists
			$return->general_homedir = '/dev/null';
			$return->general_shell = '/bin/false';
			if ($values->general_gecos=='') $return->general_gecos = $values->general_username;
			break;
		}
	if ($temp = ldapexists($values, $type, $values_old)) return $temp;
	// Check if UID is valid. If none value was entered, the next useable value will be inserted
	$return->general_uidNumber = checkid($values, $type, $values_old);
	if (is_string($return->general_uidNumber)) return $return->general_uidNumber;
	// Check if Name-length is OK. minLength=3, maxLength=20
	if ( !ereg('.{3,20}', $values->general_username)) return _('Name must content between 3 and 20 characters.');
	// Check if Name starts with letter
	if ( !ereg('^[a-z].*$', $values->general_username))
		return _('Name contents invalid characters. First character must be a letter');
	return $return;
	}


function checkunix($values, $type) { // This function checks all unix account paramters
	if ($type=='user' && !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $values->unix_password))
		return _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
	if ( !ereg('^([0-9]*)$', $values->unix_pwdminage))  return _('Password Minage must be are natural number.');
	if ( $values->unix_pwdminage > $values->unix_pwdmaxage ) return _('Password Maxage must bigger as Password Minage.');
	if ( !ereg('^([1-9]+)([0-9]*)$', $values->unix_pwdmaxage)) return _('Password Maxage must be are natural number.');
	if ($values->unix_pwdminage=='') return _('No value for Password Minage.');
	if ( !ereg('^(([-][1])|([0-9]*))$', $values->unix_pwdallowlogin))
	return _('Password Expire must be are natural number or -1.');
	if ($values->unix_pwdmaxage=='') return _('No value for Password Maxage.');
	if ( !ereg('^([1-9]+)([0-9]*)$', $values->unix_pwdwarn)) return _('Password Warn must be are natural number.');
	if ($values->unix_pwdallowlogin=='') return _('No value for Password Expire.');
	if ($values->unix_pwdwarn=='') return _('No value for Password Warn.');
	return 0;
	}

function checksamba($values, $type) { // This function checks all samba account paramters
	$return = new account();
	if ($values->smb_useunixpwd) $return->smb_password = $values->unix_password;
	switch ($type) {
		case 'user' :
			$return->smb_scriptPath = str_replace('$user', $values->general_username, $values->smb_scriptPath);
			$return->smb_scriptPath = str_replace('$group', $values->general_group, $values->smb_scriptPath);
			$return->smb_profilePath = str_replace('$user', $values->general_username, $values->smb_profilePath);
			$return->smb_profilePath = str_replace('$group', $return->general_group, $return->smb_profilePath);
			$return->smb_smbHome = str_replace('$user', $values->general_username, $values->smb_smbHome);
			$return->smb_smbHome = str_replace('$group', $return->general_group, $return->smb_smbHome);
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
				$values->smb_password)) return _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ( (!$return->smb_scriptPath=='') && (!ereg('^([/])*[a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $return->smb_scriptPath)))
				return _('Scriptpath is invalid');
			if ( (!$return->smb_profilePath=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $return->smb_profilePath))
				&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-])+)+$', $return->smb_profilePath)))
					return _('ProfilePath is invalid.');
			if ( (!$return->smb_smbHome=='') && !ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-])+)+$', $return->smb_smbhome))
				return _('smbHome is invalid.');
			if ((!$values->smb_smbuserworkstations=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $values->smb_smbuserworkstations))
				return _('User Workstations is invalid.');
			$return->smb_flagsW = 0;
			break;
		case 'host' :
			$return->smb_password = $values->unix_password;
			$return->smb_flagsW = 1;
			break;
		}
	if ((!$values->smb_domain=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $values->smb_domain))
		return _('Domain Name contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.');
	if ($values->smb_useunixpwd) $return->smb_useunixpwd = 1; else $return->smb_useunixpwd = 0;
	if ($values->smb_pwdcanchange) $return->smb_pwdcanchange = 1; else $return->smb_pwdcanchange = 0;
	if ($values->smb_pwdmustchange) $return->smb_pwdmustchange = 1; else $return->smb_pwdmustchange = 0;
	return $return;
	}

function checkquota($values) { // This function checks all quota paramters
	$return = new account();
	$i=0;
	while ($values->quota[$i][0]) {
		if (!$values->quota[$i][2]) $return->quota[$i][2] = 0;
			else if (!ereg('^([0-9])*$', $values->quota[$i][2]))
				return _('Block soft quota contains invalid characters. Only natural numbers are allowed');
		if (!$values->quota[$i][3]) $return->quota[$i][3] = 0;
			else if (!ereg('^([0-9])*$', $values->quota[$i][3]))
				return _('Block hard quota contains invalid characters. Only natural numbers are allowed');
		if (!$values->quota[$i][6]) $return->quota[$i][6] = 0;
			else if (!ereg('^([0-9])*$', $values->quota[$i][6]))
				return _('Inode soft quota contains invalid characters. Only natural numbers are allowed');
		if (!$values->quota[$i][7]) $return->quota[$i][7] = 0;
			else if (!ereg('^([0-9])*$', $values->quota[$i][7]))
				return _('Inode hard quota contains invalid characters. Only natural numbers are allowed');
		$return->quota[$i][2] = $values->quota[$i][2];
		$return->quota[$i][3] = $values->quota[$i][3];
		$return->quota[$i][6] = $values->quota[$i][6];
		$return->quota[$i][7] = $values->quota[$i][7];
		$i++;
		}
	return $return;
	}


function checkpersonal($values) {
	$return = new account();
	$return = $values;
	return $return;
	}

function genpasswd() { // This function will return a password with max. 8 characters
	// Allowed Characters to generate passwords
	$LCase = 'abcdefghjkmnpqrstuvwxyz';
	$UCase = 'ABCDEFGHJKMNPQRSTUVWXYZ';
	$Integer = '23456789';
	// DEFINE CONSTANTS FOR ALGORTTHM
	define("LEN", '1');
	$a = RndInt('letter');
	$b = RndInt('letter');
	$c = RndInt('letter');
	$d = RndInt('letter');
	$e = RndInt('number');
	$f = RndInt('number');
	$g = RndInt('letter');
	$h = RndInt('letter');
	// EXTRACT 8 CHARACTERS RANDOMLY FROM TH // E DEFINITION STRINGS
	$L1 = substr($LCase, $a, LEN);
	$L2 = substr($LCase, $b, LEN);
	$L3 = substr($LCase, $h, LEN);
	$U1 = substr($UCase, $c, LEN);
	$U2 = substr($UCase, $d, LEN);
	$U3 = substr($UCase, $g, LEN);
	$I1 = substr($Integer, $e, LEN);
	$I2 = substr($Integer, $f, LEN);
	// COMBINE THE CHARACTERS AND DISPLAY TH // E NEW PASSWORD
	$PW = $L1 . $U2 . $I1 . $L2 . $I2 . $U1 . $U3 . $L3;
	return $PW;
	}

/* THIS FUNCTION GENERATES A RANDOM NUMBER THAT WILL BE USED TO
* RANDOMLY SELECT CHARACTERS FROM THE STRINGS ABOVE
*/
function RndInt($Format){
	switch ($Format){
		case 'letter':
			$Rnd = rand(0,23);
			if ($Rnd > 23){
				$Rnd = $Rnd - 1;
				}
			break;
		case 'number':
			$Rnd = rand(2,9);
			if ($Rnd > 8){
				$Rnd = $Rnd - 1;
				}
			break;
		}
	return $Rnd;
	} // END RndInt() FUNCTION
/* RUN THE FUNCTION TO GENERATE RANDOM INTEGERS FOR EACH OF THE
* 8 CHARACTERS IN THE PASSWORD PRODUCED.
*/

function getquotas($type,$user='+') { // Whis function will return the quotas from the specified user If empty only filesystems with enabled quotas are returned
	$return = new account();
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' quota get ';
	if ($type=='user') $towrite = $towrite.'u';
	 else $towrite = $towrite.'g';
	exec("/usr/bin/ssh ".$_SESSION['config']->scriptServer." sudo ".$_SESSION['config']->scriptPath." $towrite", $vals);
	$vals = explode(':', $vals[0]);
	for ($i=0; $i<sizeof($vals); $i++) {
		$vals2 = explode(',', $vals[$i]);
		for ($j=0; $j<sizeof($vals2); $j++) {
			$return->quota[$i][$j] = $vals2[$j];
			}
		if ($return->quota[$i][4]<$time) $return->quota[$i][4] = '';
			else $return->quota[$i][4] = strval(($return->quota[$i][4]-$time)/3600) . _(' hours');
		if ($return->quota[$i][8]<$time) $return->quota[$i][8] = '';
			else $return->quota[$i][8] = strval(($return->quota[$i][8]-$time)/3600) . _(' hours');
		}
	return $return;
	}

function setquotas($values,$type,$values_old=false) { // Whis function will set the quotas from the specified user.
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$values->general_username.' quota set ';
	if ($type=='user') $towrite = $towrite.'u ';
	 else $towrite = $towrite.'g ';
	$i=0;
	while ($values->quota[$i][0]) {
		if ($values->quota[$i] != $values_old->quota[$i]) {
			$towrite = $towrite. $values->quota[$i][0] .','.$values->quota[$i][2] .','.$values->quota[$i][3]
			.','.$values->quota[$i][6] .','. $values->quota[$i][7] .':';
			}
		$i++;
		}
	if ($i!=0) exec("/usr/bin/ssh ".$_SESSION['config']->scriptServer." sudo ".$_SESSION['config']->scriptPath." $towrite", $vals);
	}

function remquotas($user, $type) { // Whis function will remove the quotas from the specified user.
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' quota set ';
	if ($type=='user') $towrite = $towrite.'u ';
	 else $towrite = $towrite.'g ';
	exec("/usr/bin/ssh ".$_SESSION['config']->scriptServer." sudo ".$_SESSION['config']->scriptPath." $towrite", $vals);
	}


function addhomedir($user) { // Create Homedirectory
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' home add';
	exec("/usr/bin/ssh ".$_SESSION['config']->scriptServer." sudo ".$_SESSION['config']->scriptPath." $towrite", $vals);
	}

function remhomedir($user) { // Remove Homedirectory
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' home rem';
	exec("/usr/bin/ssh ".$_SESSION['config']->scriptServer." sudo ".$_SESSION['config']->scriptPath." $towrite", $vals);
	}


function ldapexists($values, $type, $values_old=false) { // This function will search if the DN already exists
	switch ($type) {
		case 'user':
			$searchbase = $_SESSION['config']->get_UserSuffix();
			$search = "uid=".$values->general_username;
			break;
		case 'group':
			$searchbase = $_SESSION['config']->get_GroupSuffix();
			$search = "cn=".$values->general_username;
			break;
		case 'host':
			$searchbase = $_SESSION['config']->get_HostSuffix();
			$search = "uid=".$values->general_username;
			break;
		}
	$result = ldap_search($_SESSION['ldap']->server(), $searchbase, $search , array(''), 1);
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	if ($dn) {
		if ($values_old->general_username != $values->general_username) return _($type . ' already exists!');
		if (!$values_old) return  _($type . ' already exists!');
		}
	return 0;
	}


function findgroups() { // Will return an array with all Groupnames found in LDAP
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'ObjectClass=PosixGroup', array(''), 1);
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$group[] = strtok(ldap_dn2ufn(ldap_get_dn($_SESSION['ldap']->server(), $entry)),',');
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	return $group;
	}


function getgid($groupname) { // Will return the the gid to an existing Groupname
	// Check if group already exists
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'cn=' . $groupname, array('gidNumber'), 0);
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	if ($entry) {
		$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr['gidNumber'][0]) return $attr['gidNumber'][0];
		}
	else return -1;
	}



function checkid($values, $type, $values_old=false) { // if value is empty will return an unused id from all ids found in LDAP else check existing value
	switch ($type) {
		case 'user':
			$ObjectClass = 'PosixAccount';
			$search = 'uidNumber';
			$minID = intval($_SESSION['config']->get_minUID());
			$maxID = intval($_SESSION['config']->get_maxUID());
			$suffix = $_SESSION['config']->get_UserSuffix();
			break;
		case 'group':
			$ObjectClass = 'PosixGroup';
			$search = 'gidNumber';
			$minID = intval($_SESSION['config']->get_MinGID());
			$maxID = intval($_SESSION['config']->get_MaxGID());
			$suffix = $_SESSION['config']->get_GroupSuffix();
			break;
		case 'host':
			$ObjectClass = 'PosixAccount';
			$search = 'uidNumber';
			$minID = intval($_SESSION['config']->get_MinMachine());
			$maxID = intval($_SESSION['config']->get_MaxMachine());
			$suffix = $_SESSION['config']->get_HostSuffix();
			break;
		}
	if ($values->general_uidNumber=='')
		if (!$values_old) {
			$result = ldap_search($_SESSION['ldap']->server(), $suffix, 'ObjectClass='.$ObjectClass);
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			while ($entry) {
				$vals = ldap_get_values($_SESSION['ldap']->server(), $entry, $search);
				$ids[] = $vals[0];
				$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
				}
			if ($ids) {
				sort ($ids, SORT_NUMERIC);
				if ($ids[count($ids)-1] < $maxID) {
					if ($minID > $ids[count($ids)-1]) $useID =  $minID;
					else $useID = $ids[count($ids)-1]+1;
					}
				else {
					$i=$minID;
					foreach ($ids as $id) if ($id == $i) $i++;
					$useID = $i;
					}
				}
			 else $useID = $minID;
			return $useID;
			}
		else return $values_old->general_uidNumber;
	// Check manual ID
	$result = ldap_search($_SESSION['ldap']->server(), $suffix, $search . '=' . $values->general_uidNumber, array(''), 1);
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	if ($entry) { // Entry with same ID found
		$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
		if (!$values_old) return _('ID is used from ' . $dn . ' !');
		 else if ($dn!=$values_old->general_dn) return _('ID is used from ' . $dn . ' !');
		 }
	if ( $values->general_uidNumber < $minID || $values->general_uidNumber > $maxID) return _('Please enter a value between '. $minID . ' and ' . $maxID . '!');
	return intval($values->general_uidNumber);
	}

function getdays() { // will return the days from 1.1.1970 until now
	$days = time() / 86400;
	settype($days, 'integer');
	return $days;
	}

function smbflag($values) { // Creates te attribute attrFlags
	$flag = "[";
	if ($values->smb_flagsW) $flag = $flag . "W"; else $flag = $flag . "U";
	if ($values->smb_flagsD) $flag = $flag . "D";
	if ($values->smb_flagsX) $flag = $flag . "X";
	$flag = str_pad($flag, 12);
	$flag = $flag. "]";
	return $flag;
	}

function loaduser($dn) { // Will load all needed values from an existing account
	$return = new account();
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['uid'][0]) {
		$return->general_username = $attr['uid'][0];
		if ($_SESSION['config']->scriptServer) getquotas('user',$attr['uid'][0]);
		}
	if ($attr['uidNumber'][0]) $return->general_uidNumber = $attr['uidNumber'][0];
	if ($attr['homeDirectory'][0]) $return->general_homedir = $attr['homeDirectory'][0];
	if ($attr['shadowLastChange'][0]) $return->unix_shadowLastChange = $attr['shadowLastChange'][0];
	if ($attr['loginShell'][0]) $return->general_shell = $attr['loginShell'][0];
	if ($attr['gecos'][0]) $return->general_gecos = $attr['gecos'][0];
	if ($attr['description'][0]) $return->general_gecos = $attr['description'][0];
	if ($attr['gidNumber'][0]) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('gidNumber', 'cn'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['gidNumber'][0]==$attr['gidNumber'][0]) $return->general_group = $attr2['cn'][0];
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('memberUid', 'cn'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr2['memberUid']) foreach ($attr2['memberUid'] as $id)
			if (($id==$return->general_username) && ($attr2['cn'][0]!=$return->general_group)) $return->general_groupadd[]=$attr2['cn'][0];
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	if ($attr['shadowMin'][0]) $return->unix_pwdminage = $attr['shadowMin'][0];
	if ($attr['shadowMax'][0]) $return->unix_pwdmaxage = $attr['shadowMax'][0];
	if ($attr['shadowWarning'][0]) $return->unix_pwdwarn = $attr['shadowWarning'][0];
	if ($attr['shadowInactive'][0]) $return->unix_pwdallowlogin = $attr['shadowInactive'][0];
	if ($attr['shadowExpire'][0]) {
		$date = getdate ($attr['shadowExpire'][0]*86400);
		$return->unix_pwdexpire_day = $date['mday'];
		$return->unix_pwdexpire_mon = $date['mon'];
		$return->unix_pwdexpire_yea = $date['year'];
		}
	if ($attr['pwdCanChange'][0]) $return->smb_pwdcanchange = $attr['pwdCanChange'][0];
	if ($attr['acctFlags'][0]) {
		if (strrpos($attr['acctFlags'][0], 'W')) $return->smb_flagsW=true;
		if (strrpos($attr['acctFlags'][0], 'D')) $return->smb_flagsD=true;
		if (strrpos($attr['acctFlags'][0], 'X')) $return->smb_flagsX=true;
		}
	if ($attr['smbHome'][0]) $return->smb_smbhome = $attr['smbHome'][0];
	if ($attr['homeDrive'][0]) $return->smb_homedrive = $attr['homeDrive'][0];
	if ($attr['scriptPath'][0])  $return->smb_scriptPath = $attr['scriptPath'][0];
	if ($attr['profilePath'][0]) $return->smb_profilePath = $attr['profilePath'][0];
	if ($attr['userWorkstations'][0]) $return->smb_smbuserworkstations = $attr['userWorkstations'][0];
	if ($attr['domain'][0]) $return->smb_domain = $attr['domain'][0];
	if ($attr['givenName'][0]) $return->general_givenname = $attr['givenName'][0];
	if ($attr['sn'][0]) $return->general_surname = $attr['sn'][0];
	if ($attr['title'][0]) $return->personal_title = $attr['title'][0];
	if ($attr['mail'][0]) $return->personal_mail = $attr['mail'][0];
	if ($attr['telephoneNumber'][0]) $return->personal_telephoneNumber = $attr['telephoneNumber'][0];
	if ($attr['mobilemobileTelephoneNumber'][0]) $return->personal_mobileTelephoneNumber = $attr['mobilemobileTelephoneNumber'][0];
		else 	if ($attr['mobile'][0]) $return->personal_mobileTelephoneNumber = $attr['mobile'][0];
	if ($attr['facsimileTelephoneNumber'][0]) $return->personal_facsimileTelephoneNumber = $attr['facsimileTelephoneNumber'][0];
	if ($attr['street'][0]) $return->personal_street = $attr['street'][0];
	if ($attr['postalCode'][0]) $return->personal_postalCode = $attr['postalCode'][0];
	if ($attr['postalAddress'][0]) $return->personal_postalAddress = $attr['postalAddress'][0];
	if ($attr['employeeType'][0]) $return->personal_employeeType = $attr['employeeType'][0];
	if (substr(str_replace('{CRYPT}', '',$attr['userPassword'][0]),0,1) == '!' ) $return->unix_deactivated=true;
	if ($attr['userPassword'][0]) $return->unix_password = $attr['userPassword'][0];
	if ($attr['ntPassword'][0])  $return->smb_password = $attr['ntPassword'][0];
	if ($_SESSION['config']->scriptServer) {
		$values = getquotas('user',$return->general_username);
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $return->$key = $val;
			}
		}
	return $return;
	}

function loadhost($dn) { // Will load all needed values from an existing account
	$return = new account();
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['uid'][0]) $return->general_username = $attr['uid'][0];
	if ($attr['uidNumber'][0]) $return->general_uidNumber = $attr['uidNumber'][0];
	if ($attr['shadowLastChange'][0]) $return->unix_shadowLastChange = $attr['shadowLastChange'][0];
	if ($attr['gecos'][0]) $return->general_gecos = $attr['gecos'][0];
	if ($attr['description'][0]) $return->general_gecos = $attr['description'][0];
	if ($attr['gidNumber'][0]) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('gidNumber', 'cn'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['gidNumber'][0]==$attr['gidNumber'][0]) $return->general_group = $attr2['cn'][0];
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('memberUid', 'cn'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr2['memberUid']) foreach ($attr2['memberUid'] as $id)
			if (($id==$return->general_username) && ($attr2['cn'][0]!=$return->general_group)) $return->general_groupadd[]=$attr2['cn'][0];
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	if ($attr['shadowMin'][0]) $return->unix_pwdminage = $attr['shadowMin'][0];
	if ($attr['shadowMax'][0]) $return->unix_pwdmaxage = $attr['shadowMax'][0];
	if ($attr['shadowWarning'][0]) $return->unix_pwdwarn = $attr['shadowWarning'][0];
	if ($attr['shadowInactive'][0]) $return->unix_pwdallowlogin = $attr['shadowInactive'][0];
	if ($attr['shadowExpire'][0]) {
		$date = getdate ($attr['shadowExpire'][0]*86400);
		$return->unix_pwdexpire_day = $date['mday'];
		$return->unix_pwdexpire_mon = $date['mon'];
		$return->unix_pwdexpire_yea = $date['year'];
		}
	if ($attr['pwdCanChange'][0]) $return->smb_pwdcanchange = $attr['pwdCanChange'][0];
	if ($attr['acctFlags'][0]) {
		if (strrpos($attr['acctFlags'][0], 'W')) $return->smb_flagsW=true;
		if (strrpos($attr['acctFlags'][0], 'D')) $return->smb_flagsD=true;
		if (strrpos($attr['acctFlags'][0], 'X')) $return->smb_flagsX=true;
		}
	if ($attr['domain'][0]) $return->smb_domain = $attr['domain'][0];
	if ($attr['givenName'][0]) $return->general_givenname = $attr['givenName'][0];
	if ($attr['sn'][0]) $return->general_surname = $attr['sn'][0];
	if (substr(str_replace('{CRYPT}', '',$attr['userPassword'][0]),0,1) == '!' ) $return->unix_deactivated=true;
	return $return;
	}


function loadgroup($dn) { // Will load all needed values from an existing group
	$return = new account();
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixGroup");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['gidNumber'][0]) {
		$return->general_uidNumber = $attr['gidNumber'][0];
		if ($_SESSION['config']->scriptServer) getquotas('group',$attr['uid'][0]);
		}
	if ($attr['description'][0]) $return->general_gecos = $attr['description'][0];
	if ($attr['cn'][0]) {
		$return->general_username = $attr['cn'][0];
		if ($_SESSION['config']->scriptServer) getquotas('group',$attr['cn'][0]);
		}
	if ($attr['memberUid']) $return->general_memberUid = $attr['memberUid'];
	if (is_array($return->general_memberUid)) array_shift($return->general_memberUid);
	$return->general_dn = $dn;
	if ($_SESSION['config']->scriptServer) {
		$values = getquotas('group',$return->general_username);
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $return->$key = $val;
			}
		}
	return $return;
	}


function createuser($values) { // Will create the LDAP-Account
	// 2 == Account allready exists at different location
	// 1 == Account has been created
	// 4 == Error while creating Account
	// values stored in shadowExpire, days since 1.1.1970
	$date = mktime(10,0,0, $values->unix_pwdexpire_mon, $values->unix_pwdexpire_day, $values->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$values->general_dn = 'uid=' . $values->general_username . ',' . $_SESSION['config']->get_UserSuffix();

	// All Values need for an user-account
	// General Objectclasses
	$attr['objectClass'][0] = 'posixAccount';
	$attr['objectClass'][1] = 'shadowAccount';
	$attr['objectClass'][2] = 'sambaAccount';
	$attr['objectClass'][3] = 'inetOrgPerson';
	$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $values->general_username; // posixAccount_req
	$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
	$attr['homeDirectory'] = $values->general_homedir; // posixAccount_req
	if ($values->personal_title!='') $attr['title'] = $values->personal_title;
	if ($values->personal_mail!='') $attr['mail'] = $values->personal_mail;
	if ($values->personal_telephoneNumber!='') $attr['telephoneNumber'] = $values->personal_telephoneNumber;
	if ($values->personal_mobileTelephoneNumber!='') $attr['mobilemobileTelephoneNumber'] = $values->personal_mobileTelephoneNumber;
	if ($values->personal_facsimileTelephoneNumber!='') $attr['facsimileTelephoneNumber'] = $values->personal_facsimileTelephoneNumber;
	if ($values->personal_street!='') $attr['street'] = $values->personal_street;
	if ($values->personal_postalCode!='') $attr['postalCode'] = $values->personal_postalCode;
	if ($values->personal_postalAddress!='') $attr['postalAddress'] = $values->personal_postalAddress;
	if ($values->personal_employeeType!='') $attr['employeeType'] = $values->personal_employeeType;
	// posixAccount_may shadowAccount_may
	if ($values->unix_password_no) $values->unix_password = '';
	if ($values->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($values->unix_password);
	else $attr['userPassword'] = '{CRYPT}' . crypt($values->unix_password);
	$attr['shadowLastChange'] = getdays(); // shadowAccount_may
	$attr['ntPassword'] = exec('../lib/createntlm.pl nt ' . $values->smb_password);
	$attr['lmPassword'] = exec('../lib/createntlm.pl lm ' . $values->smb_password);
	$attr['pwdLastSet'] = time(); // sambaAccount_may
	if ($values->smb_password_no) {
		$attr['ntPassword'] = 'NO PASSWORD*****';
		$attr['lmPassword'] = 'NO PASSWORD*****';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	$attr['loginShell'] = $values->general_shell; // posixAccount_may
	$attr['gecos'] = $values->general_gecos; // posixAccount_may
	$attr['description'] = $values->general_gecos; // posixAccount_may sambaAccount_may

	$attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	$attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	$attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	$attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	$attr['shadowExpire'] =  $date ; // shadowAccount_may
	$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
	$attr['PrimaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
	if ($values->smb_pwdcanchange) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdmustchange) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag(values); // sambaAccount_may
	$attr['displayName'] = $values->general_gecos; // sambaAccount_may
	if ($values->smb_smbhome!='') $attr['smbHome'] = $values->smb_smbhome; // sambaAccount_may
	if ($values->smb_homedrive!='') $attr['homeDrive'] = $values->smb_homedrive; // sambaAccount_may
	if ($values->smb_scriptPath!='') $attr['scriptPath']  = $values->smb_scriptPath; // sambaAccount_may
	if ($values->smb_profilePath!='') $attr['profilePath'] = $values->smb_profilePath; // sambaAccount_may
	if ($values->smb_smbuserworkstations!='') $attr['userWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
	if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may

	if ($values->general_givenname!='') $attr['givenName'] = $values->general_givenname;
	if ($values->general_surname!='') $attr['sn'] = $values->general_surname;

	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	if (!$success) return 4;
	if ($_SESSION['config']->scriptServer) {
		setquotas($values->general_username,'user');
		addhomedir($values->general_username);
		}
	// Add User to Additional Groups
	if ($values->general_groupadd)
		foreach ($values->general_groupadd as $group2) {
			$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup");
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($group['memberUid']) array_shift($group['memberUid']);
			if (! in_array($values->general_username, $group['memberUid'])) {
				$toadd['memberUid'] = $values->general_username;
				$success = ldap_mod_add($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), $toadd);
				}
			if (!$success) return 4;
		}
	return 1;
	}

function modifyuser($values,$values_old) { // Will modify the LDAP-Account
	// 2 == Account allready exists at different location
	// 3 == Account has been modified
	// 5 == Error while modifying Account
	// Value stored in shadowExpire, days since 1.1.1970
	$date = mktime(10,0,0, $values->unix_pwdexpire_mon, $values->unix_pwdexpire_day, $values->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$values->general_dn = 'uid=' . $values->general_username . ',' . $_SESSION['config']->get_UserSuffix();
	if ($values->general_username != $values_old->general_username) {
		$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
		$attr['uid'] = $values->general_username; // posixAccount_req
		}
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		}
	if ($values->general_group != $values_old->general_group) {
		$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
		$attr['PrimaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
		}
	if ($values->general_homedir != $values_old->general_homedir)
		$attr['homeDirectory'] = $values->general_homedir; // posixAccount_req
	// posixAccount_may shadowAccount_may
	$password_old = str_replace('{CRYPT}', '',$values_old->unix_password);
	if (substr($password_old,0,1) == '!' ) $password_old = substr($password_old,1,strlen($password_old));
	if ($values->unix_password=='') {
		if ($values->unix_password_no) $password_old = '';
		if ($values->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . $password_old;
		else $attr['userPassword'] = '{CRYPT}' . $password_old;
		$attr['shadowLastChange'] =  $values_old->unix_shadowLastChange; // shadowAccount_may
		}
	 else {
		if ($values->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($values->unix_password);
		else $attr['userPassword'] = '{CRYPT}' . crypt($values->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccount_may
		}
	if ($values->smb_password_no) {
		$attr['ntPassword'] = 'NO PASSWORD*****';
		$attr['lmPassword'] = 'NO PASSWORD*****';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	 else
		if ($values->smb_password!='') {
			$attr['ntPassword'] = exec('../lib/createntlm.pl nt ' . $values->smb_password);
			$attr['lmPassword'] = exec('../lib/createntlm.pl lm ' . $values->smb_password);
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
	if ($values->general_shell != $values_old->general_shell)
		$attr['loginShell'] = $values->general_shell; // posixAccount_may
	if ($values->general_gecos != $values_old->general_gecos) {
		$attr['gecos'] = $values->general_gecos; // posixAccount_may
		$attr['description'] = $values->general_gecos; // posixAccount_may sambaAccount_may
		$attr['displayName'] = $values->general_gecos; // sambaAccount_may
		}
	if ($values->general_pwdminage != $values_old->general_pwdminage)
		$attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	if ($values->general_pwdmaxage != $values_old->general_pwdmaxage)
		$attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	if ($values->general_pwdwarn != $values_old->general_pwdwarn)
		$attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	if ($values->general_pwdallowlogin != $values_old->general_pwdallowlogin)
		$attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	if (($values->personal_title != $values_old->personal_title) && ($values->personal_title != ''))
		$attr['title'] = $values->personal_title;
	if (($values->personal_title != $values_old->personal_title) && ($values->personal_title == ''))
		$attr_rem['title'] = $values_old->personal_title;
	if (($values->personal_mail != $values_old->personal_mail) && ($values->personal_mail != ''))
		$attr['mail'] = $values->personal_mail;
	if (($values->personal_mail != $values_old->personal_mail) && ($values->personal_mail == ''))
		$attr_rem['mail'] = $values_old->personal_mail;
	if (($values->personal_telephoneNumber != $values_old->personal_telephoneNumber) && ($values->personal_telephoneNumber !=''))
		$attr['telephoneNumber'] = $values->personal_telephoneNumber;
	if (($values->personal_telephoneNumber != $values_old->personal_telephoneNumber) && ($values->personal_telephoneNumber ==''))
		$attr_rem['telephoneNumber'] = $values_old->personal_telephoneNumber;
	if (($values->personal_mobileTelephoneNumber != $values_old->personal_mobileTelephoneNumber) && ($values->personal_mobileTelephoneNumber!=''))
		$attr['mobileTelephoneNumber'] = $values->personal_mobileTelephoneNumber;
	if (($values->personal_mobileTelephoneNumber != $values_old->personal_mobileTelephoneNumber) && ($values->personal_mobileTelephoneNumber==''))
		$attr_rem['mobilemobileTelephoneNumber'] = $values_old->personal_mobileTelephoneNumber;
	if (($values->personal_facsimileTelephoneNumber != $values_old->personal_facsimileTelephoneNumber) && ($values->personal_facsimileTelephoneNumber!=''))
		$attr['facsimileTelephoneNumber'] = $values->personal_facsimileTelephoneNumber;
	if (($values->personal_facsimileTelephoneNumber != $values_old->personal_facsimileTelephoneNumber) && ($values->personal_facsimileTelephoneNumber==''))
		$attr_rem['facsimileTelephoneNumber'] = $values_old->personal_facsimileTelephoneNumber;
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street!=''))
		$attr['street'] = $values->personal_street;
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street==''))
		$attr_rem['street'] = $values_old->personal_street;
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street!=''))
		$attr['postalCode'] = $values->personal_street;
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street==''))
		$attr_rem['postalCode'] = $values_old->personal_street;
	if (($values->personal_postalAddress != $values_old->personal_postalAddress) && ($values->personal_postalAddress!=''))
		$attr['postalAddress'] = $values->personal_postalAddress;
	if (($values->personal_postalAddress != $values_old->personal_postalAddress) && ($values->personal_postalAddress==''))
		$attr_rem['postalAddress'] = $values_old->personal_postalAddress;
	if (($values->personal_employeeType != $values_old->personal_employeeType) && ($values->personal_employeeType!=''))
		$attr['employeeType'] = $values->personal_employeeType;
	if (($values->personal_employeeType != $values_old->personal_employeeType) && ($values->personal_employeeType==''))
		$attr_rem['employeeType'] = $values_old->personal_employeeType;
	if (($values->unix_pwdexpire_day = $date['mday']!=$values_old->unix_pwdexpire_day = $date['mday']) ||
		($values->unix_pwdexpire_mon = $date['mon'] != $values_old->unix_pwdexpire_mon = $date['mon']) ||
		($values->unix_pwdexpire_yea = $date['year'] != $values->unix_pwdexpire_yea = $date['year']))
			$attr['shadowExpire'] =  $date ; // shadowAccount_may
	if ($values->smb_pwdcanchange && $values_old->smb_pwdcanchange==0) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdcanchange==0 && $values_old->smb_pwdcanchange==1) $attr_rem['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdmustchange && $values->smb_pwdmustchange==0) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdmustchange==0 && $values->smb_pwdmustchange==1) $attr_rem['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag($values); // sambaAccount_may
	if (($values->smb_smbhome!='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr['smbHome'] = $values->smb_smbhome; // sambaAccount_may
	if (($values->smb_smbhome=='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr_rem['smbHome'] = $values_old->smb_smbhome; // sambaAccount_may
	if (($values->smb_homedrive!='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr['homeDrive'] = $values->smb_homedrive; // sambaAccount_may
	if (($values->smb_homedrive=='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr_rem['homeDrive'] = $values_old->smb_homedrive; // sambaAccount_may
	if (($values->smb_scriptPath!='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr['scriptPath']  = $values->smb_scriptPath; // sambaAccount_may
	if (($values->smb_scriptPath=='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr_rem['scriptPath']  = $values_old->smb_scriptPath; // sambaAccount_may
	if (($values->smb_profilePath!='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr['profilePath'] = $values->smb_profilePath; // sambaAccount_may
	if (($values->smb_profilePath=='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr_rem['profilePath'] = $values_old->smb_profilePath; // sambaAccount_may
	if (($values->smb_smbuserworkstations!='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr['userWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
	if (($values->smb_smbuserworkstations=='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr_rem['userWorkstations'] = $values_old->smb_smbuserworkstations; // sambaAccount_may
	if (($values->smb_domain!='') && ($values->smb_domain!=$values_old->smb_domain)) $attr['domain'] = $values->smb_domain; // sambaAccount_may
	if (($values->smb_domain=='') && ($values->smb_domain!=$values_old->smb_domain)) $attr_rem['domain'] = $values_old->smb_domain; // sambaAccount_may
	if ($values->general_givenname!=$values_old->general_givenname) $attr['givenName'] = $values->general_givenname;
	if ($values->general_surname!=$values_old->general_surname) $attr['sn'] = $values->general_surname;

	if ($attr_rem) {
			$success = ldap_mod_del($_SESSION['ldap']->server(),$values->general_dn, $attr_rem);
			if (!$success) return 5;
			}
	if ($values->general_username == $values_old->general_username) // Username hasn't changed
		$success = ldap_modify($_SESSION['ldap']->server(),$values->general_dn, $attr);
			else {
				$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
				$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
				if ($success) $success = ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
				if ($success) $success = ldap_mod_replace($_SESSION['ldap']->server(),$values->general_dn, $attr);
				}
	if (!$success) return 5;
	// Write Groupmemberchips
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'objectClass=PosixGroup', array('memberUid', 'cn'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$modifygroup=0;
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr2['memberUid']) {
			array_shift($attr2['memberUid']);
			foreach ($attr2['memberUid'] as $nam) {
				if ( ($nam==$values->general_username) && !in_array($attr2['cn'][0], $values->general_groupadd)) {
					$todelete['memberUid'] = $nam;
					$success = ldap_mod_del($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry) ,$todelete);
					if (!$success) return 5;
					}
				}
			if (!in_array($values->general_username, $attr2['memberUid']) && in_array($attr2['cn'][0], $values->general_groupadd) && ($attr2['cn'][0]!=$values->general_group)) {
				$toadd['memberUid'] = $attr2['memberUid'];
				$toadd['memberUid'][] = $values->general_username;
				$success = ldap_mod_replace($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
				if (!$success) return 5;
				}
			}
		 else {
			if (in_array($attr2['cn'][0], $values->general_groupadd) && ($attr2['cn'][0]!=$values->general_group)) {
				$toadd['memberUid'] = $values->general_username;
				$success = ldap_mod_add($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
				if (!$success) return 5;
				}
			}
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	if ($_SESSION['config']->scriptServer) setquotas($values->general_username,'user',$values_old->general_username);
	return 3;
	}



function createhost($values) { // Will create the LDAP-Account
	// 2 == Account allready exists at different location
	// 1 == Account has been created
	// 3 == Account has been modified
	// 4 == Error while creating Account
	// 5 == Error while modifying Account
	// Value stored in shadowExpire, days since 1.1.1970
	$date = mktime(10,0,0, $values->unix_pwdexpire_mon, $values->unix_pwdexpire_day, $values->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$values->general_dn = 'uid=' . $values->general_username . ',' . $_SESSION['config']->get_HostSuffix();

	// All Values need for an host-account
	// General Objectclasses
	$attr['objectClass'][0] = 'posixAccount';
	$attr['objectClass'][1] = 'shadowAccount';
	$attr['objectClass'][2] = 'sambaAccount';
	$attr['objectClass'][3] = 'account';
	$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $values->general_username; // posixAccount_req
	$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
	$attr['homeDirectory'] = $values->general_homedir; // posixAccount_req

	// posixAccount_may shadowAccount_may
	if ($values->unix_password_no) $values->unix_password = '';
	if ($values->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($values->unix_password);
	else $attr['userPassword'] = '{CRYPT}' . crypt($values->unix_password);
	$attr['shadowLastChange'] = getdays(); // shadowAccount_may
	$attr['ntPassword'] = exec('../lib/createntlm.pl nt ' . $values->smb_password);
	$attr['lmPassword'] = exec('../lib/createntlm.pl lm ' . $values->smb_password);
	$attr['pwdLastSet'] = time(); // sambaAccount_may
	if ($values->smb_password_no) {
		$attr['ntPassword'] = 'NO PASSWORD*****';
		$attr['lmPassword'] = 'NO PASSWORD*****';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	$attr['loginShell'] = $values->general_shell; // posixAccount_may
	$attr['gecos'] = $values->general_gecos; // posixAccount_may
	$attr['description'] = $values->general_gecos; // posixAccount_may sambaAccount_may

	$attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	$attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	$attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	$attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	$attr['shadowExpire'] =  $date ; // shadowAccount_may
	$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
	$attr['PrimaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
	if ($values->smb_pwdcanchange) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdmustchange) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag(); // sambaAccount_may
	$attr['displayName'] = $values->general_gecos; // sambaAccount_may
	if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	if (!$success) return 4;
	// Add Host to Additional Groups
	if ($values->general_groupadd)
		foreach ($values->general_groupadd as $group2) {
			$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup");
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($group['memberUid']) array_shift($group['memberUid']);
			if (! in_array($values->general_username, $group['memberUid'])) {
				$toadd['memberUid'] = $values->general_username;
				$success = ldap_mod_add($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), $toadd);
				}
			if (!$success) return 4;
			}
	return 1;
	}

function modifyhost($values,$values_old) { // Will modify the LDAP-Account
	// 2 == Account allready exists at different location
	// 3 == Account has been modified
	// 5 == Error while modifying Account
	// Value stored in shadowExpire, days since 1.1.1970
	$date = mktime(10,0,0, $values->unix_pwdexpire_mon, $values->unix_pwdexpire_day, $values->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$values->general_dn = 'uid=' . $values->general_username . ',' . $_SESSION['config']->get_HostSuffix();
	if ($values->general_username != $values_old->general_username) {
		$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
		$attr['uid'] = $values->general_username; // posixAccount_req
		}
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		}
	if ($values->general_group != $values_old->general_group) {
		$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
		$attr['PrimaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
		}
	if ($values->general_homedir != $values_old->general_homedir)
		$attr['homeDirectory'] = $values->general_homedir; // posixAccount_req
	// posixAccount_may shadowAccount_may
	$password_old = str_replace('{CRYPT}', '',$values_old->unix_password);
	if (substr($password_old,0,1) == '!' ) $password_old = substr($password_old,1,strlen($password_old));
	if ($values->unix_password=='') {
		if ($values->unix_password_no) $password_old = '';
		if ($values->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . $password_old;
		else $attr['userPassword'] = '{CRYPT}' . $password_old;
		$attr['shadowLastChange'] =  $values_old->unix_shadowLastChange; // shadowAccount_may
		}
	 else {
		if ($values->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($values->unix_password);
		else $attr['userPassword'] = '{CRYPT}' . crypt($values->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccount_may
		}
	if ($values->smb_password_no) {
		$attr['ntPassword'] = 'NO PASSWORD*****';
		$attr['lmPassword'] = 'NO PASSWORD*****';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	 else
		if ($values->smb_password!='') {
			$attr['ntPassword'] = exec('../lib/createntlm.pl nt ' . $values->smb_password);
			$attr['lmPassword'] = exec('../lib/createntlm.pl lm ' . $values->smb_password);
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
	if ($values->general_shell != $values_old->general_shell)
		$attr['loginShell'] = $values->general_shell; // posixAccount_may
	if ($values->general_gecos != $values_old->general_gecos) {
		$attr['gecos'] = $values->general_gecos; // posixAccount_may
		$attr['description'] = $values->general_gecos; // posixAccount_may sambaAccount_may
		$attr['displayName'] = $values->general_gecos; // sambaAccount_may
		}
	if ($values->general_pwdminage != $values_old->general_pwdminage)
		$attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	if ($values->general_pwdmaxage != $values_old->general_pwdmaxage)
		$attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	if ($values->general_pwdwarn != $values_old->general_pwdwarn)
		$attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	if ($values->general_pwdallowlogin != $values_old->general_pwdallowlogin)
		$attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	if (($values->unix_pwdexpire_day = $date['mday']!=$values_old->unix_pwdexpire_day = $date['mday']) ||
		($values->unix_pwdexpire_mon = $date['mon'] != $values_old->unix_pwdexpire_mon = $date['mon']) ||
		($values->unix_pwdexpire_yea = $date['year'] != $values->unix_pwdexpire_yea = $date['year']))
			$attr['shadowExpire'] =  $date ; // shadowAccount_may
	if ($values->smb_pwdcanchange && $values_old->smb_pwdcanchange==0) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdcanchange==0 && $values_old->smb_pwdcanchange==1) $attr_rem['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdmustchange && $values->smb_pwdmustchange==0) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	if ($values->smb_pwdmustchange==0 && $values->smb_pwdmustchange==1) $attr_rem['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag($values); // sambaAccount_may
	if (($values->smb_domain!='') && ($values->smb_domain!=$values_old->smb_domain)) $attr['domain'] = $values->smb_domain; // sambaAccount_may
	if (($values->smb_domain=='') && ($values->smb_domain!=$values_old->smb_domain)) $attr_rem['domain'] = $values_old->smb_domain; // sambaAccount_may

	if ($attr_rem) {
		$success = ldap_mod_del($_SESSION['ldap']->server(),$values->general_dn, $attr_rem);
		if (!$success) return 5;
		}
	if ($values->general_username == $values_old->general_username) // Hostname hasn't changed
		$success = ldap_modify($_SESSION['ldap']->server(),$values->general_dn, $attr);
	else {
		$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
		if ($success) $success = ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
		if ($success) $success = ldap_mod_replace($_SESSION['ldap']->server(),$values->general_dn, $attr);
		}
	if (!$success) return 5;
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'objectClass=PosixGroup', array('memberUid', 'cn'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$modifygroup=0;
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr2['memberUid']) {
			array_shift($attr2['memberUid']);
			foreach ($attr2['memberUid'] as $nam) {
				if ( ($nam==$values->general_username) && !in_array($attr2['cn'][0], $values->general_groupadd)) {
					$todelete['memberUid'] = $nam;
					$success = ldap_mod_del($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry) ,$todelete);
					if (!$success) return 5;
					}
				}
			if (!in_array($values->general_username, $attr2['memberUid']) && in_array($attr2['cn'][0], $values->general_groupadd) && ($attr2['cn'][0]!=$values->general_group)) {
				$toadd['memberUid'] = $attr2['memberUid'];
				$toadd['memberUid'][] = $values->general_username;
				$success = ldap_mod_replace($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
				if (!$success) return 5;
				}
			}
		 else {
			if (in_array($attr2['cn'][0], $values->general_groupadd) && ($attr2['cn'][0]!=$values->general_group)) {
				$toadd['memberUid'] = $values->general_username;
				$success = ldap_mod_add($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
				if (!$success) return 5;
				}
			}
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	return 3;
	}



function creategroup($values) { // Will create the LDAP-Group
	// 2 == Group allready exists at different location
	// 1 == Group has been created
	// 3 == Group has been modified
	// 4 == Error while creating Group
	// 5 == Error while modifying Group
	$values->general_dn = 'cn=' . $values->general_username . ',' . $_SESSION['config']->get_GroupSuffix();
	$attr['objectClass'] = 'posixGroup';
	$attr['cn'] = $values->general_username;
	$attr['gidNumber'] = $values->general_uidNumber;
	$attr['description'] = $values->general_gecos;
	if ($values->general_memeberUid) $attr['memberUid'] = $values->general_memberUid;
	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	if ($_SESSION['config']->scriptServer) setquotas($attr['uid'][0],'group');
	if ($success) return 1;
	else return 4;
	}

function modifygroup($values,$values_old) { // Will modify the LDAP-Group
	// 2 == Group allready exists at different location
	// 3 == Group has been modified
	// 5 == Error while modifying Group
	$values->general_dn = 'cn=' . $values->general_username . ',' . $_SESSION['config']->get_GroupSuffix();
	if ($values->general_username != $values_old->general_username) $attr['cn'] = $values->general_username;
	if ($values->general_uidNumber != $values_old->general_uidNumber) $attr['gidNumber'] = $values->general_uidNumber;
	if ($values->general_gecos != $values_old->general_gecos) $attr['description'] = $values->general_gecos;
	if ($values->general_memeberUid != $values_old->general_memberUid) $attr['memberUid'] = $values->general_memberUid;
	if ($values->general_username == $values_old->general_username) // Groupname hasn't changed
		$success = ldap_mod_replace($_SESSION['ldap']->server(),$values->general_dn, $attr);
	else {
		$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixGroup");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
		if ($success) ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
		if ($success) $success = ldap_mod_replace($_SESSION['ldap']->server(),$values->general_dn, $attr);
		}
	if (!$success) return 5;
	if ( $_SESSION['final_changegids']==true ) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), 'gidNumber=' . $values_old->general_uidNumber, array('gidNumber'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$user['gidNumber'][0] = $values->general_uidNumber;
			ldap_modify($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry), $user);
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}
	if ($_SESSION['config']->scriptServer) setquotas($attr['uid'][0],'group');
	return 3;
	}


?>
