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


  Start page of configuration wizard.
*/

include_once('../../lib/config.inc');
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check if user clicked on buttons
if ($_POST['submit'] || $_POST['cancel']) {
	unset($error);
	if ($_POST['cancel']) {
		metarefresh('../config/conflogin.php');
	}
	else {
		// check if master password is correct
		$cfg = new CfgMain();
		if ($cfg->password != $_POST['masterpwd']) {
			$error = _("Master password is wrong!");
		}
		// check if passwords are equal and not empty
		elseif ($_POST['passwd1'] && ($_POST['passwd1'] != "") && ($_POST['passwd1'] == $_POST['passwd2'])) {
			// check if profile name is valid
			if (eregi("^[a-z0-9\-_]+$", $_POST['profname']) && !in_array($_POST['profname'], getConfigProfiles())) {
				// create new profile file
				@copy("../../config/lam.conf_sample", "../../config/" . $_POST['profname'] . ".conf");
				@chmod ("../../config/" . $_POST['profname'] . ".conf", 0600);
				$file = is_file("../../config/" . $_POST['profname'] . ".conf");
				if ($file) {
					// load as config and write new password
					$conf = new Config($_POST['profname']);
					$conf->Passwd = $_POST['passwd1'];
					$conf->save();
					$_SESSION['confwiz_config'] = $conf;
					$_SESSION['confwiz_masterpwd'] = $_POST['masterpwd'];
				}
				else $error = _("Unable to create new profile!");
			}
			else $error = _("Profile name is invalid!");
		}
		else $error = _("Profile passwords are different or empty!");
		// print error message if needed
		if ($error) {
			echo $_SESSION['header'];
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
			echo "<title>\n";
				echo _("Configuration wizard");
			echo "</title>\n";
			echo "</head><body>\n";
			StatusMessage("ERROR", $error, "");
			echo "<p><br><br><a href=\"../config/conflogin.php\">" . _("Back to profile login") . "</a></p>\n";
			echo "</body></html>\n";
		}
		// if all ok, go to next page
		else {
			metarefresh('server.php');
		}
	}
	exit;
}

// remove variables of older wizard calls
unset($_SESSION['conwiz_masterpwd']);
unset($_SESSION['confwiz_config']);

echo $_SESSION['header'];

	echo "<title>\n";
		echo _("Configuration wizard");
	echo "</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
echo "<body>\n";
	echo "<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\">\n";
	echo "<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a>\n";
	echo "</p>\n";
	echo "<hr>\n";

// formular
echo "<form action=\"start.php\" method=\"post\">\n";
	echo "<h2 align=\"center\">" . _("Welcome to LAM Configuration wizard.") . "</h2>\n";
	echo "<p align=\"center\">\n";
		echo "This druid will help you to create a configuration file for LAM and set up LDAP.\n";
	echo "</p>\n";
	echo "<br><br>\n";
	echo "<table border=0>\n";

	// profile name
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("Please enter a name for the new profile. The name may contain letters, digits and -_.") . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Profile name") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"text\" name=\"profname\">\n";
		echo "</td>\n";
	echo "</tr>\n";

	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";

	// password
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("Configuration profiles are protected with a password from unauthorised access. Please enter it here.") . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Password") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"password\" name=\"passwd1\">\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Reenter Password") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"password\" name=\"passwd2\">\n";
		echo "</td>\n";
	echo "</tr>\n";

	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";

	// master password
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("Please enter your configuration master password. This password is \"lam\" by default.") . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Master password") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<input type=\"password\" name=\"masterpwd\">\n";
		echo "</td>\n";
	echo "</tr>\n";

	echo "</table>\n";

	echo "<p><br></p>\n";

// next/cancel button
	echo "<p>\n";
		echo "<input type=\"submit\" name=\"submit\" value=\"" . _("Next") . "\">\n";
		echo "<input type=\"submit\" name=\"cancel\" value=\"" . _("Cancel") . "\">\n";
	echo "</p>\n";

echo "</form>\n";

echo "</body>\n</html>\n";

?>
