<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2009  Roland Gruber

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
* Login page to change the preferences.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');
/** Used to print status messages */
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// get error message from confmain.php
if (isset($_SESSION['conf_message'])) $message = $_SESSION['conf_message'];

// remove settings from session
$sessionKeys = array_keys($_SESSION);
for ($i = 0; $i < sizeof($sessionKeys); $i++) {
	if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
}

echo $_SESSION['header'];

?>

		<title>
			<?php
				echo _("Login");
			?>
		</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
	</head>
	<body>
		<?php
			echo "<script type=\"text/javascript\" src=\"../wz_tooltip.js\"></script>\n";
			// set focus on password field
			echo "<script type=\"text/javascript\" language=\"javascript\">\n";
			echo "<!--\n";
			echo "window.onload = function() {\n";
				echo "loginField = document.getElementsByName('passwd')[0];\n";
				echo "loginField.focus();\n";
			echo "}\n";
			echo "//-->\n";
			echo "</script>\n";
		?>
		<p align="center"><a href="http://lam.sourceforge.net" target="_blank">
			<img src="../../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</p>
		<hr><br><br>
		<!-- form to change existing profiles -->
		<form action="confmain.php" method="post">
		<table align="center" border="2" rules="none" bgcolor="white">
			<tr>
				<td style="border-style:none" rowspan="3" width="20"></td>
				<td style="border-style:none" height="20"></td>
				<td style="border-style:none" rowspan="3" width="20"></td>
			</tr>
			<tr>
				<td style="border-style:none" align="center"><b> <?php echo _("Please enter your password to change the server preferences:"); ?> </b></td>
			</tr>
			<tr><td style="border-style:none" >&nbsp;</td></tr>
<?php
	$files = getConfigProfiles();
	if (sizeof($files) < 1) $message = _("No server profiles found. Please create one.");
	// print message if login was incorrect or no config profiles are present
	if (isset($message)) {  // $message is set by confmain.php (requires conflogin.php then)
		echo "<tr>\n";
			echo "<td style=\"border-style:none\" rowspan=\"2\"></td>\n";
			echo "<td style=\"border-style:none\" align=\"center\"><b><font color=red>" . $message . "</font></b></td>\n";
			echo "<td style=\"border-style:none\" rowspan=\"2\"></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
			echo "<td style=\"border-style:none\" >&nbsp;</td>\n";
		echo "</tr>\n";
	}
?>
			<tr>
				<td style="border-style:none" rowspan="4" width="20"></td>
				<td style="border-style:none" align="center">
					<?php
						if (sizeof($files) > 0) {
							echo "<select size=1 name=\"filename\">\n";
							$conf = new LAMCfgMain();
							$defaultprofile = $conf->default;
							for ($i = 0; $i < sizeof($files); $i++) {
								if ($files[$i] == $defaultprofile) echo ("<option selected>" . $files[$i] . "</option>\n");
								else echo ("<option>" . $files[$i] . "</option>\n");
							}
							echo "</select>\n";
						}
						else echo "<select disabled size=1 name=\"filename\">\n<option></option>\n</select>\n";
						if (sizeof($files) > 0) echo "<input type=\"password\" name=\"passwd\">\n";
						else echo "<input disabled type=\"password\" name=\"passwd\">\n";
						if (sizeof($files) > 0) echo "<input type=\"submit\" name=\"submit\" value=\"" . _("Ok") . "\">\n";
						else echo "<input disabled type=\"submit\" name=\"submit\" value=\"" . _("Ok") . "\">&nbsp;\n";
						// help link
						printHelpLink(getHelp('', '200'), '200');
					?>
				</td>
				<td style="border-style:none" rowspan="4" width="20"></td>
			</tr>
			<tr>
				<td  style="border-style:none">&nbsp;</td>
			</tr>
			<tr>
				<td style="border-style:none" align="center">
					<b><a href="profmanage.php"><?php echo _("Manage server profiles") ?></a></b>
				</td>
			</tr>
			<tr>
				<td style="border-style:none" height="20"></td>
			</tr>
		</table>
		</form>

		<p><br><br><br><br><br></p>

		<!-- back to login page -->
		<p>
			<a href="../login.php"> <?php echo _("Back to Login"); ?> </a>
		</p>

	</body>
</html>
