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
session_save_path('../sess');
@session_start();
setlanguage();

echo $_SESSION['header'];
echo '<html><head><title>';
echo _('Delete Account');
echo '</title>'."\n".
	'<link rel="stylesheet" type="text/css" href="'.$_SESSION['lamurl'].'style/layout.css">'."\n".
	'<meta http-equiv="pragma" content="no-cache">'."\n".
	'<meta http-equiv="cache-control" content="no-cache">'."\n";

if ($_POST['backmain'])
	switch ( $_POST['type5'] ) {
		case 'user' :
			echo "<meta http-equiv=\"refresh\" content=\"2; URL=lists/listusers.php\">\n";
			break;
		case 'group' :
			echo "<meta http-equiv=\"refresh\" content=\"2; URL=lists/listgroups.php\">\n";
			break;
		case 'host' :
			echo "<meta http-equiv=\"refresh\" content=\"2; URL=lists/listhosts.php\">\n";
			break;
			}
echo '</head>'."\n".
	'<body>'."\n".
	'<form action="delete.php" method="post">'."\n";

if ($_GET['type']) {
	$DN2 = explode(";", str_replace("\'", '',$_GET['DN']));
	echo '<input name="type5" type="hidden" value="'.$_GET['type'].'">';
	echo '<input name="DN" type="hidden" value="'.$_GET['DN'].'">';
	switch ($_GET['type']) {
		case 'user':
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Delete user(s)');
			echo "</b></legend>\n";
			echo _('<b>Do you really want to delete user(s):</b>');
			break;
		case 'host':
			echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
			echo _('Delete host(s)');
			echo "</b></legend>\n";
			echo _('<b>Do you really want to delete host(s):</b>');
			break;
		case 'group':
			echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
			echo _('Delete group(s)');
			echo "</b></legend>\n";
			echo _('<b>Do you really want to delete group(s):</b>');
			break;
		}
	echo "<table border=0 width=\"100%\">\n";
	foreach ($DN2 as $dn) echo '<tr><td>'.$dn.'</td></tr>';
	echo "</table>\n";
	if (($_GET['type']== user) && $_SESSION['config']->scriptServer) {
		echo "<table border=0 width=\"100%\">\n";
		echo '<tr><td>';
		echo _('Delete also Homedirectories');
		echo '</td>'."\n".'<td><input name="f_rem_home" type="checkbox">'.
			'</td></tr>'."\n";
		echo "</table>\n";
		}

	echo "<br><table border=0 width=\"100%\">\n";
	echo '<tr><td>'.
		'<input name="delete_no" type="submit" value="';
	echo _('Cancel'); echo '"></td><td></td><td>'.
		'<input name="delete_yes" type="submit" value="';
	echo _('Commit'); echo '"></td></tr>';
	echo "</table></fieldset>\n";
	}


if ($_POST['delete_yes'] && !$_POST['backmain']) {

	switch ($_POST['type5']) {
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
	echo "<br><table border=0 width=\"100%\">\n";
	$DN2 = explode(";", str_replace("\\", '',str_replace("\'", '',$_POST['DN'])));
	foreach ($DN2 as $dn) {
		echo '<input name="type5" type="hidden" value="'.$_POST['type5'].'">';
		switch ($_POST['type5']) {
			case 'user':
				$temp=explode(',', $dn);
				$username = str_replace('uid=', '', $temp[0]);
				if ($_SESSION['config']->scriptServer) {
					if ($_POST['f_rem_home']) remhomedir($username);
					remquotas($username, $_POST['type5']);
					}
				$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'objectClass=PosixGroup', array('memberUid'));
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if ($attr2['memberUid']) {
						array_shift($attr2['memberUid']);
						foreach ($attr2['memberUid'] as $nam) {
							if ($nam==$username) {
								$todelete['memberUid'] = $nam;
								$success = ldap_mod_del($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry) ,$todelete);
								}
							}
						}
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				$success = ldap_delete($_SESSION['ldap']->server(), $dn);
				if (!$success) $error = _('Could not delete user:').' '.$dn;
				break;
			case 'host':
				$success = ldap_delete($_SESSION['ldap']->server(), $dn);
				if (!$success) $error = _('Could not delete host:').' '.$dn;
				break;
			case 'group':
				$temp=explode(',', $dn);
				$groupname = str_replace('cn=', '', $temp[0]);
				$result = ldap_search($_SESSION['ldap']->server(), $dn, 'objectClass=*', array('gidNumber'));
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if ($attr2['gidNumber']==getgid($groupname)) $error = _('Could not delete group. Still users in group:').' '.$dn;
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				if (!$error) {
					if ($_SESSION['config']->scriptServer) remquotas($groupname, $_POST['type5']);
					$success = ldap_delete($_SESSION['ldap']->server(), $dn);
					if (!$success) $error = _('Could not delete group:').' '.$dn;
					}
				break;
			}
		if (!$error) echo "<tr><td><b>$dn ". _('deleted').".</b></td></tr>\n";
		 else echo "<tr><td><b>$error</b></td></tr>\n";
		}
	echo "</table><br>\n";
	switch ($_POST['type5']) {
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
	echo "</fieldset>\n";
	}

if ($_POST['delete_no']) {
	switch ($_POST['type5']) {
		case 'user':
			echo "<fieldset class=\"useredit-bright\"><legend class=\"useredit-bright\"><b>";
			echo _('Deleting user(s) canceled.');
			echo "</b></legend>\n";
			echo _('No user(s) were deleted');
			echo "<br>";
			echo '<input name="backmain" type="submit" value="'; echo _('Back to user list'); echo '">';
			echo "</fieldset>\n";
			break;
		case 'host':
			echo "<fieldset class=\"hostedit-bright\"><legend class=\"hostedit-bright\"><b>";
			echo _('Deleting host(s) canceled.');
			echo "</b></legend>\n";
			echo _('No host(s) were deleted');
			echo "<br>";
			echo '<input name="backmain" type="submit" value="'; echo _('Back to host list'); echo '">';
			echo "</fieldset>\n";
			break;
		case 'group':
			echo "<fieldset class=\"groupedit-bright\"><legend class=\"groupedit-bright\"><b>";
			echo _('Deleting group(s) canceled.');
			echo "</b></legend>\n";
			echo _('No group(s) were deleted');
			echo "<br>";
			echo '<input name="backmain" type="submit" value="'; echo _('Back to group list'); echo '">';
			echo "</fieldset>\n";
			break;
		}

	}

if ($_POST['backmain'])
	switch ( $_POST['type5'] ) {
		case 'user' :
			echo '<a href="lists/listusers.php">';
			echo _('Please press here if meta-refresh didn\'t work.');
			echo "</a>\n";
			break;
		case 'group' :
			echo '<a href="lists/listgroups.php">';
			echo _('Please press here if meta-refresh didn\'t work.');
			echo "</a>\n";
			break;
		case 'host' :
			echo '<a href="lists/listhosts.php">';
			echo _('Please press here if meta-refresh didn\'t work.');
			echo "</a>\n";
			break;
			}
echo '</form></body></html>'."\n";
?>
