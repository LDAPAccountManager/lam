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

  This is an editor for orgnizational units.

*/

include_once ("../lib/config.inc");
include_once ("../lib/ldap.inc");
include_once ("../lib/status.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();

// check if submit button was pressed
if ($_POST['submit']) {
	// user operations
	// new user ou
	if ($_POST['type'] == "new_usr") {
		// create ou if valid
		if (eregi("^[a-z0-9 _\\-]+$", $_POST['newsuff_u'])) {
			// check if ou already exists
			$new_dn = "ou=" . $_POST['newsuff_u'] . "," . $_POST['usersuff_n'];
			if (!in_array(strtolower($new_dn), $_SESSION['ldap']->search_units($_POST['usersuff_n']))) {
				// add new ou
				$ou = array();
				$ou['objectClass'] = "organizationalunit";
				$ou['ou'] = $_POST['newsuff_u'];
				$ret = @ldap_add($_SESSION['ldap']->server(), $new_dn, $ou);
				if ($ret) {
					$message = _("New OU created successfully.");
				}
				else {
					$error = _("Unable to create new OU!");
				}
			}
			else $error = _("OU already exists!");
		}
		// show errormessage if ou is invalid
		else {
			$error = _("OU is invalid!") . " " . $_POST['newsuff_u'];
		}
	}
	// delete ou, user was sure
	elseif (($_POST['type'] == "del_usr") && ($_POST['sure'])) {
		$ret = @ldap_delete($_SESSION['ldap']->server(), $_POST['usersuff_d']);
		if ($ret) {
			$message = _("OU deleted successfully.");
		}
		else {
			$error = _("Unable to delete OU!");
		}
	}
	// do not delete ou
	elseif (($_POST['type'] == "del_usr") && ($_POST['abort'])) {
		display_main();
		exit;
	}
	// ask if user is sure to delete
	elseif ($_POST['type'] == "del_usr") {
		// check for sub entries
		$sr = @ldap_list($_SESSION['ldap']->server(), $_POST['usersuff_d'], "ObjectClass=*", array(""));
		$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($sr && $info['count'] == 0) {
			$text = "<br>\n" .
				"<p><big><b>" . _("Do you really want to delete this OU?") . " </b></big>" . "\n" .
				"<br>\n<p>" . $_POST['usersuff_d'] . "</p>\n" .
				"<br>\n" .
				"<form action=\"ou_edit.php?type=user\" method=\"post\">\n" .
				"<input type=\"hidden\" name=\"type\" value=\"del_usr\">\n" .
				"<input type=\"hidden\" name=\"submit\" value=\"submit\">\n" .
				"<input type=\"hidden\" name=\"usersuff_d\" value=\"" . $_POST['usersuff_d'] . "\">\n" .
				"<input type=\"submit\" name=\"sure\" value=\"" . _("Submit") . "\">\n" .
				"<input type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\">\n" .
				"</form>";
		}
		else {
			$error = _("OU is not empty or invalid!");
		}
	}

	// group operations
	// new group ou
	if ($_POST['type'] == "new_grp") {
		// create ou if valid
		if (eregi("^[a-z0-9 _\\-]+$", $_POST['newsuff_g'])) {
			// check if ou already exists
			$new_dn = "ou=" . $_POST['newsuff_g'] . "," . $_POST['groupsuff_n'];
			if (!in_array(strtolower($new_dn), $_SESSION['ldap']->search_units($_POST['groupsuff_n']))) {
				// add new ou
				$ou = array();
				$ou['objectClass'] = "organizationalunit";
				$ou['ou'] = $_POST['newsuff_g'];
				$ret = @ldap_add($_SESSION['ldap']->server(), $new_dn, $ou);
				if ($ret) {
					$message = _("New OU created successfully.");
				}
				else {
					$error = _("Unable to create new OU!");
				}
			}
			else $error = _("OU already exists!");
		}
		// show errormessage if ou is invalid
		else {
			$error = _("OU is invalid!") . " " . $_POST['newsuff_g'];
		}
	}
	// delete ou, user was sure
	elseif (($_POST['type'] == "del_grp") && ($_POST['sure'])) {
		$ret = @ldap_delete($_SESSION['ldap']->server(), $_POST['groupsuff_d']);
		if ($ret) {
			$message = _("OU deleted successfully.");
		}
		else {
			$error = _("Unable to delete OU!");
		}
	}
	// do not delete ou
	elseif (($_POST['type'] == "del_grp") && ($_POST['abort'])) {
		display_main();
		exit;
	}
	// ask if user is sure to delete
	elseif ($_POST['type'] == "del_grp") {
		// check for sub entries
		$sr = @ldap_list($_SESSION['ldap']->server(), $_POST['groupsuff_d'], "ObjectClass=*", array(""));
		$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($sr && $info['count'] == 0) {
			$text = "<br>\n" .
				"<p><big><b>" . _("Do you really want to delete this OU?") . " </b></big>" . "\n" .
				"<br>\n<p>" . $_POST['groupsuff_d'] . "</p>\n" .
				"<br>\n" .
				"<form action=\"ou_edit.php?type=group\" method=\"post\">\n" .
				"<input type=\"hidden\" name=\"type\" value=\"del_grp\">\n" .
				"<input type=\"hidden\" name=\"submit\" value=\"submit\">\n" .
				"<input type=\"hidden\" name=\"groupsuff_d\" value=\"" . $_POST['groupsuff_d'] . "\">\n" .
				"<input type=\"submit\" name=\"sure\" value=\"" . _("Submit") . "\">\n" .
				"<input type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\">\n" .
				"</form>";
		}
		else {
			$error = _("OU is not empty or invalid!");
		}
	}

	// host operations
	// new host ou
	if ($_POST['type'] == "new_hst") {
		// create ou if valid
		if (eregi("^[a-z0-9 _\\-]+$", $_POST['newsuff_h'])) {
			// check if ou already exists
			$new_dn = "ou=" . $_POST['newsuff_h'] . "," . $_POST['hostsuff_n'];
			if (!in_array(strtolower($new_dn), $_SESSION['ldap']->search_units($_POST['hostsuff_n']))) {
				// add new ou
				$ou = array();
				$ou['objectClass'] = "organizationalunit";
				$ou['ou'] = $_POST['newsuff_h'];
				$ret = @ldap_add($_SESSION['ldap']->server(), $new_dn, $ou);
				if ($ret) {
					$message = _("New OU created successfully.");
				}
				else {
					$error = _("Unable to create new OU!");
				}
			}
			else $error = _("OU already exists!");
		}
		// show errormessage if ou is invalid
		else {
			$error = _("OU is invalid!") . " " . $_POST['newsuff_h'];
		}
	}
	// delete ou, user was sure
	elseif (($_POST['type'] == "del_hst") && ($_POST['sure'])) {
		$ret = @ldap_delete($_SESSION['ldap']->server(), $_POST['hostsuff_d']);
		if ($ret) {
			$message = _("OU deleted successfully.");
		}
		else {
			$error = _("Unable to delete OU!");
		}
	}
	// do not delete ou
	elseif (($_POST['type'] == "del_hst") && ($_POST['abort'])) {
		display_main();
		exit;
	}
	// ask if user is sure to delete
	elseif ($_POST['type'] == "del_hst") {
		// check for sub entries
		$sr = @ldap_list($_SESSION['ldap']->server(), $_POST['hostsuff_d'], "ObjectClass=*", array(""));
		$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($sr && $info['count'] == 0) {
			$text = "<br>\n" .
				"<p><big><b>" . _("Do you really want to delete this OU?") . " </b></big>" . "\n" .
				"<br>\n<p>" . $_POST['hostsuff_d'] . "</p>\n" .
				"<br>\n" .
				"<form action=\"ou_edit.php?type=host\" method=\"post\">\n" .
				"<input type=\"hidden\" name=\"type\" value=\"del_hst\">\n" .
				"<input type=\"hidden\" name=\"submit\" value=\"submit\">\n" .
				"<input type=\"hidden\" name=\"hostsuff_d\" value=\"" . $_POST['hostsuff_d'] . "\">\n" .
				"<input type=\"submit\" name=\"sure\" value=\"" . _("Submit") . "\">\n" .
				"<input type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\">\n" .
				"</form>";
		}
		else {
			$error = _("OU is not empty or invalid!");
		}
	}

	// domain operations
	// new domain ou
	if ($_POST['type'] == "new_dom") {
		// create ou if valid
		if (eregi("^[a-z0-9 _\\-]+$", $_POST['newsuff_d'])) {
			// check if ou already exists
			$new_dn = "ou=" . $_POST['newsuff_d'] . "," . $_POST['domsuff_n'];
			if (!in_array(strtolower($new_dn), $_SESSION['ldap']->search_units($_POST['domsuff_n']))) {
				// add new ou
				$ou = array();
				$ou['objectClass'] = "organizationalunit";
				$ou['ou'] = $_POST['newsuff_d'];
				$ret = @ldap_add($_SESSION['ldap']->server(), $new_dn, $ou);
				if ($ret) {
					$message = _("New OU created successfully.");
				}
				else {
					$error = _("Unable to create new OU!");
				}
			}
			else $error = _("OU already exists!");
		}
		// show errormessage if ou is invalid
		else {
			$error = _("OU is invalid!") . " " . $_POST['newsuff_d'];
		}
	}
	// delete ou, user was sure
	elseif (($_POST['type'] == "del_dom") && ($_POST['sure'])) {
		$ret = @ldap_delete($_SESSION['ldap']->server(), $_POST['domsuff_d']);
		if ($ret) {
			$message = _("OU deleted successfully.");
		}
		else {
			$error = _("Unable to delete OU!");
		}
	}
	// do not delete ou
	elseif (($_POST['type'] == "del_dom") && ($_POST['abort'])) {
		display_main();
		exit;
	}
	// ask if user is sure to delete
	elseif ($_POST['type'] == "del_dom") {
		// check for sub entries
		$sr = @ldap_list($_SESSION['ldap']->server(), $_POST['domsuff_d'], "ObjectClass=*", array(""));
		$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($sr && $info['count'] == 0) {
			$text = "<br>\n" .
				"<p><big><b>" . _("Do you really want to delete this OU?") . " </b></big>" . "\n" .
				"<br>\n<p>" . $_POST['domsuff_d'] . "</p>\n" .
				"<br>\n" .
				"<form action=\"ou_edit.php?type=host\" method=\"post\">\n" .
				"<input type=\"hidden\" name=\"type\" value=\"del_dom\">\n" .
				"<input type=\"hidden\" name=\"submit\" value=\"submit\">\n" .
				"<input type=\"hidden\" name=\"domsuff_d\" value=\"" . $_POST['domsuff_d'] . "\">\n" .
				"<input type=\"submit\" name=\"sure\" value=\"" . _("Submit") . "\">\n" .
				"<input type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\">\n" .
				"</form>";
		}
		else {
			$error = _("OU is not empty or invalid!");
		}
	}

	// print header
	echo $_SESSION['header'];
	echo ("<title>OU-Editor</title>\n");
	echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n");
	echo ("</head>\n");
	echo ("<body>\n");
	// display messages
	if ($error || $message || $text) {
		if ($text) echo $text;
		elseif ($error) {
			StatusMessage("ERROR", "", $error);
			echo ("<br><a href=\"ou_edit.php\">" . _("Back to OU-Editor") . "</a>\n");
		}
		else {
			StatusMessage("INFO", "", $message);
			echo ("<br><a href=\"ou_edit.php\">" . _("Back to OU-Editor") . "</a>\n");
		}
	}

echo ("</body></html>\n");
exit;
}
else display_main();

function display_main() {
	// generate lists of possible suffixes
	$usr_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_UserSuffix());
	$grp_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_GroupSuffix());
	$hst_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_HostSuffix());
	$dom_units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_DomainSuffix());

	// display main page
	echo $_SESSION['header'];
	echo ("<title>OU-Editor</title>\n");
	echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n");
	echo ("</head>\n");
	echo ("<body>\n");
	echo ("<br>\n");
	echo ("<form action=\"ou_edit.php?type=user\" method=\"post\">\n");

	// user OUs
	echo ("<fieldset><legend><b>" . _("Users") . "</b></legend>\n");
	echo ("<table border=0>\n");
	// new OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"new_usr\" checked></td>\n");
	echo ("<td><b>" . _("New organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=usersuff_n>");
	for ($i = 0; $i < sizeof($usr_units); $i++) {
		echo ("<option>" . $usr_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td><input type=text name=newsuff_u></td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=601\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	// delete OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"del_usr\"></td>\n");
	echo ("<td><b>" . _("Delete organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=usersuff_d>");
	for ($i = 0; $i < sizeof($usr_units); $i++) {
		echo ("<option>" . $usr_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=602\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	echo ("</table>\n");
	echo ("</fieldset>\n");
	echo ("<br>\n");

	// group OUs
	echo ("<fieldset><legend><b>" . _("Groups") . "</b></legend>\n");
	echo ("<table border=0>\n");
	// new OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"new_grp\"></td>\n");
	echo ("<td><b>" . _("New organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=groupsuff_n>");
	for ($i = 0; $i < sizeof($grp_units); $i++) {
		echo ("<option>" . $grp_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td><input type=text name=newsuff_g></td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=601\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	// delete OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"del_grp\"></td>\n");
	echo ("<td><b>" . _("Delete organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=groupsuff_d>");
	for ($i = 0; $i < sizeof($grp_units); $i++) {
		echo ("<option>" . $grp_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=602\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	echo ("</table>\n");
	echo ("</fieldset>\n");
	echo ("<br>\n");

	// host OUs
	echo ("<fieldset><legend><b>" . _("Samba Hosts") . "</b></legend>\n");
	echo ("<table border=0>\n");
	// new OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"new_hst\"></td>\n");
	echo ("<td><b>" . _("New organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=hostsuff_n>");
	for ($i = 0; $i < sizeof($hst_units); $i++) {
		echo ("<option>" . $hst_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td><input type=text name=newsuff_h></td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=601\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	// delete OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"del_hst\"></td>\n");
	echo ("<td><b>" . _("Delete organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=hostsuff_d>");
	for ($i = 0; $i < sizeof($hst_units); $i++) {
		echo ("<option>" . $hst_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=602\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	echo ("</table>\n");
	echo ("</fieldset>\n");
	echo ("<br>\n");

	// domain OUs
	echo ("<fieldset><legend><b>" . _("Domains") . "</b></legend>\n");
	echo ("<table border=0>\n");
	// new OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"new_dom\"></td>\n");
	echo ("<td><b>" . _("New organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=domsuff_n>");
	for ($i = 0; $i < sizeof($dom_units); $i++) {
		echo ("<option>" . $dom_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td><input type=text name=newsuff_d></td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=601\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	// delete OU
	echo ("<tr>\n");
	echo ("<td><input type=radio name=\"type\" value=\"del_dom\"></td>\n");
	echo ("<td><b>" . _("Delete organizational unit") . ":</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=domsuff_d>");
	for ($i = 0; $i < sizeof($dom_units); $i++) {
		echo ("<option>" . $dom_units[$i] . "</option>\n");
	}
	echo ("</select><td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><a href=\"help.php?HelpNumber=602\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
	echo ("</tr>\n");
	echo ("</table>\n");
	echo ("</fieldset>\n");
	echo ("<br>\n");

	echo ("<input type=\"submit\" name=\"submit\" value=\"" . _("Submit") . "\">");
	echo ("</form>\n");
	echo ("</body></html>\n");
}
