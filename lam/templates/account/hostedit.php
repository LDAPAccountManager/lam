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
if (!isset($_SESSION['account_'.$varkey.'_account_old'])) $_SESSION['account_'.$varkey.'_account_old'] = new account();

// Register Session-Variables with references so we don't net to change to complete code if names changes
$account_new =& $_SESSION['account_'.$varkey.'_account_new'];
if (isset($_SESSION['account_'.$varkey.'_account_old'])) $account_old =& $_SESSION['account_'.$varkey.'_account_old'];

$ldap_intern =& $_SESSION['ldap'];
$config_intern =& $_SESSION['config'];
$lamurl_intern =& $_SESSION['lamurl'];
$header_intern =& $_SESSION['header'];



if (isset($_GET['DN'])) {
	if (isset($_GET['DN']) && $_GET['DN']!='') {
		if (isset($_SESSION['account_'.$varkey.'_account_old'])) {
			unset($account_old);
			unset($_SESSION['account_'.$varkey.'_account_old']);
			$_SESSION['account_'.$varkey.'_account_old'] = new account();
			$account_old =& $_SESSION['account_'.$varkey.'_account_old'];
			}
		$DN = str_replace("\'", '',$_GET['DN']);
		$account_new = loadhost($DN);
		$account_new->smb_flagsW = 1;
		$account_new->smb_flagsX = 1;
		$account_old = $account_new;
		// Store only DN without uid=$name
		$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
		$_SESSION['final_changegids'] = '';
		}
	}
else if (count($_POST)==0) { // Startcondition. hostedit.php was called from outside
	$account_new = loadHostProfile('default');
	$account_new ->type = 'host';
	$account_new->smb_flagsW = 1;
	$account_new->smb_flagsX = 1;
	$account_new->general_homedir = '/dev/null';
	$account_new->general_shell = '/bin/false';
	if (isset($_SESSION['account_'.$varkey.'_account_old'])) {
		unset($account_old);
		unset($_SESSION['account_'.$varkey.'_account_old']);
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
		// Write all general values into $account_new if no profile should be loaded
		if (!$_POST['load']) {
			$account_new->general_dn = $_POST['f_general_suffix'];
			$account_new->general_username = $_POST['f_general_username'];
			$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
			$account_new->general_group = $_POST['f_general_group'];
			$account_new->general_gecos = $_POST['f_general_gecos'];

			// Check if values are OK and set automatic values.  if not error-variable will be set
			if ( substr($account_new->general_username, strlen($account_new->general_username)-1, strlen($account_new->general_username)) != '$' ) {
				$account_new->general_username = $account_new->general_username . '$';
				$errors[] = array('WARN', _('Host name'), _('Added $ to hostname.'));
				}
			$tempname = $account_new->general_username;
			// Check if Hostname contains only valid characters
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[$])*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Host name'), _('Hostname contains invalid characters. Valid characters are: a-z, 0-9 and .-_ !'));

			if ($account_new->general_gecos=='') {
				$account_new->general_gecos = $account_new->general_username;
				$errors[] = array('INFO', _('Gecos'), _('Inserted hostname in gecos-field.'));
				}
			// Create automatic Hostname with number if original user already exists
			// Reset name to original name if new name is in use
			if (ldapexists($account_new, 'host', $account_old) && is_object($account_old))
				$account_new->general_username = $account_old->general_username;
			while ($temp = ldapexists($account_new, 'host', $account_old)) {
				// get last character of username
				$account_new->general_username = substr($account_new->general_username, 0, $account_new->general_username-1);
				$lastchar = substr($account_new->general_username, strlen($account_new->general_username)-2, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
					$account_new->general_username = $account_new->general_username . '2';
				 else {
				 	$i=strlen($account_new->general_username)-3;
					$mark = false;
				 	while (!$mark) {
						if (ereg('^([0-9])+$',substr($account_new->general_username, $i, strlen($account_new->general_username)-1))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($account_new->general_username, 0, $i+1);
					$lastchars = substr($account_new->general_username, $i+1, strlen($account_new->general_username)-$i);
					$account_new->general_username = $firstchars . (intval($lastchars)+1). '$';
				 	}
				$account_new->general_username = $account_new->general_username . "$";
				}
			if ($account_new->general_username != $tempname)
				$errors[] = array('WARN', _('Host name'), _('Hostname already in use. Selected next free hostname.'));

			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$account_new->general_uidNumber = checkid($account_new, 'host', $account_old);
			if (is_string($account_new->general_uidNumber)) { // true if checkid has returned an error
				$errors[] = array('ERROR', _('ID-Number'), $account_new->general_uidNumber);
				unset($account_new->general_uidNumber);
				}
			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $account_new->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));

			}
		break;

	case 'samba':
		// Write all general values into $account_new
		$account_new->smb_displayName = $_POST['f_smb_displayName'];

		if (isset($_POST['f_smb_flagsD'])) $account_new->smb_flagsD = true;
			else $account_new->smb_flagsD = false;

		if ($config_intern->is_samba3()) {
			$samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());
			for ($i=0; $i<sizeof($samba3domains); $i++)
				if ($_POST['f_smb_domain'] == $samba3domains[$i]->name) {
					$account_new->smb_domain = $samba3domains[$i];
					}
			}
		else {
			$account_new->smb_domain = $_POST['f_smb_domain'];
			}
		// Check if values are OK and set automatic values. if not error-variable will be set
		if (($account_new->smb_displayName=='') && isset($account_new->general_gecos)) {
			$account_new->smb_displayName = $account_new->general_gecos;
			$errors[] = array('INFO', _('Display name'), _('Inserted gecos-field as display name.'));
			}

		if ((!$account_new->smb_domain=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $account_new->smb_domain))
			$errors[] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));

		// Reset password if reset button was pressed. Button only vissible if account should be modified
		if ($_POST['respass']) {
			$account_new->unix_password_no=true;
			$account_new->smb_password_no=true;
			$select_local = 'samba';
			}
		break;
	case 'final':
		$select_local = 'final';
		break;
	}


do { // X-Or, only one if() can be true
	if ($_POST['next_general']) {
		if (!is_array($errors)) $select_local='general';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_samba']) {
		if (!is_array($errors)) $select_local='samba';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_final']) {
		if (!is_array($errors)) $select_local='final';
			else $select_local=$_POST['select'];
		break;
		}
	if ($_POST['next_reset']) {
		$account_new = $account_old;
		$account_new->unix_password='';
		$account_new->smb_password='';
		$account_new->smb_flagsW = 0;
		$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
		$select_local = $_POST['select'];
		break;
		}
	if ( $_POST['create'] ) { // Create-Button was pressed
		// Create or modify an account
		if ($account_old) $result = modifyhost($account_new,$account_old);
		 else $result = createhost($account_new); // account.inc
		if ( $result==1 || $result==3 ) $select_local = 'finish';
		 else $select_local = 'final';
		}
	if ($_POST['createagain']) {
		$select_local='general';
		unset($account_new);
		$account_new = loadHostProfile('default');
		$account_new ->type = 'host';
		break;
		}
	if ($_POST['load']) {
		$account_new->general_dn = $_POST['f_general_suffix'];
		$account_new->general_username = $_POST['f_general_username'];
		$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
		$account_new->general_group = $_POST['f_general_group'];
		$account_new->general_gecos = $_POST['f_general_gecos'];
		// load profile
		if ($_POST['f_general_selectprofile']!='') $values = loadHostProfile($_POST['f_general_selectprofile']);
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $account_new->$key = $val;
			}
		// select general page after group has been loaded
		$select_local='general';
		break;
		}
	if ($_POST['save']) {
		// save profile
		saveHostProfile($account_new, $_POST['f_finish_safeProfile']);
		// select last page displayed before user is created
		$select_local='final';
		break;
		}
	if ($_POST['backmain']) {
		metaRefresh($lamurl_intern."templates/lists/listhosts.php");
		die;
		break;
		}
	if (!$select_local) $select_local='general';
	} while(0);

// Write HTML-Header
echo $header_intern;
echo "<html><head><title>";
echo _("Create new Account");
echo "</title>\n".
	"<link rel=\"stylesheet\" type=\"text/css\" href=\"".$lamurl_intern."style/layout.css\">\n".
	"<meta http-equiv=\"pragma\" content=\"no-cache\">\n".
	"<meta http-equiv=\"cache-control\" content=\"no-cache\">\n".
	"</head><body>\n".
	"<form action=\"hostedit.php\" method=\"post\">\n".
	"<input name=\"varkey\" type=\"hidden\" value=\"".$varkey."\">\n";

if (is_array($errors)) {
	echo "<table class=\"account\" width=\"100%\">\n";
	for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);
	echo "</table>";
	}


// print_r($account_new);


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
		// load list of profiles
		$profilelist = getHostProfiles();
		// Show page info
		echo '<input name="select" type="hidden" value="general">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"hostedit-dark\"><legend class=\"hostedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" disabled value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
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
		echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
		echo _("General properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _('Host name').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $account_new->general_username . '">'.
			'</td><td>'.
			'<a href="'.$lamurl_intern.'templates/help.php?HelpNumber=410" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('UID number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $account_new->general_uidNumber . '">'.
			'</td>'."\n".'<td>'.
			'<a href="'.$lamurl_intern.'templates/help.php?HelpNumber=411" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Primary group').'*';
		echo '</td>'."\n".'<td><select name="f_general_group">';
		foreach ($groups as $group) {
			if ($account_new->general_group == $group) echo '<option selected>' . $group. '</option>';
			else echo '<option>' . $group. '</option>';
			 }
		echo '</select></td><td>'.
			'<a href="'.$lamurl_intern.'templates/help.php?HelpNumber=412" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Gecos');
		echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $account_new->general_gecos . '">'.
			'</td>'."\n".'<td>'.
			'<a href="'.$lamurl_intern.'templates/help.php?HelpNumber=413" target="lamhelp">'._('Help').'</a>'.
			'</td></tr><tr><td>';
		echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
		foreach ($ldap_intern->search_units($config_intern->get_HostSuffix()) as $suffix) {
			if ($account_new->general_dn) {
				if ($account_new->general_dn == $suffix)
					echo '<option selected>' . $suffix. '</option>';
				else echo '<option>' . $suffix. '</option>';
				}
			else echo '<option>' . $suffix. '</option>';
			}
		echo '</select></td><td><a href="'.$lamurl_intern.'templates/help.php?HelpNumber=463" target="lamhelp">'._('Help').'</a>'.
			"</td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr><tr><td>";
		if (count($profilelist)!=0) {
			echo "<fieldset class=\"hostedit-dark\"><legend class=\"hostedit-bright\"><b>";
			echo _("Load profile");
			echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
			echo "<select name=\"f_general_selectprofile\" >";
			foreach ($profilelist as $profile) echo "	<option>$profile</option>\n";
			echo "</select></td><td>\n".
				"<input name=\"load\" type=\"submit\" value=\""; echo _('Load Profile');
			echo "\"></td><td><a href=\"".$lamurl_intern."templates/help.php?HelpNumber=421\" target=\"lamhelp\">";
			echo _('Help')."</a></td>\n</tr>\n</table>\n</fieldset>\n";
			}
		echo "</td></tr></table>\n</td></tr>\n</table>\n";
		break;

	case 'samba':
		// Samba Settings
		if ($config_intern->is_samba3()) $samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());
		if ($account_new->smb_password_no) echo '<input name="f_smb_password_no" type="hidden" value="1">';
		echo '<input name="select" type="hidden" value="samba">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"hostedit-dark\"><legend class=\"hostedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" disabled value=\""; echo _('Samba'); echo "\">\n<br>";
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
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>"._('Samba properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _("Display name");
		echo "</td>\n<td>".
			"<input name=\"f_smb_displayName\" type=\"text\" size=\"30\" maxlength=\"50\" value=\"".$account_new->smb_displayName."\">".
			"</td>\n<td><a href=\"".$lamurl_intern."templates/help.php?HelpNumber=420\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Password');
		echo '</td><td>';
		if (isset($account_old)) {
			echo '<input name="respass" type="submit" value="';
			echo _('Reset password'); echo '">';
			}
		echo '</td></tr>'."\n".'<tr><td>';
		echo _('Account is deactivated');
		echo '</td>'."\n".'<td><input name="f_smb_flagsD" type="checkbox"';
		if ($account_new->smb_flagsD) echo ' checked ';
		echo '></td><td>'.
			'<a href="'.$lamurl_intern.'templates/help.php?HelpNumber=432" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo '</td></tr>'."\n".'<tr><td>';
		echo _('Domain');
		if ($config_intern->is_samba3()) {
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
			echo '</td>'."\n".'<td><input name="f_smb_domain" type="text" size="20" maxlength="80" value="' . $account_new->smb_domain . '">';
			}
		echo	'</td>'."\n".'<td><a href="'.$lamurl_intern.'templates/help.php?HelpNumber=460" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</td></tr></table></td></tr>\n</table>\n";
		break;

	case 'final':
		// Final Settings
		echo '<input name="select" type="hidden" value="final">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr><td><fieldset class=\"hostedit-dark\"><legend class=\"hostedit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" disabled value=\""; echo _('Final');
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
		echo "<table border=0 width=\"100%\"><tr><td><fieldset class=\"hostedit-dark\"><legend class=\"hostedit-bright\"><b>";
		echo _("Save profile");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo '<input name="f_finish_safeProfile" type="text" size="30" maxlength="50">';
		echo '</td><td><input name="save" type="submit" value="';
		echo _('Save profile');
		echo '"></td><td><a href="'.$lamurl_intern.'templates/help.php?HelpNumber=457" target="lamhelp">'._('Help');
		echo "</a></td>\n</tr>\n</table>\n</fieldset>\n</td></tr>\n<tr><td>\n";
		echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
		if ($account_old) echo _('Modify');
		 else echo _('Create');
		echo "</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		if (isset($account_old->general_objectClass)) {
			if (!in_array('posixAccount', $account_old->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass posixAccount not found.'), _('Have to add objectClass posixAccount.'));
				echo "</tr>\n";
				}
			if (!in_array('shadowAccount', $account_old->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass shadowAccount not found.'), _('Have to add objectClass shadowAccount.'));
				echo "</tr>\n";
				}
			if ($config_intern->is_samba3()) {
				if (!in_array('sambaSamAccount', $account_old->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass sambaSamAccount not found.'), _('Have to add objectClass sambaSamAccount. Host with sambaAccount will be updated.'));
					echo "</tr>\n";
					}}
				else
				if (!in_array('sambaAccount', $account_old->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass sambaAccount not found.'), _('Have to add objectClass sambaSamAccount. Host with sambaSamAccount will be set back to sambaAccount.'));
					echo "</tr>\n";
					}
			}
		echo '<input name="create" type="submit" value="';
		if ($account_old) echo _('Modify Account');
		 else echo _('Create Account');
		echo '">'."\n";
		echo "</td></tr></table></fieldset>\n</td></tr></table></td></tr></table>\n</tr></table>";
		break;
	case 'finish':
		// Final Settings
		echo '<input name="select" type="hidden" value="finish">';
		echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>"._('Success')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo '<tr><td>';
		echo _('Host');
		echo ' '.$account_new->general_username.' ';
		if ($account_old) echo ' '._('has been modified').'.';
		 else echo ' '._('has been created').'.';
		echo '</td></tr>'."\n".'<tr><td>';
		if (!$account_old)
			{ echo '<input name="createagain" type="submit" value="'; echo _('Create another host'); echo '">'; }
		echo '</td><td>'."\n".'</td><td>'.
			'<input name="backmain" type="submit" value="'; echo _('Back to host list'); echo '">'.
			'</td></tr></table></fieldset'."\n";
		break;
	case 'backmain':
		// unregister sessionvar and select which list should be shown
		echo '<a href="'.$lamurl_intern.'templates/lists/listhosts.php">';
		echo _('Please press here if meta-refresh didn\'t work.');
		echo "</a>\n";
		if (isset($_SESSION['account_'.$varkey.'_account_new'])) unset($_SESSION['account_'.$varkey.'_account_new']);
		if (isset($_SESSION['account_'.$varkey.'_account_old'])) unset($_SESSION['account_'.$varkey.'_account_old']);
		break;
	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
