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
include_once('../lib/config.inc');
session_save_path('../sess');
@session_start();


echo '<html><head><title>';
echo _('Delete Account');
echo '</title>
	</head><body>
	<link rel="stylesheet" type="text/css" href="../style/delete.css">
	<form action="account.php" method="get">
	<meta http-equiv="pragma" content="no-cache">
	<meta http-equiv="cache-control" content="no-cache">
	<table rules="all" class="delete" width="100%">
	<tr><td>';


if ($type) {
	$DN = str_replace("\'", '',$DN);
	$DN2 = explode(";", $DN);
	foreach ($DN2 as $dn) {
		switch ($type) {
			case 'user':
				$success = ldap_delete($_SESSION['ldap']->server(), $dn);
				if (!$success) $error = _('Could not delete user: ').$dn;
				break;
			case 'host':
				$success = ldap_delete($_SESSION['ldap']->server(), $dn);
				if (!$success) $error = _('Could not delete host: ').$dn;
				break;
			case 'group':
				$result = ldap_search($_SESSION['ldap']->server(), $dn, 'objectClass=*');
				if (!$result) $error = _('Could not delete group: ').$dn;
				$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
				$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
				if ($attr['memberUid']) $error = _('Could not delete group. Still users in group: ').$dn;
				    else {
					$success = ldap_delete($_SESSION['ldap']->server(), $dn);
					if (!$success) $error = _('Could not delete user: ').$dn;
					}
				break;
			}
		if (!$error) echo $dn. _(' deleted.');
		 else echo $error;
		echo '</td></tr><tr><td>';
		}
	}
echo '</td></tr>';
echo '</form></body></html>';
?>
