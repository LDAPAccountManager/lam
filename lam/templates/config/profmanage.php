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


  Configuration profile management.
*/

include_once('../../lib/config.inc');
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

echo $_SESSION['header'];

?>

<html>
	<head>
		<title>
			<?php
				echo _("Profile management");
			?>
		</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p align="center"><a href="http://lam.sf.net" target="_blank">
			<img src="../../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</p>
		<hr><br>

<?php

// check if submit button was pressed
if ($_POST['submit']) {
	$cfg = new CfgMain();
	// check master password
	if ($cfg->password != $_POST['passwd']) {
		$error = _("Master password is wrong!");
	}
	// add new profile
	elseif ($_POST['action'] == "add") {
		if (eregi("^[a-z0-9\-_]+$", $_POST['addprofile']) && !in_array($_POST['addprofile'], getConfigProfiles())) {
			// check profile password
			if ($_POST['addpassword'] && $_POST['addpassword2'] && ($_POST['addpassword'] == $_POST['addpassword2'])) {
				// create new profile file
				@copy("../../config/lam.conf_sample", "../../config/" . $_POST['addprofile'] . ".conf");
				@chmod ("../../config/" . $_POST['addprofile'] . ".conf", 0600);
				$file = is_file("../../config/" . $_POST['addprofile'] . ".conf");
				if ($file) {
					// load as config and write new password
					$conf = new Config($_POST['addprofile']);
					$conf->Passwd = $_POST['addpassword'];
					$conf->save();
					$msg = _("Created new profile.");
				}
				else $error = _("Unable to create new profile!");
			}
			else $error = _("Profile passwords are different or empty!");
		}
		else $error = _("Profile name is invalid!");
	}
	// rename profile
	elseif ($_POST['action'] == "rename") {
		if (eregi("^[a-z0-9\-_]+$", $_POST['renfilename']) && !in_array($_POST['renprofile'], getConfigProfiles())) {
			if (rename("../../config/" . $_POST['oldfilename'] . ".conf",
				"../../config/" . $_POST['renfilename'] . ".conf")) {
				$msg = _("Renamed profile.");
			}
			else $error = _("Could not rename file!");
		}
		else $error = _("Profile name is invalid!");
	}
	// delete profile
	elseif ($_POST['action'] == "delete") {
		if (@unlink("../../config/" . $_POST['delfilename'] . ".conf")) {
			$msg = _("Profile deleted.");
		}
		else $error = _("Unable to delete profile!");
	}
	// set new profile password
	elseif ($_POST['action'] == "setpass") {
		if ($_POST['setpassword'] && $_POST['setpassword2'] && ($_POST['setpassword'] == $_POST['setpassword2'])) {
			$config = new Config($_POST['setprofile']);
			$config->set_Passwd($_POST['setpassword']);
			$config->save();
			$msg = _("New password set successfully.");
		}
		else $error = _("Profile passwords are different or empty!");
	}
	// set master password
	elseif ($_POST['action'] == "setmasterpass") {
		if ($_POST['masterpassword'] && $_POST['masterpassword2'] && ($_POST['masterpassword'] == $_POST['masterpassword2'])) {
			$config = new CfgMain();
			$config->password = $_POST['masterpassword'];
			$config->save();
			$msg = _("New master password set successfully.");
		}
		else $error = _("Master passwords are different or empty!");
	}
	// set default profile
	elseif ($_POST['action'] == "setdefault") {
		$config = new CfgMain();
		$config->default = $_POST['defaultfilename'];
		$config->save();
		$msg = _("New default profile set successfully.");
	}
	// print messages
	if ($error || $msg) {
		if ($error) StatusMessage("ERROR", "", $error);
		if ($msg) StatusMessage("INFO", "", $msg);
	}
	else exit;
}

?>

		<br>
		<!-- form for adding/renaming/deleting profiles -->
		<form action="profmanage.php" method="post">
		<fieldset>
			<legend><b> <?php echo _("Profile management"); ?> </b></legend>
			<p>
			<table border=0>

				<!-- add profile -->
				<tr>
					<td>
						<input type="radio" name="action" value="add" checked>
					</td>
					<td>
						<b>
							<?php echo _("Add profile:"); ?>
							&nbsp
						</b>
						&nbsp <?php echo _("Profile name"); ?> :
						<input type="text" name="addprofile">
					</td>
					<td>
						<a href="../help.php?HelpNumber=230" target="lamhelp"><?php echo _("Help"); ?></a>
					</td>
				</tr>
				<tr>
					<td></td>
					<td>
						<?php echo _("Profile password:"); ?>
						<input type="password" name="addpassword">
						&nbsp <?php echo _("Reenter profile password:"); ?>
						<input type="password" name="addpassword2">
					</td>
					<td></td>
				</tr>

				<tr>
					<td colspan=3>&nbsp</td>
				</tr>

				<!-- rename profile -->
				<tr>
					<td>
						<input type="radio" name="action" value="rename">
					</td>
					<td>
						<select size=1 name="oldfilename">
						<?php
							$files = getConfigProfiles();
							for ($i = 0; $i < sizeof($files); $i++) echo ("<option>" . $files[$i] . "</option>\n");
						?>
						</select>
						<b>
							<?php echo _("Rename profile to:"); ?>
							&nbsp
						</b>
						<input type="text" name="renfilename">
					</td>
					<td>
						<a href="../help.php?HelpNumber=230" target="lamhelp"><?php echo _("Help"); ?></a>
					</td>
				</tr>

				<tr>
					<td colspan=3>&nbsp</td>
				</tr>

				<!-- delete profile -->
				<tr>
					<td>
						<input type="radio" name="action" value="delete">
					</td>
					<td>
						<select size=1 name="delfilename">
						<?php
							$files = getConfigProfiles();
							for ($i = 0; $i < sizeof($files); $i++) echo ("<option>" . $files[$i] . "</option>\n");
						?>
						</select>
						<b>
							<?php echo _("Delete profile"); ?>
						</b>
					</td>
					<td>
						<a href="../help.php?HelpNumber=230" target="lamhelp"><?php echo _("Help"); ?></a>
					</td>
				</tr>

				<tr>
					<td colspan=3>&nbsp</td>
				</tr>

				<!-- set profile password -->
				<tr>
					<td>
						<input type="radio" name="action" value="setpass">
					</td>
					<td>
						<select size=1 name="setprofile">
						<?php
							$files = getConfigProfiles();
							for ($i = 0; $i < sizeof($files); $i++) echo ("<option>" . $files[$i] . "</option>\n");
						?>
						</select>
						<b>
							<?php echo _("Set profile password"); ?>
						</b>
					</td>
					<td>
						<a href="../help.php?HelpNumber=230" target="lamhelp"><?php echo _("Help"); ?></a>
					</td>
				</tr>
				<tr>
					<td></td>
					<td>
						<?php echo _("Profile password:"); ?>
						<input type="password" name="setpassword">
						&nbsp <?php echo _("Reenter profile password:"); ?>
						<input type="password" name="setpassword2">
					</td>
					<td></td>
				</tr>

				<tr>
					<td colspan=3>&nbsp</td>
				</tr>
				<tr>
					<td colspan=3>&nbsp</td>
				</tr>

				<!-- change default profile -->
				<tr>
					<td>
						<input type="radio" name="action" value="setdefault">
					</td>
					<td>
						<select size=1 name="defaultfilename">
						<?php
							$files = getConfigProfiles();
							$conf = new CfgMain();
							$defaultprofile = $conf->default;
							for ($i = 0; $i < sizeof($files); $i++) {
								if ($files[$i] == $defaultprofile) echo ("<option selected>" . $files[$i] . "</option>\n");
								else echo ("<option>" . $files[$i] . "</option>\n");
							}
						?>
						</select>
						<b>
							<?php echo _("Change default profile"); ?>
						</b>
					</td>
					<td>
						<a href="../help.php?HelpNumber=230" target="lamhelp"><?php echo _("Help"); ?></a>
					</td>
				</tr>

				<!-- set master password -->
				<tr>
					<td>
						<input type="radio" name="action" value="setmasterpass">
					</td>
					<td>
						<b>
							<?php echo _("Change master password"); ?>
						</b>
						&nbsp <?php echo _("New master password:"); ?>
						<input type="password" name="masterpassword">
						&nbsp <?php echo _("Reenter new master password:"); ?>
						<input type="password" name="masterpassword2">
					</td>
					<td>
						<a href="../help.php?HelpNumber=230" target="lamhelp"><?php echo _("Help"); ?></a>
					</td>
				</tr>

			</table>
			</fieldset>
			<p>&nbsp</p>

			<!-- password field and submit button -->
			<b>
				<?php echo _("Master Password:"); ?>
			</b>
			&nbsp
			<input type="password" name="passwd">
			&nbsp
			<input type="submit" name="submit" value=" <?php echo _("Submit"); ?> ">

		</form>
		<p><br></p>

		<!-- back to login page -->
		<p>
			<a href="conflogin.php"> <?php echo _("Back to profile login"); ?> </a>
		</p>

	</body>
</html>

