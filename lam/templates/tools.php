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

*/

/**
* Provides a list of tools like file upload or profile editor.
*
* @author Roland Gruber
* @package tools
*/

/** access to configuration options */
include_once("../lib/config.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();

echo $_SESSION['header'];


echo "<title></title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">\n";
echo "</head>";

echo "<body>\n";

// list of tools and descriptions
$tools = array();
// profile editor
$tools[] = array(
		"name" => _("Profile editor"),
		"description" => _("Here you can manage your account profiles."),
		"link" => "profedit/profilemain.php"
	);

// Samba 3 domains
if ($_SESSION['config']->get_DomainSuffix() && ($_SESSION['config']->get_DomainSuffix() != "")) {
$tools[] = array(
		"name" => _("Samba 3 domains"),
		"description" => _("Manages Samba 3 domain accounts."),
		"link" => "lists/listdomains.php"
	);
}

// file upload
$tools[] = array(
		"name" => _("File upload"),
		"description" => _("Creates accounts by uploading a CSV formated file."),
		"link" => "masscreate.php"
	);

// OU editor
$tools[] = array(
		"name" => _("OU editor"),
		"description" => _("Manages OU objects in your LDAP tree."),
		"link" => "ou_edit.php"
	);

// PDF editor
$tools[] = array(
		"name" => _("PDF editor"),
		"description" => _("This tool allows you to customize the PDF pages."),
		"link" => "pdfedit/pdfmain.php"
	);

echo "<p>&nbsp;</p>\n";

// print tools table
echo "<table class=\"userlist\" rules=\"none\">\n";

for ($i = 0; $i < sizeof($tools); $i++) {
	echo "<tr class=\"userlist\">\n";
		echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
		echo "<td><br>";
			echo "<a href=\"" . $tools[$i]['link'] . "\" target=\"mainpart\"><b>" . $tools[$i]['name'] . "</b></a>";
		echo "<br><br></td>\n";
		echo "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n";
		echo "<td>";
			echo $tools[$i]['description'];
		echo "</td>\n";
		echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
	echo "</tr>\n";
}

echo "</table>\n";


echo "</body>\n";
echo "</html>\n";

?>
