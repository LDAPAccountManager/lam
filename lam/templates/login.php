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


  LDAP Account Manager checking login datas.
*/

include_once("../lib/config.inc"); // Include config.inc which provides Config class

session_save_path("../sess"); // Set session save path
@session_start(); // Start LDAP Account Manager session

function display_LoginPage($config_object)
{
	global $error_message;
	// generate 256 bit key and initialization vector for user/passwd-encryption
	$key = mcrypt_create_iv(32, MCRYPT_DEV_RANDOM);
	$iv = mcrypt_create_iv(32, MCRYPT_DEV_RANDOM);

	// save both in cookie
	setcookie("Key", base64_encode($key), 0, "/");
	setcookie("IV", base64_encode($iv), 0, "/");

	session_register("language");
	$_SESSION["language"] = $config_object->get_defaultLanguage();

	// loading available languages from language.conf file
	$languagefile = "../config/language.conf";
	if(is_file($languagefile) == True)
	{
		$file = fopen($languagefile, "r");
		$i = 0;
		while(!feof($file))
		{
			$line = fgets($file, 1024);
			if($line == "" || $line == "\n" || $line[0] == "#") continue; // ignore comment and empty lines
			$value = explode(":", $line);
			$languages[$i]["link"] = $value[0] . ":" . $value[1];
			$languages[$i]["descr"] = $value[2];
			if(rtrim($line) == $_SESSION["language"])
			{
				$languages[$i]["default"] = "YES";
			}
			else
			{
				$languages[$i]["default"] = "NO";
			}
			$i++;
		}
		fclose($file);
	}
	else
	{
		$message = _("Unable to load available languages. Setting English as default language. For further instructions please contact the Admin of this site.");
	}

	setlanguage(); // setting correct language

	echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">

		<html>
			<head>
				<title>
					";
	echo "LDAP Account Manager -Login-";
	echo "
				</title>
				<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">
			</head>
			<body>
				<p align=\"center\"><a href=\"http://lam.sf.net\" target=\"_blank\"><img src=\"../graphics/banner.jpg\" border=\"1\"></a></p>
			<table width=\"100%\" border=\"0\">
				<tr>
					<td width=\"100%\" align=\"right\">
						<a href=\"./config/conflogin.php\" target=\"_self\">";
						 echo _("Configuration Login");
	echo "
						</a>
					</td>
				</tr>
			</table>
			<hr><br><br>
			<p align=\"center\"><b>";
			echo _("Enter Username and Password for Account:");
	echo "
			</b></p>";
			if($error_message != "")
				{
					echo "<p align=\"center\">";
					echo $error_message;
					echo "</p>";
				}
	echo "
				<form action=\"login.php\" method=\"post\">
					<input type=\"hidden\" name=\"action\" value=\"checklogin\">
					<table width=\"500\" align=\"center\" border=\"0\">
						<tr>
							<td width=\"45%\" align=\"right\">";
								echo _("Username:");
	echo "
							</td>
							<td width=\"10%\">
							</td>
							<td width=\"45%\" align=\"left\">
								<select name=\"username\" size=\"1\">";
								for($i = 0; $i < count($config_object->Admins); $i++)
								{
									$text = explode(",", $config_object->Admins[$i]);
									$text = explode("=", $text[0]);
									echo "<option value=\"" . $config_object->Admins[$i] . "\">" . $text[1] . "</option>";
								}
	echo "
								</select>
							</td>
						</tr>
						<tr>
							<td width=\"45%\" align=\"right\">";
								echo _("Password:");
	echo "
							</td>
							<td width=\"10%\">
							</td>
							<td width=\"45%\" align=\"left\">
								<input type=\"password\" name=\"passwd\">
							</td>
						</tr>
						<tr>";
							if($message != "")
							{
	echo "					<td width=\"100%\" colspan=\"3\" align=\"center\">";
								echo $message;
	echo "						<input type=\"hidden\" name=\"language\" value=\"english\">
							</td>";
							}
							else
							{
	echo "					<td width=\"45%\" align=\"right\">";
								echo _("Your Language:");
	echo "
							</td>
							<td width=\"10%\">
							</td>
							<td width=\"45%\" align=\"left\">
								<select name=\"language\" size=\"1\">";
								for($i = 0; $i < count($languages); $i++)
								{
									if($languages[$i]["default"] == "YES")
									{
										echo "<option selected value=\"" . $languages[$i]["link"] . ":" . $languages[$i]["descr"] . "\">" . $languages[$i]["descr"] . "</option>";
									}
									else
									{
										echo "<option value=\"" . $languages[$i]["link"] . ":" . $languages[$i]["descr"] . "\">" . $languages[$i]["descr"] . "</option>";
									}
								}
	echo "						</select>
							</td>";
							}
	echo "
						</tr>
						<tr>
							<td width=\"100%\" colspan=\"3\" align=\"center\">
								<input type=\"submit\" name=\"submit\" value=\"";
								echo _("Login") . "\">";
	echo "
							</td>
						</tr>
					</table>
					<br><br><br>
					<table width=\"345\" align=\"center\" bgcolor=\"#C7E7C7\" border=\"0\">
						<tr>
							<td width=\"100%\" align=\"center\">";
								echo _("You are connecting to ServerURL: ");
	echo "						<b>";
								echo $config_object->get_ServerURL();
	echo "
								</b></td>
						</tr>
					</table>
				</form>
			</body>
		</html>";
}

// checking if the submitted username/password is correct.
if($_POST['action'] == "checklogin")
{
	include_once("../lib/ldap.inc"); // Include ldap.php which provides Ldap class

	$ldap = new Ldap($_SESSION['config']); //$config); // Create new Ldap object
	if($_POST['passwd'] == "")
	{
		$error_message = _("Empty Password submitted. Try again.");
		display_LoginPage($_SESSION['config']); // Empty password submitted. Return to login page.
	}
	else
	{
		$result = $ldap->connect($_POST['username'],$_POST['passwd']); // Connect to LDAP server for verifing username/password
		if($result == True) // Username/password correct. Do some configuration and load main frame.
		{
			$_SESSION["language"] = $_POST["language"]; // Write selected language in session
			session_register("ldap"); // Register $ldap object in session

			include("./main.php"); // Load main frame
		}
		else
		{
			if($ldap->server)
			{
				$error_message = _("Wrong Password/Username  combination. Try again.");
				display_LoginPage($_SESSION['config']); // Username/password invalid. Return to login page.
			}
			else
			{
				$error_message = _("Cannot connect to specified LDAP-Server. Try again.");
				display_LoginPage($_SESSION['config']); // Username/password invalid. Return to login page.
			}
		}
	}
}
// Load login page
else
{
	session_register("config"); // Register $config object in session

	$config = new Config; // Create new Config object

	display_LoginPage($config); // Load Login page
}
?>
