<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2010  Roland Gruber

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
/** tool definitions */
include_once("../lib/tools.inc");

// start session
startSecureSession();

setlanguage();

include 'main_header.php';

// get tool list
$availableTools = getTools();
// sort tools
$toSort = array();
for ($i = 0; $i < sizeof($availableTools); $i++) {
	$myTool = new $availableTools[$i]();
	$toSort[$availableTools[$i]] = $myTool->getPosition();
}
asort($toSort);
$tools = array();
foreach ($toSort as $key => $value) {
	$tools[] = new $key();
}

echo "<p>&nbsp;</p>\n";

// print tools table
echo "<table class=\"userlist\" rules=\"none\">\n";

for ($i = 0; $i < sizeof($tools); $i++) {
	// check access level
	if ($tools[$i]->getRequiresWriteAccess() && !checkIfWriteAccessIsAllowed()) {
		continue;
	}
	if ($tools[$i]->getRequiresPasswordChangeRights() && !checkIfPasswordChangeIsAllowed()) {
		continue;
	}
	// print tool
	echo "<tr class=\"userlist\">\n";
		echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
		echo "<td><br>";
			echo "<a href=\"" . $tools[$i]->getLink() . "\">";
				echo "<img src=\"../graphics/" . $tools[$i]->getImageLink() . "\" alt=\"" . $tools[$i]->getName() . "\">";
				echo " &nbsp;<b>" . $tools[$i]->getName() . "</b>";
			echo "</a>\n";
		echo "<br><br></td>\n";
		echo "<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n";
		echo "<td>";
			echo $tools[$i]->getDescription();
		echo "</td>\n";
		echo "<td>&nbsp;&nbsp;&nbsp;</td>\n";
	echo "</tr>\n";
}

echo "</table>\n";


echo "</body>\n";
echo "</html>\n";

?>
