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

// include all needed files
include_once('../../lib/account.inc'); // File with all account-funtions
include_once('../../lib/config.inc'); // File with configure-functions
include_once('../../lib/profiles.inc'); // functions to load and save profiles
include_once('../../lib/status.inc'); // Return error-message
include_once('../../lib/pdf.inc'); // Return a pdf-file
include_once('../../lib/ldap.inc'); // LDAP-functions

// Start session
session_save_path('../../sess');
@session_start();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	metaRefresh("../login.php");
	die;
	}

// Set correct language, codepages, ....
setlanguage();

/* groupedit.php is using dynamic session varialenames so
* we can run several copies of groupedit.php at the same
* time
* $varkey is the dynamic part of the variable name
*/
if (!isset($_POST['varkey'])) $varkey = session_id().time();
	else $varkey = $_POST['varkey'];

// Register Session Vars
if (!isset($_SESSION['account_'.$varkey.'_account_new'])) $_SESSION['account_'.$varkey.'_account_new'] = new account();
if (!isset($_SESSION['account_'.$varkey.'_final_changegids'])) $_SESSION['account_'.$varkey.'_final_changegids'] = '';
if (!isset($_SESSION['account_'.$varkey.'_shelllist'])) $_SESSION['account_'.$varkey.'_shelllist'] = getshells();

// Register Session-Variables with references so we don't net to change to complete code if names changes
$account_new =& $_SESSION['account_'.$varkey.'_account_new'];
$shelllist =& $_SESSION['account_'.$varkey.'_shelllist'];
if (is_object($_SESSION['account_'.$varkey.'_account_old'])) $account_old =& $_SESSION['account_'.$varkey.'_account_old'];
$ldap_intern =& $_SESSION['ldap'];
$config_intern =& $_SESSION['config'];
$header_intern =& $_SESSION['header'];
$hostDN_intern =& $_SESSION['hostDN'];
$groupDN_intern =& $_SESSION['groupDN'];

// $_GET is only valid if useredit.php was called from userlist.php
if (isset($_GET['DN']) && $_GET['DN']!='') {
	// useredit.php should edit an existing account
	// reset variables
	if (isset($_SESSION['account_'.$varkey.'_account_old'])) {
		unset($account_old);
		unset($_SESSION['account_'.$varkey.'_account_old']);
		}
	$_SESSION['account_'.$varkey.'_account_old'] = new account();
	$account_old =& $_SESSION['account_'.$varkey.'_account_old'];
	// get "real" DN from variable
	$DN = str_replace("\'", '',$_GET['DN']);
	// Load existing group
	$account_new = loaduser($DN);
	$account_new ->type = 'user';
	$account_old = $account_new;
	$account_new->unix_password='';
	$account_new->smb_password='';
	$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
	// Display general-page
	$select_local = 'general';
	}
// Startcondition. useredit.php was called from outside to create a new group
 else if (count($_POST)==0) {
	// Create new account object with settings from default profile
	$account_new = loadUserProfile('default');
	$account_new ->type = 'user';
	if ($config_intern->scriptServer) {
		// load quotas and check if quotas from profile are valid
		$values = getquotas('user');
		if (isset($account_new->quota[0])) {
			 // check quotas from profile
			$i=0;
			// check quota settings, loop for every partition with quotas
			while (isset($account_new->quota[$i])) {
				// search if quotas from profile fit to a real quota
				$found = (-1);
				for ($j=0; $j<count($values->quota); $j++)
					if ($values->quota[$j][0]==$account_new->quota[$i][0]) $found = $j;
				// unset quota from profile if quotas (mointpoint) doesn't exists anymore
				if ($found==-1) unset($account_new->quota[$i]);
				else {
					// Set missing part in quota-array
					$account_new->quota[$i][1] = $values->quota[$found][1];
					$account_new->quota[$i][5] = $values->quota[$found][5];
					$account_new->quota[$i][4] = $values->quota[$found][4];
					$account_new->quota[$i][8] = $values->quota[$found][8];
					$i++;
					}
				}
			// Beautify array, repair index
			$account_new->quota = array_values($account_new->quota);
			}
		else { // No quotas saved in profile
			// Display quotas for new users (Quota set to 0)
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $account_new->$key = $val;
				}
			}
		}
	// Display general-page
	$select_local = 'general';
	}


switch ($_POST['select']) {
	/* Select which part of page should be loaded and check values
	* groups = page with all groups to which user is additional member
	* workstations = page with all workstations the user is allowed to login
	* general = startpage, general account paramters
	* samba = page with all samba-related parameters e.g. smbpassword
	* quota = page with all quota-related parameters e.g. hard file quota
	* personal = page with all personal-related parametergs, e.g. phone number
	* final = last page shown before account is created/modified
	* finish = page shown after account has been created/modified
	*/
	case 'groups':
		do { // X-Or, only one if() can be true
			if (isset($_POST['allgroups']) && isset($_POST['add'])) { // Add groups to list
				// Add new group
				$account_new->general_groupadd = @array_merge($account_new->general_groupadd, $_POST['allgroups']);
				// remove doubles
				$account_new->general_groupadd = @array_flip($account_new->general_groupadd);
				array_unique($account_new->general_groupadd);
				$account_new->general_groupadd = @array_flip($account_new->general_groupadd);
				// sort groups
				sort($account_new->general_groupadd);
				break;
				}
			if (isset($_POST['selectedgroups']) && isset($_POST['remove'])) { // remove groups from list
				$account_new->general_groupadd = array_delete($_POST['selectedgroups'], $account_new->general_groupadd);
				break;
				}
			} while(0);
		// display group page
		$select_local = 'groups';
		break;
	case 'workstations':
		do { // X-Or, only one if() can be true
			if (isset($_POST['hosts']) && isset($_POST['add'])) { // Add workstations to list
				$temp = str_replace(' ', '', $account_new->smb_smbuserworkstations);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++)
					if ($workstations[$i]=='') unset($workstations[$i]);
				$workstations = array_values($workstations);
				// Add new // Add workstations
				$workstations = array_merge($workstations, $_POST['hosts']);
				// remove doubles
				$workstations = array_flip($workstations);
				array_unique($workstations);
				$workstations = array_flip($workstations);
				// sort workstations
				sort($workstations);
				// Recreate workstation string
				$account_new->smb_smbuserworkstations = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$account_new->smb_smbuserworkstations = $account_new->smb_smbuserworkstations . ", " . $workstations[$i];
					}
				break;
				}
			if (isset($_POST['members']) && isset($_POST['remove'])) { // remove // Add workstations from list
				// Put all workstations in array
				$temp = str_replace(' ', '', $account_new->smb_smbuserworkstations);
				$workstations = explode (',', $temp);
				for ($i=0; $i<count($workstations); $i++)
					if ($workstations[$i]=='') unset($workstations[$i]);
				$workstations = array_values($workstations);
				// Remove unwanted workstations from array
				$workstations = array_delete($_POST['members'], $workstations);
				// Recreate workstation string
				$account_new->smb_smbuserworkstations = $workstations[0];
				for ($i=1; $i<count($workstations); $i++) {
					$account_new->smb_smbuserworkstations = $account_new->smb_smbuserworkstations . ", " . $workstations[$i];
					}
				break;
				}
			} while(0);
		// display workstations page
		$select_local = 'workstations';
		break;
	case 'general':
		if (!$_POST['load']) {
			// Write all general values into $account_new if no profile should be loaded
			$account_new->general_dn = $_POST['f_general_suffix'];
			$account_new->general_username = $_POST['f_general_username'];
			$account_new->general_surname = $_POST['f_general_surname'];
			$account_new->general_givenname = $_POST['f_general_givenname'];
			$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
			$account_new->general_group = $_POST['f_general_group'];
			$account_new->general_homedir = $_POST['f_general_homedir'];
			$account_new->general_shell = $_POST['f_general_shell'];
			$account_new->general_gecos = $_POST['f_general_gecos'];
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $account_new->general_givenname)) $errors[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $account_new->general_surname)) $errors[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
			if ( ($account_new->general_gecos=='') || ($account_new->general_gecos==' ')) {
				$account_new->general_gecos = $account_new->general_givenname . " " . $account_new->general_surname ;
				$errors[] = array('INFO', _('Gecos'), _('Inserted sur- and given name in gecos-field.'));
				}
			if ($account_new->general_group=='') $errors[] = array('ERROR', _('Primary group'), _('No primary group defined!'));
			// Check if Username contains only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, 0-9 and .-_ !'));
			// Check if user already exists
			// Remove primary group from additional groups if set.
			if (isset($account_new->general_groupadd) && in_array($account_new->general_group, $account_new->general_groupadd)) {
				for ($i=0; $i<count($account_new->general_groupadd); $i++ )
					if ($account_new->general_groupadd[$i] == $account_new->general_group) {
						unset ($account_new->general_groupadd[$i]);
						$account_new->general_groupadd = array_values($account_new->general_groupadd);
						}
				}
			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			if (ldapexists($account_new, $account_old) && is_object($account_old))
				$account_new->general_username = $account_old->general_username;
			while ($temp = ldapexists($account_new, $account_old)) {
				// get last character of username
				$lastchar = substr($account_new->general_username, strlen($account_new->general_username)-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
					/* Last character is no number. Therefore we only have to
					* add "2" to it.
					*/
					$account_new->general_username = $account_new->general_username . '2';
				 else {
					/* Last character is a number -> we have to increase the number until we've
					* found a groupname with trailing number which is not in use.
					*
					* $i will show us were we have to split groupname so we get a part
					* with the groupname and a part with the trailing number
					*/
				 	$i=strlen($account_new->general_username)-1;
					$mark = false;
					// Set $i to the last character which is a number in $account_new->general_username
				 	while (!$mark) {
						if (ereg('^([0-9])+$',substr($account_new->general_username, $i, strlen($account_new->general_username)-$i))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($account_new->general_username, 0, $i+1);
					$lastchars = substr($account_new->general_username, $i+1, strlen($account_new->general_username)-$i);
					// Put username together
					$account_new->general_username = $firstchars . (intval($lastchars)+1);
				 	}
				}
			// Show warning if lam has changed username
			if ($account_new->general_username != $_POST['f_general_username']) $errors[] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
			// Check if Homedir is valid
			$account_new->general_homedir = str_replace('$group', $account_new->general_group, $account_new->general_homedir);
			if ($account_new->general_username != '')
				$account_new->general_homedir = str_replace('$user', $account_new->general_username, $account_new->general_homedir);
			if ($account_new->general_homedir != $_POST['f_general_homedir']) $errors[] = array('INFO', _('Home directory'), _('Replaced $user or $group in homedir.'));
			if ( !ereg('^[/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*([/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*)*$', $account_new->general_homedir ))
				$errors[] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$temp = explode(':', checkid($account_new, $account_old));
			$account_new->general_uidNumber = $temp[0];
			// true if checkid has returned an error
			if ($temp[1]!='') $errors[] = explode(';',$temp[1]);
			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $account_new->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));
			}
		break;
	case 'unix':
		// Write all general values into $account_new
		if (isset($_POST['f_unix_password'])) {
			// Encraypt password
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$account_new->unix_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $_POST['f_unix_password'], MCRYPT_MODE_ECB, $iv));
			}
		 else $account_new->unix_password = '';
		if ($_POST['f_unix_password_no']) $account_new->unix_password_no = true;
			else $account_new->unix_password_no = false;
		$account_new->unix_pwdwarn = $_POST['f_unix_pwdwarn'];
		$account_new->unix_pwdallowlogin = $_POST['f_unix_pwdallowlogin'];
		$account_new->unix_pwdmaxage = $_POST['f_unix_pwdmaxage'];
		$account_new->unix_pwdminage = $_POST['f_unix_pwdminage'];
		$account_new->unix_host = $_POST['f_unix_host'];
		$account_new->unix_pwdexpire = mktime(10, 0, 0, $_POST['f_unix_pwdexpire_mon'],
			$_POST['f_unix_pwdexpire_day'], $_POST['f_unix_pwdexpire_yea']);
		if ($_POST['f_unix_deactivated']) $account_new->unix_deactivated = $_POST['f_unix_deactivated'];
			else $account_new->unix_deactivated = false;
		if ($_POST['genpass']) {
			// Generate a random password if generate-button was pressed
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$account_new->unix_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, genpasswd(), MCRYPT_MODE_ECB, $iv));
			// Keep unix-page acitve
			$select_local = 'unix';
			}
		// Check if values are OK and set automatic values. if not error-variable will be set
		else { // account.inc
			if ($account_new->unix_password != '') {
				$iv = base64_decode($_COOKIE["IV"]);
				$key = base64_decode($_COOKIE["Key"]);
				$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account_new->unix_password), MCRYPT_MODE_ECB, $iv);
				$password = str_replace(chr(00), '', $password);
				}
			if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $password))
				$errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
			if ( !ereg('^([0-9])*$', $account_new->unix_pwdminage))  $errors[] = array('ERROR', _('Password minage'), _('Password minage must be are natural number.'));
			if ( $account_new->unix_pwdminage > $account_new->unix_pwdmaxage ) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must bigger as Password Minage.'));
			if ( !ereg('^([0-9]*)$', $account_new->unix_pwdmaxage)) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must be are natural number.'));
			if ( !ereg('^(([-][1])|([0-9]*))$', $account_new->unix_pwdallowlogin))
				$errors[] = array('ERROR', _('Password Expire'), _('Password expire must be are natural number or -1.'));
			if ( !ereg('^([0-9]*)$', $account_new->unix_pwdwarn)) $errors[] = array('ERROR', _('Password warn'), _('Password warn must be are natural number.'));
			if ((!$account_new->unix_host=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([ ])*([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $account_new->unix_host))
				$errors[] = array('ERROR', _('Unix workstations'), _('Unix workstations is invalid.'));
			}
		break;
	case 'samba':
		// Write all general values into $account_new
		$account_new->smb_pwdcanchange = mktime($_POST['f_smb_pwdcanchange_s'], $_POST['f_smb_pwdcanchange_m'], $_POST['f_smb_pwdcanchange_h'],
			$_POST['f_smb_pwdcanchange_mon'], $_POST['f_smb_pwdcanchange_day'], $_POST['f_smb_pwdcanchange_yea']);
		$account_new->smb_pwdmustchange = mktime($_POST['f_smb_pwdmustchange_s'], $_POST['f_smb_pwdmustchange_m'], $_POST['f_smb_pwdmustchange_h'],
			$_POST['f_smb_pwdmustchange_mon'], $_POST['f_smb_pwdmustchange_day'], $_POST['f_smb_pwdmustchange_yea']);
		if ($_POST['f_smb_password_no']) $account_new->smb_password_no = true;
			else $account_new->smb_password_no = false;
		if ($_POST['f_smb_useunixpwd']) $account_new->smb_useunixpwd = true;
			else $account_new->smb_useunixpwd = false;
		$account_new->smb_homedrive = $_POST['f_smb_homedrive'];
		$account_new->smb_scriptPath = $_POST['f_smb_scriptpath'];
		$account_new->smb_smbhome = stripslashes($_POST['f_smb_smbhome']);
		$account_new->smb_profilePath = stripslashes($_POST['f_smb_profilePath']);
		$account_new->smb_displayName = $_POST['f_smb_displayName'];
		if ($_POST['f_smb_flagsD']) $account_new->smb_flagsD = true;
			else $account_new->smb_flagsD = false;
		if ($_POST['f_smb_flagsX']) $account_new->smb_flagsX = true;
			else $account_new->smb_flagsX = false;

		if ($config_intern->is_samba3()) {
			// samba 3 uses object with SID and domainname
			$samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());
			for ($i=0; $i<sizeof($samba3domains); $i++)
				if ($_POST['f_smb_domain'] == $samba3domains[$i]->name) {
					$account_new->smb_domain = $samba3domains[$i];
					}
			// Check if user is member of a well known windows group
			switch ($_POST['f_smb_mapgroup']) {
				case '*'._('Domain Guests'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '514'; break;
				case '*'._('Domain Users'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '513'; break;
				case '*'._('Domain Admins'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '512'; break;
				case $account_new->general_group:
						$account_new->smb_mapgroup = $account_new->smb_domain->SID . "-".
							(2 * getgid($account_new->general_group) + $account_new->smb_domain->RIDbase +1);
					break;
				}
			}
		else {
			// samba 2.2 only uses a string as domainname
			if (isset($_POST['f_smb_domain'])) $account_new->smb_domain = $_POST['f_smb_domain'];
				else $account_new->smb_domain = '';
			// Check if user is member of a well known windows group
			switch ($_POST['f_smb_mapgroup']) {
				case '*'._('Domain Guests'): $account_new->smb_mapgroup = '514'; break;
				case '*'._('Domain Users'): $account_new->smb_mapgroup = '513'; break;
				case '*'._('Domain Admins'): $account_new->smb_mapgroup = '512'; break;
				case $account_new->general_group:
					$account_new->smb_mapgroup = (2 * getgid($account_new->general_group) + 1001);
					break;
				}
			}
		// Set samba password
		$smb_password = $_POST['f_smb_password'];
		// Decrypt unix-password if needed password
		$iv = base64_decode($_COOKIE["IV"]);
		$key = base64_decode($_COOKIE["Key"]);
		if ( ($account_new->smb_useunixpwd && !$account_old) || ($account_new->smb_useunixpwd && $account_new->unix_password!='') ) {
			// Set Samba-Password to unix-password if option is set
			$unix_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account_new->unix_password), MCRYPT_MODE_ECB, $iv);
			$smb_password = str_replace(chr(00), '', $unix_password);
			}
		// Check values
		$account_new->smb_scriptPath = str_replace('$user', $account_new->general_username, $account_new->smb_scriptPath);
		$account_new->smb_scriptPath = str_replace('$group', $account_new->general_group, $account_new->smb_scriptPath);
		if ($account_new->smb_scriptPath != $_POST['f_smb_scriptpath']) $errors[] = array('INFO', _('Script path'), _('Inserted user- or groupname in scriptpath.'));
		$account_new->smb_profilePath = str_replace('$user', $account_new->general_username, $account_new->smb_profilePath);
		$account_new->smb_profilePath = str_replace('$group', $account_new->general_group, $account_new->smb_profilePath);
		if ($account_new->smb_profilePath != stripslashes($_POST['f_smb_profilePath'])) $errors[] = array('INFO', _('Profile path'), _('Inserted user- or groupname in profilepath.'));
		$account_new->smb_smbhome = str_replace('$user', $account_new->general_username, $account_new->smb_smbhome);
		$account_new->smb_smbhome = str_replace('$group', $account_new->general_group, $account_new->smb_smbhome);
		if ($account_new->smb_smbhome != stripslashes($_POST['f_smb_smbhome'])) $errors[] = array('INFO', _('Home path'), _('Inserted user- or groupname in HomePath.'));
		if ( (!$account_new->smb_smbhome=='') && (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+)+$', $account_new->smb_smbhome)))
				$errors[] = array('ERROR', _('Home path'), _('Home path is invalid.'));
		if ( !ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$',
			$smb_password)) $errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
		if ( (!$account_new->smb_scriptPath=='') && (!ereg('^([/])*([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*'.
			'([/]([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+([a-z]|[0-9]|[.]|[-]|[_]|[%]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*)*(([.][b][a][t])|([.][c][m][d]))$', $account_new->smb_scriptPath)))
			$errors[] = array('ERROR', _('Script path'), _('Script path is invalid!'));
		if ( (!$account_new->smb_profilePath=='') && (!ereg('^[/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*([/][a-z]([a-z]|[0-9]|[.]|[-]|[_]|[%])*)*$', $account_new->smb_profilePath))
			&& (!ereg('^[\][\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+([\]([a-z]|[A-Z]|[0-9]|[.]|[-]|[%])+)+$', $account_new->smb_profilePath)))
				$errors[] = array('ERROR', _('Profile path'), _('Profile path is invalid!'));
		if ((!$account_new->smb_domain=='') && (!is_object($account_new->smb_domain)) && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $account_new->smb_domain))
			$errors[] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));
		if ($account_new->smb_useunixpwd) $account_new->smb_useunixpwd = 1; else $account_new->smb_useunixpwd = 0;
		if (($account_new->smb_displayName=='') && isset($account_new->general_gecos)) {
			$account_new->smb_displayName = $account_new->general_gecos;
			$errors[] = array('INFO', _('Display name'), _('Inserted gecos-field as display name.'));
			}
		if ($smb_password!='') {
			// Encrypt password
			$account_new->smb_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, $key, $smb_password,
			MCRYPT_MODE_ECB, $iv));
			}
		else $account_new->smb_password = '';
		break;
	case 'quota':
		// Write all general values into $account_new
		$i=0;
		// loop for every mointpoint with quotas
		while ($account_new->quota[$i][0]) {
			$account_new->quota[$i][2] = $_POST['f_quota_'.$i.'_2'];
			$account_new->quota[$i][3] = $_POST['f_quota_'.$i.'_3'];
			$account_new->quota[$i][6] = $_POST['f_quota_'.$i.'_6'];
			$account_new->quota[$i][7] = $_POST['f_quota_'.$i.'_7'];
			// Check if values are OK and set automatic values. if not error-variable will be set
			if (!ereg('^([0-9])*$', $account_new->quota[$i][2]))
				$errors[] = array('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed'));
			if (!ereg('^([0-9])*$', $account_new->quota[$i][3]))
				$errors[] = array('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed'));
			if (!ereg('^([0-9])*$', $account_new->quota[$i][6]))
				$errors[] = array('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed'));
			if (!ereg('^([0-9])*$', $account_new->quota[$i][7]))
				$errors[] = array('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed'));
			$i++;
			}
		break;
	case 'personal':
		// Write all general values into $account_new
		$account_new->personal_title = $_POST['f_personal_title'];
		$account_new->personal_mail = $_POST['f_personal_mail'];
		$account_new->personal_telephoneNumber = $_POST['f_personal_telephoneNumber'];
		$account_new->personal_mobileTelephoneNumber = $_POST['f_personal_mobileTelephoneNumber'];
		$account_new->personal_facsimileTelephoneNumber = $_POST['f_personal_facsimileTelephoneNumber'];
		$account_new->personal_street = $_POST['f_personal_street'];
		$account_new->personal_postalCode = $_POST['f_personal_postalCode'];
		$account_new->personal_postalAddress = $_POST['f_personal_postalAddress'];
		$account_new->personal_employeeType = $_POST['f_personal_employeeType'];
		// Check if values are OK and set automatic values. if not error-variable will be set
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $account_new->personal_telephoneNumber))  $errors[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $account_new->personal_mobileTelephoneNumber))  $errors[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
		if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $account_new->personal_facsimileTelephoneNumber))  $errors[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $account_new->personal_mail))  $errors[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $account_new->personal_street))  $errors[] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $account_new->personal_postalAddress))  $errors[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $account_new->personal_title))  $errors[] = array('ERROR', _('Title'), _('Please enter a valid title!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $account_new->personal_employeeType))  $errors[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $account_new->personal_postalCode))  $errors[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		break;
	case 'final':
		// Write all general values into $account_new
		break;
	case 'finish':
		// Check if pdf-file should be created
		if ($_POST['outputpdf']) {
			// Load quotas if not yet done because they are needed for the pdf-file
			if ($config_intern->scriptServer && !isset($account_new->quota[0])) { // load quotas
				$values = getquotas('user', $account_old->general_username);
				if (is_object($values)) {
					while (list($key, $val) = each($values)) // Set only defined values
						if (isset($val)) $account_new->$key = $val;
					}
				if (is_object($values) && isset($account_old)) {
					while (list($key, $val) = each($values)) // Set only defined values
						if (isset($val)) $account_old->$key = $val;
					}
				}
			// Create / display PDf-file
			createUSerPDF(array($account_new));
			// Stop script
			die;
			}
		break;
	}



do { // X-Or, only one if() can be true
	if ($_POST['next_general']) {
		// Go from general to next page if no error did ocour
		if (!is_array($errors)) $select_local='general';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_unix']) {
		// Go from unix to next page if no error did ocour
		if (!is_array($errors)) $select_local='unix';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_samba']) {
		// Go from samba to next page if no error did ocour
		if (!is_array($errors)) $select_local='samba';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_quota']) {
		// Go from quota to next page if no error did ocour
		if (!is_array($errors)) $select_local='quota';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_personal']) {
		// Go from personal to next page if no error did ocour
		if (!is_array($errors)) $select_local='personal';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_final']) {
		// Go from final to next page if no error did ocour
		$stay = false;
		if (($account_old) && ($account_new->general_uidNumber != $account_old->general_uidNumber))
			$errors[] = array('INFO', _('UID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
				'find / -gid ' . $account_old->general_uidNumber . ' -exec chown ' . $account_new->general_uidNumber . ' {} \;');
		if (($account_old) && ($account_new->general_group != $account_old->general_group))
			$errors[] = array('INFO', _('Primary group has changed. You have to run the following command as root in order to change existing file-permissions:'),
				'find / -uid ' . $account_new->general_uidNumber . ' -gid ' . getgid($account_old->general_group) .' -exec chown ' . $account_new->general_uidNumber . ':'.getgid($account_new->general_group). ' {} \;');
		if (($account_old) && ($account_new->general_homedir != $account_old->general_homedir))
			$errors[] = array('INFO', _('Home Directory has changed. You have to run the following command as root in order to change the existing homedirectory:'),
				'mv ' . $account_old->general_homedir . ' ' . $account_new->general_homedir);
		if ($config_intern->is_samba3() && !isset($account_new->smb_domain)) {
			// Samba page not viewed; can not create user because if missing options
			$errors[] = array("ERROR", _("Samba Options not set!"), _("Please check settings on samba page."));
			$stay = true;
			}
		if (!$config_intern->is_samba3()) {
			$found = false;
			if (strstr($account_new->smb_scriptPath, '$group')) $found = true;
			if (strstr($account_new->smb_scriptPath, '$user')) $found = true;
			if (strstr($account_new->smb_profilePath, '$group')) $found = true;
			if (strstr($account_new->smb_profilePath, '$user')) $found = true;
			if (strstr($account_new->smb_smbhome, '$group')) $found = true;
			if (strstr($account_new->smb_smbhome, '$user')) $found = true;
			if ($found)
				// Samba page not viewed; can not create group because if missing options
				$stay = true;
				$errors[] = array("ERROR", _("Samba Options not set!"), _("Please check settings on samba page."));
			}
		if (isset($account_old->general_objectClass)) {
			if (!in_array('posixAccount', $account_old->general_objectClass))
				$errors[] = array('WARN', _('ObjectClass posixAccount not found.'), _('Have to add objectClass posixAccount.'));
			if (!in_array('shadowAccount', $account_old->general_objectClass))
				$errors[] = array('WARN', _('ObjectClass shadowAccount.'), _('Have to add objectClass shadowAccount.'));
			if ($config_intern->is_samba3()) {
				if (!in_array('sambaSamAccount', $account_old->general_objectClass))
					$errors[] = array('WARN', _('ObjectClass sambaSamAccount not found.'), _('Have to add objectClass sambaSamAccount. USer with sambaAccount will be updated.'));
				}
			else {
				if (!in_array('sambaAccount', $account_old->general_objectClass))
					$errors[] = array('WARN', _('ObjectClass sambaAccount not found.'), _('Have to add objectClass sambaAccount. User with sambaSamAccount will be set back to sambaAccount.'));
				}
			}
		if (!$stay) $select_local='final';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_workstations']) {
		// Go from workstations to next page if no error did ocour
		if (!is_array($errors)) $select_local='workstations';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_groups']) {
		// Go from groups to next page if no error did ocour
		if (!is_array($errors)) $select_local='groups';
			else $select_local=$_POST['select'];
		break;
		}
	// Reset account to original settings if undo-button was pressed
	if ($_POST['next_reset']) {
		$account_new = $account_old;
		$account_new->unix_password='';
		$account_new->smb_password='';
		$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
		$select_local = $_POST['select'];
		break;
		}
	if ( $_POST['create'] ) { // Create-Button was pressed
		// Create or modify an account
		if ($account_old) $result = modifyuser($account_new,$account_old);
			 else $result = createuser($account_new); // account.inc
		if ( $result==4 || $result==5 ) $select_local = 'final';
			else $select_local = 'finish';
		break;
		}
	// Load Profile and reset all attributes to settings in profile
	if ($_POST['createagain']) {
		$select_local='general';
		unset ($_SESSION['account_'.$varkey.'_account_new']);
		unset($account_new);
		$_SESSION['account_'.$varkey.'_account_new'] = loadUserProfile('default');
		$account_new =& $_SESSION['account_'.$varkey.'_account_new'];
		$account_new ->type = 'user';
		break;
		}
	// Load Profile and reset all attributes to settings in profile
	if ($_POST['load']) {
		$account_new->general_dn = $_POST['f_general_suffix'];
		$account_new->general_username = $_POST['f_general_username'];
		$account_new->general_surname = $_POST['f_general_surname'];
		$account_new->general_givenname = $_POST['f_general_givenname'];
		$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
		$account_new->general_group = $_POST['f_general_group'];
		if (isset($_POST['f_general_groupadd'])) $account_new->general_groupadd = $_POST['f_general_groupadd'];
			else $account_new->general_groupadd = array('');
		$account_new->general_homedir = $_POST['f_general_homedir'];
		$account_new->general_shell = $_POST['f_general_shell'];
		$account_new->general_gecos = $_POST['f_general_gecos'];
		if ($_POST['f_general_selectprofile']!='') $values = loadUserProfile($_POST['f_general_selectprofile']);
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $account_new->$key = $val;
			}
		// insert autoreplace values
		$account_new->general_homedir = str_replace('$group', $account_new->general_group, $account_new->general_homedir);
		if ($account_new->general_username != '')
			$account_new->general_homedir = str_replace('$user', $account_new->general_username, $account_new->general_homedir);
		$account_new->smb_scriptPath = str_replace('$group', $account_new->general_group, $account_new->smb_scriptPath);
		if ($account_new->general_username != '')
			$account_new->smb_scriptPath = str_replace('$user', $account_new->general_username, $account_new->smb_scriptPath);
		$account_new->smb_profilePath = str_replace('$group', $account_new->general_group, $account_new->smb_profilePath);
		if ($account_new->general_username != '')
			$account_new->smb_profilePath = str_replace('$user', $account_new->general_username, $account_new->smb_profilePath);
		$account_new->smb_smbhome = str_replace('$group', $account_new->general_group, $account_new->smb_smbhome);
		if ($account_new->general_username != '')
			$account_new->smb_smbhome = str_replace('$user', $account_new->general_username, $account_new->smb_smbhome);
		if ($config_intern->scriptServer) {
			// load quotas and check if quotas from profile are valid
			$values = getquotas('user');
			if (isset($account_new->quota[0])) {
				 // check quotas from profile
				$i=0;
				// check quota settings, loop for every partition with quotas
				while (isset($account_new->quota[$i])) {
					// search if quotas from profile fit to a real quota
					$found = (-1);
					for ($j=0; $j<count($values->quota); $j++)
						if ($values->quota[$j][0]==$account_new->quota[$i][0]) $found = $j;
					// unset quota from profile if quotas (mointpoint) doesn't exists anymore
					if ($found==-1) unset($account_new->quota[$i]);
					else {
						// Set missing part in quota-array
						$account_new->quota[$i][1] = $values->quota[$found][1];
						$account_new->quota[$i][5] = $values->quota[$found][5];
						$account_new->quota[$i][4] = $values->quota[$found][4];
						$account_new->quota[$i][8] = $values->quota[$found][8];
						$i++;
						}
					}
				// Beautify array, repair index
				$account_new->quota = array_values($account_new->quota);
				}
			else { // No quotas saved in profile
				// Display quotas for new users (Quota set to 0)
				if (is_object($values)) {
					while (list($key, $val) = each($values)) // Set only defined values
					if (isset($val)) $account_new->$key = $val;
					}
				}
			}
		// select general page after group has been loaded
		$select_local='general';
		break;
		}
	// Save Profile
	if ($_POST['save']) {
		// save profile
		if ($_POST['f_finish_safeProfile']=='')
			$errors[] = array('ERROR', _('Save profile'), _('No profilename given.'));
		else {
			saveUSerProfile($account_new, $_POST['f_finish_safeProfile']);
			$errors[] = array('INFO', _('Save profile'), _('New profile created.'));
			}
		// select last page displayed before user is created
		$select_local='final';
		break;
		}
	// Go back to listgroups.php
	if ($_POST['backmain']) {
		if (isset($_SESSION['account_'.$varkey.'_account_new'])) unset($_SESSION['account_'.$varkey.'_account_new']);
		if (isset($_SESSION['account_'.$varkey.'_account_old'])) unset($_SESSION['account_'.$varkey.'_account_old']);
		if (isset($_SESSION['account_'.$varkey.'_final_changegids'])) unset($_SESSION['account_'.$varkey.'_final_changegids']);
		if (isset($_SESSION['account_'.$varkey.'_shelllist'])) unset($_SESSION['account_'.$varkey.'_shelllist']);
		metaRefresh("../lists/listusers.php");
		die;
		break;
		}
	} while(0);


// Write HTML-Header
echo $header_intern;
echo "<html><head><title>";
echo _("Create new Account");
echo "</title>\n".
	"<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n".
	"<meta http-equiv=\"pragma\" content=\"no-cache\">\n".
	"<meta http-equiv=\"cache-control\" content=\"no-cache\">\n".
	"</head><body>\n".
	"<form action=\"useredit.php\" method=\"post\">\n".
	"<input name=\"varkey\" type=\"hidden\" value=\"".$varkey."\">\n";

// Display errir-messages
if (is_array($errors))
	for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);

// print_r($account_new);

switch ($select_local) {
	/* Select which part of page should be loaded and check values
	* groups = page with all groups to which user is additional member
	* workstations = page with all workstations the user is allowed to login
	* general = startpage, general account paramters
	* samba = page with all samba-related parameters e.g. smbpassword
	* quota = page with all quota-related parameters e.g. hard file quota
	* personal = page with all personal-related parametergs, e.g. phone number
	* final = last page shown before account is created/modified
	* finish = page shown after account has been created/modified
	*/
	case 'workstations':
		// Validate cache-array
		ldapreload('host');
		// Get copy of cache-array
		$temp2 = $hostDN_intern;
		// unset timestamp stored in $temp2[0]
		unset($temp2[0]);
		// Remove $ from workstations
		foreach ($temp2 as $temp) $hosts[] = str_replace("$", '',$temp['cn']);
		// sort workstations
		sort($hosts, SORT_STRING);
		// get workstation array
		$temp = str_replace(' ', '', $account_new->smb_smbuserworkstations);
		$workstations = explode (',', $temp);
		// Remove workstations to which the user is allowed to login from array
		$hosts = array_delete($workstations, $hosts);
		echo '<input name="select" type="hidden" value="workstations">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		echo _("Select workstations");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td valign=\"top\">";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\">";
		echo _("Allowed workstations");
		echo "</legend>\n";
		// display all workstations the user is allowed to login
		if (count($workstations)!=0) {
			echo "<select name=\"members[]\" class=\"useredit-bright\" size=15 multiple>\n";
			for ($i=0; $i<count($workstations); $i++)
				if ($workstations[$i]!='') echo "		<option>".$workstations[$i]."</option>\n";
			echo "</select>\n";
			}
		echo "</fieldset></td>\n";
		echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"add\" value=\"<=\">";
		echo " ";
		echo "<input type=\"submit\" name=\"remove\" value=\"=>\"><br><br>";
		echo "<a href=\""."../help.php?HelpNumber=436\" target=\"lamhelp\">"._('Help')."</a></td>\n";
		echo "<td valign=\"top\"><fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\">";
		echo _('Available workstations');
		echo "</legend>\n";
		// Display all workstations without these the user is allowed to login
		if (count($hosts)!=0) {
			echo "<select name=\"hosts[]\" size=15 multiple class=\"useredit-bright\">\n";
			foreach ($hosts as $temp) echo "		<option>$temp</option>\n";
			echo "</select>\n";
			}
		echo "</fieldset></td>\n</tr>\n</table>\n";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Back'); echo "\">\n";
		echo "</fieldset></td></tr></table>\n</td></tr>\n</table>\n";
		break;
	case 'groups':
		// Validate cache-array
		ldapreload('group');
		// Get copy of cache-array
		$temp2 = $groupDN_intern;
		// unset timestamp stored in $temp2[0]
		unset($temp2[0]);
		// load list with all groups
		foreach ($temp2 as $temp) $groups[] = $temp['cn'];
		// sort groups
		sort($groups, SORT_STRING);
		// remove groups the user is member of from grouplist
		$groups = array_delete($account_new->general_groupadd, $groups);
		// Remove primary group from grouplist
		$groups = array_flip($groups);
		if (isset($groups[$account_new->general_group])) unset ($groups[$account_new->general_group]);
		$groups = array_flip($groups);
		echo '<input name="select" type="hidden" value="groups">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		echo _("Additional groups");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td valign=\"top\">";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\">";
		echo _("Selected groups");
		echo "</legend>\n";
		// Show all groups the user is additional member of
		if (count($account_new->general_groupadd)!=0) {
			echo "<select name=\"selectedgroups[]\" class=\"useredit-bright\" size=15 multiple>\n";
			for ($i=0; $i<count($account_new->general_groupadd); $i++)
				if ($account_new->general_groupadd[$i]!='') echo "		<option>".$account_new->general_groupadd[$i]."</option>\n";
			echo "</select>\n";
			}
		echo "</fieldset></td>\n";
		echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"add\" value=\"<=\">";
		echo " ";
		echo "<input type=\"submit\" name=\"remove\" value=\"=>\"><br><br>";
		echo "<a href=\""."../help.php?HelpNumber=402\" target=\"lamhelp\">"._('Help')."</a></td>\n";
		echo "<td valign=\"top\"><fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\">";
		echo _('Available groups');
		echo "</legend>\n";
		// show all groups expect these the user is member of
		if (count($groups)!=0) {
			echo "<select name=\"allgroups[]\" size=15 multiple class=\"useredit-bright\">\n";
			foreach ($groups as $temp) {
					$temp = str_replace("$", '',$temp);
					echo "		<option>$temp</option>\n";
					}
			echo "</select>\n";
			}
		echo "</fieldset></td>\n</tr>\n</table>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('Back'); echo "\">\n";
		echo "</fieldset></td></tr></table>\n</td></tr>\n</table>\n";
		break;
	case 'general':
		// General Account Settings
		// load list of all groups
		$groups = findgroups();
		// load list of profiles
		$profilelist = getUserProfiles();
		echo '<input name="select" type="hidden" value="general">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" disabled value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		echo _("General properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _('Username').'*';
		echo "</td>\n<td>".
			'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $account_new->general_username . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=400" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('UID number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $account_new->general_uidNumber . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=401" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Given name').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_givenname" type="text" size="20" maxlength="20" value="' . $account_new->general_givenname . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=425" target="lamhelp">'._('Help').'</a>'.
			'</td>'."\n".'</tr>'."\n".'<tr><td>';
		echo _('Surname').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_surname" type="text" size="20" maxlength="20" value="' . $account_new->general_surname . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=424" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Primary group').'*';
		echo '</td>'."\n".'<td><select name="f_general_group">';
		// loop trough existing groups
		foreach ($groups as $group) {
			if ($account_new->general_group == $group) echo '<option selected>' . $group. '</option>';
			else echo '<option>' . $group. '</option>';
			 }
		echo '</select></td><td>'.
			'<a href="../help.php?HelpNumber=406" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';

		echo _('Additional groups');
		echo '</td>'."\n".'<td><input name="next_groups" type="submit" value="'. _('Edit groups') . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=402" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Home directory').'*';
		echo '</td>'."\n".'<td><input name="f_general_homedir" type="text" size="30" value="' . $account_new->general_homedir . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=403" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Gecos');
		echo '</td>'."\n".'<td><input name="f_general_gecos" type="text" size="30" value="' . $account_new->general_gecos . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=404" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Login shell').'*';
		echo '</td>'."\n".'<td><select name="f_general_shell" >';
			// loop through shells
			foreach ($shelllist as $shell)
				if ($account_new->general_shell==trim($shell)) echo '<option selected>'.$shell. '</option>';
					else echo '<option>'.$shell. '</option>';
		echo '</select></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=405" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
		// loop through all user suffixes
		foreach ($ldap_intern->search_units($config_intern->get_UserSuffix()) as $suffix) {
			if ($account_new->general_dn) {
				if ($account_new->general_dn == $suffix)
					echo '<option selected>' . $suffix. '</option>';
				else echo '<option>' . $suffix. '</option>';
				}
			else echo '<option>' . $suffix. '</option>';
			}
		echo '</select></td><td><a href="../help.php?HelpNumber=461" target="lamhelp">'._('Help').
			"</a></td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr><tr><td>";
		// Show fieldset with list of all user profiles
		if (count($profilelist)!=0) {
			echo "<fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
			echo _("Load profile");
			echo "</b></legend>\n<table border=0>\n<tr>\n<td>";
			echo "<select name=\"f_general_selectprofile\" >";
			foreach ($profilelist as $profile) echo "	<option>$profile</option>\n";
			echo "</select>\n".
				"<input name=\"load\" type=\"submit\" value=\""; echo _('Load Profile');
			echo "\"></td><td><a href=\""."../help.php?HelpNumber=421\" target=\"lamhelp\">";
			echo _('Help')."</a></td>\n</tr>\n</table>\n</fieldset>\n";
			}
		echo "</td></tr>\n</table>\n</td></tr></table>\n";
		break;
	case 'unix':
		// Unix Password Settings
		// decrypt password
		if ($account_new->unix_password != '') {
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account_new->unix_password), MCRYPT_MODE_ECB, $iv);
			$password = str_replace(chr(00), '', $password);
			}
		 else $password='';
		// Use dd-mm-yyyy format of date because it's easier to read for humans
		$date = getdate ($account_new->unix_pwdexpire);
		echo "<input name=\"select\" type=\"hidden\" value=\"unix\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" disabled value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td valign=\"top\">";
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
		if ($account_new->unix_password_no) echo ' checked ';
		echo '></td>'."\n".'<td>'.
		'<a href="../help.php?HelpNumber=426" target="lamhelp">'._('Help').'</a>'.
		'</td></tr>'."\n".'<tr><td>';
		echo _('Password warn');
		echo '</td>'."\n".'<td><input name="f_unix_pwdwarn" type="text" size="4" maxlength="4" value="' . $account_new->unix_pwdwarn . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=414" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Password expire');
		echo '</td>'."\n".'<td><input name="f_unix_pwdallowlogin" type="text" size="4" maxlength="4" value="' . $account_new->unix_pwdallowlogin . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=415" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Maximum password age');
		echo '</td>'."\n".'<td><input name="f_unix_pwdmaxage" type="text" size="5" maxlength="5" value="' . $account_new->unix_pwdmaxage . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=416" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Minimum password age');
		echo '</td>'."\n".'<td><input name="f_unix_pwdminage" type="text" size="4" maxlength="4" value="' . $account_new->unix_pwdminage . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=417" target="lamhelp">'._('Help').'</a>'.
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
			'<a href="../help.php?HelpNumber=418" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Account deactivated');
		echo '</td>'."\n".'<td><input name="f_unix_deactivated" type="checkbox"';
		if ($account_new->unix_deactivated) echo ' checked ';
		echo '></td>'."\n".'<td>'.
		'<a href="../help.php?HelpNumber=427" target="lamhelp">'._('Help').'</a>'.
		'</td></tr>'."\n";
		// show only hosts if schema does allow hosts
		if ($_SESSION['ldap']->support_unix_hosts) {
			echo '<tr><td>';
			echo _('Unix workstations');
			echo '</td>'."\n".'<td><input name="f_unix_host" type="text" size="20" maxlength="80" value="' . $account_new->unix_host . '">'.
				'</td>'."\n".'<td>'.
				'<a href="../help.php?HelpNumber=466" target="lamhelp">'._('Help').
				"</a></td>\n</tr>\n";
			}
		echo "</table>\n";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;
	case 'samba':
		// Samba Settings
		// decrypt password
		if ($account_new->smb_password != '') {
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($account_new->smb_password), MCRYPT_MODE_ECB, $iv);
			$password = str_replace(chr(00), '', $password);
			}
		else $password = "";
		if ($config_intern->is_samba3()) $samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());
		// Use dd-mm-yyyy format of date because it's easier to read for humans
		$canchangedate = getdate($account_new->smb_pwdcanchange);
		$mustchangedate = getdate($account_new->smb_pwdmustchange);
		echo '<input name="select" type="hidden" value="samba">';
		// Save all values smaller than "day" so we don't loose them
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
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		echo _("Samba properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _("Display name");
		echo "</td>\n<td>".
			"<input name=\"f_smb_displayName\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"".$account_new->smb_displayName."\">".
			"</td>\n<td><a href=\""."../help.php?HelpNumber=420\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Samba password');
		echo '</td>'."\n".'<td><input name="f_smb_password" type="text" size="20" maxlength="20" value="' . $password . '">'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Use unix password');
		echo '</td><td><input name="f_smb_useunixpwd" type="checkbox"';
		if ($account_new->smb_useunixpwd) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=428" target="lamhelp">'._('Help').'</a>';
		echo '</td></tr>'."\n".'<tr><td>';
		echo _('Use no password');
		echo '</td>'."\n".'<td><input name="f_smb_password_no" type="checkbox"';
		if ($account_new->smb_password_no) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=426" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Password does not expire');
		echo '</td>'."\n".'<td><input name="f_smb_flagsX" type="checkbox"';
		if ($account_new->smb_flagsX) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=429" target="lamhelp">'._('Help').'</a>'.
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
		echo	'<a href="../help.php?HelpNumber=430" target="lamhelp">'._('Help').'</a>'.
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
		echo	'<a href="../help.php?HelpNumber=431" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Account is deactivated');
		echo '</td>'."\n".'<td><input name="f_smb_flagsD" type="checkbox"';
		if ($account_new->smb_flagsD) echo ' checked ';
		echo '></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=432" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Home drive');
		echo '</td>'."\n".'<td><select name="f_smb_homedrive" >';
			for ($i=90; $i>67; $i--)
				if ($account_new->smb_homedrive== chr($i).':') echo '<option selected> '.chr($i).':</option>'; else echo '<option> '.chr($i).':</option>';
		echo	'</select></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=433" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Home path');
		echo '</td>'."\n".'<td><input name="f_smb_smbhome" type="text" size="20" maxlength="80" value="' . $account_new->smb_smbhome . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=437" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Profile path');
		echo '</td>'."\n".'<td><input name="f_smb_profilePath" type="text" size="20" maxlength="80" value="' . $account_new->smb_profilePath . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=435" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Script path');
		echo '</td>'."\n".'<td><input name="f_smb_scriptpath" type="text" size="20" maxlength="80" value="' . $account_new->smb_scriptPath . '">'.
		'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=434" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Samba workstations');
		echo '</td>'."\n".'<td><input name="next_workstations" type="submit" value="'. _('Edit workstations') . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=436" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Windows groupname');
		echo '</td>'."\n".'<td><select name="f_smb_mapgroup" >';
		// Display if group SID should be mapped to a well kown SID
		if ($config_intern->is_samba3()) {
			if ( $account_new->smb_mapgroup == $account_new->smb_domain->SID . "-".
			(2 * getgid($account_new->general_group) + $values->smb_domain->RIDbase+1)) {
				echo '<option selected> ';
				echo $account_new->general_group;
				echo "</option>\n"; }
			 else {
				echo '<option> ';
				echo $account_new->general_group;
				echo "</option>\n";
				}
			if ( $account_new->smb_mapgroup == $account_new->smb_domain->SID . "-" . '514' ) {
				echo '<option selected> *';
				echo _('Domain Guests');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Guests');
				echo "</option>\n";
				}
			if ( $account_new->smb_mapgroup == $account_new->smb_domain->SID . "-" . '513' ) {
				echo '<option selected> *';
				echo _('Domain Users');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Users');
				echo "</option>\n";
				}
			if ( $account_new->smb_mapgroup == $account_new->smb_domain->SID . "-" . '512' ) {
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
			if ( $account_new->smb_mapgroup == (2 * getgid($account_new->general_group) +1001)) {
				echo '<option selected> ';
				echo $account_new->general_group;
				echo "</option>\n"; }
			 else {
				echo '<option> ';
				echo $account_new->general_group;
				echo "</option>\n";
				}
			if ( $account_new->smb_mapgroup == '514' ) {
				echo '<option selected> *';
				echo _('Domain Guests');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Guests');
				echo "</option>\n";
				}
			if ( $account_new->smb_mapgroup == '513' ) {
				echo '<option selected> *';
				echo _('Domain Users');
				echo "</option>\n"; }
			 else {
				echo '<option> *';
				echo _('Domain Users');
				echo "</option>\n";
				}
			if ( $account_new->smb_mapgroup == '512' ) {
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
			'<a href="../help.php?HelpNumber=464" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Domain');
		// select which domain name should be displayed
		if ($config_intern->is_samba3()) {
			// samba 3 uses object to store SID and name of domain
			echo '</td><td><select name="f_smb_domain">';
			for ($i=0; $i<sizeof($samba3domains); $i++) {
				if ($account_new->smb_domain->name) {
					if ($account_new->smb_domain->name == $samba3domains[$i]->name)
						echo '<option selected>' . $samba3domains[$i]->name. '</option>';
					else echo '<option>' . $samba3domains[$i]->name. '</option>';
					}
				else echo '<option>' . $samba3domains[$i]->name. '</option>';
				}
			echo '</select>';
			}
		else {
			// Samba 2.2 just uses a string as domain name
			echo '</td>'."\n".'<td><input name="f_smb_domain" type="text" size="20" maxlength="80" value="' . $account_new->smb_domain . '">';
			}
		echo	'</td>'."\n".'<td><a href="../help.php?HelpNumber=438" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;
	case 'quota':
		// Quota Settings
		// Load quotas if not yet done
		if ($config_intern->scriptServer && !isset($account_new->quota[0])) { // load quotas
			$values = getquotas('user', $account_old->general_username);
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if (isset($val)) $account_new->$key = $val;
				}
			if (is_object($values) && isset($account_old)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if (isset($val)) $account_old->$key = $val;
				}
			}
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
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td valign=\"top\">";
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>"._('Quota properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Mountpoint'); echo '</td>'."\n".'<td>'; echo _('Used blocks'); echo '</td>'."\n".'<td>';
		echo _('Soft block limit'); echo '</td>'."\n".'<td>'; echo _('Hard block limit'); echo '</td>'."\n".'<td>'; echo _('Grace block period');
		echo '</td>'."\n".'<td>'; echo _('Used inodes'); echo '</td>'."\n".'<td>'; echo _('Soft inode limit'); echo '</td>'."\n".'<td>';
		echo _('Hard inode limit'); echo '</td>'."\n".'<td>'; echo _('Grace inode period'); echo '</td></tr>'."\n";
		echo '<tr><td><a href="../help.php?HelpNumber=439" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="../help.php?HelpNumber=440" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=441" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="../help.php?HelpNumber=442" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=443" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="../help.php?HelpNumber=444" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=445" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="../help.php?HelpNumber=446" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=447" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		$i=0;
		// loop for every mointpoint with enabled quotas
		while ($account_new->quota[$i][0]) {
			echo '<tr><td>'.$account_new->quota[$i][0].'</td><td>'.$account_new->quota[$i][1].'</td>'; // used blocks
			echo '<td><input name="f_quota_'.$i.'_2" type="text" size="12" maxlength="20" value="'.$account_new->quota[$i][2].'"></td>'; // blocks soft limit
			echo '<td><input name="f_quota_'.$i.'_3" type="text" size="12" maxlength="20" value="'.$account_new->quota[$i][3].'"></td>'; // blocks hard limit
			echo '<td>'.$account_new->quota[$i][4].'</td>'; // block grace period
			echo '<td>'.$account_new->quota[$i][5].'</td>'; // used inodes
			echo '<td><input name="f_quota_'.$i.'_6" type="text" size="12" maxlength="20" value="'.$account_new->quota[$i][6].'"></td>'; // inodes soft limit
			echo '<td><input name="f_quota_'.$i.'_7" type="text" size="12" maxlength="20" value="'.$account_new->quota[$i][7].'"></td>'; // inodes hard limit
			echo '<td>'.$account_new->quota[$i][8].'</td></tr>'; // inodes grace period
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
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" disabled value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td valign=\"top\">";
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>"._('Personal properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Title');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_title" type="text" size="10" maxlength="10" value="' . $account_new->personal_title . '"> ';
		echo $account_new->general_surname . ' ' . $account_new->general_givenname . '</td><td>'.
			'<a href="../help.php?HelpNumber=448" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Employee type');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_employeeType" type="text" size="30" maxlength="30" value="' . $account_new->personal_employeeType . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=449" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Street');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_street" type="text" size="30" maxlength="30" value="' . $account_new->personal_street . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=450" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Postal code');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_postalCode" type="text" size="5" maxlength="5" value="' . $account_new->personal_postalCode . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=451" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Postal address');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_postalAddress" type="text" size="30" maxlength="80" value="' . $account_new->personal_postalAddress . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=452" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Telephone number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_telephoneNumber" type="text" size="30" maxlength="30" value="' . $account_new->personal_telephoneNumber . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=453" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Mobile number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mobileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $account_new->personal_mobileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=454" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Fax number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_facsimileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $account_new->personal_facsimileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=455" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('eMail address');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mail" type="text" size="30" maxlength="80" value="' . $account_new->personal_mail . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=456" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;
	case 'final':
		// Final Settings
		echo '<input name="select" type="hidden" value="final">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_unix\" type=\"submit\" value=\""; echo _('Unix'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_personal\" type=\"submit\" value=\""; echo _('Personal'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" disabed value=\""; echo _('Final');
		echo "\">";
		if (isset($account_old)) {
			echo "<br><br>";
			echo _("Reset all changes.");
			echo "<br>";
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo');
			echo "\">\n";
			}
		echo "</fieldset></td></tr></table></td>\n<td valign=\"top\">";
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
		if ($account_old) echo _('Modify');
		 else echo _('Create');
		echo "</b></legend>\n";
		echo "<table border=0 width=\"100%\">";
		echo "<tr><td><input name=\"create\" type=\"submit\" value=\"";
		if ($account_old) echo _('Modify Account');
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
		echo $account_new->general_username;
		if ($account_old) echo ' '._('has been modified').'.';
		 else echo ' '._('has been created').'.';
		echo '</td></tr>'."\n".'<tr><td>';
		if (!$account_old)
			{ echo '<input name="createagain" type="submit" value="'; echo _('Create another user'); echo '">'; }
		echo '</td>'."\n".'<td>'.
			'<input name="outputpdf" type="submit" value="'; echo _('Create PDF file'); echo '">'.
			'</td>'."\n".'<td>'.
			'<input name="backmain" type="submit" value="'; echo _('Back to user list'); echo '">'.
			'</td></tr></table></fieldset'."\n";
		break;
	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
