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

	This test establishes a connection to a LDAP server and checks the
	crypt-functions of ldap.inc.

*/
include_once("../lib/config.inc");
include_once("../lib/ldap.inc");

// check if login page was displayed before
if ($url && $user && $pass){
	$config = new Config();
	$ldap = new Ldap($config);
	echo ("Trying to connect...");
	if ($ldap->connect($user, $pass)) echo "ok";
	echo "<br>Check if __sleep/__wakeup works";
	$ldap->__sleep();
	$ldap->__wakeup();
	echo "<br>Closing connection";
	$ldap->destroy();
	echo "<br><br><br>If you do not see any error messages all should be ok.";
	exit;
}
// display login page
else {
	// generate 256 bit key and initialization vector for user/passwd-encryption
	$key = mcrypt_create_iv(32, MCRYPT_DEV_RANDOM);
	$iv = mcrypt_create_iv(32, MCRYPT_DEV_RANDOM);

	// save both in cookie
	setcookie("Key", base64_encode($key), 0, "/");
	setcookie("IV", base64_encode($iv), 0, "/");
}
?>
<html>
	<head>
		<title>
		<?php
		echo ("Login");
		?>
		</title>
	</head>
	<body>
		<form action="ldap-test.php" method="post">
			<p align="center"><b>Server URL:</b> <input type="text" name="url" value="ldap://localhost"></p>
			<p align="center"><b>Username:</b> <input type="text" name="user"></p>
			<p align="center"><b>Password:</b> <input type="password" name="pass"></p>
			<p align="center"><input type="submit" name="submit" value="Ok"></p>
		</form>
	</body>
</html>
