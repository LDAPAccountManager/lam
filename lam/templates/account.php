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
include_once('../lib/config.inc');
include_once('../lib/ldap.inc');
include_once('../lib/profiles.inc');

registervars(); // Register all needed variables in session and register session
$error = "0";
if ( $_GET['type'] ) { // Type is true if account.php was called from Users/Group/Hosts-List
	$_SESSION['type2'] = $_GET['type']; // Register $type in Session for further usage
	$_SESSION['account'] = ""; // Delete $_SESSION['account'] because values are now invalid
	$_SESSION['account_old'] = ""; // Delete $_SESSION['account_old'] because values are now invalid
	$_SESSION['account_temp'] = ""; // Delete $_SESSION['account_temp'] because values are now invalid
	$_SESSION['modify'] = 0; // Set modify back to false
	$_SESSION['shelllist'] = getshells(); // Write List of all valid shells in variable
	}

if ( $_GET['DN'] ) { // $DN is true if an entry should be modified and account.php was called from Users/Group/Host-List
	$_SESSION['modify'] = 1;
	$DN = str_replace("\'", '',$_GET['DN']);
	switch ($_SESSION['type2']) {
		case 'user': loaduser($DN); break;
		case 'group': loadgroup($DN); break;
		case 'host': loadhost($DN); break;
		}
	}

switch ($_POST['select']) {
	case 'general':
		// Write alle values in temporary object
		if ($_POST['f_general_username']) $_SESSION['account_temp']->general_username = $_POST['f_general_username'];
			else $_SESSION['account_temp']->general_username = $_POST['f_general_username'];
		if ($_POST['f_general_surname']) $_SESSION['account_temp']->general_surname = $_POST['f_general_surname'];
			else $_SESSION['account_temp']->general_surname = "";
		if ($_POST['f_general_givenname']) $_SESSION['account_temp']->general_givenname = $_POST['f_general_givenname'];
			else $_SESSION['account_temp']->general_givenname = "";
		if ($_POST['f_general_uidNumber']) $_SESSION['account_temp']->general_uidNumber = $_POST['f_general_uidNumber'];
			else $_SESSION['account_temp']->general_uidNumber = "";
		if ($_POST['f_general_group']) $_SESSION['account_temp']->general_group = $_POST['f_general_group'];
		if ($_POST['f_general_groupadd']) $_SESSION['account_temp']->general_groupadd = $_POST['f_general_groupadd'];
		if ($_POST['f_general_homedir']) $_SESSION['account_temp']->general_homedir = $_POST['f_general_homedir'];
			else $_SESSION['account_temp']->general_homedir = "";
		if ($_POST['f_general_shell']) $_SESSION['account_temp']->general_shell = $_POST['f_general_shell'];
		if ($_POST['f_general_gecos']) $_SESSION['account_temp']->general_gecos = $_POST['f_general_gecos'];
			else $_SESSION['account_temp']->general_gecos = "";
		// Check Values
		$error = checkglobal(); // account.inc
		// Check which part Site should be displayd
		if ($_POST['next'] && ($error=="0"))
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'unix'; break;
				case 'group': $select_local = 'quota'; break;
				case 'host': $select_local = 'unix'; break;
				}
		break;
	case 'unix':
		// Write alle values in temporary object
		if ($_POST['f_unix_password']) $_SESSION['account_temp']->unix_password = $_POST['f_unix_password'];
			else $_SESSION['account_temp']->unix_password = '';
		if ($_POST['genpass']) { $_SESSION['account_temp']->unix_password = genpasswd(); }
		if ($_POST['f_unix_password_no']) $_SESSION['account_temp']->unix_password_no = $_POST['f_unix_password_no'];
			else $_SESSION['account_temp']->unix_password_no = false;
		if ($_POST['f_unix_pwdwarn']) $_SESSION['account_temp']->unix_pwdwarn = $_POST['f_unix_pwdwarn'];
			else $_SESSION['account_temp']->unix_pwdwarn = '';
		if ($_POST['f_unix_pwdallowlogin']) $_SESSION['account_temp']->unix_pwdallowlogin = $_POST['f_unix_pwdallowlogin'];
			else $_SESSION['account_temp']->unix_pwdallowlogin = '';
		if ($_POST['f_unix_pwdmaxage']) $_SESSION['account_temp']->unix_pwdmaxage = $_POST['f_unix_pwdmaxage'];
			else $_SESSION['account_temp']->unix_pwdmaxage = '';
		if ($_POST['f_unix_pwdminage']) $_SESSION['account_temp']->unix_pwdminage = $_POST['f_unix_pwdminage'];
			else $_SESSION['account_temp']->unix_pwdminage = '';
		if ($_POST['f_unix_pwdexpire_day']) $_SESSION['account_temp']->unix_pwdexpire_day = $_POST['f_unix_pwdexpire_day'];
		if ($_POST['f_unix_pwdexpire_mon']) $_SESSION['account_temp']->unix_pwdexpire_mon = $_POST['f_unix_pwdexpire_mon'];
		if ($_POST['f_unix_pwdexpire_yea']) $_SESSION['account_temp']->unix_pwdexpire_yea = $_POST['f_unix_pwdexpire_yea'];
		if ($_POST['f_unix_deactivated']) $_SESSION['account_temp']->unix_deactivated = $_POST['f_unix_deactivated'];
			else $_SESSION['account_temp']->unix_deactivated = false;
		// Check Values
		$error = checkunix(); // account.inc
		// Check which part Site should be displayd
		if ($_POST['back'] && ($error=="0")) $select_local = 'general';
		if ($_POST['genpass'] && ($error=="0")) $select_local = 'unix';
		if ($_POST['next'] && ($error=="0")) $select_local = 'samba';
		break;
	case 'samba':
		// Write alle values in temporary object
		if ($_POST['f_smb_password']) $_SESSION['account_temp']->smb_password = $_POST['f_smb_password'];
			else $_SESSION['account_temp']->smb_password = "";
		if ($_POST['f_smb_password_no']) $_SESSION['account_temp']->smb_password_no = $_POST['f_smb_password_no'];
			else $_SESSION['account_temp']->smb_password_no = false;
		if ($_POST['f_smb_useunixpwd']) $_SESSION['account_temp']->smb_useunixpwd = $_POST['f_smb_useunixpwd'];
			else $_SESSION['account_temp']->smb_useunixpwd = false;
		if ($_POST['f_smb_pwdcanchange']) $_SESSION['account_temp']->smb_pwdcanchange = $_POST['f_smb_pwdcanchange'];
			else $_SESSION['account_temp']->smb_pwdcanchange = false;
		if ($_POST['f_smb_pwdmustchange']) $_SESSION['account_temp']->smb_pwdmustchange = $_POST['f_smb_pwdmustchange'];
			else $_SESSION['account_temp']->smb_pwdmustchange = false;
		if ($_POST['f_smb_homedrive']) $_SESSION['account_temp']->smb_homedrive = $_POST['f_smb_homedrive'];
		if ($_POST['f_smb_scriptpath']) $_SESSION['account_temp']->smb_scriptpath = $_POST['f_smb_scriptpath'];
			else $_SESSION['account_temp']->smb_scriptpath = '';
		if ($_POST['f_smb_smbuserworkstations']) $_SESSION['account_temp']->smb_smbuserworkstations = $_POST['f_smb_smbuserworkstations'];
			else $_SESSION['account_temp']->smb_smbuserworkstations = "";
		if ($_POST['f_smb_smbhome']) $_SESSION['account_temp']->smb_smbhome = stripslashes($_POST['f_smb_smbhome']);
			else $_SESSION['account_temp']->smb_smbhome = "";
		if ($_POST['f_smb_profilePath']) $_SESSION['account_temp']->smb_profilePath = stripslashes($_POST['f_smb_profilePath']);
			else $_SESSION['account_temp']->smb_profilePath = "";
		if ($_POST['f_smb_domain']) $_SESSION['account_temp']->smb_domain = $_POST['f_smb_domain'];
			else $_SESSION['account_temp']->smb_domain = false;
		if ($_POST['f_smb_flagsW']) $_SESSION['account_temp']->smb_flagsW = $_POST['f_smb_flagsW'];
			else $_SESSION['account_temp']->smb_flagsW = false;
		if ($_POST['f_smb_flagsD']) $_SESSION['account_temp']->smb_flagsD = $_POST['f_smb_flagsD'];
			else $_SESSION['account_temp']->smb_flagsD = false;
		if ($_POST['f_smb_flagsX']) $_SESSION['account_temp']->smb_flagsX = $_POST['f_smb_flagsX'];
			else $_SESSION['account_temp']->smb_flagsX = false;
		// Check Values
		$error = checksamba(); // account.inc
		// Check which part Site should be displayd
		if ($_POST['back'] && ($error=="0")) $select_local = 'unix';
		if ($_POST['next'] && ($error=="0"))
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'quota'; break;
				case 'host': $select_local = 'final'; break;
				}
		break;
	case 'quota':
		// Check which part Site should be displayd
		if ($_POST['back'] && ($error=="0"))
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'samba'; break;
				case 'group': $select_local = 'general'; break;
				}
		if ($_POST['next'] && ($error=="0"))
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'personal'; break;
				case 'group': $select_local = 'final'; break;
				}
		break;
	case 'personal':
		if ($_POST['f_personal_title']) $_SESSION['account_temp']->personal_title = $_POST['f_personal_title'];
			else $_SESSION['account_temp']->personal_title = "";
		if ($_POST['f_personal_mail']) $_SESSION['account_temp']->personal_mail = $_POST['f_personal_mail'];
			else $_SESSION['account_temp']->personal_mail = "";
		if ($_POST['f_personal_telephoneNumber']) $_SESSION['account_temp']->personal_telephoneNumber = $_POST['f_personal_telephoneNumber'];
			else $_SESSION['account_temp']->personal_telephoneNumber = "";
		if ($_POST['f_personal_mobileTelephoneNumber']) $_SESSION['account_temp']->personal_mobileTelephoneNumber = $_POST['f_personal_mobileTelephoneNumber'];
			else $_SESSION['account_temp']->personal_mobileTelephoneNumber = "";
		if ($_POST['f_personal_facsimileTelephoneNumber']) $_SESSION['account_temp']->personal_facsimileTelephoneNumber = $_POST['f_personal_facsimileTelephoneNumber'];
			else $_SESSION['account_temp']->personal_facsimileTelephoneNumber = "";
		if ($_POST['f_personal_street']) $_SESSION['account_temp']->personal_street = $_POST['f_personal_street'];
			else $_SESSION['account_temp']->personal_street = "";
		if ($_POST['f_personal_postalCode']) $_SESSION['account_temp']->personal_postalCode = $_POST['f_personal_postalCode'];
			else $_SESSION['account_temp']->personal_postalCode = "";
		if ($_POST['f_personal_postalAddress']) $_SESSION['account_temp']->personal_postalAddress = $_POST['f_personal_postalAddress'];
			else $_SESSION['account_temp']->personal_postalAddress = "";
		if ($_POST['f_personal_employeeType']) $_SESSION['account_temp']->personal_employeeType = $_POST['f_personal_employeeType'];
			else $_SESSION['account_temp']->personal_employeeType = "";
		// Check which part Site should be displayd
		$error = checkpersonal(); // account.inc
		if ($_POST['back'] && ($error=="0")) $select_local = 'quota';
		if ($_POST['next'] && ($error=="0")) $select_local = 'final';
		break;
	case 'final':
		if ($_POST['back'] && ($error=="0"))
			switch ($_SESSION['type2']) {
				case 'user': $select_local = 'personal'; break;
				case 'group': $select_local = 'quota'; break;
				case 'host': $select_local = 'samba'; break;
				}
		break;
	}



if ( $_POST['create'] ) { // Create-Button was pressed
	$_SESSION['account']->final_changegids = $_POST['f_final_changegids'];
	switch ($_SESSION['type2']) {
		case 'user':
			$result = createuser(); // account.inc
			if ( $result==1 || $result==3 ) $select_local = 'finish';
			break;
		case 'group':
			$result = creategroup(); // account.inc
			if ( $result==1 || $result==3 ) $select_local = 'finish';
			break;
		case 'host':
			$result = createhost(); // account.inc
			if ( $result==1 || $result==3 ) $select_local = 'finish';
			break;
		}
	}

// Write HTML-Header and part of Table
echo '<html><head><title>';
echo _('Create new Account');
echo '</title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
	</head><body>
	<form action="account.php" method="post">
	<meta http-equiv="pragma" content="no-cache">
	<meta http-equiv="cache-control" content="no-cache">
	<table rules="all" class="account" width="100%">
	<tr><td>';
	if ($error != "0") echo $error;
	echo '</td></tr>';


if (!$select_local) $select_local='general';
if ($_POST['createagain']) {
	$select_local='general';
	$_SESSION['account']="";
	$_SESSION['account_temp']="";
	$_SESSION['account_old']="";
	}
if ($_POST['backmain']) {
	$select_local='backmain';
	$_SESSION['account']="";
	$_SESSION['account_temp']="";
	$_SESSION['account_old']="";
	}

if ($_POST['load']) $select_local='load';
if ($_POST['save']) $select_local='save';




switch ($select_local) {
	case 'general':
		// General Account Settings
		$groups = findgroups();
		echo '
		<input name="select" type="hidden" value="general">
		<tr><td>';
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
					</td><td>';
				echo _('Can be left empty.');
				echo '</td></tr><tr><td>';
				echo _('Given name');
				echo '</td><td>
					<input name="f_general_givenname" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_givenname . '">
					</td><td>';
				echo _('Can be left empty.');
				echo '</td></tr><tr><td>';
				echo _('Primary Group');
				echo '</td><td><select name="f_general_group">';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group;
					else echo '<option>' . $group;
					 }
				echo '</td></tr><tr><td>';
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
						if ($_SESSION['account']->general_shell==$shell) echo '<option selected> '.$shell;
							else echo '<option> '.$shell;
				echo	'</select></td><td>';
				echo _('To disable login use /bin/false.');
				echo '</td></tr><tr><td><select name="f_general_selectprofile">';
				foreach ($profilelist as $profile) echo '<option>' . $profile;
				echo '</select>
				<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">
				</td><td>';
				break;
			case 'group':
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
				echo '</td></tr>';
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
				echo '</td></tr><tr><td>';
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
		echo '<input name="select" type="hidden" value="unix">';
		echo '<tr><td>Unix Properties</td></tr>';
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
			}
		echo '<tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td></td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'samba':
		// Samba Settings
		echo '<input name="select" type="hidden" value="samba">';
		echo '<tr><td>'; echo _('Samba Properties'); echo '</td></tr>';
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
				echo '</td><td><input name="f_smb_scriptpath" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_scriptpath . '">
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
				echo _('Workstations the user is allowed to login. * means every workstation. Can be left empty.');
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
		echo '<input name="select" type="hidden" value="quota">
		<tr><td>';
		echo _('Quota Properties');
		echo '</td></tr><tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td></td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'personal':
		// Personal Settings
		echo '<input name="select" type="hidden" value="personal">
		<tr><td>';
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
		echo '<input name="select" type="hidden" value="final">
		<tr><td>';
		echo _('Create');
		echo '</td></tr>';
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				if (($_SESSION['modify']==1) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
					echo '<tr><td>';
					echo _('UID-number has changed. You have to run the following command as root in order to change existing file-permissions:');
					echo '</td><td>';
					echo 'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chown ' . $_SESSION['account']->general_uidNumber . ' {} \;';
					echo '</td></tr>';
					}
				if (($_SESSION['modify']==1) && ($_SESSION['account']->general_homedir != $_SESSION['account_old']->general_homedir)) {
					echo '<tr><td>';
					echo _('Home Directory has changed. You have to run the following command as root in order to change the existing homedirectory:');
					echo '</td><td>';
					echo 'mv ' . $_SESSION['account_old' ]->general_homedir . ' ' . $_SESSION['account']->general_homedir;
					echo '</td></tr>';
					}
				break;
			case 'group' :
				if (($_SESSION['modify']==1) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
					echo '<tr><td>';
					echo _('GID-number has changed. You have to run the following command as root in order to change existing file-permissions:');
					echo '</td><td>';
					echo 'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chgrp ' . $_SESSION['account']->general_uidNumber . ' {} \;';
					echo '</td></tr>';
					echo '<tr><td>';
					echo '<input name="f_final_changegids" type="checkbox"';
						if ($_SESSION['account']->final_changegids) echo ' checked ';
					echo ' >';
					echo _('Change GID-Number of all users in group to new value');
					echo '</td></tr>';
					}
				break;
			}
		echo '<tr><td>';
		echo '<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td>';
		if (($_SESSION['type2']=='user') || ($_SESSION['type2']=='host')) {
			echo '</td><td><input name="f_finish_safeProfile" type="text" size="30" maxlength="30">
				<input name="save" type="submit" value="';
			echo _('Save Profile');
			echo '">';
			}
		echo '</td><td>
		<input name="create" type="submit" value="'; echo _('Create Account'); echo '">
		</td></tr>';
		break;
	case 'finish':
		// Final Settings
		echo '<input name="select" type="hidden" value="final">
		<tr><td>';
		echo _('Success');
		echo '</td></tr>';
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				echo '<tr><td>';
				echo _('User ');
				echo $_SESSION['account']->general_username;
				echo _('has been created');
				echo '</td></tr>';
				foreach (file('../config/print.html') as $line) eval("?".">".$line."<"."?");
				echo '<tr><td>
				<input name="createagain" type="submit" value="'; echo _('Create another user'); echo '">
				</td><td>
				<a href  ="javascript:self.print();">';
				echo _('Print');
				echo '</a></td><td>
				<input name="backmain" type="submit" value="'; echo _('Back to userlist'); echo '">
				</td></tr>';
				break;
			case 'group' :
				echo '<tr><td>';
				echo _('Group ');
				echo $_SESSION['account']->general_username;
				echo _('has been created');
				echo '</td></tr><tr><td>
				<input name="createagain" type="submit" value="'; echo _('Create another group'); echo '">
				</td><td></td><td>
				<input name="backmain" type="submit" value="'; echo _('Back to grouplist'); echo '">
				</td></tr>';
				break;
			case 'host' :
				echo '<tr><td>';
				echo _('Host ');
				echo $_SESSION['account']->general_username;
				echo _('has been created');
				echo '</td></tr><tr><td>
				<input name="createagain" type="submit" value="'; echo _('Create another host'); echo '">
				</td><td></td><td>
				<input name="backmain" type="submit" value="'; echo _('Back to hostlist'); echo '">
				</td></tr>';
				break;
			}
		break;
	case 'backmain':
		switch ( $_SESSION['type2'] ) {
			case 'user' :
				echo '<meta http-equiv="refresh" content="0; URL=lists/listusers.php">';
				break;
			case 'group' :
				echo '<meta http-equiv="refresh" content="0; URL=lists/listgroups.php">';
				break;
			case 'host' :
				echo '<meta http-equiv="refresh" content="0; URL=lists/listhosts.php">';
				break;
			}
		break;
	case 'load':
		switch ( $_SESSION['type2'] ) {
			case 'user':
				$_SESSION['account'] = loadUserProfile($f_general_selectprofile);
				break;
			case 'host':
				$_SESSION['account'] = loadHostProfile($f_general_selectprofile);
				break;
			}
		echo '<meta http-equiv="refresh" content="2; URL=account.php">';
		break;
	case 'save':
		switch ( $_SESSION['type2'] ) {
			case 'user':
				saveUserProfile($_SESSION['account'], $f_finish_safeProfile);
			break;
			case 'host':
				saveHostProfile($_SESSION['account'], $f_finish_safeProfile);
			break;
			}
		echo '<meta http-equiv="refresh" content="0; URL=account.php?select=final">';
		break;
	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
