<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Michael Duergner

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
* Login form of LDAP Account Manager.
*
* @author Michael Duergner
* @package main
*/

/** status messages */
include_once("../lib/status.inc");
/** security functions */
include_once("../lib/security.inc");
/** self service functions */
include_once("../lib/selfService.inc");

// check environment
$criticalErrors = array();
// check if PHP has session support
if (! function_exists('session_start')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no session support!", "Please install the session extension for PHP.");
}
// check if PHP has LDAP support
if (! function_exists('ldap_search')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no LDAP support!", "Please install the LDAP extension for PHP.");
}
// check if PHP has gettext support
if (! function_exists('gettext') || !function_exists('_')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no gettext support!", "Please install gettext for PHP.");
}
// check if PHP has XML support
if (! function_exists('utf8_decode')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no XML support!", "Please install the XML extension for PHP.");
}
// check if PHP >= 5.1
if (version_compare(phpversion(), '5.1.0') < 0) {
	$criticalErrors[] = array("ERROR", "LAM needs PHP 5 greater or equal as 5.1.0!", "Please upgrade your PHP installation.");
}
// check file permissions
$writableDirs = array('sess', 'tmp');
for ($i = 0; $i < sizeof($writableDirs); $i++) {
	$path = realpath('../') . "/" . $writableDirs[$i];
	if (!is_writable($path)) {
		$criticalErrors[] = array("ERROR", 'The directory %s is not writable for the web server. Please change your file permissions.', '', array($path));
	}
}
// check session auto start
if (ini_get("session.auto_start") == "1") {
	$criticalErrors[] = array("ERROR", "Please deactivate session.auto_start in your php.ini. LAM will not work if it is activated.");
}
$memLimit = ini_get('memory_limit');
if (isset($memLimit) && ($memLimit != '') && (substr(strtoupper($memLimit), strlen($memLimit) - 1) == 'M')) {
	if (intval(substr($memLimit, 0, strlen($memLimit) - 1)) < 64) {
		$criticalErrors[] = array("ERROR", "Please increase the \"memory_limit\" parameter in your php.ini to at least \"64M\".",
			"Your current memory limit is $memLimit.");	
	}
}
// stop login if critical errors occured
if (sizeof($criticalErrors) > 0) {
	echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";
	echo "<html>\n<head>\n";
	echo "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";
	echo "<meta http-equiv=\"pragma\" content=\"no-cache\">\n		<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/layout.css\">";
	echo "<title>LDAP Account Manager</title>\n";
	echo "</head><body>\n";
	for ($i = 0; $i < sizeof($criticalErrors); $i++) {
		call_user_func_array("StatusMessage", $criticalErrors[$i]);
		echo "<br><br>";
	}
	echo "</body></html>";
	exit();
}


/** access to configuration options */
include_once("../lib/config.inc"); // Include config.inc which provides Config class

session_save_path("../sess"); // Set session save path
session_start(); // Start LDAP Account Manager session

/**
* Displays the login window.
*
* @param object $config_object current active configuration
*/
function display_LoginPage($config_object) {
	global $error_message;
	// generate 256 bit key and initialization vector for user/passwd-encryption
	// check if we can use /dev/random otherwise use /dev/urandom or rand()
	if(function_exists('mcrypt_create_iv')) {
		$key = @mcrypt_create_iv(32, MCRYPT_DEV_URANDOM);
		if (! $key) {
			srand((double)microtime()*1234567);
			$key = mcrypt_create_iv(32, MCRYPT_RAND);
		}
		$iv = @mcrypt_create_iv(32, MCRYPT_DEV_URANDOM);
		if (! $iv) {
			srand((double)microtime()*1234567);
			$iv = mcrypt_create_iv(32, MCRYPT_RAND);
		}
		// save both in cookie
		setcookie("Key", base64_encode($key), 0, "/");
		setcookie("IV", base64_encode($iv), 0, "/");
	}

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

	$profiles = getConfigProfiles();

	setlanguage(); // setting correct language

	echo $_SESSION["header"];
	?>
		<title>LDAP Account Manager -Login-</title>
		<link rel="stylesheet" type="text/css" href="../style/layout.css">
	</head>
	<body>
	<?php
		// set focus on password field
		echo "<script type=\"text/javascript\" language=\"javascript\">\n";
		echo "<!--\n";
		echo "window.onload = function() {\n";
			echo "loginField = document.getElementsByName('passwd')[0];\n";
			echo "loginField.focus();\n";
		echo "}\n";
		echo "//-->\n";
		echo "</script>\n";
	?>
		<p align="center">
			<a href="http://lam.sourceforge.net" target="_blank"><img src="../graphics/banner.jpg" border="1" alt="LDAP Account Manager"></a>
		</p>
		<table width="100%" border="0">
			<tr>
				<TD width="50%" align="left">
					<?PHP
						if (!isLAMProVersion()) {
							echo "<a href=\"http://lam.sourceforge.net/lamPro/index.htm\">" . _("Want more features? Get LAM Pro!") . "</a>";
						}
					?>
				</TD>
				<td width="50%" align="right">
					<a href="./config/index.php"><IMG alt="configuration" src="../graphics/tools.png">&nbsp;<?php echo _("LAM configuration") ?></a>
				</td>
			</tr>
		</table>
		<hr><br><br>
		<?php
		// check extensions
		$extList = getRequiredExtensions();
		for ($i = 0; $i < sizeof($extList); $i++) {
			if (!extension_loaded($extList[$i])) {
				StatusMessage("ERROR", "A required PHP extension is missing!", $extList[$i]);
				echo "<br>";
			}
		}
		// check if session expired
		if (isset($_GET['expired'])) {
			StatusMessage("ERROR", _("Your session expired, please log in again."));
			echo "<br>";
		}
		?>
		<table width="650" align="center" border="2" rules="none" bgcolor="white">
			<tr>
				<td style="border-style:none" width="70" rowspan="2">
					<img src="../graphics/lam.png" alt="Logo">
				</td>
				<td style="border-style:none" width="580">
					<form action="login.php" method="post">
						<table width="580">
							<tr>
								<td style="border-style:none" height="70" colspan="2" align="center">
									<font color="darkblue"><b><big><?php echo _("Please select your user name and enter your password to log in."); ?></big></b></font>
								</td>
							</tr>
							<tr>
								<td style="border-style:none" height="35" align="right"><b>
									<?php
									echo _("User name") . ":";
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
								<td style="border-style:none" align="right"><b>
									<?php
									echo _("Language") . ":";
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
							</tr>
							<tr>
								<td style="border-style:none" height="50" colspan="2" align="center">
									<input name="checklogin" type="hidden" value="checklogin">
									<input name="submit" type="submit" value="<?php echo _("Login"); ?>" tabindex="3">
								</td>
							</tr>
							<tr>
								<td style="border-style:none" colspan="2" align="center">
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
				<td style="border-style:none">
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
								echo _("Server profile") . ": ";
								if(empty($_POST['profileChange'])) {
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
								<input name="submit" type="submit" value="<?php echo _("Change profile"); ?>" tabindex="5">
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
			<TABLE style="position:absolute; bottom:10px;" border="0" width="99%">
				<TR><TD align="right"><HR>
					<SMALL>
					<?php
						if (isLAMProVersion()) {
							echo "LDAP Account Manager <b>Pro</b>: <b>" . LAMVersion() . "</b>&nbsp;&nbsp;&nbsp;";
						}
						else {
							echo "LDAP Account Manager: <b>" . LAMVersion() . "</b>&nbsp;&nbsp;&nbsp;";
						}
					?>
					</SMALL>
				</TD></TR>
			</TABLE>
	</body>
</html>
<?php
}

// checking if the submitted username/password is correct.
if(!empty($_POST['checklogin']))
{
	$_SESSION['lampath'] = realpath('../') . "/";  // Save full path to lam in session

	include_once("../lib/ldap.inc"); // Include ldap.php which provides Ldap class

	$_SESSION['ldap'] = new Ldap($_SESSION['config']); // Create new Ldap object

	if($_POST['passwd'] == "")
	{
		$error_message = _("Empty password submitted. Please try again.");
		display_LoginPage($_SESSION['config']); // Empty password submitted. Return to login page.
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
			// set security settings for session
			$_SESSION['sec_session_id'] = session_id();
			$_SESSION['sec_client_ip'] = $_SERVER['REMOTE_ADDR'];
			$_SESSION['sec_sessionTime'] = time();
			// logging
			logNewMessage(LOG_NOTICE, 'User ' . $_POST['username'] . ' successfully logged in.');
			// Load main frame
			metaRefresh("./main.php");
			die();
		}
		else
		{
			if ($result === False) {
				$error_message = _("Cannot connect to specified LDAP server. Please try again.");
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
				display_LoginPage($_SESSION['config']); // connection failed
			}
			elseif ($result == 81) {
				$error_message = _("Cannot connect to specified LDAP server. Please try again.");
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
				display_LoginPage($_SESSION['config']); // connection failed
			}
			elseif ($result == 49) {
				$error_message = _("Wrong password/user name combination. Please try again.");
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' failed to log in (wrong password).');
				display_LoginPage($_SESSION['config']); // Username/password invalid. Return to login page.
			}
			else {
				$error_message = _("LDAP error, server says:") .  "\n<br>($result) " . ldap_err2str($result);
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
				display_LoginPage($_SESSION['config']); // other errors
			}
		}
	}
}
// Reload loginpage after a profile change
elseif(!empty($_POST['profileChange'])) {
	$_SESSION['config'] = new LAMConfig($_POST['profile']); // Recreate the config object with the submited
	display_LoginPage($_SESSION['config']); // Load login page
}
// Load login page
else
{
	$_SESSION['loggedIn'] = false;
	$default_Config = new LAMCfgMain();
	$default_Profile = $default_Config->default;
	$_SESSION["config"] = new LAMConfig($default_Profile); // Create new Config object
	$_SESSION["cfgMain"] = $default_Config; // Create new CfgMain object

	display_LoginPage($_SESSION["config"]); // Load Login page
}
?>
