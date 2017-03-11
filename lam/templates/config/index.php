<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2016  Roland Gruber

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

		<meta name="viewport" content="width=device-width, initial-scale=1.0">
		<title>
			<?php
				echo _("Configuration overview");
			?>
		</title>
		<link rel="stylesheet" type="text/css" href="../../style/responsive/105_normalize.css">
		<link rel="stylesheet" type="text/css" href="../../style/responsive/110_foundation.css">
	<?php
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../../style';
		$cssDir = dir($cssDirName);
		$cssFiles = array();
		$cssEntry = $cssDir->read();
		while ($cssEntry !== false) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) == '.css') {
				$cssFiles[] = $cssEntry;
			}
			$cssEntry = $cssDir->read();
		}
		sort($cssFiles);
		foreach ($cssFiles as $cssEntry) {
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
		}
	?>
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
		<link rel="icon" href="../../graphics/logo136.png">
	</head>
	<body class="admin">
		<table class="lamTop ui-corner-all">
			<tr>
				<td align="left">
					<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">LDAP Account Manager</a>
				</td>
			</tr>
		</table>
		<br><br>
		<TABLE border="0" width="100%" class="ui-corner-all roundedShadowBox">
		<?php
			if (isLAMProVersion()) {
				echo "<tr><td rowspan=4 width=20>&nbsp;</td><td></td><td></td></tr>\n";
			}
			else {
				echo "<tr><td rowspan=3 width=20>&nbsp;</td><td></td><td></td></tr>\n";
			}
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
		if (isLAMProVersion()) {
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
		<p><br></p>

		<?php
		if (isLAMProVersion()) {
			include_once("../../lib/env.inc");
			$printer = new \LAM\ENV\LAMLicenseInfoPrinter();
			$printer->printLicenseInfo();
			echo "<br><br>";
		}
		?>

		<p>&nbsp;<a href="../login.php"><IMG alt="back" src="../../graphics/undo.png">&nbsp;<?php echo _("Back to login") ?></a></p>
		<p><br><br></p>

	</body>
</html>
