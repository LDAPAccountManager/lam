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


  Login page to change the preferences.
*/

include_once('../../lib/config.inc');
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// remove settings from session
session_unregister('passwd');
session_unregister('passwd1');
session_unregister('passwd2');
session_unregister('serverurl');
session_unregister('admins');
session_unregister('suffusers');
session_unregister('suffgroups');
session_unregister('suffhosts');
session_unregister('minUID');
session_unregister('maxUID');
session_unregister('minGID');
session_unregister('maxGID');
session_unregister('minMach');
session_unregister('maxMach');
session_unregister('usrlstattr');
session_unregister('grplstattr');
session_unregister('hstlstattr');
session_unregister('maxlistentries');
session_unregister('lang');
session_unregister('scriptpath');
session_unregister('scriptserver');
session_unregister('samba3');
session_unregister('domainSID');
session_unregister('filename');

echo $_SESSION['header'];

?>

<html>
	<head>
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
		<table border=0 align="center">
			<tr>
				<td colspan=4 align="center"><b> <?php echo _("Please enter password to change preferences:"); ?> </b></td>
			</tr>
			<tr><td colspan=4 >&nbsp;</td></tr>
<?php
	// print message if login was incorrect
	if ($message) echo ("<tr><td colspan=4 align=\"center\"><font color=red>" . $message . "</font></td></tr>");
?>
			<tr>
				<td>
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
				</td>
				<td align="center"><input type="password" name="passwd"></td>
				<td>
					<input type="submit" name="submit" value= <?php echo _("Ok"); ?>
				</td>
				<td><a href="../help.php?HelpNumber=200" target="lamhelp"><?php echo _("Help") ?></a></td>
			</tr>
			<tr>
				<td colspan=3>&nbsp;</td>
			</tr>
			<tr>
				<td colspan=3 align="center">
					<b><a href="profmanage.php"><?php echo _("Manage profiles") ?></a></b>
				</td>
			</tr>
		</table>
		</form>

		<p><br><br><br><br><br><br><br></p>

		<!-- back to login page -->
		<p>
			<a href="../login.php"> <?php echo _("Back to Login"); ?> </a>
		</p>

	</body>
</html>
