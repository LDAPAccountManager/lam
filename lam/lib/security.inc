<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2006  Roland Gruber

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
* This file includes functions to perform several security checks on each page load.
*
* @package lib
* @author Roland Gruber
*/

/**
 * Starts a session and checks the environment.
 * The script is stopped if one of the checks fail.
 */
function startSecureSession() {
	// start session
	if (isset($_SESSION)) unset($_SESSION);
	$sessionDir = substr(__FILE__, 0, strlen(__FILE__) - 17) . "/sess";
	session_save_path($sessionDir);
	@session_start();
	// check session id
	if (! isset($_SESSION["sec_session_id"]) || ($_SESSION["sec_session_id"] != session_id())) {
		// session id is invalid
		die();		
	}
	// check if client IP has not changed
	if (!isset($_SESSION["sec_client_ip"]) || ($_SESSION["sec_client_ip"] != $_SERVER['REMOTE_ADDR'])) {
		// IP is invalid
		die();
	}
	// check if client IP is on the list of valid IPs
	checkClientIP();
	// check if session time has not expired
	// TODO
}

/**
 * Checks if the client's IP address is on the list of allowed IPs.
 * The script is stopped if the host is not valid.
 *
 */
function checkClientIP() {
	
}

/**
 * Checks if the user is allowed to access LAM at this time.
 * The script is stopped if time is exceeded.
 *
 * @param unknown_type $dn
 */
function checkUserTime($dn) {
	
}

/**
 * Returns a list of DNs of valid LAM users.
 *
 * @param string $dn configuration DN
 * @return  array $dn user list
 */
function getValidUserDNs($dn) {
	return array("uid=test,o=test", "uid=test2,o=test");
}

?>