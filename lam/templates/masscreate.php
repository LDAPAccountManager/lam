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

/* We have to include all modules
* before start session
* *** fixme I would prefer loading them dynamic but
* i don't know how to to this
*/
$dir = opendir('../lib/modules');
while ($entry = readdir($dir))
	if (is_file('../lib/modules/'.$entry)) include_once ('../lib/modules/'.$entry);

// Start session
session_save_path('../sess');
@session_start();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	metaRefresh("login.php");
	exit;
	}

// Set correct language, codepages, ....
setlanguage();

if (!isset($_SESSION['cache'])) {
	$_SESSION['cache'] = new cache();
	}

/* Save current time in $time. We need $time to check out how
* long masscreate.php is running. To avoid max. execution time
* set in php.ini masscreate.php will create a redirect to
* itself.
*/
$time=time();
/* Startcondition massdetail.php was called from outside or
* from masscreate.php itself via meta refresh
*/
if (count($_POST)==0) {
	// Register new account_container
	$_SESSION['account'] = new accountContainer('user', 'account');
	// load profile

	// Find out list of attribtues which must be set put not allready covered by profile

	// Print first HTML-Page
	echo $_SESSION['header'];
	echo "<title>" . _('Create new Accounts') . "</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";

	}
else {
	/* Check loaded attributed in $_SESSION['accounts'] if file was loaded and
	* filesize is bigger as 0.
	*/
	if ($_POST['tolist'] && ($_FILES['userfile']['size']>0)) $select = 'list';
	// Go the corresponding page if button was pressed
	else if ($_POST['list2']) $select = 'list2';
	else if ($_POST['back']) $select = 'main';
	else if ($_POST['cancel']) $select = 'cancel';
	else if ($_POST['create']) $select = 'create';
	else if ($_POST['pdf']) {
		// Create PDF-File
		createUserPDF($_SESSION['accounts']);
		// Stop script
		die;
		}
	}

switch ($select) {
	/* Select which part of page should be loaded
	* cacnel = Go back to listusers.php
	* list = Load csv-file. Refresh to list2
	*/
	case 'cancel' :
		// go back to user list page
		metaRefresh("lists/listusers.php");
		// Stop script
		die;
		break;
	case 'list' :
		if (loadfile()) {
			// Do Refresh to masscreate.php itself if csv-file was loaded successfully
			$_SESSION['group_suffix'] = $_POST['f_group_suffix'];
			$_SESSION['group_selectprofile'] =  $_POST['f_selectgroupprofile'];
			metaRefresh("masscreate.php?list2=true");
			// Stop script
			die;
			}
		else {
			/* Loadfile has returned an error because masscreate.php can only
			* handle max 400 new users.
			* lam will show an error-page with a notice everything after line
			* 400 in csv-file will be ignored
			*/
			echo $_SESSION['header'];
			echo '<title>';
			echo _('Create new Accounts');
			echo '</title>'."\n".
				'<link rel="stylesheet" type="text/css" href="../style/layout.css">'."\n".
				'<meta http-equiv="pragma" content="no-cache">'."\n".
				'<meta http-equiv="cache-control" content="no-cache">'."\n".
				'</head><body>'."\n".
				'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
				'<table class="masscreate" width="100%">'.
				'<tr><td>';
			echo _('Max 400 users allowed. Ignored additional users.');
			echo '</td></tr>'."\n";
			echo '<tr><td><a href="lists/listusers.php">';
			echo _('Cancel');
			echo '</a></td><td><a href="masscreate.php?list2=true">';
			echo _('Contiune');
			echo "</a></td></tr></table>\n";
			// Stop script
			die;
			}
		break;
	}

// Write HTML-Header
echo $_SESSION['header'];
echo '<title>';
echo _('Create new Accounts');
echo '</title>'."\n".
	'<link rel="stylesheet" type="text/css" href="../style/layout.css">'."\n".
	'<meta http-equiv="pragma" content="no-cache">'."\n".
	'<meta http-equiv="cache-control" content="no-cache">'."\n";


switch ($select) {
	/* Select which part of page should be loaded
	* create = Create new users
	* list2 = Show page with all users who should be created.
	* main = Show startpegae where settings and file can be selected
	*/
	case 'create':
		/* Set Metarefresh to max_execution_time - 5sec
		* 5 sec. should be enough to create the current
		* user
		*/
		if ($_SESSION['pointer'] < sizeof($_SESSION['accounts'])) {
			$refresh = get_cfg_var('max_execution_time')-5;
			echo '<meta http-equiv="refresh" content="'.$refresh.'; URL=masscreate.php?create=true">'."\n";
			}
		// Display start of body
		echo	'</head><body>'."\n".
			'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
			"<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Creating users. Please stand by ....');
			echo "</b></legend>\n<table border=0 width=\"100%\">\n";
		// Keys needed to encrypt passwords from session
		$stay=true;
		// Stay in loop as long there are still users to create and no error did ocour
		while (($_SESSION['pointer'] < sizeof($_SESSION['accounts'])) && $stay) {
			if (getgid($_SESSION['accounts'][$_SESSION['pointer']]->general_group)==-1) {
					// Create group if it doesn't exist yet
					$group = LoadGroupProfile($_SESSION['group_selectprofile']);
					$group->type = 'group';
					// load quotas from profile and check if they are valid
					if ($config_intern->scriptServer) {
						// load quotas and check if quotas from profile are valid
						$quotas = getquotas(array($group));
						for ($i=0; $i<count($group->quota); $i++) $profile_quotas[] = $group->quota[$i][0];
						for ($i=0; $i<count($quotas[0]->quota); $i++) {
							$real_quotas[] = $quotas[0]->quota[$i][0];
							if (is_array($profile_quotas)) {
								if (!in_array($quotas[0]->quota[$i][0], $profile_quotas)) $group->quota[]=$quotas[0]->quota[$i];
								}
							else $group->quota[]=$quotas[0]->quota[$i];
							}
						$j=0;
						// delete not existing quotas
						while (isset($group->quota[$j][0])) {
							// remove invalid quotas
							if (!in_array($group->quota[$j][0], $real_quotas)) unset($group->quota[$j]);
								else $j++;
							}
						// Beautify array, repair index
						$group->quota = array_values($group->quota);
						}
					// Get groupname from current user
					$group->general_username=$_SESSION['accounts'][$_SESSION['pointer']]->general_group;
					// gid Number
					$temp = explode(':', checkid($group));
					$group->general_uidNumber = $temp[0];
					// Set Gecos to groupname
					$group->general_gecos=$_SESSION['accounts'][$_SESSION['pointer']]->general_group;
					// Set DN
					$group->general_dn=$_SESSION['group_suffix'];
					// Create group
					$error = creategroup($group);
					// Show success or failure-message about group creation
					if ($error==1) {
						echo '<tr><td>';
						sprintf (_('Created group %s.'), $_SESSION['accounts'][$_SESSION['pointer']]->general_group);
						echo '</td></tr>'."\n";
						}
					else {
						$stay = false;
						StatusMessage('ERROR', _('Could not create group!'), sprintf (_('Was unable to create %s.'), $_SESSION['accounts'][$row]->general_group));
						}
					}
			// Check if Homedir is valid
			$_SESSION['accounts'][$_SESSION['pointer']]->general_homedir = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->general_homedir);
			if ($_SESSION['accounts'][$_SESSION['pointer']]->general_username != '')
				$_SESSION['accounts'][$_SESSION['pointer']]->general_homedir = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->general_homedir);
			// Set uid number
			$temp = explode(':', checkid($_SESSION['accounts'][$_SESSION['pointer']]));
			$_SESSION['accounts'][$_SESSION['pointer']]->general_uidNumber = $temp[0];
			$_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath);
			$_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->smb_scriptPath);
			$_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath);
			$_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->smb_profilePath);
			$_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome = str_replace('$user', $_SESSION['accounts'][$_SESSION['pointer']]->general_username, $_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome);
			$_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome = str_replace('$group', $_SESSION['accounts'][$_SESSION['pointer']]->general_group, $_SESSION['accounts'][$_SESSION['pointer']]->smb_smbhome);
			$_SESSION['accounts'][$_SESSION['pointer']]->unix_password = base64_encode($_SESSION['ldap']->encrypt(genpasswd()));
			$_SESSION['accounts'][$_SESSION['pointer']]->smb_password = $_SESSION['accounts'][$_SESSION['pointer']]->unix_password;
				// Only create user if we have at least 5sec time to create the user
			if ( (time()-$time)<(get_cfg_var('max_execution_time')-10)) {
				$error = createuser($_SESSION['accounts'][$_SESSION['pointer']], false);
					// Show error or success message
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
				// End loop if we don't have enough time to create user
			else $stay=false;
			}
		if (!$stay) {
			// Display rest of meta-refreh page if there are still users to create
			echo '<tr><td><a href="masscreate.php?create=true">';
			echo _('Click here if you are not directed to the next page.');
			echo '</a></td></tr>'."\n";
			echo '<tr><td><input name="cancel" type="submit" value="'; echo _('Cancel');
			echo '"></td></tr></table>';
			echo "</fieldset>\n";
			}
		else {
			// Write homedirs and quotas if needed
			if ($_SESSION['config']->scriptServer) {
				setquotas ($_SESSION['accounts']);
				// Get array with new usernames
				foreach ($_SESSION['accounts'] as $account) $users[] = $account->general_username;
				addhomedir($users);
				}
			// Show success-page
			echo '<tr><td>';
			echo _('All Users have been created');
			echo '</td></tr>'."\n".'<tr><td>';
			echo '<tr><td><input name="cancel" type="submit" value="'; echo _('User list'); echo '">';
			echo '</td><td></td><td><input name="pdf" type="submit" value="'; echo _('Create PDF file'); echo '">';
			echo '</td></tr></table>'."\n</fieldset>\n";
			// unset variables
			if ( isset($_SESSION['pointer'])) unset($_SESSION['pointer']);
			if ( isset($_SESSION['mass_errors'])) unset($_SESSION['mass_errors']);
			if ( isset($_SESSION['group_suffix'])) unset($_SESSION['group_suffix']);
			if ( isset($_SESSION['group_selectprofile'])) unset($_SESSION['group_selectprofile']);
			}
		break;
	case 'list2':
		// Show table with all users
		echo	'</head><body>'."\n".
			'<form enctype="multipart/form-data" action="masscreate.php" method="post">'."\n".
			'<table border=0 width="100%">';
		for ($i=0; $i<sizeof($groups); $i++)
			if ($_SESSION['accounts'][$i]->general_group!='')
				StatusMessage('INFO', _('Group').' '. $_SESSION['accounts'][$i]->general_group.' '._('not found!'), _('It will be created.'));
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
			// Show infos
			for ($i=0; $i<sizeof($_SESSION['mass_errors'][$row]); $i++)
				if ($_SESSION['mass_errors'][$row][$i][0] == 'INFO') $found=true;
			if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&amp;type=info">'._('Show Infos.').'</a>';
			echo '</td>'."\n".'<td>';
			$found=false;
			// Show warnings
			for ($i=0; $i<sizeof($_SESSION['mass_errors'][$row]); $i++)
				if ($_SESSION['mass_errors'][$row][$i][0] == 'WARN') $found=true;
			if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&amp;type=warn">'._('Show Warnings.').'</a>';
			echo '</td>'."\n".'<td>';
			$found=false;
			// Show errors
			for ($i=0; $i<sizeof($_SESSION['mass_errors'][$row]); $i++)
				if ($_SESSION['mass_errors'][$row][$i][0] == 'ERROR') $found=true;
			if ($found) echo '<a target="massdetail" href="massdetail.php?row='.$row.'&amp;type=error">'._('Show Errors.').'</a>';
			echo '</td></tr>'."\n";
			}
		$noerrors=true;
		for ($i=0; $i<sizeof($_SESSION['mass_errors']); $i++)
			for ($j=0; $j<sizeof($_SESSION['mass_errors'][$i]); $j++)
				if ($_SESSION['mass_errors'][$i][$j][0] == 'ERROR') $noerrors=false;
		$nowarn=true;
		for ($i=0; $i<sizeof($_SESSION['mass_errors']); $i++)
			for ($j=0; $j<sizeof($_SESSION['mass_errors'][$i]); $j++)
				if ($_SESSION['mass_errors'][$i][$j][0] == 'WARN') $nowarn=false;
		echo '<br>';
		if (!$noerrors) { echo '<tr><td>'. _('There are some errors.') . '</td></tr>'."\n"; }
		if (!$nowarn) { echo '<tr><td>'. _('There are some warnings.') . '</td></tr>'."\n"; }
		echo '</table></fieldset>';
		echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
		echo _('Please select page:');
		echo "</b></legend>\n<table border=0 width=\"100%\">\n".
			'<tr><td><input name="back" type="submit" value="'; echo _('Back');
		echo '"></td><td><input name="cancel" type="submit" value="'; echo _('Cancel');
		echo '"></td><td><input name="list2" type="submit" value="'; echo _('Refresh'); echo '">';
		if ($noerrors) { echo '</td><td><input name="create" type="submit" value="'; echo _('Create'); echo '">'; }
		echo '</td></tr>'."\n"."</table>\n</fieldset>";
		break;
	case 'main':
		// Unset old variables
		if ( isset($_SESSION['accounts'])) unset($_SESSION['accounts']);
		if ( isset($_SESSION['pointer'])) unset($_SESSION['pointer']);
		if ( isset($_SESSION['mass_errors'])) unset($_SESSION['mass_errors']);
		if ( isset($_SESSION['group_suffix'])) unset($_SESSION['group_suffix']);
		if ( isset($_SESSION['group_selectprofile'])) unset($_SESSION['group_selectprofile']);
		// Set pointer to 0, first user
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
		// Show list with all user profiles
		foreach (getUserProfiles() as $profile) echo '<option>' . $profile;
		echo '</select>';
		echo "</td>\n<td><a href=\"help.php?HelpNumber=421\" target=\"lamhelp\">";
		echo _('Help')."</a></td>\n</tr>\n<tr><td>";
		echo _('User suffix'); echo '</td><td><select name="f_general_suffix">';
		// Show list with all user suffixes
		foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_UserSuffix()) as $suffix)
			echo '<option>' . $suffix. '</option>';
		echo '</select></td>'."\n".'<td><a href="help.php?HelpNumber=461" target="lamhelp">'._('Help').'</a>'.
			'</td></tr><tr><td>'."\n";
		echo _("Expand suffix with primary groupname");
		echo '</td>'."\n".'<td><input name="f_ou_expand" type="checkbox">';
		echo "</td>\n<td><a href=\"help.php?HelpNumber=422\" target=\"lamhelp\">";
		echo _('Help')."</a></td>\n</tr>\n<tr><td>";
		echo _('Group suffix'); echo '</td><td><select name="f_group_suffix">';
		// Show list with all group suffixes
		foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_GroupSuffix()) as $suffix)
			echo '<option>' . $suffix. '</option>';
		echo '</select></td>'."\n".'<td><a href="help.php?HelpNumber=423" target="lamhelp">'._('Help').'</a>'.
			'</td></tr><tr><td>'."\n";
		echo _('Select group profile');
		echo '</td><td><select name="f_selectgroupprofile">'."\n";
		// Show list with group profiles
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


/* Whis function will load a csv-file and
* load all attributes into $_SESSION['accounts'][$row] which
* is an array of account objects
* The csv file is using the following syntax:
*/
function loadfile() {
	if ($_FILES['userfile']['size']>0) {
		// Array with all OUs from users
		$OUs = $_SESSION['ldap']->search_units($_SESSION['config']->get_UserSuffix());
		// fixme **** load all existing OUs in Array
		// open csv-file
		$handle = fopen($_FILES['userfile']['tmp_name'], 'r');
		// Load profile which should be used for all users
		$profile = loadUserProfile($_POST['f_selectprofile']) ;
		// Set type to user
		$profile->type = 'user';
		if ($config_intern->scriptServer) {
			// load quotas and check if quotas from profile are valid
			$quotas = getquotas(array($profile));
			for ($i=0; $i<count($profile->quota); $i++) $profile_quotas[] = $profile->quota[$i][0];
			for ($i=0; $i<count($quotas[0]->quota); $i++) {
				$real_quotas[] = $quotas[0]->quota[$i][0];
				if (is_array($profile_quotas)) {
					if (!in_array($quotas[0]->quota[$i][0], $profile_quotas)) $profile->quota[]=$quotas[0]->quota[$i];
					}
				else $profile->quota[]=$quotas[0]->quota[$i];
				}
			$j=0;
			// delete not existing quotas
			while (isset($profile->quota[$j][0])) {
				// remove invalid quotas
				if (!in_array($profile->quota[$j][0], $real_quotas)) unset($profile->quota[$j]);
					else $j++;
				}
			// Beautify array, repair index
			$profile->quota = array_values($profile->quota);
			}
		// Get keys to en/decrypt passwords
		for ($row=0; $line_array=fgetcsv($handle,2048); $row++) {
			 // loops for every row
			// Set corrent user to profile
			$_SESSION['accounts'][$row] = $profile;
			// Load values from file into array
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
				// Expand DN of user with ou=$group
				$_SESSION['accounts'][$row]->general_dn = "ou=".$_SESSION['accounts'][$row]->general_group .','. $_POST['f_general_suffix'];
				// Create OUs if needed
				if (!in_array("ou=".$_SESSION['accounts'][$row]->general_group.",".$_POST['f_general_suffix'], $OUs)) {
					$attr['objectClass']= 'organizationalUnit';
					$attr['ou'] = $_SESSION['accounts'][$row]->general_group;
					$success = ldap_add($_SESSION['ldap']->server(), $_SESSION['accounts'][$row]->general_dn, $attr);
					if ($success) $OUs[] = "ou=".$_SESSION['accounts'][$row]->general_group.",".$_POST['f_general_suffix'];
					}
				}
			// Set DN without uid=$username
			else $_SESSION['accounts'][$row]->general_dn = $_POST['f_general_suffix'];
			// Create Random Password
			$_SESSION['accounts'][$row]->unix_password = base64_encode($_SESSION['ldap']->encrypt(genpasswd()));
			$_SESSION['accounts'][$row]->smb_password=$_SESSION['accounts'][$row]->unix_password;
			}
		}
	// Validate cache-array
	ldapreload('user');
	// Get List with all existing usernames
	foreach ($_SESSION['userDN'] as $user_array) $users[] = $user_array['cn'];
	for ($row2=0; $row2<sizeof($_SESSION['accounts']); $row2++) {
		/* loops for every user
		* Check for double entries in $_SESSION['accounts']
		* Stop Execution after line 400 because max executiontime would be to close
		*/
		if ($row2<401) {
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
			// Add uername to array so it's not used again for another user in masscreate
			$users[] = $_SESSION['accounts'][$row2]->general_username;
			if ($_SESSION['accounts'][$row2]->general_username != $username) $_SESSION['mass_errors'][$row2][] = array('WARN', _('Username'), _('Username in use. Selected next free username.'));
			// Check if givenname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['accounts'][$row2]->general_givenname)) $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Given name'), _('Given name contains invalid characters'));
			// Check if surname is valid
			if ( !ereg('^([a-z]|[A-Z]|[-]|[ ]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])+$', $_SESSION['accounts'][$row2]->general_surname)) $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Surname'), _('Surname contains invalid characters'));
			if ( ($_SESSION['accounts'][$row2]->general_gecos=='') || ($_SESSION['accounts'][$row2]->general_gecos==' ')) {
				$_SESSION['accounts'][$row2]->general_gecos = $_SESSION['accounts'][$row2]->general_givenname . " " . $_SESSION['accounts'][$row2]->general_surname ;
				$_SESSION['mass_errors'][$row2][] = array('INFO', _('Gecos'), _('Inserted sur- and given name in gecos-field.'));
				}
			$_SESSION['accounts'][$row2]->smb_displayName = $_SESSION['accounts'][$row2]->general_gecos;
			if ($_SESSION['accounts'][$row2]->general_group=='') $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Primary group'), _('No primary group defined!'));
			// Check if Username contains only valid characters
			if ( !ereg('^([a-z]|[0-9]|[.]|[-]|[_])*$', $_SESSION['accounts'][$row2]->general_username))
				$_SESSION['mass_errors'][$row2][] = array('ERROR', _('Username'), _('Username contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
			// Check if Name-length is OK. minLength=3, maxLength=20
			if ( !ereg('.{3,20}', $_SESSION['accounts'][$row2]->general_username)) $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Name'), _('Name must contain between 3 and 20 characters.'));
			// Check if Name starts with letter
			if ( !ereg('^([a-z]|[A-Z]).*$', $_SESSION['accounts'][$row2]->general_username))
				$_SESSION['mass_errors'][$row2][] = array('ERROR', _('Name'), _('Name contains invalid characters. First character must be a letter.'));
			// Personal Settings
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['accounts'][$row2]->personal_telephoneNumber))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['accounts'][$row2]->personal_mobileTelephoneNumber))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Mobile number'), _('Please enter a valid mobile number!'));
			if ( !ereg('^(\+)*([0-9]|[ ]|[.]|[(]|[)]|[/])*$', $_SESSION['accounts'][$row2]->personal_facsimileTelephoneNumber))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Fax number'), _('Please enter a valid fax number!'));
			if ( !ereg('^(([0-9]|[A-Z]|[a-z]|[.]|[-]|[_])+[@]([0-9]|[A-Z]|[a-z]|[-])+([.]([0-9]|[A-Z]|[a-z]|[-])+)*)*$', $_SESSION['accounts'][$row2]->personal_mail))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('eMail address'), _('Please enter a valid eMail address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_street))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Street'), _('Please enter a valid street name!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_postalAddress))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Postal address'), _('Please enter a valid postal address!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_title))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Title'), _('Please enter a valid title!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z]|[ ]|[.]|[ä]|[Ä]|[ö]|[Ö]|[ü]|[Ü]|[ß])*$', $_SESSION['accounts'][$row2]->personal_employeeType))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Employee type'), _('Please enter a valid employee type!'));
			if ( !ereg('^([0-9]|[A-Z]|[a-z])*$', $_SESSION['accounts']->personal_postalCode))  $_SESSION['mass_errors'][$row2][] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
			}
		}
	// Close file if it was opened
	if ($_FILES['userfile']['size']>0) {
		fclose($handle);
		unlink($_FILES['userfile']['tmp_name']);
		}
	// Return false if more than 400 users were found
	if ($row2>400) return false;
		else return true;
	}


?>
