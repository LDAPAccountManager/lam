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

if (!isset($_SESSION['account_'.$varkey.'_account_new'])) $_SESSION['account_'.$varkey.'_account_new'] = new account();
if (!isset($_SESSION['account_'.$varkey.'_final_changegids'])) $_SESSION['account_'.$varkey.'_final_changegids'] = '';

// Register Session-Variables with references so we don't net to change to complete code if names changes
$account_new =& $_SESSION['account_'.$varkey.'_account_new'];
$final_changegids =& $_SESSION['account_'.$varkey.'_final_changegids'];
if (is_object($_SESSION['account_'.$varkey.'_account_old'])) $account_old =& $_SESSION['account_'.$varkey.'_account_old'];
$ldap_intern =& $_SESSION['ldap'];
$config_intern =& $_SESSION['config'];
$header_intern =& $_SESSION['header'];
$userDN_intern =& $_SESSION['userDN'];

// $_GET is only valid if groupedit.php was called from grouplist.php
if (isset($_GET['DN']) && $_GET['DN']!='') {
	// groupedit.php should edit an existing account
	// reset variables
	if (isset($_SESSION['account_'.$varkey.'_account_old'])) {
		unset($account_old);
		unset($_SESSION['account_'.$varkey.'_account_old']);
		}
	$_SESSION['account_'.$varkey.'_account_old'] = new account();
	$account_old =& $_SESSION['account_'.$varkey.'_account_old'];
	// get "real" DN from variable
	$DN = str_replace("\'", '',$_GET['DN']);
	if ($_GET['DN'] == $DN) $DN = str_replace("'", '',$_GET['DN']);
	// Load existing group
	$account_new = loadgroup($DN);
	// Get a copy of original host
	$account_old = $account_new;
	// Store only DN without cn=$name
	$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
	$final_changegids = '';
	// Display general-page
	$select_local = 'general';
	}
// Startcondition. groupedit.php was called from outside to create a new group
else if (count($_POST)==0) {
	// Create new account object with settings from default profile
	$account_new = loadGroupProfile('default');
	$account_new ->type = 'group';
	if ($config_intern->scriptServer) {
		// load quotas and check if quotas from profile are valid
		$quotas = getquotas(array($account_new));
		for ($i=0; $i<count($account_new->quota); $i++) $profile_quotas[] = $account_new->quota[$i][0];
		for ($i=0; $i<count($quotas[0]->quota); $i++) {
			$real_quotas[] = $quotas[0]->quota[$i][0];
			if (is_array($profile_quotas)) {
				if (!in_array($quotas[0]->quota[$i][0], $profile_quotas)) $account_new->quota[]=$quotas[0]->quota[$i];
				}
			else $account_new->quota[]=$quotas[0]->quota[$i];
			}
		$j=0;
		// delete not existing quotas
		while (isset($account_new->quota[$j][0])) {
			// remove invalid quotas
			if (!in_array($account_new->quota[$j][0], $real_quotas)) unset($account_new->quota[$j]);
				else $j++;
			}
		// Beautify array, repair index
		if (is_array($account_new->quota)) $account_new->quota = array_values($account_new->quota);
		// Set used blocks
		for ($i=0; $i<count($account_new->quota); $i++) {
			$account_new->quota[$i][1] = 0;
			$account_new->quota[$i][5] = 0;
			}
		}
	// Display general-page
	$select_local = 'general';
	}

switch ($_POST['select']) {
	/* Select which part of page should be loaded and check values
	* groupmembers = page with all users which are additional members of group
	* general = startpage, general account paramters
	* samba = page with all samba-related parameters e.g. smbpassword
	* quota = page with all quota-related parameters e.g. hard file quota
	* final = last page shown before account is created/modified
	* finish = page shown after account has been created/modified
	*/
	case 'groupmembers':
		do { // X-Or, only one if() can be true
			if (isset($_POST['users']) && isset($_POST['add'])) { // Add users to list
				// Add new user
				$account_new->unix_memberUid = array_merge($account_new->unix_memberUid, $_POST['users']);
				// remove doubles
				$account_new->unix_memberUid = array_flip($account_new->unix_memberUid);
				array_unique($account_new->unix_memberUid);
				$account_new->unix_memberUid = array_flip($account_new->unix_memberUid);
				// sort users
				sort($account_new->unix_memberUid);
				break;
				}
			if (isset($_POST['members']) && isset($_POST['remove'])) { // remove users from list
				$account_new->unix_memberUid = array_delete($_POST['members'], $account_new->unix_memberUid);
				break;
				}
			} while(0);
		// display groupmembers page
		$select_local = 'groupmembers';
		break;
	case 'general':
		if (!$_POST['load']) {
			if (($account_new->general_username != $_POST['f_general_username']) &&  ereg('[A-Z]$', $_POST['f_general_username']))
				$errors[] = array('WARN', _('Groupname'), _('You are using a capital letters. This can cause problems because not all programs are case-sensitive.'));
			// Write all general attributes into $account_new if no profile should be loaded
			$account_new->general_dn = $_POST['f_general_suffix'];
			$account_new->general_username = $_POST['f_general_username'];
			$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
			$account_new->general_gecos = $_POST['f_general_gecos'];

			// Check if values are OK and set automatic values.  if not error-variable will be set
			// Check if Groupname contains only valid characters
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Groupname'), _('Groupname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
			if ($account_new->general_gecos=='') {
				$account_new->general_gecos = $account_new->general_username ;
				$errors[] = array('INFO', _('Gecos'), _('Inserted groupname in gecos-field.'));
				}
			// Create automatic groupaccount with number if original group already exists
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
					// Put groupname together
					$account_new->general_username = $firstchars . (intval($lastchars)+1);
				 	}
				}
			// Show warning if lam has changed groupname
			if ($account_new->general_username != $_POST['f_general_username']) $errors[] = array('WARN', _('Groupname'), _('Groupname already in use. Selected next free groupname.'));
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$temp = explode(':', checkid($account_new, $account_old));
			$account_new->general_uidNumber = $temp[0];
			// true if checkid has returned an error
			if ($temp[1]!='') $errors[] = explode(';',$temp[1]);
			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $account_new->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter.'));
			}
		break;
	case 'samba':
		// Write all samba attributes into $account_new
		// Get all domains
		$samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());
		// Search the corrct domain in array
		unset($account_new->smb_domain);
		$i = 0;
		while (!is_object($account_new->smb_domain) && isset($samba3domains[$i])) {
			if ($_POST['f_smb_domain'] == $samba3domains[$i]->name)
				$account_new->smb_domain = $samba3domains[$i];
			else $i++;
			}
		$account_new->smb_displayName = $_POST['f_smb_displayName'];
		// Check if group SID should be mapped to a well known SID
		switch ($_POST['f_smb_mapgroup']) {
			case '*'._('Domain Guests'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '514'; break;
			case '*'._('Domain Users'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '513'; break;
			case '*'._('Domain Admins'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '512'; break;
			case $account_new->general_username:
					$account_new->smb_mapgroup = $account_new->smb_domain->SID . "-".
						(2 * $account_new->general_uidNumber + $account_new->smb_domain->RIDbase +1);
				break;
			}
			// Check if values are OK and set automatic values. if not error-variable will be set
		if (($account_new->smb_displayName=='') && isset($account_new->general_gecos)) {
			$account_new->smb_displayName = $account_new->general_gecos;
			$errors[] = array('INFO', _('Display name'), _('Inserted gecos-field as display name.'));
			}
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
	case 'final':
		// Ask if we should change gidNumber of every user which is member of the group
		if ($_POST['f_final_changegids']) $final_changegids = $_POST['f_final_changegids'] ;
		break;
	case 'finish':
		// Check if pdf-file should be created
		if ($_POST['outputpdf']) {
			// Load quotas if not yet done because they are needed for the pdf-file
			if ($config_intern->scriptServer && !isset($account_new->quota[0])) { // load quotas
				$quotas = getquotas(array($account_old));
				$account_new->quota = $quotas[0]->quota;
				}
			// Create / display PDf-file
			createGroupPDF(array($account_new));
			// Stop script
			die;
			}
		break;
	}


do { // X-Or, only one if() can be true
	if ($_POST['next_members']) {
		// Go from groupmembers to next page if no error did ocour
		if (!is_array($errors)) $select_local='groupmembers';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_general']) {
		// Go from general to next page if no error did ocour
		if (!is_array($errors)) $select_local='general';
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
	if ($_POST['next_final']) {
		// Check if objectclasses are OK
		if (is_array($errors)) $stay=true;
			else $stay = false;
		if ($config_intern->is_samba3() && !isset($account_new->smb_domain)) {
			// Samba page not viewed; can not create group because if missing options
			$errors[] = array("ERROR", _("Samba Options not set!"), _("Please check settings on samba page."));
			$stay = true;
			}
		if (isset($account_old->general_objectClass)) {
			if (($config_intern->is_samba3()) && (!in_array('sambaGroupMapping', $account_old->general_objectClass)))
				$errors[] = array('WARN', _('ObjectClass sambaGroupMapping not found.'), _('Have to add objectClass sambaGroupMapping.'));
			if (!in_array('posixGroup', $account_old->general_objectClass))
				$errors[] = array('WARN', _('ObjectClass posixGroup not found.'), _('Have to add objectClass posixGroup.'));
			}
		// Show info if gidNumber has changed
		if (($account_old) && ($account_new->general_uidNumber != $account_old->general_uidNumber))
			$errors[] = array('INFO', _('GID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
			'find / -gid ' . $account_old->general_uidNumber . ' -exec chgrp ' . $account_new->general_uidNumber . ' {} \;');
		// Go from final to next page if no error did ocour
		if (!$stay) $select_local='final';
			else $select_local=$_POST['select'];
		break;
		}
	// Reset account to original settings if undo-button was pressed
	if ($_POST['next_reset']) {
		$account_new = $account_old;
		$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
		$select_local = $_POST['select'];
		break;
		}
	// Create-Button was pressed
	if ( $_POST['create'] ) {
		// Create or modify an account
		if ($account_old) $result = modifygroup($account_new,$account_old);
		 else $result = creategroup($account_new); // account.inc
		if ( $result==4 || $result==5 ) $select_local = 'final';
			else $select_local = 'finish';
		break;
		}
	// Load Profile and reset all attributes to settings in profile
	if ($_POST['createagain']) {
		$select_local='general';
		unset ($_SESSION['account_'.$varkey.'_account_new']);
		unset($account_new);
		$_SESSION['account_'.$varkey.'_account_new'] = loadGroupProfile('default');
		$account_new =& $_SESSION['account_'.$varkey.'_account_new'];
		$account_new ->type = 'group';
		break;
		}
	// Go back to listgroups.php
	if ($_POST['backmain']) {
		if (isset($_SESSION['account_'.$varkey.'_account_new'])) unset($_SESSION['account_'.$varkey.'_account_new']);
		if (isset($_SESSION['account_'.$varkey.'_account_old'])) unset($_SESSION['account_'.$varkey.'_account_old']);
		if (isset($_SESSION['account_'.$varkey.'_final_changegids'])) unset($_SESSION['account_'.$varkey.'_final_changegids']);
		metaRefresh("../lists/listgroups.php");
		die;
		break;
		}
	// Load Profile and reset all attributes to settings in profile
	if ($_POST['load']) {
		$account_new->general_dn = $_POST['f_general_suffix'];
		$account_new->general_username = $_POST['f_general_username'];
		$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
		$account_new->general_gecos = $_POST['f_general_gecos'];
		// load profile
		if ($_POST['f_general_selectprofile']!='') $values = loadGroupProfile($_POST['f_general_selectprofile']);
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $account_new->$key = $val;
			}
		if ($config_intern->scriptServer) {
			// load quotas and check if quotas from profile are valid
			$quotas = getquotas(array($account_new));
			for ($i=0; $i<count($account_new->quota); $i++) $profile_quotas[] = $account_new->quota[$i][0];
			for ($i=0; $i<count($quotas[0]->quota); $i++) {
				$real_quotas[] = $quotas[0]->quota[$i][0];
				if (is_array($profile_quotas)) {
					if (!in_array($quotas[0]->quota[$i][0], $profile_quotas)) $account_new->quota[]=$quotas[0]->quota[$i];
					}
				else $account_new->quota[]=$quotas[0]->quota[$i];
				}
			$j=0;
			// delete not existing quotas
			while (isset($account_new->quota[$j][0])) {
				// remove invalid quotas
				if (!in_array($account_new->quota[$j][0], $real_quotas)) unset($account_new->quota[$j]);
					else $j++;
				}
			// Beautify array, repair index
			if (is_array($account_new->quota)) $account_new->quota = array_values($account_new->quota);
			// Set used blocks
			if (isset($account_old)) {
				for ($i=0; $i<count($account_new->quota); $i++)
					for ($j=0; $j<count($quotas[0]->quota); $j++)
						if ($quotas[0]->quota[$j][0] == $account_new->quota[$i][0]) {
							$account_new->quota[$i][1] = $quotas[0]->quota[$i][1];
							$account_new->quota[$i][4] = $quotas[0]->quota[$i][4];
							$account_new->quota[$i][5] = $quotas[0]->quota[$i][5];
							$account_new->quota[$i][8] = $quotas[0]->quota[$i][8];
							}
				}
			else for ($i=0; $i<count($account_new->quota); $i++) {
				$account_new->quota[$i][1] = 0;
				$account_new->quota[$i][5] = 0;
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
			if (saveGroupProfile($account_new, $_POST['f_finish_safeProfile']))
				$errors[] = array('INFO', _('Save profile'), _('New profile created.'));
			else $errors[] = array('ERROR', _('Save profile'), _('Wrong profilename given.'));
			}
		// select last page displayed before user is created
		$select_local='final';
		break;
		}
	if ($_POST['groupmembers']) {
		$select_local='groupmembers';
		break;
		}
	} while(0);

// Write HTML-Header
echo $header_intern;
echo "<title>";
echo _("Create new Account");
echo "</title>\n".
	"<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n".
	"<meta http-equiv=\"pragma\" content=\"no-cache\">\n".
	"<meta http-equiv=\"cache-control\" content=\"no-cache\">\n".
	"</head><body>\n".
	"<form action=\"groupedit.php\" method=\"post\">\n".
	"<input name=\"varkey\" type=\"hidden\" value=\"".$varkey."\">\n";

// Display errir-messages
if (is_array($errors))
	for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);

// print_r($account_new);


switch ($select_local) {
	/* Select which part of page should be loaded and check values
	* groupmembers = page with all users which are additional members of group
	* general = startpage, general account paramters
	* samba = page with all samba-related parameters e.g. smbpassword
	* quota = page with all quota-related parameters e.g. hard file quota
	* personal = page with all personal-related parametergs, e.g. phone number
	* final = last page shown before account is created/modified
	* finish = page shown after account has been created/modified
	*/
	case 'groupmembers':
		// Validate cache-array
		ldapreload('user');
		// Get copy of cache-array
		$temp2 = $userDN_intern;
		// unset timestamp stored in $temp2[0]
		unset($temp2[0]);
		// load list with all users
		foreach ($temp2 as $temp) $users[] = $temp['uid'];
		// sort users
		if (is_array($users)) sort($users, SORT_STRING);
		// remove users which are allready additional members of group
		$users = array_delete($account_new->unix_memberUid, $users);
		/* Now we have to remove all users from list who are primary member of group
		* At the moment lam is doing an extra ldap-search. In future this should be done
		* via cache-array **** fixme
		*/
		// Do a ldap-search
		if (isset($account_old->general_uidNumber))
			$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), "(&(objectClass=PosixAccount)(gidNumber=$account_old->general_uidNumber))", array('uid'));
		else $result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), "(&(objectClass=PosixAccount)(gidNumber=$account_new->general_uidNumber))", array('uid'));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		// loop for every user which is primary member of group
		while ($entry) {
			$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if (isset($attr['uid'][0])) {
				// Remove user from user list
				$users = @array_flip($users);
				unset ($users[$attr['uid'][0]]);
				$users = @array_flip($users);
				}
			// Go to next entry
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}

		echo "<input name=\"select\" type=\"hidden\" value=\"groupmembers\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" disabled value=\""; echo _('Members'); echo "\">\n<br>";
		// samba 2.2 doesn't have any settings for groups
		if ($config_intern->is_samba3()) {
			echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
			}
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
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
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>". _('Additional group members') . "</b></legend>\n";
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr><td valign=\"top\"><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\">";
		echo _('Group members');
		echo "</legend>";
		// display all users which are additional members of group
		if (count($account_new->unix_memberUid)!=0) {
			echo "<select name=\"members[]\" class=\"groupedit-bright\" size=15 multiple>\n";
			for ($i=0; $i<count($account_new->unix_memberUid); $i++)
				if ($account_new->unix_memberUid[$i]!='') echo "		<option>".$account_new->unix_memberUid[$i]."</option>\n";
			echo "</select>\n";
			}
		echo "</fieldset></td>\n";
		echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"add\" value=\"<=\">";
		echo " ";
		echo "<input type=\"submit\" name=\"remove\" value=\"=>\"><br><br>";
		echo "<a href=\"../help.php?HelpNumber=419\" target=\"lamhelp\">"._('Help')."</a></td>\n";
		echo "<td valign=\"top\"><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\">";
		echo _('Available users');
		echo "</legend>\n";
		// Display all users which are not member of group in any way
		if ((count($users)!=0) && is_array($users)) {
			echo "<select name=\"users[]\" size=15 multiple class=\"groupedit-bright\">\n";
			foreach ($users as $temp)
				echo "		<option>$temp</option>\n";
			echo "</select>\n";
			}
		echo "</fieldset></td>\n</tr>\n</table>\n</fieldset></td></tr></table>\n</td></tr>\n</table>\n";
		break;
	case 'general':
		// General Account Settings
		// load list of profiles
		$profilelist = getGroupProfiles();
		// Show page info
		echo "<input name=\"select\" type=\"hidden\" value=\"general\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" disabled value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		// samba 2.2 doesn't have any settings for groups
		if ($config_intern->is_samba3()) {
			echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
			}
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
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
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
		echo _("General properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _("Groupname")."*";
		echo "</td>\n<td>".
			"<input name=\"f_general_username\" type=\"text\" size=\"30\" maxlength=\"20\" value=\"".$account_new->general_username."\">".
			"</td>\n<td><a href=\"../help.php?HelpNumber=407\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('GID number');
		echo "</td>\n<td><input name=\"f_general_uidNumber\" type=\"text\" size=\"30\" maxlength=\"6\" value=\"".$account_new->general_uidNumber."\">".
			"</td>\n<td><a href=\"../help.php?HelpNumber=408\" target=\"lamhelp\">"._('Help').
			"</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Description');
		echo "</td>\n<td><input name=\"f_general_gecos\" type=\"text\" size=\"30\" value=\"".$account_new->general_gecos."\"></td>\n".
			"<td><a href=\"../help.php?HelpNumber=409\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Suffix'); echo "</td>\n<td><select name=\"f_general_suffix\">";
		// Display all allowed group suffixes
		foreach ($ldap_intern->search_units($config_intern->get_GroupSuffix()) as $suffix) {
			if ($account_new->general_dn) {
				if ($account_new->general_dn == $suffix)
					echo "	<option selected>$suffix</option>\n";
				else echo "	<option>$suffix</option>\n";
				}
			else echo "	<option>$suffix</option>\n";
			}
		echo "</select></td>\n<td><a href=\"../help.php?HelpNumber=462\" target=\"lamhelp\">"._('Help').
			"</a></td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr><tr><td>";
		// Show fieldset with list of all group profiles
		if (count($profilelist)!=0) {
			echo "<fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
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
	case 'samba':
		// Samba Settings
		// samba 2.2 doesn't have any settings for groups
		$samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());
		echo "<input name=\"select\" type=\"hidden\" value=\"samba\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" disabled value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
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
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>"._('Samba properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _("Display name");
		echo "</td>\n<td>".
			"<input name=\"f_smb_displayName\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"".$account_new->smb_displayName."\">".
			"</td>\n<td><a href=\"../help.php?HelpNumber=420\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Windows groupname');
		echo "</td>\n<td><select name=\"f_smb_mapgroup\">";
		// Display if group SID should be mapped to a well kown SID
		if ( $account_new->smb_mapgroup == $account_new->smb_domain->SID . "-".
		(2 * getgid($account_new->general_username) + $values->smb_domain->RIDbase+1)) {
			echo '<option selected> ';
			echo $account_new->general_username;
			echo "</option>\n"; }
		 else {
			echo '<option> ';
			echo $account_new->general_username;
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
		echo	"</select></td>\n<td>".
			'<a href="../help.php?HelpNumber=464" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Domain');
		echo '</td><td>';
		// select which domain name should be displayed
		if (count($samba3domains)!=0) {
			echo '<select name="f_smb_domain">';
			for ($i=0; $i<sizeof($samba3domains); $i++) {
				if ($account_new->smb_domain->name) {
					if ($account_new->smb_domain->name == $samba3domains[$i]->name)
						echo '<option selected>' . $samba3domains[$i]->name. '</option>';
					else echo '<option>' . $samba3domains[$i]->name. '</option>';
					}
				else echo '<option>' . $samba3domains[$i]->name. '</option>';
				}
			echo	'</select>';
			}
		echo "</td>\n<td><a href=\"../help.php?HelpNumber=467\" target=\"lamhelp\">"._('Help')."</a></td></tr>\n";
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;
	case 'quota':
		// Quota Settings
		// Load quotas if not yet done
		if ($config_intern->scriptServer && !isset($account_new->quota[0]) ) { // load quotas
			$quotas = getquotas(array($account_new));
			$account_new->quota = $quotas[0]->quota;
			}
		echo "<input name=\"select\" type=\"hidden\" value=\"samba\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		// samba 2.2 doesn't have any settings for groups
		if ($config_intern->is_samba3()) {
			echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
			}
		echo "<input name=\"next_quota\" type=\"submit\" disabled value=\""; echo _('Quota'); echo "\">\n<br>";
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
		echo '<input name="select" type="hidden" value="quota">';
		echo "<table border=0><tr><td><fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>"._('Quota properties')."</b></legend>\n";
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
	case 'final':
		// Final Settings
		$disabled = "";
		if ($config_intern->is_samba3() && !isset($account_new->smb_domain))
			// Samba page not viewed; can not create group because if missing options
			$disabled = "disabled";

		echo '<input name="select" type="hidden" value="final">';
		echo "<input name=\"select\" type=\"hidden\" value=\"final\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		if ($config_intern->is_samba3()) {
			echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
			}
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($config_intern->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" disabled value=\""; echo _('Final');
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
		echo "<fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _("Save profile");
		echo "</b></legend>\n";
		echo '<input name="f_finish_safeProfile" type="text" size="30" maxlength="50">';
		echo "&nbsp;<input name=\"save\" type=\"submit\" $disabled value=\"";
		echo _('Save profile');
		echo '">&nbsp;<a href="../help.php?HelpNumber=457" target="lamhelp">'._('Help');
		echo "</a>\n</fieldset>\n</td></tr>\n<tr><td>\n";
		echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
		if ($account_old) echo _('Modify');
		 else echo _('Create');
		echo "</b></legend>\n";
		echo "<table border=0 width=\"100%\">";
		// Ask if gidNumbers of primary group members should be changed
		if (($account_old) && ($account_new->general_uidNumber != $account_old->general_uidNumber)) {
			echo '<tr><td>';
			echo '<input name="f_final_changegids" type="checkbox"';
				if ($final_changegids) echo ' checked ';
			echo ' >';
			echo _('Change GID-Number of all users in group to new value');
			echo '</td></tr>'."\n";
			}
		echo "<tr><td><input name=\"create\" type=\"submit\" $disabled value=\"";
		if ($account_old) echo _('Modify Account');
		 else echo _('Create Account');
		echo '">'."\n";
		echo "</td></tr></table></fieldset>\n</td></tr></table>\n</tr></table>";
		break;

	case 'finish':
		// Final Settings
		echo '<input name="select" type="hidden" value="finish">';
		echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>"._('Note')."</b></legend>\n";
		if ($account_old) {
			printf(_("Group %s has been modified."), $account_new->general_username);
		}
		else {
			printf(_("Group %s has been created."), $account_new->general_username);
		}
		echo "<br><br>";
		if (!$account_old) {
			echo '<input name="createagain" type="submit" value="'; echo _('Create another group'); echo '">';
		}
		echo '<input name="outputpdf" type="submit" value="'; echo _('Create PDF file'); echo '">'.
			'&nbsp;<input name="backmain" type="submit" value="'; echo _('Back to group list'); echo '">'.
			'</fieldset'."\n";
		break;

	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
