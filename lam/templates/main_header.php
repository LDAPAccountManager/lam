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
* Head frame in main window, includes links to lists etc.
*
* @package main
* @author Roland Gruber
*/

/** access to configuration options */
include_once ("../lib/config.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();

echo $_SESSION['header'];

// number of list views (users, groups, ...)
$lists = 0;
if ($_SESSION['config']->get_Usersuffix() != "") $lists++;
if ($_SESSION['config']->get_Groupsuffix() != "") $lists++;
if ($_SESSION['config']->get_Hostsuffix() != "") $lists++;
if ($_SESSION['config']->get_Suffix('tree') != "") $lists++;

?>

	<title></title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
</head>

<body>
<table border=0 width="100%">
	<tr>
		<td width="200">
			<a href="tools.php" target="mainpart"><?php echo _("Tools") ?></a>
		</td>
		<?php
			echo "<td colspan=$lists align=\"center\">\n";
		?>
			<a href="http://lam.sf.net" target="new_window"><img src="../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</td>
	<td width="200" align="right" height=20><a href="./logout.php" target="_top"><big><b><?php echo _("Logout") ?></b></big></a></td>
	</tr>
	<tr>
		<?php
			$temp = $lists + 2;
			echo "<td colspan=$temp><font size=1>&nbsp;</font></td>\n";
		?>
	</tr>
	<tr>
		<td></td>
		<?php
			if ($_SESSION['config']->get_Suffix('tree') != "") {
				echo '<td width="120" align="center"><a href="./tree/tree_view.php" target="mainpart"><big>' . _("Tree view") . '</big></a></td>' . "\n";
			}
			if ($_SESSION['config']->get_Usersuffix() != "") {
				echo '<td width="120" align="center"><a href="./lists/listusers.php" target="mainpart"><big>' . _("Users") . '</big></a></td>' . "\n";
			}
			if ($_SESSION['config']->get_Groupsuffix() != "") {
				echo '<td width="120" align="center"><a href="./lists/listgroups.php" target="mainpart"><big>' . _("Groups") . '</big></a></td>' . "\n";
			}
			if ($_SESSION['config']->get_Hostsuffix() != "") {
				echo '<td width="120" align="center"><a href="./lists/listhosts.php" target="mainpart"><big>' . _("Hosts") . '</big></a></td>' . "\n";
			}
		?>
		<td></td>
	</tr>
</table>
</body>
</html>
