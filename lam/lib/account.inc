<?
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
	var $smb_scriptpath;
	var $smb_profilePath;
	var $smb_smbuserworkstations;
	var $smb_smbhome;
	var $smb_domain;
	var $smb_flagsW;
	var $smb_flagsD;
	var $smb_flagsX;
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

function registervars() { // This function registers all needes session-varibales needed by account.php
	// if session was started previos, the existing session will be continued
	session_save_path('../sess');
	@session_start();
	if ( !session_is_registered("type2")) session_register("type2"); // $type2 stores the kind of account (User|GRoup|Host)
	if ( !session_is_registered("shelllist")) session_register("shelllist"); // $shelllist contains all shells defined in /etc/shells
	if ( !session_is_registered("modify")) session_register("modify"); // if an existing account should be modified modify is true
	if ( !session_is_registered("account")) session_register("account"); // The new Accout properties are stored here
	if ( !session_is_registered("account_temp")) session_register("account_temp"); // Tempoprary account properties. If values are OK they will be moved to var account
	if ( !session_is_registered("account_old")) session_register("account_old"); // Only valid if an account should be modified. It'll contains the existing account properties
	if (!is_object($account)) $account = new account();
	if (!is_object($account_temp)) $account_temp = new account();
	if (!is_object($account_old)) $account = new account();
	}

function getshells() { // Return a list of all shells listed in /etc/shells
	$shells =  file('/etc/shells');
	foreach ($shells as $shell) chop($shell);
	return $shells;
	}

function checkglobal() { // This functions checks all global account parameters
	// Check if username has been entered
	$error = "0";
	switch ( $_SESSION['type2'] ) {
		case 'user' :
			// Check if Username-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account_temp']->general_username))   $error = _('Username must content between 3 and 20 characters.');
			// Check if Username starts with letter
			if ( !ereg('^[a-z].*$', $_SESSION['account_temp']->general_username)) $error = _('Username contents invalid characters. First character must be a letter');
			// Check if Username contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['account_temp']->general_username)) $error = _('Username contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if user already exists
			$temp = ldapexists();
			if ($temp) $error = $temp;
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z])*$', $_SESSION['account_temp']->surname)) $error = _('Surname contents invalid characters');
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z])*$', $_SESSION['account_temp']->givenname)) $error = _('Givenname contents invalid characters');
			// Check if Homedir is valid
			$_SESSION['account_temp']->general_homedir = str_replace('$user', $_SESSION['account_temp']->general_username, $_SESSION['account_temp']->general_homedir);
			$_SESSION['account_temp']->general_homedir = str_replace('$group', $_SESSION['account_temp']->general_group, $_SESSION['account_temp']->general_homedir);
			if ( !ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account_temp']->general_homedir )) $error = _('Homedirectory contents invalid characters.');
			if ($_SESSION['account_temp']->general_gecos=='') $_SESSION['account_temp']->general_gecos = $_SESSION['account_temp']->general_givenname . " " . $_SESSION['account_temp']->general_surname ;
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$temp = checkid();
			if ($temp) $error = $temp;
			break;
		case 'group' :
			// Check if Groupname-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account_temp']->general_username))   $error = _('Groupname must content between 3 and 20 characters.');
			// Check if Groupname starts with letter
			if ( !ereg('^[a-z].*$', $_SESSION['account_temp']->general_username)) $error = _('Groupname contents invalid characters. First character must be a letter');
			// Check if Groupname contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['account_temp']->general_username)) $error = _('Groupname contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if group already exists
			$temp = ldapexists();
			if ($temp) $error = $temp;
			// Check if GID is valid. If none value was entered, the next useable value will be inserted
			$temp = checkid();
			if ($temp) $error = $temp;
			if ($_SESSION['account_temp']->general_gecos=='') $_SESSION['account_temp']->general_gecos = $_SESSION['account_temp']->general_username ;
			break;
		case 'host' :
			if ( substr($_SESSION['account_temp']->general_username, strlen($_SESSION['account_temp']->general_username)-1, strlen($_SESSION['account_temp']->general_username)) != '$' ) $_SESSION['account_temp']->general_username = $_SESSION['account_temp']->general_username . '$';
			// Check if Hostname-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account_temp']->general_username))   $error = _('Hostname must content between 3 and 20 characters.');
			// Check if Hostname starts with letter
			if ( !ereg('^[a-z].*$', $_SESSION['account_temp']->general_username)) $error = _('Hostname contents invalid characters. First character must be a letter');
			// Check if Hostname contents only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[$])*$', $_SESSION['account_temp']->general_username)) $error = _('Hostname contents invalid characters. Valid characters are: a-z, 0-9 and .-_ !');
			// Check if Hostname already exists
			$temp = ldapexists();
			if ($temp) $error = $temp;
			$_SESSION['account_temp']->general_homedir = '/dev/null';
			$_SESSION['account_temp']->general_shell = '/bin/false';
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$temp = checkid();
			if ($temp) $error = $temp;
			if ($_SESSION['account_temp']->general_gecos=='') $_SESSION['account_temp']->general_gecos = $_SESSION['account_temp']->general_username;
			break;
		}
	if ($_SESSION['account_temp']->general_username) $_SESSION['account']->general_username = $_SESSION['account_temp']->general_username;
	if ($_SESSION['account_temp']->general_surname) $_SESSION['account']->general_surname = $_SESSION['account_temp']->general_surname;
	if ($_SESSION['account_temp']->general_givenname) $_SESSION['account']->general_givenname = $_SESSION['account_temp']->general_givenname;
	if ($_SESSION['account_temp']->general_uidNumber) $_SESSION['account']->general_uidNumber = $_SESSION['account_temp']->general_uidNumber;
	if ($_SESSION['account_temp']->general_group) $_SESSION['account']->general_group = $_SESSION['account_temp']->general_group;
	if ($_SESSION['account_temp']->general_groupadd) $_SESSION['account']->general_groupadd = $_SESSION['account_temp']->general_groupadd;
	if ($_SESSION['account_temp']->general_homedir) $_SESSION['account']->general_homedir = $_SESSION['account_temp']->general_homedir;
	if ($_SESSION['account_temp']->general_shell) $_SESSION['account']->general_shell = $_SESSION['account_temp']->general_shell;
	if ($_SESSION['account_temp']->general_dn) $_SESSION['account']->general_dn = $_SESSION['account_temp']->general_dn;
	if ($_SESSION['account_temp']->general_gecos) $_SESSION['account']->general_gecos = $_SESSION['account_temp']->general_gecos;
	return $error;
	}


function checkunix() { // This function checks all unix account paramters
	$error = "0";
	switch ( $_SESSION['type2'] ) {
		case 'user' :
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->unix_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ($_SESSION['account_temp']->unix_pwdwarn=='') $error = _('No value for Password Warn.');
			if ( !ereg('^([1-9]+)([0-9]*)$', $_SESSION['account_temp']->unix_pwdwarn))  $error = _('Password Warn must be are natural number.');
			if ($_SESSION['account_temp']->unix_pwdallowlogin=='') $error = _('No value for Password Expire.');
			if ( !ereg('^(([-][1])|([0-9]*))$', $_SESSION['account_temp']->unix_pwdallowlogin))  $error = _('Password Expire must be are natural number or -1.');
			if ($_SESSION['account_temp']->unix_pwdmaxage=='') $error = _('No value for Password Maxage.');
			if ( !ereg('^([1-9]+)([0-9]*)$', $_SESSION['account_temp']->unix_pwdmaxage))  $error = _('Password Maxage must be are natural number.');
			if ($_SESSION['account_temp']->unix_pwdminage=='') $error = _('No value for Password Minage.');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdminage))  $error = _('Password Minage must be are natural number.');
			if ( $_SESSION['account_temp']->unix_pwdminage > $_SESSION['account_temp']->unix_pwdmaxage ) $error = _('Password Maxage must bigger as Password Minage.');
			break;
		case 'host' :
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->unix_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ($_SESSION['account_temp']->unix_pwdwarn=='') $error = _('No value for Password Warn.');
			if ( !ereg('^([1-9]+)([0-9]*)$', $_SESSION['account_temp']->unix_pwdwarn))  $error = _('Password Warn must be are natural number.');
			if ($_SESSION['account_temp']->unix_pwdallowlogin=='') $error = _('No value for Password Expire.');
			if ( !ereg('^(([-][1])|([0-9]*))$', $_SESSION['account_temp']->unix_pwdallowlogin))  $error = _('Password Expire must be are natural number or -1.');
			if ($_SESSION['account_temp']->unix_pwdmaxage=='') $error = _('No value for Password Maxage.');
			if ( !ereg('^([1-9]+)([0-9]*)$', $_SESSION['account_temp']->unix_pwdmaxage))  $error = _('Password Maxage must be are natural number.');
			if ($_SESSION['account_temp']->unix_pwdminage=='') $error = _('No value for Password Minage.');
			if ( !ereg('^([0-9]*)$', $_SESSION['account_temp']->unix_pwdminage))  $error = _('Password Minage must be are natural number.');
			if ( $_SESSION['account_temp']->unix_pwdminage > $_SESSION['account_temp']->unix_pwdmaxage ) $error = _('Password Maxage must bigger as Password Minage.');
			break;
		}
	// Write Values from Webpage to Session-Variables
	$_SESSION['account']->unix_password = $_SESSION['account_temp']->unix_password;
	$_SESSION['account']->unix_password_no = $_SESSION['account_temp']->unix_password_no;
	$_SESSION['account']->unix_pwdwarn = $_SESSION['account_temp']->unix_pwdwarn;
	$_SESSION['account']->unix_pwdallowlogin = $_SESSION['account_temp']->unix_pwdallowlogin;
	$_SESSION['account']->unix_pwdmaxage = $_SESSION['account_temp']->unix_pwdmaxage;
	$_SESSION['account']->unix_pwdminage = $_SESSION['account_temp']->unix_pwdminage;
	$_SESSION['account']->unix_pwdexpire_day = $_SESSION['account_temp']->unix_pwdexpire_day;
	$_SESSION['account']->unix_pwdexpire_mon = $_SESSION['account_temp']->unix_pwdexpire_mon;
	$_SESSION['account']->unix_pwdexpire_yea = $_SESSION['account_temp']->unix_pwdexpire_yea;
	if ($_SESSION['account_temp']->unix_deactivated) $_SESSION['account']->unix_deactivated = 1; else $_SESSION['account']->unix_deactivated = 0;
	return $error;
	}

function checksamba() { // This function checks all samba account paramters
	$error = "0";
	if ($_SESSION['account_temp']->smb_useunixpwd) $_SESSION['account_temp']->smb_password = $_SESSION['account_temp']->unix_password;
	switch ( $_SESSION['type2'] ) {
		case 'user' :
			$_SESSION['account_temp']->smb_scriptpath = str_replace('$user', $_SESSION['account_temp']->general_username, $_SESSION['account_temp']->smb_scriptpath);
			$_SESSION['account_temp']->smb_scriptpath = str_replace('$group', $_SESSION['account_temp']->general_group, $_SESSION['account_temp']->smb_scriptpath);
			$_SESSION['account_temp']->smb_profilePath = str_replace('$user', $_SESSION['account_temp']->general_username, $_SESSION['account_temp']->smb_profilePath);
			$_SESSION['account_temp']->smb_profilePath = str_replace('$group', $_SESSION['account_temp']->general_group, $_SESSION['account_temp']->smb_profilePath);
			$_SESSION['account_temp']->smb_smbHome = str_replace('$user', $_SESSION['account_temp']->general_username, $_SESSION['account_temp']->smb_smbHome);
			$_SESSION['account_temp']->smb_smbHome = str_replace('$group', $_SESSION['account_temp']->general_group, $_SESSION['account_temp']->smb_smbHome);
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->smb_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ( (!$_SESSION['account_temp']->smb_scriptpath=='') && (!ereg('^([/])*[a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account_temp']->smb_scriptpath))) $error = _('Scriptpath is invalid');
			if ( (!$_SESSION['account_temp']->smb_profilePath=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account_temp']->smb_profilePath)) && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-])+)+$', $_SESSION['account_temp']->smb_profilePath))) $error = _('ProfilePath is invalid.');
			if ( (!$_SESSION['account_temp']->smb_smbHome=='') && !ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-])+)+$', $_SESSION['account_temp']->smb_smbhome)) $error = _('smbHome is invalid.');
			if ( ((!$_SESSION['account_temp']->smb_smbuserworkstations=='') && $_SESSION['account_temp']->smb_smbuserworkstations!='*') && (!ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([ ])+([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $_SESSION['account_temp']->smb_smbuserworkstations))) $error = _('User Workstations is invalid.');
			if ( (!$_SESSION['account_temp']->smb_domain=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $_SESSION['account_temp']->smb_domain)) $error = _('Domain Name contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.');
			$_SESSION['account_temp']->smb_flagsW = 0;
			break;
		case 'host' :
			// Sonderzeichen |#*,.;:_-+!$%&/|?{[()]} ****************************************************
			if ( !ereg('^([a-z]|[A-Z]|[0-9])*$', $_SESSION['account_temp']->smb_password)) $error = _('Password contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !');
			if ( (!$_SESSION['account_temp']->smb_domain=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $_SESSION['account_temp']->smb_domain)) $error = _('Domain Name contents invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.');
			$_SESSION['account_temp']->smb_flagsW = 1;
			break;
		}
	// Write Values from Webpage to Session-Variables
	$_SESSION['account']->smb_password = $_SESSION['account_temp']->smb_password;
	$_SESSION['account']->smb_password_no = $_SESSION['account_temp']->smb_password_no;
	if ($_SESSION['account_temp']->smb_useunixpwd ) $_SESSION['account']->smb_useunixpwd = 1; else $_SESSION['account']->smb_useunixpwd = 0;
	if ($_SESSION['account_temp']->smb_pwdcanchange ) $_SESSION['account']->smb_pwdcanchange = 1; else $_SESSION['account']->smb_pwdcanchange = 0;
	if ($_SESSION['account_temp']->smb_pwdmustchange) $_SESSION['account']->smb_pwdmustchange = 1; else $_SESSION['account']->smb_pwdmustchange = 0;
	$_SESSION['account']->smb_homedrive = $_SESSION['account_temp']->smb_homedrive;
	$_SESSION['account']->smb_profilePath = $_SESSION['account_temp']->smb_profilePath;
	$_SESSION['account']->smb_scriptpath = $_SESSION['account_temp']->smb_scriptpath;
	$_SESSION['account']->smb_smbuserworkstations = $_SESSION['account_temp']->smb_smbuserworkstations;
	$_SESSION['account']->smb_smbhome = $_SESSION['account_temp']->smb_smbhome;
	$_SESSION['account']->smb_domain = $_SESSION['account_temp']->smb_domain;
	if ($_SESSION['account_temp']->smb_flagsW) $_SESSION['account']->smb_flagsW = 1; else $_SESSION['account']->smb_flagsW = 0;
	if ($_SESSION['account_temp']->smb_flagsD) $_SESSION['account']->smb_flagsD = 1; else $_SESSION['account']->smb_flagsD = 0;
	if ($_SESSION['account_temp']->smb_flagsX) $_SESSION['account']->smb_flagsX = 1; else $_SESSION['account']->smb_flagsX = 0;
	return $error;
	}

function checkpersonal() {
	$error = "0";
	if ($_SESSION['account_temp']->personal_title) $_SESSION['account']->personal_title = $_SESSION['account_temp']->personal_title ;
	if ($_SESSION['account_temp']->personal_mail) $_SESSION['account']->personal_mail = $_SESSION['account_temp']->personal_mail ;
	if ($_SESSION['account_temp']->personal_telephoneNumber) $_SESSION['account']->personal_telephoneNumber = $_SESSION['account_temp']->personal_telephoneNumber ;
	if ($_SESSION['account_temp']->personal_mobileTelephoneNumber) $_SESSION['account']->personal_mobileTelephoneNumber = $_SESSION['account_temp']->personal_mobileTelephoneNumber ;
	if ($_SESSION['account_temp']->personal_facsimileTelephoneNumber) $_SESSION['account']->personal_facsimileTelephoneNumber = $_SESSION['account_temp']->personal_facsimileTelephoneNumber ;
	if ($_SESSION['account_temp']->personal_street) $_SESSION['account']->personal_street = $_SESSION['account_temp']->personal_street ;
	if ($_SESSION['account_temp']->personal_postalCode) $_SESSION['account']->personal_postalCode = $_SESSION['account_temp']->personal_postalCode ;
	if ($_SESSION['account_temp']->personal_postalAddress) $_SESSION['account']->personal_postalAddress = $_SESSION['account_temp']->personal_postalAddress ;
	if ($_SESSION['account_temp']->personal_employeeType) $_SESSION['account']->personal_employeeType = $_SESSION['account_temp']->personal_employeeType ;
	return $error;
	}

function genpasswd() { // This function will return a password with max. 8 characters
	// Allowed Characters to generate passwords
	$LCase = 'abcdefghjkmnpqrstuvwxyz';
	$UCase = 'ABCDEFGHJKMNPQRSTUVWXYZ';
	$Integer = '23456789';
	// DEFINE CONSTANTS FOR ALGORTTHM
	define("LEN", '1');
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

function ldapexists() { // This function will search if the DN already exists
	switch ($_SESSION['type2']) {
		case 'user': $searchbase = $_SESSION['config']->get_UserSuffix(); break;
		case 'group': $searchbase = $_SESSION['config']->get_GroupSuffix(); break;
		case 'host': $searchbase = $_SESSION['config']->get_HostSuffix(); break;
		}
	$result = ldap_search($_SESSION['ldap']->server(), $searchbase, 'cn=' . $_SESSION['account_temp']->general_username, array(''), 1);
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	if ($dn) {
		if ($_SESSION['modify']==1 && $_SESSION['account_temp']->general_username != $_SESSION['account_old']->general_username) return _($_SESSION['type2'] . ' already exists!');
		if ($_SESSION['modify']==0) return  _($_SESSION['type2'] . ' already exists!');
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
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	return $attr['gidNumber'][0];
	}



function checkid() { // if value is empty will return an unused id from all ids found in LDAP else check existing value
	switch ($_SESSION['type2']) {
		case 'user':
			$ObjectClass = 'PosixAccount';
			$search = 'uidNumber';
			$minID = $_SESSION['config']->get_minUID();
			$maxID = $_SESSION['config']->get_maxUID();
			$suffix = $_SESSION['config']->get_UserSuffix();
			break;
		case 'group':
			$ObjectClass = 'PosixGroup';
			$search = 'gidNumber';
			$minID = $_SESSION['config']->get_MinGID();
			$maxID = $_SESSION['config']->get_MaxGID();
			$suffix = $_SESSION['config']->get_GroupSuffix();
			break;
		case 'host':
			$ObjectClass = 'PosixAccount';
			$search = 'uidNumber';
			$minID = $_SESSION['config']->get_MinMachine();
			$maxID = $_SESSION['config']->get_MaxMachine();
			$suffix = $_SESSION['config']->get_HostSuffix();
			break;
		}
	if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify']==0) {
		$result = ldap_search($_SESSION['ldap']->server(), $suffix, 'ObjectClass='.$ObjectClass);
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$vals = ldap_get_values($_SESSION['ldap']->server(), $entry, $search);
			$ids[] = $vals[0];
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
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
		$_SESSION['account_temp']->general_uidNumber = $useID;
		}
	if ($_SESSION['modify']==0) {
		if (($_SESSION['account_temp']->general_uidNumber=='') && $_SESSION['modify'] == 1) $_SESSION['account_temp']->general_uidNumber = $_SESSION['account_old']->general_uidNumber ;
		$result = ldap_search($_SESSION['ldap']->server(), $suffix, $search . '=' . $_SESSION['account_temp']->general_uidNumber, array(''), 1);
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		if ($entry) $dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
		if ( $dn && $_SESSION['modify']==0) return _('ID is used from group' . $dn . ' !');
		if ( $_SESSION['account_temp']->general_uidNumber < $minID || $_SESSION['account_temp']->general_uidNumber > $maxID) return _('Please enter a value between '. $minID . ' and ' . $maxID . '!');
		if ( $dn && ($dn != $_SESSION['account_old']->general_dn) && $_SESSION['modify']==1) return _('ID is used from user ' . $dn . ' !');
		}
	return 0;
	}

function getdays() { // will return the days from 1.1.1970 until now
	$days = time() / 86400;
	settype($days, 'integer');
	return $days;
	}

function smbflag() { // Creates te attribute attrFlags
	$flag = "[";
	if ($_SESSION['account']->smb_flagsW) $flag = $flag . "W"; else $flag = $flag . "U";
	if ($_SESSION['account']->smb_flagsD) $flag = $flag . "D";
	if ($_SESSION['account']->smb_flagsX) $flag = $flag . "X";
	$flag = $flag. "]";
	return $flag;
	}

function loaduser($dn) { // Will load all needed values from an existing account
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['uid'][0]) $_SESSION['account']->general_username = $attr['uid'][0];
	if ($attr['uidNumber'][0]) $_SESSION['account']->general_uidNumber = $attr['uidNumber'][0];
	if ($attr['homeDirectory'][0]) $_SESSION['account']->general_homedir = $attr['homeDirectory'][0];
	if ($attr['shadowLastChange'][0]) $_SESSION['account']->unix_shadowLastChange = $attr['shadowLastChange'][0];
	if ($attr['loginShell'][0]) $_SESSION['account']->general_shell = $attr['loginShell'][0];
	if ($attr['gecos'][0]) $_SESSION['account']->general_gecos = $attr['gecos'][0];
	if ($attr['description'][0]) $_SESSION['account']->general_gecos = $attr['description'][0];
	if ($attr['gidNumber'][0]) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('uidNumber'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['gidNumber'][0]==$attr['gidNumber'][0]) $_SESSION['account']->general_group = $attr2['cn'][0];
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('memberUid'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr2['memberUid']) foreach ($attr2['memberUid'] as $id)
			if (($id==$_SESSION['account']->general_username) && ($attr2['cn'][0]!=$_SESSION['account']->general_group)) $_SESSION['account']->general_groupadd[]=$attr2['cn'][0];
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	if ($attr['shadowMin'][0]) $_SESSION['account']->unix_pwdminage = $attr['shadowMin'][0];
	if ($attr['shadowMax'][0]) $_SESSION['account']->unix_pwdmaxage = $attr['shadowMax'][0];
	if ($attr['shadowWarning'][0]) $_SESSION['account']->unix_pwdwarn = $attr['shadowWarning'][0];
	if ($attr['shadowInactive'][0]) $_SESSION['account']->unix_pwdallowlogin = $attr['shadowInactive'][0];
	if ($attr['shadowExpire'][0]) {
		$date = getdate ($attr['shadowExpire'][0]*86400);
		$_SESSION['account']->unix_pwdexpire_day = $date['mday'];
		$_SESSION['account']->unix_pwdexpire_mon = $date['mon'];
		$_SESSION['account']->unix_pwdexpire_yea = $date['year'];
		}
	if ($attr['pwdCanChange'][0]) $_SESSION['account']->smb_pwdcanchange = $attr['pwdCanChange'][0];
	if ($attr['acctFlags'][0]) {
		if (strrpos($attr['acctFlags'][0], 'W')) $_SESSION['account']->smb_flagsW=true;
		if (strrpos($attr['acctFlags'][0], 'D')) $_SESSION['account']->smb_flagsD=true;
		if (strrpos($attr['acctFlags'][0], 'X')) $_SESSION['account']->smb_flagsX=true;
		}
	if ($attr['smbHome'][0]) $_SESSION['account']->smb_smbhome = $attr['smbHome'][0];
	if ($attr['homeDrive'][0]) $_SESSION['account']->smb_homedrive = $attr['homeDrive'][0];
	if ($attr['scriptPath'][0])  $_SESSION['account']->smb_scriptpath = $attr['scriptPath'][0];
	if ($attr['profilePath'][0]) $_SESSION['account']->smb_profilePath = $attr['profilePath'][0];
	if ($attr['userWorkstations'][0]) $_SESSION['account']->smb_smbuserworkstations = $attr['userWorkstations'][0];
	if ($attr['domain'][0]) $_SESSION['account']->smb_domain = $attr['domain'][0];
	if ($attr['givenName'][0]) $_SESSION['account']->general_givenname = $attr['givenName'][0];
	if ($attr['sn'][0]) $_SESSION['account']->general_surname = $attr['sn'][0];
	if ($attr['title'][0]) $_SESSION['account_temp']->personal_title = $attr['title'][0];
	if ($attr['mail'][0]) $_SESSION['account_temp']->personal_mail = $attr['mail'][0];
	if ($attr['telephoneNumber'][0]) $_SESSION['account_temp']->personal_telephoneNumber = $attr['telephoneNumber'][0];
	if ($attr['mobileTelephoneNumber'][0]) $_SESSION['account_temp']->personal_mobileTelephoneNumber = $attr['mobileTelephoneNumber'][0];
	if ($attr['facsimileTelephoneNumber'][0]) $_SESSION['account_temp']->personal_facsimileTelephoneNumber = $attr['facsimileTelephoneNumber'][0];
	if ($attr['street'][0]) $_SESSION['account_temp']->personal_street = $attr['street'][0];
	if ($attr['postalCode'][0]) $_SESSION['account_temp']->personal_postalCode = $attr['postalCode'][0];
	if ($attr['postalAddress'][0]) $_SESSION['account_temp']->personal_postalAddress = $attr['postalAddress'][0];
	if ($attr['employeeType'][0]) $_SESSION['account_temp']->personal_employeeType = $attr['employeeType'][0];
	if (substr(str_replace('{CRYPT}', '',$attr['userPassword'][0]),0,1) == '!' ) $_SESSION['account']->unix_deactivated=true;
	$_SESSION['account_old'] = $_SESSION['account'];
	if ($attr['userPassword'][0]) $_SESSION['account_old']->unix_password = $attr['userPassword'][0];
	if ($attr['ntPassword'][0])  $_SESSION['account_old']->smb_password = $attr['ntPassword'][0];
	}

function loadhost($dn) { // Will load all needed values from an existing account
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['uid'][0]) $_SESSION['account']->general_username = $attr['uid'][0];
	if ($attr['uidNumber'][0]) $_SESSION['account']->general_uidNumber = $attr['uidNumber'][0];
	if ($attr['shadowLastChange'][0]) $_SESSION['account']->unix_shadowLastChange = $attr['shadowLastChange'][0];
	if ($attr['gecos'][0]) $_SESSION['account']->general_gecos = $attr['gecos'][0];
	if ($attr['description'][0]) $_SESSION['account']->general_gecos = $attr['description'][0];
	if ($attr['gidNumber'][0]) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('uidNumber'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['gidNumber'][0]==$attr['gidNumber'][0]) $_SESSION['account']->general_group = $attr2['cn'][0];
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "objectclass=PosixGroup", array('memberUid'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($attr2['memberUid']) foreach ($attr2['memberUid'] as $id)
			if (($id==$_SESSION['account']->general_username) && ($attr2['cn'][0]!=$_SESSION['account']->general_group)) $_SESSION['account']->general_groupadd[]=$attr2['cn'][0];
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}
	if ($attr['shadowMin'][0]) $_SESSION['account']->unix_pwdminage = $attr['shadowMin'][0];
	if ($attr['shadowMax'][0]) $_SESSION['account']->unix_pwdmaxage = $attr['shadowMax'][0];
	if ($attr['shadowWarning'][0]) $_SESSION['account']->unix_pwdwarn = $attr['shadowWarning'][0];
	if ($attr['shadowInactive'][0]) $_SESSION['account']->unix_pwdallowlogin = $attr['shadowInactive'][0];
	if ($attr['shadowExpire'][0]) {
		$date = getdate ($attr['shadowExpire'][0]*86400);
		$_SESSION['account']->unix_pwdexpire_day = $date['mday'];
		$_SESSION['account']->unix_pwdexpire_mon = $date['mon'];
		$_SESSION['account']->unix_pwdexpire_yea = $date['year'];
		}
	if ($attr['pwdCanChange'][0]) $_SESSION['account']->smb_pwdcanchange = $attr['pwdCanChange'][0];
	if ($attr['acctFlags'][0]) {
		if (strrpos($attr['acctFlags'][0], 'W')) $_SESSION['account']->smb_flagsW=true;
		if (strrpos($attr['acctFlags'][0], 'D')) $_SESSION['account']->smb_flagsD=true;
		if (strrpos($attr['acctFlags'][0], 'X')) $_SESSION['account']->smb_flagsX=true;
		}
	if ($attr['domain'][0]) $_SESSION['account']->smb_domain = $attr['domain'][0];
	if ($attr['givenName'][0]) $_SESSION['account']->general_givenname = $attr['givenName'][0];
	if ($attr['sn'][0]) $_SESSION['account']->general_surname = $attr['sn'][0];
	if (substr(str_replace('{CRYPT}', '',$attr['userPassword'][0]),0,1) == '!' ) $_SESSION['account']->unix_deactivated=true;
	$_SESSION['account_old'] = $_SESSION['account'];
	if ($attr['userPassword'][0]) $_SESSION['account_old']->unix_password = $attr['userPassword'][0];
	if ($attr['ntPassword'][0])  $_SESSION['account_old']->smb_password = $attr['ntPassword'][0];
	}


function loadgroup($dn) { // Will load all needed values from an existing group
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixGroup");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
	if ($attr['gidNumber'][0]) $_SESSION['account']->general_uidNumber = $attr['gidNumber'][0];
	if ($attr['description'][0]) $_SESSION['account']->general_gecos = $attr['description'][0];
	if ($attr['cn'][0]) $_SESSION['account']->general_username = $attr['cn'][0];
	if ($attr['memberUid']) $_SESSION['account']->general_memberUid = $attr['memberUid'];
	if (is_array($_SESSION['account']->general_memberUid)) array_shift($_SESSION['account']->general_memberUid);
	$_SESSION['account']->general_dn = $dn;
	$_SESSION['account_old'] = $_SESSION['account'];
	}


function createuser() { // Will create the LDAP-Account
	// 2 == Account allready exists at different location
	// 1 == Account has been created
	// 3 == Account has been modified
	// 4 == Error while creating Account
	// 5 == Error while modifying Account
	// Value stored in shadowExpire, days since 1.1.1970
	$date = mktime(0,0,0, $_SESSION['account']->unix_pwdexpire_day, $_SESSION['account']->unix_pwdexpire_mon, $_SESSION['account']->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$_SESSION['account']->general_dn = 'cn=' . $_SESSION['account']->general_username . ',' . $_SESSION['config']->get_UserSuffix();

	// All Values need for an user-account
	// General Objectclasses
	$attr['objectClass'][0] = 'inetOrgPerson';
	$attr['objectClass'][1] = 'posixAccount';
	$attr['objectClass'][2] = 'shadowAccount';
	$attr['objectClass'][3] = 'sambaAccount';
	$attr['cn'] = $_SESSION['account']->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $_SESSION['account']->general_username; // posixAccount_req
	$attr['uidNumber'] = $_SESSION['account']->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($_SESSION['account']->general_group); // posixAccount_req
	$attr['homeDirectory'] = $_SESSION['account']->general_homedir; // posixAccount_req
	if ($_SESSION['account']->personal_title!='') $attr['title'] = $_SESSION['account']->personal_title;
	if ($_SESSION['account']->personal_mail!='') $attr['mail'] = $_SESSION['account']->personal_mail;
	if ($_SESSION['account']->personal_telephoneNumber!='') $attr['telephoneNumber'] = $_SESSION['account']->personal_telephoneNumber;
	if ($_SESSION['account']->personal_mobileTelephoneNumber!='') $attr['mobileTelephoneNumber'] = $_SESSION['account']->personal_mobileTelephoneNumber;
	if ($_SESSION['account']->personal_facsimileTelephoneNumber!='') $attr['facsimileTelephoneNumber'] = $_SESSION['account']->personal_facsimileTelephoneNumber;
	if ($_SESSION['account']->personal_street!='') $attr['street'] = $_SESSION['account']->personal_street;
	if ($_SESSION['account']->personal_postalCode!='') $attr['postalCode'] = $_SESSION['account']->personal_postalCode;
	if ($_SESSION['account']->personal_postalAddress!='') $attr['postalAddress'] = $_SESSION['account']->personal_postalAddress;
	if ($_SESSION['account']->personal_employeeType!='') $attr['employeeType'] = $_SESSION['account']->personal_employeeType;
	// posixAccount_may shadowAccount_may
	if ($_SESSION['modify']==1) {
		$password_old = str_replace('{CRYPT}', '',$_SESSION['account_old']->unix_password);
		if (substr($password_old,0,1) == '!' ) $password_old = substr($password_old,1,strlen($password_old));
		if ($_SESSION['account']->unix_password=='') {
			if ($_SESSION['account']->unix_password_no) $password_old = '';
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . $password_old;
			else $attr['userPassword'] = '{CRYPT}' . $password_old;
			$attr['shadowLastChange'] =  $_SESSION['account_old']->unix_shadowLastChange; // shadowAccount_may
			}
		 else {
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
			else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
			$attr['shadowLastChange'] = getdays(); // shadowAccount_may
			}
		if ($_SESSION['account']->smb_password!='') {
			$attr['ntPassword'] = exec('../lib/createntlm.pl nt ' . $_SESSION['account']->smb_password);
			$attr['lmPassword'] = exec('../lib/createntlm.pl lm ' . $_SESSION['account']->smb_password);
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
		}
	 else {
		if ($_SESSION['account']->unix_password_no) $_SESSION['account']->unix_password = '';
		if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
		else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccount_may
		$attr['ntPassword'] = exec('../lib/createntlm.pl nt ' . $_SESSION['account']->smb_password);
		$attr['lmPassword'] = exec('../lib/createntlm.pl lm ' . $_SESSION['account']->smb_password);
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	if ($_SESSION['account']->smb_password_no) {
		$attr['ntPassword'] = 'NO PASSWORD*****';
		$attr['lmPassword'] = 'NO PASSWORD*****';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	$attr['loginShell'] = $_SESSION['account']->general_shell; // posixAccount_may
	$attr['gecos'] = $_SESSION['account']->general_gecos; // posixAccount_may
	$attr['description'] = $_SESSION['account']->general_gecos; // posixAccount_may sambaAccount_may

	$attr['shadowMin'] = $_SESSION['account']->unix_pwdminage; // shadowAccount_may
	$attr['shadowMax'] = $_SESSION['account']->unix_pwdmaxage; // shadowAccount_may
	$attr['shadowWarning'] = $_SESSION['account']->unix_pwdwarn; // shadowAccount_may
	$attr['shadowInactive'] = $_SESSION['account']->unix_pwdallowlogin; // shadowAccount_may
	$attr['shadowExpire'] =  $date ; // shadowAccount_may
	$attr['rid'] = (2 * $_SESSION['account']->general_uidNumber + 1000); // sambaAccount_may
	$attr['PrimaryGroupID'] = (2 * getgid($_SESSION['account']->general_group) + 1001); // sambaAccount_req
	if ($_SESSION['account']->smb_pwdcanchange) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($_SESSION['account']->smb_pwdmustchange) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag(); // sambaAccount_may
	$attr['displayName'] = $_SESSION['account']->general_gecos; // sambaAccount_may
	if ($_SESSION['account']->smb_smbhome!='') $attr['smbHome'] = $_SESSION['account']->smb_smbhome; // sambaAccount_may
	if ($_SESSION['account']->smb_homedrive!='') $attr['homeDrive'] = $_SESSION['account']->smb_homedrive; // sambaAccount_may
	if ($_SESSION['account']->smb_scriptpath!='') $attr['scriptPath']  = $_SESSION['account']->smb_scriptpath; // sambaAccount_may
	if ($_SESSION['account']->smb_profilePath!='') $attr['profilePath'] = $_SESSION['account']->smb_profilePath; // sambaAccount_may
	if ($_SESSION['account']->smb_smbuserworkstations!='') $attr['userWorkstations'] = $_SESSION['account']->smb_smbuserworkstations; // sambaAccount_may
	if ($_SESSION['account']->smb_domain!='') $attr['domain'] = $_SESSION['account']->smb_domain; // sambaAccount_may

	if ($_SESSION['account']->general_givenname!='') $attr['givenName'] = $_SESSION['account']->general_givenname;
	if ($_SESSION['account']->general_surname!='') $attr['sn'] = $_SESSION['account']->general_surname;

	if ( $_SESSION['modify'] == 1 ) {
		if ($_SESSION['account']->general_username == $_SESSION['account_old']->general_username) // Username hasn't changed
			$success = ldap_modify($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
			else {
				$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
				if ($success) ldap_delete($_SESSION['ldap']->server(),$_SESSION['account_old']->general_dn);
				}
		if (!$success) return 5;
		// Write Groupmemberchips
		$allgroups = $_SESSION['account']->general_groupadd;
		if (!in_array($_SESSION['account']->general_group, $allgroups)) $allgroups[] = $_SESSION['account']->general_group;
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'objectClass=PosixGroup', array('uidNumber'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$modifygroup=0;
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['memberUid']) {
				array_shift($attr2['memberUid']);
				foreach ($attr2['memberUid'] as $nam) {
					if ( ($attr2['memberUid'][$nam]==$_SESSION['account']->general_username) && !in_array($attr2['memberUid'][$nam], $allgroups)) {
						$todelete['memberUid'] = $attr2['memberUid'][$nam];
						$success = ldap_mod_del($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry) ,$todelete);
						}
					}
				if (!in_array($_SESSION['account']->general_username, $attr2['memberUid']) && in_array($attr2['cn'][0], $allgroups)) {
					$toadd['memberUid'] = $attr2['memberUid'];
					$toadd['memberUid'][] = $_SESSION['account']->general_username;
					$success = ldap_mod_replace($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
					}
				}
			 else {
			 	if (in_array($attr2['cn'][0], $allgroups)) {
					$toadd['memberUid'] = $_SESSION['account']->general_username;
					$success = ldap_mod_add($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
					}
			 	}
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		if (!$success) return 5;
		return 3;
		}
	 else {
		// Write a new entry if user doesn't exists
		$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		if (!$success) return 4;
		// Add user to groups
		$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup", array('memberUid'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($group['memberUid']) array_shift($group['memberUid']);
		if (! in_array($_SESSION['account']->general_username, $group['memberUid'])) {
			$toadd['memberUid'] = $_SESSION['account']->general_username;
			$success = ldap_mod_add($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), $toadd);
			}
		if (!$success) return 4;
		// Add User to Additional Groups
		if ($_SESSION['account']->general_groupadd)
			foreach ($_SESSION['account']->general_groupadd as $group2) {
				$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup", array('memberUid'));
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
				if ($group['memberUid']) array_shift($group['memberUid']);
				if (! in_array($_SESSION['account']->general_username, $group['memberUid'])) {
					$toadd['memberUid'] = $_SESSION['account']->general_username;
					$success = ldap_mod_add($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), $toadd);
					}
				if (!$success) return 4;
			}
		return 1;
		}
	}

function createhost() { // Will create the LDAP-Host
	// 2 == Host allready exists at different location
	// 1 == Host has been created
	// 3 == Host has been modified
	// 4 == Error while creating Host
	// 5 == Error while modifying Host

	// Value stored in shadowExpire, days since 1.1.1970
	$date = mktime(0,0,0, $_SESSION['account']->unix_pwdexpire_day, $_SESSION['account']->unix_pwdexpire_mon, $_SESSION['account']->unix_pwdexpire_yea) / 86400 ;
	settype($date, 'integer');
	$_SESSION['account']->general_dn = 'cn=' . $_SESSION['account']->general_username . ',' . $_SESSION['config']->get_HostSuffix();

	// All Values needed for an user-account
	// General Objectclasses
	$attr['objectClass'][0] = 'top';
	$attr['objectClass'][1] = 'posixAccount';
	$attr['objectClass'][2] = 'shadowAccount';
	$attr['objectClass'][3] = 'sambaAccount';
	$attr['cn'] = $_SESSION['account']->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
	$attr['uid'] = $_SESSION['account']->general_username; // posixAccount_req
	$attr['uidNumber'] = $_SESSION['account']->general_uidNumber; // posixAccount_req
	$attr['gidNumber'] = getgid($_SESSION['account']->general_group); // posixAccount_req
	$attr['homeDirectory'] = $_SESSION['account']->general_homedir; // posixAccount_req
	// posixAccount_may shadowAccount_may
	if ($_SESSION['modify']==1) {
		$password_old = str_replace('{CRYPT}', '',$_SESSION['account_old']->unix_password);
		if (substr($password_old,0,1) == '!' ) $password_old = substr($password_old,1,strlen($password_old));
		if ($_SESSION['account']->unix_password=='') {
			if ($_SESSION['account']->unix_password_no) $password_old = '';
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . $password_old;
			else $attr['userPassword'] = '{CRYPT}' . $password_old;
			$attr['shadowLastChange'] =  $_SESSION['account_old']->unix_shadowLastChange; // shadowAccount_may
			}
		 else {
			if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
			else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
			$attr['shadowLastChange'] = getdays(); // shadowAccount_may
			}
		if ($_SESSION['account']->smb_password!='') {
			$attr['ntPassword'] = exec('../lib/createntlm.pl nt' . $_SESSION['account']->smb_password);
			$attr['lmPassword'] = exec('../lib/createntlm.pl lm' . $_SESSION['account']->smb_password);
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
		}
	 else {
		if ($_SESSION['account']->unix_password_no) $_SESSION['account']->unix_password = '';
		if ($_SESSION['account']->unix_deactivated) $attr['userPassword'] = '{CRYPT}!' . crypt($_SESSION['account']->unix_password);
		else $attr['userPassword'] = '{CRYPT}' . crypt($_SESSION['account']->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccount_may
		$attr['ntPassword'] = exec('../lib/createntlm.pl nt' . $_SESSION['account']->smb_password);
		$attr['lmPassword'] = exec('../lib/createntlm.pl lm' . $_SESSION['account']->smb_password);
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	if ($_SESSION['account']->smb_password_no) {
		$attr['ntPassword'] = 'NO PASSWORD*****';
		$attr['lmPassword'] = 'NO PASSWORD*****';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		}
	$attr['loginShell'] = $_SESSION['account']->general_shell; // posixAccount_may
	$attr['gecos'] = $_SESSION['account']->general_gecos; // posixAccount_may
	$attr['description'] = $_SESSION['account']->general_gecos; // posixAccount_may sambaAccount_may
	$attr['shadowMin'] = $_SESSION['account']->unix_pwdminage; // shadowAccount_may
	$attr['shadowMax'] = $_SESSION['account']->unix_pwdmaxage; // shadowAccount_may
	$attr['shadowWarning'] = $_SESSION['account']->unix_pwdwarn; // shadowAccount_may
	$attr['shadowInactive'] = $_SESSION['account']->unix_pwdallowlogin; // shadowAccount_may
	$attr['shadowExpire'] =  $date ; // shadowAccount_may
	$attr['rid'] = (2 * $_SESSION['account']->general_uidNumber + 1000); // sambaAccount_may
	$attr['PrimaryGroupID'] = (2 * getgid($_SESSION['account']->general_group) + 1001); // sambaAccount_req
	if ($_SESSION['account']->smb_pwdcanchange) $attr['pwdCanChange'] = "1"; else $attr['pwdCanChange'] = "0"; // sambaAccount_may
	if ($_SESSION['account']->smb_pwdmustchange) $attr['pwdMustChange'] = "1"; else $attr['pwdMustChange'] = "0"; // sambaAccount_may
	$attr['acctFlags'] = smbflag(); // sambaAccount_may
	$attr['displayName'] = $_SESSION['account']->general_gecos; // sambaAccount_may
	$attr['domain'] = $_SESSION['account']->smb_domain; // sambaAccount_may

	if ( $_SESSION['modify'] == 1 ) {
		if ($_SESSION['account']->general_username == $_SESSION['account_old']->general_username) // Username hasn't changed
			$success = ldap_modify($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
			else {
				$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
				if ($success) ldap_delete($_SESSION['ldap']->server(),$_SESSION['account_old']->general_dn);
				}
		if (!$success) return 5;
		if ($attr['uidNumber']!=$_SESSION['account_old']->general_uidNumber) echo ('find / -uid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chown ' . $_SESSION['account']->general_uidNumber . ' {} \;');
		// Write Groupmemberchips
		$allgroups = $_SESSION['account']->general_groupadd;
		if (!in_array($_SESSION['account']->general_group, $allgroups)) $allgroups[] = $_SESSION['account']->general_group;
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'objectClass=PosixGroup', array('memberUid'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$modifygroup=0;
			$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if ($attr2['memberUid']) {
				array_shift($attr2['memberUid']);
				foreach ($attr2['memberUid'] as $nam) {
					if ( ($attr2['memberUid'][$nam]==$_SESSION['account']->general_username) && !in_array($attr2['memberUid'][$nam], $allgroups)) {
						$todelete['memberUid'] = $attr2['memberUid'][$nam];
						$success = ldap_mod_del($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry) ,$todelete);
						}
					}
				if (!in_array($_SESSION['account']->general_username, $attr2['memberUid']) && in_array($attr2['cn'][0], $allgroups)) {
					$toadd['memberUid'] = $attr2['memberUid'];
					$toadd['memberUid'][] = $_SESSION['account']->general_username;
					$success = ldap_mod_replace($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
					}
				}
			 else {
			 	if (in_array($attr2['cn'][0], $allgroups)) {
					$toadd['memberUid'] = $_SESSION['account']->general_username;
					$success = ldap_mod_add($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry),  $toadd);
					}
			 	}
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		if (!$success) return 5;
		return 3;
		}
	 else {
		// Write a new entry if user doesn't exists
		$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		if (!$success) return 4;
		// Add Host to groups
		$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup", array('memberUid'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if ($group['memberUid']) array_shift($group['memberUid']);
		if (! in_array($_SESSION['account']->general_username, $group['memberUid'])) {
			$toadd['memberUid'] = $_SESSION['account']->general_username;
			$success = ldap_mod_add($_SESSION['ldap']->server(), 'cn='.$_SESSION['account']->general_group.','.$_SESSION['config']->get_GroupSuffix(), $toadd);
			}
		if (!$success) return 4;
		// Add Host to Additional Groups
		if ($_SESSION['account']->general_groupadd)
			foreach ($_SESSION['account']->general_groupadd as $group2) {
				$result = ldap_search($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), "objectclass=posixGroup", array('memberUid'));
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				$group = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
				if ($group['memberUid']) array_shift($group['memberUid']);
				if (! in_array($_SESSION['account']->general_username, $group['memberUid'])) {
					$toadd['memberUid'] = $_SESSION['account']->general_username;
					$success = ldap_mod_add($_SESSION['ldap']->server(), 'cn='.$group2.','.$_SESSION['config']->get_GroupSuffix(), $toadd);
					}
				if (!$success) return 4;
			}
		return 1;
		}
	}


function creategroup() { // Will create the LDAP-Group
	// 2 == Group allready exists at different location
	// 1 == Group has been created
	// 3 == Group has been modified
	// 4 == Error while creating Group
	// 5 == Error while modifying Group
	$_SESSION['account']->general_dn = 'cn=' . $_SESSION['account']->general_username . ',' . $_SESSION['config']->get_GroupSuffix();
	$attr['objectClass'] = 'posixGroup';
	$attr['cn'] = $_SESSION['account']->general_username;
	$attr['gidNumber'] = $_SESSION['account']->general_uidNumber;
	$attr['description'] = $_SESSION['account']->general_gecos;
	if ($_SESSION['account']->general_memeberUid) $attr['memberUid'] = $_SESSION['account']->general_memberUid;
	if ( $_SESSION['modify']==0 ) { // Write a new entry if group doesn't exists
		$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
		if ($success) return 1;
		else return 4;
		}
	 else { // Modify an Existing entry
		if ($_SESSION['account']->general_username == $_SESSION['account_old']->general_username) // Groupname hasn't changed
			$success = ldap_modify($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
			else {
				$success = ldap_add($_SESSION['ldap']->server(),$_SESSION['account']->general_dn, $attr);
				if ($success) ldap_delete($_SESSION['ldap']->server(),$_SESSION['account_old']->general_dn);
				}
		// Fragen, ob bei geänderter gid die gids der Beutzer in der Gruppe geändert werden sollen. *********************************
		if ( $_SESSION['account']->final_changegids==true ) {
			$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), 'gidNumber=' . $_SESSION['account_old']->general_uidNumber, array('gidNumber'));
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			while ($entry) {
				$user['gidNumber'][0] = $_SESSION['account']->general_uidNumber;
				ldap_modify($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry), $user);
				$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
				}
			}
		if ($success) return 3;
		else return 5;
		}
	}

?>
