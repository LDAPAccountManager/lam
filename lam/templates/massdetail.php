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

session_save_path('../sess');
@session_start();

echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
	"http://www.w3.org/TR/html4/loose.dtd">
	<html><head><title>';
echo _('Create new Accounts');
echo '</title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
	<meta http-equiv="pragma" content="no-cache">
	<meta http-equiv="cache-control" content="no-cache">
	<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-15">
	</head><body>
	<form enctype="multipart/form-data" action="massdetail.php" method="post">
	<table class="massdetail" width="100%">
	<tr><td></td></tr>';

if ($_GET) {
	$row = $_GET['row'];
	$select = $_GET['type'];
	}
if ($_POST) {
	$row = $_POST['row'];
	$select = $_POST['type'];
	}

if ($_POST['apply']) {
	if ($_POST['f_general_surname']) $_SESSION['accounts'][$row]->general_surname = $_POST['f_general_surname'];
		else $_SESSION['accounts'][$row]->general_surname='';
	if ($_POST['f_general_givenname']) $_SESSION['accounts'][$row]->general_givenname = $_POST['f_general_givenname'];
		else $_SESSION['accounts'][$row]->general_givenname='';
	if ($_POST['f_general_username']) $_SESSION['accounts'][$row]->general_username = $_POST['f_general_username'];
		else $_SESSION['accounts'][$row]->general_username='';
	if ($_POST['f_general_group']) $_SESSION['accounts'][$row]->general_group = $_POST['f_general_group'];
		else $_SESSION['accounts'][$row]->general_group='';
	if ($_POST['f_personal_title']) $_SESSION['accounts'][$row]->personal_title = $_POST['f_personal_title'];
		else $_SESSION['accounts'][$row]->personal_title='';
	if ($_POST['f_personal_employeeType']) $_SESSION['accounts'][$row]->personal_employeeType = $_POST['f_personal_employeeType'];
		else $_SESSION['accounts'][$row]->personal_employeeType='';
	if ($_POST['f_personal_street']) $_SESSION['accounts'][$row]->personal_street = $_POST['f_personal_street'];
		else $_SESSION['accounts'][$row]->personal_street='';
	if ($_POST['f_personal_postalCode']) $_SESSION['accounts'][$row]->personal_postalCode = $_POST['f_personal_postalCode'];
		else $_SESSION['accounts'][$row]->personal_postalCode='';
	if ($_POST['f_personal_postalAddress']) $_SESSION['accounts'][$row]->personal_postalAddress = $_POST['f_personal_postalAddress'];
		else $_SESSION['accounts'][$row]->personal_postalAddress='';
	if ($_POST['f_personal_telephoneNumber']) $_SESSION['accounts'][$row]->personal_telephoneNumber = $_POST['f_personal_telephoneNumber'];
		else $_SESSION['accounts'][$row]->personal_telephoneNumber='';
	if ($_POST['f_personal_mobileTelephoneNumber']) $_SESSION['accounts'][$row]->personal_mobileTelephoneNumber = $_POST['f_personal_mobileTelephoneNumber'];
		else $_SESSION['accounts'][$row]->personal_mobileTelephoneNumber='';
	if ($_POST['f_personal_facsimileTelephoneNumber']) $_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber = $_POST['f_personal_facsimileTelephoneNumber'];
		else $_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber='';
	if ($_POST['f_personal_mail']) $_SESSION['accounts'][$row]->personal_mail = $_POST['f_personal_mail'];
		else $_SESSION['accounts'][$row]->personal_mail='';
	}

echo '<tr><td><input name="type" type="hidden" value="'.$select.'">';
echo '<tr><td><input name="row" type="hidden" value="'.$row.'">';
switch ($select) {
	case 'error':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'ERROR')
				StatusMessage('ERROR', _('Invalid Value!'), $_SESSION['errors'][$row][$i][2]);
		break;
	case 'warn':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'INFO')
				StatusMessage('INFO', _('Check Value.'), $_SESSION['errors'][$row][$i][2]);
		break;
	case 'detail':
		echo '<tr><td>';
		echo _('Surname*');
		echo '</td>'."\n".'<td>
			<input name="f_general_surname" type="text" size="20" maxlength="20" value="' . $_SESSION['accounts'][$row]->general_surname . '">
			</td><td>
			<a href="help.php?HelpNumber=424" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Given name*');
		echo '</td>'."\n".'<td>
			<input name="f_general_givenname" type="text" size="20" maxlength="20" value="' . $_SESSION['accounts'][$row]->general_givenname . '">
			</td>'."\n".'<td>
			<a href="help.php?HelpNumber=425" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Username*');
		echo "</td>\n<td>".
			'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['accounts'][$row]->general_username . '">
			</td><td>
			<a href="help.php?HelpNumber=400" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Primary Group*');
		echo "</td>\n<td>".
			'<input name="f_general_group" type="text" size="20" maxlength="20" value="' . $_SESSION['accounts'][$row]->general_group . '">
			</td><td>
			<a href="help.php?HelpNumber=406" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Title');
		echo '</td>'."\n".'<td>
			<input name="f_personal_title" type="text" size="10" maxlength="10" value="' . $_SESSION['accounts'][$row]->personal_title . '"> ';
		echo $_SESSION['account']->general_surname . ' ' . $_SESSION['account']->general_givenname . '</td><td>
			<a href="help.php?HelpNumber=448" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Employee Type');
		echo '</td>'."\n".'<td>
			<input name="f_personal_employeeType" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_employeeType . '">
			</td><td>
			<a href="help.php?HelpNumber=449" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Street');
		echo '</td>'."\n".'<td>
			<input name="f_personal_street" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_street . '">
			</td><td>
			<a href="help.php?HelpNumber=450" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Postal code');
		echo '</td>'."\n".'<td>
			<input name="f_personal_postalCode" type="text" size="5" maxlength="5" value="' . $_SESSION['accounts'][$row]->personal_postalCode . '">
			</td><td>
			<a href="help.php?HelpNumber=451" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Postal address');
		echo '</td>'."\n".'<td>
			<input name="f_personal_postalAddress" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_postalAddress . '">
			</td><td>
			<a href="help.php?HelpNumber=452" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Telephone Number');
		echo '</td>'."\n".'<td>
			<input name="f_personal_telephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_telephoneNumber . '">
			</td><td>
			<a href="help.php?HelpNumber=453" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Mobile Phonenumber');
		echo '</td>'."\n".'<td>
			<input name="f_personal_mobileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_mobileTelephoneNumber . '">
			</td><td>
			<a href="help.php?HelpNumber=454" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('Facsimile Number');
		echo '</td>'."\n".'<td>
			<input name="f_personal_facsimileTelephoneNumber" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber . '">
			</td><td>
			<a href="help.php?HelpNumber=455" target="lamhelp">'._('Help').'</a>
			</td></tr>'."\n".'<tr><td>';
		echo _('eMail Address');
		echo '</td>'."\n".'<td>
			<input name="f_personal_mail" type="text" size="30" maxlength="30" value="' . $_SESSION['accounts'][$row]->personal_mail . '">
			</td><td>
			<a href="help.php?HelpNumber=456" target="lamhelp">'._('Help').'</a>
			</td></tr><br>';
		echo '<tr><td><input name="apply" type="submit" value="'; echo _('Apply Changes'); echo '"></td><td></td><td>';
		echo '<input name="undo" type="submit" value="'; echo _('Undo last Changes'); echo '"></td></tr>';

		break;
	}


echo '</table></form></body></html>';
?>
