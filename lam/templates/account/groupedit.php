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

include_once('../../lib/account.inc'); // File with all account-funtions
include_once('../../lib/config.inc'); // File with configure-functions
include_once('../../lib/profiles.inc'); // functions to load and save profiles
include_once('../../lib/status.inc'); // Return error-message
include_once('../../lib/pdf.inc'); // Return a pdf-file
include_once('../../lib/ldap.inc'); // LDAP-functions

session_save_path('../../sess');
@session_start();
setlanguage();
if (isset($_GET['DN'])) {
	if ($_GET['DN']!='') {
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		$DN = str_replace("\'", '',$_GET['DN']);
		$_SESSION['account'] = loadgroup($DN);
		$_SESSION['account'] ->type = 'group';
		$_SESSION['account_old'] = $_SESSION['account'];
		$_SESSION['account']->general_dn = substr($_SESSION['account']->general_dn, strpos($_SESSION['account']->general_dn, ',')+1);
		$_SESSION['final_changegids'] = '';
		}
	else {
		$_SESSION['account'] = loadGroupProfile('default');
		$_SESSION['account'] ->type = 'group';
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		}
	$values = getquotas($type);
	if (is_object($values)) {
		while (list($key, $val) = each($values)) // Set only defined values
			if (isset($val)) $_SESSION['account']->$key = $val;
			}
	}
else if (count($_POST)==0) { // Startcondition. groupedit.php was called from outside
	$_SESSION['account'] = loadGroupProfile('default');
	$_SESSION['account'] ->type = 'group';
	if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
	}

switch ($_POST['select']) { // Select which part of page should be loaded and check values
	// general = startpage, general account paramters
	// samba = page with all samba-related parameters e.g. smbpassword
	// quota = page with all quota-related parameters e.g. hard file quota
	// personal = page with all personal-related parametergs, e.g. phone number
	// final = last page shown before account is created/modified
	//		if account is modified commands might be ran are shown
	// finish = page shown after account has been created/modified
	case 'groupmembers':
		do { // X-Or, only one if() can be true
			if (isset($_POST['users']) && isset($_POST['add'])) { // Add users to list
				// Add new user
				$_SESSION['account']->unix_memberUid = array_merge($_SESSION['account']->unix_memberUid, $_POST['users']);
				// remove doubles
				$_SESSION['account']->unix_memberUid = array_flip($_SESSION['account']->unix_memberUid);
				array_unique($_SESSION['account']->unix_memberUid);
				$_SESSION['account']->unix_memberUid = array_flip($_SESSION['account']->unix_memberUid);
				// sort user
				sort($_SESSION['account']->unix_memberUid);
				// display groupmembers page
				$select_local = 'groupmembers';
				break;
				}
			if (isset($_POST['members']) && isset($_POST['remove'])) { // remove users fromlist
				$_SESSION['account']->unix_memberUid = array_delete($_POST['members'], $_SESSION['account']->unix_memberUid);
				$select_local = 'groupmembers';
				break;
				}
			$select_local = 'groupmembers';
			} while(0);
		break;

	case 'general':
		// Write all general values into $_SESSION['account'] if no profile should be loaded
		if (!$_POST['load']) {
			$_SESSION['account']->general_dn = $_POST['f_general_suffix'];
			$_SESSION['account']->general_username = $_POST['f_general_username'];
			$_SESSION['account']->general_uidNumber = $_POST['f_general_uidNumber'];
			$_SESSION['account']->general_gecos = $_POST['f_general_gecos'];
			// Check if values are OK and set automatic values.  if not error-variable will be set
			if (isset($_SESSION['account_old'])) list($values, $errors) = checkglobal($_SESSION['account'], 'group', $_SESSION['account_old']); // account.inc
				else list($values, $errors) = checkglobal($_SESSION['account'], 'group'); // account.inc
			if (is_object($values)) { // Set only defined values
				while (list($key, $val) = each($values))
					if (isset($val)) $_SESSION['account']->$key = $val;
				}
			// Check which part Site should be displayed next
			$select_local = 'general';
			}
		break;

	case 'samba':
		$_SESSION['account']->smb_domain = $_POST['f_smb_domain'];
		switch ($_POST['f_smb_mapgroup']) {
			case '*'._('Domain Guests'): $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '514'; break;
			case '*'._('Domain Users'): $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '513'; break;
			case '*'._('Domain Admins'): $_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-" . '512'; break;
			case $_SESSION['account']->general_group:
				if ($_SESSION['config']->samba3 == 'yes')
					$_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-".
						(2 * getgid($_SESSION['account']->general_group) + $_SESSION['account']->smb_domain->RIDbase +1);
				else $_SESSION['account']->smb_mapgroup = (2 * getgid($_SESSION['account']->general_group) + 1001);
				break;
			case $_SESSION['account']->general_username:
				if ($_SESSION['config']->samba3 == 'yes')
					$_SESSION['account']->smb_mapgroup = $_SESSION['account']->smb_domain->SID . "-".
						(2 * $_SESSION['account']->general_uidNumber + $_SESSION['account']->smb_domain->RIDbase +1);
				else $_SESSION['account']->smb_mapgroup = (2 * $_SESSION['account']->general_uidNumber + 1001);
				break;
			}
		break;

	case 'quota':
		// Write all general values into $_SESSION['account']
		$i=0;
		while ($_SESSION['account']->quota[$i][0]) {
			$_SESSION['account']->quota[$i][2] = $_POST['f_quota_'.$i.'_2'];
			$_SESSION['account']->quota[$i][3] = $_POST['f_quota_'.$i.'_3'];
			$_SESSION['account']->quota[$i][6] = $_POST['f_quota_'.$i.'_6'];
			$_SESSION['account']->quota[$i][7] = $_POST['f_quota_'.$i.'_7'];
			$i++;
			}

		// Check if values are OK and set automatic values. if not error-variable will be set
		list($values, $errors) = checkquota($_SESSION['account'], $_SESSION['account']->type); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $_SESSION['account']->$key = $val;
			}

		// Check which part Site should be displayed next
		break;

	case 'final':
		// Write all general values into $_SESSION['account']
		if ($_POST['f_final_changegids']) $_SESSION['final_changegids'] = $_POST['f_final_changegids'] ;
		// Check which part Site should be displayed next
		break;

	}

// Write HTML-Header and part of Table
echo $_SESSION['header'];
echo "<html><head><title>";
echo _("Create new Account");
echo "</title>\n".
	"<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n".
	"<meta http-equiv=\"pragma\" content=\"no-cache\">\n".
	"<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";

do { // X-Or, only one if() can be true
	if ($_POST['next_members']) {
		if (!isset($errors)) $select_local='groupmembers';
		break;
		}
	if ($_POST['next_general']) {
		if (!isset($errors)) $select_local='general';
		break;
		}
	if ($_POST['next_samba']) {
		if (!isset($errors)) $select_local='samba';
		break;
		}
	if ($_POST['next_quota']) {
		if (!isset($errors)) $select_local='quota';
		break;
		}
	if ($_POST['next_final']) {
		if (!isset($errors)) $select_local='final';
		break;
		}
	if ( $_POST['create'] ) { // Create-Button was pressed
		if ($_SESSION['account_old']) $result = modifygroup($_SESSION['account'],$_SESSION['account_old']);
		 else $result = creategroup($_SESSION['account']); // account.inc
		if ( $result==1 || $result==3 ) $select_local = 'finish';
			else $select_local = 'final';
		break;
		}
	// Reset variables if recreate-button was pressed
	if ($_POST['createagain']) {
		$select_local='general';
		unset($_SESSION['account']);
		$_SESSION['account'] = loadGroupProfile('default');
		$_SESSION['account'] ->type = 'group';
		break;
		}
	if ($_POST['backmain']) {
		$select_local='backmain';
		echo "<meta http-equiv=\"refresh\" content=\"2; URL=../lists/listgroups.php\">\n";
		break;
		}
	if ($_POST['load']) {
		// load profile
		if ($_POST['f_general_selectprofile']!='') $values = loadGroupProfile($_POST['f_general_selectprofile']);
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $_SESSION['account']->$key = $val;
			}
		// select general page after group has been loaded
		$select_local='general';
		break;
		}
	if ($_POST['save']) {
		// save profile
		saveGroupProfile($_SESSION['account'], $_POST['f_finish_safeProfile']);
		// select last page displayed before user is created
		$select_local='final';
		break;
		}
	if ($_POST['groupmembers']) {
		$select_local='groupmembers';
		break;
		}
	// Set selected page to general if no page was defined. should only true if groupedit.php wasn't called by itself
	if (!$select_local) $select_local='general';
	} while(0);

echo "</head><body>\n";
echo "<form action=\"groupedit.php\" method=\"post\">\n";

if (is_array($errors)) {
	echo "<table class=\"account\" width=\"100%\">\n";
	for ($i=0; $i<sizeof($errors); $i++) StatusMessage($errors[$i][0], $errors[$i][1], $errors[$i][2]);
	echo "</table>";
	}
// print_r($_SESSION['account']);

switch ($select_local) { // Select which part of page will be loaded
	// general = startpage, general account paramters
	// unix = page with all shadow-options and password
	// samba = page with all samba-related parameters e.g. smbpassword
	// quota = page with all quota-related parameters e.g. hard file quota
	// personal = page with all personal-related parametergs, e.g. phone number
	// final = last page shown before account is created/modified
	//		if account is modified commands might be ran are shown
	// finish = page shown after account has been created/modified
	case 'groupmembers':
		ldapreload('user');
		echo "<input name=\"select\" type=\"hidden\" value=\"groupmembers\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" disabled value=\""; echo _('Members'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td>\n<td>";
		echo "<fieldset><legend><b>". _('Additional group members') . "</b></legend>\n";
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr><td valign=\"top\"><fieldset><legend>";
		echo _('Group members');
		echo "</legend><select name=\"members[]\" size=15 multiple>\n";
		for ($i=0; $i<count($_SESSION['account']->unix_memberUid); $i++)
			if ($_SESSION['account']->unix_memberUid[$i]!='') echo "		<option>".$_SESSION['account']->unix_memberUid[$i]."</option>\n";
		echo "</select></fieldset></td>\n";
		echo "<td align=\"center\" width=\"10%\"><input type=\"submit\" name=\"add\" value=\"<=\">";
		echo " ";
		echo "<input type=\"submit\" name=\"remove\" value=\"=>\"><br><br>";
		echo "<a href=\"help.php?HelpNumber=XXX\" target=\"lamhelp\">"._('Help-XX')."</a></td>\n";
		echo "<td valign=\"top\"><fieldset><legend>";
		echo _('Available users');
		echo "</legend><select name=\"users[]\" size=15 multiple>\n";
		foreach ($_SESSION['userDN'] as $temp)
			if (is_array($temp)) {
				echo "		<option>$temp[cn]</option>\n";
				}
		echo "</select></fieldset></td>\n</tr>\n</table>\n</fieldset>\n</tr>\n</table>\n";
		break;

	case 'general':
		// General Account Settings
		// load list of profiles
		$profilelist = getGroupProfiles();
		// Show page info
		echo "<input name=\"select\" type=\"hidden\" value=\"general\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" disabled value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset><legend><b>";
		echo _("General properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _("Groupname")."*";
		echo "</td>\n<td>".
			"<input name=\"f_general_username\" type=\"text\" size=\"30\" maxlength=\"20\" value=\"".$_SESSION['account']->general_username."\">".
			"</td>\n<td><a href=\"help.php?HelpNumber=407\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('GID number');
		echo "</td>\n<td><input name=\"f_general_uidNumber\" type=\"text\" size=\"30\" maxlength=\"6\" value=\"".$_SESSION['account']->general_uidNumber."\">".
			"</td>\n<td><a href=\"help.php?HelpNumber=408\" target=\"lamhelp\">"._('Help').
			"</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Gecos');
		echo "</td>\n<td><input name=\"f_general_gecos\" type=\"text\" size=\"30\" value=\"".$_SESSION['account']->general_gecos."\"></td>\n".
			"<td><a href=\"help.php?HelpNumber=409\" target=\"lamhelp\">"._('Help')."</a></td>\n</tr>\n<tr>\n<td>";
		echo _('Suffix'); echo "</td>\n<td><select name=\"f_general_suffix\">";

		foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_GroupSuffix()) as $suffix) {
			if ($_SESSION['account']->general_dn) {
				if ($_SESSION['account']->general_dn == $suffix)
					echo "	<option selected>$suffix</option>\n";
				else echo "	<option>$suffix</option>\n";
				}
			else echo "	<option>$suffix</option>\n";
			}
		echo "</select></td>\n<td><a href=\"help.php?HelpNumber=462\" target=\"lamhelp\">"._('Help').
			"</a></td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr><tr><td>";
		if (count($profilelist)!=0) {
			echo "<fieldset><legend>";
			echo _("Load profile");
			echo "</legend>\n<table border=0>\n<tr>\n<td>";
			echo "<select name=\"f_general_selectprofile\" >";
			foreach ($profilelist as $profile) echo "	<option>$profile</option>\n";
			echo "</select>\n".
				"<input name=\"load\" type=\"submit\" value=\""; echo _('Load Profile');
			echo "\"></td>\n</tr>\n</table>\n</fieldset>\n";
			}
		echo "</td></tr>\n</table>\n</td></tr></table>\n";
		break;

	case 'samba':
		// Samba Settings
		if ($_SESSION['config']->samba3 == 'yes') $samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
		echo "<input name=\"select\" type=\"hidden\" value=\"samba\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" disabled value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td>\n<td>";
		echo "<fieldset><legend><b>"._('Samba properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Windows groupname');
		echo "</td>\n<td><select name=\"f_smb_mapgroup\">";
		if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-".
			(2 * $_SESSION['account']->uidNumber) + $values->smb_domain->RIDbase +1) {
			echo '<option selected> ';
			echo $_SESSION['account']->general_username;
			echo "</option>\n"; }
		 else {
			echo '<option> ';
			echo $_SESSION['account']->general_username;
			echo "</option>\n";
			}
		if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '514' ) {
			echo '<option selected> *';
			echo _('Domain Guests');
			echo "</option>\n"; }
		 else {
			echo '<option> *';
			echo _('Domain Guests');
			echo "</option>\n";
			}
		if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '513' ) {
			echo '<option selected> *';
			echo _('Domain Users');
			echo "</option>\n"; }
		 else {
			echo '<option> *';
			echo _('Domain Users');
			echo "</option>\n";
			}
		if ( $_SESSION['account']->smb_mapgroup == $_SESSION['account']->smb_domain->SID . "-" . '512' ) {
			echo '<option selected> *';
			echo _('Domain Admins');
			echo "</option>\n"; }
		 else {
			echo '<option> *';
			echo _('Domain Admins');
			echo "</option>\n";
			}
		echo	"</select></td>\n<td>".
			'<a href="help.php?HelpNumber=464" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Domain');
		echo '</td><td><select name="f_smb_domain">';
		for ($i=0; $i<sizeof($samba3domains); $i++) {
			if ($_SESSION['account']->smb_domain->name) {
				if ($_SESSION['account']->smb_domain->name == $samba3domains[$i]->name)
					echo '<option selected>' . $samba3domains[$i]->name. '</option>';
				else echo '<option>' . $samba3domains[$i]->name. '</option>';
				}
			else echo '<option>' . $samba3domains[$i]->name. '</option>';
			}
		echo	'</select></td>'."\n".'<td><a href="help.php?HelpNumber=467" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</tr>\n</table>\n";
		break;

	case 'quota':
		// Quota Settings
		echo "<input name=\"select\" type=\"hidden\" value=\"samba\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\" disabled value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td>\n<td>";
		echo '<input name="select" type="hidden" value="quota">';
		echo "<fieldset><legend><b>"._('Quota properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Mountpoint'); echo '</td>'."\n".'<td>'; echo _('Used blocks'); echo '</td>'."\n".'<td>';
		echo _('Soft block limit'); echo '</td>'."\n".'<td>'; echo _('Hard block limit'); echo '</td>'."\n".'<td>'; echo _('Grace block period');
		echo '</td>'."\n".'<td>'; echo _('Used inodes'); echo '</td>'."\n".'<td>'; echo _('Soft inode limit'); echo '</td>'."\n".'<td>';
		echo _('Hard inode limit'); echo '</td>'."\n".'<td>'; echo _('Grace inode period'); echo '</td></tr>'."\n";
		echo '<tr><td><a href="help.php?HelpNumber=439" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=440" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=441" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=442" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=443" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=444" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=445" target="lamhelp">'._('Help').'</a></td>'."\n".'<td><a href="help.php?HelpNumber=446" target="lamhelp">'._('Help').'</a></td>'."\n".'<td>'.
			'<a href="help.php?HelpNumber=447" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		$i=0;
		while ($_SESSION['account']->quota[$i][0]) {
			echo '<tr><td>'.$_SESSION['account']->quota[$i][0].'</td><td>'.$_SESSION['account']->quota[$i][1].'</td>'; // used blocks
			echo '<td><input name="f_quota_'.$i.'_2" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][2].'"></td>'; // blocks soft limit
			echo '<td><input name="f_quota_'.$i.'_3" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][3].'"></td>'; // blocks hard limit
			echo '<td>'.$_SESSION['account']->quota[$i][4].'</td>'; // block grace period
			echo '<td>'.$_SESSION['account']->quota[$i][5].'</td>'; // used inodes
			echo '<td><input name="f_quota_'.$i.'_6" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][6].'"></td>'; // inodes soft limit
			echo '<td><input name="f_quota_'.$i.'_7" type="text" size="12" maxlength="20" value="'.$_SESSION['account']->quota[$i][7].'"></td>'; // inodes hard limit
			echo '<td>'.$_SESSION['account']->quota[$i][8].'</td></tr>'; // inodes grace period
			$i++;
			}
		echo "</table>\n</fieldset>\n</tr>\n</table>\n";
		break;

	case 'final':
		// Final Settings
		echo '<input name="select" type="hidden" value="final">';
		echo "<input name=\"select\" type=\"hidden\" value=\"samba\">\n";
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_members\" type=\"submit\" value=\""; echo _('Members'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_quota\" type=\"submit\""; if (!isset($_SESSION['config']->scriptPath)) echo " disabled ";
		echo "value=\""; echo _('Quota'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" disabled value=\""; echo _('Final');
		echo "\"></fieldset></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset><legend>";
		echo _("Save profile");
		echo "</legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo '<input name="f_finish_safeProfile" type="text" size="30" maxlength="50">';
		echo '</td><td><input name="save" type="submit" value="';
		echo _('Save profile');
		echo '"></td><td><a href="../help.php?HelpNumber=457" target="lamhelp">'._('Help');
		echo "</a></td>\n</tr>\n</table>\n</fieldset>\n</td></tr>\n<tr><td>\n";
		echo "<fieldset><legend><b>";
		if ($_SESSION['account_old']) echo _('Modify');
		 else echo _('Create');
		echo "</b></legend>\n";
		echo "<table border=0 width=\"100%\">";
		if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
			echo '<tr>';
			StatusMessage ('INFO', _('GID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
			'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chgrp ' . $_SESSION['account']->general_uidNumber . ' {} \;');
			echo '</tr>'."\n";
			echo '<tr><td>';
			echo '<input name="f_final_changegids" type="checkbox"';
				if ($_SESSION['final_changegids']) echo ' checked ';
			echo ' >';
			echo _('Change GID-Number of all users in group to new value');
			echo '</td></tr>'."\n";
			}
		$disabled = "";
		if (!isset($_SESSION['account']->smb_mapgroup)) { // Samba page nit viewd; can not create group because if missing options
			$disabled = "disabled";
			echo "<tr>";
			StatusMessage("ERROR", _("Samba Options not set!"), _("Please check settings on samba page."));
			echo "</tr>";
			}
		if (isset($_SESSION['account_old']->general_objectClass)) {
			if (($_SESSION['config']->samba3 == 'yes') && (!in_array('sambaGroupMapping', $_SESSION['account_old']->general_objectClass))) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass sambaGroupMapping not found.'), _('Have to recreate entry.'));
				echo "</tr>\n";
				}
			if (!in_array('posixGroup', $_SESSION['account_old']->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass posixGroup not found.'), _('Have to recreate entry.'));
				echo "</tr>\n";
				}
			}
		echo "<tr><td><input name=\"create\" type=\"submit\" $disabled value=\"";
		if ($_SESSION['account_old']) echo _('Modify Account');
		 else echo _('Create Account');
		echo '">'."\n";
		echo "</td></tr></table></fieldset>\n</td></tr></table>\n</tr></table>";
		break;

	case 'finish':
		// Final Settings
		echo '<input name="select" type="hidden" value="finish">';
		echo "<fieldset><legend><b>"._('Success')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo '<tr><td>';
		echo _('Group').' ';
		echo $_SESSION['account']->general_username;
		if ($_SESSION['account_old']) echo ' '._('has been modified').'.';
		 else echo ' '._('has been created').'.';
		echo '</td></tr>'."\n".'<tr><td>';
		if (!$_SESSION['account_old'])
			{ echo' <input name="createagain" type="submit" value="'; echo _('Create another group'); echo '">'; }
		echo '</td><td></td><td>'.
			'<input name="backmain" type="submit" value="'; echo _('Back to group list'); echo '">'.
			'</td></tr></table></fieldset'."\n";
		break;

	case 'backmain':
		// unregister sessionvar and select which list should be shown
		echo '<a href="../lists/listgroups.php">';
		echo _('Please press here if meta-refresh didn\'t work.');
		echo "</a>\n";
		if (isset($_SESSION['shelllist'])) unset($_SESSION['shelllist']);
		if (isset($_SESSION['account'])) unset($_SESSION['account']);
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		break;
	}

// Print end of HTML-Page
echo '</form></body></html>';
?>
