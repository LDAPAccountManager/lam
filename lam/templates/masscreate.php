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
setlanguage();

$time=time();
if (count($_POST)==0) {
	if (isset($_GET['list2'])) $select = 'list2';
	else if (isset($_GET['create'])) $select='create';
		else $select='main';
	}
else {
	if ($_POST['tolist'] && ($_FILES['userfile']['size']>0)) $select = 'list';
	else if ($_POST['list2']) $select = 'list2';
	else if ($_POST['back']) $select = 'main';
	else if ($_POST['cancel']) $select = 'cancel';
	else if ($_POST['create']) $select = 'create';
	else if ($_POST['pdf']) {
		createUserPDF($_SESSION['accounts']);
		$select='pdf';
		}
	}

if ($select!='pdf') {
	// Write HTML-Header and part of Table
	echo $_SESSION['header'];
	echo '<html><head><title>';
	echo _('Create new Accounts');
	echo '</title>'."\n".
		'<link rel="stylesheet" type="text/css" href="../style/layout.css">'."\n".
		'<meta http-equiv="pragma" content="no-cache">'."\n".
		'<meta http-equiv="cache-control" content="no-cache">'."\n";
	switch ($select) {
		case 'cancel':
			if ( isset($_SESSION['accounts'])) unset($_SESSION['accounts']);
			if ( isset($_SESSION['pointer'])) unset($_SESSION['pointer']);
			if ( isset($_SESSION['errors'])) unset($_SESSION['errors']);
			echo '<meta http-equiv="refresh" content="1; URL=lists/listusers.php">'."\n".
				'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td></td></tr>'."\n";
			break;
		case 'create':
			if ($_SESSION['pointer'] < sizeof($_SESSION['accounts'])) {
				$refresh = get_cfg_var('max_execution_time')-5;
				echo '<meta http-equiv="refresh" content="'.$refresh.'; URL=masscreate.php?create">'."\n";
				}
			echo	'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td></td></tr>'."\n";
			echo	'<tr><td>';
			echo	_('Creating users. Please stand by.');
			echo	'</td></tr>'."\n";
			$stay=true;
			while (($_SESSION['pointer'] < sizeof($_SESSION['accounts'])) && $stay) {
				if ($_SESSION['accounts'][$_SESSION['pointer']]->general_username!='') {
					if (getgid($_SESSION['accounts'][$_SESSION['pointer']]->general_group)==-1) {
						$group = new account();
						$group->general_username=$_SESSION['accounts'][$_SESSION['pointer']]->general_group;
						$group->general_uidNumber=checkid($_SESSION['accounts'][$_SESSION['pointer']], 'group');
						$group->general_gecos=$_SESSION['accounts'][$_SESSION['pointer']]->general_group;
						$group->general_dn=$_SESSION['config']->get_GroupSuffix();
						creategroup($group);
						}
					$_SESSION['accounts'][$_SESSION['pointer']]->general_uidNumber = checkid($_SESSION['accounts'][$_SESSION['pointer']], 'user');
					$iv = base64_decode($_COOKIE["IV"]);
					$key = base64_decode($_COOKIE["Key"]);
					$_SESSION['accounts'][$_SESSION['pointer']]->unix_password = base64_encode(mcrypt_encrypt(
						MCRYPT_RIJNDAEL_256, $key, genpasswd(), MCRYPT_MODE_ECB, $iv));
					$_SESSION['accounts'][$_SESSION['pointer']]->smb_password = $_SESSION['accounts'][$_SESSION['pointer']]->unix_password;
					if ( (time()-$time)<(get_cfg_var('max_execution_time')-10)) {
						$error = createuser($_SESSION['accounts'][$_SESSION['pointer']]);
						if ($error==1) {
							$_SESSION['pointer']++;
							echo '<tr><td>';
							sprintf (_('Created user %s.'), $_SESSION['accounts'][$_SESSION['pointer']]->general_username);
							echo '</td></tr>'."\n";
							}
						else {
							$stay = false;
							StatusMessage('ERROR', _('Could not create user!'), sprintf (_('Was unable to create %s.'), $_SESSION['accounts'][$row]->general_username));
							}
						}
						else $stay=false;
					}
				else $_SESSION['pointer']++;
				}
			if (!$stay) {
				echo '<tr><td><a href="masscreate.php?create">';
				echo _('Please press here if meta-refresh didn\'t work.');
				echo '</a></td></tr>'."\n";
				echo '<tr><td><input name="cancel" type="submit" value="'; echo _('Cancel');
				echo '"></td></tr>';
				}
			else {
				echo '<tr><td>';
				echo _('All Users have been created');
				echo '</td></tr>'."\n".'<tr><td>';
				echo '<tr><td><input name="cancel" type="submit" value="'; echo _('User list'); echo '">';
				echo '</td><td></td><td><input name="pdf" type="submit" value="'; echo _('Create PDF file'); echo '">';
				echo '</td></tr>'."\n";
				if ( isset($_SESSION['pointer'])) unset($_SESSION['pointer']);
				if ( isset($_SESSION['errors'])) unset($_SESSION['errors']);
				}
			break;
		case 'list':
			if (!is_array($accounts)) $accounts = array();
			$groups = array();
			if (loadfile()) {
				echo '<meta http-equiv="refresh" content="2; URL=masscreate.php?list2">'."\n".
					'</head><body>'."\n".
					'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
					'<table class="masscreate" width="100%">'.
					'<tr><td><a href="masscreate.php?list2">';
				echo _('Please press here if meta-refresh didn\'t work.');
				echo "</a></td></tr>\n";
				}
			else {
				//echo '<meta http-equiv="refresh" content="2; URL=masscreate.php?list2">'."\n".
				echo	'</head><body>'."\n".
					'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
					'<table class="masscreate" width="100%">'.
					'<tr><td>';
				echo _('Max 400 users allowed. Ignored additional users.');
				echo '</td></tr>'."\n";
				echo '<tr><td><a href="lists/listusers.php">';
				echo _('Cancel');
				echo '</a></td><td><a href="masscreate.php?list2">';
				echo _('Contiune');
				echo "</a></td></tr>\n";
				}
			break;
		case 'list2':
			echo	'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td></td></tr>'."\n".
				'<tr><td>';
			echo _('Confirm List');
			echo '</td></tr>'."\n";
			for ($i=0; $i<sizeof($groups); $i++)
				if ($_SESSION['accounts'][$i]->general_group!='')
					StatusMessage('INFO', _('Group').' '.
						$_SESSION['accounts'][$i]->general_group.' '._('not found!'), _('It will be created.'));
			echo '<tr><td>'._('row').'</td>'."\n".'<td>'. _('Surname'). '</td>'."\n".'<td>'. _('Given name'). '</td>'."\n".'<td>'. _('User name'). '</td>'."\n".'<td>'. _('Primary group'). '</td>'."\n".'<td>'.
				_('Details'). '</td>'."\n".'<td>' . _('Infos'). '</td>'."\n".'<td>' . _('Warnings'). '</td>'."\n".'<td>' . _('Errors') . '</td>'."\n".'</tr>'."\n";
			if (!isset($_SESSION['rowstart'])) $_SESSION['rowstart'] = 0;
			//if (sizeof($_SESSION['accounts'])<($_SESSION['rowstart']+10)) $end = sizeof($_SESSION['accounts']);
			//	else $end = $_SESSION['rowstart']+10;
			$end = sizeof($_SESSION['accounts']);
			for ($row=0; $row<$end; $row++) { // loops for every row
				echo '<tr><td>'.$row.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_surname.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_givenname.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_username.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_group.'</td>'."\n".'<td>'.
					'<a target=_blank href="massdetail.php?row='.$row.'&type=detail">'._('Show Details.').'</a></td>'."\n".'<td>';
					$found=false;
					for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
						if ($_SESSION['errors'][$row][$i][0] == 'INFO') $found=true;
					if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&type=info">'._('Show Infos.').'</a>';
					echo '</td>'."\n".'<td>';
					$found=false;
					for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
						if ($_SESSION['errors'][$row][$i][0] == 'WARN') $found=true;
					if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&type=warn">'._('Show Warnings.').'</a>';
					echo '</td>'."\n".'<td>';
					$found=false;
					for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
						if ($_SESSION['errors'][$row][$i][0] == 'ERROR') $found=true;
					if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&type=error">'._('Show Errors.').'</a>';
					echo '</td></tr>'."\n";
				}
			$noerrors=true;
			for ($i=0; $i<sizeof($_SESSION['errors']); $i++)
				for ($j=0; $j<sizeof($_SESSION['errors'][$i]); $j++)
					if ($_SESSION['errors'][$i][$j][0] == 'ERROR') $noerrors=false;
			$nowarn=true;
			for ($i=0; $i<sizeof($_SESSION['errors']); $i++)
				for ($j=0; $j<sizeof($_SESSION['errors'][$i]); $j++)
					if ($_SESSION['errors'][$i][$j][0] == 'WARN') $nowarn=false;
			echo '<br>';
			if (!$noerrors) { echo '<tr><td>'. _('There are some errors.') . '</td></tr>'."\n"; }
			if (!$nowarn) { echo '<tr><td>'. _('There are some warnings.') . '</td></tr>'."\n"; }
			echo '</table><br><table class="masscreate" width="100%">'.
				'<tr><td><input name="back" type="submit" value="'; echo _('Back');
			echo '"></td><td><input name="cancel" type="submit" value="'; echo _('Cancel');
			echo '"></td><td><input name="list2" type="submit" value="'; echo _('Refresh'); echo '">';
			if ($noerrors) { echo '</td><td><input name="create" type="submit" value="'; echo _('Create'); echo '">'; }
			echo '</td></tr>'."\n";
			break;
		case 'main':
			if ( isset($_SESSION['accounts'])) unset($_SESSION['accounts']);
			if ( isset($_SESSION['pointer'])) unset($_SESSION['pointer']);
			if ( isset($_SESSION['errors'])) unset($_SESSION['errors']);
			$_SESSION['pointer']=0;
			$profilelist = getUserProfiles();
			echo	'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td></td></tr>'."\n".
				'<tr><td><input name="select" type="hidden" value="main">';
			echo _('Mass Creation');
			echo '</td></tr><tr><td>'."\n";
			echo _('Please provide a csv-file with the following syntax. Values with * are required:');
			echo '</td></tr></table>'.
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td>'."\n";
			echo _('Surname').'*,';
			echo '</td>'."\n".'<td>';
			echo _('Given name').'*,';
			echo '</td>'."\n".'<td>';
			echo _('Username').'*,';
			echo "</td>\n<td>";
			echo _('Primary group').',';
			echo '</td>'."\n".'<td>';
			echo _('Title').',';
			echo '</td>'."\n".'<td>';
			echo _('eMail address').',';
			echo '</td>'."\n".'<td>';
			echo _('Telephone number').',';
			echo '</td></tr>'."\n".'<tr><td>';
			echo _('Mobile number').',';
			echo '</td>'."\n".'<td>';
			echo _('Fax number').',';
			echo '</td>'."\n".'<td>';
			echo _('Street').',';
			echo '</td>'."\n".'<td>';
			echo _('Postal code').',';
			echo '</td>'."\n".'<td>';
			echo _('Postal address').',';
			echo '</td></tr>'."\n".'<tr><td>';
			echo _('Employee type').' &lt;CR&gt;';
			echo '</td></tr></table>'.
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td>'."\n";
			echo _('If Primary group is not given it\'ll used from profile.');
			echo '</td></tr><tr><td>'."\n";
			echo _('If PrimaryGroup does not exist it will be created.');
			echo '</td></tr></table>'.
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td>'."\n";
			echo _('Select Profile:');
			echo '</td><td><select name="f_selectprofile">'."\n";
			foreach ($profilelist as $profile) echo '<option>' . $profile;
			echo '</select>'.
				'</td></tr>'."\n".'<tr><td>';
			echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
			foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_UserSuffix()) as $suffix)
				echo '<option>' . $suffix. '</option>';
			echo '</select></td>'."\n".'<td><a href="help.php?HelpNumber=461" target="lamhelp">'._('Help').'</a>'.
				'</td></tr><tr><td>'."\n".
				'<input type="hidden" name="MAX_FILE_SIZE" value="100000">';
			echo _('Select file:');
			echo '</td><td><input name="userfile" type="file"></td></tr>'."\n".
				'<tr><td><input name="tolist" type="submit" value="'; echo _('Commit'); echo '">'."\n".
				'</td></tr>'."\n";
			break;
		}
	echo '</table></form></body></html>';
	}



function loadfile() {
	if ($_FILES['userfile']['size']>0) {
		$handle = fopen($_FILES['userfile']['tmp_name'], 'r');
		$profile = loadUserProfile($_POST['f_selectprofile']) ;
		for ($row=0; $line_array=fgetcsv($handle,2048); $row++) { // loops for every row
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$_SESSION['accounts'][$row] = $profile;
			$_SESSION['accounts'][$row]->general_dn = $_POST['f_general_suffix'];
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
			$_SESSION['accounts'][$row]->unix_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256,
				$key, genpasswd(), MCRYPT_MODE_ECB, $iv));
			$_SESSION['accounts'][$row]->smb_password=$_SESSION['accounts'][$row]->unix_password;
			}
		}
	for ($row2=0; $row2<sizeof($_SESSION['accounts']); $row2++) { // loops for every row
		// Check for double entries in $_SESSION['accounts']
		if ($row2<401) {
			for ($i=$row2+1; $i<sizeof($_SESSION['accounts']); $i++ ) {
				if ($_SESSION['accounts'][$row2]->general_username == $_SESSION['accounts'][$i]->general_username) { // Found user with same name
					// get last character of username
					if (!is_numeric($_SESSION['accounts'][$i]->general_username{strlen($_SESSION['accounts'][$i]->general_username)-1}))
					$_SESSION['accounts'][$i]->general_username = $_SESSION['accounts'][$i]->general_username . '2';
						else {
						// Get interger-end of string hello456 -> hello + 456
						$start = strlen($_SESSION['accounts'][$i]->general_username)-1;
						while (is_numeric(substr($_SESSION['accounts'][$i]->general_username, $start))) $start--;
						// Increse rusultung number
						$first = substr($_SESSION['accounts'][$i]->general_username, 0, $start+1);
						$second = intval(substr($_SESSION['accounts'][$i]->general_username, $start+1))+1;
						$_SESSION['accounts'][$i]->general_username = $first . $second;
						}
					while ($temp = ldapexists($_SESSION['accounts'][$i], 'user')) {
						// Get interger-end of string hello456 -> hello + 456
						$start = strlen($_SESSION['accounts'][$i]->general_username)-1;
						while (is_numeric(substr($_SESSION['accounts'][$i]->general_username, $start))) $start--;
						// Increse rusultung number
						$first = substr($_SESSION['accounts'][$i]->general_username, 0, $start+1);
						$second = intval(substr($_SESSION['accounts'][$i]->general_username, $start+1))+1;
						$_SESSION['accounts'][$i]->general_username = $first . $second;
						}
					}
				}
			if ($values->general_username != $return->general_username) $error[] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $error);
			// Check if Homedir is valid
			$_SESSION['account'][$row2]->general_homedir = str_replace('$group', $_SESSION['account'][$row2]->general_group, $_SESSION['account'][$row2]->general_homedir);
			if ($_SESSION['account'][$row2]->general_username != '')
				$_SESSION['account'][$row2]->general_homedir = str_replace('$user', $_SESSION['account'][$row2]->general_username, $_SESSION['account'][$row2]->general_homedir);
			if ( !ereg('^[/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*([/]([a-z]|[A-Z])([a-z]|[A-Z]|[0-9]|[.]|[-]|[_])*)*$', $_SESSION['account'][$row2]->general_homedir ))
				$errors[] = array('ERROR', _('Home directory'), _('Homedirectory contains invalid characters.'));
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['account'][$row2]->general_givenname)) $errors[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['account'][$row2]->general_surname)) $errors[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
			if ( ($_SESSION['account'][$row2]->general_gecos=='') || ($_SESSION['account'][$row2]->general_gecos==' ')) {
				$_SESSION['account'][$row2]->general_gecos = $_SESSION['account'][$row2]->general_givenname . " " . $_SESSION['account'][$row2]->general_surname ;
				$errors[] = array('INFO', _('Gecos'), _('Inserted sur- and given name in gecos-field.'));
				}
			if ($_SESSION['account'][$row2]->general_group=='') $errors[] = array('ERROR', _('Primary group'), _('No primary group defined!'));
			// Check if Username contains only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['account'][$row2]->general_username))
				$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, 0-9 and .-_ !'));
			// Check if user already exists
			if (isset($_SESSION['account'][$row2]->general_groupadd) && in_array($_SESSION['account'][$row2]->general_group, $_SESSION['account'][$row2]->general_groupadd)) {
				for ($i=0; $i<count($_SESSION['account'][$row2]->general_groupadd); $i++ )
					if ($_SESSION['account'][$row2]->general_groupadd[$i] == $_SESSION['account'][$row2]->general_group) {
						unset ($_SESSION['account'][$row2]->general_groupadd[$i]);
						$_SESSION['account'][$row2]->general_groupadd = array_values($_SESSION['account'][$row2]->general_groupadd);
						}
				}
			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			while ($temp = ldapexists($_SESSION['account'][$row2], 'user')) {
				// get last character of username
				$lastchar = substr($_SESSION['account'][$row2]->general_username, strlen($_SESSION['account'][$row2]->general_username)-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
					$_SESSION['account'][$row2]->general_username = $_SESSION['account'][$row2]->general_username . '2';
				 else {
				 	$i=strlen($_SESSION['account'][$row2]->general_username)-1;
					$mark = false;
				 	while (!$mark) {
						if (ereg('^([0-9])+$',substr($_SESSION['account'][$row2]->general_username, $i, strlen($_SESSION['account'][$row2]->general_username)-$i))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($_SESSION['account'][$row2]->general_username, 0, $i+1);
					$lastchars = substr($_SESSION['account'][$row2]->general_username, $i+1, strlen($_SESSION['account'][$row2]->general_username)-$i);
					$_SESSION['account'][$row2]->general_username = $firstchars . (intval($lastchars)+1);
				 	}
				}

			// Check if UID is valid. If none value was entered, the next useable value will be inserted
			$_SESSION['account'][$row2]->general_uidNumber = checkid($_SESSION['account'][$row2], 'user');
			if (is_string($_SESSION['account'][$row2]->general_uidNumber)) { // true if checkid has returned an error
				$errors[] = array('ERROR', _('ID-Number'), $_SESSION['account'][$row2]->general_uidNumber);
				unset($_SESSION['account'][$row2]->general_uidNumber);
				}
			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['account'][$row2]->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $_SESSION['account'][$row2]->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $errors);
			if (isset($errors)) unset ($errors);

			if ($_SESSION['account'][$row2]->unix_password != '') {
				$iv = base64_decode($_COOKIE["IV"]);
				$key = base64_decode($_COOKIE["Key"]);
				$password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($_SESSION['account'][$row2]->unix_password), MCRYPT_MODE_ECB, $iv);
				$password = str_replace(chr(00), '', $password);
				}
			if (!ereg('^([a-z]|[A-Z]|[0-9]|[\|]|[\#]|[\*]|[\,]|[\.]|[\;]|[\:]|[\_]|[\-]|[\+]|[\!]|[\%]|[\&]|[\/]|[\?]|[\{]|[\[]|[\(]|[\)]|[\]]|[\}])*$', $password))
				$errors[] = array('ERROR', _('Password'), _('Password contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and #*,.;:_-+!$%&/|?{[()]}= !'));
			if ( !ereg('^([0-9])*$', $_SESSION['account'][$row2]->unix_pwdminage))  $errors[] = array('ERROR', _('Password minage'), _('Password minage must be are natural number.'));
			if ( $_SESSION['account'][$row2]->unix_pwdminage > $_SESSION['account'][$row2]->unix_pwdmaxage ) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must bigger as Password Minage.'));
			if ( !ereg('^([0-9]*)$', $_SESSION['account'][$row2]->unix_pwdmaxage)) $errors[] = array('ERROR', _('Password maxage'), _('Password maxage must be are natural number.'));
			if ( !ereg('^(([-][1])|([0-9]*))$', $_SESSION['account'][$row2]->unix_pwdallowlogin))
				$errors[] = array('ERROR', _('Password Expire'), _('Password expire must be are natural number or -1.'));
			if ( !ereg('^([0-9]*)$', $_SESSION['account'][$row2]->unix_pwdwarn)) $errors[] = array('ERROR', _('Password warn'), _('Password warn must be are natural number.'));
			if ((!$_SESSION['account'][$row2]->unix_host=='') && !ereg('^([a-z]|[A-Z]|[0-9]|[.]|[-])+(([,])+([ ])*([a-z]|[A-Z]|[0-9]|[.]|[-])+)*$', $_SESSION['account']->unix_host))
				$errors[] = array('ERROR', _('Unix workstations'), _('Unix workstations is invalid.'));
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $errors);
			if (isset($errors)) unset ($errors);

			$_SESSION['account'][$row2]->smb_displayName = $_SESSION['account'][$row2]->general_gecos;

			$i=0;
			while ($_SESSION['account'][$row2]->quota[$i][0]) {
				// Check if values are OK and set automatic values. if not error-variable will be set
				if (!ereg('^([0-9])*$', $_SESSION['account'][$row2]->quota[$i][2]))
					$errors[] = array('ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed'));
				if (!ereg('^([0-9])*$', $_SESSION['account'][$row2]->quota[$i][3]))
					$errors[] = array('ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed'));
				if (!ereg('^([0-9])*$', $_SESSION['account'][$row2]->quota[$i][6]))
					$errors[] = array('ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed'));
				if (!ereg('^([0-9])*$', $_SESSION['account'][$row2]->quota[$i][7]))
					$errors[] = array('ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed'));
				$i++;
				}
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $errors);
			if (isset($errors)) unset ($errors);

			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['account'][$row2]->personal_telephoneNumber))  $errors[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['account'][$row2]->personal_mobileTelephoneNumber))  $errors[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['account'][$row2]->personal_facsimileTelephoneNumber))  $errors[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
			if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $_SESSION['account'][$row2]->personal_mail))  $errors[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account'][$row2]->personal_street))  $errors[] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account'][$row2]->personal_postalAddress))  $errors[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account'][$row2]->personal_title))  $errors[] = array('ERROR', _('Title'), _('Please enter a valid title!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['account'][$row2]->personal_employeeType))  $errors[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $_SESSION['account']->personal_postalCode))  $errors[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $errors);
			if (isset($errors)) unset ($errors);

			}
		}
	if ($_FILES['userfile']['size']>0) {
		fclose($handle);
		unlink($_FILES['userfile']['tmp_name']);
		}
	if ($row2>400) return false;
		else return true;
	}


?>
