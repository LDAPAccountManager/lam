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
session_start();
?>

<html>
	<head>
		<title>
		<?
		echo _("Login");
		?>
		</title>
	</head>
	<body>
		<p align="center"><a href="http://lam.sf.net" target="new_window"><img src="../graphics/banner.jpg" border=1></a></p><hr><br><br>
		<b><p align="center"> <? echo _("Password to enter preferences:"); ?> </b></p>
		<form action="confmain.php" method="post">
			<p align="center"><input type="password" name="passwd">
			<input type="submit" name="submit" value=<? echo _("Ok"); ?> ></p>
		</form>
	</body>
</html>
