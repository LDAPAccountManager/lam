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


  Configuration wizard - server settings
*/

include_once('../../lib/config.inc');
include_once('../../lib/ldap.inc');
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check master password
$cfg = new CfgMain();
if ($cfg->password != $_SESSION['confwiz_masterpwd']) {
	require("../config/conflogin.php");
	exit;
}


// check if user clicked on buttons
if ($_POST['submit'] || $_POST['cancel']) {
	unset($error);
	unset($ret);
	if ($_POST['cancel']) {
		@unlink("../../config/" . $_SESSION['confwiz_config']->file . ".conf");
		metarefresh('../config/conflogin.php');
	}
	else {
		// check server URL
		if ($_SESSION['confwiz_config']->set_serverURL($_POST['serverurl'])) {
			// set Samba version
			if ($_POST['sambaversion'] == "2") $_SESSION['confwiz_config']->set_samba3("no");
			else $_SESSION['confwiz_config']->set_samba3("yes");
			$_SESSION['confwiz_config']->set_Adminstring($_POST['ldapadmin']);
			// save settings
			$_SESSION['confwiz_config']->save();
			// create LDAP object and test connection
			$_SESSION['confwiz_ldap'] = new Ldap($_SESSION['confwiz_config']);
			$ret = $_SESSION['confwiz_ldap']->connect($_POST['ldapadmin'], $_POST['ldappwd']);
			if ($ret === 0) {
				metarefresh('server2.php');
			}
			elseif ($ret === False) $error = _("Cannot connect to specified LDAP-Server. Please try again.");
			elseif ($ret == 81) $error = _("Cannot connect to specified LDAP-Server. Please try again.");
			elseif ($ret == 49) $error = _("Wrong Password/Username combination. Try again.");
			else $error = _("LDAP error, server says:") . "\n<br>($ret) " . ldap_err2str($ret);
		}
		else {
			$error = _("Server Address is empty!");
		}
		// print error message if needed
		if ($error) {
			echo $_SESSION['header'];
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
			echo "<title>\n";
				echo _("Configuration wizard");
			echo "</title>\n";
			echo "</head><body>\n";
			StatusMessage("ERROR", $error, "");
			echo "<p><br><br><a href=\"server.php\">" . _("Back to last page") . "</a></p>\n";
			echo "</body></html>\n";
		}
		// if all ok, go to next page
		else {
			metarefresh('server2.php');
		}
	}
	exit;
}

// check if back button was pressed
$back = false;
if ($_GET['back'] || $_POST['back']) {
	$back = true;
	$auth = $_SESSION['confwiz_ldap']->decrypt();
}

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
echo "<form action=\"server.php\" method=\"post\">\n";
	echo "<br><br>\n";
	echo "<table border=0>\n";

	// server URL
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("Please enter the URL of your LDAP server.") . "<br><br><b>" .
					 _("Examples") . ":</b><br><br>ldap://myserver.mydomain.org<br>ldaps:myserver.mydomain.org<br>localhost:389" . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Server address") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			if ($back) echo "<input type=\"text\" name=\"serverurl\" value =\"" . $_SESSION['confwiz_config']->get_ServerURL() . "\">\n";
			else echo "<input type=\"text\" name=\"serverurl\">\n";
		echo "</td>\n";
	echo "</tr>\n";

	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";

	// admin user+password
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("To connect to your LDAP server please enter now the DN of your administrative user and the password.") . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("LDAP admin DN") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			if ($back) echo "<input type=\"text\" name=\"ldapadmin\" value=\"" . $auth[0] . "\">\n";
			else echo "<input type=\"text\" name=\"ldapadmin\">\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Password") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			if ($back) echo "<input type=\"password\" name=\"ldappwd\" value=\"" . $auth[1] . "\">\n";
			else echo "<input type=\"password\" name=\"ldappwd\">\n";
		echo "</td>\n";
	echo "</tr>\n";

	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";

	// master password
	echo "<tr>\n";
		echo "<td colspan=2>\n";
			echo _("Which Samba version do you use?") . "\n";
		echo "</td>\n";
	echo "</tr>\n";
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
	echo "<tr>\n";
		echo "<td>\n";
			echo "<b>" . _("Samba version") . ":</b>\n";
		echo "</td>\n";
		echo "<td>\n";
			echo "<select name=\"sambaversion\">\n";
				echo "<option value=3>3</option>";
				if ($back && !$_SESSION['confwiz_config']->is_samba3()) echo "<option selected value=2>2.x</option>";
				else echo "<option value=2>2.x</option>";
			echo "</select>\n";
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
