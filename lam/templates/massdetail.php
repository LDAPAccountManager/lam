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

// Start Session
session_save_path('../sess');
@session_start();

// Print header and part of body
echo	'<html><head><title>';
echo _('Create new accounts');
echo '</title>'.
	'<link rel="stylesheet" type="text/css" href="../style/layout.css">'.
	'<meta http-equiv="pragma" content="no-cache">'.
	'<meta http-equiv="cache-control" content="no-cache">'.
	'</head><body>'.
	'<form enctype="multipart/form-data" action="massdetail.php" method="post">'.
	'<table class="massdetail" width="100%">';

if (isset($_GET)) {
	$row = $_GET['row'];
	$select = $_GET['type'];
	}
if ($_POST) {
	$row = $_POST['row'];
	$select = $_POST['type'];
	}

if ($_POST['apply']) {


	// Check if surname is valid
	if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_POST['f_general_surname'])) $errors2[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
		else $_SESSION['accounts'][$row]->general_surname = $_POST['f_general_surname'];
	// Check if givenname is valid
	if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_POST['f_general_givenname'])) $errors2[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
		else $_SESSION['accounts'][$row]->general_givenname = $_POST['f_general_givenname'];
	// Check if username is valid
	if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_POST['f_general_username']))
		$errors2[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, 0-9 and .-_ !'));
	else if ( !ereg('^([a-z]|[A-Z]).*$', $_POST['f_general_username']))
		$errors2[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));
	else {
		$_SESSION['accounts'][$row]->general_username = $_POST['f_general_username'];
		// Check if user already exists
		if (isset($_SESSION['accounts'][$row]->general_groupadd) && in_array($_SESSION['accounts'][$row]->general_group, $_SESSION['accounts'][$row]->general_groupadd)) {
			for ($i=0; $i<count($_SESSION['accounts'][$row]->general_groupadd); $i++ )
				if ($_SESSION['accounts'][$row]->general_groupadd[$i] == $_SESSION['accounts'][$row]->general_group) {
					unset ($_SESSION['accounts'][$row]->general_groupadd[$i]);
					$_SESSION['accounts'][$row]->general_groupadd = array_values($_SESSION['accounts'][$row]->general_groupadd);
					}
			}
		// Create automatic useraccount with number if original user already exists
		// Reset name to original name if new name is in use
		while ($temp = ldapexists($_SESSION['accounts'][$row])) {
			// get last character of username
			$lastchar = substr($_SESSION['accounts'][$row]->general_username, strlen($_SESSION['accounts'][$row]->general_username)-1, 1);
			// Last character is no number
			if ( !ereg('^([0-9])+$', $lastchar))
				$_SESSION['accounts'][$row]->general_username = $_SESSION['accounts'][$row]->general_username . '2';
			 else {
			 	$i=strlen($_SESSION['accounts'][$row]->general_username)-1;
				$mark = false;
			 	while (!$mark) {
					if (ereg('^([0-9])+$',substr($_SESSION['accounts'][$row]->general_username, $i, strlen($_SESSION['accounts'][$row]->general_username)-$i))) $i--;
						else $mark=true;
					}
				// increase last number with one
				$firstchars = substr($_SESSION['accounts'][$row]->general_username, 0, $i+1);
				$lastchars = substr($_SESSION['accounts'][$row]->general_username, $i+1, strlen($_SESSION['accounts'][$row]->general_username)-$i);
				$_SESSION['accounts'][$row]->general_username = $firstchars . (intval($lastchars)+1);
			 	}
			}
		}
	// check if group is valid
	if ($_POST['f_general_group']!='') $_SESSION['accounts'][$row]->general_group = $_POST['f_general_group'];
		else $errors2[] = array('ERROR', _('Primary group'), _('No primary group defined.'));
	if (in_array($_POST['f_general_group'], findgroups())) $_SESSION['accounts'][$row]->general_group = $_POST['f_general_group'];
		else $errors2[] = array('WARN', _('Primary group'), _('Primary group does not exist. Will create group automaticly.'));
	if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_POST['f_personal_title']))  $errors2[] = array('ERROR', _('Title'), _('Please enter a valid title!'));
		else $_SESSION['accounts'][$row]->personal_title = $_POST['f_personal_title'];
	if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_POST['f_personal_employeeType']))  $errors2[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
		else $_SESSION['accounts'][$row]->personal_employeeType = $_POST['f_personal_employeeType'];
	if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_POST['f_personal_street']))  $errors2[] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
		else $_SESSION['accounts'][$row]->personal_street = $_POST['f_personal_street'];
	if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $_POST['f_personal_postalCode']))  $errors2[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		else $_SESSION['accounts'][$row]->personal_postalCode = $_POST['f_personal_postalCode'];
	if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_POST['f_personal_postalAddress']))  $errors2[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
		else $_SESSION['accounts'][$row]->personal_postalAddress = $_POST['f_personal_postalAddress'];
	if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_POST['f_personal_telephoneNumber']))  $errors2[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		else $_SESSION['accounts'][$row]->personal_telephoneNumber = $_POST['f_personal_telephoneNumber'];
	if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_POST['f_personal_mobileTelephoneNumber']))  $errors2[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
		else $_SESSION['accounts'][$row]->personal_mobileTelephoneNumber = $_POST['f_personal_mobileTelephoneNumber'];

	if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_POST['f_personal_facsimileTelephoneNumber']))  $errors2[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
		else $_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber = $_POST['f_personal_facsimileTelephoneNumber'];
	if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $_POST['f_personal_mail']))  $errors2[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
		else $_SESSION['accounts'][$row]->personal_mail = $_POST['f_personal_mail'];

	}

echo '<tr><td><input name="type" type="hidden" value="'.$select.'"></td></tr>';
echo '<tr><td><input name="row" type="hidden" value="'.$row.'"></td></tr>';

if (is_array($errors2)) {
	for ($i=0; $i<sizeof($errors2); $i++) {
		StatusMessage($errors2[$i][0], $errors2[$i][1], $errors2[$i][2]);
		}
	}


switch ($select) {
	case 'error':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'ERROR') {
				StatusMessage('ERROR', _('Invalid Value!'), $_SESSION['errors'][$row][$i][2]);
				}
		break;
	case 'info':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'INFO') {
				StatusMessage('INFO', _('Check values.'), $_SESSION['errors'][$row][$i][2]);
				}
		break;
	case 'warn':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'WARN') {
				StatusMessage('WARN', _('Check values.'), $_SESSION['errors'][$row][$i][2]);
				}
		break;
	case 'detail':
		echo '<tr><td>';
		echo _('Surname').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_surname" type="text" size="20" maxlength="20" value="' . $_SESSION['accounts'][$row]->general_surname . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=424" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Given name').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_givenname" type="text" size="20" maxlength="20" value="' . $_SESSION['accounts'][$row]->general_givenname . '">'.
			'</td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=425" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Username').'*';
		echo "</td>\n<td>".
			'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['accounts'][$row]->general_username . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=400" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Title');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_title" type="text" size="10" maxlength="10" value="' . $_SESSION['accounts'][$row]->personal_title . '"> ';
		echo $_SESSION['accounts']->general_surname . ' ' . $_SESSION['accounts']->general_givenname . '</td><td>'.
			'<a href="help.php?HelpNumber=448" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Employee type');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_employeeType" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_employeeType . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=449" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Street');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_street" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_street . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=450" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Postal code');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_postalCode" type="text" size="5" maxlength="5" value="' . $_SESSION['accounts'][$row]->personal_postalCode . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=451" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Postal address');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_postalAddress" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_postalAddress . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=452" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Telephone number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_telephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_telephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=453" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Mobile number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mobileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_mobileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=454" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Fax number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_facsimileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=455" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('eMail address');
		echo '</td>'."\n".'<td>'.
			'<input name="f_personal_mail" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_mail . '">'.
			'</td><td>'.
			'<a href="help.php?HelpNumber=456" target="lamhelp">'._('Help').'</a>'.
			'</td></tr><br>';
		echo '<tr><td><input name="apply" type="submit" value="'; echo _('Apply'); echo '"></td><td></td><td>';
		echo '<input name="undo" type="submit" value="'; echo _('Undo'); echo '"></td></tr>';

		break;
	}


echo '</table></form></body></html>';
?>
