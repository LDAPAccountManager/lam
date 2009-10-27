<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2006  Michael Duergner
  Copyright (C) 2007         Roland Gruber

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
* Manages deletion of pdf structures.
*
* @package PDF
* @author Michael Duergner
*/

/** security functions */
include_once("../../lib/security.inc");
/** helper functions for pdf */
include_once('../../lib/pdfstruct.inc');

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// print standard header
echo $_SESSION['header'];
echo ("<title>" . _("Delete PDF structure") . "</title>\n");
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $_POST['type'] . ".css\">\n";
echo ("</head>\n<body>\n<p><br></p>\n");

// check if admin has submited delete operation
if ($_POST['submit']) {
	// delete user profile
	if(!deletePDFStructureDefinition($_POST['type'],$_POST['delete'])) {
		StatusMessage('ERROR', '', _('Unable to delete PDF structure!') . ' ' . _('Scope') . ': ' . $_POST['type'] . ' ' . _('Name') . ': ' . $_POST['delete']);
	}
	else {
		StatusMessage('INFO', '', _('Deleted PDF structure:') . ' ' . _('Scope') . ': ' . $_POST['type'] . ' ' . _('Name') . ': ' . $_POST['delete']);
	}
	echo ("<br><a href=\"pdfmain.php\">" . _("Back to PDF Editor") . "</a>");
	echo ("</body></html>\n");
	exit;
}

// check if admin has aborted delete operation
if ($_POST['abort']) {
	StatusMessage("INFO", "", _("Delete operation canceled."));
	echo ("<br><a href=\"pdfmain.php\">" . _("Back to PDF Editor") . "</a>");
	echo ("</body></html>\n");
	exit;
}

// check if right type was given
$type = $_GET['type'];
echo ("<p align=\"center\"><big>" . _("Do you really want to delete this PDF structure?") . "</big>");
echo "<br>\n";
echo "<br></p>\n";
echo "<table align=\"center\">\n";
	echo "<tr><td>\n";
		echo "<b>" . _('Account type') . ': </b>' . getTypeAlias($_GET['type']);
	echo "</td></tr>\n";
	echo "<tr><td>\n";
		echo "<b>" . _('Name') . ': </b>' . $_GET['delete'] . "<br>\n";
	echo "</td></tr>\n";
echo "</table>\n";
echo "<br>\n";
echo ("<form action=\"pdfdelete.php\" method=\"post\">\n");
echo ("<p align=\"center\">\n");
echo ("<input type=\"submit\" name=\"submit\" value=\"" . _("Delete") . "\">\n");
echo ("<input type=\"submit\" name=\"abort\" value=\"" . _("Cancel") . "\">\n");
echo ("<input type=\"hidden\" name=\"type\" value=\"" . $_GET['type'] . "\">");
echo ("<input type=\"hidden\" name=\"delete\" value=\"" . $_GET['delete'] . "\">");
echo ("</p></form></body></html>\n");
