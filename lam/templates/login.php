<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2005 - 2013  Roland Gruber

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

/** Upgrade functions */
include_once("../lib/upgrade.inc");

// set session save path
if (strtolower(session_module_name()) == 'files') {
	session_save_path(dirname(__FILE__) . '/../sess');
}

// start empty session and change ID for security reasons
session_start();
session_destroy();
session_start();
session_regenerate_id(true);

$profiles = getConfigProfiles();

// save last selected login profile
if(isset($_GET['useProfile'])) {
	if (in_array($_GET['useProfile'], $profiles)) {
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
if(isset($_COOKIE["lam_default_profile"]) && in_array($_COOKIE["lam_default_profile"], $profiles)) {
	$default_Profile = $_COOKIE["lam_default_profile"];
}
// Reload loginpage after a profile change
if(isset($_GET['useProfile']) && in_array($_GET['useProfile'], $profiles)) {
	logNewMessage(LOG_DEBUG, "Change server profile to " . $_GET['useProfile']);
	$_SESSION['config'] = new LAMConfig($_GET['useProfile']); // Recreate the config object with the submited
}
// Load login page
elseif (!empty($default_Profile) && in_array($default_Profile, $profiles)) {
	$_SESSION["config"] = new LAMConfig($default_Profile); // Create new Config object
}
else if (sizeof($profiles) > 0) {
	// use first profile as fallback
	$_SESSION["config"] = new LAMConfig($profiles[0]);
}
else {
	$_SESSION["config"] = null;
}

if (!isset($default_Config->default) || !in_array($default_Config->default, $profiles)) {
	$error_message = _('No default profile set. Please set it in the server profile configuration.');
}

if (!empty($_SESSION["config"])) {
	$_SESSION['language'] = $_SESSION["config"]->get_defaultLanguage();
}
else {
	$_SESSION['language'] = 'en_GB.utf8:UTF-8:English (Great Britain)';
}
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
	// check if we can use /dev/urandom otherwise use rand()
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
		$cssFiles = array();
		$cssEntry = $cssDir->read();
		while ($cssEntry !== false) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) == '.css') {
				$cssFiles[] = $cssEntry;
			}
			$cssEntry = $cssDir->read();
		}
		sort($cssFiles);
		foreach ($cssFiles as $cssEntry) {
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
	
	// upgrade if pdf/profiles contain single files
	if (containsFiles('../config/profiles') || containsFiles('../config/pdf')) {
		$result = testPermissions();
		if (sizeof($result) > 0) {
		    StatusMessage('ERROR', 'Unable to migrate configuration files. Please allow write access to these paths:', implode('<br>', $result));
		}
		else {
			upgradeConfigToServerProfileFolders($profiles);
			StatusMessage('INFO', 'Config file migration finished.');
		}
	}
	// copy any missing default profiles
	copyConfigTemplates($profiles);
	
	// set focus on password field
	if (!empty($config_object)) {
		echo "<script type=\"text/javascript\" language=\"javascript\">\n";
		echo "<!--\n";
		echo "function focusLogin() {\n";
		if (($config_object->getLoginMethod() == LAMConfig::LOGIN_LIST) || isset($_COOKIE['lam_login_name'])) {
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
	}
	?>

		<script type="text/javascript">
			jQuery(document).ready(function() {
				var equalWidthElements = new Array('#username', '#passwd', '#language');
				equalWidth(equalWidthElements);
			});
		</script>

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
		
		<br><br>

		<?php
		if (!empty($config_object)) {
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
		}
		else {
			StatusMessage('WARN', _('Please enter the configuration and create a server profile.'));
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
			StatusMessage("INFO", _("Your settings were successfully saved."), htmlspecialchars($_GET['selfserviceSaveOk']));
			echo "<br>";
		}
		if (!empty($config_object)) {
		?>
		<br><br>
		<div style="position:relative; z-index:5;">
		<table width="650" align="center" border="2" rules="none" bgcolor="white" class="ui-corner-all">
			<tr>
				<td style="border-style:none" width="70" rowspan="2">
					<img src="../graphics/lam.png" alt="Logo" style="margin-top: 5px; margin-bottom: 5px;">
				</td>
				<td style="border-style:none" width="580px">
					<form action="login.php" method="post">
						<?php
							$table = new htmlTable('580px');
							$spacer = new htmlSpacer(null, '30px');
							$spacer->colspan = 3;
							$table->addElement($spacer, true);
							// user name
							$userLabel = new htmlOutputText(_("User name"));
							$userLabel->alignment = htmlElement::ALIGN_RIGHT;
							$table->addElement($userLabel);
							$gap = new htmlSpacer('5px', '30px');
							$table->addElement($gap);
							if ($config_object->getLoginMethod() == LAMConfig::LOGIN_LIST) {
								$admins = $config_object->get_Admins();
								$adminList = array();
								for($i = 0; $i < count($admins); $i++) {
									$text = explode(",", $admins[$i]);
									$text = explode("=", $text[0]);
									$adminList[$text[1]] = $admins[$i];
								}
								$userSelect = new htmlSelect('username', $adminList);
								$userSelect->setHasDescriptiveElements(true);
								$userSelect->alignment = htmlElement::ALIGN_LEFT;
								$table->addElement($userSelect);
							}
							else {
								if ($config_object->getHttpAuthentication() == 'true') {
									$httpAuth = new htmlOutputText($_SERVER['PHP_AUTH_USER']);
									$httpAuth->alignment = htmlElement::ALIGN_LEFT;
									$table->addElement($httpAuth);
								}
								else {
									$user = '';
									if (isset($_COOKIE["lam_login_name"])) {
										$user = $_COOKIE["lam_login_name"];
									}
									$userInput = new htmlInputField('username', $user);
									$userInput->alignment = htmlElement::ALIGN_LEFT;
									$table->addElement($userInput);
								}
							}
							$table->addNewLine();
							// password
							$passwordLabel = new htmlOutputText(_("Password"));
							$passwordLabel->alignment = htmlElement::ALIGN_RIGHT;
							$table->addElement($passwordLabel);
							$table->addElement($gap);
							if (($config_object->getLoginMethod() == LAMConfig::LOGIN_SEARCH) && ($config_object->getHttpAuthentication() == 'true')) {
								$passwordInputFake = new htmlOutputText('**********');
								$passwordInputFake->alignment = htmlElement::ALIGN_LEFT;
								$table->addElement($passwordInputFake);
							}
							else {
								$passwordInput = new htmlInputField('passwd');
								$passwordInput->alignment = htmlElement::ALIGN_LEFT;
								$passwordInput->setIsPassword(true);
								$passwordInput->setFieldSize('20px');
								$table->addElement($passwordInput);
							}
							$table->addNewLine();
							// language
							$languageLabel = new htmlOutputText(_("Language"));
							$languageLabel->alignment = htmlElement::ALIGN_RIGHT;
							$table->addElement($languageLabel);
							$table->addElement($gap);
							$languageList = array();
							$defaultLanguage = array();
							for($i = 0; $i < count($languages); $i++) {
								$languageList[$languages[$i]["descr"]] = $languages[$i]["link"] . ":" . $languages[$i]["descr"];
								if($languages[$i]["default"] == "YES") {
									$defaultLanguage[] = $languages[$i]["link"] . ":" . $languages[$i]["descr"];
								}
							}
							$languageSelect = new htmlSelect('language', $languageList, $defaultLanguage);
							$languageSelect->setHasDescriptiveElements(true);
							$languageSelect->alignment = htmlElement::ALIGN_LEFT;
							$table->addElement($languageSelect, true);
							// remember login user
							if (($config_object->getLoginMethod() == LAMConfig::LOGIN_SEARCH) && !($config_object->getHttpAuthentication() == 'true')) {
								$rememberLabel = new htmlOutputText('');
								$rememberLabel->alignment = htmlElement::ALIGN_RIGHT;
								$table->addElement($rememberLabel);
								$table->addElement($gap);
								$rememberGroup = new htmlGroup();
								$doRemember = false;
								if (isset($_COOKIE["lam_login_name"])) {
									$doRemember = true;
								}
								$rememberGroup->addElement(new htmlInputCheckbox('rememberLogin', $doRemember));
								$rememberGroup->addElement(new htmlSpacer('1px', null));
								$rememberGroup->addElement(new htmlOutputText(_('Remember user name')));
								$table->addElement($rememberGroup, true);
							}
							// login button
							$table->addElement(new htmlSpacer(null, '35px'));
							$table->addElement(new htmlHiddenInput('checklogin', 'checklogin'));
							$loginButton = new htmlButton('submit', _("Login"));
							$loginButton->alignment = htmlElement::ALIGN_LEFT;
							$table->addElement($loginButton, true);
							// error message
							if($error_message != "") {
								$message = new htmlStatusMessage('ERROR', $error_message);
								$message->colspan = 3;
								$table->addElement($message, true);
							}
							
							$tabindex = 1;
							parseHtml(null, $table, array(), false, $tabindex, 'user');
						?>
					</form>
				</td>
			</tr>
			<tr>
				<td align="left" style="border-style:none">
					<form action="login.php" method="post">
					<?php
						$table = new htmlTable('580px');
						$line = new htmlHorizontalLine();
						$line->colspan = 2;
						$table->addElement($line, true);
						$subTable = new htmlTable();
						$subTable->alignment = htmlElement::ALIGN_LEFT;
						// LDAP server
						$serverLabel = new htmlOutputText(_("LDAP server"));
						$serverLabel->alignment = htmlElement::ALIGN_RIGHT;
						$subTable->addElement($serverLabel);
						$subTable->addElement($gap);
						$serverName = new htmlOutputText($config_object->get_ServerURL());
						$serverName->alignment = htmlElement::ALIGN_LEFT;
						$subTable->addElement($serverName, true);
						// server profile
						$profileLabel = new htmlOutputText(_("Server profile"));
						$profileLabel->alignment = htmlElement::ALIGN_RIGHT;
						$subTable->addElement($profileLabel);
						$subTable->addElement($gap);
						$profileSelect = new htmlSelect('profile', $profiles, array($_SESSION['config']->getName()));
						$profileSelect->alignment = htmlElement::ALIGN_LEFT;
						$profileSelect->setOnchangeEvent('loginProfileChanged(this)');
						$subTable->addElement($profileSelect, true);
						$subTable->addElement(new htmlSpacer(null, '10px'));
						$table->addElement($subTable);

						parseHtml(null, $table, array(), true, $tabindex, 'user');
					?>
					</form>
				</td>
			</tr>
		</table>
		</div>
		<?php
		}
		?>
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

	$clientSource = $_SERVER['REMOTE_ADDR'];
	if (isset($_SERVER['REMOTE_HOST'])) {
		$clientSource .= '/' . $_SERVER['REMOTE_HOST'];
	}
	if (($_SESSION['config']->getLoginMethod() == LAMConfig::LOGIN_SEARCH) && ($_SESSION['config']->getHttpAuthentication() == 'true')) {
		$username = $_SERVER['PHP_AUTH_USER'];
		$password = $_SERVER['PHP_AUTH_PW'];
	}
	else {
		if (isset($_POST['rememberLogin']) && ($_POST['rememberLogin'] == 'on')) {
			setcookie('lam_login_name', $_POST['username'], time() + 60*60*24*365);
		}
		else if (isset($_COOKIE['lam_login_name']) && ($_SESSION['config']->getLoginMethod() == LAMConfig::LOGIN_SEARCH)) {
			setcookie('lam_login_name', '', time() + 60*60*24*365);
		}
		if($_POST['passwd'] == "") {
			logNewMessage(LOG_DEBUG, "Empty password for login");
			$error_message = _("Empty password submitted. Please try again.");
			display_LoginPage($_SESSION['config']); // Empty password submitted. Return to login page.
			exit();
		}
		if (get_magic_quotes_gpc() == 1) {
			$_POST['passwd'] = stripslashes($_POST['passwd']);
		}
		$username = $_POST['username'];
		$password = $_POST['passwd'];
	}
	// search user in LDAP if needed
	if ($_SESSION['config']->getLoginMethod() == LAMConfig::LOGIN_SEARCH) {
		$searchFilter = $_SESSION['config']->getLoginSearchFilter();
		$searchFilter = str_replace('%USER%', $username ,$searchFilter);
		$searchDN = '';
		$searchPassword = '';
		if (($_SESSION['config']->getLoginSearchDN() != null) && ($_SESSION['config']->getLoginSearchDN() != '')) {
			$searchDN = $_SESSION['config']->getLoginSearchDN();
			$searchPassword = $_SESSION['config']->getLoginSearchPassword();
		}
		$searchSuccess = true;
		$searchError = '';
		$searchLDAP = new Ldap($_SESSION['config']);
		$searchLDAPResult = $searchLDAP->connect($searchDN, $searchPassword, true);
		if (! ($searchLDAPResult == 0)) {
			$searchSuccess = false;
			$searchError = _('Cannot connect to specified LDAP server. Please try again.') . ' ' . @ldap_error($searchLDAP->server());
		}
		else {
			$searchResult = @ldap_search($searchLDAP->server(), $_SESSION['config']->getLoginSearchSuffix(), $searchFilter, array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
			if ($searchResult) {
				$searchInfo = @ldap_get_entries($searchLDAP->server(), $searchResult);
				if ($searchInfo) {
					cleanLDAPResult($searchInfo);
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
			logNewMessage(LOG_ERR, 'User ' . $username . ' (' . $clientSource . ') failed to log in. ' . $searchError . '');
			$searchLDAP->close();
			display_LoginPage($_SESSION['config']);
			exit();
		}
		$searchLDAP->close();
	}
	// try to connect to LDAP
	$result = $_SESSION['ldap']->connect($username, $password); // Connect to LDAP server for verifing username/password
	if($result === 0) {// Username/password correct. Do some configuration and load main frame.
		$_SESSION['loggedIn'] = true;
		// set security settings for session
		$_SESSION['sec_session_id'] = session_id();
		$_SESSION['sec_client_ip'] = $_SERVER['REMOTE_ADDR'];
		$_SESSION['sec_sessionTime'] = time();
		// logging
		logNewMessage(LOG_NOTICE, 'User ' . $username . ' (' . $clientSource . ') successfully logged in.');
		// Load main frame
		metaRefresh("./main.php");
		die();
	}
	else {
		if ($result === False) {
			// connection failed
			$error_message = _("Cannot connect to specified LDAP server. Please try again.");
			logNewMessage(LOG_ERR, 'User ' . $username . ' (' . $clientSource . ') failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
		}
		elseif ($result == 81) {
			// connection failed
			$error_message = _("Cannot connect to specified LDAP server. Please try again.");
			logNewMessage(LOG_ERR, 'User ' . $username . ' (' . $clientSource . ') failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
		}
		elseif ($result == 49) {
			// user name/password invalid. Return to login page.
			$error_message = _("Wrong password/user name combination. Please try again.");
			logNewMessage(LOG_ERR, 'User ' . $username . ' (' . $clientSource . ') failed to log in (wrong password).');
		}
		else {
			// other errors
			$error_message = _("LDAP error, server says:") .  "\n<br>($result) " . ldap_err2str($result);
			logNewMessage(LOG_ERR, 'User ' . $username . ' (' . $clientSource . ') failed to log in (LDAP error: ' . ldap_err2str($result) . ').');
		}
		display_LoginPage($_SESSION['config']);
		exit();
	}
}

//displays the login window
display_LoginPage($_SESSION["config"]);
?>
