<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
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
* Head frame in main window, includes links to lists etc.
*
* @package main
* @author Roland Gruber
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration options */
include_once("../lib/config.inc");
/** self service functions */
include_once("../lib/selfService.inc");

// start session
startSecureSession();

setlanguage();

echo $_SESSION['header'];

// number of list views (users, groups, ...)
$types = $_SESSION['config']->get_ActiveTypes();

?>

	<title></title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
</head>

<body>
<table border=0 width="100%">
	<tr>
		<td width="200">
			<?PHP
			if (!isLAMProVersion()) {
				echo "<a href=\"http://lam.sourceforge.net/sponsors/donations.htm\" target=\"_blank\"><img alt=\"donations\" src=\"../graphics/smile.png\">&nbsp;" . _("Donate") . "</a>";
				echo "<br><br>";
			}
			?>
			<a href="tools.php" target="mainpart"><img alt="tools" src="../graphics/tools.png">&nbsp;<BIG><B><?php echo _("Tools") ?></B></BIG></a>
		</td>
		<td align="center">
			<a href="http://lam.sourceforge.net" target="new_window"><img src="../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</td>
	<td width="200" align="right" height=20><a href="./logout.php" target="_top"><img alt="logout" src="../graphics/exit.png">&nbsp;<big><b><?php echo _("Logout") ?></b></big></a></td>
	</tr>
</table>
	<p align="center">
		<?php
			$linkList = array();
			if ($_SESSION['config']->get_Suffix('tree') != "") {
				$linkList[] = '<a href="./tree/tree_view.php" target="mainpart"><img alt="tree view" src="../graphics/process.png">&nbsp;<big>' . _("Tree view") . '</big></a>' . "\n";
			}
			for ($i = 0; $i < sizeof($types); $i++) {
					$linkList[] = '<a href="./lists/list.php?type=' . $types[$i] . '" target="mainpart">' .
					'<img alt="' . $types[$i] . '" src="../graphics/' . $types[$i] . '.png">&nbsp;' .
					'<big>' . getTypeAlias($types[$i]) . '</big></a>';
			}
			echo implode('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;', $linkList);
		?>
	</p>
</body>
</html>
