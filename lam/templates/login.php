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

	// Starting LDAP Account Manager session
	//session_name("LDAP Account Manager");
	@session_start();

// checking if the submitted username/password is correct.
if($action == "checklogin")
{
	// including ldap.php which provides basic ldap functions
	include_once("../lib/ldap.php");

	//$config = new Config; // Creating new Config object
	$ldap = new Ldap($config); // Creating new Ldap object
	//$result = $ldap->connect($username,$passwd);
	if($result == True) // Username/password correct. Doing some configuration and loading main Frame.
	{
		// setting language
		$language = explode(":", $language);
		putenv("LANG=" . $language[1]);
		setlocale("LC_ALL", $language[0]);
		bindtextdomain("lam", "../locale");
		textdomain("lam");

		include("./main.php");

		session_register("ldap"); // Register $ldap object in session
		session_register("language"); // Register $language in session
	}
	else
	{
		if($ldap->server)
		{
			$error_message = "Wrong Password/Username  combination. Try again.";
			include("./login.inc"); // Username/password invalid. Returning to Login page.
		}
		else
		{
			$error_message = "Cannot connect to specified LDAP-Server. Try again.";
			include("./login.inc"); // Server not reachable. Returning to Login page.
		}
	}
}
// Loading Login page
else
{
	session_register("config"); // Register $config object in session

	// including ldap.php which provides basic ldap functions
	include_once("../lib/ldap.php");

	$config = new Config; // Creating new Config object

	include("./login.inc");
}
?>
