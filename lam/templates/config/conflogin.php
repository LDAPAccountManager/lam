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

// start session
session_save_path("../../sess");
session_start();

echo ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
echo ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">");

?>

<html>
	<head>
		<title>
			<?
				echo _("Login");
			?>
		</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p align="center"><a href="http://lam.sf.net" target="new_window">
			<img src="../../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
		</p>
		<hr><br><br>
		<form action="confmain.php" method="post">
		<table border=0 align="center">
			<tr>
				<td colspan=3 align="center"><b> <? echo _("Password to enter preferences:"); ?> </b></td>
			</tr>
			<tr>
				<td align="center"><input type="password" name="passwd"></td>
				<td><input type="submit" name="submit" value= <? echo _("Ok"); ?> </td>
				<td><a href="../help.php?HelpNumber=200" target="lamhelp"><? echo _("Help") ?></a></td>
			</tr>
		</table>
		</form>
	</body>
</html>
