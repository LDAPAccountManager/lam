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
?>

<?
// including ldap.php which provides basic ldap functions
include_once("../lib/ldap.php");

// checking if the submitted username/password is correct.
if($action == "checklogin")
{
	$config = new Config;
	$ldap = new Ldap($config);
	$result = $ldap->connect($username,$password);
	if($result == True)
	{
		include("./main.php"); // Username/password correct. Loading main Frame.
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
include("./login.inc");
}
?>
