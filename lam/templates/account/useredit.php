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


  LDAP Account Manager displays table for creating or modifying accounts in LDAP
*/

include_once('../../lib/account.inc'); // File with all account-funtions
include_once('../../lib/config.inc'); // File with configure-functions
include_once('../../lib/profiles.inc'); // functions to load and save profiles
include_once('../../lib/status.inc'); // Return error-message
include_once('../../lib/pdf.inc'); // Return a pdf-file
include_once('../../lib/ldap.inc'); // LDAP-functions

session_save_path('../../sess');
@session_start();
setlanguage();
$_SESSION['shelllist'] = getshells(); // Write List of all valid shells in variable
if (isset($_GET['DN'])) {
	if (isset($_GET['DN']) && $_GET['DN']!='') {
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		$DN = str_replace("\'", '',$_GET['DN']);
		$_SESSION['account'] = loaduser($DN);
		$_SESSION['account'] ->type = 'user';
		$_SESSION['account_old'] = $_SESSION['account'];
		$_SESSION['account']->unix_password='';
		$_SESSION['account']->smb_password='';
		$_SESSION['account']->smb_flagsW = 0;
		$_SESSION['account']->general_dn = substr($_SESSION['account']->general_dn, strpos($_SESSION['account']->general_dn, ',')+1);
		$_SESSION['final_changegids'] = '';
		}
	else {
		$_SESSION['account'] = loadUserProfile('default');
		$_SESSION['account'] ->type = 'user';
		$_SESSION['account']->smb_flagsW = 0;
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		$_SESSION['account_old'] = false;
		$values = getquotas('user');
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $_SESSION['account']->$key = $val;
				}
		}
	}
else if (count($_POST)==0) { // Startcondition. useredit.php was called from outside
	$_SESSION['account'] = loadUserProfile('default');
	$_SESSION['account'] ->type = 'user';
	$_SESSION['account']->smb_flagsW = 0;
	if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
	$_SESSION['account_old'] = false;
	$values = getquotas('user');
	if (is_object($values)) {
		while (list($key, $val) = each($values)) // Set only defined values
			if (isset($val)) $_SESSION['account']->$key = $val;
		}
	}


switch ($_POST['select']) { // Select which part of page should be loaded and check values
	// general = startpage, general account paramters
	// unix = page with all shadow-options and password
	// samba = page with all samba-related parameters e.g. smbpassword
	// quota = page with all quota-related parameters e.g. hard file quota
	// personal = page with all personal-related parametergs, e.g. phone number
	// final = last page shown before account is created/modified
	//		if account is modified commands might be ran are shown
	// finish = page shown after account has been created/modified
	case 'general':
		// Write all general values into $_SESSION['account'] if no profile should be loaded
		if (!$_POST['load']) {
			$_SESSION['account']->general_dn = $_POST['f_general_suffix'];
			$_SESSION['account']->general_username = $_POST['f_general_username'];
			$_SESSION['account']->general_surname = $_POST['f_general_surname'];
			$_SESSION['account']->general_givenname = $_POST['f_general_givenname'];
			$_SESSION['account']->general_uidNumber = $_POST['f_general_uidNumber'];
			$_SESSION['account']->general_group = $_POST['f_general_group'];
			if (isset($_POST['f_general_groupadd'])) $_SESSION['account']->general_groupadd = $_POST['f_general_groupadd'];
				else $_SESSION['account']->general_groupadd = array('');
			$_SESSION['account']->general_homedir = $_POST['f_general_homedir'];
			$_SESSION['account']->general_shell = $_POST['f_general_shell'];
			$_SESSION['account']->general_gecos = $_POST['f_general_gecos'];

			// Check if Homedir is valid
			$_SESSION['account']->general_homedir = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->general_homedir);
			if ($_SESSION['account']->general_username != '')
				$_SESSION['account']->general_homedir = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->general_homedir);
			if ($_SESSION['account']->general_homedir != $_POST['f_general_homedir']) $errors[] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
			if ( !ereg('^[/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*([/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account']->general_homedir ))
				$errors[] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['account']->general_givenname)) $errors[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['account']->general_surname)) $errors[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
			if ( ($_SESSION['account']->general_gecos=='') || ($_SESSION['account']->general_gecos==' ')) {
				$_SESSION['account']->general_gecos = $_SESSION['account']->general_givenname . " " . $_SESSION['account']->general_surname ;
				$errors[] = array('INFO', _('Gecos'), _('Inserted sur- and given name in gecos-field.'));
				}
			if ($_SESSION['account']->general_group=='') $errors[] = array('ERROR', _('Primary group'), _('No primary group defined!'));
			// Check if Username contains only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['account']->general_username))
				$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, 0-9 and .-_ !'));
			// Check if user already exists
			if (isset($_SESSION['account']->general_groupadd) && in_array($_SESSION['account']->general_group, $_SESSION['account']->general_groupadd)) {
				for ($i=0; $i<count($_SESSION['account']->general_groupadd); $i++ )
					if ($_SESSION['account']->general_groupadd[$i] == $_SESSION['account']->general_group) {
						unset ($_SESSION['account']->general_groupadd[$i]);
						$_SESSION['account']->general_groupadd = array_values($_SESSION['account']->general_groupadd);
						}
				}
			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			if (ldapexists($_SESSION['account'], 'user', $_SESSION['account_old']) && is_object($_SESSION['account_old']))
				$_SESSION['account']->general_username = $_SESSION['account_old']->general_username;
			while ($temp = ldapexists($_SESSION['account'], 'user', $_SESSION['account_old'])) {
				// get last character of username
				$lastchar = substr($_SESSION['account']->general_username, strlen($_SESSION['account']->general_username)-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
					$_SESSION['account']->general_username = $_SESSION['account']->general_username . '2';
				 else {
				 	$i=strlen($_SESSION['account']->general_username)-1;
					$mark = false;
				 	while (!$mark) {
						if (ereg('^([0-9])+$',substr($_SESSION['account']->general_username, $i, strlen($_SESSION['account']->general_username)-$i))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($_SESSION['account']->general_username, 0, $i+1);
					$lastchars = substr($_SESSION['account']->general_username, $i+1, strlen($_SESSION['account']->general_username)-$i);
					$_SESSION['account']->general_username = $firstchars . (intval($lastchars)+1);
				 	}
				}
			if ($_SESSION['account']->general_username != $_POST['f_general_username']) $errors[] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));

			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$_SESSION['account']->general_uidNumber = checkid($_SESSION['account'], 'user', $_SESSION['account_old']);
			if (is_string($_SESSION['account']->general_uidNumber)) { // true if checkid has returned an error
				$errors[] = array('ERROR', _('ID-Number'), $_SESSION['account']->general_uidNumber);
				unset($_SESSION['account']->general_uidNumber);
				}
			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account']->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $_SESSION['account']->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));

			}
		break;
	case 'unix':
		// Write all general values into $_SESSION['account']
		if (isset($_POST['f_unix_password'])) {
			// Encraypt password
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$_SESSION['account']->unix_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $_POST['f_unix_password'], MCRYPT_MODE_ECB, $iv));
			}
		 else $_SESSION['account']->unix_password = '';
		if ($_POST['f_unix_password_no']) $_SESSION['account']->unix_password_no = true;
			else $_SESSION['account']->unix_password_no = false;
		$_SESSION['account']->unix_pwdwarn = $_POST['f_unix_pwdwarn'];
		$_SESSION['account']->unix_pwdallowlogin = $_POST['f_unix_pwdallowlogin'];
		$_SESSION['account']->unix_pwdmaxage = $_POST['f_unix_pwdmaxage'];
		$_SESSION['account']->unix_pwdminage = $_POST['f_unix_pwdminage'];
		$_SESSION['account']->unix_host = $_POST['f_unix_host'];
		$_SESSION['account']->unix_pwdexpire = mktime(10, 0, 0, $_POST['f_unix_pwdexpire_mon'],
			$_POST['f_unix_pwdexpire_day'], $_POST['f_unix_pwdexpire_yea']);
		if ($_POST['f_unix_deactivated']) $_SESSION['account']->unix_deactivated = $_POST['f_unix_deactivated'];
			else $_SESSION['account']->unix_deactivated = false;
		if ($_POST['genpass']) {
			// Generate a random password if generate-button was pressed
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$_SESSION['account']->unix_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, genpasswd(), MCRYPT_MODE_ECB, $iv));
			// Keep unix-page acitve
			$select_local = 'unix';
			}
			// Check if values are OK and set automatic values. if not error-variable will be set
			else { // account.inc
				if ($_SESSION['account']->unix_password != '') {
					$iv = base64_decode($_COOKIE["IV"]);
					$key = base64_decode($_COOKIE["Key"]);
					$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($_SESSION['account']->unix_password), MCRYPT_MODE_ECB, $iv);
					$password = str_replace(chr(00), '', $password);
					}
				if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $password))
					$errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
				if ( !ereg('^([0-9])*$', $_SESSION['account']->unix_pwdminage))  $errors[] = array('ERROR', _('Password minage'), _('Password minage must be are natural number.'));
				if ( $_SESSION['account']->unix_pwdminage > $_SESSION['account']->unix_pwdmaxage ) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must bigger as Password Minage.'));
				if ( !ereg('^([0-9]*)$', $_SESSION['account']->unix_pwdmaxage)) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must be are natural number.'));
				if ( !ereg('^(([-][1])|([0-9]*))$', $_SESSION['account']->unix_pwdallowlogin))
					$errors[] = array('ERROR', _('Password Expire'), _('Password expire must be are natural number or -1.'));
				if ( !ereg('^([0-9]*)$', $_SESSION['account']->unix_pwdwarn)) $errors[] = array('ERROR', _('Password warn'), _('Password warn must be are natural number.'));
				if ((!$_SESSION['account']->unix_host=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([ ])*([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $_SESSION['account']->unix_host))
					$errors[] = array('ERROR', _('Unix workstations'), _('Unix workstations is invalid.'));
				}

		break;
	case 'samba':
		// Write all general values into $_SESSION['account']
		$_SESSION['account']->smb_pwdcanchange = mktime($_POST['f_smb_pwdcanchange_s'], $_POST['f_smb_pwdcanchange_m'], $_POST['f_smb_pwdcanchange_h'],
			$_POST['f_smb_pwdcanchange_mon'], $_POST['f_smb_pwdcanchange_day'], $_POST['f_smb_pwdcanchange_yea']);
		$_SESSION['account']->smb_pwdmustchange = mktime($_POST['f_smb_pwdmustchange_s'], $_POST['f_smb_pwdmustchange_m'], $_POST['f_smb_pwdmustchange_h'],
			$_POST['f_smb_pwdmustchange_mon'], $_POST['f_smb_pwdmustchange_day'], $_POST['f_smb_pwdmustchange_yea']);
		if ($_POST['f_smb_password_no']) $_SESSION['account']->smb_password_no = true;
			else $_SESSION['account']->smb_password_no = false;
		if ($_POST['f_smb_useunixpwd']) $_SESSION['account']->smb_useunixpwd = $_POST['f_smb_useunixpwd'];
			else $_SESSION['account']->smb_useunixpwd = false;
		$_SESSION['account']->smb_homedrive = $_POST['f_smb_homedrive'];
		$_SESSION['account']->smb_scriptPath = $_POST['f_smb_scriptpath'];
		$_SESSION['account']->smb_smbuserworkstations = $_POST['f_smb_smbuserworkstations'];
		$_SESSION['account']->smb_smbhome = stripslashes($_POST['f_smb_smbhome']);
		$_SESSION['account']->smb_profilePath = stripslashes($_POST['f_smb_profilePath']);
		$_SESSION['account']->smb_displayName = $_POST['f_smb_displayName'];
		if ($_POST['f_smb_flagsW']) $_SESSION['account']->smb_flagsW = true;
			else $_SESSION['account']->smb_flagsW = false;
		if ($_POST['f_smb_flagsD']) $_SESSION['account']->smb_flagsD = true;
			else $_SESSION['account']->smb_flagsD = false;
		if ($_POST['f_smb_flagsX']) $_SESSION['account']->smb_flagsX = true;
			else $_SESSION['account']->smb_flagsX = false;

		if ($_SESSION['config']->samba3 == 'yes') {
			$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
			for ($i=0; $i<sizeof($samba3domains); $i++)
				if ($_POST['f_smb_domain'] == $samba3domains[$i]->name) {
					$_SESSION['account']->smb_domain = $samba3domains[$i];
					}
			}
		else {
			if (isset($_POST['f_smb_domain'])) $_SESSION['account']->smb_domain = $_POST['f_smb_domain'];
				else $_SESSION['account']->smb_domain = '';
			}

		if ($_SESSION['config']->samba3 == 'yes')
			switch ($_POST['f_smb_mapgroup']) {
				case '*'._('Domain Guests'): $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '514'; break;
				case '*'._('Domain Users'): $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '513'; break;
				case '*'._('Domain Admins'): $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '512'; break;
				case $_SESSION['account']->general_group:
						$_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-".
							(2 * getgid($_SESSION['account']->general_group) + $_SESSION['account']->smb_domain->RIDbase +1);
					break;
				}
		else
			switch ($_POST['f_smb_mapgroup']) {
				case '*'._('Domain Guests'): $_SESSION['account']->smb_mapgroup = '514'; break;
				case '*'._('Domain Users'): $_SESSION['account']->smb_mapgroup = '513'; break;
				case '*'._('Domain Admins'): $_SESSION['account']->smb_mapgroup = '512'; break;
				case $_SESSION['account']->general_group:
					$_SESSION['account']->smb_mapgroup = (2 * getgid($_SESSION['account']->general_group) + 1001);
					break;
				}


		$smb_password = $_POST['f_smb_password'];

		// Decrypt unix-password if needed password
		$iv = base64_decode($_COOKIE["IV"]);
		$key = base64_decode($_COOKIE["Key"]);
		if (($values->smb_useunixpwd) &&($values->unix_password != '')) {
			$smb_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($_SESSION['account']->unix_password), MCRYPT_MODE_ECB, $iv);
			$smb_password = str_replace(chr(00), '', $smb_password);
			}
		// Check values
		$_SESSION['account']->smb_scriptPath = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->smb_scriptPath);
		$_SESSION['account']->smb_scriptPath = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->smb_scriptPath);
		if ($_SESSION['account']->smb_scriptPath != $_POST['f_smb_scriptpath']) $errors[] = array('INFO', _('Script path'), _('Inserted user- or groupname in scriptpath.'));

		$_SESSION['account']->smb_profilePath = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->smb_profilePath);
		$_SESSION['account']->smb_profilePath = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->smb_profilePath);
		if ($_SESSION['account']->smb_profilePath != $_POST['f_smb_profilePath']) $errors[] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'));

		$_SESSION['account']->smb_smbhome = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->smb_smbhome);
		$_SESSION['account']->smb_smbhome = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->smb_smbhome);
		if ($_SESSION['account']->smb_smbhome != $_POST['f_smb_smbhome']) $errors[] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'));

		if ( (!$_SESSION['account']->smb_smbhome=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+)+$', $_SESSION['account']->smb_smbhome)))
				$errors[] = array('ERROR', _('Home path'), _('Home path is invalid.'));
		if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
			$smb_password)) $errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		if ( (!$_SESSION['account']->smb_scriptPath=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*'.
			'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*)*$', $_SESSION['account']->smb_scriptPath)))
			$errors[] = array('ERROR', _('Script path'), _('Script path is invalid!'));
		if ( (!$_SESSION['account']->smb_profilePath=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $_SESSION['account']->smb_profilePath))
			&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $_SESSION['account']->smb_profilePath)))
				$errors[] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		if ((!$_SESSION['account']->smb_smbuserworkstations=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $_SESSION['account']->smb_smbuserworkstations))
			$errors[] = array('ERROR', _('Samba workstations'), _('Samba workstations are invalid!'));
		if ((!$_SESSION['account']->smb_domain=='') && (!is_object($_SESSION['account']->smb_domain)) && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $_SESSION['account']->smb_domain))
			$errors[] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));
		if ($_SESSION['account']->smb_useunixpwd) $_SESSION['account']->smb_useunixpwd = 1; else $_SESSION['account']->smb_useunixpwd = 0;

		if (($_SESSION['account']->smb_displayName=='') && isset($_SESSION['account']->general_gecos)) {
			$_SESSION['account']->smb_displayName = $_SESSION['account']->general_gecos;
			$errors[] = array('INFO', _('Display name'), _('Inserted gecos-field as display name.'));
			}

		if ($smb_password!='') {
			// Encrypt password
			$_SESSION['account']->smb_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $smb_password,
			MCRYPT_MODE_ECB, $iv));
			}

		break;
	case 'quota':
		// Write all general values into $_SESSION['account']
		$i=0;
		while ($_SESSION['account']->quota[$i][0]) {
			$_SESSION['account']->quota[$i][2] = $_POST['f_quota_'.$i.'_2'];
			$_SESSION['account']->quota[$i][3] = $_POST['f_quota_'.$i.'_3'];
			$_SESSION['account']->quota[$i][6] = $_POST['f_quota_'.$i.'_6'];
			$_SESSION['account']->quota[$i][7] = $_POST['f_quota_'.$i.'_7'];
			// Check if values are OK and set automatic values. if not error-variable will be set
			if (!ereg('^([0-9])*$', $_SESSION['account']->quota[$i][2]))
				$errors[] = array('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed'));
			if (!ereg('^([0-9])*$', $_SESSION['account']->quota[$i][3]))
				$errors[] = array('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed'));
			if (!ereg('^([0-9])*$', $_SESSION['account']->quota[$i][6]))
				$errors[] = array('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed'));
			if (!ereg('^([0-9])*$', $_SESSION['account']->quota[$i][7]))
				$errors[] = array('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed'));
			$i++;
			}
		break;

	case 'personal':
		// Write all general values into $_SESSION['account']
		$_SESSION['account']->personal_title = $_POST['f_personal_title'];
		$_SESSION['account']->personal_mail = $_POST['f_personal_mail'];
		$_SESSION['account']->personal_telephoneNumber = $_POST['f_personal_telephoneNumber'];
		$_SESSION['account']->personal_mobileTelephoneNumber = $_POST['f_personal_mobileTelephoneNumber'];
		$_SESSION['account']->personal_facsimileTelephoneNumber = $_POST['f_personal_facsimileTelephoneNumber'];
		$_SESSION['account']->personal_street = $_POST['f_personal_street'];
		$_SESSION['account']->personal_postalCode = $_POST['f_personal_postalCode'];
		$_SESSION['account']->personal_postalAddress = $_POST['f_personal_postalAddress'];
		$_SESSION['account']->personal_employeeType = $_POST['f_personal_employeeType'];
		// Check if values are OK and set automatic values. if not error-variable will be set
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['account']->personal_telephoneNumber))  $errors[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['account']->personal_mobileTelephoneNumber))  $errors[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['account']->personal_facsimileTelephoneNumber))  $errors[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $_SESSION['account']->personal_mail))  $errors[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account']->personal_street))  $errors[] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account']->personal_postalAddress))  $errors[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account']->personal_title))  $errors[] = array('ERROR', _('Title'), _('Please enter a valid title!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account']->personal_employeeType))  $errors[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $_SESSION['account']->personal_postalCode))  $errors[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		break;

	case 'final':
		// Write all general values into $_SESSION['account']
		if ($_POST['f_final_changegids']) $_SESSION['final_changegids'] = $_POST['f_final_changegids'] ;
		// Check which part Site should be displayed next
		break;
	case 'finish':
		// Check if pdf-file should be created
		if ($_POST['outputpdf']) {
			createUserPDF(array($_SESSION['account']));
			$select_local = 'pdf';
			}
		break;
	}

if ($select_local != 'pdf') {
	// Write HTML-Header
	echo $_SESSION['header'];
	echo "<html><head><title>";
	echo _("Create new Account");
	echo "</title>\n".
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n".
		"<meta http-equiv=\"pragma\" content=\"no-cache\">\n".
		"<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
	}


do { // X-Or, only one if() can be true
	if ($_POST['next_general']) {
		if (!is_array($errors)) $select_local='general';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_unix']) {
		if (!is_array($errors)) $select_local='unix';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_samba']) {
		if (!is_array($errors)) $select_local='samba';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_quota']) {
		if (!is_array($errors)) $select_local='quota';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_personal']) {
		if (!is_array($errors)) $select_local='personal';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_final']) {
		if (!is_array($errors)) $select_local='final';
			else $select_local=$_POST['select'];
		break;
		}
	if ( $_POST['create'] ) { // Create-Button was pressed
		// Create or modify an account
		if ($_SESSION['account_old']) $result = modifyuser($_SESSION['account'],$_SESSION['account_old']);
			 else $result = createuser($_SESSION['account']); // account.inc
		if ( $result==1 || $result==3 ) $select_local = 'finish';
			else $select_local = 'final';
		break;
		}
	if ($_POST['createagain']) {
		$select_local='general';
		unset($_SESSION['account']);
		$_SESSION['account'] = loadUserProfile('default');
		$_SESSION['account'] ->type = 'user';
		break;
		}
	if ($_POST['load']) {
		// load profile
		if ($_POST['f_general_selectprofile']!='') $values = loadUserProfile($_POST['f_general_selectprofile']);
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $_SESSION['account']->$key = $val;
			}
		// insert autoreplace values
		$_SESSION['account']->general_homedir = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->general_homedir);
		if ($_SESSION['account']->general_username != '')
			$_SESSION['account']->general_homedir = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->general_homedir);
		$_SESSION['account']->smb_scriptPath = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->smb_scriptPath);
		if ($_SESSION['account']->general_username != '')
			$_SESSION['account']->smb_scriptPath = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->smb_scriptPath);

		$_SESSION['account']->smb_profilePath = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->smb_profilePath);
		if ($_SESSION['account']->general_username != '')
			$_SESSION['account']->smb_profilePath = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->smb_profilePath);

		$_SESSION['account']->smb_smbhome = str_replace('$group', $_SESSION['account']->general_group, $_SESSION['account']->smb_smbhome);
		if ($_SESSION['account']->general_username != '')
			$_SESSION['account']->smb_smbhome = str_replace('$user', $_SESSION['account']->general_username, $_SESSION['account']->smb_smbhome);

		// select general page after group has been loaded
		$select_local='general';
		break;
		}
	if ($_POST['save']) {
		// save profile
		saveUserProfile($_SESSION['account'], $_POST['f_finish_safeProfile']);
		// select last page displayed before user is created
		$select_local='final';
		break;
		}
	if ($_POST['backmain']) {
		echo "<meta http-equiv=\"refresh\" content=\"2; URL=../lists/listusers.php\">\n";
		$select_local='backmain';
		break;
		}
	if (!$select_local) $select_local='general';
	} while(0);


if ($select_local != 'pdf') {
	echo "</head><body>\n";
	echo "<form action=\"useredit.php\" method=\"post\">\n";

	if (is_array($errors)) {
		echo "<table class=\"account\" width=\"100%\">\n";
		for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);
		echo "</table>";
		}
	}

// print_r($_SESSION['account']);
// print_r($_POST);

switch ($select_local) { // Select which part of page will be loaded
	// general = startpage, general account paramters
	// unix = page with all shadow-options and password
	// samba = page with all samba-related parameters e.g. smbpassword
	// quota = page with all quota-related parameters e.g. hard file quota
	// personal = page with all personal-related parametergs, e.g. phone number
	// final = last page shown before account is created/modified
	//		if account is modified commands might be ran are shown
	// finish = page shown after account has been created/modified
	case 'general':
		// General Account Settings
		// load list of all groups
		$groups = findgroups();
		$profilelist = getUserProfiles();
		// Show page info
		// Show page info
		echo '<input name="select" type="hidden" value="general">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" disabled value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td></tr></table></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		echo _("General properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _('Username').'*';
		echo "</td>\n<td>".
			'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=400" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('UID number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $_SESSION['account']->general_uidNumber . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=401" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Surname').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_surname" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_surname . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=424" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Given name').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_givenname" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_givenname . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=425" target="lamhelp">'._('Help').'</a>'.
			'</td>'."\n".'</tr>'."\n".'<tr><td>';
		echo _('Primary group').'*';
		echo '</td>'."\n".'<td><select name="f_general_group">';
		// loop trough existing groups
		foreach ($groups as $group) {
			if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group. '</option>';
			else echo '<option>' . $group. '</option>';
			 }
		echo '</select></td><td>'.
			'<a href="help.php?HelpNumber=406" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Additional groups');
		echo '</td>'."\n".'<td><select name="f_general_groupadd[]" size="3" multiple>';
		// loop though existing groups for additional groups
		foreach ($groups as $group) {
			if ($_SESSION['account']->general_groupadd) {
				if (in_array($group, $_SESSION['account']->general_groupadd)) echo '<option selected>'.$group. '</option>';
				else echo '<option>'.$group. '</option>';
				}
			else echo '<option>'.$group. '</option>';
			}
		echo	'</select></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=402" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Home directory').'*';
		echo '</td>'."\n".'<td><input name="f_general_homedir" type="text" size="30" value="' . $_SESSION['account']->general_homedir . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=403" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Gecos');
		echo '</td>'."\n".'<td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=404" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Login shell').'*';
		echo '</td>'."\n".'<td><select name="f_general_shell" >';
			// loop through shells
			foreach ($_SESSION['shelllist'] as $shell)
				if ($_SESSION['account']->general_shell==trim($shell)) echo '<option selected>'.$shell. '</option>';
					else echo '<option>'.$shell. '</option>';
		echo '</select></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=405" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
		foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_UserSuffix()) as $suffix) {
			if ($_SESSION['account']->general_dn) {
				if ($_SESSION['account']->general_dn == $suffix)
					echo '<option selected>' . $suffix. '</option>';
				else echo '<option>' . $suffix. '</option>';
				}
			else echo '<option>' . $suffix. '</option>';
			}
		echo '</select></td><td><a href="help.php?HelpNumber=461" target="lamhelp">'._('Help').
			"</a></td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr><tr><td>";
		if (count($profilelist)!=0) {
			echo "<fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
			echo _("Load profile");
			echo "</b></legend>\n<table border=0>\n<tr>\n<td>";
			echo "<select name=\"f_general_selectprofile\" >";
			foreach ($profilelist as $profile) echo "	<option>$profile</option>\n";
			echo "</select>\n".
				"<input name=\"load\" type=\"submit\" value=\""; echo _('Load Profile');
			echo "\"></td><td><a href=\"../help.php?HelpNumber=421\" target=\"lamhelp\">";
			echo _('Help')."</a></td>\n</tr>\n</table>\n</fieldset>\n";
			}
		echo "</td></tr>\n</table>\n</td></tr></table>\n";

		break;
	case 'unix':
		// Unix Password Settings
		// decrypt password
		if ($_SESSION['account']->unix_password != '') {
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($_SESSION['account']->unix_password), MCRYPT_MODE_ECB, $iv);
			$password = str_replace(chr(00), '', $password);
			}
		 else $password='';
		$date = getdate ($_SESSION['account']->unix_pwdexpire);
		echo "<input name=\"select\" type=\"hidden\" value=\"unix\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" disabled value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td></tr></table></td>\n<td valign=\"top\">";
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>"._('Unix properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Password');
		echo '</td>'."\n".'<td>'.
			'<input name="f_unix_password" type="text" size="20" maxlength="20" value="' . $password . '">'.
			'</td>'."\n".'<td>'.
			'<input name="genpass" type="submit" value="';
		echo _('Generate password'); echo '"></td></tr><tr><td>';
		echo _('Use no password');
		echo '</td>'."\n".'<td><input name="f_unix_password_no" type="checkbox"';
		if ($_SESSION['account']->unix_password_no) echo ' checked ';
		echo '></td>'."\n".'<td>'.
		'<a href="help.php?HelpNumber=426" target="lamhelp">'._('Help').'</a>'.
		'</td></tr>'."\n".'<tr><td>';
		echo _('Password warn');
		echo '</td>'."\n".'<td><input name="f_unix_pwdwarn" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdwarn . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=414" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Password expire');
		echo '</td>'."\n".'<td><input name="f_unix_pwdallowlogin" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdallowlogin . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=415" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Maximum password age');
		echo '</td>'."\n".'<td><input name="f_unix_pwdmaxage" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->unix_pwdmaxage . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=416" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Minimum password age');
		echo '</td>'."\n".'<td><input name="f_unix_pwdminage" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdminage . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=417" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Expire date');
		echo '</td>'."\n".'<td><select name="f_unix_pwdexpire_day">';
		for ( $i=1; $i<=31; $i++ ) {
			if ($date['mday']==$i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select><select name="f_unix_pwdexpire_mon">';
		for ( $i=1; $i<=12; $i++ ) {
			if ($date['mon'] == $i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select><select name="f_unix_pwdexpire_yea">';
		for ( $i=2030; $i>=2003; $i-- ) {
			if ($date['year']==$i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=418" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Account deactivated');
		echo '</td>'."\n".'<td><input name="f_unix_deactivated" type="checkbox"';
		if ($_SESSION['account']->unix_deactivated) echo ' checked ';
		echo '></td>'."\n".'<td>'.
		'<a href="help.php?HelpNumber=427" target="lamhelp">'._('Help').'</a>'.
		'</td></tr>'."\n".'<tr><td>';
		echo _('Unix workstations');
		echo '</td>'."\n".'<td><input name="f_unix_host" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->unix_host . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=466" target="lamhelp">'._('Help').
			"</a></td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr></table></td></tr>\n</table>\n";


		break;
	case 'samba':
		// Samba Settings
		// decrypt password
		if ($_SESSION['account']->smb_password != '') {
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($_SESSION['account']->smb_password), MCRYPT_MODE_ECB, $iv);
			$password = str_replace(chr(00), '', $password);
			}
		if ($_SESSION['config']->samba3 == 'yes') $samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
		$canchangedate = getdate($_SESSION['account']->smb_pwdcanchange);
		$mustchangedate = getdate($_SESSION['account']->smb_pwdmustchange);

		echo '<input name="select" type="hidden" value="samba">';
		echo	'<input name="f_smb_pwdcanchange_h" type="hidden" value="'.$canchangedate['hours'].'">'.
			'<input name="f_smb_pwdcanchange_m" type="hidden" value="'.$canchangedate['minutes'].'">'.
			'<input name="f_smb_pwdcanchange_s" type="hidden" value="'.$canchangedate['seconds'].'">'.
			'<input name="f_smb_pwdmustchange_h" type="hidden" value="'.$mustchangedate['hours'].'">'.
			'<input name="f_smb_pwdmustchange_m" type="hidden" value="'.$mustchangedate['minutes'].'">'.
			'<input name="f_smb_pwdmustchange_s" type="hidden" value="'.$mustchangedate['seconds'].'">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" disabled value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td></tr></table></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		echo _("Samba properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _("Display name");
		echo "</td>\n<td>".
			"<input name=\"f_smb_displayName\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"".$_SESSION['account']->smb_displayName."\">".
			"</td>\n<td><a href=\"../help.php?HelpNumber=420\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Samba password');
		echo '</td>'."\n".'<td><input name="f_smb_password" type="text" size="20" maxlength="20" value="' . $password . '">'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Use unix password');
		echo '</td><td><input name="f_smb_useunixpwd" type="checkbox"';
		if ($_SESSION['account']->smb_useunixpwd) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=428" target="lamhelp">'._('Help').'</a>';
		echo '</td></tr>'."\n".'<tr><td>';
		echo _('Use no password');
		echo '</td>'."\n".'<td><input name="f_smb_password_no" type="checkbox"';
		if ($_SESSION['account']->smb_password_no) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=426" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Password does not expire');
		echo '</td>'."\n".'<td><input name="f_smb_flagsX" type="checkbox"';
		if ($_SESSION['account']->smb_flagsX) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=429" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('User can change password');
		echo '</td>'."\n".'<td><select name="f_smb_pwdcanchange_day">';
		for ( $i=1; $i<=31; $i++ ) {
			if ($canchangedate['mday']==$i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select><select name="f_smb_pwdcanchange_mon">';
		for ( $i=1; $i<=12; $i++ ) {
			if ($canchangedate['mon'] == $i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select><select name="f_smb_pwdcanchange_yea">';
		for ( $i=2003; $i<=2030; $i++ ) {
			if ($canchangedate['year']==$i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select></td>'."\n".'<td>';
		echo	'<a href="help.php?HelpNumber=430" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('User must change password');
		echo '</td>'."\n".'<td><select name="f_smb_pwdmustchange_day">';
		for ( $i=1; $i<=31; $i++ ) {
			if ($mustchangedate['mday']==$i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select><select name="f_smb_pwdmustchange_mon">';
		for ( $i=1; $i<=12; $i++ ) {
			if ($mustchangedate['mon'] == $i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select><select name="f_smb_pwdmustchange_yea">';
		for ( $i=2030; $i>=2003; $i-- ) {
			if ($mustchangedate['year']==$i) echo "<option selected> $i". '</option>';
			else echo "<option> $i". '</option>';
			}
		echo '</select></td>'."\n".'<td>';
		echo	'<a href="help.php?HelpNumber=431" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Account is deactivated');
		echo '</td>'."\n".'<td><input name="f_smb_flagsD" type="checkbox"';
		if ($_SESSION['account']->smb_flagsD) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=432" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Home drive');
		echo '</td>'."\n".'<td><select name="f_smb_homedrive" >';
			for ($i=68; $i<91; $i++)
				if ($_SESSION['account']->smb_homedrive== chr($i).':') echo '<option selected> '.chr($i).':</option>'; else echo '<option> '.chr($i).':</option>';
		echo	'</select></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=433" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Home path');
		echo '</td>'."\n".'<td><input name="f_smb_smbhome" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_smbhome . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=437" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Profile path');
		echo '</td>'."\n".'<td><input name="f_smb_profilePath" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_profilePath . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=435" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Script path');
		echo '</td>'."\n".'<td><input name="f_smb_scriptpath" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_scriptPath . '">'.
		'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=434" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Samba workstations');
		echo '</td>'."\n".'<td><input name="f_smb_smbuserworkstations" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_smbuserworkstations . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=436" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Windows groupname');
		echo '</td>'."\n".'<td><select name="f_smb_mapgroup" >';
		if ($_SESSION['config']->samba3=='yes') {
			if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-".
			(2 * getgid($_SESSION['account']->general_group) + $values->smb_domain->RIDbase+1)) {
				echo '<option selected> ';
				echo $_SESSION['account']->general_group;
				echo "</option>\n"; }
			 else {
				echo '<option> ';
				echo $_SESSION['account']->general_group;
				echo "</option>\n";
				}
			if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '514' ) {
				echo '<option selected> *';
				echo _('Domain Guests');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Guests');
				echo "</option>\n";
				}
			if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '513' ) {
				echo '<option selected> *';
				echo _('Domain Users');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Users');
				echo "</option>\n";
				}
			if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '512' ) {
				echo '<option selected> *';
				echo _('Domain Admins');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Admins');
				echo "</option>\n";
				}
			}
		else {
			if ( $_SESSION['account']->smb_mapgroup == (2 * getgid($_SESSION['account']->general_group) +1001)) {
				echo '<option selected> ';
				echo $_SESSION['account']->general_group;
				echo "</option>\n"; }
			 else {
				echo '<option> ';
				echo $_SESSION['account']->general_group;
				echo "</option>\n";
				}
			if ( $_SESSION['account']->smb_mapgroup == '514' ) {
				echo '<option selected> *';
				echo _('Domain Guests');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Guests');
				echo "</option>\n";
				}
			if ( $_SESSION['account']->smb_mapgroup == '513' ) {
				echo '<option selected> *';
				echo _('Domain Users');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Users');
				echo "</option>\n";
				}
			if ( $_SESSION['account']->smb_mapgroup == '512' ) {
				echo '<option selected> *';
				echo _('Domain Admins');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Admins');
				echo "</option>\n";
				}
			}
		echo	'</select></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=464" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Domain');
		if ($_SESSION['config']->samba3 == 'yes') {
			echo '</td><td><select name="f_smb_domain">';
			for ($i=0; $i<sizeof($samba3domains); $i++) {
				if ($_SESSION['account']->smb_domain->name) {
					if ($_SESSION['account']->smb_domain->name == $samba3domains[$i]->name)
						echo '<option selected>' . $samba3domains[$i]->name. '</option>';
					else echo '<option>' . $samba3domains[$i]->name. '</option>';
					}
				else echo '<option>' . $samba3domains[$i]->name. '</option>';
				}
			echo '</select>';
			}
		else {
			echo '</td>'."\n".'<td><input name="f_smb_domain" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_domain . '">';
			}
		echo	'</td>'."\n".'<td><a href="help.php?HelpNumber=438" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;
	case 'quota':
		// Quota Settings
		echo "<input name=\"select\" type=\"hidden\" value=\"quota\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\" disabled value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td></tr></table></td>\n<td valign=\"top\">";
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>"._('Quota properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Mountpoint'); echo '</td>'."\n".'<td>'; echo _('Used blocks'); echo '</td>'."\n".'<td>';
		echo _('Soft block limit'); echo '</td>'."\n".'<td>'; echo _('Hard block limit'); echo '</td>'."\n".'<td>'; echo _('Grace block period');
		echo '</td>'."\n".'<td>'; echo _('Used inodes'); echo '</td>'."\n".'<td>'; echo _('Soft inode limit'); echo '</td>'."\n".'<td>';
		echo _('Hard inode limit'); echo '</td>'."\n".'<td>'; echo _('Grace inode period'); echo '</td></tr>'."\n";
		echo '<tr><td><a href="help.php?HelpNumber=439" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=440" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=441" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=442" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=443" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=444" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=445" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=446" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=447" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		$i=0;
		while ($_SESSION['account']->quota[$i][0]) {
			echo '<tr><td>'.$_SESSION['account']->quota[$i][0].'</td><td>'.$_SESSION['account']->quota[$i][1].'</td>'; // used blocks
			echo '<td><input name="f_quota_'.$i.'_2" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][2].'"></td>'; // blocks soft limit
			echo '<td><input name="f_quota_'.$i.'_3" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][3].'"></td>'; // blocks hard limit
			echo '<td>'.$_SESSION['account']->quota[$i][4].'</td>'; // block grace period
			echo '<td>'.$_SESSION['account']->quota[$i][5].'</td>'; // used inodes
			echo '<td><input name="f_quota_'.$i.'_6" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][6].'"></td>'; // inodes soft limit
			echo '<td><input name="f_quota_'.$i.'_7" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][7].'"></td>'; // inodes hard limit
			echo '<td>'.$_SESSION['account']->quota[$i][8].'</td></tr>'; // inodes grace period
			$i++;
			}
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;

	case 'personal':
		// Personal Settings
		echo "<input name=\"select\" type=\"hidden\" value=\"personal\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" disabled value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td></tr></table></td>\n<td valign=\"top\">";
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>"._('Personal properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Title');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_title" type="text" size="10" maxlength="10" value="' . $_SESSION['account']->personal_title . '"> ';
		echo $_SESSION['account']->general_surname . ' ' . $_SESSION['account']->general_givenname . '</td><td>'.
			'<a href="help.php?HelpNumber=448" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Employee type');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_employeeType" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_employeeType . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=449" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Street');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_street" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_street . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=450" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Postal code');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_postalCode" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->personal_postalCode . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=451" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Postal address');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_postalAddress" type="text" size="30" maxlength="80" value="' . $_SESSION['account']->personal_postalAddress . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=452" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Telephone number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_telephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_telephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=453" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Mobile number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mobileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_mobileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=454" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Fax number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_facsimileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_facsimileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=455" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('eMail address');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mail" type="text" size="30" maxlength="80" value="' . $_SESSION['account']->personal_mail . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=456" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;
	case 'final':
		// Final Settings
		$disabled = "";
		if ($_SESSION['config']->samba3 == 'yes') {
			if (!isset($_SESSION['account']->smb_domain)) { // Samba page nit viewd; can not create group because if missing options
				$disabled = "disabled";
				}
			}
		echo '<input name="select" type="hidden" value="final">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" disabed value=\""; echo _('Final');
		echo "\"></fieldset></td></tr></table></td>\n<td valign=\"top\">";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _("Save profile");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo '<input name="f_finish_safeProfile" type="text" size="30" maxlength="50">';
		echo "</td><td><input name=\"save\" type=\"submit\" $disabled value=\"";
		echo _('Save profile');
		echo '"></td><td><a href="../help.php?HelpNumber=457" target="lamhelp">'._('Help');
		echo "</a></td>\n</tr>\n</table>\n</fieldset>\n</td></tr>\n<tr><td>\n";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		if ($_SESSION['account_old']) echo _('Modify');
		 else echo _('Create');
		echo "</b></legend>\n";
		echo "<table border=0 width=\"100%\">";
		if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
			echo '<tr>';
			StatusMessage ('INFO', _('UID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
			'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chown ' . $_SESSION['account']->general_uidNumber . ' {} \;');
			echo '</tr>'."\n";
			}
		if (($_SESSION['account_old']) && ($_SESSION['account']->general_homedir != $_SESSION['account_old']->general_homedir)) {
			echo '<tr>';
			StatusMessage ('INFO', _('Home Directory has changed. You have to run the following command as root in order to change the existing homedirectory:'),
			'mv ' . $_SESSION['account_old' ]->general_homedir . ' ' . $_SESSION['account']->general_homedir);
			echo '</tr>'."\n";
			}

		if ($disabled=='disabled') { // Samba page nit viewd; can not create group because if missing options
			echo "<tr>";
			StatusMessage("ERROR", _("Samba Options not set!"), _("Please check settings on samba page."));
			echo "</tr>";
			}

		else {
			$found = false;
			if (strstr($_SESSION['account']->smb_scriptPath, '$group')) $found = true;
			if (strstr($_SESSION['account']->smb_scriptPath, '$user')) $found = true;
			if (strstr($_SESSION['account']->smb_profilePath, '$group')) $found = true;
			if (strstr($_SESSION['account']->smb_profilePath, '$user')) $found = true;
			if (strstr($_SESSION['account']->smb_smbhome, '$group')) $found = true;
			if (strstr($_SESSION['account']->smb_smbhome, '$user')) $found = true;
			if ($found) { // Samba page nit viewd; can not create group because if missing options
				$disabled = "disabled";
				echo "<tr>";
				StatusMessage("ERROR", _("Samba Options not set!"), _("Please check settings on samba page."));
				echo "</tr>";
				}
			}

		if (isset($_SESSION['account_old']->general_objectClass)) {
			if (!in_array('posixAccount', $_SESSION['account_old']->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass posixAccount not found.'), _('Have to add objectClass posixAccount.'));
				echo "</tr>\n";
				}
			if (!in_array('shadowAccount', $_SESSION['account_old']->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass shadowAccount.'), _('Have to add objectClass shadowAccount.'));
				echo "</tr>\n";
				}
			if ($_SESSION['config']->samba3 == 'yes') {
				if (!in_array('sambaSamAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass sambaSamAccount not found.'), _('Have to add objectClass sambaSamAccount. USer with sambaAccount will be updated.'));
					echo "</tr>\n";
					}}
				else
				if (!in_array('sambaAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass sambaAccount not found.'), _('Have to add objectClass sambaAccount. User with sambaSamAccount will be set back to sambaAccount.'));
					echo "</tr>\n";
					}
			}




		echo "<tr><td><input name=\"create\" $disabled type=\"submit\" value=\"";
		if ($_SESSION['account_old']) echo _('Modify Account');
		 else echo _('Create Account');
		echo '">'."\n";
		echo "</td></tr></table></fieldset>\n</td></tr></table></td></tr></table>\n</tr></table>";
		break;
	case 'finish':
		// Final Settings
		echo '<input name="select" type="hidden" value="finish">';
		echo "<fieldset class=\"groupedit-bright\"><legend class=\"useredit-bright\"><b>"._('Success')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo '<tr><td>';
		echo _('User ');
		echo $_SESSION['account']->general_username;
		if ($_SESSION['account_old']) echo ' '._('has been modified').'.';
		 else echo ' '._('has been created').'.';
		echo '</td></tr>'."\n".'<tr><td>';
		if (!$_SESSION['account_old'])
			{ echo '<input name="createagain" type="submit" value="'; echo _('Create another user'); echo '">'; }
		echo '</td>'."\n".'<td>'.
			'<input name="outputpdf" type="submit" value="'; echo _('Create PDF file'); echo '">'.
			'</td>'."\n".'<td>'.
			'<input name="backmain" type="submit" value="'; echo _('Back to user list'); echo '">'.
			'</td></tr></table></fieldset'."\n";
		break;
	case 'backmain':
		// unregister sessionvar and select which list should be shown
		echo '<tr><td><a href="../lists/listusers.php">';
		echo _('Please press here if meta-refresh didn\'t work.');
		echo "</a></td></tr>\n";
		if (isset($_SESSION['shelllist'])) unset($_SESSION['shelllist']);
		if (isset($_SESSION['account'])) unset($_SESSION['account']);
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		break;
	}

// Print end of HTML-Page
if ($select_local != 'pdf')
	echo '</form></body></html>';
?>
