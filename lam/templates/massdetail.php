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
include_once('../lib/account.inc'); // File with all account-funtions
include_once('../lib/config.inc'); // File with configure-functions
include_once('../lib/profiles.inc'); // functions to load and save profiles
include_once('../lib/status.inc'); // Return error-message
include_once('../lib/pdf.inc'); // Return a pdf-file
include_once('../lib/ldap.inc'); // LDAP-functions

// Start Session
session_save_path('../sess');
@session_start();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	metaRefresh("login.php");
	die;
	}

// Set correct language, codepages, ....
setlanguage();

// Startcondition massdetail.php was called from masscreate.php
if (isset($_GET)) {
	// $row the the position of the useraccount in an array of account-objects
	$row = $_GET['row'];
	/* $select chooses which kind of page should be displayed
	* detail = Show settings which are individuel for every user. These
	*          settings can be changed
	* info = Show all infos about user
	* warn = Show all warning about user
	* error = Show all errors about user
	*/
	$select = $_GET['type'];
	// Get Copy of current account so we can undo all settings
	if ($select=='detail') $_SESSION['accounts_backup'] = $_SESSION['accounts'][$row];
	}
// massdetail.php was called from itself
else if (isset($_POST)) {
	// $row the the position of the useraccount in an array of account-objects
	$row = $_POST['row'];
	/* $select chooses which kind of page should be displayed
	* detail = Show settings which are individuel for every user. These
	*          settings can be changed
	* info = Show all infos about user
	* warn = Show all warning about user
	* error = Show all errors about user
	*/
	$select = $_POST['type'];
	}

// Undo-Button was pressed.
if ($_POST['undo']) {
	$_SESSION['accounts'][$row] = $_SESSION['accounts_backup'];
	$errors2[] = array('INFO', _('Undo'), _('All changes were reseted'));
	$select = 'detail';
	}

// Apply-Button was pressed.
if ($_POST['apply']) {
	// Show Detail-page
	$select = 'detail';
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
		// Create Array with all users in ldap and in array
		// Validate cache-array
		ldapreload('user');
		// Get List with all existing usernames
		foreach ($_SESSION['userDN'] as $user_array) $users[] = $user_array['cn'];
		// Get List with all users in array
		foreach ($_SESSION['accounts'] as $user_array) $users[] = $user_array->general_username;
		// unset old username in user-array
		$users = @array_flip($users);
		unset ($users[$_SESSION['accounts'][$row]->general_username]);
		$users = array_flip($users);
		// Store new username
		$_SESSION['accounts'][$row]->general_username = $_POST['f_general_username'];
			// Set all usernames to unique usernames
		while (in_array($_SESSION['accounts'][$row2]->general_username, $users)) {
			// get last character of username
			$lastchar = substr($_SESSION['accounts'][$row2]->general_username, strlen($_SESSION['accounts'][$row2]->general_username)-1, 1);
			// Last character is no number
			if ( !ereg('^([0-9])+$', $lastchar))
				/* Last character is no number. Therefore we only have to
				* add "2" to it.
				*/
				$_SESSION['accounts'][$row2]->general_username = $_SESSION['accounts'][$row2]->general_username . '2';
			 else {
				/* Last character is a number -> we have to increase the number until we've
				* found a groupname with trailing number which is not in use.
				*
				* $i will show us were we have to split groupname so we get a part
				* with the groupname and a part with the trailing number
				*/
			 	$i=strlen($_SESSION['accounts'][$row2]->general_username)-1;
				$mark = false;
				// Set $i to the last character which is a number in $account_new->general_username
			 	while (!$mark) {
					if (ereg('^([0-9])+$',substr($_SESSION['accounts'][$row2]->general_username, $i, strlen($_SESSION['accounts'][$row2]->general_username)-$i))) $i--;
						else $mark=true;
					}
				// increase last number with one
				$firstchars = substr($_SESSION['accounts'][$row2]->general_username, 0, $i+1);
				$lastchars = substr($_SESSION['accounts'][$row2]->general_username, $i+1, strlen($_SESSION['accounts'][$row2]->general_username)-$i);
				// Put username together
				$_SESSION['accounts'][$row2]->general_username = $firstchars . (intval($lastchars)+1);
			 	}
			}
			// Show warning if lam has changed username
			if ($_SESSION['accounts'][$row2]->general_username != $_POST['f_general_username']) $errors2[] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
		}
	// Check personal settings
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

// Print header and part of body
echo	'<html><head><title>';
echo _('Create new accounts');
echo '</title>'.
	'<link rel="stylesheet" type="text/css" href="../style/layout.css">'.
	'<meta http-equiv="pragma" content="no-cache">'.
	'<meta http-equiv="cache-control" content="no-cache">'.
	'</head><body>'.
	'<form enctype="multipart/form-data" action="massdetail.php" method="post">';
// Display errir-messages
if (is_array($errors2))
	for ($i=0; $i<sizeof($errors2); $i++) StatusMessage($errors2[$i][0], $errors2[$i][1], $errors2[$i][2]);


switch ($select) {
	/* $select chooses which kind of page should be displayed
	* detail = Show settings which are individuel for every user. These
	*          settings can be changed
	* info = Show all infos about user
	* warn = Show all warning about user
	* error = Show all errors about user
	*/
	case 'error':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'ERROR')
				StatusMessage('ERROR', _('Invalid Value!'), $_SESSION['errors'][$row][$i][2]);
		break;
	case 'info':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'INFO')
				StatusMessage('INFO', _('Check values.'), $_SESSION['errors'][$row][$i][2]);
		break;
	case 'warn':
		for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
			if ($_SESSION['errors'][$row][$i][0] == 'WARN')
				StatusMessage('WARN', _('Check values.'), $_SESSION['errors'][$row][$i][2]);
		break;
	case 'detail':
		echo '<table class="massdetail" width="100%">';
		// Store variabled in $_POST
		echo '<tr><td><input name="type" type="hidden" value="'.$select.'"></td></tr>';
		echo '<tr><td><input name="row" type="hidden" value="'.$row.'"></td></tr>';
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

// Print end of HTML-Page
echo '</table></form></body></html>';
?>
