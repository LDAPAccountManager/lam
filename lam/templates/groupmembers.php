<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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

  Add and remove users from a specific group.

*/
include_once ("../lib/config.inc");
include_once ("../lib/ldap.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();

echo $_SESSION['header'];

echo "<html>\n";
echo "<head>\n";
echo "<title></title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "</head>\n";
echo "<body>\n";

// remove \' from DN string
$_GET['DN'] = str_replace("\'", "", $_GET['DN']);

// add/remove users
if ($_POST['add'] || $_POST['remove']) {
	// search group and its members
	$members = array();
	$DN = $_GET['DN'];
	$filter = "(objectClass=posixGroup)";
	$attrs = array("memberUID", "cn");
	$sr = @ldap_search($_SESSION["ldap"]->server(), $DN, $filter, $attrs);
	if ($sr) {
		$grp_info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
		ldap_free_result($sr);
		if ($grp_info['count'] != 0) {
			$members = $grp_info[0]['memberuid'];
			// delete count entry
			if ($members) array_shift($members);
			else $members = array();
		}
	}
	// add users
	if ($_POST['add']) {
		$addmembers = $_POST['users'];
		if (!$addmembers) $addmembers = array();
		if (sizeof($addmembers) > 0) {
			// search new members
			for ($i = 0; $i < sizeof($addmembers); $i++) {
				if (!in_array($addmembers[$i], $members)) $members[] = $addmembers[$i];
			}
			$mod = array();
			$mod['memberuid'] = $members;
			// change LDAP entry
			if (ldap_modify($_SESSION["ldap"]->server(), $DN, $mod)) {
				StatusMessage("INFO", _("Added users:"), implode("; ", $addmembers));
			}
			else {
				StatusMessage("ERROR", _("Unable to add users!"), "");
			}
		}
	}
	// remove users
	if ($_POST['remove']) {
		$delmembers = $_POST['members'];
		if (!$delmembers) $delmembers = array();
		if (sizeof($delmembers) > 0) {
			$mod = array();
			// search remaining users
			for ($i = 0; $i < sizeof($members); $i++) {
				if (!in_array($members[$i], $delmembers)) $mod['memberuid'][] =  $members[$i];
			}
			// remove selected users
			if (sizeof($mod['memberuid']) > 0) {
				if (ldap_modify($_SESSION["ldap"]->server(), $DN, $mod)) {
					StatusMessage("INFO", _("Removed users:"), implode("; ", $delmembers));
				}
				else {
					StatusMessage("ERROR", _("Unable to remove users!"), "");
				}
			}
			// remove all users
			else {
				$mod = array();
				$mod['memberuid'] = $delmembers;
				if (ldap_mod_del($_SESSION["ldap"]->server(), $DN, $mod)) {
					StatusMessage("INFO", _("Removed users:"), implode("; ", $delmembers));
				}
				else {
					StatusMessage("ERROR", _("Unable to remove users!"), "");
				}
			}
		}
	}
}

// search group and its members
$DN = $_GET['DN'];
$filter = "(objectClass=posixGroup)";
$attrs = array("memberUID", "cn");
$sr = @ldap_search($_SESSION["ldap"]->server(), $DN, $filter, $attrs);
if ($sr) {
	$grp_info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
	ldap_free_result($sr);
	if ($grp_info["count"] == 0) StatusMessage("WARN", "", _("Unable to find group!"));
	// use first search result
	$grp_info = $grp_info[0];

	echo "<p>&nbsp;</p>\n";

	// title
	echo "<p align=\"center\"><big><b>";
	echo _("Edit group members of:") . " " . $grp_info["cn"][0];
	echo "</b></big></p>\n";

	echo "<p>&nbsp;</p>\n";

	echo "<form action=\"groupmembers.php?DN='" . $_GET['DN'] . "'\" method=\"post\">\n";

	echo "<p align=\"center\">\n";
	echo "<table width=800 border=0>\n";
		// table header
		echo "<tr>\n";
			echo "<th width=370>" . _("Group members") . "</th>\n";
			echo "<th width=60></th>\n";
			echo "<th width=370>" . _("Available users") . "</th>\n";
		echo "</tr>\n";
		echo "<tr>\n";
			// group members
			echo "<td>\n";
				$members = $grp_info["memberuid"];
				if (!$members) $members = array();
				array_shift($members);
				sort($members);
				echo "<select style=\"width:100%\" name=\"members[]\" size=20 multiple>\n";
				for ($i = 0; $i < sizeof($members); $i++) {
					echo "<option>\n";
					echo $members[$i];
					echo "</option>\n";
				}
				echo "</select>\n";
			echo "</td>\n";
			// add/remove users
			echo "<td align=\"center\">\n";
				echo "<input type=\"submit\" value=\"<=\" name=\"add\">\n";
				echo "<br><br>\n";
				echo "<input type=\"submit\" value=\"=>\" name=\"remove\">\n";
			echo "</td>\n";
			// available users
			echo "<td>\n";
				// search available users
				$DN = $_SESSION['config']->get_UserSuffix();
				if ($_SESSION['config']->get_samba3() == "yes") {
					$filter = "(&(objectClass=posixAccount)(objectClass=sambaSamAccount))";
				}
				else $filter = "(&(objectClass=posixAccount)(objectClass=sambaAccount))";
				$attrs = array("uid");
				$sr = @ldap_search($_SESSION["ldap"]->server(), $DN, $filter, $attrs);
				if ($sr) {
					$usr_info = ldap_get_entries($_SESSION["ldap"]->server, $sr);
					ldap_free_result($sr);
					// delete count entry
					array_shift($usr_info);
					echo "<select style=\"width:100%\" name=\"users[]\" size=20 multiple>\n";
					$users = array();
					// extract user names
					for ($i = 0; $i < sizeof($usr_info); $i++) {
						$users[] = $usr_info[$i]["uid"][0];
					}
					sort($users);
					for ($i = 0; $i < sizeof($users); $i++) {
						// show only users who are not already in group
						if (!in_array($users[$i], $members)) {
							echo "<option>\n";
							echo $users[$i];
							echo "</option>\n";
						}
					}
					echo "</select>\n";
				}
				// show empty box if no users were found
				else {
					echo "<select style=\"width:100%\" name=\"users[]\" size=20 multiple>\n";
					echo "</select>\n";
				}
			echo "</td>\n";
		echo "</tr>\n";
	echo "</table>\n";
	echo "</p>\n";

	echo "</form>\n";
}
else StatusMessage("ERROR", "Unable to find group!", "");


echo "</body>";
echo "</html>";


?>