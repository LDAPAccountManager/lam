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

include_once('../lib/account.inc'); // File with all account-funtions
include_once('../lib/config.inc'); // File with configure-functions
include_once('../lib/profiles.inc'); // functions to load and save profiles
include_once('../lib/status.inc'); // Return error-message
include_once('../lib/pdf.inc'); // Return a pdf-file
include_once('../lib/ldap.inc'); // LDAP-functions
initvars($_GET['type'], $_GET['DN']); // Initialize all needed vars


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
			if (isset($_POST['f_general_username'])) $_SESSION['account']->general_username = $_POST['f_general_username'];
				else $_SESSION['account']->general_username = '';
			if (isset($_POST['f_general_surname'])) $_SESSION['account']->general_surname = $_POST['f_general_surname'];
				else $_SESSION['account']->general_surname = "";
			if (isset($_POST['f_general_givenname'])) $_SESSION['account']->general_givenname = $_POST['f_general_givenname'];
				else $_SESSION['account']->general_givenname = "";
			if (isset($_POST['f_general_uidNumber'])) $_SESSION['account']->general_uidNumber = $_POST['f_general_uidNumber'];
				else $_SESSION['account']->general_uidNumber = "";
			if (isset($_POST['f_general_group'])) $_SESSION['account']->general_group = $_POST['f_general_group'];
			if (isset($_POST['f_general_groupadd'])) $_SESSION['account']->general_groupadd = $_POST['f_general_groupadd'];
				else $_SESSION['account']->general_groupadd = array('');
			if (isset($_POST['f_general_homedir'])) $_SESSION['account']->general_homedir = $_POST['f_general_homedir'];
				else $_SESSION['account']->general_homedir = "";
			if (isset($_POST['f_general_shell'])) $_SESSION['account']->general_shell = $_POST['f_general_shell'];
			if (isset($_POST['f_general_gecos'])) $_SESSION['account']->general_gecos = $_POST['f_general_gecos'];
				else $_SESSION['account']->general_gecos = "";
			// Check if values are OK and set automatic values.  if not error-variable will be set
			if ($_SESSION['account_old']) list($values, $errors) = checkglobal($_SESSION['account'], $_SESSION['type2'], $_SESSION['account_old']); // account.inc
				else list($values, $errors) = checkglobal($_SESSION['account'], $_SESSION['type2']); // account.inc
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if ($val) $_SESSION['account']->$key = $val;
				}
			// Check which part Site should be displayed next
			if ($_POST['next'] && ($errors==''))
				switch ($_SESSION['type2']) {
					case 'user': $select_local = 'unix'; break;
					case 'group': if ($_SESSION['config']->samba3=='yes') $select_local = 'samba';
						else $select_local = 'quota'; break;
					case 'host': $select_local = 'samba'; break;
					}
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
		if (isset($_POST['f_unix_pwdwarn'])) $_SESSION['account']->unix_pwdwarn = $_POST['f_unix_pwdwarn'];
			else $_SESSION['account']->unix_pwdwarn = '';
		if (isset($_POST['f_unix_pwdallowlogin'])) $_SESSION['account']->unix_pwdallowlogin = $_POST['f_unix_pwdallowlogin'];
			else $_SESSION['account']->unix_pwdallowlogin = '';
		if (isset($_POST['f_unix_pwdmaxage'])) $_SESSION['account']->unix_pwdmaxage = $_POST['f_unix_pwdmaxage'];
			else $_SESSION['account']->unix_pwdmaxage = '';
		if (isset($_POST['f_unix_pwdminage'])) $_SESSION['account']->unix_pwdminage = $_POST['f_unix_pwdminage'];
			else $_SESSION['account']->unix_pwdminage = '';
		if (isset($_POST['f_unix_host'])) $_SESSION['account']->unix_host = $_POST['f_unix_host'];
			else $_SESSION['account']->unix_host = '';
		if (isset($_POST['f_unix_pwdexpire_mon'])) $_SESSION['account']->unix_pwdexpire = mktime(10, 0, 0, $_POST['f_unix_pwdexpire_mon'],
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
			else $errors = checkunix($_SESSION['account'], $_SESSION['type2']); // account.inc
		// Check which part Site should be displayd
		// Check which part Site should be displayed next
		if ($_POST['back']) $select_local = 'general';
		else if (($_POST['next']) && ($errors=='')) $select_local = 'samba';
			else $select_local = 'unix';
		break;
	case 'samba':
		// Write all general values into $_SESSION['account']
		if ($_POST['f_smb_password']) {
			// Encrypt password
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$_SESSION['account']->smb_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256, base64_decode($_COOKIE['Key']), $_POST['f_smb_password'],
			MCRYPT_MODE_ECB, base64_decode($_COOKIE['IV'])));
			}
		 else $_SESSION['account']->smb_password = "";
		$_SESSION['account']->smb_pwdcanchange = mktime($_POST['f_smb_pwdcanchange_s'], $_POST['f_smb_pwdcanchange_m'], $_POST['f_smb_pwdcanchange_h'],
			$_POST['f_smb_pwdcanchange_mon'], $_POST['f_smb_pwdcanchange_day'], $_POST['f_smb_pwdcanchange_yea']);
		$_SESSION['account']->smb_pwdmustchange = mktime($_POST['f_smb_pwdmustchange_s'], $_POST['f_smb_pwdmustchange_m'], $_POST['f_smb_pwdmustchange_h'],
			$_POST['f_smb_pwdmustchange_mon'], $_POST['f_smb_pwdmustchange_day'], $_POST['f_smb_pwdmustchange_yea']);
		if ($_POST['f_smb_password_no']) $_SESSION['account']->smb_password_no = true;
			else $_SESSION['account']->smb_password_no = false;
		if ($_POST['f_smb_useunixpwd']) $_SESSION['account']->smb_useunixpwd = $_POST['f_smb_useunixpwd'];
			else $_SESSION['account']->smb_useunixpwd = false;
		if (isset($_POST['f_smb_homedrive'])) $_SESSION['account']->smb_homedrive = $_POST['f_smb_homedrive'];
		if (isset($_POST['f_smb_scriptpath'])) $_SESSION['account']->smb_scriptPath = $_POST['f_smb_scriptpath'];
			else $_SESSION['account']->smb_scriptPath = '';
		if (isset($_POST['f_smb_smbuserworkstations'])) $_SESSION['account']->smb_smbuserworkstations = $_POST['f_smb_smbuserworkstations'];
			else $_SESSION['account']->smb_smbuserworkstations = "";
		if (isset($_POST['f_smb_smbhome'])) $_SESSION['account']->smb_smbhome = stripslashes($_POST['f_smb_smbhome']);
			else $_SESSION['account']->smb_smbhome = "";
		if (isset($_POST['f_smb_profilePath'])) $_SESSION['account']->smb_profilePath = stripslashes($_POST['f_smb_profilePath']);
			else $_SESSION['account']->smb_profilePath = "";
		if ($_POST['f_smb_flagsW']) $_SESSION['account']->smb_flagsW = true;
			else $_SESSION['account']->smb_flagsW = false;
		if ($_POST['f_smb_flagsD']) $_SESSION['account']->smb_flagsD = true;
			else $_SESSION['account']->smb_flagsD = false;
		if ($_POST['f_smb_flagsX']) $_SESSION['account']->smb_flagsX = true;
			else $_SESSION['account']->smb_flagsX = false;
		if (isset($_POST['f_smb_displayName'])) $_SESSION['account']->smb_displayName = $_POST['f_smb_displayName'];
			else $_SESSION['account']->smb_displayName = '';

		if ($_SESSION['config']->samba3 == 'yes') {
			$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
			for ($i=0; $i<sizeof($samba3domains); $i++)
				if ($_POST['f_smb_domain'] == $samba3domains[$i]->name) {
					$_SESSION['account']->smb_domain = $samba3domains[$i];
					}
			if ($_POST['f_smb_mapgroup'] == _('Domain Guests')) $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '514';
			if ($_POST['f_smb_mapgroup'] == _('Domain Users')) $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '513';
			if ($_POST['f_smb_mapgroup'] == _('Domain Admins')) $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '512';
			}
		else {
			if (isset($_POST['f_smb_domain'])) $_SESSION['account']->smb_domain = $_POST['f_smb_domain'];
				else $_SESSION['account']->smb_domain = false;
			}
		// Reset password if reset button was pressed. Button only vissible if account should be modified
		// Check if values are OK and set automatic values. if not error-variable will be set
		list($values, $errors) = checksamba($_SESSION['account'], $_SESSION['type2']); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['account']->$key = $val;
			}
		// Check which part Site should be displayed next
		if ($_POST['back'])
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'unix'; break;
				case 'group': $select_local = 'general'; break;
				}
		else if ($_POST['next'])
			if($errors=='')
				switch ($_SESSION['type2']) {
					case 'user': $select_local = 'quota'; break;
					case 'group': $select_local = 'quota'; break;
					case 'host': $select_local = 'final'; break;
					}
				else $select_local = 'samba';
		if ($_POST['respass']) {
			$_SESSION['account']->unix_password_no=true;
			$_SESSION['account']->smb_password_no=true;
			$select_local = 'samba';
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
			$i++;
			}
		// Check if values are OK and set automatic values. if not error-variable will be set
		list($values, $errors) = checkquota($_SESSION['account'], $_SESSION['type2']); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['account']->$key = $val;
			}
		// Check which part Site should be displayed next
		if ($_POST['back'])
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'samba'; break;
				case 'group': if ($_SESSION['config']->samba3=='yes') $select_local = 'samba';
					else $select_local = 'general'; break;
				}
		else if ($_POST['next'])
			if ($errors=='')
				switch ($_SESSION['type2']) {
					case 'user': $select_local = 'personal'; break;
					case 'group': $select_local = 'final'; break;
					}
				else $select_local = 'quota';
		break;
	case 'personal':
		// Write all general values into $_SESSION['account']
		if (isset($_POST['f_personal_title'])) $_SESSION['account']->personal_title = $_POST['f_personal_title'];
			else $_SESSION['account']->personal_title = "";
		if (isset($_POST['f_personal_mail'])) $_SESSION['account']->personal_mail = $_POST['f_personal_mail'];
			else $_SESSION['account']->personal_mail = "";
		if (isset($_POST['f_personal_telephoneNumber'])) $_SESSION['account']->personal_telephoneNumber = $_POST['f_personal_telephoneNumber'];
			else $_SESSION['account']->personal_telephoneNumber = "";
		if (isset($_POST['f_personal_mobileTelephoneNumber'])) $_SESSION['account']->personal_mobileTelephoneNumber = $_POST['f_personal_mobileTelephoneNumber'];
			else $_SESSION['account']->personal_mobileTelephoneNumber = "";
		if (isset($_POST['f_personal_facsimileTelephoneNumber'])) $_SESSION['account']->personal_facsimileTelephoneNumber = $_POST['f_personal_facsimileTelephoneNumber'];
			else $_SESSION['account']->personal_facsimileTelephoneNumber = "";
		if (isset($_POST['f_personal_street'])) $_SESSION['account']->personal_street = $_POST['f_personal_street'];
			else $_SESSION['account']->personal_street = "";
		if (isset($_POST['f_personal_postalCode'])) $_SESSION['account']->personal_postalCode = $_POST['f_personal_postalCode'];
			else $_SESSION['account']->personal_postalCode = "";
		if (isset($_POST['f_personal_postalAddress'])) $_SESSION['account']->personal_postalAddress = $_POST['f_personal_postalAddress'];
			else $_SESSION['account']->personal_postalAddress = "";
		if (isset($_POST['f_personal_employeeType'])) $_SESSION['account']->personal_employeeType = $_POST['f_personal_employeeType'];
			else $_SESSION['account']->personal_employeeType = "";
		// Check if values are OK and set automatic values. if not error-variable will be set
		list($values, $errors) = checkpersonal($_SESSION['account'], $_SESSION['type2']); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['account']->$key = $val;
			}
		// Check which part Site should be displayed next
		if ($_POST['back']) $select_local = 'quota';
		else if ($_POST['next'])
			if  ($errors=='') $select_local = 'final';
				else $select_local = 'personal';
		break;
	case 'final':
		// Write all general values into $_SESSION['account']
		if ($_POST['f_final_changegids']) $_SESSION['final_changegids'] = $_POST['f_final_changegids'] ;
		// Check which part Site should be displayed next
		if ($_POST['back'])
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'personal'; break;
				case 'group': $select_local = 'quota'; break;
				case 'host': $select_local = 'samba'; break;
				}
		break;
	case 'finish':
		// Check if pdf-file should be created
		if ($_POST['outputpdf']) {
			createpdf(array($_SESSION['account']));
			$select_local = 'pdf';
			}
		break;
	}



if ( $_POST['create'] ) { // Create-Button was pressed
	// Create or modify an account
	switch ($_SESSION['type2']) {
		case 'user':
			if ($_SESSION['account_old']) $result = modifyuser($_SESSION['account'],$_SESSION['account_old']);
			 else $result = createuser($_SESSION['account']); // account.inc
			if ( $result==1 || $result==3 ) $select_local = 'finish';
			break;
		case 'group':
			if ($_SESSION['account_old']) $result = modifygroup($_SESSION['account'],$_SESSION['account_old']);
			 else $result = creategroup($_SESSION['account']); // account.inc
			if ( $result==1 || $result==3 ) $select_local = 'finish';
			break;
		case 'host':
			if ($_SESSION['account_old']) $result = modifyhost($_SESSION['account'],$_SESSION['account_old']);
			 else $result = createhost($_SESSION['account']); // account.inc
			if ( $result==1 || $result==3 ) $select_local = 'finish';
			break;
		}
	}


// Set selected page to general if no page was defined. should only true if account.php wasn't called by itself
if (!$select_local) $select_local='general';
// Reset variables if recreate-button was pressed
if ($_POST['createagain']) {
	$select_local='general';
	$_SESSION['account']="";
	}
// Set selected page to backmain (Back to main listmenu)
if ($_POST['backmain']) {
	$select_local='backmain';
	}
// Set selected page to load (load profile)
if ($_POST['load']) $select_local='load';
// Set selected page to save (save profile)
if ($_POST['save']) $select_local='save';


if ($select_local != 'pdf') {
	// Write HTML-Header and part of Table
	echo $_SESSION['header'];
	echo "<html><head><title>";
	echo _("Create new Account");
	echo "</title>\n".
		"<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n".
		"<meta http-equiv=\"pragma\" content=\"no-cache\">\n".
		"<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
	}

switch ($select_local) {
	// backmain = back to lists
	// load = load profile
	// save = save profile
	case 'backmain':
		// unregister sessionvar and select which list should be shown
		if (session_is_registered("shelllist")) session_unregister("shelllist");
		if (session_is_registered("account")) session_unregister("account");
		if (session_is_registered("account_old")) session_unregister("account_old");
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				if (session_is_registered("type2")) session_unregister("type2");
				echo "<meta http-equiv=\"refresh\" content=\"0; URL=lists/listusers.php\">\n";
				break;
			case 'group' :

				if (session_is_registered("type2")) session_unregister("type2");
				echo "<meta http-equiv=\"refresh\" content=\"0; URL=lists/listgroups.php\">\n";
				break;
			case 'host' :
				if (session_is_registered("type2")) session_unregister("type2");
				echo "<meta http-equiv=\"refresh\" content=\"0; URL=lists/listhosts.php\">\n";
				break;
			}
		break;
	case 'load':
		// load profile
		switch ( $_SESSION['type2'] ) {
			case 'user':
				$_SESSION['account'] = loadUserProfile($_POST['f_general_selectprofile']);
				break;
			case 'host':
				$_SESSION['account'] = loadHostProfile($_POST['f_general_selectprofile']);
				break;
			case 'group':
				$_SESSION['account'] = loadGroupProfile($_POST['f_general_selectprofile']);
				break;
			}
		// select general page after group has been loaded
		$select_local='general';
		break;
	case 'save':
		// save profile
		switch ( $_SESSION['type2'] ) {
			case 'user':
				saveUserProfile($_SESSION['account'], $_POST['f_finish_safeProfile']);
			break;
			case 'host':
				saveHostProfile($_SESSION['account'], $_POST['f_finish_safeProfile']);
			break;
			case 'group':
				saveGroupProfile($_SESSION['account'], $_POST['f_finish_safeProfile']);
			break;
			}
		// select last page displayed before user is created
		$select_local='final';
		break;
	}


if ($select_local != 'pdf') {
	echo "</head><body>\n";
	echo "<form action=\"account.php\" method=\"post\">\n";
	echo "<table class=\"account\" width=\"100%\">\n";
	if (is_array($errors))
		for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);
	}

// print_r($_SESSION['account']);
//print_r($_POST);

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
		// Show page info
		echo '<tr><td><input name="select" type="hidden" value="general">';
		echo _('General Properties');
		echo "</td></tr>\n";
		switch ( $_SESSION['type2'] ) {
			case 'user':
				// load list of profiles
				$profilelist = getUserProfiles();
				// Create HTML-page
				echo '<tr><td>';
				echo _('Username').'*';
				echo "</td>\n<td>".
					'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">'.
					'</td><td>'.
					'<a href="help.php?HelpNumber=400" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('UID Number');
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
				echo _('Additional Groupmembership');
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
				echo _('Home Directory').'*';
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
				echo '</select></td><td><a href="help.php?HelpNumber=461" target="lamhelp">'._('Help').'</a>'.
					'</td></tr><tr><td>';
					echo _('Values with * are required');
					echo '</td></tr><tr><td><select name="f_general_selectprofile">';
				// loop through profiles
				foreach ($profilelist as $profile) echo '<option>' . $profile. '</option>';
				echo '</select>'.
					'<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">'.
					'</td>'."\n".'<td>';
				break;
			case 'group':
				// load list of profiles
				$profilelist = getGroupProfiles();
				// Create HTML-page
				echo '<tr><td>';
				echo _('Groupname').'*';
				echo '</td>'."\n".'<td>'.
					'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">'.
					'</td><td>'.
					'<a href="help.php?HelpNumber=407" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('GID Number');
				echo '</td>'."\n".'<td>'.
					'<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $_SESSION['account']->general_uidNumber . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=408" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Gecos');
				echo '</td>'."\n".'<td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=409" target="lamhelp">'._('Help').'</a>'.
					'</td></tr><tr><td>';
				echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
				foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_GroupSuffix()) as $suffix) {
					if ($_SESSION['account']->general_dn) {
						if ($_SESSION['account']->general_dn == $suffix)
							echo '<option selected>' . $suffix. '</option>';
						else echo '<option>' . $suffix. '</option>';
						}
					else echo '<option>' . $suffix. '</option>';
					}
				echo '</select></td><td><a href="help.php?HelpNumber=462" target="lamhelp">'._('Help').'</a>'.
					'</td></tr><tr><td>';
					echo _('Values with * are required');
					echo '</td></tr>'."\n".'<tr><td><select name="f_general_selectprofile" >';
				foreach ($profilelist as $profile) echo '<option>' . $profile. '</option>';
				echo '</select>'.
					'<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">'.
					'</td>'."\n".'<td>';
				break;
			case 'host':
				// load list of profiles
				$profilelist = getHostProfiles();
				// Create HTML-page
				echo '<tr><td>';
				echo _('Host name').'*';
				echo '</td>'."\n".'<td>'.
					'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">'.
					'</td><td>'.
					'<a href="help.php?HelpNumber=410" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('UID Number');
				echo '</td>'."\n".'<td>'.
					'<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $_SESSION['account']->general_uidNumber . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=411" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Primary group').'*';
				echo '</td>'."\n".'<td><select name="f_general_group">';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group. '</option>';
					else echo '<option>' . $group. '</option>';
					 }
				echo '</select></td><td>'.
					'<a href="help.php?HelpNumber=412" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Gecos');
				echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=413" target="lamhelp">'._('Help').'</a>'.
					'</td></tr><tr><td>';
				echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
				foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_HostSuffix()) as $suffix) {
					if ($_SESSION['account']->general_dn) {
						if ($_SESSION['account']->general_dn == $suffix)
							echo '<option selected>' . $suffix. '</option>';
						else echo '<option>' . $suffix. '</option>';
						}
					else echo '<option>' . $suffix. '</option>';
					}
				echo '</select></td><td><a href="help.php?HelpNumber=463" target="lamhelp">'._('Help').'</a>'.
					'</td></tr><tr><td>';
					echo _('Values with * are required');
					echo '</td></tr>'."\n".'<tr><td><select name="f_general_selectprofile">';
				foreach ($profilelist as $profile) echo '<option>' . $profile. '</option>';
				echo '</select>'.
					'<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">'.
					'</td>'."\n".'<td>';
				break;
			}
		echo '</td>'."\n".'<td>'.
			'<input name="next" type="submit" value="'; echo _('next'); echo '">'.
			'</td></tr>'."\n";
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
		$date = getdate ($_SESSION['account']->unix_pwdexpire);
		echo '<tr><td><input name="select" type="hidden" value="unix">';
		echo _('Unix Properties');
		echo '</td></tr>'."\n".'';
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				echo '<tr><td>';
				echo _('Password');
				echo '</td>'."\n".'<td>'.
					'<input name="f_unix_password" type="text" size="20" maxlength="20" value="' . $password . '">'.
					'</td>'."\n".'<td>'.
					'<input name="genpass" type="submit" value="';
				echo _('Generate Password'); echo '"></td></tr><tr><td>';
				echo _('Use no Password.');
				echo '</td>'."\n".'<td><input name="f_unix_password_no" type="checkbox"';
				if ($_SESSION['account']->unix_password_no) echo ' checked ';
				echo '></td>'."\n".'<td>'.
				'<a href="help.php?HelpNumber=426" target="lamhelp">'._('Help').'</a>'.
				'</td></tr>'."\n".'<tr><td>';
				echo _('Password Warn');
				echo '</td>'."\n".'<td><input name="f_unix_pwdwarn" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdwarn . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=414" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Password Expire');
				echo '</td>'."\n".'<td><input name="f_unix_pwdallowlogin" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdallowlogin . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=415" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Maximum Passwordage');
				echo '</td>'."\n".'<td><input name="f_unix_pwdmaxage" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->unix_pwdmaxage . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=416" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Minimum Passwordage');
				echo '</td>'."\n".'<td><input name="f_unix_pwdminage" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdminage . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=417" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Expire Date');
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
					'<a href="help.php?HelpNumber=466" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Values with * are required');
				echo '</td></tr>'."\n".'<tr><td>';
				break;
			}
		echo '<tr><td>'.
		'<input name="back" type="submit" value="'; echo _('back'); echo '">'.
		'</td>'."\n".'<td></td>'."\n".'<td>'.
		'<input name="next" type="submit" value="'; echo _('next'); echo '">'.
		'</td></tr>'."\n";
		break;
	case 'samba':
		// Samba Settings
		echo '<tr><td><input name="select" type="hidden" value="samba">'; echo _('Samba Properties'); echo '</td></tr>'."\n";
		// decrypt password
		// decrypt password
		if ($_SESSION['account']->smb_password != '') {
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($_SESSION['account']->smb_password), MCRYPT_MODE_ECB, $iv);
			$password = str_replace(chr(00), '', $password);
			}
		if ($_SESSION['config']->samba3 == 'yes') $samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
		switch ( $_SESSION['type2'] ) {
			case 'user':
				// Set Account is samba-workstation to false
				$canchangedate = getdate($_SESSION['account']->smb_pwdcanchange);
				$mustchangedate = getdate($_SESSION['account']->smb_pwdmustchange);
				$_SESSION['account']->smb_flagsW = 0;
				echo '<tr><td>';
				echo	'<input name="f_smb_pwdcanchange_h" type="hidden" value="'.$canchangedate['hours'].'">'.
					'<input name="f_smb_pwdcanchange_m" type="hidden" value="'.$canchangedate['minutes'].'">'.
					'<input name="f_smb_pwdcanchange_s" type="hidden" value="'.$canchangedate['seconds'].'">'.
					'<input name="f_smb_pwdmustchange_h" type="hidden" value="'.$mustchangedate['hours'].'">'.
					'<input name="f_smb_pwdmustchange_m" type="hidden" value="'.$mustchangedate['minutes'].'">'.
					'<input name="f_smb_pwdmustchange_s" type="hidden" value="'.$mustchangedate['seconds'].'">';
				echo _('Samba Password');
				echo '</td>'."\n".'<td><input name="f_smb_password" type="text" size="20" maxlength="20" value="' . $password . '">'.
					'</td>'."\n".'<td><input name="f_smb_useunixpwd" type="checkbox"';
				if ($_SESSION['account']->smb_useunixpwd) echo ' checked ';
				echo '>';
				echo _('Use Unix-Password');
				echo '</td></tr>'."\n".'<tr><td>';
				echo _('Use no Password.');
				echo '</td>'."\n".'<td><input name="f_smb_password_no" type="checkbox"';
				if ($_SESSION['account']->smb_password_no) echo ' checked ';
				echo '></td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=428" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Password doesn\'t expire.');
				echo '</td>'."\n".'<td><input name="f_smb_flagsX" type="checkbox"';
				if ($_SESSION['account']->smb_flagsX) echo ' checked ';
				echo '></td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=429" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('User can change Password');
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
				echo _('Script path');
				echo '</td>'."\n".'<td><input name="f_smb_scriptpath" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_scriptPath . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=434" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Profile path');
				echo '</td>'."\n".'<td><input name="f_smb_profilePath" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_profilePath . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=435" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Samba workstations');
				echo '</td>'."\n".'<td><input name="f_smb_smbuserworkstations" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_smbuserworkstations . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=436" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('smb home');
				echo '</td>'."\n".'<td><input name="f_smb_smbhome" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_smbhome . '">'.
					'</td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=437" target="lamhelp">'._('Help').'</a>'.
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
					}
				else {
					echo '</td>'."\n".'<td><input name="f_smb_domain" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_domain . '">';
					}
				echo	'</td>'."\n".'<td><a href="help.php?HelpNumber=438" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
				break;
			case 'group':
				echo '<tr><td>';
				echo _('Windows well known group');
				echo '</td>'."\n".'<td><select name="f_smb_mapgroup" >';
					if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '514' ) {
						echo '<option selected> ';
						echo _('Domain Guests');
						echo "</option>\n"; }
					 else {
						echo '<option> ';
						echo _('Domain Guests');
						echo "</option>\n";
						}
					if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '513' ) {
						echo '<option selected> ';
						echo _('Domain Users');
						echo "</option>\n"; }
					 else {
						echo '<option> ';
						echo _('Domain Users');
						echo "</option>\n";
						}
					if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '512' ) {
						echo '<option selected> ';
						echo _('Domain Admins');
						echo "</option>\n"; }
					 else {
						echo '<option> ';
						echo _('Domain Admins');
						echo "</option>\n";
						}
				echo	'</select></td>'."\n".'<td>'.
					'<a href="help.php?HelpNumber=464" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
					echo _('Windows Groupname');
					echo '</td><td>'.
					'<input name="f_smb_displayName" type="text" size="30" maxlength="80" value="' . $_SESSION['account']->smb_displayName . '">'.
					'</td><td>'.
					'<a href="help.php?HelpNumber=465" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo _('Domain');
				echo '</td><td><select name="f_smb_domain">';
				for ($i=0; $i<sizeof($samba3domains); $i++) {
					if ($_SESSION['account']->smb_domain->name) {
						if ($_SESSION['account']->smb_domain->name == $samba3domains[$i]->name)
							echo '<option selected>' . $samba3domains[$i]->name. '</option>';
						else echo '<option>' . $samba3domains[$i]->name. '</option>';
						}
					else echo '<option>' . $samba3domains[$i]->name. '</option>';
					}
				break;
			case 'host':
				// set smb_flgasW true because account is host
				$_SESSION['account']->smb_flagsW = 1;
				if ($_SESSION['account']->smb_password_no) echo '<input name="f_smb_password_no" type="hidden" value="1l">';
				echo '<input name="f_unix_password_no" type="hidden" value="';
				if ($_SESSION['account']->unix_password_no) echo 'checked';
				echo  '">';
				echo '<tr><td>';
				echo _('Password');
				echo '</td><td>';
				if ($_SESSION['account_old']) {
					echo '<input name="respass" type="submit" value="';
					echo _('Reset password'); echo '">';
					}
				echo '</td></tr>'."\n".'<tr><td>';
				echo _('Account is deactivated');
				echo '</td>'."\n".'<td><input name="f_smb_flagsD" type="checkbox"';
				if ($_SESSION['account']->smb_flagsD) echo ' checked ';
				echo '></td><td>'.
					'<a href="help.php?HelpNumber=432" target="lamhelp">'._('Help').'</a>'.
					'</td></tr>'."\n".'<tr><td>';
				echo '</td></tr>'."\n".'<tr><td>';
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
					}
				else {
					echo '</td>'."\n".'<td><input name="f_smb_domain" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_domain . '">';
					}
				echo	'</td>'."\n".'<td><a href="help.php?HelpNumber=460" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
				break;
			}
		echo '<tr><td><input name="back" type="submit" value="'; echo _('back');
		echo '"></td><td></td><td><input name="next" type="submit" value="';
		echo _('next'); echo '"></td></tr>'."\n";
		break;
	case 'quota':
		// Quota Settings
		echo '<tr><td><input name="select" type="hidden" value="quota">';
		echo _('Quota Properties');
		echo '</td></tr>'."\n".'<tr><td>'; echo _('Mointpoint'); echo '</td>'."\n".'<td>'; echo _('used blocks'); echo '</td>'."\n".'<td>';
		echo _('soft block limit'); echo '</td>'."\n".'<td>'; echo _('hard block limit'); echo '</td>'."\n".'<td>'; echo _('grace block period');
		echo '</td>'."\n".'<td>'; echo _('used inodes'); echo '</td>'."\n".'<td>'; echo _('soft inode limit'); echo '</td>'."\n".'<td>';
		echo _('hard inode limit'); echo '</td>'."\n".'<td>'; echo _('grace inode period'); echo '</td></tr>'."\n";
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
		echo '<tr><td>'.
		'<input name="back" type="submit" value="'; echo _('back'); echo '">'.
		'</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>'.
		'<input name="next" type="submit" value="'; echo _('next'); echo '">'.
		'</td></tr>'."\n";
		break;
	case 'personal':
		// Personal Settings
		echo '<tr><td><input name="select" type="hidden" value="personal">';
		echo _('Personal Properties');
		echo '</td></tr>'."\n".'<tr><td>';
		echo _('Title');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_title" type="text" size="10" maxlength="10" value="' . $_SESSION['account']->personal_title . '"> ';
		echo $_SESSION['account']->general_surname . ' ' . $_SESSION['account']->general_givenname . '</td><td>'.
			'<a href="help.php?HelpNumber=448" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Employee Type');
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
		echo _('Telephone Number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_telephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_telephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=453" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Mobile Phonenumber');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mobileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_mobileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=454" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Facsimile Number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_facsimileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_facsimileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=455" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('eMail Address');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mail" type="text" size="30" maxlength="80" value="' . $_SESSION['account']->personal_mail . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=456" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>'.
			'<input name="back" type="submit" value="'; echo _('back'); echo '">'.
			'</td><td></td>'."\n".'<td>'.
			'<input name="next" type="submit" value="'; echo _('next'); echo '">'.
			'</td></tr>'."\n";
		break;
	case 'final':
		// Final Settings
		echo '<tr><td><input name="select" type="hidden" value="final">';
		if ($_SESSION['account_old']) echo _('Modify');
		 else echo _('Create');
		echo '</td></tr>'."\n";
		switch ( $_SESSION['type2'] ) {
			case 'user' :
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
				if (!in_array('posixAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				if (!in_array('shadowAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				if (!in_array('inetOrgPerson', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				if ($_SESSION['config']->samba3 == 'yes') {
					if (!in_array('sambaSamAccount', $_SESSION['account_old']->general_objectClass)) {
						echo '<tr>';
						StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
						echo "</tr>\n";
						}}
					else
					if (!in_array('sambaAccount', $_SESSION['account_old']->general_objectClass)) {
						echo '<tr>';
						StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
						echo "</tr>\n";
						}
				break;
			case 'group' :
				if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
					echo '<tr>';
					StatusMessage ('INFO', _('GID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
					'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chgrp ' . $_SESSION['account']->general_uidNumber . ' {} \;');
					echo '</tr>'."\n";
					echo '<tr><td>';
					echo '<input name="f_final_changegids" type="checkbox"';
						if ($_SESSION['final_changegids']) echo ' checked ';
					echo ' >';
					echo _('Change GID-Number of all users in group to new value');
					echo '</td></tr>'."\n";
					}
				if (($_SESSION['config']->samba3 == 'yes') && (!in_array('sambaGroupMapping', $_SESSION['account_old']->general_objectClass))) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				if (!in_array('posixGroup', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				break;
			case 'host':
				if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
					echo '<tr>';
					StatusMessage ('INFO', _('UID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
					'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chown ' . $_SESSION['account']->general_uidNumber . ' {} \;');
					echo '</tr>'."\n";
					}
				if (!in_array('posixAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				if (!in_array('shadowAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				if (!in_array('account', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
				if ($_SESSION['config']->samba3 == 'yes') {
					if (!in_array('sambaSamAccount', $_SESSION['account_old']->general_objectClass)) {
						echo '<tr>';
						StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
						echo "</tr>\n";
						}}
					else
					if (!in_array('sambaAccount', $_SESSION['account_old']->general_objectClass)) {
						echo '<tr>';
						StatusMessage('WARN', _('ObjectClass doesn\'t fit.'), _('Have to recreate entry.'));
						echo "</tr>\n";
						}
				break;
			}
		echo '<tr><td>'.
			'<input name="back" type="submit" value="'; echo _('back'); echo '">'.
			'</td>'."\n".'<td>'.
			'</td>'."\n".'<td><input name="f_finish_safeProfile" type="text" size="30" maxlength="50">'.
			'<input name="save" type="submit" value="';
		echo _('Save Profile');
		echo '"><a href="help.php?HelpNumber=457" target="lamhelp">'._('Help').'</a>'.
			'</td>'."\n".'<td>'.
			'<input name="create" type="submit" value="';
		if ($_SESSION['account_old']) echo _('Modify Account');
		 else echo _('Create Account');
		echo '">'.
		'</td></tr>'."\n";
		break;
	case 'finish':
		// Final Settings
		echo '<tr><td><input name="select" type="hidden" value="finish">';
		echo _('Success');
		echo '</td></tr>'."\n";
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				echo '<tr><td>';
				echo _('User ');
				echo $_SESSION['account']->general_username;
				if ($_SESSION['account_old']) echo ' '._('has been modified').'.';
				 else echo ' '._('has been created').'.';
				if (!$_SESSION['account_old'])
					{ echo '<input name="createagain" type="submit" value="'; echo _('Create another user'); echo '">'; }
				echo '</td>'."\n".'<td>'.
					'<input name="outputpdf" type="submit" value="'; echo _('Create PDF file'); echo '">'.
					'</td>'."\n".'<td>'.
					'<input name="backmain" type="submit" value="'; echo _('Back to user list'); echo '">'.
					'</td></tr>'."\n";
				break;
			case 'group' :
				echo '<tr><td>';
				echo _('Group ');
				echo $_SESSION['account']->general_username;
				if ($_SESSION['account_old']) echo ' '._('has been modified').'.';
				 else echo ' '._('has been created').'.';
				echo '</td></tr>'."\n".'<tr><td>';
				if (!$_SESSION['account_old'])
					{ echo' <input name="createagain" type="submit" value="'; echo _('Create another group'); echo '">'; }
				echo '</td><td></td><td>'.
					'<input name="backmain" type="submit" value="'; echo _('Back to group list'); echo '">'.
					'</td></tr>'."\n";
				break;
			case 'host' :
				echo '<tr><td>';
				echo _('Host');
				echo ' '.$_SESSION['account']->general_username.' ';
				if ($_SESSION['account_old']) echo ' '._('has been modified').'.';
				 else echo ' '._('has been created').'.';
				echo '</td></tr>'."\n".'<tr><td>';
				if (!$_SESSION['account_old'])
					{ echo '<input name="createagain" type="submit" value="'; echo _('Create another host'); echo '">'; }
				echo '</td><td>'."\n".'</td><td>'.
					'<input name="backmain" type="submit" value="'; echo _('Back to host list'); echo '">'.
					'</td></tr>'."\n";
				break;
			}
		break;
	}

// Print end of HTML-Page
if ($select_local != 'pdf') echo '</table></form></body></html>';
?>
