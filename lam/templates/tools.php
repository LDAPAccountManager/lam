<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
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
* Provides a list of tools like file upload or profile editor.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration options */
include_once("../lib/config.inc");

// start session
startSecureSession();

setlanguage();

echo $_SESSION['header'];


echo "<title></title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/type_user.css\">\n";
echo "</head>";

echo "<body>\n";

// list of tools and descriptions
$tools = array();

// profile editor
$pEditor = new LAMTool();
$pEditor->name = _("Profile editor");
$pEditor->description = _("Here you can manage your account profiles.");
$pEditor->link = "profedit/profilemain.php";
$pEditor->requiresWriteAccess = true;
$tools[] = $pEditor;

// file upload
$fUpload = new LAMTool();
$fUpload->name = _("File upload");
$fUpload->description = _("Creates accounts by uploading a CSV formated file.");
$fUpload->link = "masscreate.php";
$fUpload->requiresWriteAccess = true;
$tools[] = $fUpload;

// OU editor
$ouEditor = new LAMTool();
$ouEditor->name = _("OU editor");
$ouEditor->description = _("Manages OU objects in your LDAP tree.");
$ouEditor->link = "ou_edit.php";
$ouEditor->requiresWriteAccess = true;
$tools[] = $ouEditor;

// PDF editor
$pdfEditor = new LAMTool();
$pdfEditor->name = _("PDF editor");
$pdfEditor->description = _("This tool allows you to customize the PDF pages.");
$pdfEditor->link = "pdfedit/pdfmain.php";
$pdfEditor->requiresWriteAccess = true;
$tools[] = $pdfEditor;

// schema browser
$sBrowser = new LAMTool();
$sBrowser->name = _("Schema browser");
$sBrowser->description = _("Here you can browse LDAP object classes and attributes.");
$sBrowser->link = "schema/schema.php";
$tools[] = $sBrowser;

// tests
$tests = new LAMTool();
$tests->name = _("Tests");
$tests->description = _("Here you can test if certain LAM features work on your installation.");
$tests->link = "tests/index.php";
$tests->requiresWriteAccess = true;
$tools[] = $tests;

echo "<p>&nbsp;</p>\n";

// print tools table
echo "<table class=\"userlist\" rules=\"none\">\n";

for ($i = 0; $i < sizeof($tools); $i++) {
	// check access level
	if ($tools[$i]->requiresWriteAccess && !checkIfWriteAccessIsAllowed()) {
		continue;
	}
	if ($tools[$i]->requiresPasswordChanges && !checkIfPasswordChangeIsAllowed()) {
		continue;
	}
	// print tool
	echo "<tr class=\"userlist\">\n";
		echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
		echo "<td><br>";
			echo "<a href=\"" . $tools[$i]->link . "\" target=\"mainpart\"><b>" . $tools[$i]->name . "</b></a>";
		echo "<br><br></td>\n";
		echo "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n";
		echo "<td>";
			echo $tools[$i]->description;
		echo "</td>\n";
		echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
	echo "</tr>\n";
}

echo "</table>\n";


echo "</body>\n";
echo "</html>\n";

/**
 * Represents a tool.
 *
 * @author Roland Gruber
 * @package tools
 */
class LAMTool {
	
	/** name of the tool */
	public $name;
	
	/** description text */
	public $description;
	
	/** link to tool page (relative to templates/) */
	public $link;
	
	/** tool requires write access to LDAP */
	public $requiresWriteAccess = false;
	
	/** tool requires password change rights */
	public $requiresPasswordChanges = false;
	
}

?>
