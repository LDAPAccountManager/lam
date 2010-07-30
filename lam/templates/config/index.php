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
* Displays links to all configuration pages.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
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
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
	</head>
	<body>
		<table border=0 width="100%" class="lamHeader">
			<tr>
				<td align="left" height="30">
					<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="../../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
				</td>
				<td align="right" height=20>
					<a href="../login.php"><IMG alt="configuration" src="../../graphics/undo.png">&nbsp;<?php echo _("Back to login") ?></a>
				</td>
			</tr>
		</table>
		<br><br>
		<fieldset>
			<legend><b> <?php echo _("LAM configuration"); ?> </b></legend>
		<TABLE border="0">
		<?php
			if (is_dir("../selfService")) echo "<tr><td rowspan=4 width=20>&nbsp;</td><td></td><td></td></tr>\n";
			else echo "<tr><td rowspan=3 width=20>&nbsp;</td><td></td><td></td></tr>\n";
		?>
		<TR>
			<TD width="60" height="70">
			<a href="mainlogin.php">
				<IMG height="32" width="32" alt="general settings" src="../../graphics/bigTools.png">
			</a>
			</TD>
			<TD><BIG>
			<a href="mainlogin.php">
				<?php echo _("Edit general settings") ?>
			</a></BIG>
			</TD>
		</TR>
		<TR>
			<TD height="70">
			<a href="conflogin.php" target="_self">
				<IMG height="32" width="32" alt="server settings" src="../../graphics/profiles.png">
			</a>
			</TD>
			<TD><BIG>
			<a href="conflogin.php" target="_self">
				<?php echo _("Edit server profiles"); ?>
			</a></BIG>
			</TD>
		</TR>
		<?php
		if (is_dir("../selfService")) {
			echo "<TR>\n";
				echo "<TD height=\"70\">\n";
				echo "<a href=\"../selfService/adminLogin.php\" target=\"_self\">\n";
					echo "<IMG height=\"32\" width=\"32\" alt=\"self service\" src=\"../../graphics/bigPeople.png\">\n";
				echo "</a>\n";
				echo "</TD>\n";
				echo "<TD><BIG>\n";
				echo "<a href=\"../selfService/adminLogin.php\" target=\"_self\">\n";
					echo _("Edit self service");
				echo "</a></BIG>\n";
				echo "</TD>\n";
			echo "</TR>\n";
		}
		?>
		</TABLE>
		</fieldset>
		<p><br><br></p>

	</body>
</html>
