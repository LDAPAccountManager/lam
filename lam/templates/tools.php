<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2018  Roland Gruber

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
enforceUserIsLoggedIn();

setlanguage();

include '../lib/adminHeader.inc';

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

echo "<div class=\"user-bright smallPaddingContent\">\n";

// print tools table
$container = new htmlResponsiveRow();
$container->add(new htmlTitle(_('Tools')), 12);
$toolSettings = $_SESSION['config']->getToolSettings();

for ($i = 0; $i < sizeof($tools); $i++) {
	// check access level
	if ($tools[$i]->getRequiresWriteAccess() && !checkIfWriteAccessIsAllowed()) {
		continue;
	}
	if ($tools[$i]->getRequiresPasswordChangeRights() && !checkIfPasswordChangeIsAllowed()) {
		continue;
	}
	// check visibility
	if (!$tools[$i]->isVisible()) {
		continue;
	}
	// check if hidden by config
	$className = get_class($tools[$i]);
	$toolName = substr($className, strrpos($className, '\\') + 1);
	if (isset($toolSettings['tool_hide_' . $toolName]) && ($toolSettings['tool_hide_' . $toolName] == 'true')) {
		continue;
	}
	// add tool
	$container->add(new htmlLink($tools[$i]->getName(), $tools[$i]->getLink(), '../graphics/' . $tools[$i]->getImageLink()), 12, 4);
	$container->add(new htmlOutputText($tools[$i]->getDescription()), 12, 8);
	$container->addVerticalSpacer('2rem');
}

$tabindex = 1;
parseHtml(null, $container, array(), true, $tabindex, 'user');

echo "</div>";

include '../lib/adminFooter.inc';

?>
