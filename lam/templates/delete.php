<?
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

echo '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"
       "http://www.w3.org/TR/html4/loose.dtd">'."\n";
echo '<html><head><title>';
echo _('Delete Account');
echo '</title>'."\n".'
	<link rel="stylesheet" type="text/css" href="../style/layout.css">'."\n".'
	<meta http-equiv="pragma" content="no-cache">'."\n".'
	<meta http-equiv="cache-control" content="no-cache">'."\n".'
	<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">'."\n".'
	</head>'."\n".'
	<body>'."\n".'
	<form action="delete.php" method="post">'."\n".'
	<table rules="all" class="delete" width="100%">'."\n".'
	<tr><td>';

if ($_GET['type']) {
	$DN2 = explode(";", str_replace("\'", '',$_GET['DN']));
	echo '<input name="type5" type="hidden" value="'.$_GET['type'].'">';
	echo '<input name="DN" type="hidden" value="'.$_GET['DN'].'">';
	switch ($_GET['type']) {
		case 'user':
			echo _('Do you really want to delete user(s):');
			break;
		case 'host':
			echo _('Do you really want to delete host(s):');
			break;
		case 'group':
			echo _('Do you really want to delete group(s):');
			break;
		}
	echo '</td></tr>'."\n";
	foreach ($DN2 as $dn) echo '<tr><td>'.$dn.'</td></tr>';
	echo '<tr><td>';
	if (($_GET['type']== user) && $_SESSION['config']->scriptServer) {
		echo _('Delete also Homedirectories');
		echo '</td>'."\n".'<td><input name="f_rem_home" type="checkbox">
			</td></tr>'."\n".'<tr><td>';
		}
	echo '<br></td></tr>'."\n".'<tr><td>
		<input name="delete_no" type="submit" value="';
	echo _('Cancel'); echo '"></td><td></td><td>
		<input name="delete_yes" type="submit" value="';
	echo _('Commit'); echo '">';
	}

if ($_POST['delete_yes']) {
	$DN2 = explode(";", str_replace("\\", '',str_replace("\'", '',$_POST['DN'])));
	foreach ($DN2 as $dn) {
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
				if (!$success) $error = _('Could not delete user: ').$dn;
				break;
			case 'host':
				$success = ldap_delete($_SESSION['ldap']->server(), $dn);
				if (!$success) $error = _('Could not delete host: ').$dn;
				break;
			case 'group':
				$temp=explode(',', $dn);
				$groupname = str_replace('cn=', '', $temp[0]);
				$result = ldap_search($_SESSION['ldap']->server(), $dn, 'objectClass=*', array('gidNumber'));
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if ($attr2['gidNumber']==getgid($groupname)) $error = _('Could not delete group. Still users in group: ').$dn;
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				if (!$error) {
					if ($_SESSION['config']->scriptServer) remquotas($groupname, $_POST['type5']);
					$success = ldap_delete($_SESSION['ldap']->server(), $dn);
					if (!$success) $error = _('Could not delete group: ').$dn;
					}
				break;
			}
		if (!$error) echo $dn. _(' deleted.');
		 else echo $error;
		echo '</td></tr>'."\n".'<tr><td>';
		}
	}

if ($_POST['delete_no']) echo _('Nothing was deleted.');

echo '</td></tr>'."\n";
echo '</table></form></body></html>'."\n";
?>
