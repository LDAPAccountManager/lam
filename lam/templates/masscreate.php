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
			echo '</td></tr><tr><td>'."\n";
			echo _('Surname*,Givenname*,Username*,PrimaryGroup,Title,Mail,telephonenumber,');
			echo '</td></tr><tr><td>'."\n";
			echo _('mobileTelephoneNumber,facsimileNumber,street,postalCode,postalAddress,');
			echo '</td></tr><tr><td>'."\n";
			echo _('employeeType. If Primary group is not given it\'ll used from profile.');
			echo '</td></tr><tr><td>'."\n";
			echo _('If PrimaryGroup does not exist it will be created.');
			echo '</td></tr><tr><td>'."\n";
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
			list($values, $error) = checkglobal($_SESSION['accounts'][$row2], 'user'); // account.inc
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $error);
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if ($val) $_SESSION['accounts'][$row2]->$key = $val;
				$_SESSION['accounts'][$row2]->general_uidNumber="";
				}
			$error = checkunix($_SESSION['accounts'][$row2], 'user'); // account.inc
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $error);
			list($values, $error) = checksamba($_SESSION['accounts'][$row2], 'user'); // account.inc
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['accounts'][$row2]->$key = $val;
				}
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $error);
			list($values, $error) = checkquota($_SESSION['accounts'][$row2], 'user'); // account.inc
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['accounts'][$row2]->$key = $val;
				}
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $error);
			list($values, $error) = checkpersonal($_SESSION['accounts'][$row2], 'user'); // account.inc
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
				if ($val) $_SESSION['accounts'][$row2]->$key = $val;
				}
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $error);
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
