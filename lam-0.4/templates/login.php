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
	// check if we can use /dev/random otherwise use /dev/urandom or rand()
	if(function_exists(mcrypt_create_iv)) {
		$key = @mcrypt_create_iv(32, MCRYPT_DEV_RANDOM);
		if (! $key) $key = @mcrypt_create_iv(32, MCRYPT_DEV_URANDOM);
		if (! $key) {
			srand((double)microtime()*1234567);
			$key = mcrypt_create_iv(32, MCRYPT_RAND);
		}
		$iv = @mcrypt_create_iv(32, MCRYPT_DEV_RANDOM);
		if (! $iv) $iv = @mcrypt_create_iv(32, MCRYPT_DEV_URANDOM);
		if (! $iv) {
			srand((double)microtime()*1234567);
			$iv = mcrypt_create_iv(32, MCRYPT_RAND);
		}
	}
	// use Blowfish if MCrypt is not available
	else {
		// generate iv and key for encryption
		$key = "";
		$iv = "";
		while (strlen($key) < 30) $key .= mt_rand();
		while (strlen($iv) < 30) $iv .= mt_rand();
	}

	// save both in cookie
	setcookie("Key", base64_encode($key), 0, "/");
	setcookie("IV", base64_encode($iv), 0, "/");

	$_SESSION['language'] = $config_object->get_defaultLanguage();

	$current_language = explode(":",$_SESSION['language']);
	$_SESSION['header'] = "<?xml version=\"1.0\" encoding=\"" . $current_language[1] . "\"?>\n";
	$_SESSION['header'] .= "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";
	$_SESSION['header'] .= "<html>\n<head>\n";
	$_SESSION['header'] .= "<meta http-equiv=\"content-type\" content=\"text/html; charset=" . $current_language[1] . "\">\n";
	$_SESSION['header'] .= "<meta http-equiv=\"pragma\" content=\"no-cache\">\n		<meta http-equiv=\"cache-control\" content=\"no-cache\">";

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
		//TODO Generate Status message
		$message = _("Unable to load available languages. Setting English as default language. For further instructions please contact the Admin of this site.");
	}

	$profiles = getConfigProfiles();

	setlanguage(); // setting correct language

	echo $_SESSION["header"];
	?>
		<title>LDAP Account Manager -Login-</title>
		<link rel="stylesheet" type="text/css" href="../style/layout.css">
	</head>
	<body>
		<p align="center">
			<a href="http://lam.sf.net" target="_blank"><img src="../graphics/banner.jpg" border="1" alt="LDAP Account Manager"></a>
		</p>
		<table width="100%" border="0">
			<tr>
				<td width="100%" align="right">
					<a href="./config/conflogin.php" target="_self"><?php echo _("Configuration Login"); ?></a>
				</td>
			</tr>
		</table>
		<hr><br><br>
		<?php
		// check if all password hashes are possible
		if ((! function_exists('mHash')) && (! function_exists('sha1'))) {
			StatusMessage("INFO", "Your PHP does not support MHash or sha1(), you will only be able to use CRYPT/PLAIN/MD5/SMD5 for user passwords!", "Please install MHash or update to PHP >4.3.");
			echo "<br><br>";
		}
		// check if PHP has LDAP support
		if (! function_exists('ldap_search')) {
			StatusMessage("ERROR", "Your PHP has no LDAP support!", "Please install the LDAP extension for PHP.");
			echo "<br><br>";
		}
		?>
		<table width="650" align="center" border="2" rules="none" bgcolor="white">
			<tr>
				<td style="border-style:none" width="70" rowspan="2">
					<img src="../graphics/lam.png" alt="Logo">
				</td>
				<td width="580">
					<form action="login.php" method="post">
						<table width="580">
							<tr>
								<td style="border-style:none" height="70" colspan="2" align="center">
									<font color="darkblue"><b><big><?php echo _("Enter Username and Password for Account"); ?></big></b></font>
								</td>
								<td style="border-style:none" rowspan="9" width="70">
									&nbsp;
								</td>
							</tr>
							<tr>
								<td style="border-style:none" height="35" align="right"><b>
									<?php
									echo _("Username") . ":";
									?>
								</b>&nbsp;&nbsp;</td>
								<td style="border-style:none" height="35" align="left">
									<select name="username" size="1" tabindex="0">
									<?php
									$admins = $config_object->get_Admins();
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
								<td style="border-style:none" height="35" align="right"><b>
									<?php
									echo _("Password") . ":";
									?>
								</b>&nbsp;&nbsp;</td>
								<td style="border-style:none" height="35" align="left">
									<input type="password" name="passwd" tabindex="1">
								</td>
							</tr>
							<tr>
							<?php
							if($message != "") {
								?>
								<td height="35" colspan="3" align="center">
								<?php
									echo $message;
								?>
								</td>
								<?php
							}
							else
							{
								?>
								<td style="border-style:none" align="right"><b>
									<?php
									echo _("Your Language") . ":";
									?>
								</b>&nbsp;&nbsp;</td>
								<td style="border-style:none" height="35" align="left">
									<select name="language" size="1" tabindex="2">
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
								<td style="border-style:none" height="50" colspan="2" align="center">
									<input name="checklogin" type="hidden" value="checklogin">
									<input name="submit" type="submit" value="<?php echo _("Login"); ?>" tabindex="3">
								</td>
							</tr>
							<tr>
								<td style="border-style:none" height="50" colspan="2" align="center">
									<?php
										if($error_message != "") {
											echo "<font color=\"red\"><b>" . $error_message . "</b></font>";
										}
									?>
								</td>
							</tr>
						</table>
					</form>
				</td>
			</tr>
			<tr>
				<td>
					<form action="login.php" method="post">
						<table width="580">
							<tr>
								<td style="border-style:none" height="30" colspan="2">
									<hr>
									<b>
									<?php
									echo _("LDAP server") . ": ";
									?></b>
									<?php echo $config_object->get_ServerURL(); ?>
								</td>
							</tr>
							<tr>
							<td style="border-style:none" height="30"><b>
								<?php
								echo _("Configuration profile") . ": ";
								if(!$_POST['profileChange']) {
									$_POST['profile'] = $_SESSION['config']->file;
								}
								?></b>
								<?php echo $_POST['profile']; ?>
							</td>
							<td style="border-style:none" height="30" align="right">
								<select name="profile" size="1" tabindex="4">
								<?php
								for($i=0;$i<count($profiles);$i++) {
									?>
									<option value="<?php echo $profiles[$i]; ?>"><?php echo $profiles[$i]; ?></option>
									<?php
								}
								?>
								</select>
								<input name="profileChange" type="hidden" value="profileChange">
								<input name="submit" type="submit" value="<?php echo _("Change Profile"); ?>" tabindex="5">
							</td>
							</tr>
							<tr>
								<td style="border-style:none" height="10" colspan="2"></td>
							</tr>
						</table>
					</form>
				</td>
			</tr>
		</table>
		<br><br>
	</body>
</html>
<?php
}

// checking if the submitted username/password is correct.
if($_POST['checklogin'])
{
	$_SESSION['lampath'] = realpath('../') . "/";  // Save full path to lam in session

	include_once("../lib/ldap.inc"); // Include ldap.php which provides Ldap class

	$_SESSION['ldap'] = new Ldap($_SESSION['config']); // Create new Ldap object

	if($_POST['passwd'] == "")
	{
		$error_message = _("Empty Password submitted. Try again.");
		display_LoginPage($_SESSION['config'],""); // Empty password submitted. Return to login page.
	}
	else
	{
		if (get_magic_quotes_gpc() == 1) {
			$_POST['passwd'] = stripslashes($_POST['passwd']);
		}
		$result = $_SESSION['ldap']->connect($_POST['username'],$_POST['passwd']); // Connect to LDAP server for verifing username/password

		if($result === 0) // Username/password correct. Do some configuration and load main frame.
		{
			$_SESSION['loggedIn'] = true;
			$_SESSION['language'] = $_POST['language']; // Write selected language in session
			$current_language = explode(":",$_SESSION['language']);
			$_SESSION['header'] = "<?xml version=\"1.0\" encoding=\"" . $current_language[1] . "\"?>\n";
			$_SESSION['header'] .= "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";
			$_SESSION['header'] .= "<html>\n<head>\n";
			$_SESSION['header'] .= "<meta http-equiv=\"content-type\" content=\"text/html; charset=" . $current_language[1] . "\">\n";
			$_SESSION['header'] .= "<meta http-equiv=\"pragma\" content=\"no-cache\">\n		<meta http-equiv=\"cache-control\" content=\"no-cache\">";

			include("./main.php"); // Load main frame
		}
		else
		{
			if ($result === False)
			{
				$error_message = _("Cannot connect to specified LDAP-Server. Please try again.");
				display_LoginPage($_SESSION['config'],""); // connection failed
			}
			elseif ($result == 81)
			{
				$error_message = _("Cannot connect to specified LDAP-Server. Please try again.");
				display_LoginPage($_SESSION['config'],""); // connection failed
			}
			elseif ($result == 49)
			{
				$error_message = _("Wrong Password/Username combination. Try again.");
				display_LoginPage($_SESSION['config'],""); // Username/password invalid. Return to login page.
			}
			else
			{
				$error_message = _("LDAP error, server says:") .  "\n<br>($result) " . ldap_err2str($result);
				display_LoginPage($_SESSION['config'],""); // other errors
			}
		}
	}
}
// Reload loginpage after a profile change
elseif($_POST['profileChange']) {
	$_SESSION['config'] = new Config($_POST['profile']); // Recreate the config object with the submited

	display_LoginPage($_SESSION['config'],""); // Load login page
}
// Load login page
else
{
	$_SESSION['loggedIn'] = false;
	$default_Config = new CfgMain();
	$default_Profile = $default_Config->default;
	$_SESSION["config"] = new Config($default_Profile); // Create new Config object

	display_LoginPage($_SESSION["config"],$default_Profile); // Load Login page
}
?>
