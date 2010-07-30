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
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	echo "<script type=\"text/javascript\" src=\"" . $headerPrefix . "lib/" . $jsEntry . "\"></script>\n";
}

?>

<table border=0 width="100%" class="lamHeader">
	<tr>
		<td align="left" height="30">
			<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="<?php echo $headerPrefix; ?>../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
		</td>
	<td align="right" height=20>
		<a href="<?php echo $headerPrefix; ?>tools.php"><img alt="tools" src="<?php echo $headerPrefix; ?>../graphics/tools.png">&nbsp;<?php echo _("Tools") ?></a>
		&nbsp;&nbsp;&nbsp;
		<a href="<?php echo $headerPrefix; ?>logout.php" target="_top"><img alt="logout" src="<?php echo $headerPrefix; ?>../graphics/exit.png">&nbsp;<?php echo _("Logout") ?></a>
		&nbsp;
	</td>
	</tr>
</table>

<br>
<div id="headerTabs" class="ui-tabs">
<ul class="ui-tabs-nav ui-helper-reset ui-helper-clearfix">
	<?php
		$linkList = array();
		if ($_SESSION['config']->get_Suffix('tree') != "") {
			$link = '<a href="' . $headerPrefix . 'tree/treeViewContainer.php"><img alt="tree view" src="' . $headerPrefix . '../graphics/process.png">&nbsp;' . _("Tree view") . '</a>' . "\n";
			echo '<li id="tab_tree" class="ui-state-default ui-corner-top">';
			echo $link;
			echo "</li>\n";
		}
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
</div>

