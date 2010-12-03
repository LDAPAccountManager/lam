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
* Head part of page which includes links to lists etc.
*
* @package main
* @author Roland Gruber
*/

// number of list views (users, groups, ...)
$types = $_SESSION['config']->get_ActiveTypes();

$headerPrefix = "";
if (is_file("../login.php")) $headerPrefix = "../";
elseif (is_file("../../login.php")) $headerPrefix = "../../";

/** tool definitions */
include_once($headerPrefix . "../lib/tools.inc");

// HTML header and title
echo $_SESSION['header'];
echo "<title>LDAP Account Manager</title>\n";

// include all CSS files
$cssDirName = dirname(__FILE__) . '/../style';
$cssDir = dir($cssDirName);
while ($cssEntry = $cssDir->read()) {
	if (substr($cssEntry, strlen($cssEntry) - 4, 4) != '.css') continue;
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"" . $headerPrefix . "../style/" . $cssEntry . "\">\n";
}

echo "</head><body>\n";

// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/lib';
$jsDir = dir($jsDirName);
$jsFiles = array();
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	$jsFiles[] = $jsEntry;
}
sort($jsFiles);
foreach ($jsFiles as $jsEntry) {
	echo "<script type=\"text/javascript\" src=\"" . $headerPrefix . "lib/" . $jsEntry . "\"></script>\n";
}

// get tool list
$availableTools = getTools();
// sort tools
$toSort = array();
for ($i = 0; $i < sizeof($availableTools); $i++) {
	$myTool = new $availableTools[$i]();
	if ($myTool->getRequiresWriteAccess() && !checkIfWriteAccessIsAllowed()) {
		continue;
	}
	if ($myTool->getRequiresPasswordChangeRights() && !checkIfPasswordChangeIsAllowed()) {
		continue;
	}
	$toSort[$availableTools[$i]] = $myTool->getPosition();
}
asort($toSort);
$tools = array();
foreach ($toSort as $key => $value) {
	$tools[] = new $key();
}
?>

<table border=0 width="100%" class="lamHeader ui-corner-all">
	<tr>
		<td align="left" height="30">
			<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="<?php echo $headerPrefix; ?>../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
		</td>
	<td align="right" height=30>
	<ul id="foo" class="dropmenu">
		<li><a href="<?php echo $headerPrefix; ?>logout.php" target="_top"><img alt="logout" src="<?php echo $headerPrefix; ?>../graphics/exit.png">&nbsp;<?php echo _("Logout") ?></a></li>
		<li>
			<a href="<?php echo $headerPrefix; ?>tools.php"><img alt="tools" src="<?php echo $headerPrefix; ?>../graphics/tools.png">&nbsp;<?php echo _("Tools") ?>&nbsp;&nbsp;&nbsp;</a>
				<ul>
				<?php
					for ($i = 0; $i < sizeof($tools); $i++) {
						$subTools = $tools[$i]->getSubTools();
						echo '<li title="' . $tools[$i]->getDescription() . '">';
						$link = $headerPrefix . $tools[$i]->getLink();
						echo '<a href="' . $link . "\">\n";
						echo '<img alt="" src="' . $headerPrefix . '../graphics/' . $tools[$i]->getImageLink() . '"> ' . $tools[$i]->getName();
						echo "</a>\n";
						if (sizeof($subTools) > 0) {
							echo "<ul>\n";
							for ($s = 0; $s < sizeof($subTools); $s++) {
								echo "<li title=\"" . $subTools[$s]->description . "\">\n";
								echo "<a href=\"" . $headerPrefix . $subTools[$s]->link . "\">\n";
								echo '<img width=16 height=16 alt="" src="' . $headerPrefix . '../graphics/' . $subTools[$s]->image . '"> ' . $subTools[$s]->name;
								echo "</a>\n";
								echo "</li>\n";
							}
							echo "</ul>\n";
						}
						echo "</li>\n";
					}
				?>
			</ul>
		</li>
		<?php
		if ($_SESSION['config']->get_Suffix('tree') != "") {
		?>
	    <li>
			<a href="<?php echo $headerPrefix; ?>tree/treeViewContainer.php"><img alt="tools" src="<?php echo $headerPrefix; ?>../graphics/process.png">&nbsp;<?php echo _("Tree view") ?></a>
		</li>
		<?php
		}
		?>
	</ul>
	</td>
	</tr>
</table>

<script type="text/javascript">
$(document).ready(function() {
	        $('#foo').dropmenu(
	            {
	            	effect  : 'slide',
	            	nbsp    : true,
	            	timeout : 350,
	            	speed   : 'fast'
	            }
	        );
	    });
</script>

<br>
<div class="ui-tabs ui-widget ui-widget-content ui-corner-all">
<ul class="ui-tabs-nav ui-helper-reset ui-helper-clearfix ui-widget-header ui-corner-all">
	<?php
		$linkList = array();
		for ($i = 0; $i < sizeof($types); $i++) {
			$link = '<a href="' . $headerPrefix . 'lists/list.php?type=' . $types[$i] . '">' .
				'<img alt="' . $types[$i] . '" src="' . $headerPrefix . '../graphics/' . $types[$i] . '.png">&nbsp;' .
				getTypeAlias($types[$i]) . '</a>';
			echo '<li id="tab_' . $types[$i] . '" class="ui-state-default ui-corner-top">';
			echo $link;
			echo "</li>\n";
		}
	?>
</ul>

