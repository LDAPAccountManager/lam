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


  LDAP Account Manager Delete user, hosts or groups
*/

include_once('../lib/ldap.inc');
include_once('../lib/account.inc');
include_once('../lib/config.inc');
// start session
session_save_path('../sess');
@session_start();
// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	metaRefresh("login.php");
	die;
	}

// set language
setlanguage();

// use references because session-vars can change in future
$ldap_intern =& $_SESSION['ldap'];
$header_intern =& $_SESSION['header'];
$config_intern =& $_SESSION['config'];
$delete_dn =& $_SESSION['delete_dn'];


if ($_POST['backmain']) {
	// back to list page
	if (isset($_SESSION['delete_dn'])) unset ($_SESSION['delete_dn']);
	metaRefresh("lists/list".$_POST['type']."s.php");
	// stop script because we don't want to reate invalid html-code
	die;
	}

// Print header and part of body
echo $header_intern;
echo '<html><head><title>';
echo _('Delete Account');
echo '</title>'."\n".
	'<link rel="stylesheet" type="text/css" href="../style/layout.css">'."\n".
	'<meta http-equiv="pragma" content="no-cache">'."\n".
	'<meta http-equiv="cache-control" content="no-cache">'."\n".
	'</head>'."\n".
	'<body>'."\n".
	'<form action="delete.php" method="post">'."\n";


if ($_GET['type']) {
	// $_GET['type'] is true if delete.php was called from *list.php
	// Store $_GET['type'] as $_POST['type']
	echo '<input name="type" type="hidden" value="'.$_GET['type'].'">';
	switch ($_GET['type']) {
		// Select which layout and text should be displayed
		case 'user':
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Delete user(s)');
			echo "</b></legend>\n";
			echo '<b>'._('Do you really want to delete user(s):').'</b>';
			break;
		case 'host':
			echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
			echo _('Delete host(s)');
			echo "</b></legend>\n";
			echo '<b>'._('Do you really want to delete host(s):').'</b>';
			break;
		case 'group':
			echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
			echo _('Delete group(s)');
			echo "</b></legend>\n";
			echo '<b>'._('Do you really want to delete group(s):').'</b>';
			break;
		}
	echo "<br>\n";
	// display all DNs in a tables
	echo "<table border=0 width=\"100%\">\n";
	foreach ($delete_dn as $dn) echo '<tr><td>'.$dn.'</td></tr>';
	echo "</table>\n";

	// Ask if lam should delete homedirs if users are deleted and lamdaemon.pl is in use
	if (($_GET['type']== user) && $config_intern->scriptServer) {
		echo "<br>\n";
		echo "<table border=0>\n";
		echo '<tr><td>';
		echo _('Delete also Homedirectories');
		echo '</td>'."\n".'<td><input name="f_rem_home" type="checkbox">'.
			'</td></tr>'."\n";
		echo "</table>\n";
		}

	// Print buttons
	echo "<br><table border=0>\n";
	echo '<tr><td>'.
		'<input name="delete_no" type="submit" value="';
	echo _('Cancel'); echo '"></td><td></td><td>'.
		'<input name="delete_yes" type="submit" value="';
	echo _('Commit'); echo '"></td></tr>';
	echo "</table></fieldset>\n";
	}


if ($_POST['delete_yes']) {
	// deletion has been confirmed.
	switch ($_POST['type']) {
		case 'user':
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Deleting user(s)...');
			echo "</b></legend>\n";
			break;
		case 'host':
			echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
			echo _('Deleting host(s)...');
			echo "</b></legend>\n";
			break;
		case 'group':
			echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
			echo _('Deleting group(s)...');
			echo "</b></legend>\n";
			break;
		}
	echo '<input name="type" type="hidden" value="'.$_POST['type'].'">';
	echo "<br><table border=0 >\n";
	// Store kind of DNs
	foreach ($delete_dn as $dn) {
		// Loop for every DN which should be deleted
		switch ($_POST['type']) {
			case 'user':
				// Get username from DN
				$temp=explode(',', $dn);
				$username = str_replace('uid=', '', $temp[0]);

				if ($config_intern->scriptServer) {
					// Remove homedir if required
					if ($_POST['f_rem_home']) remhomedir(array($username));
					// Remove quotas if lamdaemon.pl is used
					if ($config_intern->scriptServer) remquotas(array($username), 'user');
					}
				// Search for groups which have memberUid set to username
				$result = ldap_search($ldap_intern->server(), $config_intern->get_GroupSuffix(), "(&(objectClass=PosixGroup)(memberUid=$username))", array(''));
				$entry = ldap_first_entry($ldap_intern->server(), $result);
				// loop for every found group and remove membership
				while ($entry) {
					$success = ldap_mod_del($ldap_intern->server(), ldap_get_dn($ldap_intern->server(), $entry) , array('memberUid' => $username));
					// *** fixme add error-message if memberUid couldn't be deleted
					$entry = ldap_next_entry($ldap_intern->server(), $entry);
					}
				// Delete user itself
				$success = ldap_delete($ldap_intern->server(), $dn);
				if (!$success) $error = _('Could not delete user:').' '.$dn;
				break;
			case 'host':
				// Delete host itself
				$success = ldap_delete($ldap_intern->server(), $dn);
				if (!$success) $error = _('Could not delete host:').' '.$dn;
				break;
			case 'group':
				/* First we have to check if any user uses $group
				* as primary group. It's not allowed to delete a
				* group if it still contains primaty members
				*/
				$temp=explode(',', $dn);
				$groupname = str_replace('cn=', '', $temp[0]);
				// Get group GIDNumber
				$groupgid = getgid($groupname);
				// Search for users which have gid set to current gid
				$result = ldap_search($ldap_intern->server(), $config_intern->get_UserSuffix(), "gidNumber=$groupgid", array(''));
				// Print error if still users in group
				if (!$result) $error = _('Could not delete group. Still users in group:').' '.$dn;
				else {
					// continue if no primary users are in group
					// Remove quotas if lamdaemon.pl is used
					if ($config_intern->scriptServer) remquotas(array($groupname), 'group');
					// Delete group itself
					$success = ldap_delete($ldap_intern->server(), $dn);
					if (!$success) $error = _('Could not delete group:').' '.$dn;
					}
				break;
			}
		// Remove DNs from cache-array
		if ($success && isset($_SESSION[$_POST['type'].'DN'][$dn])) unset($_SESSION[$_POST['type'].'DN'][$dn]);
		// Display success or error-message
		if (!$error) echo "<tr><td><b>$dn ". _('deleted').".</b></td></tr>\n";
		 else echo "<tr><td><b>$error</b></td></tr>\n";
		}
	echo "</table><br>\n";
	switch ($_POST['type']) {
		// Select which page should be displayd if back-button will be pressed
		case 'user':
			echo '<input name="backmain" type="submit" value="'; echo _('Back to user list'); echo '">';
			break;
		case 'group':
			echo '<input name="backmain" type="submit" value="'; echo _('Back to group list'); echo '">';
			break;
		case 'host':
			echo '<input name="backmain" type="submit" value="'; echo _('Back to host list'); echo '">';
			break;
		}
	echo "<br></fieldset>\n";
	}

if ($_POST['delete_no']) {
	// Delete no accounts
	echo '<input name="type" type="hidden" value="'.$_POST['type'].'">';
	switch ($_POST['type']) {
		// Select which page should be displayd if back-button will be pressed
		case 'user':
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Deleting user(s) canceled.');
			echo "</b></legend>\n";
			echo _('No user(s) were deleted');
			echo "<br>";
			echo '<input name="backmain" type="submit" value="'; echo _('Back to user list'); echo '">';
			break;
		case 'host':
			echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
			echo _('Deleting host(s) canceled.');
			echo "</b></legend>\n";
			echo _('No host(s) were deleted');
			echo "<br>";
			echo '<input name="backmain" type="submit" value="'; echo _('Back to host list'); echo '">';
			break;
		case 'group':
			echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
			echo _('Deleting group(s) canceled.');
			echo "</b></legend>\n";
			echo _('No group(s) were deleted');
			echo "<br>";
			echo '<input name="backmain" type="submit" value="'; echo _('Back to group list'); echo '">';
			break;
		}
	echo "<br></fieldset>\n";
	}

echo '</form></body></html>'."\n";
?>
