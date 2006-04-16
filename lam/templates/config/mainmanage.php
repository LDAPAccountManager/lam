<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Roland Gruber

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

*/

  
/**
* Manages the main configuration options.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');
/** Used to print status messages */
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$cfg = new CfgMain();

// check if user is logged in
if (!isset($_SESSION["mainconf_password"]) || ($_SESSION["mainconf_password"] != $cfg->password)) {
	require('mainlogin.php');
	exit();
}

echo $_SESSION['header'];

?>

		<title>
			<?php
				echo _("Edit general settings");
			?>
		</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p align="center"><a href="http://lam.sourceforge.net" target="_blank">
			<img src="../../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</p>
		<hr><br>

<?php

// check if submit button was pressed
if ($_POST['submit']) {
	// set master password
	if (isset($_POST['masterpassword']) && ($_POST['masterpassword'] != "")) {
		if ($_POST['masterpassword'] && $_POST['masterpassword2'] && ($_POST['masterpassword'] == $_POST['masterpassword2'])) {
			$cfg->password = $_POST['masterpassword'];
			$cfg->save();
			$msg = _("New master password set successfully.");
			unset($_SESSION["mainconf_password"]);
		}
		else $error = _("Master passwords are different or empty!");
	}
	else {
		$msg = _("No changes were made.");
	}
	// print messages
	if ($error || $msg) {
		if ($error) StatusMessage("ERROR", "", $error);
		if ($msg) {
			StatusMessage("INFO", "", $msg);
			// back to login page
			echo "<p><a href=\"../login.php\">" . _("Back to login") . "</a></p>";
			exit();
		}
	}
	else exit;
}
?>

		<br>
		<!-- form for adding/renaming/deleting profiles -->
		<form action="mainmanage.php" method="post">
		<table border="0">
		<tr><td>
		<fieldset>
			<legend><b> <?php echo _("Change master password"); ?> </b></legend>
			<p>
			<table cellspacing="0" border="0">
				<!-- set master password -->
				<tr>
					<td align="right">
						<FONT color="Red"><B>
						<?php echo _("New master password") . ":"; ?>
						</B></FONT>
						<input type="password" name="masterpassword">
					</td>
					<td>&nbsp;
					<?PHP
						// help link
						echo "<a href=\"../help.php?HelpNumber=235\" target=\"lamhelp\">";
						echo "<img src=\"../../graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
						echo "</a>\n";
					?>
					</td>
				</tr>
				<tr>
					<td align="right">
						<FONT color="Red"><B>
						<?php echo _("Reenter new master password") . ":"; ?>
						</B></FONT>
						<input type="password" name="masterpassword2">
					</td>
					<td>&nbsp;</td>
				</tr>

			</table>
			</fieldset>
			</td></tr>
			</table>
			<BR>
			
			<input type="submit" name="submit" value=" <?php echo _("Ok"); ?> ">
			
		</form>
		<p><br></p>

		<!-- back to login page -->
		<p>
			<a href="../login.php"> <?php echo _("Back to login"); ?> </a>
		</p>

	</body>
</html>

