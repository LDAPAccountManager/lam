<?
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


  LDAP Account Manager checking login data.
*/

include_once("../config/config.php"); // Include config.php which provides Config class

// Get session save path
$path = getcwd();
$path = explode("/", substr($path,1));
for($i = 0; $i < (count($path) - 1); $i++)
{
	$session_save_path .= "/" . $path[$i];
}
$session_save_path .= "/sess";

session_save_path($session_save_path); // Set session save path
@session_start(); // Start LDAP Account Manager session

// checking if the submitted username/password is correct.
if($action == "checklogin")
{
	include_once("../lib/ldap.php"); // Include ldap.php which provides Ldap class

	$ldap = new Ldap($config); //$config); // Create new Ldap object
	$result = $ldap->connect($username,$passwd); // Connect to LDAP server for verifing username/password
	if($result == True) // Username/password correct. Do some configuration and load main frame.
	{
		// setting language
		$language = explode(":", $language);
		putenv("LANG=" . $language[1]);
		setlocale("LC_ALL", $language[0]);
		bindtextdomain("lam", "../locale");
		textdomain("lam");

		include("./main.php"); // Load main frame

		session_register("ldap"); // Register $ldap object in session
		session_register("language"); // Register $language in session
	}
	else
	{
		if($ldap->server)
		{
			$error_message = "Wrong Password/Username  combination. Try again.";
			include("./login.inc"); // Username/password invalid. Return to login page.
		}
		else
		{
			$error_message = "Cannot connect to specified LDAP-Server. Try again.";
			include("./login.inc"); // Server not reachable. Return to login page.
		}
	}
}
// Load login page
else
{
	session_register("config"); // Register $config object in session

	include_once("../lib/ldap.php"); // Includ ldap.php which provides Ldap class

	$config = new Config; // Create new Config object

	include("./login.inc"); // Load login page
}
?>
