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


  Configuration wizard - lamdaemon and PDF text
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
	if (!$_SESSION['confwiz_config']->set_scriptpath($_POST['scriptpath'])) {
		$errors[] = _("Script path is invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_scriptserver($_POST['scriptserver'])) {
		$errors[] = _("Script server is invalid!");
	}
	if (!$_SESSION['confwiz_config']->set_pdftext($_POST['pdf_usertext'])) {
		$errors[] = _("Saving PDF text failed!");
	}
	// if no errors save and go back to optional.php
	if (sizeof($errors) < 1) {
		$_SESSION['confwiz_config']->save();
		$_SESSION['confwiz_optional']['daemon'] = 'done';
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
		echo "<p><br><br><a href=\"o_daemon.php\">" . _("Back to lamdaemon and PDF settings.") . "</a></p>\n";
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
echo "<form action=\"o_daemon.php\" method=\"post\">\n";
	echo "<p>&nbsp;</p>\n";

	// lamdaemon
	echo "<p><b>Lamdaemon.pl:</b></p>\n";
	echo "<p>" . _("If you want to manage quotas and homedirectories with LAM you need to setup lamdaemon.pl.") .
								"<br>" .
								_("This is the server and path where the lamdaemon.pl script is stored. LDAP Account Manager will make a SSH connection to this server with username and password provided at login.") .
								"<br><br><font color=red>" . _("Use it at your own risk and read the documentation for lamdaemon before you use it!") . "</font><br><br></p>\n";

	echo ("<table border=0>");

echo ("<tr><td align=\"right\"><b>".
	_("Server of external script") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"scriptserver\" value=\"" . $_SESSION['confwiz_config']->get_scriptServer() . "\"></td>\n");
echo ("</tr>\n");
echo ("<tr><td align=\"right\"><b>".
	_("Path to external script") . ": </b></td>".
	"<td><input size=50 type=\"text\" name=\"scriptpath\" value=\"" . $_SESSION['confwiz_config']->get_scriptPath() . "\"></td>\n");
echo ("</tr>\n");

	echo "</table>\n";

	echo "<p><br></p>\n";

	// PDF text
	echo "<p><b>" . _("PDF text") . ":</b></p>\n";
	echo "<p>" . _("This text will appear on top of every user PDF file.") . "</p>\n";

	echo ("<table border=0>");

echo "<tr>";
	echo "<td><textarea name=\"pdf_usertext\" cols=\"80\" rows=\"5\">" . $_SESSION['confwiz_config']->get_pdftext() . "</textarea></td>\n";
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







