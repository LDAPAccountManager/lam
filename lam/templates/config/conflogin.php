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
if (strtolower(session_module_name()) == 'files') {
	session_save_path(dirname(__FILE__) . '/../../sess');
}
session_start();
session_regenerate_id(true);

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
	<?php 
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../../style';
		$cssDir = dir($cssDirName);
		while ($cssEntry = $cssDir->read()) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) != '.css') continue;
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
		}
	?>
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
	</head>
	<body>
		<?php
			// include all JavaScript files
			$jsDirName = dirname(__FILE__) . '/../lib';
			$jsDir = dir($jsDirName);
			$jsFiles = array();
			while ($jsEntry = $jsDir->read()) {
				if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
				$jsFiles[] = $jsEntry;
			}
			sort($jsFiles);
			foreach ($jsFiles as $jsEntry) {
				echo "<script type=\"text/javascript\" src=\"../lib/" . $jsEntry . "\"></script>\n";
			}
			// set focus on password field
			?>
			<script type="text/javascript" language="javascript">
			<!--
			window.onload = function() {
				loginField = document.getElementsByName('passwd')[0];
				loginField.focus();
			}
			jQuery(document).ready(function() {
				jQuery('#submitButton').button();
			});
			//-->
			</script>
		<table border=0 width="100%" class="lamHeader ui-corner-all">
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
						if (sizeof($files) > 0) echo "<button id=\"submitButton\" class=\"smallPadding\" name=\"submit\">" . _("Ok") . "</button>\n";
						else echo "<button id=\"submitButton\" class=\"smallPadding\" name=\"submit\" disabled>" . _("Ok") . "</button>&nbsp;\n";
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

		<p><br><br></p>


	</body>
</html>
