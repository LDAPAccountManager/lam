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
include_once ("../lib/config.inc");

// start session
session_save_path("../sess");
@session_start();

setlanguage();

echo $_SESSION['header'];
?>

	<title></title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
</head>

<body>
<table border=0 width="100%">
	<tr>
    	<td width="100" align="left"><a href="./profedit/profilemain.php" target="mainpart"><?php echo _("Profile Editor"); ?></a></td>
		<?php
			// Samba 3 has more list views
			if ($_SESSION['config']->is_samba3()) echo "<td rowspan=3 colspan=4 align=\"center\">\n";
			else echo "<td rowspan=3 colspan=3 align=\"center\">\n";
		?>
			<a href="http://lam.sf.net" target="new_window"><img src="../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</td>
	<td width="100" align="right"><a href="./logout.php" target="_top"><big><b><?php echo _("Logout") ?></b></big></a></td>
	</tr>
	<tr>
    	<td align="left"><a href="ou_edit.php" target="mainpart"><?php echo _("OU-Editor") ?></a></td>
		<td rowspan=2></td>
	</tr>
	<tr>
    	<td align="left"><a href="masscreate.php" target="mainpart"><?php echo _("File Upload") ?></a></td>
	</tr>
	<tr>
		<?php
			// Samba 3 has more list views
			if ($_SESSION['config']->is_samba3()) echo "<td colspan=6><font size=1>&nbsp;</font></td>\n";
			else echo "<td colspan=5><font size=1>&nbsp;</font></td>\n";
		?>
	</tr>
	<tr>
		<td></td>
		<?php
			// Samba 3 has more list views
			if ($_SESSION['config']->is_samba3()) {
				echo '<td width="120" align="center"><a href="./lists/listdomains.php" target="mainpart">' . _("Domains") . '</a></td>' . "\n";
				echo '<td width="120" align="center"><a href="./lists/listusers.php" target="mainpart">' . _("Users") . '</a></td>' . "\n";
				echo '<td width="120" align="center"><a href="./lists/listgroups.php" target="mainpart">' . _("Groups") . '</a></td>' . "\n";
				echo '<td width="120" align="center"><a href="./lists/listhosts.php" target="mainpart">' . _("Hosts") . '</a></td>' . "\n";
			}
			else {
				echo '<td width="200" align="center"><a href="./lists/listusers.php" target="mainpart">' . _("Users") . '</a></td>' . "\n";
				echo '<td width="200" align="center"><a href="./lists/listgroups.php" target="mainpart">' . _("Groups") . '</a></td>' . "\n";
				echo '<td width="200" align="center"><a href="./lists/listhosts.php" target="mainpart">' . _("Hosts") . '</a></td>' . "\n";
			}
		?>
		<td></td>
	</tr>
</table>
</body>
</html>
