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


  Configuration wizard - UID/GID ranges
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
	if (!$_SESSION['confwiz_config']->set_minUID($_POST['minUID'])) {
		$errors[] = _("Minimum UID number is invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_maxUID($_POST['maxUID'])) {
		$errors[] = _("Maximum UID number is invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_minGID($_POST['minGID'])) {
		$errors[] = _("Minimum GID number is invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_maxGID($_POST['maxGID'])) {
		$errors[] = _("Maximum GID number is invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_minMachine($_POST['minMach'])) {
		$errors[] = _("Minimum Machine number is invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_maxMachine($_POST['maxMach'])) {
		$errors[] = _("Maximum Machine number is invalid!");
	}
	// if no errors save and go back to optional.php
	if (sizeof($errors) < 1) {
		$_SESSION['confwiz_config']->save();
		$_SESSION['confwiz_optional']['ranges'] = 'done';
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
		echo "<p><br><br><a href=\"o_ranges.php\">" . _("Back to range settings.") . "</a></p>\n";
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
echo "<form action=\"o_ranges.php\" method=\"post\">\n";
	echo "<p>&nbsp;</p>\n";

	echo "<p><b>" . _("Please enter the UID/GID ranges for your accounts:") . "</b></p>\n";
	echo "<p>&nbsp;</p>\n";

	echo ("<table border=0>");

	// minUID
	echo ("<tr><td align=\"right\"><b>".
		_("Minimum UID number") . " *: </b>".
		"<input size=6 type=\"text\" name=\"minUID\" value=\"" . $_SESSION['confwiz_config']->get_minUID() . "\"></td>\n");
	// maxUID
	echo ("<td align=\"right\"><b>&nbsp;" . _("Maximum UID number") . " *: </b>".
		"<input size=6 type=\"text\" name=\"maxUID\" value=\"" . $_SESSION['confwiz_config']->get_maxUID() . "\"></td>\n");
	// UID text
	echo ("<td><a href=\"../help.php?HelpNumber=203\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
	// minGID
	echo ("<tr><td align=\"right\"><b>".
		_("Minimum GID number") . " *: </b>".
		"<input size=6 type=\"text\" name=\"minGID\" value=\"" . $_SESSION['confwiz_config']->get_minGID() . "\"></td>\n");
	// maxGID
	echo ("<td align=\"right\"><b>&nbsp;" . _("Maximum GID number")." *: </b>".
		"<input size=6 type=\"text\" name=\"maxGID\" value=\"" . $_SESSION['confwiz_config']->get_maxGID() . "\"></td>\n");
	// GID text
	echo ("<td><a href=\"../help.php?HelpNumber=204\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");
	// minMach
	echo ("<tr><td align=\"right\"><b>".
		_("Minimum Machine number") . " *: </b>".
		"<input size=6 type=\"text\" name=\"minMach\" value=\"" . $_SESSION['confwiz_config']->get_minMachine() . "\"></td>\n");
	// maxMach
	echo ("<td align=\"right\"><b>&nbsp;" . _("Maximum Machine number") . " *: </b>".
		"<input size=6 type=\"text\" name=\"maxMach\" value=\"" . $_SESSION['confwiz_config']->get_maxMachine() . "\"></td>\n");
	// Machine text
	echo ("<td><a href=\"../help.php?HelpNumber=205\" target=\"lamhelp\">" . _("Help") . "</a></td></tr>\n");

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




