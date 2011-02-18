<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2005 - 2011  Roland Gruber

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
* @author Roland Gruber
* @package main
*/

/** status messages */
include_once("../lib/status.inc");

/** check environment */
include '../lib/checkEnvironment.inc';

/** security functions */
include_once("../lib/security.inc");
/** self service functions */
include_once("../lib/selfService.inc");
/** access to configuration options */
include_once("../lib/config.inc"); // Include config.inc which provides Config class

// set session save path
if (strtolower(session_module_name()) == 'files') {
	session_save_path(dirname(__FILE__) . '/../sess');
}

// start empty session and change ID for security reasons
session_start();
session_destroy();
session_start();
session_regenerate_id(true);

// save last selected login profile
if(isset($_GET['useProfile'])) {
	if (in_array($_GET['useProfile'], getConfigProfiles())) {
		setcookie("lam_default_profile", $_GET['useProfile'], time() + 365*60*60*24);
	}
	else {
		unset($_GET['useProfile']);
	}
}

// init some session variables
$default_Config = new LAMCfgMain();
$_SESSION["cfgMain"] = $default_Config;
$default_Profile = $default_Config->default;
if(isset($_COOKIE["lam_default_profile"]) && in_array($_COOKIE["lam_default_profile"], getConfigProfiles())) {
	$default_Profile = $_COOKIE["lam_default_profile"];
}
// Reload loginpage after a profile change
if(isset($_GET['useProfile'])) {
	logNewMessage(LOG_DEBUG, "Change server profile to " . $_GET['useProfile']);
	$_SESSION['config'] = new LAMConfig($_GET['useProfile']); // Recreate the config object with the submited
}
// Load login page
else {
	$_SESSION["config"] = new LAMConfig($default_Profile); // Create new Config object
}

$_SESSION['language'] = $_SESSION["config"]->get_defaultLanguage();
if (isset($_POST['language'])) {
	$_SESSION['language'] = $_POST['language']; // Write selected language in session
}
$current_language = explode(":",$_SESSION['language']);
$_SESSION['header'] = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";
$_SESSION['header'] .= "<html>\n<head>\n";
$_SESSION['header'] .= "<meta http-equiv=\"content-type\" content=\"text/html; charset=" . $current_language[1] . "\">\n";
$_SESSION['header'] .= "<meta http-equiv=\"pragma\" content=\"no-cache\">\n		<meta http-equiv=\"cache-control\" content=\"no-cache\">";


/**
* Displays the login window.
*
* @param object $config_object current active configuration
*/
function display_LoginPage($config_object) {
	logNewMessage(LOG_DEBUG, "Display login page");
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
			if(trim($line) == trim($_SESSION["language"]))
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
		<title>LDAP Account Manager</title>
	<?php 
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../style';
		$cssDir = dir($cssDirName);
		while ($cssEntry = $cssDir->read()) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) != '.css') continue;
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/" . $cssEntry . "\">\n";
		}
	?>
		<link rel="shortcut icon" type="image/x-icon" href="../graphics/favicon.ico">
	</head>
	<body onload="focusLogin()">
	<?php
	// include all JavaScript files
	$jsDirName = dirname(__FILE__) . '/lib';
	$jsDir = dir($jsDirName);
	$jsFiles = array();
	while ($jsEntry = $jsDir->read()) {
		if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
		$jsFiles[] = $jsEntry;
	}
	sort($jsFiles);
	foreach ($jsFiles as $jsEntry) {
		echo "<script type=\"text/javascript\" src=\"lib/" . $jsEntry . "\"></script>\n";
	}
	
	// set focus on password field
		echo "<script type=\"text/javascript\" language=\"javascript\">\n";
		echo "<!--\n";
		echo "function focusLogin() {\n";
		if ($config_object->getLoginMethod() == LAMConfig::LOGIN_LIST) {
			echo "myElement = document.getElementsByName('passwd')[0];\n";
			echo "myElement.focus();\n";
		}
		else {
			echo "myElement = document.getElementsByName('username')[0];\n";
			echo "myElement.focus();\n";
		}
		echo "}\n";
		?>
			jQuery(document).ready(function() {
				jQuery('#loginButton').button();
			});
		<?php
		echo "//-->\n";
		echo "</script>\n";
	?>

		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
				</td>
			<td align="right" height=20>
				<a href="./config/index.php"><IMG alt="configuration" src="../graphics/tools.png">&nbsp;<?php echo _("LAM configuration") ?></a>
			</td>
			</tr>
		</table>
		
		<br><br><br><br>

		<?php
		// check extensions
		$extList = getRequiredExtensions();
		for ($i = 0; $i < sizeof($extList); $i++) {
			if (!extension_loaded($extList[$i])) {
				StatusMessage("ERROR", "A required PHP extension is missing!", $extList[$i]);
				echo "<br>";
			}
		}
		// check TLS
		$useTLS = $config_object->getUseTLS();
		if (isset($useTLS) && ($useTLS == "yes")) {
			if (!function_exists('ldap_start_tls')) {
				StatusMessage("ERROR", "Your PHP installation does not support TLS encryption!");
				echo "<br>";
			}
		}
		// check if session expired
		if (isset($_GET['expired'])) {
			StatusMessage("ERROR", _("Your session expired, please log in again."));
			echo "<br>";
		}
		// check if main config was saved
		if (isset($_GET['confMainSavedOk'])) {
			StatusMessage("INFO", _("Your settings were successfully saved."));
			echo "<br>";
		}
		// check if a server profile was saved
		if (isset($_GET['configSaveOk'])) {
			StatusMessage("INFO", _("Your settings were successfully saved."), htmlspecialchars($_GET['configSaveFile']));
			echo "<br>";
		}
		elseif (isset($_GET['configSaveFailed'])) {
			StatusMessage("ERROR", _("Cannot open config file!"), htmlspecialchars($_GET['configSaveFile']));
			echo "<br>";
		}
		// check if self service was saved
		if (isset($_GET['selfserviceSaveOk'])) {
			StatusMessage("INFO", _("Your settings were successfully saved."), $_GET['selfserviceSaveOk']);
			echo "<br>";
		}
		?>
		<div style="position:relative; z-index:5;">
		<table width="650" align="center" border="2" rules="none" bgcolor="white">
			<tr>
				<td style="border-style:none" width="70" rowspan="2">
					<img src="../graphics/lam.png" alt="Logo">
				</td>
				<td style="border-style:none" width="580">
					<form action="login.php" method="post">
						<table width="580">
							<tr>
								<td style="border-style:none" height="30" colspan="2" align="center">
								</td>
							</tr>
							<tr>
								<td style="border-style:none" height="35" align="right"><b>
									<?php
									echo _("User name") . ":";
									?>
								</b>&nbsp;&nbsp;</td>
								<td style="border-style:none" height="35" align="left">
									<?php
									if ($config_object->getLoginMethod() == LAMConfig::LOGIN_LIST) {
										echo '<select name="username" size="1" tabindex="1">';
										$admins = $config_object->get_Admins();
										for($i = 0; $i < count($admins); $i++) {
											$text = explode(",", $admins[$i]);
											$text = explode("=", $text[0]);
											echo '<option value="' . $admins[$i] . '">' . $text[1] . '</option>';
										}
										echo '</select>';
									}
									else {
										echo '<input type="text" name="username" tabindex="1">';
									}
									?>
								</td>
							</tr>
							<tr>
								<td style="border-style:none" height="35" align="right"><b>
									<?php
									echo _("Password") . ":";
									?>
								</b>&nbsp;&nbsp;</td>
								<td style="border-style:none" height="35" align="left">
									<input type="password" name="passwd" tabindex="2">
								</td>
							</tr>
							<tr>
								<td style="border-style:none" align="right"><b>
									<?php
									echo _("Language") . ":";
									?>
								</b>&nbsp;&nbsp;</td>
								<td style="border-style:none" height="35" align="left">
									<select name="language" size="1" tabindex="3">
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
									<button id="loginButton" class="smallPadding" name="submit" tabindex="4"><?php echo _("Login"); ?></button>
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
								<td height="30" colspan=2>
								<hr>
								</td>
							</tr>
							<tr>
								<td height="30" style="white-space: nowrap">
									<b>
									<?php
									echo _("LDAP server") . ": ";
									?></b>
								</td>
								<td width="100%" height="30">
									<?php echo $config_object->get_ServerURL(); ?>
								</td>
							</tr>
							<tr>
							<td height="30" style="white-space: nowrap">
								<b>
								<?php
								echo _("Server profile") . ": ";
								?></b>
							</td>
							<td height="30">
								<select name="profile" size="1" tabindex="5" onchange="loginProfileChanged(this)">
								<?php
								for($i=0;$i<count($profiles);$i++) {
									$selected = '';
									if ($profiles[$i] == $_SESSION['config']->getName()) {
										$selected = ' selected';
									}
									echo '<option value="' . $profiles[$i] . '"' . $selected . '>' . $profiles[$i] . '</option>';
								}
								?>
								</select>
							</td>
							</tr>
							<tr>
								<td height="10" colspan="2"></td>
							</tr>
						</table>
					</form>
				</td>
			</tr>
		</table>
		</div>
		<br><br>
			<TABLE style="position:absolute; bottom:10px;" border="0" width="99%">
				<tr><td colspan=2><HR></td></tr>
				<TR>
				<td align="left">
					<?PHP
						if (!isLAMProVersion()) {
							echo "<a href=\"http://www.ldap-account-manager.org/lamcms/lamPro\">" . _("Want more features? Get LAM Pro!") . "</a>";
						}
					?>
				</td>
				<TD align="right">
					<SMALL>
					<?php
						if (isLAMProVersion()) {
							echo "LDAP Account Manager Pro - " . LAMVersion() . "&nbsp;&nbsp;&nbsp;";
							logNewMessage(LOG_DEBUG, "LAM Pro " . LAMVersion());
						}
						else {
							echo "LDAP Account Manager - " . LAMVersion() . "&nbsp;&nbsp;&nbsp;";
							logNewMessage(LOG_DEBUG, "LAM " . LAMVersion());
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
if(!empty($_POST['checklogin'])) {
	include_once("../lib/ldap.inc"); // Include ldap.php which provides Ldap class

	$_SESSION['ldap'] = new Ldap($_SESSION['config']); // Create new Ldap object

	if($_POST['passwd'] == "") {
		logNewMessage(LOG_DEBUG, "Empty password for login");
		$error_message = _("Empty password submitted. Please try again.");
		display_LoginPage($_SESSION['config']); // Empty password submitted. Return to login page.
		exit();
	}
	else {
		$clientSource = $_SERVER['REMOTE_ADDR'];
		if (isset($_SERVER['REMOTE_HOST'])) {
			$clientSource .= '/' . $_SERVER['REMOTE_HOST'];
		}
		if (get_magic_quotes_gpc() == 1) {
			$_POST['passwd'] = stripslashes($_POST['passwd']);
		}
		$username = $_POST['username'];
		// search user in LDAP if needed
		if ($_SESSION['config']->getLoginMethod() == LAMConfig::LOGIN_SEARCH) {
			$searchFilter = $_SESSION['config']->getLoginSearchFilter();
			$searchFilter = str_replace('%USER%', $username ,$searchFilter);
			$searchSuccess = true;
			$searchError = '';
			$searchLDAP = new Ldap($_SESSION['config']);
			$searchLDAPResult = $searchLDAP->connect('', '', true);
			if (! ($searchLDAPResult == 0)) {
				$searchSuccess = false;
				$searchError = _('Cannot connect to specified LDAP server. Please try again.') . ' ' . @ldap_error($searchLDAP->server());
			}
			else {
				$searchResult = @ldap_search($searchLDAP->server(), $_SESSION['config']->getLoginSearchSuffix(), $searchFilter, array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
				if ($searchResult) {
					$searchInfo = @ldap_get_entries($searchLDAP->server(), $searchResult);
					if ($searchInfo) {
						$searchInfo = cleanLDAPResult($searchInfo);
						if (sizeof($searchInfo) == 0) {
							$searchSuccess = false;
							$searchError = _('Wrong password/user name combination. Please try again.');
						}
						elseif (sizeof($searchInfo) > 1) {
							$searchSuccess = false;
							$searchError = _('The given user name matches multiple LDAP entries.');
						}
						else {
							$username = $searchInfo[0]['dn'];
						}
					}
					else {
						$searchSuccess = false;
						$searchError = _('Unable to find the user name in LDAP.');
						if (ldap_errno($searchLDAP->server()) != 0) $searchError .= ' ' . ldap_error($searchLDAP->server());
					}
				}
				else {
					$searchSuccess = false;
					$searchError = _('Unable to find the user name in LDAP.');
					if (ldap_errno($searchLDAP->server()) != 0) $searchError .= ' ' . ldap_error($searchLDAP->server());
				}
			}
			if (!$searchSuccess) {
				$error_message = $searchError;
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' (' . $clientSource . ') failed to log in. ' . $searchError . '');
				$searchLDAP->close();
				display_LoginPage($_SESSION['config']);
				exit();
			}
			$searchLDAP->close();
		}
		// try to connect to LDAP
		$result = $_SESSION['ldap']->connect($username,$_POST['passwd']); // Connect to LDAP server for verifing username/password
		if($result === 0) {// Username/password correct. Do some configuration and load main frame.
			$_SESSION['loggedIn'] = true;
			// set security settings for session
			$_SESSION['sec_session_id'] = session_id();
			$_SESSION['sec_client_ip'] = $_SERVER['REMOTE_ADDR'];
			$_SESSION['sec_sessionTime'] = time();
			// logging
			logNewMessage(LOG_NOTICE, 'User ' . $_POST['username'] . ' (' . $clientSource . ') successfully logged in.');
			// Load main frame
			metaRefresh("./main.php");
			die();
		}
		else {
			if ($result === False) {
				// connection failed
				$error_message = _("Cannot connect to specified LDAP server. Please try again.");
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' (' . $clientSource . ') failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
			}
			elseif ($result == 81) {
				// connection failed
				$error_message = _("Cannot connect to specified LDAP server. Please try again.");
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' (' . $clientSource . ') failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
			}
			elseif ($result == 49) {
				// user name/password invalid. Return to login page.
				$error_message = _("Wrong password/user name combination. Please try again.");
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' (' . $clientSource . ') failed to log in (wrong password).');
			}
			else {
				// other errors
				$error_message = _("LDAP error, server says:") .  "\n<br>($result) " . ldap_err2str($result);
				logNewMessage(LOG_ERR, 'User ' . $_POST['username'] . ' (' . $clientSource . ') failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
			}
			display_LoginPage($_SESSION['config']);
			exit();
		}
	}
}

display_LoginPage($_SESSION["config"]);

?>
