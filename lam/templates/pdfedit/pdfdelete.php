<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004  Michael Dürgner

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
* @author Michael Dürgner
*/

/** helper functions for pdf */
include_once('../../lib/pdfstruct.inc');

// start session
session_save_path("../../sess");
@session_start();

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
echo ("</head>\n<body>\n<p><br></p>\n");

// check if admin has submited delete operation
if ($_POST['submit']) {
	// delete user profile
	if(!deletePDFStructureDefinition($_POST['type'],$_POST['delete'])) {
		StatusMessage('ERROR', '', _('Unable to delete PDF structure!') . ' ' . _('Scope') . ': ' . $_POST['type'] . ' ' . _('Name') . ': ' . substr($_POST['delete'],0,strlen($_POST['delete']) - 4));
	}
	else {
		StatusMessage('INFO', '', _('Deleted PDF structure:') . ' ' . _('Scope') . ': ' . $_POST['type'] . ' ' . _('Name') . ': ' . substr($_POST['delete'],0,strlen($_POST['delete']) - 4));
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
if (($type == "user") || ($type == "host") || ($type == "group")) {
	echo ("<p align=\"center\"><big>" . _("Do you really want to delete this PDF structure?") . " <b>");
		echo (_('Scope') . ': ' . $_GET['type'] . ' ' . _('Name') . ': ' . substr($_GET['delete'],0,strlen($_GET['delete']) - 4) . "</b></big><br></p>\n");
	echo ("<form action=\"pdfdelete.php\" method=\"post\">\n");
	echo ("<p align=\"center\">\n");
	echo ("<input type=\"submit\" name=\"submit\" value=\"" . _("Submit") . "\">\n");
	echo ("<input type=\"submit\" name=\"abort\" value=\"" . _("Abort") . "\">\n");
	echo ("<input type=\"hidden\" name=\"type\" value=\"" . $_GET['type'] . "\">");
	echo ("<input type=\"hidden\" name=\"delete\" value=\"" . $_GET['delete'] . "\">");
	echo ("</p></form></body></html>\n");
}
else{
	// no valid pdf type
	StatusMessage("ERROR", "", _("Wrong or missing type!") . " " . $type);
	echo ("<a href=\"pdfmain.php\">" . _("Back to PDF Editor") . "</a>");
}
