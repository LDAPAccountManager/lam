<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2019  Roland Gruber

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
* User is logged off from LDAP server, session is destroyed.
*
* @package main
* @author Roland Gruber
*/


/** security functions */
include_once(__DIR__ . "/../lib/security.inc");
/** Used to display status messages */
include_once(__DIR__ . "/../lib/status.inc");
/** LDAP settings are deleted at logout */
include_once(__DIR__ . "/../lib/ldap.inc");

// delete key and iv in cookie
$cookieOptions = lamDefaultCookieOptions();
$cookieOptions['expires'] = 0;
setcookie("Key", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", $cookieOptions);
setcookie("IV", "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", $cookieOptions);

// start session
startSecureSession();

// log message
if (isset($_SESSION['loggedIn']) || ($_SESSION['loggedIn'] === true)) {
	logNewMessage(LOG_NOTICE, 'User logged off.');

	// close LDAP connection
	if (!empty($_SESSION["ldap"])) {
		$_SESSION["ldap"]->destroy();
	}
}

setlanguage();

// destroy session
session_destroy();
unset($_SESSION);

// redirect to login page
metaRefresh('login.php');
?>
