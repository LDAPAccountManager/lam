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
* Manages deletion of profiles.
*
* @package profiles
* @author Roland Gruber
*/

/** security functions */
include_once("../../lib/security.inc");
/** helper functions for profiles */
include_once("../../lib/profiles.inc");
/** access to LDAP server */
include_once("../../lib/ldap.inc");
/** access to configuration options */
include_once("../../lib/config.inc");

// start session
startSecureSession();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// print standard header
echo $_SESSION['header'];
echo ("<title>" . _("Delete profile") . "</title>\n");
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo ("</head>\n<body>\n<p><br></p>\n");

// check if admin has submited delete operation
if ($_POST['submit']) {
	// delete profile
	if (!delAccountProfile($_POST['del'], $_POST['type'])) {
		StatusMessage("ERROR", _("Unable to delete profile!"), $_POST['del'] . "." . $_POST['type']);
	}
	else StatusMessage("INFO", _("Deleted profile:"), $_POST['del'] . "." . $_POST['type']);
	echo ("<br><a href=\"profilemain.php\">" . _("Back to profile editor") . "</a>");
	echo ("</body></html>\n");
	exit;
}

// check if admin has aborted delete operation
if ($_POST['abort']) {
	StatusMessage("INFO", "", _("Delete operation canceled."));
	echo ("<br><a href=\"profilemain.php\">" . _("Back to profile editor") . "</a>");
	echo ("</body></html>\n");
	exit;
}

$type = $_GET['type'];
echo ("<p align=\"center\"><big>" . _("Do you really want to delete this profile?") . " <b>");
echo ($_GET['del'] . "</b></big><br></p>\n");
echo ("<form action=\"profiledelete.php\" method=\"post\">\n");
echo ("<p align=\"center\">\n");
echo ("<input type=\"submit\" name=\"submit\" value=\"" . _("Ok") . "\">\n");
echo ("<input type=\"submit\" name=\"abort\" value=\"" . _("Cancel") . "\">\n");
echo ("<input type=\"hidden\" name=\"type\" value=\"$type\">");
echo ("<input type=\"hidden\" name=\"del\" value=\"" . $_GET['del'] . "\">");
echo ("</p></form></body></html>\n");
