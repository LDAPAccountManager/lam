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
* Displays links to all configuration pages.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

echo $_SESSION['header'];

?>

		<title>
			<?php
				echo _("Configuration overview");
			?>
		</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p align="center"><a href="http://lam.sourceforge.net" target="_blank">
			<img src="../../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</p>
		<hr>
		<H1 align="center">
			<?php
				echo _("LAM configuration");
			?>
		</H1>
		<TABLE border="0">
		<?php
			if (is_dir("../selfService")) echo "<tr><td rowspan=4 width=20>&nbsp;</td><td></td><td></td></tr>\n";
			else echo "<tr><td rowspan=2 width=20>&nbsp;</td><td></td><td></td></tr>\n";
		?>
		<TR>
			<TD width="60" height="70">
			<a href="mainlogin.php">
				<IMG height="50" width="50" alt="general settings" src="../../graphics/bigTools.png">
			</a>
			</TD>
			<TD>
			<a href="mainlogin.php">
				<?php echo _("Edit general settings") ?>
			</a>
			</TD>
		</TR>
		<TR>
			<TD height="70">
			<a href="conflogin.php" target="_self">
				<IMG height="50" width="50" alt="server settings" src="../../graphics/bigServers.png">
			</a>
			</TD>
			<TD>
			<a href="conflogin.php" target="_self">
				<?php echo _("Edit server profiles"); ?>
			</a>
			</TD>
		</TR>
		<?php
		if (is_dir("../selfService")) {
			echo "<TR>\n";
				echo "<TD height=\"70\">\n";
				echo "<a href=\"../selfService/confLogin.php\" target=\"_self\">\n";
					echo "<IMG height=\"50\" width=\"50\" alt=\"self service\" src=\"../../graphics/bigPeople.png\">\n";
				echo "</a>\n";
				echo "</TD>\n";
				echo "<TD>\n";
				echo "<a href=\"../selfService/confLogin.php\" target=\"_self\">\n";
					echo _("Edit self service");
				echo "</a>\n";
				echo "</TD>\n";
			echo "</TR>\n";
		}
		?>
		</TABLE>
		<p><br><br><br><br><br></p>

		<!-- back to login page -->
		<p>
			<a href="../login.php"> <?php echo _("Back to Login"); ?> </a>
		</p>
	</body>
</html>
