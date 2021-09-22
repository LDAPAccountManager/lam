<?php
namespace LAM\HEADER;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018 - 2021  Roland Gruber

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

use htmlContentLink;
use htmlDiv;
use htmlGroup;
use htmlImage;
use htmlLink;
use htmlOutputText;
use htmlResponsiveRow;
use htmlSpan;
use LAMException;
use ServerProfilePersistenceManager;

/**
* Head part of page which includes links to lists etc.
*
* @package main
* @author Roland Gruber
*/

$headerPrefix = "";
if (is_file("login.php")) {
    $headerPrefix = "..";
}
elseif (is_file("../../templates/login.php")) {
    $headerPrefix = "../..";
}
elseif (is_file("../../../templates/login.php")) {
    $headerPrefix = "../../..";
}

/** tool definitions */
include_once(__DIR__ . "/tools.inc");

$pro = '';
if (isLAMProVersion()) {
    $pro = ' Pro';
}

// HTML header and title
echo $_SESSION['header'];
$title = "LDAP Account Manager" . $pro . " (" . str_replace(array('ldap://', 'ldaps://'), array('', ''), $_SESSION['config']->get_ServerURL()) . ")";
printHeaderContents($title, $headerPrefix);
echo "</head><body class=\"admin\">\n";

// include all JavaScript files
printJsIncludes($headerPrefix);

// get tool list
$availableTools = getTools();
// sort tools
$toSort = array();
foreach ($availableTools as $myTool) {
    if ($myTool->getRequiresWriteAccess() && !checkIfWriteAccessIsAllowed()) {
        continue;
    }
    if ($myTool->getRequiresPasswordChangeRights() && !checkIfPasswordChangeIsAllowed()) {
        continue;
    }
    // check visibility
    if (!$myTool->isVisible()) {
        continue;
    }
    // check if hidden by config
    $toolClass = get_class($myTool);
    $toolName = substr($toolClass, strrpos($toolClass, '\\') + 1);
    if (!$_SESSION['config']->isToolActive($toolName)) {
        continue;
    }
    $toSort[$toolClass] = $myTool->getPosition();
}
asort($toSort);
$tools = array();
foreach ($toSort as $key => $value) {
    $tools[] = new $key();
}
$userData = $_SESSION['ldap']->getUserName();
$userName = extractRDNValue($userData);
?>

<div id="lam-topnav" class="lam-header">
            <div style="margin-top: 5px;" class="lam-header-left lam-menu-stay">
                <a href="https://www.ldap-account-manager.org/" target="new_window">
                    <img class="align-middle" width="24" height="24" alt="help" src="<?php echo $headerPrefix; ?>/graphics/logo24.png">
                    <span class="hide-on-tablet">&nbsp;&nbsp;<?php echo $userName?></span>
                    <span class="hide-on-mobile">
                        <?php
                            echo getLAMVersionText();
                        ?>
                    </span>
                </a>
                <span class="hide-on-mobile lam-margin-small">&nbsp;&nbsp;
                    <?php
                    $serverProfileLabel = $_SESSION['config']->getName() . ' - ';
                    $serverProfilesPersistenceManager = new ServerProfilePersistenceManager();
                    try {
	                    $serverProfileNames = $serverProfilesPersistenceManager->getProfiles();
	                    if (sizeof($serverProfileNames) < 2) {
		                    $serverProfileLabel = '';
	                    }
                    }
                    catch (LAMException $e) {
	                    logNewMessage(LOG_ERR, 'Unable to read server profiles: ' . $e->getTitle());
                    }
                    echo $serverProfileLabel . $userName;
                    ?>
                </span>
            </div>
            <a class="lam-header-right lam-menu-icon hide-on-tablet" href="javascript:void(0);" class="icon" onclick="window.lam.topmenu.toggle();">
                <img class="align-middle" width="16" height="16" alt="menu" src="<?php echo $headerPrefix; ?>/graphics/menu.svg">
                <span class="padding0">&nbsp;</span>
            </a>
            <a class="lam-header-right lam-menu-entry" href="<?php echo $headerPrefix; ?>/templates/logout.php" target="_top">
                <img class="align-middle" height="16" width="16" alt="logout" src="<?php echo $headerPrefix; ?>/graphics/exit.png">
                <span class="padding0">&nbsp;<?php echo _("Logout") ?></span>
            </a>
            <?php
            if (is_dir(dirname(__FILE__) . '/../docs/manual')) {
            ?>
                <a class="lam-header-right lam-menu-entry" target="_blank" href="<?php echo $headerPrefix; ?>/docs/manual/index.html">
                    <img class="align-middle" width="16" height="16" alt="help" src="<?php echo $headerPrefix; ?>/graphics/help.png">
                    <span class="padding0">&nbsp;<?php echo _("Help") ?></span>
                </a>
            <?php
            }
            ?>
                <?php
                if (sizeof($tools) > 0) {
                    $toolGroup = new htmlGroup();
                    $row = new htmlResponsiveRow();
	                foreach ($tools as $tool) {
		                $link = $headerPrefix . '/templates/' . $tool->getLink();
	                    $toolLink = new htmlLink($tool->getName(), $link, $headerPrefix . '/graphics/' . $tool->getImageLink());
	                    $row->add($toolLink, 12, 6);
	                }
	                $toolDiv = new htmlDiv('tool-popup', $row, array('hidden tool-popup'));
	                $toolGroup->addElement($toolDiv);
	                $toolLinkContent = new htmlGroup();
	                $toolLinkImage = new htmlImage($headerPrefix . '/graphics/tools.png', '16px', '16px');
	                $toolLinkImage->setCSSClasses(array('align-middle'));
	                $toolLinkContent->addElement($toolLinkImage);
	                $toolLinkSpan = new htmlSpan(new htmlOutputText('&nbsp;&nbsp;' . _('Tools'), false) , array('padding0'));
	                $toolLinkContent->addElement($toolLinkSpan);
	                $toolLink = new htmlContentLink($toolLinkContent, "javascript:void(0);");
	                $toolLink->setOnClick("showSimpleDialog('" . _('Tools') . "', null, '" . _('Close') . "', null, 'tool-popup');");
	                $toolLink->setCSSClasses(array('lam-header-right', 'lam-menu-entry'));
	                $toolGroup->addElement($toolLink);

                    $tabIndex = 0;
                    parseHtml(null, $toolGroup, array(), false, $tabIndex, null);
                }
                $typeManager = new \LAM\TYPES\TypeManager();
                $types = $typeManager->getConfiguredTypes();
                if (!empty($types)) {
                    $accountTypesGroup = new htmlGroup();
	                $row = new htmlResponsiveRow();
	                foreach ($types as $type) {
		                $link = $headerPrefix . '/templates/lists/list.php?type=' . $type->getId();
		                $accountTypeLink = new htmlLink($type->getAlias(), $link, $headerPrefix . '/graphics/' . $type->getIcon());
		                $row->add($accountTypeLink, 12, 6);
	                }
	                $accountTypesDiv = new htmlDiv('account-types-popup', $row, array('hidden account-types-popup'));
	                $accountTypesGroup->addElement($accountTypesDiv);
                    $accountTypesLinkContent = new htmlGroup();
                    $accountTypesLinkImage = new htmlImage($headerPrefix . '/graphics/gear.png', '16px', '16px');
                    $accountTypesLinkImage->setCSSClasses(array('align-middle'));
                    $accountTypesLinkContent->addElement($accountTypesLinkImage);
                    $accountTypesLinkSpan = new htmlSpan(new htmlOutputText('&nbsp;&nbsp;' . _('Accounts'), false) , array('padding0'));
                    $accountTypesLinkContent->addElement($accountTypesLinkSpan);
                    $accountTypesLink = new htmlContentLink($accountTypesLinkContent, "javascript:void(0);");
                    $accountTypesLink->setOnClick("showSimpleDialog('" . _('Accounts') . "', null, '" . _('Close') . "', null, 'account-types-popup');");
                    $accountTypesLink->setCSSClasses(array('lam-header-right', 'lam-menu-entry'));
                    $accountTypesGroup->addElement($accountTypesLink);

	                parseHtml(null, $accountTypesGroup, array(), false, $tabIndex, null);
                }
                ?>

</div>
