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

if (isset($_POST['select'])) $select =& $_POST['select'];
if (isset($_POST['load'])) $load =& $_POST['load'];

if (isset($_GET['DN']) && $_GET['DN']!='') {
	if (isset($_SESSION['account_'.$varkey.'_account_old'])) {
		unset($account_old);
		unset($_SESSION['account_'.$varkey.'_account_old']);
		}
	$_SESSION['account_'.$varkey.'_account_old'] = new account();
	$account_old =& $_SESSION['account_'.$varkey.'_account_old'];
	$DN = str_replace("\'", '',$_GET['DN']);
	$account_new = loadgroup($DN);
	$account_old = $account_new;
	$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
	$final_changegids = '';
	}

else if (count($_POST)==0) { // Startcondition. groupedit.php was called from outside
	$account_new = loadGroupProfile('default');
	$account_new ->type = 'group';
	if ($config_intern->scriptServer) {
		// load quotas from profile and check if they are valid
		$values = getquotas('group');
		if (isset($account_new->quota[0])) { // check quotas from profile
			$i=0;
			// check quota settings
			while (isset($account_new->quota[$i])) {
				$found = (-1);
				for ($j=0; $j<count($values->quota); $j++)
					if ($values->quota[$j][0]==$account_new->quota[$i][0]) $found = $j;
				if ($found==-1) unset($account_new->quota[$i]);
				else {
					$account_new->quota[$i][1] = $values->quota[$found][1];
					$account_new->quota[$i][5] = $values->quota[$found][5];
					$account_new->quota[$i][4] = $values->quota[$found][4];
					$account_new->quota[$i][8] = $values->quota[$found][8];
					$i++;
					}
				}
			$account_new->quota = array_values($account_new->quota);
			}
		else { // No quotas saved in profile
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $account_new->$key = $val;
				}
			}
		}
	unset($account_old);
	unset($_SESSION['account_'.$varkey.'_account_old']);
	}

switch ($select) { // Select which part of page should be loaded and check values
	// general = startpage, general account paramters
	// samba = page with all samba-related parameters e.g. smbpassword
	// quota = page with all quota-related parameters e.g. hard file quota
	// personal = page with all personal-related parametergs, e.g. phone number
	// final = last page shown before account is created/modified
	//		if account is modified commands might be ran are shown
	// finish = page shown after account has been created/modified
	case 'groupmembers':
		do { // X-Or, only one if() can be true
			if (isset($_POST['users']) && isset($_POST['add'])) { // Add users to list
				// Add new user
				$account_new->unix_memberUid = array_merge($account_new->unix_memberUid, $_POST['users']);
				// remove doubles
				$account_new->unix_memberUid = array_flip($account_new->unix_memberUid);
				array_unique($account_new->unix_memberUid);
				$account_new->unix_memberUid = array_flip($account_new->unix_memberUid);
				// sort user
				sort($account_new->unix_memberUid);
				// display groupmembers page
				break;
				}
			if (isset($_POST['members']) && isset($_POST['remove'])) { // remove users fromlist
				$account_new->unix_memberUid = array_delete($_POST['members'], $account_new->unix_memberUid);
				break;
				}
			} while(0);
		$select_local = 'groupmembers';
		break;

	case 'general':
		// Write all general values into $account_new if no profile should be loaded
		if (!$load) {
			$account_new->general_dn = $_POST['f_general_suffix'];
			$account_new->general_username = $_POST['f_general_username'];
			$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
			$account_new->general_gecos = $_POST['f_general_gecos'];

			// Check if values are OK and set automatic values.  if not error-variable will be set

			// Check if Groupname contains only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Groupname'), _('Groupname contains invalid characters. Valid characters are: a-z, 0-9 and .-_ !'));
			if ($account_new->general_gecos=='') {
				$account_new->general_gecos = $account_new->general_username ;
				$errors[] = array('INFO', _('Gecos'), _('Inserted groupname in gecos-field.'));
				}
			// Create automatic groupaccount with number if original group already exists
			// Reset name to original name if new name is in use
			if (ldapexists($account_new, 'group', $account_old) && is_object($account_old))
				$account_new->general_username = $account_old->general_username;
			while ($temp = ldapexists($account_new, 'group', $account_old)) {
				// get last character of username
				$lastchar = substr($account_new->general_username, strlen($account_new->general_username)-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
					$account_new->general_username = $account_new->general_username . '2';
				 else {
				 	$i=strlen($account_new->general_username)-1;
					$mark = false;
				 	while (!$mark) {
						if (ereg('^([0-9])+$',substr($account_new->general_username, $i, strlen($account_new->general_username)-$i))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($account_new->general_username, 0, $i+1);
					$lastchars = substr($account_new->general_username, $i+1, strlen($account_new->general_username)-$i);
					$account_new->general_username = $firstchars . (intval($lastchars)+1);
				 	}
				}
			if ($account_new->general_username != $_POST['f_general_username']) $errors[] = array('WARN', _('Groupname'), _('Groupname already in use. Selected next free groupname.'));

			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$account_new->general_uidNumber = checkid($account_new, 'group', $account_old);
			if (is_string($account_new->general_uidNumber)) { // true if checkid has returned an error
				$errors[] = array('ERROR', _('ID-Number'), $account_new->general_uidNumber);
				if (isset($account_old)) $account_new->general_uidNumber = $account_old->general_uidNumber;
				else unset($account_new->general_uidNumber);
				}

			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $account_new->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));

			}
		break;

	case 'samba':
		$samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());
		foreach ($samba3domains as $domain)
			if ($_POST['f_smb_domain'] == $domain->name)
				$account_new->smb_domain = $domain;
		$account_new->smb_displayName = $_POST['f_smb_displayName'];

		if ($config_intern->is_samba3())
			switch ($_POST['f_smb_mapgroup']) {
				case '*'._('Domain Guests'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '514'; break;
				case '*'._('Domain Users'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '513'; break;
				case '*'._('Domain Admins'): $account_new->smb_mapgroup = $account_new->smb_domain->SID . "-" . '512'; break;
				case $account_new->general_username:
						$account_new->smb_mapgroup = $account_new->smb_domain->SID . "-".
							(2 * getgid($account_new->general_username) + $account_new->smb_domain->RIDbase +1);
					break;
				}
		else
			switch ($_POST['f_smb_mapgroup']) {
				case '*'._('Domain Guests'): $account_new->smb_mapgroup = '514'; break;
				case '*'._('Domain Users'): $account_new->smb_mapgroup = '513'; break;
				case '*'._('Domain Admins'): $account_new->smb_mapgroup = '512'; break;
				case $account_new->general_username:
					$account_new->smb_mapgroup = (2 * getgid($account_new->general_username) + 1001);
					break;
				}

		// Check if value is set
		if (($account_new->smb_displayName=='') && isset($account_new->general_gecos)) {
			$account_new->smb_displayName = $account_new->general_gecos;
			$errors[] = array('INFO', _('Display name'), _('Inserted gecos-field as display name.'));
			}

		break;

	case 'quota':
		// Write all general values into $account_new
		$i=0;
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
		// Write all general values into $account_new
		if ($_POST['f_final_changegids']) $final_changegids = $_POST['f_final_changegids'] ;
		break;

	}


do { // X-Or, only one if() can be true
	if ($_POST['next_members']) {
		if (!is_array($errors)) $select_local='groupmembers';
			else $select_local=$select;
		break;
		}
	if ($_POST['next_general']) {
		if (!is_array($errors)) $select_local='general';
			else $select_local=$select;
		break;
		}
	if ($_POST['next_samba']) {
		if (!is_array($errors)) $select_local='samba';
			else $select_local=$select;
		break;
		}
	if ($_POST['next_quota']) {
		if (!is_array($errors)) $select_local='quota';
			else $select_local=$select;
		break;
		}
	if ($_POST['next_final']) {
		if (!isset($errors)) $select_local='final';
			else $select_local=$select;
		break;
		}
	if ($_POST['next_reset']) {
		$account_new = $account_old;
		$account_new->unix_password='';
		$account_new->smb_password='';
		$account_new->smb_flagsW = 0;
		$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
		$select_local = $select;
		break;
		}
	if ( $_POST['create'] ) { // Create-Button was pressed
		if ($account_old) $result = modifygroup($account_new,$account_old);
		 else $result = creategroup($account_new); // account.inc
		if ( $result==1 || $result==3 ) $select_local = 'finish';
			else $select_local = 'final';
		break;
		}
	// Reset variables if recreate-button was pressed
	if ($_POST['createagain']) {
		$select_local='general';
		unset($account_new);
		$account_new = loadGroupProfile('default');
		$account_new ->type = 'group';
		break;
		}
	if ($_POST['backmain']) {
		metaRefresh("../lists/listgroups.php");
		if (isset($_SESSION['account_'.$varkey.'_account_new'])) unset($_SESSION['account_'.$varkey.'_account_new']);
		if (isset($_SESSION['account_'.$varkey.'_account_old'])) unset($_SESSION['account_'.$varkey.'_account_old']);
		if (isset($_SESSION['account_'.$varkey.'_final_changegids'])) unset($_SESSION['account_'.$varkey.'_final_changegids']);
		die;
		break;
		}
	if ($load) {
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
			// load quotas from profile and check if they are valid
			$values = getquotas('group', $account_old->general_username);
			if (isset($account_new->quota[0])) { // check quotas from profile
				$i=0;
				// check quota settings
				while (isset($account_new->quota[$i])) {
					$found = (-1);
					for ($j=0; $j<count($values->quota); $j++)
						if ($values->quota[$j][0]==$account_new->quota[$i][0]) $found = $j;
					if ($found==-1) unset($account_new->quota[$i]);
						else {
						$account_new->quota[$i][1] = $values->quota[$found][1];
						$account_new->quota[$i][5] = $values->quota[$found][5];
						$account_new->quota[$i][4] = $values->quota[$found][4];
						$account_new->quota[$i][8] = $values->quota[$found][8];
						$i++;
						}
					}
				$account_new->quota = array_values($account_new->quota);
				}
			else { // No quotas saved in profile
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
	if ($_POST['save']) {
		// save profile
		saveGroupProfile($account_new, $_POST['f_finish_safeProfile']);
		// select last page displayed before user is created
		$select_local='final';
		break;
		}
	if ($_POST['groupmembers']) {
		$select_local='groupmembers';
		break;
		}
	// Set selected page to general if no page was defined. should only true if groupedit.php wasn't called by itself
	if (!$select_local) $select_local='general';
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
	"<form action=\"groupedit.php\" method=\"post\">\n".
	"<input name=\"varkey\" type=\"hidden\" value=\"".$varkey."\">\n";

if (is_array($errors)) {
	echo "<table class=\"groupedit\" width=\"100%\">\n";
	for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);
	echo "</table>";
	}

// print_r($account_old);

switch ($select_local) { // Select which part of page will be loaded
	// general = startpage, general account paramters
	// unix = page with all shadow-options and password
	// samba = page with all samba-related parameters e.g. smbpassword
	// quota = page with all quota-related parameters e.g. hard file quota
	// personal = page with all personal-related parametergs, e.g. phone number
	// final = last page shown before account is created/modified
	//		if account is modified commands might be ran are shown
	// finish = page shown after account has been created/modified
	case 'groupmembers':
		ldapreload('user');
		$temp2 = $userDN_intern;
		unset($temp2[0]);
		foreach ($temp2 as $temp) $users[] = $temp['cn'];
		sort($users, SORT_STRING);
		echo "<input name=\"select\" type=\"hidden\" value=\"groupmembers\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" disabled value=\""; echo _('Members'); echo "\">\n<br>";
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
		echo "<table border=0><tr><td><fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>". _('Additional group members') . "</b></legend>\n";
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr><td valign=\"top\"><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\">";
		echo _('Group members');
		echo "</legend>";
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
		if (count($users)!=0) {
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
		if ($config_intern->samba3=='yes') {
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
			}
		else {
			if ( $account_new->smb_mapgroup == (2 * getgid($account_new->general_username) +1001)) {
				echo '<option selected> ';
				echo $account_new->general_username;
				echo "</option>\n"; }
			 else {
				echo '<option> ';
				echo $account_new->general_username;
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
		echo	"</select></td>\n<td>".
			'<a href="../help.php?HelpNumber=464" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Domain');
		echo '</td><td><select name="f_smb_domain">';
		for ($i=0; $i<sizeof($samba3domains); $i++) {
			if ($account_new->smb_domain->name) {
				if ($account_new->smb_domain->name == $samba3domains[$i]->name)
					echo '<option selected>' . $samba3domains[$i]->name. '</option>';
				else echo '<option>' . $samba3domains[$i]->name. '</option>';
				}
			else echo '<option>' . $samba3domains[$i]->name. '</option>';
			}
		echo	'</select></td>'."\n".'<td><a href="../help.php?HelpNumber=467" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;

	case 'quota':
		// Quota Settings
		if ($config_intern->scriptServer && !isset($account_new->quota[0]) ) { // load quotas
			$values = getquotas('group', $account_new->general_username);
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if (isset($val)) $account_new->$key = $val;
				}
			if (is_object($values) && isset($account_old)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if (isset($val)) $account_old->$key = $val;
				}
			}

		echo "<input name=\"select\" type=\"hidden\" value=\"samba\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table border=0><tr><td><fieldset class=\"groupedit-middle\"><legend class=\"groupedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
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
		if ($config_intern->is_samba3()) {
			if (!isset($account_new->smb_domain)) { // Samba page nit viewd; can not create group because if missing options
				$disabled = "disabled";
				}
			}

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
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo '<input name="f_finish_safeProfile" type="text" size="30" maxlength="50">';
		echo "</td><td><input name=\"save\" type=\"submit\" $disabled value=\"";
		echo _('Save profile');
		echo '"></td><td><a href="../help.php?HelpNumber=457" target="lamhelp">'._('Help');
		echo "</a></td>\n</tr>\n</table>\n</fieldset>\n</td></tr>\n<tr><td>\n";
		echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
		if ($account_old) echo _('Modify');
		 else echo _('Create');
		echo "</b></legend>\n";
		echo "<table border=0 width=\"100%\">";
		if (($account_old) && ($account_new->general_uidNumber != $account_old->general_uidNumber)) {
			echo '<tr>';
			StatusMessage ('INFO', _('GID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
			'find / -gid ' . $account_old->general_uidNumber . ' -exec chgrp ' . $account_new->general_uidNumber . ' {} \;');
			echo '</tr>'."\n";
			echo '<tr><td>';
			echo '<input name="f_final_changegids" type="checkbox"';
				if ($final_changegids) echo ' checked ';
			echo ' >';
			echo _('Change GID-Number of all users in group to new value');
			echo '</td></tr>'."\n";
			}
		if ($disabled == "disabled") { // Samba page nit viewd; can not create group because if missing options
			echo "<tr>";
			StatusMessage("ERROR", _("Samba Options not set!"), _("Please check settings on samba page."));
			echo "</tr>";
			}
		if (isset($account_old->general_objectClass)) {
			if (($config_intern->is_samba3()) && (!in_array('sambaGroupMapping', $account_old->general_objectClass))) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass sambaGroupMapping not found.'), _('Have to add objectClass sambaGroupMapping.'));
				echo "</tr>\n";
				}
			if (!in_array('posixGroup', $account_old->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass posixGroup not found.'), _('Have to add objectClass posixGroup.'));
				echo "</tr>\n";
				}
			}
		echo "<tr><td><input name=\"create\" type=\"submit\" $disabled value=\"";
		if ($account_old) echo _('Modify Account');
		 else echo _('Create Account');
		echo '">'."\n";
		echo "</td></tr></table></fieldset>\n</td></tr></table>\n</tr></table>";
		break;

	case 'finish':
		// Final Settings
		if (($config_intern->samba3 =='yes') && !isset($account_new->smb_mapgroup)) $disabled = 'disabled';
			else $disabled = '';
		echo '<input name="select" type="hidden" value="finish">';
		echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>"._('Success')."</b></legend>\n";
		echo "<table border=0 width=\"100%\">";
		echo '<tr><td>';
		echo _('Group').' ';
		echo $account_new->general_username;
		if ($account_old) echo ' '._('has been modified').'.';
		 else echo ' '._('has been created').'.';
		echo '</td></tr>'."\n".'<tr><td>';
		if (!$account_old)
			{ echo' <input name="createagain" type="submit" value="'; echo _('Create another group'); echo '">'; }
		echo '</td><td></td><td>'.
			'<input name="backmain" type="submit" value="'; echo _('Back to group list'); echo '">'.
			'</td></tr></table></fieldset'."\n";
		break;

	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
