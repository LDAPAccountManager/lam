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
	switch ($select) {
		case 'cancel' : // go back to user list page
			metaRefresh($_SESSION['lamurl']."templates/lists/listusers.php");
			die;
			break;
		case 'list' : // refreh to masscreate
			if (!is_array($accounts)) $accounts = array();
			$groups = array();
			if (loadfile()) {
				$_SESSION['group_suffix'] = $_POST['f_group_suffix'];
				$_SESSION['group_selectprofile'] =  $_POST['f_selectgroupprofile'];
				metaRefresh($_SESSION['lamurl']."templates/masscreate.php?list2");
				die;
				}
			else {
				echo $_SESSION['header'];
				echo '<html><head><title>';
				echo _('Create new Accounts');
				echo '</title>'."\n".
					'<link rel="stylesheet" type="text/css" href="'.$_SESSION['lamurl'].'style/layout.css">'."\n".
					'<meta http-equiv="pragma" content="no-cache">'."\n".
					'<meta http-equiv="cache-control" content="no-cache">'."\n";
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
				echo "</a></td></tr></table>\n";
				}
			break;
		}

	// Write HTML-Header and part of Table
	echo $_SESSION['header'];
	echo '<html><head><title>';
	echo _('Create new Accounts');
	echo '</title>'."\n".
		'<link rel="stylesheet" type="text/css" href="'.$_SESSION['lamurl'].'style/layout.css">'."\n".
		'<meta http-equiv="pragma" content="no-cache">'."\n".
		'<meta http-equiv="cache-control" content="no-cache">'."\n";
	switch ($select) {
		case 'create':
			if ($_SESSION['pointer'] < sizeof($_SESSION['accounts'])) {
				$refresh = get_cfg_var('max_execution_time')-5;
				echo '<meta http-equiv="refresh" content="'.$refresh.'; URL=masscreate.php?create">'."\n";
				}
			echo	'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				"<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
				echo _('Creating users. Please stand by ....');
				echo "</b></legend>\n<table border=0 width=\"100%\">\n";
			$stay=true;
			while (($_SESSION['pointer'] < sizeof($_SESSION['accounts'])) && $stay) {
				if ($_SESSION['accounts'][$_SESSION['pointer']]->general_username!='') {

				// Check if Homedir is valid
				$_SESSION['accounts'][$_SESSION['pointer']]->general_homedir = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->general_homedir);
				if ($_SESSION['accounts'][$_SESSION['pointer']]->general_username != '')
					$_SESSION['accounts'][$_SESSION['pointer']]->general_homedir = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->general_homedir);

				// Set uid number
				$_SESSION['accounts'][$_SESSION['pointer']]->general_uidNumber = checkid($_SESSION['accounts'][$_SESSION['pointer']], 'user');

				$_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath);
				$_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath);

				$_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath);
				$_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath);

				$_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome);
				$_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome);


					if (getgid($_SESSION['accounts'][$_SESSION['pointer']]->general_group)==-1) {
						$group = LoadGroupProfile($_SESSION['group_selectprofile']);

						$group->type = 'group';
						// load quotas from profile and check if they are valid
						$values = getquotas('group');
						if (isset($group->quota[0])) { // check quotas from profile
							$i=0;
							// check quota settings
							while (isset($group->quota[$i])) {
								$found = (-1);
								for ($j=0; $j<count($values->quota); $j++)
									if ($values->quota[$j][0]==$group->quota[$i][0]) $found = $j;
								if ($found==-1) unset($group->quota[$i]);
								else {
									$group->quota[$i][1] = $values->quota[$found][1];
									$group->quota[$i][5] = $values->quota[$found][5];
									$group->quota[$i][4] = $values->quota[$found][4];
									$group->quota[$i][8] = $values->quota[$found][8];
									$i++;
									}
								}
							$group->quota = array_values($group->quota);
							}
						else { // No quotas saved in profile
							if (is_object($values)) {
								while (list($key, $val) = each($values)) // Set only defined values
								if (isset($val)) $group->$key = $val;
								}
							}

						$group->general_username=$_SESSION['accounts'][$_SESSION['pointer']]->general_group;
						$group->general_uidNumber=checkid($_SESSION['accounts'][$_SESSION['pointer']], 'group');
						$group->general_gecos=$_SESSION['accounts'][$_SESSION['pointer']]->general_group;
						$group->general_dn=$_SESSION['group_suffix'];
						$error = creategroup($group);
						if ($error==1) {
							$_SESSION['pointer']++;
							echo '<tr><td>';
							sprintf (_('Created group %s.'), $_SESSION['accounts'][$_SESSION['pointer']]->general_group);
							echo '</td></tr>'."\n";
							}
						else {
							$stay = false;
							StatusMessage('ERROR', _('Could not create group!'), sprintf (_('Was unable to create %s.'), $_SESSION['accounts'][$row]->general_group));
							}
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
				echo '"></td></tr></table>';
				echo "</fieldset>\n";
				}
			else {
				echo '<tr><td>';
				echo _('All Users have been created');
				echo '</td></tr>'."\n".'<tr><td>';
				echo '<tr><td><input name="cancel" type="submit" value="'; echo _('User list'); echo '">';
				echo '</td><td></td><td><input name="pdf" type="submit" value="'; echo _('Create PDF file'); echo '">';
				echo '</td></tr></table>'."\n</fieldset>\n";
				if ( isset($_SESSION['pointer'])) unset($_SESSION['pointer']);
				if ( isset($_SESSION['errors'])) unset($_SESSION['errors']);
				if ( isset($_SESSION['group_suffix'])) unset($_SESSION['group_suffix']);
				if ( isset($_SESSION['group_selectprofile'])) unset($_SESSION['group_selectprofile']);
				}
			break;
		case 'list2':
			echo	'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table border=0 width="100%">';
				for ($i=0; $i<sizeof($groups); $i++)
					if ($_SESSION['accounts'][$i]->general_group!='')
						StatusMessage('INFO', _('Group').' '.
							$_SESSION['accounts'][$i]->general_group.' '._('not found!'), _('It will be created.'));
			echo "</table>\n";
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Confirm List');
			echo "</b></legend>\n<table border=0 width=\"100%\">\n";
			echo '<tr><td>'._('row').'</td>'."\n".'<td>'. _('Surname'). '</td>'."\n".'<td>'. _('Given name'). '</td>'."\n".'<td>'. _('User name'). '</td>'."\n".'<td>'. _('Primary group'). '</td>'."\n".'<td>'.
				_('Details'). '</td>'."\n".'<td>' . _('Infos'). '</td>'."\n".'<td>' . _('Warnings'). '</td>'."\n".'<td>' . _('Errors') . '</td>'."\n".'</tr>'."\n";
			$end = sizeof($_SESSION['accounts']);
			for ($row=0; $row<$end; $row++) { // loops for every row
				echo '<tr><td>'.$row.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_surname.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_givenname.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_username.'</td>'."\n".'<td>'.
					$_SESSION['accounts'][$row]->general_group.'</td>'."\n".'<td>'.
					'<a target=_blank href="massdetail.php?row='.$row.'&amp;type=detail">'._('Show Details.').'</a></td>'."\n".'<td>';
					$found=false;
					for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
						if ($_SESSION['errors'][$row][$i][0] == 'INFO') $found=true;
					if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&amp;type=info">'._('Show Infos.').'</a>';
					echo '</td>'."\n".'<td>';
					$found=false;
					for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
						if ($_SESSION['errors'][$row][$i][0] == 'WARN') $found=true;
					if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&amp;type=warn">'._('Show Warnings.').'</a>';
					echo '</td>'."\n".'<td>';
					$found=false;
					for ($i=0; $i<sizeof($_SESSION['errors'][$row]); $i++)
						if ($_SESSION['errors'][$row][$i][0] == 'ERROR') $found=true;
					if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&amp;type=error">'._('Show Errors.').'</a>';
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
			echo '</table></fieldset>';
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Please select page');
			echo "</b></legend>\n<table border=0 width=\"100%\">\n".
				'<tr><td><input name="back" type="submit" value="'; echo _('Back');
			echo '"></td><td><input name="cancel" type="submit" value="'; echo _('Cancel');
			echo '"></td><td><input name="list2" type="submit" value="'; echo _('Refresh'); echo '">';
			if ($noerrors) { echo '</td><td><input name="create" type="submit" value="'; echo _('Create'); echo '">'; }
			echo '</td></tr>'."\n"."</table>\n</fieldset>";
			break;
		case 'main':
			if ( isset($_SESSION['accounts'])) unset($_SESSION['accounts']);
			if ( isset($_SESSION['pointer'])) unset($_SESSION['pointer']);
			if ( isset($_SESSION['errors'])) unset($_SESSION['errors']);
			if ( isset($_SESSION['group_suffix'])) unset($_SESSION['group_suffix']);
			if ( isset($_SESSION['group_selectprofile'])) unset($_SESSION['group_selectprofile']);
			$_SESSION['pointer']=0;
			echo	'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				"<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Mass Creation');
			echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
			echo _('Please provide a csv-file with the following syntax. Values with * are required:');
			echo '</td></tr></table>'.
				'<table class="masscreate" width="100%" border=1>'.
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
			echo '</td>'."\n".'<td>';
			echo _('Employee type');
			echo '</td><td>&lt;CR&gt;';
			echo '</td></tr></table>';
			echo "<br>";
			echo _('If Primary group is not given it\'ll used from profile.');
			echo "<br>";
			echo _('If Primary group does not exist it will be created.');
			echo "</fieldset>\n";
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Select settings');
			echo "</b></legend>\n<table class=\"masscreate\" width=\"100%\">".
				'<tr><td>'."\n";
			echo _('Select user profile:');
			echo '</td><td><select name="f_selectprofile">'."\n";
			foreach (getUserProfiles() as $profile) echo '<option>' . $profile;
			echo '</select>';
			echo "</td>\n<td><a href=\"help.php?HelpNumber=421\" target=\"lamhelp\">";
			echo _('Help')."</a></td>\n</tr>\n<tr><td>";
			echo _('User suffix'); echo '</td><td><select name="f_general_suffix">';
			foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_UserSuffix()) as $suffix)
				echo '<option>' . $suffix. '</option>';
			echo '</select></td>'."\n".'<td><a href="help.php?HelpNumber=461" target="lamhelp">'._('Help').'</a>'.
				'</td></tr><tr><td>'."\n";
			echo _("Expand suffix with primary groupname");
			echo '</td>'."\n".'<td><input name="f_ou_expand" type="checkbox">';
			echo "</td>\n<td><a href=\"help.php?HelpNumber=422\" target=\"lamhelp\">";
			echo _('Help')."</a></td>\n</tr>\n<tr><td>";
			echo _('Group suffix'); echo '</td><td><select name="f_group_suffix">';
			foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_GroupSuffix()) as $suffix)
				echo '<option>' . $suffix. '</option>';
			echo '</select></td>'."\n".'<td><a href="help.php?HelpNumber=423" target="lamhelp">'._('Help').'</a>'.
				'</td></tr><tr><td>'."\n";
			echo _('Select group profile:');
			echo '</td><td><select name="f_selectgroupprofile">'."\n";
			foreach (getGroupProfiles() as $profile) echo '<option>' . $profile;
			echo '</select>';
			echo "</td>\n<td><a href=\"help.php?HelpNumber=458\" target=\"lamhelp\">";
			echo _('Help')."</a></td>\n</tr>\n<tr><td>";
			echo '<input type="hidden" name="MAX_FILE_SIZE" value="100000">';
			echo _('Select file:');
			echo '</td><td><input name="userfile" type="file"></td></tr>'."\n".
				'<tr><td></td><td><input name="tolist" type="submit" value="'; echo _('Next'); echo '">'."\n".
				'</td><td></td></tr>'."\n"."</table>\n</fieldset>\n";
			break;
		}
	echo '</form></body></html>';
	}



function loadfile() {
	if ($_FILES['userfile']['size']>0) {
		$OUs = array();
		$handle = fopen($_FILES['userfile']['tmp_name'], 'r');
		$profile = loadUserProfile($_POST['f_selectprofile']) ;
		$profile->type = 'user';
		$profile->smb_flagsW = 0;

		// load quotas from profile and check if they are valid
		$values = getquotas('user');
		if (isset($profile->quota[0])) { // check quotas from profile
			$i=0;
			// check quota settings
			while (isset($profile->quota[$i])) {
				$found = (-1);
				for ($j=0; $j<count($values->quota); $j++)
					if ($values->quota[$j][0]==$profile->quota[$i][0]) $found = $j;
				if ($found==-1) unset($profile->quota[$i]);
				else {
					$profile->quota[$i][1] = $values->quota[$found][1];
					$profile->quota[$i][5] = $values->quota[$found][5];
					$profile->quota[$i][4] = $values->quota[$found][4];
					$profile->quota[$i][8] = $values->quota[$found][8];
					$i++;
					}
				}
			$profile->quota = array_values($profile->quota);
			}
		else { // No quotas saved in profile
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $profile->$key = $val;
				}
			}

		for ($row=0; $line_array=fgetcsv($handle,2048); $row++) { // loops for every row
			$iv = base64_decode($_COOKIE["IV"]);
			$key = base64_decode($_COOKIE["Key"]);
			$_SESSION['accounts'][$row] = $profile;
			$_SESSION['accounts'][$row]->type = 'user';
			if (isset($line_array[0])) $_SESSION['accounts'][$row]->general_surname = $line_array[0];
			if (isset($line_array[1])) $_SESSION['accounts'][$row]->general_givenname = $line_array[1];
			if (isset($line_array[2])) $_SESSION['accounts'][$row]->general_username = $line_array[2];
			if (isset($line_array[3])) $_SESSION['accounts'][$row]->general_group = $line_array[3];
			if (isset($line_array[4])) $_SESSION['accounts'][$row]->personal_title = $line_array[4];
			if (isset($line_array[5])) $_SESSION['accounts'][$row]->personal_mail = $line_array[5];
			if (isset($line_array[6])) $_SESSION['accounts'][$row]->personal_telephoneNumber = $line_array[6];
			if (isset($line_array[7])) $_SESSION['accounts'][$row]->personal_mobileTelephoneNumber = $line_array[7];
			if (isset($line_array[8])) $_SESSION['accounts'][$row]->personal_facsimileTelephoneNumber = $line_array[8];
			if (isset($line_array[9])) $_SESSION['accounts'][$row]->personal_street = $line_array[9];
			if (isset($line_array[10])) $_SESSION['accounts'][$row]->personal_postalCode = $line_array[10];
			if (isset($line_array[11])) $_SESSION['accounts'][$row]->personal_postalAddress = $line_array[11];
			if (isset($line_array[12])) $_SESSION['accounts'][$row]->personal_employeeType = $line_array[12];

			if ($_POST['f_ou_expand']) {
				$_SESSION['accounts'][$row]->general_dn = "ou=".$_SESSION['accounts'][$row]->general_group .','. $_POST['f_general_suffix'];
				// Create OUs if needed
				if (!in_array($_SESSION['accounts'][$row]->general_group, $OUs)) {
					$attr['objectClass']= 'organizationalUnit';
					$attr['ou'] = $_SESSION['accounts'][$row]->general_group;
					$success = @ldap_add($_SESSION['ldap']->server(), $_SESSION['accounts'][$row]->general_dn, $attr);
					if ($success) $OUs[] = $_SESSION['accounts'][$row]->general_group;
					}
				}
			else $_SESSION['accounts'][$row]->general_dn = $_POST['f_general_suffix'];
			$_SESSION['accounts'][$row]->unix_password = base64_encode(mcrypt_encrypt(MCRYPT_RIJNDAEL_256,
				$key, genpasswd(), MCRYPT_MODE_ECB, $iv));
			$_SESSION['accounts'][$row]->smb_password=$_SESSION['accounts'][$row]->unix_password;

			}
		}

	// check if account allready exists
	for ($i=0; $i<sizeof($_SESSION['accounts']); $i++) { // loops for every row
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
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['accounts'][$row2]->general_givenname)) $errors[] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['accounts'][$row2]->general_surname)) $errors[] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
			if ( ($_SESSION['accounts'][$row2]->general_gecos=='') || ($_SESSION['accounts'][$row2]->general_gecos==' ')) {
				$_SESSION['accounts'][$row2]->general_gecos = $_SESSION['accounts'][$row2]->general_givenname . " " . $_SESSION['accounts'][$row2]->general_surname ;
				$errors[] = array('INFO', _('Gecos'), _('Inserted sur- and given name in gecos-field.'));
				}
			if ($_SESSION['accounts'][$row2]->general_group=='') $errors[] = array('ERROR', _('Primary group'), _('No primary group defined!'));
			// Check if Username contains only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['accounts'][$row2]->general_username))
				$errors[] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, 0-9 and .-_ !'));

			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			while ($temp = ldapexists($_SESSION['accounts'][$row2], 'user')) {
				// get last character of username
				$lastchar = substr($_SESSION['accounts'][$row2]->general_username, strlen($_SESSION['accounts'][$row2]->general_username)-1, 1);
				// Last character is no number
				if ( !ereg('^([0-9])+$', $lastchar))
					$_SESSION['accounts'][$row2]->general_username = $_SESSION['accounts'][$row2]->general_username . '2';
				 else {
				 	$i=strlen($_SESSION['accounts'][$row2]->general_username)-1;
					$mark = false;
				 	while (!$mark) {
						if (ereg('^([0-9])+$',substr($_SESSION['accounts'][$row2]->general_username, $i, strlen($_SESSION['accounts'][$row2]->general_username)-$i))) $i--;
							else $mark=true;
						}
					// increase last number with one
					$firstchars = substr($_SESSION['accounts'][$row2]->general_username, 0, $i+1);
					$lastchars = substr($_SESSION['accounts'][$row2]->general_username, $i+1, strlen($_SESSION['accounts'][$row2]->general_username)-$i);
					$_SESSION['accounts'][$row2]->general_username = $firstchars . (intval($lastchars)+1);
					}
				}

			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['accounts'][$row2]->general_username)) $errors[] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $_SESSION['accounts'][$row2]->general_username))
				$errors[] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter'));
			$_SESSION['errors'][$row2] = array_merge($_SESSION['errors'][$row2], $errors);
			if (isset($errors)) unset ($errors);

			$_SESSION['accounts'][$row2]->smb_displayName = $_SESSION['accounts'][$row2]->general_gecos;

			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['accounts'][$row2]->personal_telephoneNumber))  $errors[] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['accounts'][$row2]->personal_mobileTelephoneNumber))  $errors[] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['accounts'][$row2]->personal_facsimileTelephoneNumber))  $errors[] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
			if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $_SESSION['accounts'][$row2]->personal_mail))  $errors[] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_street))  $errors[] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_postalAddress))  $errors[] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_title))  $errors[] = array('ERROR', _('Title'), _('Please enter a valid title!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[Ä]|[ä]|[Ö]|[ö]|[Ü]|[ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_employeeType))  $errors[] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $_SESSION['accounts']->personal_postalCode))  $errors[] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
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
