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
	metaRefresh("login.php");
	die;
	}

// Set correct language, codepages, ....
setlanguage();

/* hostedit.php is using dynamic session varialenames so
* we can run several copies of hostedit.php at the same
* time
* $varkey is the dynamic part of the variable name
*/
if (!isset($_POST['varkey'])) $varkey = session_id().time();
	else $varkey = $_POST['varkey'];
if (!isset($_SESSION['account_'.$varkey.'_account_new'])) $_SESSION['account_'.$varkey.'_account_new'] = new account();

// Register Session-Variables with references so we don't net to change to complete code if names changes
$account_new =& $_SESSION['account_'.$varkey.'_account_new'];
if (is_object($_SESSION['account_'.$varkey.'_account_old'])) $account_old =& $_SESSION['account_'.$varkey.'_account_old'];
$ldap_intern =& $_SESSION['ldap'];
$config_intern =& $_SESSION['config'];
$header_intern =& $_SESSION['header'];

// $_GET is only valid if hostedit.php was called from hostlist.php
if (isset($_GET['DN']) && $_GET['DN']!='') {
	// hostedit.php should edit an existing account
	// reset variables
	if (isset($_SESSION['account_'.$varkey.'_account_old'])) {
		unset($account_old);
		unset($_SESSION['account_'.$varkey.'_account_old']);
		}
	$_SESSION['account_'.$varkey.'_account_old'] = new account();
	$account_old =& $_SESSION['account_'.$varkey.'_account_old'];
	// get "real" DN from variable
	$DN = str_replace("\'", '',$_GET['DN']);
	// Load existing host
	$account_new = loadhost($DN);
	// Get a copy of original host
	$account_old = $account_new;
	// Store only DN without uid=$name
	$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
	}
// Startcondition. hostedit.php was called from outside to create a new host
else if (count($_POST)==0) {
	// Create new account object with settings from default profile
	$account_new = loadHostProfile('default');
	$account_new ->type = 'host';
	$account_new->smb_flagsW = 1;
	$account_new->smb_flagsX = 1;
	$account_new->general_homedir = '/dev/null';
	$account_new->general_shell = '/bin/false';
	}

switch ($_POST['select']) {
	/* Select which page should be displayed. For hosts we have
	* only have general and finish
	* general = page with all settings for hosts
	* final = page which will be displayed if changes were made
	*/
	case 'general':
		if (!$_POST['load']) {
			if (($account_new->general_username != $_POST['f_general_username']) &&  ereg('[A-Z]$', $_POST['f_general_username']))
				$errors[] = array('WARN', _('Hostname'), _('You are using a capital letters. This can cause problems because user and uSer could have the same mail-address.'));
			// Write all general values into $account_new if no profile should be loaded
			$account_new->general_dn = $_POST['f_general_suffix'];
			$account_new->general_username = $_POST['f_general_username'];
			$account_new->general_uidNumber = $_POST['f_general_uidNumber'];
			$account_new->general_group = $_POST['f_general_group'];
			$account_new->general_gecos = $_POST['f_general_gecos'];
			// Check if values are OK and set automatic values.  if not error-variable will be set
			// Add $ to end of hostname if hostname doesn't end with "$"
			if ( substr($account_new->general_username, strlen($account_new->general_username)-1, strlen($account_new->general_username)) != '$' ) {
				$account_new->general_username = $account_new->general_username . '$';
				$errors[] = array('WARN', _('Host name'), _('Added $ to hostname.'));
				}
			// Get copy of hostname so we can check if changes were made
			$tempname = $account_new->general_username;
			// Check if Hostname contains only valid characters
			if ( !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-]|[$])*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Host name'), _('Hostname contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));

			// Create automatic Hostname with number if original host already exists
			// Reset name to original name if new name is in use
			if (ldapexists($account_new, $account_old) && is_object($account_old))
				$account_new->general_username = $account_old->general_username;
			while ($temp = ldapexists($account_new, $account_old)) {
				// Remove "$" at end of hostname
				$account_new->general_username = substr($account_new->general_username, 0, $account_new->general_username-1);
				// get last character of username
				$lastchar = substr($account_new->general_username, strlen($account_new->general_username)-1, 1);
				if ( !ereg('^([0-9])+$', $lastchar)) {
					/* Last character is no number. Therefore we only have to
					* add "2" to it.
					*/
					$account_new->general_username = $account_new->general_username . '2$';
					}
				else {
					/* Last character is a number -> we have to increase the number until we've
					* found a hostname with trailing number which is not in use.
					*
					* $i will show us were we have to split hostname so we get a part
					* with the hostname and a part with the trailing number
					*/
					$i=strlen($account_new->general_username)-3;
					$mark = false;
					// Set $i to the last character which is a number in $account_new->general_username
					while (!$mark) {
						if (ereg('^([0-9])+$',substr($account_new->general_username, $i, strlen($account_new->general_username)-1))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($account_new->general_username, 0, $i+2);
					$lastchars = substr($account_new->general_username, $i+2, strlen($account_new->general_username)-$i);
					// Put hostname together
					$account_new->general_username = $firstchars . (intval($lastchars)+1). '$';
				 	}
				}
			// Show warning if lam has changed hostname
			if ($account_new->general_username != $tempname)
				$errors[] = array('WARN', _('Host name'), _('Hostname already in use. Selected next free hostname.'));
			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $account_new->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $account_new->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter.'));
			// Set gecos-field to hostname if it's empty
			if ($account_new->general_gecos=='') {
				$account_new->general_gecos = $account_new->general_username;
				$errors[] = array('INFO', _('Gecos'), _('Inserted hostname in gecos-field.'));
				}
			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$temp = explode(':', checkid($account_new, $account_old));
			$account_new->general_uidNumber = $temp[0];
			// true if checkid has returned an error
			if ($temp[1]!='') $errors[] = explode(';',$temp[1]);
			// Set Samba-Domain
			if ($config_intern->is_samba3()) {
				// Samba 3 used a samba3domain object
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
				}
			// Samba 2.2 uses only a string as domainname
			else {
				$account_new->smb_domain = $_POST['f_smb_domain'];
				// Check if Domain-name is OK
				if ((!$account_new->smb_domain=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[-])+$', $account_new->smb_domain))
					$errors[] = array('ERROR', _('Domain name'), _('Domain name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and -.'));
				}
			// Reset password if reset button was pressed. Button only vissible if account should be modified
			if ($_POST['respass']) {
				$account_new->unix_password_no=true;
				$account_new->smb_password_no=true;
				}
			}
		// Check Objectclasses. Display Warning if objectclasses don'T fot
		if (isset($account_old->general_objectClass)) {
			if (!in_array('posixAccount', $account_old->general_objectClass)) $errors[] = array('WARN', _('ObjectClass posixAccount not found.'), _('Have to add objectClass posixAccount.'));
			if (!in_array('shadowAccount', $account_old->general_objectClass)) $errors[] = array('WARN', _('ObjectClass shadowAccount not found.'), _('Have to add objectClass shadowAccount.'));
			if ($config_intern->is_samba3()) {
				if (!in_array('sambaSamAccount', $account_old->general_objectClass)) $errors[] = array('WARN', _('ObjectClass sambaSamAccount not found.'), _('Have to add objectClass sambaSamAccount. Host with sambaAccount will be updated.'));
				}
			else if (!in_array('sambaAccount', $account_old->general_objectClass)) $errors[] = array('WARN', _('ObjectClass sambaAccount not found.'), _('Have to add objectClass sambaAccount. Host with sambaSamAccount will be set back to sambaAccount.'));
			}

		break;
	case 'finish':
		// Check if pdf-file should be created
		if ($_POST['outputpdf']) {
			createHostPDF(array($account_new));
			die;
			}
		break;
	}


do { // X-Or, only one if() can be true
	// Reset account to original settings if undo-button was pressed
	if ($_POST['next_reset']) {
		$account_new = $account_old;
		$account_new->general_dn = substr($account_new->general_dn, strpos($account_new->general_dn, ',')+1);
		break;
		}
	// Create-Button was pressed
	if ( $_POST['create'] && !isset($errors)) {
		// Create or modify an account
		if ($account_old) $result = modifyhost($account_new,$account_old);
		 else $result = createhost($account_new); // account.inc
		if ($result==5 || $result==4) $select_local = 'general';
		 else $select_local = 'finish';
		}
	// Back to main-page
	if ($_POST['createagain']) {
		$select_local='general';
		unset ($_SESSION['account_'.$varkey.'_account_new']);
		unset($account_new);
		$_SESSION['account_'.$varkey.'_account_new'] = loadHostProfile('default');
		$account_new =& $_SESSION['account_'.$varkey.'_account_new'];
		$account_new ->type = 'host';
		break;
		}
	// Load Profile and reset all attributes to settings in profile
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
		$errors[] = array('INFO', _('Load profile'), _('Profile loaded.'));
		break;
		}
	// Save Profile
	if ($_POST['save']) {
		// save profile
		if ($_POST['f_finish_safeProfile']=='')
			$errors[] = array('ERROR', _('Save profile'), _('No profilename given.'));
		else {
			if (saveHostProfile($account_new, $_POST['f_finish_safeProfile']))
				$errors[] = array('INFO', _('Save profile'), _('New profile created.'));
			else $errors[] = array('ERROR', _('Save profile'), _('Wrong profilename given.'));
			}
		break;
		}
	// Go back to listhosts.php
	if ($_POST['backmain']) {
		if (isset($_SESSION['account_'.$varkey.'_account_new'])) unset($_SESSION['account_'.$varkey.'_account_new']);
		if (isset($_SESSION['account_'.$varkey.'_account_old'])) unset($_SESSION['account_'.$varkey.'_account_old']);
		metaRefresh("../lists/listhosts.php");
		die;
		break;
		}
	} while(0);
// Display main page if nothing else was selected
if (!isset($select_local)) $select_local = 'general';



// Write HTML-Header
echo $header_intern;
echo "<html><head><title>";
echo _("Create new Account");
echo "</title>\n".
	"<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n".
	"<meta http-equiv=\"pragma\" content=\"no-cache\">\n".
	"<meta http-equiv=\"cache-control\" content=\"no-cache\">\n".
	"</head><body>\n".
	"<form action=\"hostedit.php\" method=\"post\">\n".
	"<input name=\"varkey\" type=\"hidden\" value=\"".$varkey."\">\n";

// Display errir-messages
if (is_array($errors))
	for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);

// print_r($account_new);

/* Select which part of page will be loaded
* Because hosts have very less settings all are
* on a single page. Only success-message is on a
* different page
*/
switch ($select_local) {
	// general = startpage, all account paramters
	// finish = page shown after account has been created/modified
	case 'general':
		// General Account Settings
		// load list of all groups
		$groups = findgroups();
		// load list of profiles
		$profilelist = getHostProfiles();
		// Get List of all domains
		if ($config_intern->is_samba3()) $samba3domains = $ldap_intern->search_domains($config_intern->get_domainSuffix());

		// Why this ?? fixme
		if ($account_new->smb_password_no) echo '<input name="f_smb_password_no" type="hidden" value="1">';


		// Show page info
		echo '<input name="select" type="hidden" value="general">';
		// Show fieldset with list of all host profiles
		if (count($profilelist)!=0) {
			echo "<fieldset class=\"hostedit-dark\"><legend class=\"hostedit-bright\"><b>";
			echo _("Load profile");
			echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td width=\"50%\">";
			echo "<select name=\"f_general_selectprofile\" >";
			foreach ($profilelist as $profile) echo "	<option>$profile</option>\n";
			echo "</select>\n".
				"<input name=\"load\" type=\"submit\" value=\""; echo _('Load Profile');
			echo "\"></td><td width=\"30%\"></td><td width=\"20\"><a href=\"../help.php?HelpNumber=421\" target=\"lamhelp\">";
			echo _('Help')."</a></td>\n</tr>\n</table>\n</fieldset>\n";
			}
		// Show Fieldset with all host settings
		echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
		echo _("General properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td width=\"50%\">";
		echo _('Host name').'*';
		echo "</td>\n<td width=\"30%\">".
			'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $account_new->general_username . '">'.
			"</td><td width=\"20%\">".
			'<a href="../help.php?HelpNumber=410" target="lamhelp">'._('Help').'</a>'.
			"</td></tr>\n<tr><td>";
		echo _('UID number');
		echo "</td>\n<td>".
			'<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $account_new->general_uidNumber . '">'.
			"</td>\n<td>".
			'<a href="../help.php?HelpNumber=411" target="lamhelp">'._('Help').'</a>'.
			"</td></tr>\n<tr><td>";
		echo _('Primary group').'*';
		echo "</td>\n<td><select name=\"f_general_group\">";
		foreach ($groups as $group) {
			if ($account_new->general_group == $group) echo '<option selected>' . $group. '</option>';
			else echo '<option>' . $group. '</option>';
			 }
		echo '</select></td><td>'.
			'<a href="../help.php?HelpNumber=412" target="lamhelp">'._('Help').'</a>'.
			"</td></tr>\n<tr><td>";
		echo _('Gecos');
		echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $account_new->general_gecos . '">'.
			"</td>\n<td>".
			'<a href="../help.php?HelpNumber=413" target="lamhelp">'._('Help').'</a>'.
			'</td></tr><tr><td>';
		echo _('Password');
		echo '</td><td>';
		if (isset($account_old)) {
			echo '<input name="respass" type="submit" value="';
			echo _('Reset password'); echo '">';
			}
		echo "</td></tr>\n<tr><td>";
		echo _('Domain');
		if ($config_intern->is_samba3()) {
			// Get Domain-name from domainlist when using samba 3
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
			// Display a textfield for samba 2.2
			echo '</td>'."\n".'<td><input name="f_smb_domain" type="text" size="20" maxlength="80" value="' . $account_new->smb_domain . '">';
			}
		echo	'</td>'."\n".'<td><a href="../help.php?HelpNumber=460" target="lamhelp">'._('Help').'</a></td></tr>'."\n<tr><td>";
		// Display all allowed host suffixes
		echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
		foreach ($ldap_intern->search_units($config_intern->get_HostSuffix()) as $suffix) {
			if ($account_new->general_dn) {
				if ($account_new->general_dn == $suffix)
					echo '<option selected>' . $suffix. '</option>';
				else echo '<option>' . $suffix. '</option>';
				}
			else echo '<option>' . $suffix. '</option>';
			}
		echo '</select></td><td><a href="../help.php?HelpNumber=463" target="lamhelp">'._('Help').'</a>'.
			"</td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n";
		// Show fieldset where to save a new profile
		echo "<fieldset class=\"hostedit-dark\"><legend class=\"hostedit-bright\"><b>";
		echo _("Save profile");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td width=\"50%\">";
		echo '<input name="f_finish_safeProfile" type="text" size="30" maxlength="50">';
		echo '<input name="save" type="submit" value="';
		echo _('Save profile');
		echo '"></td><td width="30%"></td><td width="20%"><a href="../help.php?HelpNumber=457" target="lamhelp">'._('Help');
		echo "</a></td>\n</tr>\n</table>\n</fieldset>";
		// Show fieldset with modify, undo and back-button
		echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
		if ($account_old) echo _('Modify');
		 else echo _('Create');
		echo "</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td width=\"50%\">";
		// display undo-button when editiing a host
		if (isset($account_old)) {
			echo "<input name=\"next_reset\" type=\"submit\" value=\""; echo _('Undo changes');
			echo "\">\n";
			}
		echo "</td>\n<td width=\"30%\">";
		echo '<input name="create" type="submit" value="';
		if ($account_old) echo _('Modify Account');
		 else echo _('Create Account');
		echo "\">\n</td><td width=\"20%\">";
		echo "</td></tr></table></fieldset>\n";
		break;

	case 'finish':
		// Final Settings
		echo '<input name="select" type="hidden" value="finish">';
		echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>"._('Note')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo '<tr><td>';
		echo _('Host');
		echo ' '.$account_new->general_username.' ';
		if ($account_old) echo ' '._('has been modified').'.';
		 else echo ' '._('has been created').'.';
		echo '</td></tr>'."\n".'<tr><td>';
		if (!$account_old)
			{ echo '<input name="createagain" type="submit" value="'; echo _('Create another host'); echo '">'; }
		echo '</td>'."\n".'<td>'.
			'<input name="outputpdf" type="submit" value="'; echo _('Create PDF file'); echo '">'.
			'</td>'."\n".'<td>'.
			'<input name="backmain" type="submit" value="'; echo _('Back to host list'); echo '">'.
			'</td></tr></table></fieldset'."\n";

		break;
	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
