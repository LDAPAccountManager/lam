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
	if (isset($_GET['DN']) && $_GET['DN']!='') {
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		$DN = str_replace("\'", '',$_GET['DN']);
		$_SESSION['account'] = loadhost($DN);
		$_SESSION['account'] ->type = 'host';
		$_SESSION['account_old'] = $_SESSION['account'];
		$_SESSION['account']->general_dn = substr($_SESSION['account']->general_dn, strpos($_SESSION['account']->general_dn, ',')+1);
		$_SESSION['final_changegids'] = '';
		}
	else {
		$_SESSION['account'] = loadHostProfile('default');
		$_SESSION['account'] ->type = 'host';
		if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
		}
	}
else if (count($_POST)==0) { // Startcondition. groupedit.php was called from outside
	$_SESSION['account'] = loadHostProfile('default');
	$_SESSION['account'] ->type = 'host';
	if (isset($_SESSION['account_old'])) unset($_SESSION['account_old']);
	}


switch ($_POST['select']) { // Select which part of page should be loaded and check values
	// general = startpage, general account paramters
	// unix = page with all shadow-options and password
	// samba = page with all samba-related parameters e.g. smbpassword
	// quota = page with all quota-related parameters e.g. hard file quota
	// personal = page with all personal-related parametergs, e.g. phone number
	// final = last page shown before account is created/modified
	//		if account is modified commands might be ran are shown
	// finish = page shown after account has been created/modified
	case 'general':
		// Write all general values into $_SESSION['account'] if no profile should be loaded
		if (!$_POST['load']) {
			$_SESSION['account']->general_dn = $_POST['f_general_suffix'];
			$_SESSION['account']->general_username = $_POST['f_general_username'];
			$_SESSION['account']->general_uidNumber = $_POST['f_general_uidNumber'];
			$_SESSION['account']->general_group = $_POST['f_general_group'];
			$_SESSION['account']->general_gecos = $_POST['f_general_gecos'];

			// Check if values are OK and set automatic values.  if not error-variable will be set
			if (isset($_SESSION['account_old'])) list($values, $errors) = checkglobal($_SESSION['account'], $_SESSION['account']->type, $_SESSION['account_old']); // account.inc
				else list($values, $errors) = checkglobal($_SESSION['account'], $_SESSION['account']->type); // account.inc
			if (is_object($values)) {
				while (list($key, $val) = each($values)) // Set only defined values
					if (isset($val)) $_SESSION['account']->$key = $val;
				}
			// Check which part Site should be displayed next
			$select_local = 'general';
			}
		break;

	case 'samba':
		// Write all general values into $_SESSION['account']
		if (isset($_POST['f_smb_flagsD'])) $_SESSION['account']->smb_flagsD = true;
			else $_SESSION['account']->smb_flagsD = false;
		if ($_SESSION['config']->samba3 == 'yes') {
			$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
			for ($i=0; $i<sizeof($samba3domains); $i++)
				if ($_POST['f_smb_domain'] == $samba3domains[$i]->name) {
					$_SESSION['account']->smb_domain = $samba3domains[$i];
					}
			}
		else {
			$_SESSION['account']->smb_domain = $_POST['f_smb_domain'];
			}
		// Reset password if reset button was pressed. Button only vissible if account should be modified
		// Check if values are OK and set automatic values. if not error-variable will be set
		list($values, $errors) = checksamba($_SESSION['account'], $_SESSION['account']->type); // account.inc
		if (is_object($values)) {
			while (list($key, $val) = each($values)) // Set only defined values
				if (isset($val)) $_SESSION['account']->$key = $val;
			}
		// Check which part Site should be displayed next
		if ($_POST['respass']) {
			$_SESSION['account']->unix_password_no=true;
			$_SESSION['account']->smb_password_no=true;
			}
		$select_local = 'samba';
		break;
	case 'final':
		$select_local = 'final';
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
	if ($_POST['next_general']) {
		if (!isset($errors)) $select_local='general';
		break;
		}
	if ($_POST['next_samba']) {
		if (!isset($errors)) $select_local='samba';
		break;
		}
	if ($_POST['next_final']) {
		if (!isset($errors)) $select_local='final';
		break;
		}
	if ( $_POST['create'] ) { // Create-Button was pressed
		// Create or modify an account
		if ($_SESSION['account_old']) $result = modifyhost($_SESSION['account'],$_SESSION['account_old']);
		 else $result = createhost($_SESSION['account']); // account.inc
		if ( $result==1 || $result==3 ) $select_local = 'finish';
		 else $select_local = 'final';
		}
	if ($_POST['createagain']) {
		$select_local='general';
		unset($_SESSION['account']);
		$_SESSION['account'] = loadHostProfile('default');
		$_SESSION['account'] ->type = 'host';
		break;
		}
	if ($_POST['load']) {
		// load profile
		if ($_POST['f_general_selectprofile']!='') $values = loadHostProfile($_POST['f_general_selectprofile']);
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
		saveHostProfile($_SESSION['account'], $_POST['f_finish_safeProfile']);
		// select last page displayed before user is created
		$select_local='final';
		break;
		}
	if ($_POST['backmain']) {
		echo "<meta http-equiv=\"refresh\" content=\"2; URL=../lists/listhosts.php\">\n";
		$select_local='backmain';
		break;
		}
	if (!$select_local) $select_local='general';
	} while(0);

echo "</head><body>\n";
echo "<form action=\"hostedit.php\" method=\"post\">\n";

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
	case 'general':
		// General Account Settings
		// load list of all groups
		$groups = findgroups();
		// load list of profiles
		$profilelist = getHostProfiles();
		// Show page info
		echo '<input name="select" type="hidden" value="general">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" disabled value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td>\n<td>";
		echo "<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo "<fieldset><legend><b>";
		echo _("General properties");
		echo "</b></legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
		echo _('Host name').'*';
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_username" type="text" size="20" maxlength="20" value="' . $_SESSION['account']->general_username . '">'.
			'</td><td>'.
			'<a href="../help.php?HelpNumber=410" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('UID number');
		echo '</td>'."\n".'<td>'.
			'<input name="f_general_uidNumber" type="text" size="6" maxlength="6" value="' . $_SESSION['account']->general_uidNumber . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=411" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Primary group').'*';
		echo '</td>'."\n".'<td><select name="f_general_group">';
		foreach ($groups as $group) {
			if ($_SESSION['account']->general_group == $group) echo '<option selected>' . $group. '</option>';
			else echo '<option>' . $group. '</option>';
			 }
		echo '</select></td><td>'.
			'<a href="../help.php?HelpNumber=412" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo _('Gecos');
		echo '</td><td><input name="f_general_gecos" type="text" size="30" value="' . $_SESSION['account']->general_gecos . '">'.
			'</td>'."\n".'<td>'.
			'<a href="../help.php?HelpNumber=413" target="lamhelp">'._('Help').'</a>'.
			'</td></tr><tr><td>';
		echo _('Suffix'); echo '</td><td><select name="f_general_suffix">';
		foreach ($_SESSION['ldap']->search_units($_SESSION['config']->get_HostSuffix()) as $suffix) {
			if ($_SESSION['account']->general_dn) {
				if ($_SESSION['account']->general_dn == $suffix)
					echo '<option selected>' . $suffix. '</option>';
				else echo '<option>' . $suffix. '</option>';
				}
			else echo '<option>' . $suffix. '</option>';
			}
		echo '</select></td><td><a href="../help.php?HelpNumber=463" target="lamhelp">'._('Help').'</a>'.
			"</td>\n</tr>\n</table>";
		echo _('Values with * are required');
		echo "</fieldset>\n</td></tr><tr><td>";
		if (count($profilelist)!=0) {
			echo "<fieldset><legend>";
			echo _("Load profile");
			echo "</legend>\n<table border=0 width=\"100%\">\n<tr>\n<td>";
			echo "<select name=\"f_general_selectprofile\" >";
			foreach ($profilelist as $profile) echo "	<option>$profile</option>\n";
			echo "</select></td><td>\n".
				"<input name=\"load\" type=\"submit\" value=\""; echo _('Load Profile');
			echo "\"></td><td><a href=\"../help.php?HelpNumber=XXX\" target=\"lamhelp\">";
			echo _('Help-XX')."</a></td>\n</tr>\n</table>\n</fieldset>\n";
			}
		echo "</td></tr></table>\n</td></tr>\n</table>\n";
		break;

	case 'samba':
		// Samba Settings
		if ($_SESSION['config']->samba3 == 'yes') $samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
		$_SESSION['account']->smb_flagsW = 1;
		if ($_SESSION['account']->smb_password_no) echo '<input name="f_smb_password_no" type="hidden" value="1">';
		echo '<input name="f_unix_password_no" type="hidden" value="';
		echo '<input name="select" type="hidden" value="samba">';

		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" disabled value=\""; echo _('Samba'); echo "\">\n<br>";
		echo "<input name=\"next_final\" type=\"submit\" value=\""; echo _('Final');
		echo "\"></fieldset></td>\n<td>";
		echo "<fieldset><legend><b>"._('Samba properties')."</b></legend>\n";
		echo "<table border=0 width=\"100%\"><tr><td>";
		echo _('Password');
		echo '</td><td>';
		if (isset($_SESSION['account_old'])) {
			echo '<input name="respass" type="submit" value="';
			echo _('Reset password'); echo '">';
			}
		echo '</td></tr>'."\n".'<tr><td>';
		echo _('Account is deactivated');
		echo '</td>'."\n".'<td><input name="f_smb_flagsD" type="checkbox"';
		if ($_SESSION['account']->smb_flagsD) echo ' checked ';
		echo '></td><td>'.
			'<a href="../help.php?HelpNumber=432" target="lamhelp">'._('Help').'</a>'.
			'</td></tr>'."\n".'<tr><td>';
		echo '</td></tr>'."\n".'<tr><td>';
		echo _('Domain');
		if ($_SESSION['config']->samba3 == 'yes') {
				echo '</td><td><select name="f_smb_domain">';
			for ($i=0; $i<sizeof($samba3domains); $i++) {
				if ($_SESSION['account']->smb_domain->name) {
					if ($_SESSION['account']->smb_domain->name == $samba3domains[$i]->name)
						echo '<option selected>' . $samba3domains[$i]->name. '</option>';
					else echo '<option>' . $samba3domains[$i]->name. '</option>';
					}
				else echo '<option>' . $samba3domains[$i]->name. '</option>';
				}
			echo '</select>';
			}
		else {
			echo '</td>'."\n".'<td><input name="f_smb_domain" type="text" size="20" maxlength="80" value="' . $_SESSION['account']->smb_domain . '">';
			}
		echo	'</td>'."\n".'<td><a href="../help.php?HelpNumber=460" target="lamhelp">'._('Help').'</a></td></tr>'."\n";
		echo "</table>\n</fieldset>\n</tr>\n</table>\n";
		break;

	case 'final':
		// Final Settings
		echo '<input name="select" type="hidden" value="final">';
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<br><fieldset><legend>";
		echo _('Please select page:');
		echo "</legend>\n";
		echo "<input name=\"next_general\" type=\"submit\" value=\""; echo _('General'); echo "\">\n<br>";
		echo "<input name=\"next_samba\" type=\"submit\" value=\""; echo _('Samba'); echo "\">\n<br>";
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
		echo "<table border=0 width=\"100%\"><tr><td>";
		if (($_SESSION['account_old']) && ($_SESSION['account']->general_uidNumber != $_SESSION['account_old']->general_uidNumber)) {
			echo '<tr>';
			StatusMessage ('INFO', _('UID-number has changed. You have to run the following command as root in order to change existing file-permissions:'),
			'find / -gid ' . $_SESSION['account_old' ]->general_uidNumber . ' -exec chown ' . $_SESSION['account']->general_uidNumber . ' {} \;');
			echo '</tr>'."\n";
			}
		if (isset($_SESSION['account_old']->general_objectClass)) {
			if (!in_array('posixAccount', $_SESSION['account_old']->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass posixAccount not found.'), _('Have to recreate entry.'));
				echo "</tr>\n";
				}
			if (!in_array('shadowAccount', $_SESSION['account_old']->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass shadowAccount not found.'), _('Have to recreate entry.'));
				echo "</tr>\n";
				}
			if (!in_array('account', $_SESSION['account_old']->general_objectClass)) {
				echo '<tr>';
				StatusMessage('WARN', _('ObjectClass account not found.'), _('Have to recreate entry.'));
				echo "</tr>\n";
				}
			if ($_SESSION['config']->samba3 == 'yes') {
				if (!in_array('sambaSamAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass sambaSamAccount not found.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}}
				else
				if (!in_array('sambaAccount', $_SESSION['account_old']->general_objectClass)) {
					echo '<tr>';
					StatusMessage('WARN', _('ObjectClass sambaAccount not found.'), _('Have to recreate entry.'));
					echo "</tr>\n";
					}
			}
		echo '<input name="create" type="submit" value="';
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
		echo _('Host');
		echo ' '.$_SESSION['account']->general_username.' ';
		if ($_SESSION['account_old']) echo ' '._('has been modified').'.';
		 else echo ' '._('has been created').'.';
		echo '</td></tr>'."\n".'<tr><td>';
		if (!$_SESSION['account_old'])
			{ echo '<input name="createagain" type="submit" value="'; echo _('Create another host'); echo '">'; }
		echo '</td><td>'."\n".'</td><td>'.
			'<input name="backmain" type="submit" value="'; echo _('Back to host list'); echo '">'.
			'</td></tr></table></fieldset'."\n";
		break;
	case 'backmain':
		// unregister sessionvar and select which list should be shown
		echo '<a href="../lists/listhosts.php">';
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
