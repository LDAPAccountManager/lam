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

include_once('../lib/account.inc'); // File with custom functions
include_once('../lib/config.inc'); // File with configure-functions
include_once('../lib/ldap.inc'); // LDAP-functions
include_once('../lib/profiles.inc'); // functions to load and save profiles
include_once('../lib/status.inc'); // Return error-message
include_once('../lib/pdf.inc'); // Return a pdf-file


$error = "0";
initvars($_GET['type'], $_GET['DN']); // Initialize alle needed vars

switch ($_POST['select']) {
	case 'general':
		if (!$_POST['load']) { // No Profile was loaded
			// Write alle values in temporary object
			if ($_POST['f_general_username']) $_SESSION['account']->general_username = $_POST['f_general_username'];
				else $_SESSION['account']->general_username = $_POST['f_general_username'];
			if ($_POST['f_general_surname']) $_SESSION['account']->general_surname = $_POST['f_general_surname'];
				else $_SESSION['account']->general_surname = "";
			if ($_POST['f_general_givenname']) $_SESSION['account']->general_givenname = $_POST['f_general_givenname'];
				else $_SESSION['account']->general_givenname = "";
			if ($_POST['f_general_uidNumber']) $_SESSION['account']->general_uidNumber = $_POST['f_general_uidNumber'];
				else $_SESSION['account']->general_uidNumber = "";
			if ($_POST['f_general_group']) $_SESSION['account']->general_group = $_POST['f_general_group'];
				if ($_POST['f_general_groupadd']) $_SESSION['account']->general_groupadd = $_POST['f_general_groupadd'];
			if ($_POST['f_general_homedir']) $_SESSION['account']->general_homedir = $_POST['f_general_homedir'];
				else $_SESSION['account']->general_homedir = "";
			if ($_POST['f_general_shell']) $_SESSION['account']->general_shell = $_POST['f_general_shell'];
			if ($_POST['f_general_gecos']) $_SESSION['account']->general_gecos = $_POST['f_general_gecos'];
				else $_SESSION['account']->general_gecos = "";
			// Check Values
			if ($_SESSION['account_old']) $values = checkglobal($_SESSION['account'], $_SESSION['type2'], $_SESSION['account_old']); // account.inc
				else $values = checkglobal($_SESSION['account'], $_SESSION['type2']); // account.inc
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if ($val) $_SESSION['account']->$key = $val;
				}
				else $error = $values;
			// Check which part Site should be displayd
			if ($_POST['next'] && ($error=="0"))
				switch ($_SESSION['type2']) {
					case 'user': $select_local = 'unix'; break;
					case 'group': $select_local = 'quota'; break;
					case 'host': $select_local = 'unix'; break;
					}
			}
		break;
	case 'unix':
		// Write alle values in temporary object
		if ($_POST['f_unix_password']) $_SESSION['account']->unix_password = $_POST['f_unix_password'];
			else $_SESSION['account']->unix_password = '';
		if ($_POST['genpass']) { $_SESSION['account']->unix_password = genpasswd(); }
		if ($_POST['f_unix_password_no']) $_SESSION['account']->unix_password_no = true;
			else $_SESSION['account']->unix_password_no = false;
		if ($_POST['f_unix_pwdwarn']) $_SESSION['account']->unix_pwdwarn = $_POST['f_unix_pwdwarn'];
			else $_SESSION['account']->unix_pwdwarn = '';
		if ($_POST['f_unix_pwdallowlogin']) $_SESSION['account']->unix_pwdallowlogin = $_POST['f_unix_pwdallowlogin'];
			else $_SESSION['account']->unix_pwdallowlogin = '';
		if ($_POST['f_unix_pwdmaxage']) $_SESSION['account']->unix_pwdmaxage = $_POST['f_unix_pwdmaxage'];
			else $_SESSION['account']->unix_pwdmaxage = '';
		if ($_POST['f_unix_pwdminage']) $_SESSION['account']->unix_pwdminage = $_POST['f_unix_pwdminage'];
			else $_SESSION['account']->unix_pwdminage = '';
		if ($_POST['f_unix_pwdexpire_day']) $_SESSION['account']->unix_pwdexpire_day = $_POST['f_unix_pwdexpire_day'];
		if ($_POST['f_unix_pwdexpire_mon']) $_SESSION['account']->unix_pwdexpire_mon = $_POST['f_unix_pwdexpire_mon'];
		if ($_POST['f_unix_pwdexpire_yea']) $_SESSION['account']->unix_pwdexpire_yea = $_POST['f_unix_pwdexpire_yea'];
		if ($_POST['f_unix_deactivated']) $_SESSION['account']->unix_deactivated = $_POST['f_unix_deactivated'];
			else $_SESSION['account']->unix_deactivated = false;
		// Check Values
		// Check which part Site should be displayd
		if ($_POST['genpass']) $select_local = 'unix';
			else $error = checkunix($_SESSION['account'], $_SESSION['type2']); // account.inc
		if ($_POST['respass']) {
			$_SESSION['account']->unix_password_no=true;
			$_SESSION['account']->smb_password_no=true;
			}
		if (($_POST['next']) && ($error=="0")) $select_local = 'samba';
			else $select_local = 'unix';
		if ($_POST['back']) $select_local = 'general';
		break;
	case 'samba':
		// Write alle values in temporary object
		if ($_POST['f_smb_password']) $_SESSION['account']->smb_password = $_POST['f_smb_password'];
			else $_SESSION['account']->smb_password = "";
		if ($_POST['f_smb_password_no']) $_SESSION['account']->smb_password_no = true;
			else $_SESSION['account']->smb_password_no = false;
		if ($_POST['f_smb_useunixpwd']) $_SESSION['account']->smb_useunixpwd = $_POST['f_smb_useunixpwd'];
			else $_SESSION['account']->smb_useunixpwd = false;
		if ($_POST['f_smb_pwdcanchange']) $_SESSION['account']->smb_pwdcanchange = $_POST['f_smb_pwdcanchange'];
			else $_SESSION['account']->smb_pwdcanchange = false;
		if ($_POST['f_smb_pwdmustchange']) $_SESSION['account']->smb_pwdmustchange = $_POST['f_smb_pwdmustchange'];
			else $_SESSION['account']->smb_pwdmustchange = false;
		if ($_POST['f_smb_homedrive']) $_SESSION['account']->smb_homedrive = $_POST['f_smb_homedrive'];
		if ($_POST['f_smb_scriptpath']) $_SESSION['account']->smb_scriptPath = $_POST['f_smb_scriptpath'];
			else $_SESSION['account']->smb_scriptPath = '';
		if ($_POST['f_smb_smbuserworkstations']) $_SESSION['account']->smb_smbuserworkstations = $_POST['f_smb_smbuserworkstations'];
			else $_SESSION['account']->smb_smbuserworkstations = "";
		if ($_POST['f_smb_smbhome']) $_SESSION['account']->smb_smbhome = stripslashes($_POST['f_smb_smbhome']);
			else $_SESSION['account']->smb_smbhome = "";
		if ($_POST['f_smb_profilePath']) $_SESSION['account']->smb_profilePath = stripslashes($_POST['f_smb_profilePath']);
			else $_SESSION['account']->smb_profilePath = "";
		if ($_POST['f_smb_domain']) $_SESSION['account']->smb_domain = $_POST['f_smb_domain'];
			else $_SESSION['account']->smb_domain = false;
		if ($_POST['f_smb_flagsW']) $_SESSION['account']->smb_flagsW = $_POST['f_smb_flagsW'];
			else $_SESSION['account']->smb_flagsW = false;
		if ($_POST['f_smb_flagsD']) $_SESSION['account']->smb_flagsD = $_POST['f_smb_flagsD'];
			else $_SESSION['account']->smb_flagsD = false;
		if ($_POST['f_smb_flagsX']) $_SESSION['account']->smb_flagsX = $_POST['f_smb_flagsX'];
			else $_SESSION['account']->smb_flagsX = false;
		// Check Values
		$values = checksamba($_SESSION['account'], $_SESSION['type2']); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['account']->$key = $val;
			}
			else $error = $values;
		// Check which part Site should be displayd
		if ($_POST['back']) $select_local = 'unix';
		if ($_POST['next']) {
			if ($error=="0")
				switch ($_SESSION['type2']) {
					case 'user': $select_local = 'quota'; break;
					case 'host': $select_local = 'final'; break;
					}
				else $select_local = 'samba';
			}
		break;
	case 'quota':
		$i=0;
		while ($_SESSION['account']->quota[$i][0]) {
			$_SESSION['account']->quota[$i][2] = $_POST['f_quota_'.$i.'_2'];
			$_SESSION['account']->quota[$i][3] = $_POST['f_quota_'.$i.'_3'];
			$_SESSION['account']->quota[$i][6] = $_POST['f_quota_'.$i.'_6'];
			$_SESSION['account']->quota[$i][7] = $_POST['f_quota_'.$i.'_7'];
			$i++;
			}
		$values = checkquota($_SESSION['account'], $_SESSION['type2']); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['account']->$key = $val;
			}
			else $error = $values;
		// Check which part Site should be displayd
		if ($_POST['back'])
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'samba'; break;
				case 'group': $select_local = 'general'; break;
				}
		if ($_POST['next']) {
			if ($error=="0")
				switch ($_SESSION['type2']) {
					case 'user': $select_local = 'personal'; break;
					case 'group': $select_local = 'final'; break;
					}
				else $select_local = 'quota';
			}
		break;
	case 'personal':
		if ($_POST['f_personal_title']) $_SESSION['account']->personal_title = $_POST['f_personal_title'];
			else $_SESSION['account']->personal_title = "";
		if ($_POST['f_personal_mail']) $_SESSION['account']->personal_mail = $_POST['f_personal_mail'];
			else $_SESSION['account']->personal_mail = "";
		if ($_POST['f_personal_telephoneNumber']) $_SESSION['account']->personal_telephoneNumber = $_POST['f_personal_telephoneNumber'];
			else $_SESSION['account']->personal_telephoneNumber = "";
		if ($_POST['f_personal_mobileTelephoneNumber']) $_SESSION['account']->personal_mobileTelephoneNumber = $_POST['f_personal_mobileTelephoneNumber'];
			else $_SESSION['account']->personal_mobileTelephoneNumber = "";
		if ($_POST['f_personal_facsimileTelephoneNumber']) $_SESSION['account']->personal_facsimileTelephoneNumber = $_POST['f_personal_facsimileTelephoneNumber'];
			else $_SESSION['account']->personal_facsimileTelephoneNumber = "";
		if ($_POST['f_personal_street']) $_SESSION['account']->personal_street = $_POST['f_personal_street'];
			else $_SESSION['account']->personal_street = "";
		if ($_POST['f_personal_postalCode']) $_SESSION['account']->personal_postalCode = $_POST['f_personal_postalCode'];
			else $_SESSION['account']->personal_postalCode = "";
		if ($_POST['f_personal_postalAddress']) $_SESSION['account']->personal_postalAddress = $_POST['f_personal_postalAddress'];
			else $_SESSION['account']->personal_postalAddress = "";
		if ($_POST['f_personal_employeeType']) $_SESSION['account']->personal_employeeType = $_POST['f_personal_employeeType'];
			else $_SESSION['account']->personal_employeeType = "";
		// Check which part Site should be displayd
		$values = checkpersonal($_SESSION['account'], $_SESSION['type2']); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['account']->$key = $val;
			}
			else $error = $values;
		if ($_POST['back'] && ($error=="0")) $select_local = 'quota';
		if ($_POST['next'] && ($error=="0")) $select_local = 'final';
		break;
	case 'final':
		if ($_POST['f_final_changegids']) $_SESSION['final_changegids'] = $_POST['f_final_changegids'] ;
		if ($_POST['back'] && ($error=="0"))
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'personal'; break;
				case 'group': $select_local = 'quota'; break;
				case 'host': $select_local = 'samba'; break;
				}
		break;
	case 'finish':
		if ($_POST['outputpdf']) createpdf($_SESSION['account']);
		break;
	}



if ( $_POST['create'] ) { // Create-Button was pressed
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


if (!$select_local) $select_local='general';
if ($_POST['createagain']) {
	$select_local='general';
	$_SESSION['account']="";
	}
if ($_POST['backmain']) {
	$select_local='backmain';
	}

if ($_POST['load']) $select_local='load';
if ($_POST['save']) $select_local='save';


// Write HTML-Header and part of Table
echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
       "http://www.w3.org/TR/html4/loose.dtd">';
echo '<html><head><title>';
echo _('Create new Account');
echo '</title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
	<meta http-equiv="pragma" content="no-cache">
	<meta http-equiv="cache-control" content="no-cache">
	<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">';

switch ($select_local) {
	case 'backmain':
		if (session_is_registered("shelllist")) session_unregister("shelllist");
		if (session_is_registered("account")) session_unregister("account");
		if (session_is_registered("account_old")) session_unregister("account_old");
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				if (session_is_registered("type2")) session_unregister("type2");
				echo '<meta http-equiv="refresh" content="0; URL=lists/listusers.php">';
				break;
			case 'group' :
				if (session_is_registered("type2")) session_unregister("type2");
				echo '<meta http-equiv="refresh" content="0; URL=lists/listgroups.php">';
				break;
			case 'host' :
				if (session_is_registered("type2")) session_unregister("type2");
				echo '<meta http-equiv="refresh" content="0; URL=lists/listhosts.php">';
				break;
			}
		break;
	case 'load':
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
		$select_local='general';
		break;
	case 'save':
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
		$select_local='final';
		break;
	}

	echo '</head><body>
	<form action="account.php" method="post">';
	if ($error != "0") StatusMessage('ERROR', _('Invalid Value!'), $error);
	echo '<table rules="all" class="account" width="100%">';


switch ($select_local) {
	case 'general':
		// General Account Settings
		$groups = findgroups();
		echo '<tr><td><input name="select" type="hidden" value="general">';
		echo _('General Properties');
		echo '</td></tr>';
		switch ( $_SESSION['type2'] ) {
			case 'user':
				$profilelist = getUserProfiles();
				echo '<tr><td>';
				echo _('Username');
				echo '</td><td>
					<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">
					</td></tr><tr><td>';
				echo _('UID Number');
				echo '</td><td>
					<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $_SESSION['account']->general_uidNumber . '">
					</td><td>';
				echo _('If empty UID Number will be generated automaticly.');
				echo '</td></tr><tr><td>';
				echo _('Surname');
				echo '</td><td>
					<input name="f_general_surname" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_surname . '">
					</td></tr><tr><td>';
				echo _('Given name');
				echo '</td><td>
					<input name="f_general_givenname" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_givenname . '">
					</td></tr><tr><td>';
				echo _('Primary Group');
				echo '</td><td><select name="f_general_group">';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group;
					else echo '<option>' . $group;
					 }
				echo '</select></td></tr><tr><td>';
				echo _('Additional Groupmembership');
				echo '</td><td><select name="f_general_groupadd[]" size="3" multiple>';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_groupadd) {
						if (in_array($group, $_SESSION['account']->general_groupadd)) echo '<option selected>'.$group;
						else echo '<option>'.$group;
						}
					else echo '<option>'.$group;
					}
				echo	'</select></td><td>';
				echo _('Can be left empty. Hold the CTRL-key to select multiple groups.');
				echo '</td></tr><tr><td>';
				echo _('Home Directory');
				echo '</td><td><input name="f_general_homedir" type="text" size="30" value="' . $_SESSION['account']->general_homedir . '">
					</td><td>';
				echo _('$user and $group are replaced with username or primary groupname.');
				echo '</td></tr><tr><td>';
				echo _('Gecos');
				echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">
					</td><td>';
				echo _('User descriptopn. If left empty sur- and givename will be used.');
				echo '</td></tr><tr><td>';
				echo _('Login Shell');
				echo '</td><td><select name="f_general_shell" >';
					foreach ($_SESSION['shelllist'] as $shell)
						if ($_SESSION['account']->general_shell==trim($shell)) echo '<option selected>'.$shell;
							else echo '<option>'.$shell;
				echo	'</select></td><td>';
				echo _('To disable login use /bin/false.');
				echo '</td></tr><tr><td><select name="f_general_selectprofile">';
				foreach ($profilelist as $profile) echo '<option>' . $profile;
				echo '</select>
				<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">
				</td><td>';
				break;
			case 'group':
				$profilelist = getGroupProfiles();
				echo '<tr><td>';
				echo _('Groupname');
				echo '</td><td>
					<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">
					</td></tr><tr><td>';
				echo _('GID Number');
				echo '</td><td>
					<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $_SESSION['account']->general_uidNumber . '">
					</td><td>';
				echo _('If empty GID Number will be generated automaticly.');
				echo '</td></tr><tr><td>';
				echo _('Gecos');
				echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">
					</td><td>';
				echo _('User descriptopn. If left empty groupname will be used.');
				echo '</td></tr><tr><td><select name="f_general_selectprofile" >';
				foreach ($profilelist as $profile) echo '<option>' . $profile;
				echo '</select>
				<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">
				</td><td>';
				break;
			case 'host':
				$profilelist = getHostProfiles();
				echo '<tr><td>';
				echo _('Hostname');
				echo '</td><td>
					<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">
					</td></tr><tr><td>';
				echo _('UID Number');
				echo '</td><td>
					<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $_SESSION['account']->general_uidNumber . '">
					</td><td>';
				echo _('If empty UID Number will be generated automaticly.');
				echo '</td></tr><tr><td>';
				echo _('Primary Group');
				echo '</td><td><select name="f_general_group">';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group;
					else echo '<option>' . $group;
					 }
				echo '</select></td></tr><tr><td>';
				echo _('Additional Groupmembership');
				echo '</td><td><select name="f_general_groupadd[]" size="3" multiple>';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_groupadd) {
						if (in_array($group, $_SESSION['account']->general_groupadd)) echo '<option selected>'.$group;
						else echo '<option>'.$group;
						}
					else echo '<option>'.$group;
					}
				echo	'</select></td><td>';
				echo _('Can be left empty. Hold the CTRL-key to select multiple groups.');
				echo '</td></tr><tr><td>';
				echo _('Gecos');
				echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">
					</td><td>';
				echo _('Host descriptopn. If left empty hostname will be used.');
				echo '</td></tr><tr><td><select name="f_general_selectprofile">';
				foreach ($profilelist as $profile) echo '<option>' . $profile;
				echo '</select>
				<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">
				</td><td>';
				break;
			}
		echo '</td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'unix':
		// Unix Password Settings
		echo '<tr><td><input name="select" type="hidden" value="unix">';
		echo _('Unix Properties');
		echo '</td></tr>';
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				echo '<tr><td>';
				echo _('Password');
				echo '</td><td>
					<input name="f_unix_password" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->unix_password . '">
					</td><td>
					<input name="genpass" type="submit" value="';
				echo _('Generate Password'); echo '">
					</td></tr><tr><td>';
				echo _('Use no Password.');
				echo '</td><td><input name="f_unix_password_no" type="checkbox"';
				if ($_SESSION['account']->unix_password_no) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('Password Warn');
				echo '</td><td><input name="f_unix_pwdwarn" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdwarn . '">
					</td><td>';
				echo _('Number of days a user will be warned when password will expire. Value must be 0<.');
				echo	'</td></tr><tr><td>';
				echo _('Password Expire');
				echo '</td><td><input name="f_unix_pwdallowlogin" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdallowlogin . '">
					</td><td>';
				echo _('Number of days a user can login even his password has expired. -1=always');
				echo '</td></tr><tr><td>';
				echo _('Maximum Passwordage');
				echo '</td><td><input name="f_unix_pwdmaxage" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->unix_pwdmaxage . '">
					</td><td>';
				echo _('Number of days after a user has to change his password again Value must be 0<.');
				echo '</td></tr><tr><td>';
				echo _('Minimum Passwordage');
				echo '</td><td><input name="f_unix_pwdminage" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdminage . '">
					</td><td>';
				echo _('Number of days a user has to wait until he\'s allowed to change his password again. Value must be 0<.');
				echo '</td></tr><tr><td>';
				echo _('Expire Date');
				echo '</td><td><select name="f_unix_pwdexpire_day">';
				for ( $i=1; $i<=31; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_day==$i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select><select name="f_unix_pwdexpire_mon">';
				for ( $i=1; $i<=12; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_mon == $i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select><select name="f_unix_pwdexpire_yea">';
				for ( $i=2003; $i<=2030; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_yea==$i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select></td><td>';
				echo _('Account expire date.');
				echo '</td></tr><tr><td>';
				echo _('Account deactivated');
				echo '</td><td><input name="f_unix_deactivated" type="checkbox"';
				if ($_SESSION['account']->unix_deactivated) echo ' checked ';
				echo '></td></tr>';
				break;
			case 'host' :
				echo '<input name="f_unix_password_no" type="hidden" value="';
				if ($_SESSION['account']->unix_password_no) echo 'checked';
				echo  '">';
				echo '<tr><td>';
				echo _('Password');
				echo '</td><td></td><td>';
				if ($_SESSION['account_old']) {
					echo '<input name="respass" type="submit" value="';
					echo _('Reset Password'); echo '">';
					}
				echo '</td></tr><tr><td>';
				echo _('Password Warn');
				echo '</td><td><input name="f_unix_pwdwarn" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdwarn . '">
					</td><td>';
				echo _('Number of host a user will be warned when password will expire. Value must be 0<.');
				echo	'</td></tr><tr><td>';
				echo _('Password Expire');
				echo '</td><td><input name="f_unix_pwdallowlogin" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdallowlogin . '">
					</td><td>';
				echo _('Number of days a host can login even his password has expired. -1=always');
				echo '</td></tr><tr><td>';
				echo _('Maximum Passwordage');
				echo '</td><td><input name="f_unix_pwdmaxage" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->unix_pwdmaxage . '">
					</td><td>';
				echo _('Number of days after a host has to change his password again Value must be 0< and should be higher as the value on client-side.');
				echo '</td></tr><tr><td>';
				echo _('Minimum Passwordage');
				echo '</td><td><input name="f_unix_pwdminage" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdminage . '">
					</td><td>';
				echo _('Number of days a user has to wait until he\'s allowed to change his password again. Value must be 0<.');
				echo '</td></tr><tr><td>';
				echo _('Expire Date');
				echo '</td><td><select name="f_unix_pwdexpire_day">';
				for ( $i=1; $i<=31; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_day==$i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select><select name="f_unix_pwdexpire_mon">';
				for ( $i=1; $i<=12; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_mon == $i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select><select name="f_unix_pwdexpire_yea">';
				for ( $i=2003; $i<=2030; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_yea==$i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select></td><td>';
				echo _('Account expire date.');
				echo '</td></tr><tr><td>';
				echo _('Account deactivated');
				echo '</td><td><input name="f_unix_deactivated" type="checkbox"';
				if ($_SESSION['account']->unix_deactivated) echo ' checked ';
				echo '></td></tr>';
				break;
			}
		echo '<tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td></td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'samba':
		// Samba Settings
		echo '<tr><td><input name="select" type="hidden" value="samba">'; echo _('Samba Properties'); echo '</td></tr>';
		switch ( $_SESSION['type2'] ) {
			case 'user':
				echo '<tr><td>';
				echo _('Samba Password');
				echo '</td><td><input name="f_smb_password" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_password . '">
				</td><td><input name="f_smb_useunixpwd" type="checkbox"';
				if ($_SESSION['account']->smb_useunixpwd) echo ' checked ';
				echo '>';
				echo _('Use Unix-Password');
				echo '</td></tr><tr><td>';
				echo _('Use no Password.');
				echo '</td><td><input name="f_smb_password_no" type="checkbox"';
				if ($_SESSION['account']->smb_password_no) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('Password doesn\'t expire.');
				echo '</td><td><input name="f_smb_flagsX" type="checkbox"';
				if ($_SESSION['account']->smb_flagsX) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('User can change Password');
				echo '</td><td><input name="f_smb_pwdcanchange" type="checkbox"';
				if ($_SESSION['account']->smb_pwdcanchange) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('User must change Password');
				echo '</td><td><input name="f_smb_pwdmustchange" type="checkbox"';
				if ($_SESSION['account']->smb_pwdmustchange) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('Accout is deactivated');
				echo '</td><td><input name="f_smb_flagsD" type="checkbox"';
				if ($_SESSION['account']->smb_flagsD) echo ' checked ';
				echo '></td></tr><tr><td>';
				$_SESSION['account']->smb_flagsW = 0;
				echo _('Home Drive');
				echo '</td><td><select name="f_smb_homedrive" >';
					if ( $_SESSION['account']->smb_homedrive == 'D:' ) echo '<option selected> D:'; else echo '<option> D:';
					if ( $_SESSION['account']->smb_homedrive == 'E:' ) echo '<option selected> E:'; else echo '<option> E:';
					if ( $_SESSION['account']->smb_homedrive == 'F:' ) echo '<option selected> F:'; else echo '<option> F:';
					if ( $_SESSION['account']->smb_homedrive == 'G:' ) echo '<option selected> G:'; else echo '<option> G:';
					if ( $_SESSION['account']->smb_homedrive == 'H:' ) echo '<option selected> H:'; else echo '<option> H:';
					if ( $_SESSION['account']->smb_homedrive == 'I:' ) echo '<option selected> I:'; else echo '<option> I:';
					if ( $_SESSION['account']->smb_homedrive == 'J:' ) echo '<option selected> J:'; else echo '<option> J:';
					if ( $_SESSION['account']->smb_homedrive == 'K:' ) echo '<option selected> K:'; else echo '<option> K:';
					if ( $_SESSION['account']->smb_homedrive == 'L:' ) echo '<option selected> L:'; else echo '<option> L:';
					if ( $_SESSION['account']->smb_homedrive == 'M:' ) echo '<option selected> M:'; else echo '<option> M:';
					if ( $_SESSION['account']->smb_homedrive == 'N:' ) echo '<option selected> N:'; else echo '<option> N:';
					if ( $_SESSION['account']->smb_homedrive == 'O:' ) echo '<option selected> O:'; else echo '<option> O:';
					if ( $_SESSION['account']->smb_homedrive == 'P:' ) echo '<option selected> P:'; else echo '<option> P:';
					if ( $_SESSION['account']->smb_homedrive == 'Q:' ) echo '<option selected> Q:'; else echo '<option> Q:';
					if ( $_SESSION['account']->smb_homedrive == 'R:' ) echo '<option selected> R:'; else echo '<option> R:';
					if ( $_SESSION['account']->smb_homedrive == 'S:' ) echo '<option selected> S:'; else echo '<option> S:';
					if ( $_SESSION['account']->smb_homedrive == 'T:' ) echo '<option selected> T:'; else echo '<option> T:';
					if ( $_SESSION['account']->smb_homedrive == 'U:' ) echo '<option selected> U:'; else echo '<option> U:';
					if ( $_SESSION['account']->smb_homedrive == 'V:' ) echo '<option selected> V:'; else echo '<option> V:';
					if ( $_SESSION['account']->smb_homedrive == 'W:' ) echo '<option selected> W:'; else echo '<option> W:';
					if ( $_SESSION['account']->smb_homedrive == 'X:' ) echo '<option selected> X:'; else echo '<option> X:';
					if ( $_SESSION['account']->smb_homedrive == 'Y:' ) echo '<option selected> Y:'; else echo '<option> Y:';
					if ( $_SESSION['account']->smb_homedrive == 'Z:' ) echo '<option selected> Z:'; else echo '<option> Z:';
				echo	'</select></td><td>';
				echo _('Driveletter assigned on Windows-Workstations as Homedirectory.');
				echo '</td></tr><tr><td>';
				echo _('Script Path');
				echo '</td><td><input name="f_smb_scriptpath" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_scriptPath . '">
					</td><td>';
				echo _('Filename and -path relative to netlogon-share which should be executed on logon. $user and $group are replaced with user- and groupname. Can be left empty.');
				echo '</td></tr><tr><td>';
				echo _('Profile Path');
				echo '</td><td><input name="f_smb_profilePath" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_profilePath . '">
					</td><td>';
				echo _('Path of the userprofile. Can be a local absolute path or a UNC-path (\\\\server\share). $user and $group are replaced with user- and groupname. Can be left empty.');
				echo '</td></tr><tr><td>';
				echo _('User Workstations');
				echo '</td><td><input name="f_smb_smbuserworkstations" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_smbuserworkstations . '">
					</td><td>';
				echo _('Komma-separated list of workstations the user is allowed to login. Empty means every workstation. Can be left empty.');
				echo '</td></tr><tr><td>';
				echo _('smb Home');
				echo '</td><td><input name="f_smb_smbhome" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_smbhome . '">
					</td><td>';
				echo _('UNC-path (\\\\server\share) of homedirectory. $user and $group are replaced with user- and groupname. Can be left empty.');
				echo '</td></tr><tr><td>';
				echo _('Domain');
				echo '</td><td><input name="f_smb_domain" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_domain . '">
					</td><td>';
				echo _('Windows-Domain of user. Can be left empty.');
				echo '</td></tr>';
				break;
			case 'host':
				echo '<input name="f_smb_password_no" type="hidden" value="'.$_SESSION['account']->unix_password_no.'">';
				echo '<tr><td>';
				echo _('Password doesn\'t expire.');
				echo '</td><td><input name="f_smb_flagsX" type="checkbox"';
				if ($_SESSION['account']->smb_flagsX) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('Host can change Password');
				echo '</td><td><input name="f_smb_pwdcanchange" type="checkbox"';
				if ($_SESSION['account']->smb_pwdcanchange) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('Host must change Password');
				echo '</td><td><input name="f_smb_pwdmustchange" type="checkbox"';
				if ($_SESSION['account']->smb_pwdmustchange) echo ' checked ';
				echo '></td></tr><tr><td>';
				echo _('Accout is deactivated');
				echo '</td><td><input name="f_smb_flagsD" type="checkbox"';
				if ($_SESSION['account']->smb_flagsD) echo ' checked ';
				echo '></td></tr><tr><td>';
				$_SESSION['account']->smb_flagsW = 1;
				echo '</td></tr><tr><td>';
				echo _('Domain');
				echo '</td><td><input name="f_smb_domain" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_domain . '">
					</td><td>';
				echo _('Windows-Domain of user. Can be left empty.');
				echo '</td></tr>';
				break;
			}
		echo '<tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td></td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'quota':
		// Quota Settings
		echo '<tr><td><input name="select" type="hidden" value="quota">';
		echo _('Quota Properties');
		echo '</td></tr><tr><td>'; echo _('Mointpoint'); echo '</td><td>'; echo _('used blocks'); echo '</td><td>';
		echo _('soft block limit'); echo '</td><td>'; echo _('hard block limit'); echo '</td><td>'; echo _('grace block period');
		echo '</td><td>'; echo _('used inodes'); echo '</td><td>'; echo _('soft inode limit'); echo '</td><td>';
		echo _('hard inode limit'); echo '</td><td>'; echo _('grace inode period'); echo '</td></tr>';
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
		echo '<tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'personal':
		// Personal Settings
		echo '<tr><td><input name="select" type="hidden" value="personal">';
		echo _('Personal Properties');
		echo '</td></tr><tr><td>';
		echo _('Title');
		echo '</td><td>
		<input name="f_personal_title" type="text" size="10" maxlength="10" value="' . $_SESSION['account']->personal_title . '"> ';
		echo $_SESSION['account']->general_surname . ' ' . $_SESSION['account']->general_givenname . '</td><td>';
		echo _('Every value on this page can be left empty.');
		echo '</td></tr><tr><td>';
		echo _('Employee Type');
		echo '</td><td>
		<input name="f_personal_employeeType" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_employeeType . '">
		</td></tr><tr><td>';
		echo _('Street');
		echo '</td><td>
		<input name="f_personal_street" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_street . '">
		</td></tr><tr><td>';
		echo _('Postal code');
		echo '</td><td>
		<input name="f_personal_postalCode" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->personal_postalCode . '">
		</td></tr><tr><td>';
		echo _('Postal address');
		echo '</td><td>
		<input name="f_personal_postalAddress" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_postalAddress . '">
		</td></tr><tr><td>';
		echo _('Telephone Number');
		echo '</td><td>
		<input name="f_personal_telephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_telephoneNumber . '">
		</td></tr><tr><td>';
		echo _('Mobile Phonenumber');
		echo '</td><td>
		<input name="f_personal_mobileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_mobileTelephoneNumber . '">
		</td></tr><tr><td>';
		echo _('Facsimile Number');
		echo '</td><td>
		<input name="f_personal_facsimileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_facsimileTelephoneNumber . '">
		</td></tr><tr><td>';
		echo _('eMail Address');
		echo '</td><td>
		<input name="f_personal_mail" type="text" size="30" maxlength="30" value="' . $_SESSION['account']->personal_mail . '">
		</td></tr><tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td></td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'final':
		// Final Settings
		echo '<tr><td><input name="select" type="hidden" value="final">';
		if ($_SESSION['account_old']) echo _('Modify');
		 else echo _('Create');
		echo '</td></tr>';
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
					echo '<tr>';
					StatusMessage ('INFO', _('UID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
					'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chown ' . $_SESSION['account']->general_uidNumber . ' {} \;');
					echo '</tr>';
					}
				if (($_SESSION['account_old']) && ($_SESSION['account']->general_homedir != $_SESSION['account_old']->general_homedir)) {
					echo '<tr>';
					StatusMessage ('INFO', _('Home Directory has changed. You have to run the following command as root in order to change the existing homedirectory:'),
					'mv ' . $_SESSION['account_old' ]->general_homedir . ' ' . $_SESSION['account']->general_homedir);
					echo '</tr>';
					}
				break;
			case 'group' :
				if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
					echo '<tr>';
					StatusMessage ('INFO', _('GID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
					'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chgrp ' . $_SESSION['account']->general_uidNumber . ' {} \;');
					echo '</tr>';
					echo '<tr><td>';
					echo '<input name="f_final_changegids" type="checkbox"';
						if ($_SESSION['final_changegids']) echo ' checked ';
					echo ' >';
					echo _('Change GID-Number of all users in group to new value');
					echo '</td></tr>';
					}
				break;
			case 'host':
				if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
					echo '<tr>';
					StatusMessage ('INFO', _('UID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
					'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chown ' . $_SESSION['account']->general_uidNumber . ' {} \;');
					echo '</tr>';
					}
				break;
			}
		echo '<tr><td>';
		echo '<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td>';
		echo '</td><td><input name="f_finish_safeProfile" type="text" size="30" maxlength="30">
			<input name="save" type="submit" value="';
		echo _('Save Profile');
		echo '">';
		echo '</td><td>
		<input name="create" type="submit" value="';
		if ($_SESSION['account_old']) echo _('Modify Account');
		 else echo _('Create Account');
		echo '">
		</td></tr>';
		break;
	case 'finish':
		// Final Settings
		echo '<tr><td><input name="select" type="hidden" value="finish">';
		echo _('Success');
		echo '</td></tr>';
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				echo '<tr><td>';
				echo _('User ');
				echo $_SESSION['account']->general_username;
				if ($_SESSION['account_old']) echo _(' has been modified. ');
				 else echo _(' has been created. ');
				if (!$_SESSION['account_old'])
					{ echo '<input name="createagain" type="submit" value="'; echo _('Create another user'); echo '">'; }
				echo '</td><td>
					<input name="outputpdf" type="submit" value="'; echo _('Create PDF-file'); echo '">
					</td><td>
				<input name="backmain" type="submit" value="'; echo _('Back to userlist'); echo '">
				</td></tr>';
				break;
			case 'group' :
				echo '<tr><td>';
				echo _('Group ');
				echo $_SESSION['account']->general_username;
				if ($_SESSION['account_old']) echo _(' has been modified. ');
				 else echo _(' has been created. ');
				echo '</td></tr><tr><td>';
				if (!$_SESSION['account_old'])
					{ echo' <input name="createagain" type="submit" value="'; echo _('Create another group'); echo '">'; }
				echo '</td><td></td><td>
				<input name="backmain" type="submit" value="'; echo _('Back to grouplist'); echo '">
				</td></tr>';
				break;
			case 'host' :
				echo '<tr><td>';
				echo _('Host ');
				echo $_SESSION['account']->general_username;
				if ($_SESSION['account_old']) echo _(' has been modified. ');
				 else echo _(' has been created. ');
				echo '</td></tr><tr><td>';
				if (!$_SESSION['account_old'])
					{ echo '<input name="createagain" type="submit" value="'; echo _('Create another host'); echo '">'; }
				echo '</td><td></td><td>
				<input name="backmain" type="submit" value="'; echo _('Back to hostlist'); echo '">
				</td></tr>';
				break;
			}
		break;
	}

// Print end of HTML-Page
echo '</table></form></body></html>';
?>
