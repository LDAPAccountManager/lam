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


  Configuration wizard - list attributes
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

// check if user clicked on cancel button
if ($_POST['cancel']) {
	@unlink("../../config/" . $_SESSION['confwiz_config']->file . ".conf");
	metarefresh('../config/conflogin.php');
	exit;
}

// check if user clicked on next button
if ($_POST['submit']) {
	$errors = array();
	if (!$_SESSION['confwiz_config']->set_userlistAttributes($_POST['usrlstattr'])) {
		$errors[] = _("User list attributes are invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_grouplistAttributes($_POST['grplstattr'])) {
		$errors[] = _("Group list attributes are invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_hostlistAttributes($_POST['hstlstattr'])) {
		$errors[] = _("Host list attributes are invalid!");
	}
	// if no errors save and go back to optional.php
	if (sizeof($errors) < 1) {
		$_SESSION['confwiz_config']->save();
		$_SESSION['confwiz_optional']['lists'] = 'done';
		metarefresh('optional.php');
	}
	else {
		// errors occured
		echo $_SESSION['header'];
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "<title>\n";
			echo _("Configuration wizard");
		echo "</title>\n";
		echo "</head><body>\n";
		for ($i = 0; $i < sizeof($errors); $i++) {
			StatusMessage("ERROR", $errors[$i], "");
		}
		echo "<p><br><br><a href=\"o_lists.php\">" . _("Back to list settings.") . "</a></p>\n";
		echo "</body></html>\n";
		exit;
	}
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
echo "<form action=\"o_lists.php\" method=\"post\">\n";
	echo "<p>&nbsp;</p>\n";

	echo "<p><b>" . _("Please enter which attributes should be displayed in the list views.") . "</b></p>\n";
	echo "<p>" . _("This can be a list of predefined attributes which have a description and are translated or you can write your own description.") .
							" " . _("Predefined attributes are of type \"#attribute\".") .
							" " . _("If you want to input your own description it would look like this: \"attribute:description\".") .
							"<br>" . _("The entries are separated by semicolons.") .
							"<br><br><br><b>" .
							_("Example") .
							": </b>#homeDirectory;#uid;#uidNumber;#gidNumber;mail:Mail address<br><br><br><u><b>" .
							_("Predefined values") . ":</b></u><br><br><b>" .
							_("Users") .
							": </b>#uid, #uidNumber, #gidNumber, #cn, #host, #givenName, #sn, #homeDirectory, #loginShell, #mail, #gecos".
							"<br><b>" .
							_("Groups") .
							": </b>#cn, #gidNumber, #memberUID, #member, #description".
							"<br><b>" .
							_("Hosts") .
							": </b>#uid, #cn, #rid, #description" . "<br><br></p>\n";
	echo "<p>&nbsp;</p>\n";

	echo ("<table border=0>");

// user list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in User List") . " *:</b></td>".
	"<td><input size=50 type=\"text\" name=\"usrlstattr\" value=\"" . $_SESSION['confwiz_config']->get_userlistAttributes() . "\"></td>");
echo ("</tr>\n");
// group list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Group List") . " *:</b></td>".
	"<td><input size=50 type=\"text\" name=\"grplstattr\" value=\"" . $_SESSION['confwiz_config']->get_grouplistAttributes() . "\"></td>");
echo ("</tr>\n");
// host list attributes
echo ("<tr><td align=\"right\"><b>".
	_("Attributes in Host List") . " *:</b></td>".
	"<td><input size=50 type=\"text\" name=\"hstlstattr\" value=\"" . $_SESSION['confwiz_config']->get_hostlistAttributes() . "\"></td>");
echo ("</tr>\n");

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





