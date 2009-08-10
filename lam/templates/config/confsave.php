<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009  Roland Gruber

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
* End page of configuration.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once("../../lib/config.inc");

/** access to module settings */
include_once("../../lib/modules.inc");

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
@session_start();

setlanguage();

// get password
if (isset($_POST['passwd'])) $passwd = $_POST['passwd'];

// check if password was entered
// if not: load login page
if (!isset($passwd) && !isset($_SESSION['conf_isAuthenticated'])) {
	$_SESSION['conf_message'] = _("No password was entered!");
	/** go back to login if password is empty */
	require('conflogin.php');
	exit;
}

if (!isset($_SESSION['conf_config']) && isset($_POST['filename'])) {
	$_SESSION['conf_config'] = new LAMConfig($_POST['filename']);
}
$conf = &$_SESSION['conf_config'];

// check if password is valid
// if not: load login page
if ((!isset($_SESSION['conf_isAuthenticated']) || !($_SESSION['conf_isAuthenticated'] === $conf->getName())) && !$conf->check_Passwd($passwd)) {
	$sessionKeys = array_keys($_SESSION);
	for ($i = 0; $i < sizeof($sessionKeys); $i++) {
		if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
	}
	$_SESSION['conf_message'] = _("The password is invalid! Please try again.");
	/** go back to login if password is invalid */
	require('conflogin.php');
	exit;
}
$_SESSION['conf_isAuthenticated'] = $conf->getName();


// page head
echo $_SESSION['header'];
echo "<title>" . _("LDAP Account Manager Configuration") . "</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../../graphics/favicon.ico\">\n";
echo "</head><body>\n";
echo ("<p align=\"center\"><a href=\"http://www.ldap-account-manager.org/\" target=\"new_window\">".
	"<img src=\"../../graphics/banner.jpg\" border=1 alt=\"LDAP Account Manager\"></a></p><hr><br><br>");
$conf->save();
echo ("<br><br><br><br><br><a href=\"../login.php\" target=\"_top\">" . _("Back to Login") . "</a>");
echo("</body></html>");
// remove settings from session
$sessionKeys = array_keys($_SESSION);
for ($i = 0; $i < sizeof($sessionKeys); $i++) {
	if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
}
exit();

?>