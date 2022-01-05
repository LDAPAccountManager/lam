<?php
namespace LAM\HEADER;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018 - 2022  Roland Gruber

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
echo "</head><body>\n";

// include all JavaScript files
printJsIncludes($headerPrefix);

printHeader($headerPrefix);

function printHeader(string $headerPrefix): void {
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
        <div class="lam-header-left lam-menu-stay">
            <a href="https://www.ldap-account-manager.org/" target="new_window">
                <img class="align-middle" width="24" height="24" alt="help" src="<?php echo $headerPrefix; ?>/graphics/logo24.png">
                <span class="hide-on-tablet">&nbsp;&nbsp;<?php echo $userName?></span>
                <span class="hide-on-mobile-and-tablet">
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
            <span class="padding0"></span>
        </a>
        <div class="lam-header-right lam-header-menublock">
		<?php
		$typeManager = new \LAM\TYPES\TypeManager();
		$types = $typeManager->getConfiguredTypes();
		if (!empty($types)) {
			$accountTypesGroup = new htmlGroup();
			$accountTypesLink = new htmlLink(_('Accounts'), "javascript:void(0);");
			$accountTypesLink->setOnClick("window.lam.topmenu.openSubmenu(event, 'lam-navigation-types', window.lam.topmenu.subMenuCloseListenerTypes);");
			$accountTypesLink->setOnMouseOver("window.lam.topmenu.openSubmenu(event, 'lam-navigation-types', window.lam.topmenu.subMenuCloseListenerTypes);");
			$accountTypesLink->setCSSClasses(array('lam-menu-entry'));
			$accountTypesGroup->addElement($accountTypesLink);
			$accountList = new htmlGroup();
			foreach ($types as $type) {
				$link = $headerPrefix . '/templates/lists/list.php?type=' . $type->getId();
				$accountTypeLink = new htmlLink($type->getAlias(), $link, $headerPrefix . '/graphics/' . $type->getIcon());
				$accountTypeLink->setCSSClasses(array('lam-menu-entry icon'));
				$accountList->addElement($accountTypeLink);
			}
			$accountTypeEntriesDiv = new htmlDiv(null, $accountList, array('lam-navigation-layer-content'));
			$accountTypesDiv = new htmlDiv('lam-navigation-types', $accountTypeEntriesDiv, array('lam-navigation-layer zeroHeight'));
			$accountTypesGroup->addElement($accountTypesDiv);

			parseHtml(null, $accountTypesGroup, array(), false, $tabIndex, null);
		}
		if (sizeof($tools) > 0) {
			$toolGroup = new htmlGroup();
			$toolLink = new htmlLink(_('Tools'), "javascript:void(0);");
			$toolLink->setOnClick("window.lam.topmenu.openSubmenu(event, 'lam-navigation-tools', window.lam.topmenu.subMenuCloseListenerTools);");
			$toolLink->setOnMouseOver("window.lam.topmenu.openSubmenu(event, 'lam-navigation-tools', window.lam.topmenu.subMenuCloseListenerTools);");
			$toolLink->setCSSClasses(array('lam-menu-entry'));
			$toolGroup->addElement($toolLink);
			$toolList = new htmlGroup();
			foreach ($tools as $tool) {
				$link = $headerPrefix . '/templates/' . $tool->getLink();
				$toolLink = new htmlLink($tool->getName(), $link, $headerPrefix . '/graphics/' . $tool->getImageLink());
				$toolLink->setCSSClasses(array('lam-menu-entry icon'));
				$toolList->addElement($toolLink);
			}
			$toolEntriesDiv = new htmlDiv(null, $toolList, array('lam-navigation-layer-content'));
			$toolDiv = new htmlDiv('lam-navigation-tools', $toolEntriesDiv, array('lam-navigation-layer zeroHeight'));
			$toolGroup->addElement($toolDiv);

			$tabIndex = 0;
			parseHtml(null, $toolGroup, array(), false, $tabIndex, null);
		}
		if (is_dir(dirname(__FILE__) . '/../docs/manual')) {
			?>
            <a class="lam-menu-entry" target="_blank" href="<?php echo $headerPrefix; ?>/docs/manual/index.html">
                <span class="padding0"><?php echo _("Help") ?></span>
            </a>
			<?php
		}
		?>

            <a class="lam-menu-entry" href="<?php echo $headerPrefix; ?>/templates/logout.php" target="_top">
                <span class="padding0"><?php echo _("Logout") ?></span>
            </a>
        </div>
    </div>
    <?php
}
