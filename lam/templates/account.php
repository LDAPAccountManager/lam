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

include('account.inc'); // File with custom functions
include('../config/config.php');
include('../lib/ldap.php');

registervars(); // Register all needed variables in session and register session
$error = "0";
if ( $type ) { // Type is true if account.php was called from Users/Group/Hosts-List
	$_SESSION['type2'] = $type; // Register $type in Session for further usage
	$_SESSION['account'] = ""; // Delete $_SESSION['account'] because values are now invalid
	$_SESSION['account_old'] = ""; // Delete $_SESSION['account_old'] because values are now invalid
	$_SESSION['account_temp'] = ""; // Delete $_SESSION['account_temp'] because values are now invalid
	$_SESSION['modify'] = 0; // Set modify back to false
	}

if ( $DN ) { // $DN is true if an entry should be modified and account.php was called from Users/roup/Host-List
	$_SESSION['modify'] = 1;
	$DN = str_replace("\'", '',$DN);
	switch ($type2) {
		// case 'user': loadaccount($DN); break;
		case 'group': loadgroup($DN); break;
		// case 'host': loadhost($DN); break;
		}
	}

switch ($select) {
	case 'general':
		// Write alle values in temporary object
		if ($f_general_username) $_SESSION['account_temp']->general_username = $f_general_username;
			else $_SESSION['account_temp']->general_username = $f_general_username;
		if ($f_general_surname) $_SESSION['account_temp']->general_surname = $f_general_surname;
			else $_SESSION['account_temp']->general_surname = "";
		if ($f_general_givenname) $_SESSION['account_temp']->general_givenname = $f_general_givenname;
			else $_SESSION['account_temp']->general_givenname = "";
		if ($f_general_uidNumber) $_SESSION['account_temp']->general_uidNumber = $f_general_uidNumber;
			else $_SESSION['account_temp']->general_uidNumber = "";
		if ($f_general_group) $_SESSION['account_temp']->general_group = $f_general_group;
		if ($f_general_groupadd) $_SESSION['account_temp']->general_groupadd = $f_general_groupadd;
		if ($f_general_homedir) $_SESSION['account_temp']->general_homedir = $f_general_homedir;
			else $_SESSION['account_temp']->general_homedir = "";
		if ($f_general_shell) $_SESSION['account_temp']->general_shell = $f_general_shell;
		if ($f_general_gecos) $_SESSION['account_temp']->general_gecos = $f_general_gecos;
			else $_SESSION['account_temp']->general_gecos = "";
		// Check Values
		$error = checkglobal(); // account.inc
		// Check which part Site should be displayd
		if ($next && ($error=="0")) $select = 'unix';
		break;
	case 'unix':
		// Write alle values in temporary object
		if ($genpass) { $f_unix_password = genpasswd(); }
		if ($f_unix_password) $_SESSION['account_temp']->unix_password = $f_unix_password;
			else $_SESSION['account_temp']->unix_password = "";
		if ($f_unix_pwdwarn) $_SESSION['account_temp']->unix_pwdwarn = $f_unix_pwdwarn;
			else $_SESSION['account_temp']->unix_pwdwarn = "0";
		if ($f_unix_pwdallowlogin) $_SESSION['account_temp']->unix_pwdallowlogin = $f_unix_pwdallowlogin;
			else $_SESSION['account_temp']->unix_pwdallowlogin = "0";
		if ($f_unix_pwdmaxage) $_SESSION['account_temp']->unix_pwdmaxage = $f_unix_pwdmaxage;
			else $_SESSION['account_temp']->unix_pwdmaxage = "0";
		if ($f_unix_pwdminage) $_SESSION['account_temp']->unix_pwdminage = $f_unix_pwdminage;
			else $_SESSION['account_temp']->unix_pwdminage = "0";
		if ($f_unix_pwdexpire_day) $_SESSION['account_temp']->unix_pwdexpire_day = $f_unix_pwdexpire_day;
		if ($f_unix_pwdexpire_mon) $_SESSION['account_temp']->unix_pwdexpire_mon = $f_unix_pwdexpire_mon;
		if ($f_unix_pwdexpire_yea) $_SESSION['account_temp']->unix_pwdexpire_yea = $f_unix_pwdexpire_yea;
		if ($f_unix_deactivated) $_SESSION['account_temp']->unix_deactivated = $f_unix_deactivated;
			else $_SESSION['account_temp']->unix_deactivated = false;
		// Check Values
		$error = checkunix(); // account.inc
		// Check which part Site should be displayd
		if ($back && ($error=="0")) $select = 'general';
		if ($next && ($error=="0")) $select = 'samba';
		break;
	case 'samba':
		// Write alle values in temporary object
		if ($f_smb_password) $_SESSION['account_temp']->smb_password = $f_smb_password;
			else $_SESSION['account_temp']->smb_password = "";
		if ($f_smb_useunixpwd) $_SESSION['account_temp']->smb_useunixpwd = $f_smb_useunixpwd;
			else $_SESSION['account_temp']->smb_useunixpwd = false;
		if ($f_smb_pwdcanchange) $_SESSION['account_temp']->smb_pwdcanchange = $f_smb_pwdcanchange;
			else $_SESSION['account_temp']->smb_pwdcanchange = false;
		if ($f_smb_pwdmustchange) $_SESSION['account_temp']->smb_pwdmustchange = $f_smb_pwdmustchange;
			else $_SESSION['account_temp']->smb_pwdmustchange = false;
		if ($f_smb_homedrive) $_SESSION['account_temp']->smb_homedrive = $f_smb_homedrive;
		if ($f_smb_scriptpath) $_SESSION['account_temp']->smb_scriptpath = $f_smb_scriptpath;
			else $_SESSION['account_temp']->smb_scriptpath = "";
		if ($f_smb_smbuserworkstations) $_SESSION['account_temp']->smb_smbuserworkstations = $f_smb_smbuserworkstations;
			else $_SESSION['account_temp']->smb_smbuserworkstations = "";
		if ($f_smb_smbhome) $_SESSION['account_temp']->smb_smbhome = stripslashes($f_smb_smbhome);
			else $_SESSION['account_temp']->smb_smbhome = "";
		if ($f_smb_profilePath) $_SESSION['account_temp']->smb_profilePath = stripslashes($f_smb_profilePath);
			else $_SESSION['account_temp']->smb_profilePath = "";
		if ($f_smb_domain) $_SESSION['account_temp']->smb_domain = $f_smb_domain;
			else $_SESSION['account_temp']->smb_domain = false;
		if ($f_smb_flagsW) $_SESSION['account_temp']->smb_flagsW = $f_smb_flagsW;
			else $_SESSION['account_temp']->smb_flagsW = false;
		if ($f_smb_flagsD) $_SESSION['account_temp']->smb_flagsD = $f_smb_flagsD;
			else $_SESSION['account_temp']->smb_flagsD = false;
		if ($f_smb_flagsX) $_SESSION['account_temp']->smb_flagsX = $f_smb_flagsX;
			else $_SESSION['account_temp']->smb_flagsX = false;
		// Check Values
		$error = checksamba(); // account.inc
		// Check which part Site should be displayd
		if ($back && ($error=="0")) $select = 'unix';
		if ($next && ($error=="0")) $select = 'quota';
		break;
	case 'quota':
		// Check which part Site should be displayd
		if ($back && ($error=="0")) $select = 'samba';
		if ($next && ($error=="0")) $select = 'personal';
		break;
	case 'personal':
		// Check which part Site should be displayd
		if ($back && ($error=="0")) $select = 'quota';
		break;
	}


if ($error != "0") {
	echo '<script language="javascript"> alert("';
	echo $error;
	echo '"); </script>';
	}

if ( $create ) { // Create-Button was pressed
	switch ($_SESSION['type2']) {
		case 'user':
			$result = createaccount(); // account.inc
			if ( $result==1 || $result==3 ) {
				$_SESSION['account'] = "";
				$_SESSION['account_old'] = "";
				$_SESSION['account_temp'] = "";
				// Dialog anzeigen, dass Benutzer angelegt wurde und fragen, ob Daten ausgedruckt werden sollen
				}
			break;
		case 'group':
			$result = creategroup(); // account.inc
			if ( $result==1 || $result==3 ) {
				$_SESSION['account'] = "";
				$_SESSION['account_old'] = "";
				$_SESSION['account_temp'] = "";
				// Dialog anzeigen, dass Gruppe angelegt wurde und fragen, ob Daten ausgedruckt werden sollen
				}
			break;
		case 'host':
			$result = createhost(); // account.inc
			if ( $result==1 || $result==3 ) {
				$_SESSION['account'] = "";
				$_SESSION['account_old'] = "";
				$_SESSION['account_temp'] = "";
				// Dialog anzeigen, dass host angelegt wurde und fragen, ob Daten ausgedruckt werden sollen
				}
			break;
		}
	}

// Write HTML-Header and part of Table
echo '<html><head><title>';
echo _('Create new Account');
echo '</title></head><body>
	<form action="account.php" method="post">
	<meta http-equiv="pragma" content="no-cache">
	<meta http-equiv="cache-control" content="no-cache">
	<table border="1">';
if (!$select) $select='general';
switch ($select) {
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
				echo '<tr><td>';
				echo _('Username');
				echo '</td><td>
					<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">
					</td></tr><tr><td>';
				echo _('UID (none=automatic)');
				echo '</td><td>
					<input name="f_general_uidNumber" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_uidNumber . '">
					</td></tr><tr><td>';
				echo _('Surname');
				echo '</td><td>
					<input name="f_general_surname" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_surname . '">
					</td></tr><tr><td>';
				echo _('Given name');
				echo '</td><td>
					<input name="f_general_givenname" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_givenname . '">
					</td></tr>';
				echo '<tr><td>';
				echo _('Primary Group');
				echo '</td><td><select name="f_general_group">';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group;
					else echo '<option selected>' . $group;
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
				echo	'</select></td></tr>';
				echo '<tr><td>';
				echo _('Home Directory ($user and $group will be replaced with username or groupname.)');
				echo '</td><td><input name="f_general_homedir" type="text" size="30" value="' . $_SESSION['account']->general_homedir . '">
				</td></tr><tr><td>';
				echo _('Gecos, User Discribtion. If empty, given- and surname is inserted.');
				echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">
				</td></tr><tr><td>';
				echo _('Login Shell');
				echo '</td><td><select name="f_general_shell" >';
					if ( $_SESSION['account']->general_shell == '/bin/ash' ) echo '<option selected> /bin/ash'; else echo '<option> /bin/ash';
					if ( $_SESSION['account']->general_shell == '/bin/bash' ) echo '<option selected> /bin/bash'; else echo '<option> /bin/bash';
					if ( $_SESSION['account']->general_shell == '/bin/csh' ) echo '<option selected> /bin/csh'; else echo '<option> /bin/csh';
					if ( $_SESSION['account']->general_shell == '/bin/false' ) echo '<option selected> /bin/false'; else echo '<option> /bin/false';
					if ( $_SESSION['account']->general_shell == '/bin/sh' ) echo '<option selected> /bin/sh'; else echo '<option> /bin/sh';
					if ( $_SESSION['account']->general_shell == '/bin/tcsh' ) echo '<option selected> /bin/tcsh'; else echo '<option> /bin/tcsh';
					if ( $_SESSION['account']->general_shell == '/bin/true' ) echo '<option selected> /bin/true'; else echo '<option> /bin/true';
					if ( $_SESSION['account']->general_shell == '/bin/zsh' ) echo '<option selected> /bin/zsh'; else echo '<option> /bin/zsh';
					if ( $_SESSION['account']->general_shell == '/usr/bin/csh' ) echo '<option selected> /usr/bin/csh'; else echo '<option> /usr/bin/csh';
					if ( $_SESSION['account']->general_shell == '/usr/bin/rbash' ) echo '<option selected> /usr/bin/rbash'; else echo '<option> /usr/bin/rbash';
					if ( $_SESSION['account']->general_shell == '/usr/bin/tcsh' ) echo '<option selected> /usr/bin/tcsh'; else echo '<option> /usr/bin/tcsh';
					if ( $_SESSION['account']->general_shell == '/usr/bin/zsh' ) echo '<option selected> /usr/bin/zsh'; else echo '<option> /usr/bin/zsh';
				echo	'</select></td></tr>';
				break;
			case 'group':
				echo '<tr><td>';
				echo _('Groupname');
				echo '</td><td>
				<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">
				</td></tr><tr><td>';
				echo _('GID (none=automatic)');
				echo '</td><td>
				<input name="f_general_uidNumber" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_uidNumber . '">
				</td></tr><tr><td>';
				echo _('Gecos, Group Discribtion.');
				echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">
				</td></tr>';
				break;
			case 'host':
				echo '<tr><td>';
				echo _('Hostname');
				echo '</td><td>
				<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">
				</td></tr><tr><td>';
				echo _('UID');
				echo '</td><td>
				<input name="f_general_uidNumber" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_uidNumber . '">
				</td></tr><tr><td>';
				echo _('Primary Group');
				echo '</td><td><select name="f_general_group">';
				foreach ($groups as $group) {
					if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group;
					else echo '<option selected>' . $group;
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
				echo	'</select></td></tr>';
				echo '<tr><td>';
				echo '</td></tr><tr><td>';
				echo _('Gecos, Host Discribtion.');
				echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">
				</td></tr>';
				echo '</td></tr>';
				break;
			}
		echo '<tr><td>
		<input name="load" type="submit" value="'; echo _('Load Profile'); echo '">
		</td><td>
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
				echo _('How many days warn before password expires?');
				echo '</td><td><input name="f_unix_pwdwarn" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdwarn . '">
				</td></tr><tr><td>';
				echo _('How many days login as allowed after password has expired (-1=allways)');
				echo '</td><td><input name="f_unix_pwdallowlogin" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdallowlogin . '">
				</td></tr><tr><td>';
				echo _('Maximum Passwordage');
				echo '</td><td><input name="f_unix_pwdmaxage" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->unix_pwdmaxage . '">
				</td></tr><tr><td>';
				echo _('Minimum Passwordage');
				echo '</td><td><input name="f_unix_pwdminage" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdminage . '">
				</td></tr><tr><td>';
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
				echo '</select>
				</td></tr><tr><td>';
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
				echo _('How many days warn before password expires?');
				echo '</td><td><input name="f_unix_pwdwarn" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdwarn . '">
				</td></tr><tr><td>';
				echo _('How many days login as allowed after password has expired (-1=allways)');
				echo '</td><td><input name="f_unix_pwdallowlogin" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdallowlogin . '">
				</td></tr><tr><td>';
				echo _('Maximum Passwordage');
				echo '</td><td><input name="f_unix_pwdmaxage" type="text" size="5" maxlength="5" value="' . $_SESSION['account']->unix_pwdmaxage . '">
				</td></tr><tr><td>';
				echo _('Minimum Passwordage');
				echo '</td><td><input name="f_unix_pwdminage" type="text" size="4" maxlength="4" value="' . $_SESSION['account']->unix_pwdminage . '">
				</td></tr><tr><td>';
				echo _('Expire Date');
				echo '</td><td><select name="f_unix_pwdexpire_day">';
				for ( $i=1; $i<=31; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_day==$i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select><select name="f_unix_pwdexpire_mon">';
				for ( $i=1; $i<=12; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_mon==$i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select><select name="f_unix_pwdexpire_yea">';
				for ( $i=2003; $i<=2030; $i++ ) {
					if ($_SESSION['account']->unix_pwdexpire_yea==$i) echo "<option selected> $i";
					else echo "<option> $i";
					}
				echo '</select>
				</td></tr><tr><td>';
				echo _('Account deactivated');
				echo '</td><td><input name="f_unix_deactivated" type="checkbox"';
				if ($_SESSION['account']->unix_deactivated) echo ' checked ';
				echo '></td></tr>';
				break;
			}
		echo '<tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td>
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
				echo	'</select></td></tr><tr><td>';
				echo _('Script Path');
				echo '</td><td><input name="f_smb_scriptpath" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_scriptpath . '">
				</td></tr><tr><td>';
				echo _('Profile Path');
				echo '</td><td><input name="f_smb_profilePath" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_profilePath . '">
				</td></tr><tr><td>';
				echo _('User Workstations');
				echo '</td><td><input name="f_smb_smbuserworkstations" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_smbuserworkstations . '">
				</td></tr><tr><td>';
				echo _('smb Home');
				echo '</td><td><input name="f_smb_smbhome" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_smbhome . '">
				</td></tr><tr><td>';
				echo _('Domain');
				echo '</td><td><input name="f_smb_domain" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_domain . '">
				</td></tr>';
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
				echo _('Domain');
				echo '</td><td><input name="f_smb_domain" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->smb_domain . '">
				</td></tr>';
				break;
			}
		echo '<tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td>
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
		</td><td>
		<input name="next" type="submit" value="'; echo _('next'); echo '">
		</td></tr>';
		break;
	case 'personal':
		// Personal Settings
		echo '<input name="select" type="hidden" value="personal">
		<tr><td>';
		echo _('Personal Properties');
		echo '</td></tr><tr><td>
		<input name="back" type="submit" value="'; echo _('back'); echo '">
		</td><td>
		<input name="save" type="submit" value="'; echo _('Save Profile'); echo '">
		</td><td>
		<input name="create" type="submit" value="'; echo _('Create Account'); echo '">
		</td></tr>';
		break;
	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
