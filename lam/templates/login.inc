<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Duergner

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


  LDAP Account Manager main login page.
*/
?>

<?

// loading available languages from language.conf file

$languagefile = "../config/language.conf";
if(is_file($languagefile) == True)
{
	$file = fopen($languagefile, "r");
	$i = 0;
	while(!feof($file))
	{
		$line = fgets($file, 1024);
		if($line == "\n" || $line[0] == "#") continue; // ignore comment and empty lines
		$value = explode(":", $line);
		$languages[$i]["link"] = $value[0] . ":" . $value[1];
		$languages[$i]["descr"] = $value[2];
		$i++;
	}
	fclose($file);
}
else
{
	$message = "Unable to load available languages. Setting English as default language. For further instructions please contact the Admin of this site.";
}
?>

<html>
	<head>
		<title>
		<?
		echo _("LDAP Account Manager -Login-");
		?>
		</title>
	</head>
	<body>
		<p align="center"><img src="../graphics/banner.jpg" border=1></p><hr><br><br>
		<b><p align="center"> <? echo _("Enter Username and Password for Account:"); ?> </b></p>
		<? if($error_message != "") { echo "<p align=\"center\">"; echo _($error_message); echo "</p>"; } ?>
		<form action="login.php" method="post">
		<input type="hidden" name="action" value="checklogin">
			<table width="500" align="center" border="0">
				<tr>
					<td width="45%" align="right"><? echo _("Username:") ?></td><td width="10%"></td><td width="45%" align="left"><input type="text" name="username"></td>
				</tr>
				<tr>
					<td width="45%" align="right"><? echo _("Password:") ?></td><td width="10%"></td><td width="45%" align="left"><input type="password" name="passwd"></td>
				</tr>
				<tr>
<? if($message != "") { ?>
					<td width="100%" colspan="3" align="center"><? echo _($message) ?><input type="hidden" name="language" value="english"></td>
<? } else { ?>
					<td width="45%" align="right"><? echo _("Your Language:") ?></td><td width="10%"></td><td width="45%" align="left"><select name="language" size="1"><? for($i = 0; $i < count($languages); $i++) { echo "<option value=\"" . $languages[$i]["link"] . "\">" . $languages[$i]["descr"] . "</option>"; } ?></select></td>
<? } ?>
				</tr>
				<tr>
					<td width="100%" colspan="3" align="center"><input type="submit" name="submit" value=<? echo _("Login"); ?>></td>
				</td>
			</table>
		</form>
	</body>
</html>
