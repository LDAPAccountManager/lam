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

function display_LoginPage($config_object,$profile)
{
	global $error_message;
	// generate 256 bit key and initialization vector for user/passwd-encryption
	srand((double)microtime()*1234567);
	$key = mcrypt_create_iv(32, MCRYPT_RAND);
	$iv = mcrypt_create_iv(32, MCRYPT_RAND);

	// save both in cookie
	setcookie("Key", base64_encode($key), 0, "/");
	setcookie("IV", base64_encode($iv), 0, "/");

	$_SESSION['language'] = $config_object->get_defaultLanguage();

	$current_language = explode(":",$_SESSION['language']);
	$_SESSION['header'] = "<?xml version=\"1.0\" encoding=\"" . $current_language[1] . "\"?>\n<!DOCTYPE>\n\n";

	// loading available languages from language.conf file
	$languagefile = "../config/language";
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

	$profiles = getConfigProfiles();

	setlanguage(); // setting correct language

	echo $_SESSION["header"];
	?>
<html>
	<head>
		<title>LDAP Account Manager -Login-</title>
		<link rel="stylesheet" type="text/css" href="../style/layout.css">
	</head>
	<body>
		<p align="center">
			<a href="http://lam.sf.net" target="_blank"><img src="../graphics/banner.jpg" border="1"></a>
		</p>
		<table width="100%" border="0">
			<tr>
				<td width="100%" align="right">
					<a href="./config/conflogin.php" target="_self"><?php echo _("Configuration Login"); ?></a>
				</td>
			</tr>
		</table>
		<hr><br><br>
		<p align="center">
			<b><?php echo _("Enter Username and Password for Account") . ":"; ?></b>
		</p>
		<?php
		if($error_message != "") {
		?>
		<p align="center">
			<?php
			echo $error_message;
			?>
		</p>
		<?php
		}
		?>
		<form action="login.php" method="post">
			<input type="hidden" name="action" value="checklogin">
			<table width="500" align="center" border="0">
				<tr>
					<td width="45%" align="right">
						<?php
						echo _("Username") . ":";
						?>
					</td>
					<td width="10%">
					</td>
					<td width="45%" align="left">
						<select name="username" size="1">
						<?php
						$admin = $config_object->get_Admins();
						for($i = 0; $i < count($admins); $i++) {
							$text = explode(",", $admins[$i]);
							$text = explode("=", $text[0]);
							?>
							<option value="<?php echo $admins[$i]; ?>"><?php echo $text[1]; ?></option>
							<?php
						}
						?>
						</select>
					</td>
				</tr>
				<tr>
					<td width="45%" align="right">
						<?php
						echo _("Password") . ":";
						?>
					</td>
					<td width="10%">
					</td>
					<td width="45%" align="left">
						<input type="password" name="passwd">
					</td>
				</tr>
				<tr>
				<?php
				if($message != "") {
					?>
					<td width="100%" colspan="3" align="center">
					<?php
						echo $message;
					?>
						<input type="hidden" name="language" value="english">
					</td>
					<?php
				}
				else
				{
					?>
					<td width="45%" align="right">
						<?php
						echo _("Your Language") . ":";
						?>
					</td>
					<td width="10%">
					</td>
					<td width="45%" align="left">
						<select name="language" size="1">
						<?php
						for($i = 0; $i < count($languages); $i++) {
							if($languages[$i]["default"] == "YES") {
							?>
							<option selected value="<?php echo $languages[$i]["link"] . ":" . $languages[$i]["descr"]; ?>"><?php echo $languages[$i]["descr"]; ?></option>
							<?php
							}
							else
							{
							?>
							<option value="<?php echo $languages[$i]["link"] . ":" . $languages[$i]["descr"]; ?>"><?php echo $languages[$i]["descr"]; ?></option>
							<?php
							}
						}
						?>
						</select>
					</td>
					<?php
				}
				?>
				</tr>
				<tr>
					<td width="100%" colspan="3" align="center">
						<input type="submit" name="submit" value="<?php echo _("Login"); ?>">
					</td>
				</tr>
			</table>
			<br><br>
			<table width="345" align="center" bgcolor="#C7E7C7" border="0">
				<tr>
					<td width="100%" align="center">
						<?php
						echo _("You are connecting to ServerURL") . ": ";
						?>
						<b><?php echo $config_object->get_ServerURL(); ?></b>
					</td>
				</tr>
			</table>
		</form>
		<br><br>
		<form action="./login.php" method="post" enctype="plain/text">
			<input type="hidden" name="action" value="profileChange">
			<p align="center">
				<?php
				echo _("You are currently using Profile") . ": ";
				if(!$_POST['profile']) {
					$_POST['profile'] = $profile;
				}
				?>
				<b><?php echo $_POST['profile']; ?></b>
				<br>
				<select name="profile" size="1">
				<?php
				for($i=0;$i<count($profiles);$i++) {
					?>
					<option value="<?php echo $profiles[$i]; ?>"><?php echo $profiles[$i]; ?></option>
					<?php
				}
				?>
				</select>
				<input type="submit" value="<?php echo _("Change Profile"); ?>">
			</p>
		</form>
	</body>
</html>
<?php
}

// checking if the submitted username/password is correct.
if($_POST['action'] == "checklogin")
{
	include_once("../lib/ldap.inc"); // Include ldap.php which provides Ldap class

	$_SESSION['ldap'] = new Ldap($_SESSION['config']); // Create new Ldap object
	if($_POST['passwd'] == "")
	{
		$error_message = _("Empty Password submitted. Try again.");
		display_LoginPage($_SESSION['config'],""); // Empty password submitted. Return to login page.
	}
	else
	{
		$result = $_SESSION['ldap']->connect($_POST['username'],$_POST['passwd']); // Connect to LDAP server for verifing username/password
		if($result == True) // Username/password correct. Do some configuration and load main frame.
		{
			$_SESSION['language'] = $_POST['language']; // Write selected language in session
			$current_language = explode(":",$_SESSION['language']);
			$_SESSION['header'] = "<?xml version=\"1.0\" encoding=\"" . $current_language[1] . "\"?>\n<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";


			include("./main.php"); // Load main frame
		}
		else
		{
			if($ldap->server)
			{
				$error_message = _("Wrong Password/Username combination. Try again.");
				display_LoginPage($_SESSION['config'],""); // Username/password invalid. Return to login page.
			}
			else
			{
				$error_message = _("Cannot connect to specified LDAP-Server. Try again.");
				display_LoginPage($_SESSION['config'],""); // Username/password invalid. Return to login page.
			}
		}
	}
}
// Reload loginpage after a profile change
elseif($_POST['action'] == "profileChange") {
	$_SESSION['config'] = new Config($_POST['profile']); // Recreate the config object with the submited

	display_LoginPage($_SESSION['config'],""); // Load login page
}
// Load login page
else
{

	$default_Config = new CfgMain();
	$default_Profile = $default_Config->default;
	$_SESSION["config"] = new Config($default_Profile); // Create new Config object

	display_LoginPage($_SESSION["config"],$default_Profile); // Load Login page
}
?>
