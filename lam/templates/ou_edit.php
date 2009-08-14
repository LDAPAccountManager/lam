<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2008  Roland Gruber

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

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

$types = $_SESSION['config']->get_ActiveTypes();

// check if deletion was canceled
if (isset($_POST['abort'])) {
	display_main();
	exit;
}

// check if submit button was pressed
if (isset($_POST['createOU']) || isset($_POST['deleteOU'])) {
	// new ou
	if (isset($_POST['createOU'])) {
		// create ou if valid
		if (preg_match("/^[a-z0-9 _\\-]+$/i", $_POST['newOU'])) {
			// check if ou already exists
			$new_dn = "ou=" . $_POST['newOU'] . "," . $_POST['parentOU'];
			if (!in_array($new_dn, $_SESSION['ldap']->search_units($_POST['parentOU']))) {
				// add new ou
				$ou = array();
				$ou['objectClass'] = "organizationalunit";
				$ou['ou'] = $_POST['newOU'];
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
			$error = _("OU is invalid!") . "<br>" . $_POST['newOU'];
		}
	}
	// delete ou, user was sure
	elseif (isset($_POST['deleteOU']) && isset($_POST['sure'])) {
		$ret = @ldap_delete($_SESSION['ldap']->server(), $_POST['deletename']);
		if ($ret) {
			$message = _("OU deleted successfully.");
		}
		else {
			$error = _("Unable to delete OU!");
		}
	}
	// ask if user is sure to delete
	elseif (isset($_POST['deleteOU'])) {
		// check for sub entries
		$sr = @ldap_list($_SESSION['ldap']->server(), $_POST['deleteableOU'], "ObjectClass=*", array(""));
		$info = @ldap_get_entries($_SESSION['ldap']->server(), $sr);
		if ($sr && $info['count'] == 0) {
			$text = "<br>\n" .
				"<p><big><b>" . _("Do you really want to delete this OU?") . " </b></big>" . "\n" .
				"<br>\n<p>" . $_POST['deleteableOU'] . "</p>\n" .
				"<br>\n" .
				"<form action=\"ou_edit.php\" method=\"post\">\n" .
				"<input type=\"hidden\" name=\"deleteOU\" value=\"submit\">\n" .
				"<input type=\"hidden\" name=\"deletename\" value=\"" . $_POST['deleteableOU'] . "\">\n" .
				"<input type=\"submit\" name=\"sure\" value=\"" . _("Delete") . "\">\n" .
				"<input type=\"submit\" name=\"abort\" value=\"" . _("Cancel") . "\">\n" .
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

display_main();

/**
 * Displays the main page of the OU editor
 */
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
	echo "<script type=\"text/javascript\" src=\"wz_tooltip.js\"></script>\n";
	echo "<h1>" . _("OU editor") . "</h1>";
	echo ("<br>\n");
	echo ("<form action=\"ou_edit.php\" method=\"post\">\n");
	
	$options = "";
	for ($i = 0; $i < sizeof($types); $i++) {
		$options .= "<optgroup label=\"" . getTypeAlias($types[$i]) . "\">\n";
		$units = $_SESSION['ldap']->search_units($_SESSION["config"]->get_Suffix($types[$i]));
		for ($u = 0; $u < sizeof($units); $u++) {
			$options .= "<option>" . $units[$u] . "</option>\n";
		}
		$options .= "</optgroup>\n";
	}
	
	echo ("<fieldset class=\"useredit\"><legend><b>" . _("OU editor") . "</b></legend><br>\n");
	echo ("<table border=0>\n");
	// new OU
	echo ("<tr>\n");
	echo ("<td><b>" . _("New organizational unit") . "</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=\"parentOU\">");
		echo $options;
	echo ("</select><td>\n");
	echo ("<td><input type=text name=\"newOU\"></td>\n");
	echo "<td>";
		echo "<input type=\"submit\" name=\"createOU\" value=\"" . _("Ok") . "\">&nbsp;";
	echo "</td>";
	echo "<td>";
		printHelpLink(getHelp('', '601'), '601');
	echo "</td>\n";
	echo ("</tr>\n");
	echo "<tr><td colspan=5>&nbsp;</td></tr>\n";
	// delete OU
	echo ("<tr>\n");
	echo ("<td><b>" . _("Delete organizational unit") . "</b></td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo ("<td><select size=1 name=\"deleteableOU\">");
		echo $options;
	echo ("</select><td>\n");
	echo ("<td>&nbsp;</td>\n");
	echo "<td>";
		echo "<input type=\"submit\" name=\"deleteOU\" value=\"" . _("Ok") . "\">&nbsp;";
	echo "</td>";
	echo "<td>";
		printHelpLink(getHelp('', '602'), '602');
	echo "</td>\n";
	echo ("</tr>\n");
	echo ("</table>\n");
	echo ("</fieldset>\n");
	echo ("<br>\n");
	
	echo ("</form>\n");
	echo ("</body></html>\n");
}
