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
?>

<table border=0 width="100%" class="lamHeader ui-corner-all">
	<tr>
		<td align="left" height="30" class="nowrap">
			<a class="lamLogo" href="https://www.ldap-account-manager.org/" target="new_window">
				<span class="hide-on-tablet">&nbsp;</span>
				<span class="hide-on-mobile">
				<?php
				echo getLAMVersionText();
				?>
				</span>
			</a>
		</td>
		<td align="left" height="30" class="nowrap header-user-label">
			<?php
				$userData = $_SESSION['ldap']->getUserName();
				$userName = extractRDNValue($userData);
			?>
			<span class="hide-on-mobile">
			<?php
				echo '&nbsp;&nbsp;<small title="' . $userData . '">';
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
				echo '(' . $serverProfileLabel . $userName . ')';
				echo '</small>';
			?>
			</span>
			<span class="hide-on-tablet">
			<?php
				echo '&nbsp;&nbsp;<small title="' . $userData . '">';
				echo $userName;
				echo '</small>';
			?>
			</span>
			<?php
				$userData = null;
			?>
		</td>
	<td align="right" height=30 width="100%">
	<ul id="dropmenu" class="dropmenu">
		<li><a href="<?php echo $headerPrefix; ?>/templates/logout.php" target="_top"><img class="align-middle" height="16" width="16" alt="logout" src="<?php echo $headerPrefix; ?>/graphics/exit.png"><span class="hide-on-mobile padding0">&nbsp;<?php echo _("Logout") ?></span></a></li>
		<?php
		if (is_dir(dirname(__FILE__) . '/../docs/manual')) {
		?>
	    <li>
			<a target="_blank" href="<?php echo $headerPrefix; ?>/docs/manual/index.html"><img class="align-middle" width="16" height="16" alt="help" src="<?php echo $headerPrefix; ?>/graphics/help.png"><span class="hide-on-mobile padding0">&nbsp;<?php echo _("Help") ?>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</span></a>
		</li>
		<?php
		}
		if (sizeof($tools) > 0) {
		?>
		<li>
			<a href="<?php echo $headerPrefix; ?>/templates/tools.php"><img class="align-middle" height="16" width="16" alt="tools" src="<?php echo $headerPrefix; ?>/graphics/tools.png"><span class="hide-on-mobile padding0">&nbsp;<?php echo _("Tools") ?></span></a>
				<ul>
				<?php
					foreach ($tools as $tool) {
						echo '<li title="' . $tool->getDescription() . '">';
						$link = $headerPrefix . '/templates/' . $tool->getLink();
						echo '<a href="' . $link . "\">\n";
						echo '<img class="max16" height="16px" width="16px" alt="" src="' . $headerPrefix . '/graphics/' . $tool->getImageLink() . '"> ' . $tool->getName();
						echo "</a>\n";
						echo "</li>\n";
					}
				?>
			</ul>
		</li>
		<?php
		}
		?>
	</ul>
	</td>
	</tr>
</table>

<script type="text/javascript">
jQuery(document).ready(function() {
	jQuery('#dropmenu').dropmenu({
		effect  : 'slide',
		nbsp    : true,
		timeout : 350,
		speed   : 'fast'
	});
});
</script>


<?php
printTypeTabs($headerPrefix);

function printTypeTabs($headerPrefix) {
	echo '<div class="ui-tabs ui-corner-all ui-widget ui-widget-content">';
	echo '<ul class="ui-tabs-nav ui-corner-all ui-helper-reset ui-helper-clearfix ui-widget-header">';
	$typeManager = new \LAM\TYPES\TypeManager();
	$types = $typeManager->getConfiguredTypes();
	foreach ($types as $type) {
		if ($type->isHidden()) {
			continue;
		}
		$link = '<a class="ui-tabs-anchor" href="' . $headerPrefix . '/templates/lists/list.php?type=' . $type->getId() .
		'" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">' .
		'<img height="16" width="16" alt="' . $type->getId() . '" src="' . $headerPrefix . '/graphics/' . $type->getIcon() . '">&nbsp;' .
		$type->getAlias() . '</a>';
		echo '<li id="tab_' . $type->getId() . '" class="ui-tabs-tab ui-corner-top ui-state-default ui-tab">';
		echo $link;
		echo "</li>\n";
	}
	echo '</ul>';
}

