<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Dürgner

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

  This is the main window of the pdf structure editor.

*/

include_once("../../lib/pdfstruct.inc");
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
	// on submit forward to other pdf structure pages
	else if($_POST['submit']) {
		// create new user pdf structure
		if ($_POST['pdf'] == "new_user") {
			metaRefresh("pdfpage.php?type=user");
		}
		// edit user pdf structure
		elseif($_POST['pdf'] == "edit_user") {
			metaRefresh("pdfpage.php?type=user&amp;edit=" . $_POST['edit_user']);
		}
		// delete user pdf structure
		elseif($_POST['pdf'] == "delete_user") {
			metaRefresh("pdfdelete.php?type=user&amp;delete=" . $_POST['delete_user']);
		}
		// create new group pdf structure
		elseif ($_POST['pdf'] == "new_group") {
			metaRefresh("pdfpage.php?type=group");
		}
		// edit group pdf structure
		elseif($_POST['pdf'] == "edit_group") {
			metaRefresh("pdfpage.php?type=group&amp;edit=" . $_POST['edit_group']);
		}
		// delete group pdf structure
		elseif($_POST['pdf'] == "delete_group") {
			metaRefresh("pdfdelete.php?type=group&amp;delete=" . $_POST['delete_group']);
		}
		// create new host pdf structure
		elseif ($_POST['pdf'] == "new_host") {
			metaRefresh("pdfpage.php?type=host");
		}
		// edit host pdf structure
		elseif($_POST['pdf'] == "edit_host") {
			metaRefresh("pdfpage.php?type=host&amp;edit=" . $_POST['edit_host']);
		}
		// delete host pdf structure
		elseif($_POST['pdf'] == "delete_host") {
			metaRefresh("pdfdelete.php?type=host&amp;delete=" . $_POST['delete_host']);
		}
	}
	exit;
}

// Get available user PDF structure definitions
$pdfStructDefs = getPDFStructureDefinitions('user');
$user_pdf = '';
for($i = 0;$i < count($pdfStructDefs); $i++) {
	$user_pdf .= '<option value="' . $pdfStructDefs[$i] . '.xml">' . $pdfStructDefs[$i] . "</option>\n";
}

// Get available group PDF structure definitions
$pdfStructDefs = getPDFStructureDefinitions('group');
$group_pdf = '';
for($i = 0;$i < count($pdfStructDefs); $i++) {
	$group_pdf .= '<option value="' . $pdfStructDefs[$i] . '.xml">' . $pdfStructDefs[$i] . "</option>\n";
}

// Get available host PDF structure definitions
$pdfStructDefs = getPDFStructureDefinitions('host');
$host_pdf = '';
for($i = 0;$i < count($pdfStructDefs); $i++) {
	$host_pdf .= '<option value="' . $pdfStructDefs[$i] . '.xml">' . $pdfStructDefs[$i] . "</option>\n";
}

echo $_SESSION['header'];
?>
		<title>LDAP Account Manager</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p></p>
		<form action="pdfmain.php" method="post">
		<!-- user pdf structure options -->
		<fieldset>
			<legend>
				<b><?php echo _("User PDF structures"); ?></b>
			</legend>
			<table border=0>
				<!-- new user pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="new_user" checked>
					</td>
					<td colspan=2><?php echo _("Create a new user PDF structure"); ?></td>
				</tr>
				<!-- edit user pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="edit_user">
					</td>
					<td>
						<select name="edit_user" size=1>
							<?php echo $user_pdf ?>
						</select>
					</td>
					<td><?php echo _("Edit user PDF structure"); ?></td>
				</tr>
				<!-- delete user pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="delete_user">
					</td>
					<td>
						<select name="delete_user" size=1>
							<?php echo $user_pdf ?>
						</select>
					</td>
					<td><?php echo _("Delete user PDF structure"); ?></td>
				</tr>
			</table>
		</fieldset>
		<p></p>
		
		<!-- group pdf structure options -->
		<fieldset>
			<legend>
				<b><?php echo _("Group PDF structures"); ?></b>
			</legend>
			<table border=0>
				<!-- new group pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="new_group">
					</td>
					<td colspan=2><?php echo _("Create a new group PDF structure"); ?></td>
				</tr>
				<!-- edit group pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="edit_group">
					</td>
					<td>
						<select name="edit_group" size=1>
							<?php echo $group_pdf ?>
						</select>
					</td>
					<td><?php echo _("Edit group PDF structure"); ?></td>
				</tr>
				<!-- delete group pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="delete_group">
					</td>
					<td>
						<select name="delete_group" size=1>
							<?php echo $group_pdf ?>
						</select>
					</td>
					<td><?php echo _("Delete group PDF structure"); ?></td>
				</tr>
			</table>
		</fieldset>
		<p></p>
		
		<!-- host pdf structure options -->
		<fieldset>
			<legend>
				<b><?php echo _("Host PDF structures"); ?></b>
			</legend>
			<table border=0>
				<!-- new host pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="new_host">
					</td>
					<td colspan=2><?php echo _("Create a new host PDF structure"); ?></td>
				</tr>
				<!-- edit host pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="edit_host">
					</td>
					<td>
						<select name="edit_host" size=1>
							<?php echo $host_pdf ?>
						</select>
					</td>
					<td><?php echo _("Edit host PDF structure"); ?></td>
				</tr>
				<!-- delete host pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="delete_host">
					</td>
					<td>
						<select name="delete_host" size=1>
							<?php echo $host_pdf ?>
						</select>
					</td>
					<td><?php echo _("Delete host PDF structure"); ?></td>
				</tr>
			</table>
		</fieldset>
		
		<!-- forward is used to check if buttons were pressed -->
		<p>
		<input type="hidden" name="forward" value="yes">

		<input type="submit" name="submit" value="<?php echo _("Submit"); ?>">
		<input type="submit" name="abort" value="<?php echo _("Abort"); ?>">
		</p>

		</form>
	</body>
</html>