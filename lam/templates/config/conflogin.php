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

// remove settings from session
unset($_SESSION['conf_passwd']);
unset($_SESSION['conf_passwd1']);
unset($_SESSION['conf_passwd2']);
unset($_SESSION['conf_serverurl']);
unset($_SESSION['conf_admins']);
unset($_SESSION['conf_suffusers']);
unset($_SESSION['conf_suffgroups']);
unset($_SESSION['conf_suffhosts']);
unset($_SESSION['conf_minUID']);
unset($_SESSION['conf_maxUID']);
unset($_SESSION['conf_minGID']);
unset($_SESSION['conf_maxGID']);
unset($_SESSION['conf_minMach']);
unset($_SESSION['conf_maxMach']);
unset($_SESSION['conf_usrlstattr']);
unset($_SESSION['conf_grplstattr']);
unset($_SESSION['conf_hstlstattr']);
unset($_SESSION['conf_maxlistentries']);
unset($_SESSION['conf_lang']);
unset($_SESSION['conf_scriptpath']);
unset($_SESSION['conf_scriptserver']);
unset($_SESSION['conf_pwdhash']);
unset($_SESSION['conf_filename']);

// remove config wizard settings
unset($_SESSION['confwiz_config']);
unset($_SESSION['confwiz_ldap']);
unset($_SESSION['confwiz_masterpwd']);

echo $_SESSION['header'];

?>

		<title>
			<?php
				echo _("Login");
			?>
		</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p align="center"><a href="http://lam.sf.net" target="_blank">
			<img src="../../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</p>
		<hr><br><br>
		<!-- form to change existing profiles -->
		<form action="confmain.php" method="post">
		<table align="center" border="2" rules="none" bgcolor="white">
			<tr>
				<td style="border-style:none" rowspan="3" width="20"></td>
				<td style="border-style:none" colspan="2" height="20"></td>
				<td style="border-style:none" rowspan="3" width="20"></td>
			</tr>
			<tr>
				<td style="border-style:none" colspan=2 align="center"><b> <?php echo _("Please enter password to change preferences:"); ?> </b></td>
			</tr>
			<tr><td style="border-style:none" colspan=2 >&nbsp;</td></tr>
<?php
	// print message if login was incorrect
	if ($message) {
		echo ("<tr><td style=\"border-style:none\" rowspan=\"2\"></td>" .
			"<td style=\"border-style:none\" colspan=2 align=\"center\"><b><font color=red>" . $message . "</font></b></td>" .
			"<td style=\"border-style:none\" rowspan=\"2\"></td></tr>");
		echo "<tr><td style=\"border-style:none\" colspan=2 >&nbsp;</td></tr>";
	}
?>
			<tr>
				<td style="border-style:none" rowspan="4" width="20"></td>
				<td style="border-style:none" colspan=2 align="center">
					<select size=1 name="filename">
					<?php
						$files = getConfigProfiles();
							$conf = new CfgMain();
							$defaultprofile = $conf->default;
							for ($i = 0; $i < sizeof($files); $i++) {
								if ($files[$i] == $defaultprofile) echo ("<option selected>" . $files[$i] . "</option>\n");
								else echo ("<option>" . $files[$i] . "</option>\n");
							}
					?>
					</select>
					<input type="password" name="passwd">
					<input type="submit" name="submit" value=" <?php echo _("Ok"); ?> ">
					<a href="../help.php?HelpNumber=200" target="lamhelp"><?php echo _("Help") ?></a></td>
				<td style="border-style:none" rowspan="4" width="20"></td>
			</tr>
			<tr>
				<td  style="border-style:none"colspan=2>&nbsp;</td>
			</tr>
			<tr>
				<td style="border-style:none" align="left">
					<b><a href="profmanage.php"><?php echo _("Manage profiles") ?></a></b>
				</td>
				<td style="border-style:none" align="right">
					<b><a href="../confwiz/start.php"><?php echo _("Configuration wizard") ?></a></b>
				</td>
			</tr>
			<tr>
				<td style="border-style:none" colspan=2 height="20"></td>
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
