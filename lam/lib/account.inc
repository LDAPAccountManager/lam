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
	// Type : user | group | host
	var $type;
	// General Settings
	var $general_objectClass;	// Array, contains old objectclasses of loaded account
	var $general_username;		// string Username, Hostname or Groupname
	var $general_uidNumber;		// string UIDNumber(user|host) GIDNumber(group) only natural numbers allowed
	var $general_surname;		// string Surname (user)
	var $general_givenname;		// string Givenname (user)
	var $general_dn;		// string DN
	var $general_group;		// string Primary group (user|host)
	var $general_groupadd;		// array(string) Addititional Groups (user|host) is member of
	var $general_homedir;		// atring Homedirectoy (user) For host it's hardcoded to/dev/null
	var $general_shell;		// array(string) list off all valid shells (user) hosts are hard-wired to /bin/false
	var $general_gecos;		// string, gecos-field (user|roup|host)
	// Unix Password Settings
	var $unix_memberUid;		// array Stores all users which are member of group but is not primary group (group)
	var $unix_password;		// string for unix-password (user|host)
	var $unix_password_no;		// string (0|1) set unix-password to none (user|host)
	var $unix_pwdwarn;		// string number of days a user is warned before password expires (user|host) value must be a natural number (user|host)
	var $unix_pwdallowlogin;	// string number of days a user can login even his password has expired (user) muste be a natural number or 0 or -1 (user|host)
	var $unix_pwdmaxage;		// string Number of days after a user has to change his password again Value must be 0<. (user|host)
	var $unix_pwdminage;		// string Number of days a user has to wait until he\'s allowed to change his password again. Value must be 0<. (user|host)
	var $unix_pwdexpire;	// string days since 1.1.1970 the account expires (user|host)
	var $unix_deactivated;		// string (1|0) account deactivated? (user|host)
	var $unix_shadowLastChange;	// string, contains the days since 1.1.1970 the password has been changed last time (user|host)
	var $unix_host;			// list of unix hosts the user is allowed to log in
	// Samba Account
	var $smb_password;		// string for samba-password (user|host)
	var $smb_password_no;		// string (1|0) set samba-password to none (user|host)
	var $smb_useunixpwd;		// string (1|0) use unix-password as samba-password (user|host)
	var $smb_pwdcanchange;		// string unix-timestamp user/host is able to change password (user|host)
	var $smb_pwdmustchange;		// string unix-timestamp user/host has to change password at next login (user|host)
	var $smb_homedrive;		// string Homedrive (C:, D:, ...) (user)
	var $smb_scriptPath;		// string ScriptPath (\\server\loginscript) (user)
	var $smb_profilePath;		// string profilePAth (\\server\profilepath) (user)
	var $smb_smbuserworkstations;	// string comma-separated list of workstations (user)
	var $smb_smbhome;		// string Home-Share (\\server\home) (user)
	var $smb_domain;		// string Domain of (user|host) or samba3domain-Object
	var $smb_flagsW;		// string (1|0) account is host? (user|host)
	var $smb_flagsD;		// string (1|0) account is disabled? (user|host)
	var $smb_flagsX;		// string (1|0) password doesn'T expire (user|host)
	var $smb_mapgroup;		// decimal ID for groups
	var $smb_displayName;		// string, description, similar to gecos-field.
	// Quota Settins
	var $quota;			// array[][] First array is an index for every chare with active quotas
					// second array Contains values for every share:
					// mountpoint, used blocks, soft block limit, hard block limit, grace block period, used inodes,
					// soft inode limit, hard inode limit, grace inode period
	// Personal Settings
	var $personal_title;		// string title of user
	var $personal_mail;		// string mailaddress of user
	var $personal_telephoneNumber;	// string telephonenumber of user
	var $personal_mobileTelephoneNumber; // string mobile umber of user
	var $personal_facsimileTelephoneNumber; // strinf fax-number of user
	var $personal_street;		// stirng streetname of user
	var $personal_postalCode;	// string postal code of user
	var $personal_postalAddress;	// string postal Address of user
	var $personal_employeeType;	// string employe type of user
	}


function getshells() { // Return a list of all shells listed in ../config/shells
	$shells =  file($_SESSION['lampath'].'config/shells');
	$i=0;
	while (count($shells) > $i) {
		// remove whitespaces
		trim($shells[$i]);
		// remove lineend
		$shells[$i] = substr($shells[$i], 0, strpos($shells[$i], "\n"));
		// unset comments
		if ($shells[$i]{0}=='#') unset ($shells[$i]);
		 else $i++;
		}
	return $shells;
	}

function replace_umlaut($text) { // This function will replace umlates with ascci-chars
	$aTranslate = array("ä"=>"ae", "Ä"=>"Ae",
		"ö"=>"oe", "Ö"=>"Oe",
		"ü"=>"ue", "Ü"=>"Ue",
		"ß"=>"ss"
		);
	return strtr($text, $aTranslate);
	}

function array_delete($values, $array) { // This function will return all values from $array without values of $values
	foreach ($array as $array_value)
		if (!in_array($array_value, $values))
			$return[] = $array_value;
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
	// $type = user or group
	// $user = user or groupname if no user or groupname is defined,
	//         an array with all quota-enabled partitions is returned in this case all returned values are 0 exept mointpoint[x][0]
	$return = new account();
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' quota get ';
	if ($type=='user') $towrite = $towrite.'u';
	 else $towrite = $towrite.'g';
	exec("perl ".$_SESSION['lampath']."lib/lamdaemon.pl ".$_SESSION['config']->scriptServer ." ".$_SESSION['config']->scriptPath." ".$towrite, $vals, $status);
	$vals = explode(':', $vals[0]);
	for ($i=0; $i<sizeof($vals); $i++) {
		$vals2 = explode(',', $vals[$i]);
		for ($j=0; $j<sizeof($vals2); $j++) {
			$return->quota[$i][$j] = $vals2[$j];
			}
		if ($return->quota[$i][4]<$time) $return->quota[$i][4] = '';
			else $return->quota[$i][4] = strval(($return->quota[$i][4]-$time)/3600) .' '. _('hours');
		if ($return->quota[$i][8]<$time) $return->quota[$i][8] = '';
			else $return->quota[$i][8] = strval(($return->quota[$i][8]-$time)/3600) .' '. _('hours');
		}
	return $return;
	}

function setquotas($values,$type,$values_old=false) { // Whis function will set the quotas from the specified user.
	// $values = object account with quotas which should be set
	// $type: user or group
	// $values_old = object account if set values and values_old will be compared. Quota will only be changed
	//               if values differ
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
	if ($i!=0) exec("perl ".$_SESSION['lampath']."lib/lamdaemon.pl ".$_SESSION['config']->scriptServer ." ".$_SESSION['config']->scriptPath." ".$towrite, $vals);
	}

function remquotas($user, $type) { // Whis function will remove the quotas from the specified user.
	// $user = username of which quta should be deleted
	// $type = user or group
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' quota set ';
	if ($type=='user') $towrite = $towrite.'u ';
	 else $towrite = $towrite.'g ';

	exec("perl ".$_SESSION['lampath']."lib/lamdaemon.pl ".$_SESSION['config']->scriptServer ." ".$_SESSION['config']->scriptPath." ".$towrite, $vals);
	}


function addhomedir($user) { // Create Homedirectory
	// $user = username
	// all other needed vars are taken from remotesystem getusrnam
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' home add';
	exec("perl ".$_SESSION['lampath']."lib/lamdaemon.pl ".$_SESSION['config']->scriptServer ." ".$_SESSION['config']->scriptPath." ".$towrite, $vals);
	}

function remhomedir($user) { // Remove Homedirectory
	// $user = username
	// all other needed vars are taken from remotesystem getusrnam
	$ldap_q = $_SESSION['ldap']->decrypt();
	$towrite = $ldap_q[0].' '.$ldap_q[1].' '.$user.' home rem';
	exec("perl ".$_SESSION['lampath']."lib/lamdaemon.pl ".$_SESSION['config']->scriptServer ." ".$_SESSION['config']->scriptPath." ".$towrite, $vals);
	}

function ldapreload($type) { // This function will load an array th cache ldap-requests
	switch ($type) {
		case 'user':
			if ((!isset($_SESSION['userDN'])) || ($_SESSION['userDN'][0] < time()-$_SESSION['config']->get_cacheTimeoutSec())) {
				if (isset($_SESSION['userDN'])) unset($_SESSION['userDN']);
				$_SESSION['userDN'][0] = time();
				$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(),
				'objectClass=posixAccount', array('cn', 'uidNumber'), 0);
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if (isset($attr['cn'][0]))
						$_SESSION['userDN'][$dn]['cn'] = $attr['cn'][0];
					if (isset($attr['uidNumber'][0]))
						$_SESSION['userDN'][$dn]['uidNumber'] = $attr['uidNumber'][0];
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				}
			break;
		case 'group':
			if ((!isset($_SESSION['groupDN'])) || ($_SESSION['groupDN'][0] < time()-$_SESSION['config']->get_cacheTimeoutSec())) {
				if (isset($_SESSION['groupDN'])) unset($_SESSION['groupDN']);
				$_SESSION['groupDN'][0] = time();
				$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(),
				'objectClass=posixGroup', array('gidNumber', 'cn'), 0);
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if (isset($attr['gidNumber'][0]))
						$_SESSION['groupDN'][$dn]['uidNumber'] = $attr['gidNumber'][0];
					if (isset($attr['cn'][0]))
						$_SESSION['groupDN'][$dn]['cn'] = $attr['cn'][0];
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				}
			break;
		case 'host':
			if ((!isset($_SESSION['hostDN'])) || ($_SESSION['hostDN'][0] < time()-$_SESSION['config']->get_cacheTimeoutSec())) {
				if (isset($_SESSION['hostDN'])) unset($_SESSION['hostDN']);
				$_SESSION['hostDN'][0] = time();
				$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_HostSuffix(),
				'objectClass=posixAccount', array('cn', 'uidNumber'), 0);
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if (isset($attr['cn'][0]))
						$_SESSION['hostDN'][$dn]['cn'] = $attr['cn'][0];
					if (isset($attr['uidNumber'][0]))
						$_SESSION['hostDN'][$dn]['uidNumber'] = $attr['uidNumber'][0];
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				}
			break;
		}
	return 0;
	}




function ldapexists($values, $type, $values_old=false) { // This function will search if the DN already exists
	switch ($type) {
		case 'user':
			ldapreload('user');
			$search = 'uid='.$values->general_username;
			$keys = array_keys($_SESSION['userDN']);
			unset ($keys[0]);
			$keys = array_values($keys);
			if ( ($values_old->general_username != $values->general_username) &&
				($_SESSION['userDN'][0] != $values->general_username)) {
				foreach ($keys as $key)
					if (strstr($key, $search)) return sprintf (_('%s already exists!'), $type);
				}
			if ((!$values_old) && ($_SESSION['userDN'][0] != $values->general_username))
				foreach ($keys as $key)
					if (strstr($key, $search)) return sprintf (_('%s already exists!'), $type);
			break;
		case 'group':
			ldapreload('group');
			$search = 'cn='.$values->general_username;
			$keys = array_keys($_SESSION['groupDN']);
			unset ($keys[0]);
			$keys = array_values($keys);
			if ( ($values_old->general_username != $values->general_username) &&
				($_SESSION['groupDN'][0] != $values->general_username)) {
				foreach ($keys as $key)
					if (strstr($key, $search)) return sprintf (_('%s already exists!'), $type);
				}
			if ((!$values_old) && ($_SESSION['groupDN'][0] != $values->general_username))
				foreach ($keys as $key)
					if (strstr($key, $search)) return sprintf (_('%s already exists!'), $type);
			break;
		case 'host':
			ldapreload('host');
			$search = 'uid='.$values->general_username;
			$keys = array_keys($_SESSION['hostDN']);
			unset ($keys[0]);
			$keys = array_values($keys);
			if ( ($values_old->general_username != $values->general_username) &&
				($_SESSION['hostDN'][0] != $values->general_username)) {
				foreach ($keys as $key)
					if (strstr($key, $search)) return sprintf (_('%s already exists!'), $type);
				}
			if ((!$values_old) && ($_SESSION['hostDN'][0] != $values->general_username))
				foreach ($keys as $key)
					if (strstr($key, $search)) return sprintf (_('%s already exists!'), $type);
			break;
		}
	return 0;
	}


function findgroups() { // Will return an array with all Groupnames found in LDAP
	ldapreload('group');
	$groups = $_SESSION['groupDN'];
	unset ($groups[0]);
	foreach ($groups as $group) {
		$return[] = $group['cn'];
		}
	sort ($return, SORT_STRING);
	return $return;
	}


function getgid($groupname) { // Will return the the gid to an existing Groupname
	ldapreload('group');
	$keys = $_SESSION['groupDN'];
	unset ($keys[0]);
	foreach ($keys as $key) {
		if ($key['cn']==$groupname)  return $key['uidNumber'];
		}
	return -1;
	}



function checkid($values, $type, $values_old=false) { // if value is empty will return an unused id from all ids found in LDAP else check existing value
	switch ($type) {
		case 'user':
			ldapreload('user');
			ldapreload('host');
			$minID = intval($_SESSION['config']->get_minUID());
			$maxID = intval($_SESSION['config']->get_maxUID());
			$suffix = $_SESSION['config']->get_UserSuffix();
			$keys = $_SESSION['userDN'];
			unset ($keys[0]);
			$keys = array_values($keys);
			$keys2 = $_SESSION['hostDN'];
			unset ($keys2[0]);
			$keys2 = array_values($keys2);
			break;
		case 'group':
			ldapreload('group');
			$minID = intval($_SESSION['config']->get_MinGID());
			$maxID = intval($_SESSION['config']->get_MaxGID());
			$suffix = $_SESSION['config']->get_GroupSuffix();
			$keys = $_SESSION['groupDN'];
			unset ($keys[0]);
			$keys = array_values($keys);
			break;
		case 'host':
			ldapreload('user');
			ldapreload('host');
			$minID = intval($_SESSION['config']->get_MinMachine());
			$maxID = intval($_SESSION['config']->get_MaxMachine());
			$suffix = $_SESSION['config']->get_HostSuffix();
			$keys = $_SESSION['hostDN'];
			unset ($keys[0]);
			$keys = array_values($keys);
			$keys2 = $_SESSION['userDN'];
			unset ($keys2[0]);
			$keys2 = array_values($keys2);
			break;
		}
	// Store all used UIDs in Array
	if (isset($keys2)) foreach ($keys2 as $key) $keys3[] = $key['uidNumber'];
	foreach ($keys as $key) $keys3[] = $key['uidNumber'];
	sort ($keys3, SORT_NUMERIC);
	if ($values->general_uidNumber=='') {
		if (!isset($values_old->general_uidNumber)) {
			if ($keys) {
				foreach ($keys as $key)
					if ($key['uidNumber'] > $id) $id = $key['uidNumber'];
				if ($id < $maxID) { // Check if Id is not used by host<->user
					if (!isset($keys2)) return intval($id+1); // keys2 is not set 4 groups
						else {
						$id++;
						while (!in_array($id, $keys3)) $id++;
						if ($id < $maxID) return intval($id);
						}
					}
				if ($id < $minID) { // Check if id is not used by host<->user
					if (!isset($keys2)) return intval($minID); // keys2 is not set 4 groups
						else {
						$id = $minID;
						while (!in_array($id, $keys3)) $id++;
						if ($id < $maxID) return intval($id);
						}
					}

				$id=$minID;
				while ($values->general_uidNumber=='') {
					if ($keys3[$id]>$maxID) return _('No free ID-Number!');
					// if !=1, we've found an unused id
					if ($keys3[$id+1]-$keys3[$id]!=1) return intval($keys3[$id]+1);
					$id++;
					}
				}
			 else $useID = $minID;
			return intval($useID);
			}
		else {
			return intval($values_old->general_uidNumber);
			}
		}
	// Check manual ID
	if ( $values->general_uidNumber < $minID || $values->general_uidNumber > $maxID) return sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID);
	if ((in_array($values->general_uidNumber, $keys3)) && !$values_old) return _('ID is already in use');
	if ((in_array($values->general_uidNumber, $keys3)) && $values_old && ($values_old->general_uidNumber != $values->general_uidNumber) )
		return _('ID is already in use');
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
	$return->type='user';
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = ldap_get_dn($_SESSION['ldap']->server(), $entry);
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);

	if (isset($attr['uid'][0])) $return->general_username = $attr['uid'][0];
	if (isset($attr['uidNumber'][0])) $return->general_uidNumber = $attr['uidNumber'][0];
	if (isset($attr['homeDirectory'][0])) $return->general_homedir = $attr['homeDirectory'][0];
	if (isset($attr['shadowLastChange'][0])) $return->unix_shadowLastChange = $attr['shadowLastChange'][0];
	if (isset($attr['loginShell'][0])) $return->general_shell = $attr['loginShell'][0];
	if (isset($attr['gecos'][0])) $return->general_gecos = $attr['gecos'][0];

	// get groupname
	if (isset($attr['gidNumber'][0])) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('gidNumber', 'cn'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['gidNumber'][0]==$attr['gidNumber'][0]) $return->general_group = $attr2['cn'][0];
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}

	// get all additional groupmemberships
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('memberUid', 'cn'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr2['memberUid']) foreach ($attr2['memberUid'] as $id)
			if (($id==$return->general_username) && ($attr2['cn'][0]!=$return->general_group)) $return->general_groupadd[]=$attr2['cn'][0];
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}

	if (isset($attr['shadowMin'][0])) $return->unix_pwdminage = $attr['shadowMin'][0];
	if (isset($attr['shadowMax'][0])) $return->unix_pwdmaxage = $attr['shadowMax'][0];
	if (isset($attr['shadowWarning'][0])) $return->unix_pwdwarn = $attr['shadowWarning'][0];
	if (isset($attr['shadowInactive'][0])) $return->unix_pwdallowlogin = $attr['shadowInactive'][0];
	if (isset($attr['shadowExpire'][0])) $return->unix_pwdexpire = $attr['shadowExpire'][0]*86400;

	// load hosts
	$i=0;
	while (isset($attr['host'][$i])) {
		if ($i==0) $return->unix_host = $attr['host'][$i];
			else $return->unix_host = $return->unix_host . ', ' . $attr['host'][$i];
		$i++;
		}
	$i=0;

	while (isset($attr['objectClass'][$i])) {
		$return->general_objectClass[$i] = $attr['objectClass'][$i];
		$i++;
		}

	// load personal settings
	if (isset($attr['givenName'][0])) $return->general_givenname = utf8_decode($attr['givenName'][0]);
	if (isset($attr['sn'][0])) $return->general_surname = utf8_decode($attr['sn'][0]);
	if (isset($attr['title'][0])) $return->personal_title = utf8_decode($attr['title'][0]);
	if (isset($attr['mail'][0])) $return->personal_mail = utf8_decode($attr['mail'][0]);
	if (isset($attr['telephoneNumber'][0])) $return->personal_telephoneNumber = utf8_decode($attr['telephoneNumber'][0]);
	if (isset($attr['mobilemobileTelephoneNumber'][0])) $return->personal_mobileTelephoneNumber = utf8_decode($attr['mobilemobileTelephoneNumber'][0]);
		else 	if (isset($attr['mobile'][0])) $return->personal_mobileTelephoneNumber = utf8_decode($attr['mobile'][0]);
	if (isset($attr['facsimileTelephoneNumber'][0])) $return->personal_facsimileTelephoneNumber = utf8_decode($attr['facsimileTelephoneNumber'][0]);
	if (isset($attr['street'][0])) $return->personal_street = utf8_decode($attr['street'][0]);
	if (isset($attr['postalCode'][0])) $return->personal_postalCode = utf8_decode($attr['postalCode'][0]);
	if (isset($attr['postalAddress'][0])) $return->personal_postalAddress = utf8_decode($attr['postalAddress'][0]);
	if (isset($attr['employeeType'][0])) $return->personal_employeeType = utf8_decode($attr['employeeType'][0]);
	if ( ereg('^[{]([A-Z]|[a-z]|[0-9])+[}][!]', $attr['userPassword'][0])) $return->unix_deactivated=true;

	if (isset($attr['userPassword'][0])) $return->unix_password = $attr['userPassword'][0];

	if (isset($attr['displayName'][0])) $return->smb_displayName = utf8_decode($attr['displayName'][0]);

	if (in_array('sambaSamAccount', $attr['objectClass'])) {
		if (isset($attr['sambaAcctFlags'][0])) {
			if (strrpos($attr['sambaAcctFlags'][0], 'W')) $return->smb_flagsW=true;
			if (strrpos($attr['sambaAcctFlags'][0], 'D')) $return->smb_flagsD=true;
			if (strrpos($attr['sambaAcctFlags'][0], 'X')) $return->smb_flagsX=true;
			}
		if (isset($attr['sambaPwdCanChange'][0])) $return->smb_pwdcanchange = $attr['sambaPwdCanChange'][0];
		if (isset($attr['sambaPwdMustChange'][0])) $return->smb_pwdmustchange = $attr['sambaPwdMustChange'][0];
		if (isset($attr['sambaHomePath'][0])) $return->smb_smbhome = utf8_decode($attr['sambaHomePath'][0]);
		if (isset($attr['sambaHomeDrive'][0])) $return->smb_homedrive = $attr['sambaHomeDrive'][0];
		if (isset($attr['sambaLogonScript'][0]))  $return->smb_scriptPath = utf8_decode($attr['sambaLogonScript'][0]);
		if (isset($attr['sambaProfilePath'][0])) $return->smb_profilePath = $attr['sambaProfilePath'][0];
		if (isset($attr['sambaUserWorkstations'][0])) $return->smb_smbuserworkstations = $attr['sambaUserWorkstations'][0];
		if (isset($attr['sambaDomainName'][0])) $return->smb_domain = $attr['sambaDomainName'][0];
		if (isset($attr['sambaNTPassword'][0]))  $return->smb_password = $attr['sambaNTPassword'][0];
		if (isset($attr['sambaDomainName'][0])) {
			if ($_SESSION['config']->samba3=='yes') {
				$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
				$i=0;
				while ($i!=-1) {
					if ($attr['sambaDomainName'][0] == $samba3domains[$i]->name) {
						$return->smb_domain = $samba3domains[$i];
						$i = -1;
						}
					else $i++;
					}
				}
			else {
				$return->smb_domain = $attr['sambaDomainName'];
				}
			}
		if (isset($attr['sambaPrimaryGroupSID'][0])) {
			if ($_SESSION['config']->samba3=='yes')
				$return->smb_mapgroup = $attr['sambaPrimaryGroupSID'][0];
				else $return->smb_mapgroup = 2*$attr['gidNumber'][0]+1001;
			}
		// return value to prevent loaded values to be overwritten from old samba 2.2 attributes
		if ($_SESSION['config']->is_samba3()) return $return;
		}

	if (in_array('sambaAccount', $attr['objectClass'])) {
		if (isset($attr['acctFlags'][0])) {
			if (strrpos($attr['acctFlags'][0], 'W')) $return->smb_flagsW=true;
			if (strrpos($attr['acctFlags'][0], 'D')) $return->smb_flagsD=true;
			if (strrpos($attr['acctFlags'][0], 'X')) $return->smb_flagsX=true;
			}
		if (isset($attr['ntPassword'][0]))  $return->smb_password = $attr['ntPassword'][0];
		if (isset($attr['smbHome'][0])) $return->smb_smbhome = utf8_decode($attr['smbHome'][0]);
		if (isset($attr['pwdCanChange'][0])) $return->smb_pwdcanchange = $attr['pwdCanChange'][0];
		if (isset($attr['pwdMustChange'][0])) $return->smb_pwdmustchange = $attr['pwdMustChange'][0];
		if (isset($attr['homeDrive'][0])) $return->smb_homedrive = $attr['homeDrive'][0];
		if (isset($attr['scriptPath'][0]))  $return->smb_scriptPath = utf8_decode($attr['scriptPath'][0]);
		if (isset($attr['profilePath'][0])) $return->smb_profilePath = $attr['profilePath'][0];
		if (isset($attr['userWorkstations'][0])) $return->smb_smbuserworkstations = $attr['userWorkstations'][0];
		if (isset($attr['domain'][0])) {
			if ($_SESSION['config']->samba3=='yes') {
				$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
				for ($i=0; $i<sizeof($samba3domains); $i++)
					if ($attr['domain'][0] == $samba3domains[$i]->name) $return->smb_domain = $samba3domains[$i];
				}
			else $return->smb_domain = $attr['domain'][0];
			}
		if (isset($attr['primaryGroupID'][0])) {
			if ($_SESSION['config']->samba3=='yes')
				$return->smb_mapgroup = $return->smb_domain->SID. '-' . (2*$attr['primaryGroupID'][0]+1);
				else $return->smb_mapgroup = $attr['primaryGroupID'][0];
			}
		}
	return $return;
	}

function loadhost($dn) { // Will load all needed values from an existing account
	$return = new account();
	$return->type='host';
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	$i=0;
	while (isset($attr['objectClass'][$i])) {
		$return->general_objectClass[$i] = $attr['objectClass'][$i];
		$i++;
		}
	if (isset($attr['uid'][0])) $return->general_username = $attr['uid'][0];
	if (isset($attr['uidNumber'][0])) $return->general_uidNumber = $attr['uidNumber'][0];
	if (isset($attr['gecos'][0])) $return->general_gecos = utf8_decode($attr['gecos'][0]);
	if (isset($attr['displayName'][0])) $return->smb_displayName = utf8_decode($attr['displayName'][0]);
	if (isset($attr['userPassword'][0])) $return->unix_password = $attr['userPassword'][0];

	// Get Groupname
	if (isset($attr['gidNumber'][0])) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('gidNumber', 'cn'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['gidNumber'][0]==$attr['gidNumber'][0]) $return->general_group = $attr2['cn'][0];
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}

	if (isset($attr['displayName'][0])) $return->smb_displayName = utf8_decode($attr['displayName'][0]);

	// load samba3 attributes
	if (in_array('sambaSamAccount', $attr['objectClass'])) {
		if (isset($attr['sambaAcctFlags'][0])) {
			if (strrpos($attr['sambaAcctFlags'][0], 'W')) $return->smb_flagsW=true;
			if (strrpos($attr['sambaAcctFlags'][0], 'D')) $return->smb_flagsD=true;
			if (strrpos($attr['sambaAcctFlags'][0], 'X')) $return->smb_flagsX=true;
			}
		if (isset($attr['sambaDomainName'][0])) {
			if ($_SESSION['config']->samba3=='yes') {
				$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
				for ($i=0; $i<sizeof($samba3domains); $i++)
					if ($attr['sambaDomainName'][0] == $samba3domains[$i]->name) $return->smb_domain = $samba3domains[$i];
				}
			else {
				$return->smb_domain = $attr['sambaDomainName'];
				}
			}
		if (isset($attr['sambaPrimaryGroupSID'][0])) {
			if ($_SESSION['config']->samba3=='yes')
				$return->smb_mapgroup = $attr['sambaPrimaryGroupSID'][0];
				else $return->smb_mapgroup = 2*$attr['gidNumber'][0]+1001;
			}
		// return value to prevent loaded values to be overwritten from old samba 2.2 attributes
		if ($_SESSION['config']->is_samba3()) return $return;
		}
	// load samba 2.2 attributes
	if (in_array('sambaAccount', $attr['objectClass'])) {
		if (isset($attr['acctFlags'][0])) {
			if (strrpos($attr['acctFlags'][0], 'W')) $return->smb_flagsW=true;
			if (strrpos($attr['acctFlags'][0], 'D')) $return->smb_flagsD=true;
			if (strrpos($attr['acctFlags'][0], 'X')) $return->smb_flagsX=true;
			}
		if (isset($attr['domain'][0])) {
			if ($_SESSION['config']->samba3=='yes') {
				$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
				for ($i=0; $i<sizeof($samba3domains); $i++)
					if ($attr['domain'][0] == $samba3domains[$i]->name) $return->smb_domain = $samba3domains[$i];
				}
			else $return->smb_domain = $attr['domain'][0];
			}
		if (isset($attr['primaryGroupID'][0])) {
			if ($_SESSION['config']->samba3=='yes')
				$return->smb_mapgroup = $return->smb_domain->SID. '-' . (2*$attr['primaryGroupID'][0]+1);
				else $return->smb_mapgroup = $attr['primaryGroupID'][0];
			}
		}
	return $return;
	}


function loadgroup($dn) { // Will load all needed values from an existing group
	$return = new account();
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixGroup");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	// Load values into account object
	$i=0;
	while (isset($attr['objectClass'][$i])) {
		$return->general_objectClass[$i] = $attr['objectClass'][$i];
		$i++;
		}
	$i=0;
	while (isset($attr['memberUid'][$i])) {
		$return->unix_memberUid[$i] = $attr['memberUid'][$i];
		$i++;
		}
	if (isset($attr['gidNumber'][0])) $return->general_uidNumber = $attr['gidNumber'][0];
	if (isset($attr['description'][0])) $return->general_gecos = utf8_decode($attr['description'][0]);
	if (isset($attr['cn'][0])) $return->general_username = $attr['cn'][0];
	if (isset($attr['sambaSID'][0])) { // Samba3 Samba 2.0 don't have any objects 4 groups
		$return->smb_mapgroup = $attr['sambaSID'][0];
		if (isset($attr['displayName'][0])) $return->smb_displayName = utf8_decode($attr['displayName'][0]);
		// extract SID from sambaSID to find domain
		$temp = explode('-', $attr['sambaSID'][0]);
		$SID = $temp[0].'-'.$temp[1].'-'.$temp[2].'-'.$temp[3].'-'.$temp[4].'-'.$temp[5].'-'.$temp[6];
		$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
		for ($i=0; $i<sizeof($samba3domains); $i++)
			if ($SID == $samba3domains[$i]->SID) $return->smb_domain = $samba3domains[$i];
		}
	$return->type='group';
	return $return;
	}


function createuser($values) { // Will create the LDAP-Account
	// 2 == Account already exists at different location
	// 1 == Account has been created
	// 4 == Error while creating Account
	// values stored in shadowExpire, days since 1.1.1970
	if ($values->unix_pwdexpire) {
		$date = $values->unix_pwdexpire / 86400 ;
		settype($date, 'integer');
		}

	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;

	// decrypt password
	$iv = base64_decode($_COOKIE["IV"]);
	$key = base64_decode($_COOKIE["Key"]);
	if ($values->unix_password != '') {
		$values->unix_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->unix_password), MCRYPT_MODE_ECB, $iv);
		$values->unix_password = str_replace(chr(00), '', $values->unix_password);
		}
	if ($values->smb_password != '') {
		$values->smb_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->smb_password), MCRYPT_MODE_ECB, $iv);
		$values->smb_password = str_replace(chr(00), '', $values->smb_password);
		}

	// All Values need for an user-account
	// General Objectclasses
	$attr['objectClass'][0] = 'posixAccount';
	$attr['objectClass'][1] = 'shadowAccount';
	if ($_SESSION['config']->is_samba3()) {
		$attr['objectClass'][2] = 'sambaSamAccount';
		if ($values->smb_password_no) {
			$attr['sambaNTPassword'] = 'NO PASSWORD*****';
			$attr['sambaLMPassword'] = 'NO PASSWORD*****';
			$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
			}
		else {
			if (file_exists($_SESSION['lampath'].'lib/createntlm.pl')) { // masscreate.php is at a different relative path
				$attr['sambaNTPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl nt ' . $values->smb_password);
				$attr['sambaLMPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl lm ' . $values->smb_password);
				}
			$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
			}
		$attr['sambaSID'] = $values->smb_domain->SID . "-"  . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
		if ($values->smb_mapgroup!='') $attr['sambaPrimaryGroupSID'] = $values->smb_mapgroup; // sambaAccount_req
		if ($values->smb_pwdcanchange!='') $attr['sambaPwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
			else $attr['sambaPwdCanChange'] = time(); // sambaAccount_may
		if ($values->smb_pwdmustchange!='') $attr['sambaPwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
			else $attr['sambaPwdMustChange'] = time() + 1000000000; // sambaAccount_may
		$attr['sambaAcctFlags'] = smbflag($values); // sambaAccount_may
		$attr['displayName'] = $values->general_gecos; // sambaAccount_may
		if ($values->smb_smbhome!='') $attr['sambaHomePath'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if ($values->smb_homedrive!='') $attr['sambaHomeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if ($values->smb_scriptPath!='') $attr['sambaLogonScript']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if ($values->smb_profilePath!='') $attr['sambaProfilePath'] = $values->smb_profilePath; // sambaAccount_may
		if ($values->smb_smbuserworkstations!='') $attr['sambaUserWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if ($values->smb_domain!='') $attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		}
	else {
		$attr['objectClass'][2] = 'sambaAccount';
		if ($values->smb_password_no) {
			$attr['ntPassword'] = 'NO PASSWORD*****';
			$attr['lmPassword'] = 'NO PASSWORD*****';
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
		else {
			$attr['ntPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl nt ' . $values->smb_password);
			$attr['lmPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl lm ' . $values->smb_password);
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		$attr['primaryGroupID'] = $values->smb_mapgroup; // sambaAccount_req
		if ($values->smb_pwdcanchange!='') $attr['pwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
			else $attr['pwdCanChange'] = time(); // sambaAccount_may
		if ($values->smb_pwdmustchange!='') $attr['pwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
			else $attr['pwdMustChange'] = time() + 1000000000; // sambaAccount_may
		$attr['pwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
		$attr['acctFlags'] = smbflag(values); // sambaAccount_may
		$attr['displayName'] = $values->general_gecos; // sambaAccount_may
		if ($values->smb_smbhome!='') $attr['smbHome'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if ($values->smb_homedrive!='') $attr['homeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if ($values->smb_scriptPath!='') $attr['scriptPath']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if ($values->smb_profilePath!='') $attr['profilePath'] = $values->smb_profilePath; // sambaAccount_may
		if ($values->smb_smbuserworkstations!='') $attr['userWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
		}
	$attr['objectClass'][3] = 'inetOrgPerson';

	$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $values->general_username; // posixAccount_req
	$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
	$attr['homeDirectory'] = $values->general_homedir; // posixAccount_req
	if ($values->personal_title!='') $attr['title'] = utf8_encode($values->personal_title);
	if ($values->personal_mail!='') $attr['mail'] = utf8_encode($values->personal_mail);
	if ($values->personal_telephoneNumber!='') $attr['telephoneNumber'] = utf8_encode($values->personal_telephoneNumber);
	if ($values->personal_mobileTelephoneNumber!='') $attr['mobilemobileTelephoneNumber'] = utf8_encode($values->personal_mobileTelephoneNumber);
	if ($values->personal_facsimileTelephoneNumber!='') $attr['facsimileTelephoneNumber'] = utf8_encode($values->personal_facsimileTelephoneNumber);
	if ($values->personal_street!='') $attr['street'] = utf8_encode($values->personal_street);
	if ($values->personal_postalCode!='') $attr['postalCode'] = utf8_encode($values->personal_postalCode);
	if ($values->personal_postalAddress!='') $attr['postalAddress'] = utf8_encode($values->personal_postalAddress);
	if ($values->personal_employeeType!='') $attr['employeeType'] = utf8_encode($values->personal_employeeType);
	// posixAccount_may shadowAccount_may
	if ($values->unix_password_no) $values->unix_password = '';
	if ($values->unix_deactivated) $attr['userPassword'] = pwd_hash($values->unix_password, false);
	    else $attr['userPassword'] = pwd_hash($values->unix_password);
	$attr['shadowLastChange'] = getdays(); // shadowAccount_may
	$attr['loginShell'] = $values->general_shell; // posixAccount_may
	$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos)); // posixAccount_may
	$attr['description'] = utf8_encode($values->general_gecos); // posixAccount_may sambaAccount_may
	$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may

	$values->unix_host = str_replace(' ', '', $values->unix_host);
	$hosts = explode (',', $values->unix_host);
	$i=0;
	while(isset($hosts[$i])) {
		if ($hosts[$i]!='') $attr['host'][$i] = $hosts[$i];
		$i++;
		}
	if ($values->unix_pwdminage!='') $attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	if ($values->unix_pwdmaxage!='') $attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	if ($values->unix_pwdwarn!='') $attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	if ($values->unix_pwdallowlogin!='') $attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	if ($date) $attr['shadowExpire'] =  $date ; // shadowAccount_may

	if ($values->general_givenname!='') $attr['givenName'] = utf8_encode($values->general_givenname);
	if ($values->general_surname!='') $attr['sn'] = utf8_encode($values->general_surname);

	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	if (!$success) return 4;
	if ($_SESSION['config']->scriptServer) {
		setquotas($values,'user');
		addhomedir($values->general_username);
		}
	// Add User to Additional Groups
	if ($values->general_groupadd[0])
		foreach ($values->general_groupadd as $group2) {
			$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "(&(objectclass=posixGroup)(cn=$group2))", array('memberUid'));
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
			if ($group['memberUid']) array_shift($group['memberUid']);
			if (! @in_array($values->general_username, $group['memberUid'])) {
				$toadd['memberUid'] = $values->general_username;
				$success = ldap_mod_add($_SESSION['ldap']->server(), $dn, $toadd);
				}
			if (!$success) return 4;
		}
	if ((isset($_SESSION['userDN']))) {
		$_SESSION['userDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['userDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	return 1;
	}

function modifyuser($values,$values_old) { // Will modify the LDAP-Account
	// 2 == Account already exists at different location
	// 3 == Account has been modified
	// 5 == Error while modifying Account
	// Value stored in shadowExpire, days since 1.1.1970
	// decrypt password
	$iv = base64_decode($_COOKIE["IV"]);
	$key = base64_decode($_COOKIE["Key"]);
	if ($values->unix_pwdexpire) {
		$date = $values->unix_pwdexpire / 86400 ;
		settype($date, 'integer');
		}
	if ($values_old->unix_pwdexpire) {
		$date_old = $values_old->unix_pwdexpire / 86400 ;
		settype($date_old, 'integer');
		}
	if ($values->unix_password != '') {
		$values->unix_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->unix_password), MCRYPT_MODE_ECB, $iv);
		$values->unix_password = str_replace(chr(00), '', $values->unix_password);
		}
	if ($values->smb_password != '') {
		$values->smb_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->smb_password), MCRYPT_MODE_ECB, $iv);
		$values->smb_password = str_replace(chr(00), '', $values->smb_password);
		}
	if ($values->unix_pwdexpire_mon) {
		$date = mktime(10,0,0, $values->unix_pwdexpire_mon, $values->unix_pwdexpire_day, $values->unix_pwdexpire_yea) / 86400 ;
		settype($date, 'integer');
		}
	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;
	if ($values->general_username != $values_old->general_username) {
		$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
		$attr['uid'] = $values->general_username; // posixAccount_req
		}
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
		if ($_SESSION['config']->is_samba3()) $attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
			else $attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		}
	if ($values->general_group != $values_old->general_group) {
		$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
		$change = false;
		if ($_SESSION['config']->is_samba3()) {
			if ($values->smb_mapgroup==$_SESSION['account']->smb_domain->SID . '-512') $found=true;
			if ($values->smb_mapgroup==$_SESSION['account']->smb_domain->SID . '-513') $found=true;
			if ($values->smb_mapgroup==$_SESSION['account']->smb_domain->SID . '-514') $found=true;
			if (!$found) $attr['sambaPrimaryGroupSID'] = $_SESSION['account']->smb_domain->SID . "-".
				(2 * getgid($_SESSION['account']->general_group) + $values->smb_domain->RIDbase+1);
			}
		else {
			if ($values->smb_mapgroup== '512') $found=true;
			if ($values->smb_mapgroup== '513') $found=true;
			if ($values->smb_mapgroup== '514') $found=true;
			if (!$found) $attr['primaryGroupID'] = (2 * getgid($_SESSION['account']->general_group) + 1001);
			}
		}

	if ($values->general_homedir != $values_old->general_homedir)
		$attr['homeDirectory'] = $values->general_homedir; // posixAccount_req
	// posixAccount_may shadowAccount_may

	// Set new password
	if ($values->unix_password=='') {
		if ($values->unix_deactivated != $values_old->unix_deactivated) {
			$i = 0;
			while ($values_old->unix_password{$i} != '}') $i++;
			$passwd = substr($values_old->unix_password, $i+1 );
			$crypt = substr($values_old->unix_password, 0, $i+1 );
			if ($passwd{0} == '!') $passwd = substr($passwd, 1);
			if ($values->unix_deactivated) $attr['userPassword'] = $crypt.'!'.$passwd;
				else $attr['userPassword'] = $crypt.$passwd;
			}
		if ($values->unix_password_no) {
			if ($values->unix_deactivated) $attr['userPassword'] = pwd_hash('', false);
				else $attr['userPassword'] = pwd_hash('');
			$attr['shadowLastChange'] = getdays(); // shadowAccount_may
			}
		}
	else {
		if ($values->unix_password_no) $values->unix_password = '';
		if ($values->unix_deactivated) $attr['userPassword'] = pwd_hash($values->unix_password, false);
		else $attr['userPassword'] = pwd_hash($values->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccount_may
		}

	if ($_SESSION['config']->is_samba3()) {
		if ($values->smb_password_no) {
			$attr['sambaNTPassword'] = 'NO PASSWORD*****';
			$attr['sambaLMPassword'] = 'NO PASSWORD*****';
			$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
			}
		 else
			if ($values->smb_password!='') {
				$attr['sambaNTPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl nt ' . $values->smb_password);
				$attr['sambaLMPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl lm ' . $values->smb_password);
				$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
				}
		if ($values->smb_pwdcanchange != $values_old->smb_pwdcanchange) $attr['sambaPwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
		if ($values->smb_pwdmustchange != $values_old->smb_pwdmustchange) $attr['sambaPwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
		$attr['sambaAcctFlags'] = smbflag($values); // sambaAccount_may
		if (($values->smb_smbhome!='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr['sambaHomePath'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if (($values->smb_smbhome=='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr_rem['sambaHomePath'] = utf8_encode($values_old->smb_smbhome); // sambaAccount_may
		if (($values->smb_homedrive!='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr['sambaHomeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if (($values->smb_homedrive=='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr_rem['sambaHomeDrive'] = $values_old->smb_homedrive; // sambaAccount_may
		if (($values->smb_scriptPath!='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr['sambaLogonScript']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if (($values->smb_scriptPath=='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr_rem['sambaLogonScript']  = utf8_encode($values_old->smb_scriptPath); // sambaAccount_may
		if (($values->smb_profilePath!='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr['sambaProfilePath'] = $values->smb_profilePath; // sambaAccount_may
		if (($values->smb_profilePath=='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr_rem['sambaProfilePath'] = $values_old->smb_profilePath; // sambaAccount_may
		if (($values->smb_smbuserworkstations!='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr['sambaUserWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if (($values->smb_smbuserworkstations=='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr_rem['sambaUserWorkstations'] = $values_old->smb_smbuserworkstations; // sambaAccount_may
		if (($values->smb_domain->name!='') && ($values->smb_domain->name!=$values_old->smb_domain->name)) $attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		if (($values->smb_domain->name=='') && ($values->smb_domain->name!=$values_old->smb_domain->name)) $attr_rem['sambaDomainName'] = $values_old->smb_domain->name; // sambaAccount_may
		if (($values->smb_mapgroup!='') && ($values->smb_mapgroup!=$values_old->smb_mapgroup)) $attr['sambaPrimaryGroupSID'] = $values->smb_mapgroup; // sambaAccount_may
		if (($values->smb_mapgroup=='') && ($values->smb_mapgroup!=$values_old->smb_mapgroup)) $attr_rem['sambaPrimaryGroupSID'] = $values_old->smb_mapgroup;
		if ($values->smb_displayName != $values_old->smb_displayName) $attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		}
	else {
		if ($values->smb_password_no) {
			$attr['ntPassword'] = 'NO PASSWORD*****';
			$attr['lmPassword'] = 'NO PASSWORD*****';
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
		 else
			if ($values->smb_password!='') {
				$attr['ntPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl nt ' . $values->smb_password);
				$attr['lmPassword'] = exec($_SESSION['lampath'].'lib/createntlm.pl lm ' . $values->smb_password);
				$attr['pwdLastSet'] = time(); // sambaAccount_may
				}
		if ($values->smb_pwdcanchange != $values_old->smb_pwdcanchange) $attr['pwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
		if ($values->smb_pwdmustchange != $values_old->smb_pwdmustchange) $attr['pwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
		$attr['acctFlags'] = smbflag($values); // sambaAccount_may
		if (($values->smb_smbhome!='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr['smbHome'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if (($values->smb_smbhome=='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr_rem['smbHome'] = utf8_encode($values_old->smb_smbhome); // sambaAccount_may
		if (($values->smb_homedrive!='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr['homeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if (($values->smb_homedrive=='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr_rem['homeDrive'] = $values_old->smb_homedrive; // sambaAccount_may
		if (($values->smb_scriptPath!='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr['scriptPath']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if (($values->smb_scriptPath=='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr_rem['scriptPath']  = utf8_encode($values_old->smb_scriptPath); // sambaAccount_may
		if (($values->smb_profilePath!='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr['profilePath'] = $values->smb_profilePath; // sambaAccount_may
		if (($values->smb_profilePath=='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr_rem['profilePath'] = $values_old->smb_profilePath; // sambaAccount_may
		if (($values->smb_smbuserworkstations!='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr['userWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if (($values->smb_smbuserworkstations=='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr_rem['userWorkstations'] = $values_old->smb_smbuserworkstations; // sambaAccount_may
		if (($values->smb_domain!='') && ($values->smb_domain!=$values_old->smb_domain)) $attr['domain'] = $values->smb_domain; // sambaAccount_may
		if (($values->smb_domain=='') && ($values->smb_domain!=$values_old->smb_domain)) $attr_rem['domain'] = $values_old->smb_domain; // sambaAccount_may
		if (($values->smb_mapgroup!='') && ($values->smb_mapgroup!=$values_old->smb_mapgroup)) $attr['primaryGroupID'] = $values->smb_mapgroup; // sambaAccount_may
		if (($values->smb_mapgroup=='') && ($values->smb_mapgroup!=$values_old->smb_mapgroup)) $attr_rem['primaryGroupID'] = $values_old->smb_mapgroup;
		if ($values->smb_displayName != $values_old->smb_displayName) $attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		}

	if ($values->general_shell != $values_old->general_shell)
		$attr['loginShell'] = $values->general_shell; // posixAccount_may
	if ($values->general_gecos != $values_old->general_gecos) {
		$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos)); // posixAccount_may
		$attr['description'] = utf8_encode($values->general_gecos); // posixAccount_may sambaAccount_may
		}

	if (($values->unix_host != $values_old->unix_host)) {
		$values->unix_host = str_replace(' ', '', $values->unix_host);
		$host = explode (',', $values->unix_host);
		$values_old->unix_host = str_replace(' ', '', $values_old->unix_host);
		$host_old = explode (',', $values_old->unix_host);
		if ($host[0]=='') $attr_rem['host'] = $host_old;
		 else if ($host[0]!='') $attr['host'] = $host;
		}

	if (($values->unix_pwdminage != $values_old->unix_pwdminage) && ($values->unix_pwdminage !=''))
		$attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	if (($values->unix_pwdminage != $values_old->unix_pwdminage) && ($values->unix_pwdminage ==''))
		$attr_rem['shadowMin'] = $values_old->unix_pwdminage; // shadowAccount_may
	if (($values->unix_pwdmaxage != $values_old->unix_pwdmaxage) && ($values->unix_pwdmaxage !=''))
		$attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	if (($values->unix_pwdmaxage != $values_old->unix_pwdmaxage) && ($values->unix_pwdmaxage ==''))
		$attr_rem['shadowMax'] = $values_old->unix_pwdmaxage; // shadowAccount_may
	if (($values->unix_pwdwarn != $values_old->unix_pwdwarn) && ($values->unix_pwdwarn !=''))
		$attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	if (($values->unix_pwdwarn != $values_old->unix_pwdwarn) && ($values->general_pwdwarn ==''))
		$attr_rem['shadowWarning'] = $values_old->unix_pwdwarn; // shadowAccount_may
	if (($values->unix_pwdallowlogin != $values_old->unix_pwdallowlogin) && ($values->unix_pwdallowlogin !=''))
		$attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	if (($values->unix_pwdallowlogin != $values_old->unix_pwdallowlogin) && ($values->unix_pwdallowlogin ==''))
		$attr_rem['shadowInactive'] = $values_old->unix_pwdallowlogin; // shadowAccount_may
	if (($date != $date_old) && $date) $attr['shadowExpire'] =  $date ; // shadowAccount_may
	if (($date != $date_old) && !$date) $attr_rem['shadowExpire'] =  $date_old ; // shadowAccount_may
	if (($values->personal_title != $values_old->personal_title) && ($values->personal_title != ''))
		$attr['title'] = utf8_encode($values->personal_title);
	if (($values->personal_title != $values_old->personal_title) && ($values->personal_title == ''))
		$attr_rem['title'] = utf8_encode($values_old->personal_title);
	if (($values->personal_mail != $values_old->personal_mail) && ($values->personal_mail != ''))
		$attr['mail'] = utf8_encode($values->personal_mail);
	if (($values->personal_mail != $values_old->personal_mail) && ($values->personal_mail == ''))
		$attr_rem['mail'] = utf8_encode($values_old->personal_mail);
	if (($values->personal_telephoneNumber != $values_old->personal_telephoneNumber) && ($values->personal_telephoneNumber !=''))
		$attr['telephoneNumber'] = utf8_encode($values->personal_telephoneNumber);
	if (($values->personal_telephoneNumber != $values_old->personal_telephoneNumber) && ($values->personal_telephoneNumber ==''))
		$attr_rem['telephoneNumber'] = utf8_encode($values_old->personal_telephoneNumber);
	if (($values->personal_mobileTelephoneNumber != $values_old->personal_mobileTelephoneNumber) && ($values->personal_mobileTelephoneNumber!=''))
		$attr['mobileTelephoneNumber'] = utf8_encode($values->personal_mobileTelephoneNumber);
	if (($values->personal_mobileTelephoneNumber != $values_old->personal_mobileTelephoneNumber) && ($values->personal_mobileTelephoneNumber==''))
		$attr_rem['mobilemobileTelephoneNumber'] = utf8_encode($values_old->personal_mobileTelephoneNumber);
	if (($values->personal_facsimileTelephoneNumber != $values_old->personal_facsimileTelephoneNumber) && ($values->personal_facsimileTelephoneNumber!=''))
		$attr['facsimileTelephoneNumber'] = utf8_encode($values->personal_facsimileTelephoneNumber);
	if (($values->personal_facsimileTelephoneNumber != $values_old->personal_facsimileTelephoneNumber) && ($values->personal_facsimileTelephoneNumber==''))
		$attr_rem['facsimileTelephoneNumber'] = utf8_encode($values_old->personal_facsimileTelephoneNumber);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street!=''))
		$attr['street'] = utf8_encode($values->personal_street);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street==''))
		$attr_rem['street'] = utf8_encode($values_old->personal_street);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street!=''))
		$attr['postalCode'] = utf8_encode($values->personal_street);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street==''))
		$attr_rem['postalCode'] = utf8_encode($values_old->personal_street);
	if (($values->personal_postalAddress != $values_old->personal_postalAddress) && ($values->personal_postalAddress!=''))
		$attr['postalAddress'] = utf8_encode($values->personal_postalAddress);
	if (($values->personal_postalAddress != $values_old->personal_postalAddress) && ($values->personal_postalAddress==''))
		$attr_rem['postalAddress'] = utf8_encode($values_old->personal_postalAddress);
	if (($values->personal_employeeType != $values_old->personal_employeeType) && ($values->personal_employeeType!=''))
		$attr['employeeType'] = utf8_encode($values->personal_employeeType);
	if (($values->personal_employeeType != $values_old->personal_employeeType) && ($values->personal_employeeType==''))
		$attr_rem['employeeType'] = utf8_encode($values_old->personal_employeeType);
	if (($values->unix_pwdexpire_day = $date['mday']!=$values_old->unix_pwdexpire_day = $date['mday']) ||
		($values->unix_pwdexpire_mon = $date['mon'] != $values_old->unix_pwdexpire_mon = $date['mon']) ||
		($values->unix_pwdexpire_yea = $date['year'] != $values->unix_pwdexpire_yea = $date['year']))
			$attr['shadowExpire'] =  $date ; // shadowAccount_may
	if ($values->general_givenname!=$values_old->general_givenname) $attr['givenName'] = utf8_encode($values->general_givenname);
	if ($values->general_surname!=$values_old->general_surname) $attr['sn'] = utf8_encode($values->general_surname);

	// Add missing objectclasses to group
	if (!in_array('posixAccount', $values->general_objectClass)) {
		$attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'posixAccount';
		}
	if (!in_array('shadowAccount', $values->general_objectClass)) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'shadowAccount';
		}


	// Add or convert samba attributes & object to samba 3
	if (($_SESSION['config']->is_samba3()) && (!in_array('sambaSamAccount', $values->general_objectClass))) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'sambaSamAccount';
		// unset old sambaAccount objectClass
		for ($i=0; $i<count($attr['objectClass']); $i++)
			if ($attr['objectClass'][$i]=='sambaAccount') unset($attr['objectClass'][$i]);
		$attr['objectClass'] = array_values($attr['objectClass']);
		// Set correct values for new objectclass
		// Load old samba-values not stored in account object
		$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if (isset($attr_old['lmPassword'][0])) $attr['sambaLMPassword'] = $attr_old['lmPassword'][0];
		if (isset($attr_old['ntPassword'][0])) $attr['sambaNTPassword'] = $attr_old['ntPassword'][0];
		if (isset($attr_old['pwdLastSet'][0])) $attr['sambaPwdLastSet'] = $attr_old['pwdLastSet'][0];
		if (isset($attr_old['logonTime'][0])) $attr['sambaLogonTime'] = $attr_old['logonTime'][0];
		if (isset($attr_old['logoffTime'][0])) $attr['sambaLogoffTime'] = $attr_old['logoffTime'][0];
		if (isset($attr_old['kickoffTime'][0])) $attr['sambaKickoffTime'] = $attr_old['kickoffTime'][0];
		if (isset($attr_old['pwdCanChange'][0])) $attr['sambaPwdCanChange'] = $attr_old['pwdCanChange'][0];
		if (isset($attr_old['pwdMustChange'][0])) $attr['sambaPwdMustChange'] = $attr_old['pwdMustChange'][0];
		if (isset($attr_old['smbHome'][0])) $attr['sambaHomePath'] = $attr_old['smbHome'][0];
		if (isset($attr_old['homeDrive'][0])) $attr['sambaHomeDrive'] = $attr_old['homeDrive'][0];
		if (isset($attr_old['scriptPath'][0])) $attr['sambaLogonScript'] = $attr_old['scriptPath'][0];
		if (isset($attr_old['profilePath'][0])) $attr['sambaProfilePath'] = $attr_old['profilePath'][0];
		if (isset($attr_old['userWorkstations'][0])) $attr['sambaUserWorkstations'] = $attr_old['userWorkstations'][0];
		// Values used from account object
		$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		$attr['sambaAcctFlags'] = smbflag($values); // sambaAccount_may
		$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
		$attr['sambaPrimaryGroupSID'] = $values->smb_mapgroup; // sambaAccount_req
		// remove old attributes
		if (in_array('sambaAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaAccount';
		if (isset($attr_old['lmPassword'][0])) $attr_rem['lmPassword'] = $attr_old['lmPassword'][0];
		if (isset($attr_old['ntPassword'][0])) $attr_rem['ntPassword'] = $attr_old['ntPassword'][0];
		if (isset($attr_old['pwdLastSet'][0])) $attr_rem['pwdLastSet'] = $attr_old['pwdLastSet'][0];
		if (isset($attr_old['logonTime'][0])) $attr_rem['logonTime'] = $attr_old['logonTime'][0];
		if (isset($attr_old['kickoffTime'][0])) $attr_rem['kickoffTime'] = $attr_old['kickoffTime'][0];
		if (isset($attr_old['pwdCanChange'][0])) $attr_rem['pwdCanChange'] = $attr_old['pwdCanChange'][0];
		if (isset($attr_old['pwdMustChange'][0])) $attr_rem['pwdMustChange'] = $attr_old['pwdMustChange'][0];
		if (isset($attr_old['smbHome'][0])) $attr_rem['smbHome'] = $attr_old['smbHome'][0];
		if (isset($attr_old['acctFlags'][0])) $attr_rem['acctFlags'] = $attr_old['acctFlags'][0];
		if (isset($attr_old['homeDrive'][0])) $attr_rem['homeDrive'] = $attr_old['homeDrive'][0];
		if (isset($attr_old['scriptPath'][0])) $attr_rem['scriptPath'] = $attr_old['scriptPath'][0];
		if (isset($attr_old['profilePath'][0])) $attr_rem['profilePath'] = $attr_old['profilePath'][0];
		if (isset($attr_old['userWorkstations'][0])) $attr_rem['userWorkstations'] = $attr_old['userWorkstations'][0];
		if (isset($attr_old['primaryGroupID'][0])) $attr_rem['primaryGroupID'] = $attr_old['primaryGroupID'][0];
		if (isset($attr_old['domain'][0])) $attr_rem['domain'] = $attr_old['domain'][0];
		if (isset($attr_old['rid'][0])) $attr_rem['rid'] = $attr_old['rid'][0];
		}

	// Add or convert samba attributes & object to samba 2.2
	if (($_SESSION['config']->samba3 == 'no') && (!in_array('sambaAccount', $values->general_objectClass))) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'sambaAccount';
		// unset old sambaAccount objectClass
		for ($i=0; $i<count($attr['objectClass']); $i++)
			if ($attr['objectClass'][$i]=='sambaSamAccount') unset($attr['objectClass'][$i]);
		$attr['objectClass'] = array_values($attr['objectClass']);
		// Set correct values for new objectclass
		// Load old samba-values not stored in account object
		$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if (isset($attr_old['sambaLMPassword'][0])) $attr['lmPassword'] = $attr_old['sambaLMPassword'][0];
		if (isset($attr_old['sambaNTPassword'][0])) $attr['ntPassword'] = $attr_old['sambaNTPassword'][0];
		if (isset($attr_old['sambaPwdLastSet'][0])) $attr['pwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
		if (isset($attr_old['sambaLogonTime'][0])) $attr['logonTime'] = $attr_old['sambaLogonTime'][0];
		if (isset($attr_old['sambaLogoffTime'][0])) $attr['logoffTime'] = $attr_old['sambaLogoffTime'][0];
		if (isset($attr_old['sambaKickoffTime'][0])) $attr['kickoffTime'] = $attr_old['sambaKickoffTime'][0];
		if (isset($attr_old['sambaPwdCanChange'][0])) $attr['pwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
		if (isset($attr_old['sambaPwdMustChange'][0])) $attr['pwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
		if (isset($attr_old['sambaHomePath'][0])) $attr['smbHome'] = $attr_old['sambaHomePath'][0];
		if (isset($attr_old['sambaHomeDrive'][0])) $attr['homeDrive'] = $attr_old['sambaHomeDrive'][0];
		if (isset($attr_old['sambaLogonScript'][0])) $attr['scriptPath'] = $attr_old['sambaLogonScript'][0];
		if (isset($attr_old['sambaProfilePath'][0])) $attr['profilePath'] = $attr_old['sambaProfilePath'][0];
		if (isset($attr_old['sambaUserWorkstations'][0])) $attr['userWorkstations'] = $attr_old['sambaUserWorkstations'][0];
		// Values used from account object
		$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		$attr['acctFlags'] = smbflag($values); // sambaAccount_may
		if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
		$attr['primaryGroupID'] = $values->smb_mapgroup; // sambaAccount_req
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may

		// remove old attributes
		if (in_array('sambaSamAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaSamAccount';
		if (isset($attr_old['sambaLMPassword'][0])) $attr_rem['sambaLMPassword'] = $attr_old['sambaLMPassword'][0];
		if (isset($attr_old['sambaNTPassword'][0])) $attr_rem['sambaNTPassword'] = $attr_old['sambaNTPassword'][0];
		if (isset($attr_old['sambaPwdLastSet'][0])) $attr_rem['sambaPwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
		if (isset($attr_old['sambaLogonTime'][0])) $attr_rem['sambaLogonTime'] = $attr_old['sambaLogonTime'][0];
		if (isset($attr_old['sambaKickoffTime'][0])) $attr_rem['sambaKickoffTime'] = $attr_old['sambaKickoffTime'][0];
		if (isset($attr_old['sambaPwdCanChange'][0])) $attr_rem['sambaPwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
		if (isset($attr_old['sambaPwdMustChange'][0])) $attr_rem['sambaPwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
		if (isset($attr_old['sambaHomePath'][0])) $attr_rem['sambaHomePath'] = $attr_old['sambaHomePAth'][0];
		if (isset($attr_old['sambaAcctFlags'][0])) $attr_rem['sambaAcctFlags'] = $attr_old['sambaAcctFlags'][0];
		if (isset($attr_old['sambaHomeDrive'][0])) $attr_rem['sambaHomeDrive'] = $attr_old['sambaHomeDrive'][0];
		if (isset($attr_old['sambaLogonScript'][0])) $attr_rem['sambaLogonScript'] = $attr_old['sambaLogonScript'][0];
		if (isset($attr_old['sambaProfilePath'][0])) $attr_rem['sambaProfilePath'] = $attr_old['sambaProfilePath'][0];
		if (isset($attr_old['sambaUserWorkstations'][0])) $attr_rem['sambaUserWorkstations'] = $attr_old['sambaUserWorkstations'][0];
		if (isset($attr_old['sambaPrimaryGroupID'][0])) $attr_rem['sambaPrimaryGroupID'] = $attr_old['sambaPrimaryGroupID'][0];
		if (isset($attr_old['sambaDomainName'][0])) $attr_rem['sambaDomainName'] = $attr_old['sambaDomainName'][0];
		if (isset($attr_old['sambaSID'][0])) $attr_rem['sambaSID'] = $attr_old['sambaSID'][0];
		}



	if ($attr_rem) {
			$success = ldap_mod_del($_SESSION['ldap']->server(),$values_old->general_dn, $attr_rem);
			if (!$success) return 5;
			}
	if ($attr) {
		$success = ldap_modify($_SESSION['ldap']->server(),$values_old->general_dn, $attr);
			if (!$success) return 5;
			}
	if ($values->general_dn != $values_old->general_dn) { // Username hasn't changed
				$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
				// remove "count" from array
				unset($attr_old['count']);
				for ($i=0; $i < sizeof($attr_old); $i++) unset($attr_old[$i]);
				$keys = array_keys($attr_old);
				for ($i=0; $i < sizeof($keys); $i++)
					unset($attr_old[$keys[$i]]['count']);
				$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
				if ($success) $success = ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
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
	if ($_SESSION['config']->scriptServer) setquotas($values,'user',$values_old);
	if ((isset($_SESSION['userDN']))) {
		if ($values->general_dn != $values_old->general_dn) {
			unset ($_SESSION['userDN'][$values_old->general_dn]);
			}
		$_SESSION['userDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['userDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	return 3;
	}



function createhost($values) { // Will create the LDAP-Account
	// 2 == Account already exists at different location
	// 1 == Account has been created
	// 3 == Account has been modified
	// 4 == Error while creating Account
	// 5 == Error while modifying Account
	// Value stored in shadowExpire, days since 1.1.1970

	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;

	// All Values need for an host-account
	// General Objectclasses
	$attr['objectClass'][0] = 'posixAccount';
	$attr['objectClass'][1] = 'shadowAccount';

	if ($_SESSION['config']->is_samba3()) {
		$attr['objectClass'][2] = 'sambaSamAccount';
		$attr['sambaNTPassword'] = 'NO PASSWORD*****';
		$attr['sambaLMPassword'] = 'NO PASSWORD*****';
		$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
		$attr['sambaSID'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
		$attr['sambaPrimaryGroupSID'] = $values->smb_domain->SID . "-" . (2 * getgid($values->general_group) + $values->smb_domain->RIDbase +1); // sambaAccount_req
		$attr['sambaPwdCanChange'] = time(); // sambaAccount_may
		$attr['sambaPwdMustChange'] = "1893452400"; // sambaAccount_may // anywhere in year 2030
		$attr['sambaAcctFlags'] = smbflag($values); // sambaAccount_may
		$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		}
	else {
		$attr['objectClass'][2] = 'sambaAccount';
		$attr['ntPassword'] = 'NO PASSWORD*****';
		$attr['lmPassword'] = 'NO PASSWORD*****';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		$attr['primaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
		$attr['pwdCanChange'] = time(); // sambaAccount_may
		$attr['pwdMustChange'] = "1893452400"; // sambaAccount_may // anywhere in 2030
		$attr['acctFlags'] = smbflag($values); // sambaAccount_may
		$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
		}
	$attr['objectClass'][3] = 'account';
	$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $values->general_username; // posixAccount_req
	$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
	$attr['homeDirectory'] = $values->general_homedir; // posixAccount_req

	if ($values->smb_flagsD) $attr['userPassword'] = pwd_hash('', false);
	else $attr['userPassword'] = pwd_hash('');

	$attr['shadowLastChange'] = getdays(); // shadowAccount_may
	$attr['loginShell'] = $values->general_shell; // posixAccount_may
	$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos)); // posixAccount_may
	$attr['description'] = utf8_encode($values->general_gecos); // posixAccount_may sambaAccount_may
	if ($date!='') $attr['shadowExpire'] =  $date ; // shadowAccount_may
	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	if (!$success) return 4;
	if ((isset($_SESSION['hostDN']))) {
		$_SESSION['hostDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['hostDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	return 1;
	}

function modifyhost($values,$values_old) { // Will modify the LDAP-Account
	// 2 == Account already exists at different location
	// 3 == Account has been modified
	// 5 == Error while modifying Account
	// Value stored in shadowExpire, days since 1.1.1970
	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;
	if ($values->general_username != $values_old->general_username) {
		$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
		$attr['uid'] = $values->general_username; // posixAccount_req
		}
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
		if ($_SESSION['config']->is_samba3()) $attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
			else $attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		}
	if ($values->general_group != $values_old->general_group) {
		$attr['gidNumber'] = getgid($values->general_group); // posixAccount_req
		$change = false;
		if ($_SESSION['config']->is_samba3()) {
			if ($values->smb_mapgroup==$_SESSION['account']->smb_domain->SID . '-512') $found=true;
			if ($values->smb_mapgroup==$_SESSION['account']->smb_domain->SID . '-513') $found=true;
			if ($values->smb_mapgroup==$_SESSION['account']->smb_domain->SID . '-514') $found=true;
			if (!$found) $attr['sambaPrimaryGroupSID'] = $_SESSION['account']->smb_domain->SID . "-".
				(2 * getgid($_SESSION['account']->general_group) + $values->smb_domain->RIDbase+1);
			}
		else {
			if ($values->smb_mapgroup== '512') $found=true;
			if ($values->smb_mapgroup== '513') $found=true;
			if ($values->smb_mapgroup== '514') $found=true;
			if (!$found) $attr['primaryGroupID'] = (2 * getgid($_SESSION['account']->general_group) + 1001);
			}
		}



	if ($values->smb_flagsD != $values_old->smb_flagsD) {
		$i = 0;
		while ($values_old->unix_password{$i} != '}') $i++;
		$passwd = substr($values_old->unix_password, $i+1 );
		$crypt = substr($values_old->unix_password, 0, $i+1 );
		if ($passwd{0} == '!') $passwd = substr($passwd, 1);
		if ($values->smb_flagsD ) $attr['userPassword'] = $crypt.'!'.$passwd;
			else $attr['userPassword'] = $crypt.$passwd;
		}
	if ($values->smb_password_no) {
		if ($values->smb_flagsD) $attr['userPassword'] = pwd_hash('', false);
			else $attr['userPassword'] = pwd_hash('');
		$attr['shadowLastChange'] = getdays(); // shadowAccount_may
		}

	// Add missing objectclasses to group
	if (!in_array('posixAccount', $values->general_objectClass)) {
		$attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'posixAccount';
		}
	if (!in_array('shadowAccount', $values->general_objectClass)) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'shadowAccount';
		}

	if ($values->smb_displayName != $values_old->smb_displayName)
		$attr['displayName'] = utf8_encode($values->smb_displayName);

	// Add or convert samba attributes & object to samba 3
	if (($_SESSION['config']->is_samba3()) && (!in_array('sambaSamAccount', $values->general_objectClass))) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'sambaSamAccount';
		// unset old sambaAccount objectClass
		for ($i=0; $i<count($attr['objectClass']); $i++)
			if ($attr['objectClass'][$i]=='sambaAccount') unset($attr['objectClass'][$i]);
		$attr['objectClass'] = array_values($attr['objectClass']);
		// Set correct values for new objectclass
		// Load old samba-values not stored in account object
		$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if (isset($attr_old['lmPassword'][0])) $attr['sambaLMPassword'] = $attr_old['lmPassword'][0];
		if (isset($attr_old['ntPassword'][0])) $attr['sambaNTPassword'] = $attr_old['ntPassword'][0];
		if (isset($attr_old['pwdLastSet'][0])) $attr['sambaPwdLastSet'] = $attr_old['pwdLastSet'][0];
		if (isset($attr_old['logonTime'][0])) $attr['sambaLogonTime'] = $attr_old['logonTime'][0];
		if (isset($attr_old['logoffTime'][0])) $attr['sambaLogoffTime'] = $attr_old['logoffTime'][0];
		if (isset($attr_old['kickoffTime'][0])) $attr['sambaKickoffTime'] = $attr_old['kickoffTime'][0];
		if (isset($attr_old['pwdCanChange'][0])) $attr['sambaPwdCanChange'] = $attr_old['pwdCanChange'][0];
		if (isset($attr_old['pwdMustChange'][0])) $attr['sambaPwdMustChange'] = $attr_old['pwdMustChange'][0];
		if (isset($attr_old['smbHome'][0])) $attr['sambaHomePath'] = $attr_old['smbHome'][0];
		if (isset($attr_old['homeDrive'][0])) $attr['sambaHomeDrive'] = $attr_old['homeDrive'][0];
		if (isset($attr_old['scriptPath'][0])) $attr['sambaLogonScript'] = $attr_old['scriptPath'][0];
		if (isset($attr_old['profilePath'][0])) $attr['sambaProfilePath'] = $attr_old['profilePath'][0];
		if (isset($attr_old['userWorkstations'][0])) $attr['sambaUserWorkstations'] = $attr_old['userWorkstations'][0];
		// Values used from account object
		$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		$attr['sambaAcctFlags'] = smbflag($values); // sambaAccount_may
		$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
		$attr['sambaPrimaryGroupSID'] = $values->smb_domain->SID . "-" . (2 * getgid($values->general_group) + $values->smb_domain->RIDbase +1); // sambaAccount_req
		// remove old attributes
		if (in_array('sambaAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaAccount';
		if (isset($attr_old['lmPassword'][0])) $attr_rem['lmPassword'] = $attr_old['lmPassword'][0];
		if (isset($attr_old['ntPassword'][0])) $attr_rem['ntPassword'] = $attr_old['ntPassword'][0];
		if (isset($attr_old['pwdLastSet'][0])) $attr_rem['pwdLastSet'] = $attr_old['pwdLastSet'][0];
		if (isset($attr_old['logonTime'][0])) $attr_rem['logonTime'] = $attr_old['logonTime'][0];
		if (isset($attr_old['kickoffTime'][0])) $attr_rem['kickoffTime'] = $attr_old['kickoffTime'][0];
		if (isset($attr_old['pwdCanChange'][0])) $attr_rem['pwdCanChange'] = $attr_old['pwdCanChange'][0];
		if (isset($attr_old['pwdMustChange'][0])) $attr_rem['pwdMustChange'] = $attr_old['pwdMustChange'][0];
		if (isset($attr_old['smbHome'][0])) $attr_rem['smbHome'] = $attr_old['smbHome'][0];
		if (isset($attr_old['acctFlags'][0])) $attr_rem['acctFlags'] = $attr_old['acctFlags'][0];
		if (isset($attr_old['homeDrive'][0])) $attr_rem['homeDrive'] = $attr_old['homeDrive'][0];
		if (isset($attr_old['scriptPath'][0])) $attr_rem['scriptPath'] = $attr_old['scriptPath'][0];
		if (isset($attr_old['profilePath'][0])) $attr_rem['profilePath'] = $attr_old['profilePath'][0];
		if (isset($attr_old['userWorkstations'][0])) $attr_rem['userWorkstations'] = $attr_old['userWorkstations'][0];
		if (isset($attr_old['primaryGroupID'][0])) $attr_rem['primaryGroupID'] = $attr_old['primaryGroupID'][0];
		if (isset($attr_old['domain'][0])) $attr_rem['domain'] = $attr_old['domain'][0];
		if (isset($attr_old['rid'][0])) $attr_rem['rid'] = $attr_old['rid'][0];
		}

	// Add or convert samba attributes & object to samba 2.2
	if (($_SESSION['config']->samba3 == 'no') && (!in_array('sambaAccount', $values->general_objectClass))) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'sambaAccount';
		// unset old sambaAccount objectClass
		for ($i=0; $i<count($attr['objectClass']); $i++)
			if ($attr['objectClass'][$i]=='sambaSamAccount') unset($attr['objectClass'][$i]);
		$attr['objectClass'] = array_values($attr['objectClass']);
		// Set correct values for new objectclass
		// Load old samba-values not stored in account object
		$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if (isset($attr_old['sambaLMPassword'][0])) $attr['lmPassword'] = $attr_old['sambaLMPassword'][0];
		if (isset($attr_old['sambaNTPassword'][0])) $attr['ntPassword'] = $attr_old['sambaNTPassword'][0];
		if (isset($attr_old['sambaPwdLastSet'][0])) $attr['pwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
		if (isset($attr_old['sambaLogonTime'][0])) $attr['logonTime'] = $attr_old['sambaLogonTime'][0];
		if (isset($attr_old['sambaLogoffTime'][0])) $attr['logoffTime'] = $attr_old['sambaLogoffTime'][0];
		if (isset($attr_old['sambaKickoffTime'][0])) $attr['kickoffTime'] = $attr_old['sambaKickoffTime'][0];
		if (isset($attr_old['sambaPwdCanChange'][0])) $attr['pwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
		if (isset($attr_old['sambaPwdMustChange'][0])) $attr['pwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
		if (isset($attr_old['sambaHomePath'][0])) $attr['smbHome'] = $attr_old['sambaHomePath'][0];
		if (isset($attr_old['sambaHomeDrive'][0])) $attr['homeDrive'] = $attr_old['sambaHomeDrive'][0];
		if (isset($attr_old['sambaLogonScript'][0])) $attr['scriptPath'] = $attr_old['sambaLogonScript'][0];
		if (isset($attr_old['sambaProfilePath'][0])) $attr['profilePath'] = $attr_old['sambaProfilePath'][0];
		if (isset($attr_old['sambaUserWorkstations'][0])) $attr['userWorkstations'] = $attr_old['sambaUserWorkstations'][0];
		// Values used from account object
		$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		$attr['acctFlags'] = smbflag($values); // sambaAccount_may
		if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
		$attr['primaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may

		// remove old attributes
		if (in_array('sambaSamAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaSamAccount';
		if (isset($attr_old['sambaLMPassword'][0])) $attr_rem['sambaLMPassword'] = $attr_old['sambaLMPassword'][0];
		if (isset($attr_old['sambaNTPassword'][0])) $attr_rem['sambaNTPassword'] = $attr_old['sambaNTPassword'][0];
		if (isset($attr_old['sambaPwdLastSet'][0])) $attr_rem['sambaPwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
		if (isset($attr_old['sambaLogonTime'][0])) $attr_rem['sambaLogonTime'] = $attr_old['sambaLogonTime'][0];
		if (isset($attr_old['sambaKickoffTime'][0])) $attr_rem['sambaKickoffTime'] = $attr_old['sambaKickoffTime'][0];
		if (isset($attr_old['sambaPwdCanChange'][0])) $attr_rem['sambaPwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
		if (isset($attr_old['sambaPwdMustChange'][0])) $attr_rem['sambaPwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
		if (isset($attr_old['sambaHomePath'][0])) $attr_rem['sambaHomePath'] = $attr_old['sambaHomePAth'][0];
		if (isset($attr_old['sambaAcctFlags'][0])) $attr_rem['sambaAcctFlags'] = $attr_old['sambaAcctFlags'][0];
		if (isset($attr_old['sambaHomeDrive'][0])) $attr_rem['sambaHomeDrive'] = $attr_old['sambaHomeDrive'][0];
		if (isset($attr_old['sambaLogonScript'][0])) $attr_rem['sambaLogonScript'] = $attr_old['sambaLogonScript'][0];
		if (isset($attr_old['sambaProfilePath'][0])) $attr_rem['sambaProfilePath'] = $attr_old['sambaProfilePath'][0];
		if (isset($attr_old['sambaUserWorkstations'][0])) $attr_rem['sambaUserWorkstations'] = $attr_old['sambaUserWorkstations'][0];
		if (isset($attr_old['sambaPrimaryGroupID'][0])) $attr_rem['sambaPrimaryGroupID'] = $attr_old['sambaPrimaryGroupID'][0];
		if (isset($attr_old['sambaDomainName'][0])) $attr_rem['sambaDomainName'] = $attr_old['sambaDomainName'][0];
		if (isset($attr_old['sambaSID'][0])) $attr_rem['sambaSID'] = $attr_old['sambaSID'][0];
		}

	if ($_SESSION['config']->is_samba3()) {
		// Reset password
		if ($values->smb_password_no) {
			$attr['sambaNTPassword'] = 'NO PASSWORD*****';
			$attr['sambaLMPassword'] = 'NO PASSWORD*****';
			$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
			$attr['userPassword'] = '';
			$attr['shadowLastChange'] = getdays();
			}
		$attr['sambaAcctFlags'] = smbflag($values); // sambaAccount_may
		if ($values->smb_domain->name!=$values_old->smb_domain->name) $attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		}
	// samba 2.2
	else {
		if ($values->smb_password_no) {
			$attr['ntPassword'] = 'NO PASSWORD*****';
			$attr['lmPassword'] = 'NO PASSWORD*****';
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			$attr['userPassword'] = '';
			$attr['shadowLastChange'] = getdays();
			}
		if (isset($attr_old['sambaSID'][0])) $attr_rem['sambaSID'] = $attr_old['sambaSID'][0];
		$attr['acctFlags'] = smbflag($values); // sambaAccount_may
		if (($values->smb_domain!='') && ($values->smb_domain!=$values_old->smb_domain)) $attr['domain'] = $values->smb_domain; // sambaAccount_may
		if (($values->smb_domain=='') && ($values->smb_domain!=$values_old->smb_domain)) $attr_rem['domain'] = $values_old->smb_domain; // sambaAccount_may
		}

	if ($values->general_gecos != $values_old->general_gecos) {
		$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos)); // posixAccount_may
		$attr['description'] = utf8_encode($values->general_gecos); // posixAccount_may sambaAccount_may
		}


	if ($attr_rem) {
		$success = ldap_mod_del($_SESSION['ldap']->server(),$values_old->general_dn, $attr_rem);
		if (!$success) return 5;
		}
	if ($attr) {
		$success = ldap_modify($_SESSION['ldap']->server(),$values_old->general_dn, $attr);
		if (!$success) return 5;
		}
	if ($values->general_dn != $values_old->general_dn) {// Hostname hasn't changed
		$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		// remove "count" from array
		unset($attr_old['count']);
		for ($i=0; $i < sizeof($attr_old); $i++) unset($attr_old[$i]);
		$keys = array_keys($attr_old);
		for ($i=0; $i < sizeof($keys); $i++)
			unset($attr_old[$keys[$i]]['count']);
		$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
		if ($success) $success = ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
		if (!$success) return 5;
		}
	if ((isset($_SESSION['hostDN']))) {
		if ($values->general_dn != $values_old->general_dn) {
			unset ($_SESSION['hostDN'][$values_old->general_dn]);
			}
		$_SESSION['hostDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['hostDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	return 3;
	}



function creategroup($values) { // Will create the LDAP-Group
	// 2 == Group already exists at different location
	// 1 == Group has been created
	// 3 == Group has been modified
	// 4 == Error while creating Group
	// 5 == Error while modifying Group
	$values->general_dn = 'cn=' . $values->general_username . ',' . $values->general_dn;
	$attr['objectClass'][0] = 'posixGroup';
	$attr['cn'] = $values->general_username;
	$attr['gidNumber'] = $values->general_uidNumber;
	if ($values->general_gecos) $attr['description'] = utf8_encode($values->general_gecos);
	if ($_SESSION['config']->samba3 =='yes') {
		$attr['sambaSID'] = $values->smb_mapgroup;
		$attr['objectClass'][1] = 'sambaGroupMapping';
		$attr['sambaGroupType'] = '2';
		if ($values->smb_displayName) $attr['displayName'] = utf8_encode($values->smb_displayName);
		}
	foreach ($values->unix_memberUid as $user)
		$attr['memberUid'][] = $user;
	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	if (!$success) return 4;
	if ($_SESSION['config']->scriptServer) setquotas($values,'group');
	// Add entry to cache-array
	if ((isset($_SESSION['groupDN']))) {
		$_SESSION['groupDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['groupDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	return 1;
	}

function modifygroup($values,$values_old) { // Will modify the LDAP-Group
	// 2 == Group already exists at different location
	// 3 == Group has been modified
	// 5 == Error while modifying Group
	$values->general_dn = 'cn=' . $values->general_username . ',' . $values->general_dn;

	if ($values->general_username != $values_old->general_username) $attr['cn'] = $values->general_username;
	// Set correct SID if UID was changed
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['uidNumber'] = $values->general_uidNumber; // posixAccount_req
		if ($_SESSION['config']->is_samba3()) $attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase +1); // sambaAccount_may
			else $attr['rid'] = (2 * $values->general_uidNumber + 1001); // sambaAccount_may
		}

	if ($values->general_gecos != $values_old->general_gecos) $attr['description'] = utf8_encode($values->general_gecos);

	if ($values->smb_displayName != $values_old->smb_displayName)
		$attr['displayName'] = utf8_encode($values->smb_displayName);

	if ($_SESSION['config']->samba3 =='yes') {
		if ($values->smb_mapgroup != $values_old->smb_mapgroup)
			$attr['sambaSID'] = $values->smb_mapgroup;
		}

	if (($values->unix_memberUid != $values_old->unix_memberUid)) {
		if (count($values->unix_memberUid)==0) $attr_rem['memberUid'] = $values_old->unix_memberUid;
		 else $attr['memberUid'] = $values->unix_memberUid;
		}


	// Add missing objectclasses to group
	if (!in_array('posixGroup', $values->general_objectClass)) {
		$attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'posixGroup';
		}
	if (($_SESSION['config']->is_samba3()) && (!in_array('sambaGroupMapping', $values->general_objectClass))) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'sambaGroupMapping';
		$attr['sambaGroupType'] = '2';
		}

	if ($attr_rem) { // Remove attributes not longer valid
		$success = ldap_mod_del($_SESSION['ldap']->server(),$values_old->general_dn, $attr_rem);
		if (!$success) return 5;
		}

	if ($attr) { // Add /replace new attributes
		$success = ldap_modify($_SESSION['ldap']->server(),$values->general_dn, $attr);
		if (!$success) return 5;
		}

	if ($values->general_dn != $values_old->general_dn) {// Groupname hasn't changed
		$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixGroup");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		// remove "count" from array
		unset($attr_old['count']);
		for ($i=0; $i < sizeof($attr_old); $i++) unset($attr_old[$i]);
		$keys = array_keys($attr_old);
		for ($i=0; $i < sizeof($keys); $i++)
			unset($attr_old[$keys[$i]]['count']);
		$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
		if ($success) ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
		if ($success) $success = ldap_mod_replace($_SESSION['ldap']->server(),$values->general_dn, $attr);
		if (!$success) return 5;
		}

	if ( $_SESSION['final_changegids']==true ) { // Chnage GIDs of all users which are member of group
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), 'gidNumber=' . $values_old->general_uidNumber, array('gidNumber'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$user['gidNumber'][0] = $values->general_uidNumber;
			ldap_modify($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry), $user);
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}
	if ($_SESSION['config']->scriptServer) setquotas($values,'group',$values_old);

	if ((isset($_SESSION['groupDN']))) { // refresh group-cache array
		if ($values->general_dn != $values_old->general_dn) {
			unset ($_SESSION['groupDN'][$values_old->general_dn]);
			}
		$_SESSION['groupDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['groupDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	return 3;
	}


?>
