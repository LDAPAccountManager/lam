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

	User is logged off from LDAP server, session is destroyed.  
  
*/

// delete key and iv in cookie
setcookie("Key", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0, "/");
setcookie("IV", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", 0, "/");

include_once("../lib/status.inc");
include_once("../lib/ldap.inc");

// start session
session_save_path("../sess");
@session_start();

// close LDAP connection
@$_SESSION["ldap"]->destroy();

setlanguage();

echo $_SESSION['header'];

// destroy session
session_destroy();
unset($_SESSION);

// print logout message
?>

	<title>
		<?php echo _("Logout"); ?>
	</title>
	<link rel="stylesheet" type="text/css" href="../style/layout.css">
	</head>
	<body>
	<p align="center"><a href="http://lam.sf.net" target="new_window">
		<img src="../graphics/banner.jpg" border=1 alt="LDAP Account Manager"></a>
	</p>
	<hr>
	<br>
	<p align="center"><big><b><?php echo _("You have been logged off from LDAP Account Manager."); ?></b></big></p>
	<br><br><br><br><br><a href="../templates/login.php" target="_top"> <?php echo _("Back to Login") ?> </a>
	</body>
</html>
