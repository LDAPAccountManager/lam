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

  This is the main window of the profile editor.

*/

include_once("../../lib/profiles.inc");
include_once("../../lib/ldap.inc");
include_once("../../lib/config.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// check if user has pressed submit or abort button
if ($_POST['forward'] == "yes") {
	// on abort go back to main page
	if ($_POST['abort']) {
		metaRefresh("../lists/listusers.php");
	}
	// on submit forward to other profile pages
	elseif ($_POST['submit']) {
		// create new user profile
		if ($_POST['profile'] == "newuser") {
			metaRefresh("profileuser.php");
		}
		// edit user profile
		elseif($_POST['profile'] == "edituser") {
			metaRefresh("profileuser.php?edit=" . $_POST['e_user']);
		}
		// delete user profile
		elseif($_POST['profile'] == "deluser") {
			metaRefresh("profiledelete.php?type=user&del=" . $_POST['d_user']);
		}
		if ($_SESSION['config']->is_samba3() || $_SESSION['config']->get_scriptPath()) {
			// create new group profile
			if ($_POST['profile'] == "newgroup") {
				metaRefresh("profilegroup.php");
			}
			// edit group profile
			elseif($_POST['profile'] == "editgroup") {
				metaRefresh("profilegroup.php?edit=" . $_POST['e_group']);
			}
			// delete group profile
			elseif($_POST['profile'] == "delgroup") {
				metaRefresh("profiledelete.php?type=group&del=" . $_POST['d_group']);
			}
		}
		// create new host profile
		if ($_POST['profile'] == "newhost") {
			metaRefresh("profilehost.php");
		}
		// edit host profile
		elseif($_POST['profile'] == "edithost") {
			metaRefresh("profilehost.php?edit=" . $_POST['e_host']);
		}
		// delete user profile
		elseif($_POST['profile'] == "delhost") {
			metaRefresh("profiledelete.php?type=host&del=" . $_POST['d_host']);
		}
	}
	exit;
}

// get list of user profiles and generate entries for dropdown box
$usrprof = getUserProfiles();
$userprofiles = "";
for ($i = 0; $i < sizeof($usrprof); $i++) {
	$userprofiles = $userprofiles . "<option>" . $usrprof[$i] . "</option>\n";
}

if ($_SESSION['config']->is_samba3() || $_SESSION['config']->get_scriptPath()) {
	// get list of group profiles and generate entries for dropdown box
	$grpprof = getGroupProfiles();
	$groupprofiles = "";
	for ($i = 0; $i < sizeof($grpprof); $i++) {
		$groupprofiles = $groupprofiles . "<option>" . $grpprof[$i] . "</option>\n";
	}
}

// get list of host profiles and generate entries for dropdown box
$hstprof = getHostProfiles();
$hostprofiles = "";
for ($i = 0; $i < sizeof($hstprof); $i++) {
	$hostprofiles = $hostprofiles . "<option>" . $hstprof[$i] . "</option>\n";
}

echo $_SESSION['header'];
?>

<html>
	<head>
		<title>LDAP Account Manager</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p></p>
		<form action="profilemain.php" method="post">

		<!-- user profile options -->
		<fieldset>
			<legend>
				<b><?php echo _("User Profiles"); ?></b>
			</legend>
			<table border=0>
				<!-- new user profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="newuser" checked>
					</td>
					<td colspan=2><?php echo _("Create a new User Profile"); ?></td>
				</tr>
				<!-- edit user profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="edituser">
					</td>
					<td>
						<select name="e_user" size=1>
							<?php echo $userprofiles ?>
						</select>
					</td>
					<td><?php echo _("Edit User Profile"); ?></td>
				</tr>
				<!-- delete user profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="deluser">
					</td>
					<td>
						<select name="d_user" size=1>
							<?php echo $userprofiles ?>
						</select>
					</td>
					<td><?php echo _("Delete User Profile"); ?></td>
				</tr>
			</table>
		</fieldset>

		<p></p>

<?php
if ($_SESSION['config']->is_samba3() || $_SESSION['config']->get_scriptPath()) {
	echo "		<!-- group profile options -->";
	echo "		<fieldset>";
	echo "			<legend>";
	echo "				<b>" . _("Group Profiles") . "</b>";
	echo "			</legend>";
	echo "			<table border=0>";
	echo "				<!-- new group profile -->";
	echo "				<tr>";
	echo "					<td>";
	echo "						<input type=\"radio\" name=\"profile\" value=\"newgroup\">";
	echo "					</td>";
	echo "					<td colspan=2>" . _("Create a new Group Profile") . "</td>";
	echo "				</tr>";
	if ($groupprofiles != "") {
		echo "				<!-- edit group profile -->";
		echo "				<tr>";
		echo "					<td>";
		echo "						<input type=\"radio\" name=\"profile\" value=\"editgroup\">";
		echo "					</td>";
		echo "					<td>";
		echo "						<select name=\"e_group\" size=1>";
		echo "							" . $groupprofiles;
		echo "						</select>";
		echo "					</td>";
		echo "					<td>" . _("Edit Group Profile") . "</td>";
		echo "				</tr>";
		echo "				<!-- delete group profile -->";
		echo "				<tr>";
		echo "					<td>";
		echo "						<input type=\"radio\" name=\"profile\" value=\"delgroup\">";
		echo "					</td>";
		echo "					<td>";
		echo "						<select name=\"d_group\" size=1>";
		echo "							" . $groupprofiles;
		echo "						</select>";
		echo "					</td>";
		echo "					<td>" . _("Delete Group Profile") . "</td>";
		echo "				</tr>";
	}
	echo "			</table>";
	echo "		</fieldset>";

	echo "		<p></p>";
}
?>

		<!-- host profile options -->
		<fieldset>
			<legend>
				<b><?php echo _("Samba Host Profiles"); ?></b>
			</legend>
			<table border=0>
				<!-- new host profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="newhost">
					</td>
					<td colspan=2><?php echo _("Create a new Samba Host Profile"); ?></td>
				</tr>
				<!-- edit host profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="edithost">
					</td>
					<td>
						<select name="e_host" size=1>
							<?php echo $hostprofiles ?>
						</select>
					</td>
					<td><?php echo _("Edit Samba Host Profile"); ?></td>
				</tr>
				<!-- delete host profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="delhost">
					</td>
					<td>
						<select name="d_host" size=1>
							<?php echo $hostprofiles ?>
						</select>
					</td>
					<td><?php echo _("Delete Samba Host Profile"); ?></td>
				</tr>
			</table>
		</fieldset>

		<p></p>

		<!-- forward is used to check if buttons were pressed -->
		<p>
		<input type="hidden" name="forward" value="yes">

		<input type="submit" name="submit" value="<?php echo _("Submit"); ?>">
		<input type="submit" name="abort" value="<?php echo _("Abort"); ?>">
		</p>

		</form>
	</body>
</html>
