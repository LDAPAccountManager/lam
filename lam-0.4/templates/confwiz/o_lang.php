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


  Configuration wizard - language and admins
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
	if (!$_SESSION['confwiz_config']->set_defaultLanguage($_POST['lang'])) {
		$errors[] = _("Language is not defined!");
	}
	if (!$_SESSION['confwiz_config']->set_Adminstring($_POST['admins'])) {
		$errors[] = _("List of admin users is empty or invalid!");
	}
	// if no errors save and go back to optional.php
	if (sizeof($errors) < 1) {
		$_SESSION['confwiz_config']->save();
		$_SESSION['confwiz_optional']['lang'] = 'done';
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
		echo "<p><br><br><a href=\"o_lang.php\">" . _("Back to language and admin settings.") . "</a></p>\n";
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
echo "<form action=\"o_lang.php\" method=\"post\">\n";
	echo "<p>&nbsp;</p>\n";

	// language
	echo "<p><b>" . _("Please select your prefered language.") . "</b></p>\n";
	echo "<p>" . _("This defines the language of the login window and sets this language as the default language. Users can change the language at login.") .
				"<br><br></p>\n";

	echo ("<table border=0>");

	echo ("<tr>");
	echo ("<td><b>" . _("Default language") . ":</b></td><td>\n");
	// read available languages
	$languagefile = "../../config/language";
	if(is_file($languagefile))
	{
		$file = fopen($languagefile, "r");
		$i = 0;
		while(!feof($file))
		{
			$line = fgets($file, 1024);
			if($line == "\n" || $line[0] == "#" || $line == "") continue; // ignore comment and empty lines
			$languages[$i] = chop($line);
			$i++;
		}
		fclose($file);
	// generate language list
	echo ("<select name=\"lang\">");
	for ($i = 0; $i < sizeof($languages); $i++) {
		$entry = explode(":", $languages[$i]);
		if ($_SESSION['confwiz_config']->get_defaultLanguage() != $languages[$i]) echo("<option value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
		else echo("<option selected value=\"" . $languages[$i] . "\">" . $entry[2] . "</option>\n");
	}
	echo ("</select>\n");
	}
	else
	{
		echo _("Unable to load available languages. Setting English as default language. For further instructions please contact the Admin of this site.");
	}
	echo ("</td>\n");
	echo ("</tr>\n");

	echo "</table>\n";

	echo "<p><br></p>\n";
	echo "<p><br></p>\n";

	// admin users
	echo "<p><b>" . _("Valid users") . ":</b></p>\n";
	echo "<p>" . _("If you want more than one user to login to LAM please enter its DN(s) here. Multiple entries are seperated by semicolons.") . "</p>\n";
	echo "<p><b>" . _("Example") . ": </b>cn=admin,dc=yourdomain,dc=org;cn=manager,dc=yourdomain,dc=org<br><br></p>\n";

	echo ("<table border=0>");

	echo ("<tr><td align=\"right\"><b>".
		_("List of valid users") . ": </b></td>".
		"<td colspan=2><input size=50 type=\"text\" name=\"admins\" value=\"" . $_SESSION['confwiz_config']->get_Adminstring() . "\"></td>\n");
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






