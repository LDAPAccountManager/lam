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
* This is an editor for organizational units.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration data */
include_once("../lib/config.inc");
/** access LDAP server */
include_once("../lib/ldap.inc");
/** used to print status messages */
include_once("../lib/status.inc");

// start session
startSecureSession();

setlanguage();

$types = $_SESSION['config']->get_ActiveTypes();

// check if submit button was pressed
if ($_POST['submit']) {
	// check user input
	for ($i = 0; $i < sizeof($types); $i++) {
		// new ou
		if ($_POST['type'] == "new_" . $types[$i]) {
			// create ou if valid
			if (eregi("^[a-z0-9 _\\-]+$", $_POST['newname_' . $types[$i]])) {
				// check if ou already exists
				$new_dn = "ou=" . $_POST['newname_' . $types[$i]] . "," . $_POST['parentsuff_' . $types[$i]];
				if (!in_array($new_dn, $_SESSION['ldap']->search_units($_POST['parentsuff_' . $types[$i]]))) {
					// add new ou
					$ou = array();
					$ou['objectClass'] = "organizationalunit";
					$ou['ou'] = $_POST['newname_' . $types[$i]];
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
				$error = _("OU is invalid!") . " " . $_POST['newname_' . $types[$i]];
			}
		}
		// delete ou, user was sure
		elseif (($_POST['type'] == "del_" . $types[$i]) && ($_POST['sure'])) {
			$ret = @ldap_delete($_SESSION['ldap']->server(), $_POST['deletename_' . $types[$i]]);
			if ($ret) {
				$message = _("OU deleted successfully.");
			}
			else {
				$error = _("Unable to delete OU!");
			}
		}
		// do not delete ou
		elseif (($_POST['type'] == "del_" . $types[$i]) && ($_POST['abort'])) {
			display_main();
			exit;
		}
		// ask if user is sure to delete
		elseif ($_POST['type'] == "del_" . $types[$i]) {
			// check for sub entries
			$sr = @ldap_list($_SESSION['ldap']->server(), $_POST['deletename_' . $types[$i]], "ObjectClass=*", array(""));
			$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
			if ($sr && $info['count'] == 0) {
				$text = "<br>\n" .
					"<p><big><b>" . _("Do you really want to delete this OU?") . " </b></big>" . "\n" .
					"<br>\n<p>" . $_POST['deletename_' . $types[$i]] . "</p>\n" .
					"<br>\n" .
					"<form action=\"ou_edit.php\" method=\"post\">\n" .
					"<input type=\"hidden\" name=\"type\" value=\"del_" . $types[$i] . "\">\n" .
					"<input type=\"hidden\" name=\"submit\" value=\"submit\">\n" .
					"<input type=\"hidden\" name=\"deletename_" . $types[$i] . "\" value=\"" . $_POST['deletename_' . $types[$i]] . "\">\n" .
					"<input type=\"submit\" name=\"sure\" value=\"" . _("Delete") . "\">\n" .
					"<input type=\"submit\" name=\"abort\" value=\"" . _("Cancel") . "\">\n" .
					"</form>";
			}
			else {
				$error = _("OU is not empty or invalid!");
			}
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
	$types = $_SESSION['config']->get_ActiveTypes();
	// display main page
	echo $_SESSION['header'];
	echo ("<title>OU-Editor</title>\n");
	echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n");
	for ($i = 0; $i < sizeof($types); $i++) {
		echo ("<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/type_" . $types[$i] . ".css\">\n");
	}
	echo ("</head>\n");
	echo ("<body>\n");
	echo "<h1>" . _("OU editor") . "</h1>";
	echo ("<br>\n");
	echo ("<form action=\"ou_edit.php\" method=\"post\">\n");

	// display fieldsets
	for ($i = 0; $i < sizeof($types); $i++) {
		// generate lists of possible suffixes
		$units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_Suffix($types[$i]));
		echo ("<fieldset class=\"" . $types[$i] . "edit\"><legend><b>" . getTypeAlias($types[$i]) . "</b></legend>\n");
		echo ("<table border=0>\n");
		// new OU
		echo ("<tr>\n");
		echo ("<td><input type=radio name=\"type\" value=\"new_" . $types[$i] . "\"></td>\n");
		echo ("<td><b>" . _("New organizational unit") . ":</b></td>\n");
		echo ("<td>&nbsp;</td>\n");
		echo ("<td><select size=1 name=parentsuff_" . $types[$i] . ">");
		for ($u = 0; $u < sizeof($units); $u++) {
			echo ("<option>" . $units[$u] . "</option>\n");
		}
		echo ("</select><td>\n");
		echo ("<td><input type=text name=newname_" . $types[$i] . "></td>\n");
		echo ("<td><a href=\"help.php?HelpNumber=601\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
		echo ("</tr>\n");
		// delete OU
		echo ("<tr>\n");
		echo ("<td><input type=radio name=\"type\" value=\"del_" . $types[$i] . "\"></td>\n");
		echo ("<td><b>" . _("Delete organizational unit") . ":</b></td>\n");
		echo ("<td>&nbsp;</td>\n");
		echo ("<td><select size=1 name=deletename_" . $types[$i] . ">");
		for ($u = 0; $u < sizeof($units); $u++) {
			echo ("<option>" . $units[$u] . "</option>\n");
		}
		echo ("</select><td>\n");
		echo ("<td>&nbsp;</td>\n");
		echo ("<td><a href=\"help.php?HelpNumber=602\" target=\"lamhelp\">". _("Help") ."</a></td>\n");
		echo ("</tr>\n");
		echo ("</table>\n");
		echo ("</fieldset>\n");
		echo ("<br>\n");
	}

	echo ("<input type=\"submit\" name=\"submit\" value=\"" . _("Ok") . "\">");
	echo ("</form>\n");
	echo ("</body></html>\n");
}
