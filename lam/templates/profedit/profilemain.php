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

// start session
session_save_path("../../sess");
@session_start();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	echo("<meta http-equiv=\"refresh\" content=\"0; URL=../login.php\">");
	exit;
}

// check if user has pressed submit or abort button
if ($_POST['forward'] == "yes") {
	// on abort go back to main page
	if ($_POST['abort']) {
		echo("<meta http-equiv=\"refresh\" content=\"0; URL=../lists/listusers.php\">");
	}
	// on submit forward to other profile pages
	elseif ($_POST['submit']) {
		// create new user profile
		if ($_POST['profile'] == "newuser") {
			echo("<meta http-equiv=\"refresh\" content=\"0; URL=profileuser.php\">");
		}
		// edit user profile
		elseif($_POST['profile'] == "edituser") {
			echo("<meta http-equiv=\"refresh\" content=\"0; URL=profileuser.php?edit=" . $_POST['e_user'] . "\">");
		}
		// delete user profile
		elseif($_POST['profile'] == "deluser") {
			echo("<meta http-equiv=\"refresh\" content=\"0; URL=profiledelete.php?type=user&del=" . $_POST['d_user'] . "\">");
		}
		// create new host profile
		elseif ($_POST['profile'] == "newhost") {
			echo("<meta http-equiv=\"refresh\" content=\"0; URL=profilehost.php\">");
		}
		// edit host profile
		elseif($_POST['profile'] == "edithost") {
			echo("<meta http-equiv=\"refresh\" content=\"0; URL=profilehost.php?edit=" . $_POST['e_host'] . "\">");
		}
		// delete user profile
		elseif($_POST['profile'] == "delhost") {
			echo("<meta http-equiv=\"refresh\" content=\"0; URL=profiledelete.php?type=host&del=" . $_POST['d_host'] . "\">");
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

// get list of host profiles and generate entries for dropdown box
$hstprof = getHostProfiles();
$hostprofiles = "";
for ($i = 0; $i < sizeof($hstprof); $i++) {
	$hostprofiles = $hostprofiles . "<option>" . $hstprof[$i] . "</option>\n";
}

echo ("<?xml version=\"1.0\" encoding=\"ISO-8859-15\"?>\n");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
?>

<html>
	<head>
		<title>LDAP Account Manager</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<br>
		<form action="profilemain.php" method="post">

		<!-- user profile options -->
		<fieldset>
			<legend>
				<b> <? echo _("User Profiles"); ?> </b>
			</legend>
			<table align="left" border=0>
				<!-- new user profile -->
				<tr>
					<td>
						<input checked type="radio" name="profile" value="newuser">
					</td>
					<td colspan=2>
						<? echo _("Create a new User Profile"); ?>
					</td>
				</tr>
				<!-- edit user profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="edituser">
					</td>
					<td>
						<select name="e_user" size=1>
							<? echo $userprofiles ?>
						</select>
					</td>
					<td>
						<? echo _("Edit User Profile"); ?>
					</td>
				</tr>
				<!-- delete user profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="deluser">
					</td>
					<td>
						<select name="d_user" size=1>
							<? echo $userprofiles ?>
						</select>
					</td>
					<td>
						<? echo _("Delete User Profile"); ?>
					</td>
				</tr>
			</table>
		</fieldset>

		<br>

		<!-- host profile options -->
		<fieldset>
			<legend>
				<b> <? echo _("Samba Host Profiles"); ?> </b>
			</legend>
			<table align="left" border=0>
				<!-- new host profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="newhost">
					</td>
					<td colspan=2>
						<? echo _("Create a new Samba Host Profile"); ?>
					</td>
				</tr>
				<!-- edit host profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="edithost">
					</td>
					<td>
						<select name="e_host" size=1>
							<? echo $hostprofiles ?>
						</select>
					</td>
					<td>
						<? echo _("Edit Samba Host Profile"); ?>
					</td>
				</tr>
				<!-- delete host profile -->
				<tr>
					<td>
						<input type="radio" name="profile" value="delhost">
					</td>
					<td>
						<select name="d_host" size=1>
							<? echo $hostprofiles ?>
						</select>
					</td>
					<td>
						<? echo _("Delete Samba Host Profile"); ?>
					</td>
				</tr>
			</table>
		</fieldset>

		<br>
		<br>

		<!-- forward is used to check if buttons were pressed -->
		<input type="hidden" name="forward" value="yes">

		<input type="submit" name="submit" value="<? echo _("Submit"); ?>">
		<input type="submit" name="abort" value="<? echo _("Abort"); ?>">

		</form>
	</body>
</html>
