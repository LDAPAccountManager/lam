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

session_save_path('../sess');
@session_start();

print_r($_FILES['userfile']);
if ($_POST['tolist'] && ($_FILES['userfile']['size']>0)) $select = 'list';
if ($_POST['back']) $select = 'main';
if ($_POST['cancel']) $select = 'cancel';
if ($_POST['create']) $select = 'create';
if ($_POST['pdf']) createpdf($_SESSION['accounts']);
if (!$select) $select='main';


// Write HTML-Header and part of Table
echo '<html><head><title>';
echo _('Create new Accounts');
echo '</title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
	<meta http-equiv="pragma" content="no-cache">
	<meta http-equiv="cache-control" content="no-cache">
	</head><body>
	<form enctype="multipart/form-data" action="masscreate.php" method="post">';
	echo '<table rules="all" class="masscreate" width="100%">
	<tr><td></td></tr>';

switch ($select) {
	case 'main':
		// if session was started previos, the existing session will be continued
		$profilelist = getUserProfiles();
		echo '<input name="select" type="hidden" value="main">';
		echo '<tr><td>';
		echo _('Mass Creation');
		echo '</td></tr><tr><td>';
		echo _('Please provide a csv-file with the following syntax. Values with * are required:');
		echo '</td></tr><tr><td>';
		echo _('Surname*,Givenname*,Username*,PrimaryGroup,Title,Mail,telephonenumber,');
		echo '</td></tr><tr><td>';
		echo _('mobileTelephoneNumber,facsimileNumber,street,postalCode,postalAddress,');
		echo '</td></tr><tr><td>';
		echo _('employeeType. If PrimaryGroup is not given it\'ll used from profile.');
		echo '</td></tr><tr><td>';
		echo _('If PrimaryGroup doesn\'t exist it will be created.');
		echo '</td></tr><tr><td>';
		echo _('Select Profile:');
		echo '</td><td><select name="f_selectprofile">';
			foreach ($profilelist as $profile) echo '<option>' . $profile;
			echo '</select>';
		echo '</td></tr><tr><td>
			<input type="hidden" name="MAX_FILE_SIZE" value="100000">';
		echo _('Select file:');
		echo '</td><td><input name="userfile" type="file"></td></tr>
			<tr><td><input name="tolist" type="submit" value="'; echo _('Commit'); echo '">';
		echo '</td></tr>';
		break;
	case 'list':
		if ( session_is_registered("accounts")) session_unregister("accounts");
		session_register("accounts");
		if (!is_array($accounts)) $accounts = array();
	 	$handle = fopen($_FILES['userfile']['tmp_name'], 'r');
		$error=false;
		echo '<tr><td>';
		echo _('Confirm List');
		echo '</td></tr>';
		for ($row=0; $line_array=fgetcsv($handle,2048); ++$row) { // loops for every row
			$_SESSION['accounts'][$row] = loadUserProfile($_POST['f_selectprofile']) ;
			if ($line_array[0]) $_SESSION['accounts'][$row]->general_surname = $line_array[0];
			if ($line_array[1]) $_SESSION['accounts'][$row]->general_givenname = $line_array[1];
			if ($line_array[2]) $_SESSION['accounts'][$row]->general_username = $line_array[2];
			if ($line_array[3]) $_SESSION['accounts'][$row]->general_group = $line_array[3];
			if ($line_array[4]) $_SESSION['accounts'][$row]->personal_title = $line_array[4];
			if ($line_array[5]) $_SESSION['accounts'][$row]->personal_mail = $line_array[5];
			if ($line_array[6]) $_SESSION['accounts'][$row]->personal_telephoneNumber = $line_array[6];
			if ($line_array[7]) $_SESSION['accounts'][$row]->personal_mobileTelephoneNumber = $line_array[7];
			if ($line_array[8]) $_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber = $line_array[8];
			if ($line_array[9]) $_SESSION['accounts'][$row]->personal_street = $line_array[9];
			if ($line_array[10]) $_SESSION['accounts'][$row]->personal_postalCode = $line_array[10];
			if ($line_array[11]) $_SESSION['accounts'][$row]->personal_postalAddress = $line_array[11];
			if ($line_array[12]) $_SESSION['accounts'][$row]->personal_employeeType = $line_array[12];
			$_SESSION['accounts'][$row]->unix_password=genpasswd();
			$_SESSION['accounts'][$row]->smb_password=genpasswd();
			$values = checkglobal($_SESSION['accounts'][$row], 'user'); // account.inc
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if ($val) $_SESSION['accounts'][$row]->$key = $val;
				}
				else $error = $values;
			if (!$error) {
				$values = checkpersonal($_SESSION['accounts'][$row], 'user'); // account.inc
				if (is_object($values)) {
					while (list($key, $val) = each($values)) // Set only defined values
						if ($val) $_SESSION['accounts'][$row]->$key = $val;
					}
					else $error = $values;
				}
			if (!$error) {
				$values = checksamba($_SESSION['accounts'][$row], 'user'); // account.inc
				while (list($key, $val) = each($values)) // Set only defined values
					if ($val) $_SESSION['accounts'][$row]->$key = $val;
				$values = checkquota($_SESSION['accounts'][$row], 'user'); // account.inc
				while (list($key, $val) = each($values)) // Set only defined values
					if ($val) $_SESSION['accounts'][$row]->$key = $val;
				}
			if ($error) StatusMessage('ERROR', _('Invalid Value in row ').$row.'!', $error);
			if (getgid($_SESSION['accounts'][$row]->general_group)==-1) StatusMessage('INFO', _('Group ').
				$_SESSION['accounts'][$row]->general_group._(' not found in row ').$row.'!', _('It will be created.'));
			}
		fclose($handle);
		unlink($_FILES['userfile']['tmp_name']);
		echo '<tr><td>'. _('Surname'). '</td><td>'. _('Givenname'). '</td><td>'. _('Username'). '</td><td>'. _('Primary Group'). '</td><td>'.
			_('Title'). '</td><td>'. _('Mail Address'). '</td><td>'. _('Telephonenumber'). '</td><td>'. _('Mobiletelephonenumber')
			. '</td><td>'. _('Facsimiletelephonenumber'). '</td><td>'. _('Street'). '</td><td>'. _('Postal Code')
			. '</td><td>'. _('Postal Address'). '</td><td>'. _('Employee Type') .'</td></tr>';
		for ($row=0; sizeof($_SESSION['accounts']); $row++) { // loops for every row
			echo '<tr><td>'.$_SESSION['accounts'][$row]->general_surname.'</td><td>'.
				$_SESSION['accounts'][$row]->general_givenname.'</td><td>'.
				$_SESSION['accounts'][$row]->general_username.'</td><td>'.
				$_SESSION['accounts'][$row]->general_group.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_title.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_mail.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_telephoneNumber.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_mobileTelephoneNumber.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_street.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_postalCode.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_postalAddress.'</td><td>'.
				$_SESSION['accounts'][$row]->personal_employeeType.'</td></tr>';
			}
		echo '<tr><td><input name="back" type="submit" value="'; echo _('Back'); echo '">';
		echo '</td><td><input name="cancel" type="submit" value="'; echo _('Cancel'); echo '">';
		echo '</td><td><input name="create" type="submit" value="'; echo _('Create'); echo '">';
		break;
	case 'cancel':
		echo '<meta http-equiv="refresh" content="0; URL=lists/listusers.php">';
		break;
	case 'create':
		$row=0;
		while ($row < sizeof($_SESSION['accounts']) || $row!=-1) {
			if (getgid($_SESSION['accounts'][$row]->general_group)==-1) {
				$group = new account();
				$group->general_username=$_SESSION['accounts'][$row]->general_group;
				$group->general_uidNumber=checkid($_SESSION['accounts'][$row], 'group');
				$group->general_gecos=$_SESSION['accounts'][$row]->general_group;
				creategroup($_SESSION['accounts'][$row]);
				}
			$error = createuser($_SESSION['accounts'][$row]);
			if ($error==1) $row++;
				else {
				$row = -1;
				StatusMessage('ERROR', _('Could not create user'), _('Was unable to create ').$_SESSION['accounts'][$row]->general_username);
				}
			}
		if ($row=-1) { echo '<tr><td><input name="cancel" type="submit" value="'; echo _('Cancel'); echo '">'; }
			else {
			echo '<tr><td>';
			echo _('All Users have been created');
			echo '</td></tr><tr><td>';
			echo '<tr><td><input name="cancel" type="submit" value="'; echo _('Mainmenu'); echo '">';
			echo '<tr><td><input name="pdf" type="submit" value="'; echo _('Create PDF-File'); echo '">';
			}
		break;
	}


echo '</form></body></html>';
?>
