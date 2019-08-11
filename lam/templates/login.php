<?php
namespace LAM\LOGIN;
use LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use \LAMConfig;
use \LAMCfgMain;
use \htmlSpacer;
use \htmlOutputText;
use \htmlSelect;
use \htmlInputField;
use \htmlGroup;
use \htmlInputCheckbox;
use \htmlButton;
use \htmlStatusMessage;
use \Ldap;
use \htmlResponsiveRow;
use \htmlDiv;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2005 - 2019  Roland Gruber

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
include_once(__DIR__ . "/../lib/status.inc");

/** check environment */
include __DIR__ . '/../lib/checkEnvironment.inc';

/** security functions */
include_once(__DIR__ . "/../lib/security.inc");
/** self service functions */
include_once(__DIR__ . "/../lib/selfService.inc");
/** access to configuration options */
include_once(__DIR__ . "/../lib/config.inc");
$licenseValidator = null;
if (isLAMProVersion()) {
	include_once(__DIR__ . "/../lib/env.inc");
	$licenseValidator = new \LAM\ENV\LAMLicenseValidator();
	$licenseValidator->validateAndRedirect('config/mainlogin.php?invalidLicense=1', 'config/mainlogin.php?invalidLicense=2');
}

/** Upgrade functions */
include_once(__DIR__ . "/../lib/upgrade.inc");

// set session save path
if (strtolower(session_module_name()) == 'files') {
	session_save_path(dirname(__FILE__) . '/../sess');
}

// start empty session and change ID for security reasons
lam_start_session();
session_destroy();
lam_start_session();
session_regenerate_id(true);

$profiles = getConfigProfiles();

// save last selected login profile
if (isset($_GET['useProfile'])) {
	if (in_array($_GET['useProfile'], $profiles)) {
		setcookie("lam_default_profile", $_GET['useProfile'], time() + 365*60*60*24, '/', null, null, true);
	}
	else {
		unset($_GET['useProfile']);
	}
}

// save last selected language
if (isset($_POST['language'])) {
	setcookie('lam_last_language', htmlspecialchars($_POST['language']), time() + 365*60*60*24, '/', null, null, true);
}

// init some session variables
$default_Config = new LAMCfgMain();
$_SESSION["cfgMain"] = $default_Config;
setSSLCaCert();

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

$error_message = null;

if (!isset($default_Config->default) || !in_array($default_Config->default, $profiles)) {
	$error_message = _('No default profile set. Please set it in the server profile configuration.');
}

$possibleLanguages = getLanguages();
$encoding = 'UTF-8';
if (isset($_COOKIE['lam_last_language'])) {
	foreach ($possibleLanguages as $lang) {
		if (strpos($_COOKIE['lam_last_language'], $lang->code) === 0) {
			$_SESSION['language'] = $lang->code;
			$encoding = $lang->encoding;
			break;
		}
	}
}
elseif (!empty($_SESSION["config"])) {
	$defaultLang = $_SESSION["config"]->get_defaultLanguage();
	foreach ($possibleLanguages as $lang) {
		if (strpos($defaultLang, $lang->code) === 0) {
			$_SESSION['language'] = $lang->code;
			$encoding = $lang->encoding;
			break;
		}
	}
}
else {
	$_SESSION['language'] = 'en_GB.utf8';
}
if (isset($_POST['language'])) {
	foreach ($possibleLanguages as $lang) {
		if (strpos($_POST['language'], $lang->code) === 0) {
			$_SESSION['language'] = $lang->code;
			$encoding = $lang->encoding;
			break;
		}
	}
}

$_SESSION['header'] = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";
$_SESSION['header'] .= "<html>\n<head>\n";
$_SESSION['header'] .= "<meta http-equiv=\"content-type\" content=\"text/html; charset=" . $encoding . "\">\n";
$_SESSION['header'] .= "<meta http-equiv=\"pragma\" content=\"no-cache\">\n		<meta http-equiv=\"cache-control\" content=\"no-cache\">";
$manifestUrl = str_replace('/templates/login.php', '', getCallingURL());
$manifestUrl = preg_replace('/http(s)?:\\/\\/([^\\/])+/', '', $manifestUrl);
$manifestUrl = preg_replace('/\\?.*/', '', $manifestUrl);
$_SESSION['header'] .= '<link rel="manifest" href="' . $manifestUrl . '/templates/manifest.php">';

/**
* Displays the login window.
*
* @param \LAM\ENV\LAMLicenseValidator $licenseValidator license validator
* @param string $error_message error message to display
*/
function display_LoginPage($licenseValidator, $error_message) {
	$config_object = $_SESSION['config'];
	$cfgMain = $_SESSION["cfgMain"];
	logNewMessage(LOG_DEBUG, "Display login page");
	// generate 256 bit key and initialization vector for user/passwd-encryption
	if(function_exists('openssl_random_pseudo_bytes') && ($cfgMain->encryptSession == 'true')) {
		$key = openssl_random_pseudo_bytes(32);
		$iv = openssl_random_pseudo_bytes(16);
		// save both in cookie
		setcookie("Key", base64_encode($key), 0, "/", null, null, true);
		setcookie("IV", base64_encode($iv), 0, "/", null, null, true);
	}

	$profiles = getConfigProfiles();

	setlanguage(); // setting correct language

	echo $_SESSION["header"];
	printHeaderContents('LDAP Account Manager', '..');
	?>
	</head>
	<body class="admin">
	<?php
	// include all JavaScript files
	printJsIncludes('..');

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

	if (isLAMProVersion() && $licenseValidator->isEvaluationLicense()) {
		StatusMessage('INFO', _('Evaluation Licence'));
	}
	?>

		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30" width="34%">
					<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">
					<?php
						echo getLAMVersionText();
					?>
					</a>
				</td>
				<td align="center" height=30 width="34%">
					<span class="hide-for-small">
						<a href="http://www.ldap-account-manager.org/lamcms/lamPro"> <?php if (!isLAMProVersion()) { echo _("Want more features? Get LAM Pro!");} ?> </a>
					</span>
				</td>
				<td align="right" height=30 width="34%">
					<a class="margin-right5" href="./config/index.php"><IMG alt="configuration" src="../graphics/tools.png">&nbsp;<span class="hide-for-small"><?php echo _("LAM configuration") ?></span></a>
				</td>
			</tr>
		</table>

		<br>

		<?php
		if (!empty($config_object)) {
			// check extensions
			$extList = getRequiredExtensions();
			foreach ($extList as $extension) {
				if (!extension_loaded($extension)) {
					StatusMessage("ERROR", "A required PHP extension is missing!", $extension);
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
		if (isset($_GET['2factor']) && ($_GET['2factor'] == 'error')) {
			StatusMessage('ERROR', _("Unable to start 2-factor authentication."));
			echo "<br>";
		}
		elseif (isset($_GET['2factor']) && ($_GET['2factor'] == 'noToken')) {
			StatusMessage('ERROR', _("Unable to start 2-factor authentication because no tokens were found."));
			echo "<br>";
		}
		if (!empty($config_object)) {
		?>
		<br><br>
		<div class="centeredTable">
		<div class="roundedShadowBox limitWidth" style="position:relative; z-index:5;">
		<table border="0" rules="none" bgcolor="white" class="ui-corner-all">
			<tr>
				<td class="loginLogo hide-for-small" style="border-style:none" rowspan="3">
				</td>
				<td style="border-style:none">
					<form action="login.php" method="post">
						<?php
							$tabindex = 1;
							$row = new htmlResponsiveRow();
							$row->add(new htmlSpacer(null, '30px'), 0, 12, 12);
							// user name
							$row->addLabel(new htmlOutputText(_("User name")));
							if ($config_object->getLoginMethod() == LAMConfig::LOGIN_LIST) {
								$admins = $config_object->get_Admins();
								$adminList = array();
								foreach ($admins as $admin) {
									$text = explode(",", $admin);
									$text = explode("=", $text[0]);
									if (isset($text[1])) {
										$adminList[$text[1]] = $admin;
									}
									else {
										$adminList[$text[0]] = $admin;
									}
								}
								$selectedAdmin = array();
								if (isset($_POST['username']) && in_array($_POST['username'], $adminList)) {
									$selectedAdmin = array($_POST['username']);
								}
								$userSelect = new htmlSelect('username', $adminList, $selectedAdmin);
								$userSelect->setHasDescriptiveElements(true);
								$userSelect->setTransformSingleSelect(false);
								if (empty($_COOKIE['lam_login_name'])) {
									$userSelect->setCSSClasses(array('lam-initial-focus'));
								}
								$row->addField(new htmlDiv(null, $userSelect));
							}
							else {
								if ($config_object->getHttpAuthentication() == 'true') {
									$httpAuth = new htmlDiv(null, new htmlOutputText($_SERVER['PHP_AUTH_USER'] . '&nbsp;', false));
									$httpAuth->setCSSClasses(array('text-left', 'margin3'));
									$row->addField($httpAuth);
								}
								else {
									$user = '';
									if (isset($_COOKIE["lam_login_name"])) {
										$user = $_COOKIE["lam_login_name"];
									}
									$userNameInput = new htmlInputField('username', $user);
									if (empty($_COOKIE['lam_login_name'])) {
										$userNameInput->setCSSClasses(array('lam-initial-focus'));
									}
									$userInput = new htmlDiv(null, $userNameInput);
									$row->addField($userInput);
								}
							}
							// password
							$row->addLabel(new \htmlOutputText(_("Password")));
							if (($config_object->getLoginMethod() == LAMConfig::LOGIN_SEARCH) && ($config_object->getHttpAuthentication() == 'true')) {
								$passwordInputFake = new htmlDiv(null, new htmlOutputText('**********'));
								$passwordInputFake->setCSSClasses(array('text-left', 'margin3'));
								$row->addField($passwordInputFake);
							}
							else {
								$passwordInput = new htmlInputField('passwd');
								$passwordInput->setIsPassword(true);
								if (($config_object->getLoginMethod() == LAMConfig::LOGIN_SEARCH) && !empty($_COOKIE['lam_login_name'])) {
									$passwordInput->setCSSClasses(array('lam-initial-focus'));
								}
								$row->addField($passwordInput);
							}
							// language
							$row->addLabel(new htmlOutputText(_("Language")));
							$possibleLanguages = getLanguages();
							$languageList = array();
							$defaultLanguage = array();
							foreach ($possibleLanguages as $lang) {
								$languageList[$lang->description] = $lang->code;
								if (strpos(trim($_SESSION["language"]), $lang->code) === 0) {
									$defaultLanguage[] = $lang->code;
								}
							}
							$languageSelect = new htmlSelect('language', $languageList, $defaultLanguage);
							$languageSelect->setHasDescriptiveElements(true);
							$row->addField($languageSelect, true);
							// remember login user
							if (($config_object->getLoginMethod() == LAMConfig::LOGIN_SEARCH) && !($config_object->getHttpAuthentication() == 'true')) {
								$row->add(new htmlOutputText('&nbsp;', false), 0, 6, 6);
								$rememberGroup = new htmlGroup();
								$doRemember = false;
								if (isset($_COOKIE["lam_login_name"])) {
									$doRemember = true;
								}
								$rememberGroup->addElement(new htmlInputCheckbox('rememberLogin', $doRemember));
								$rememberGroup->addElement(new htmlSpacer('1px', null));
								$rememberGroup->addElement(new htmlOutputText(_('Remember user name')));
								$rememberDiv = new htmlDiv(null, $rememberGroup);
								$rememberDiv->setCSSClasses(array('text-left', 'margin3'));
								$row->add($rememberDiv, 12, 6, 6);
							}
							// login button
							$row->add(new htmlSpacer(null, '20px'), 12);
							$row->add(new htmlButton('checklogin', _("Login")), 12);
							// error message
							if(!empty($error_message)) {
								$row->add(new \htmlSpacer(null, '5px'), 12);
								$message = new htmlStatusMessage('ERROR', $error_message);
								$message->colspan = 3;
								$row->add($message, 12);
							}

							parseHtml(null, $row, array(), false, $tabindex, 'user');
						?>
					</form>
				</td>
				<td class="loginRightBox hide-for-small" style="border-style:none">
				</td>
			</tr>
			<tr>
				<td colspan="2" style="border-style:none;">
					<hr class="margin20">
				</td>
			</tr>
			<tr>
				<td style="border-style:none;">
					<form action="login.php" method="post">
					<?php
						$row = new htmlResponsiveRow();
						$row->addLabel(new htmlOutputText(_("LDAP server")));
						$serverUrl = new htmlOutputText($config_object->getServerDisplayNameGUI());
						$serverUrlDiv = new htmlDiv(null, $serverUrl);
						$serverUrlDiv->setCSSClasses(array('text-left', 'margin3'));
						$row->addField($serverUrlDiv);
						$row->addLabel(new htmlOutputText(_("Server profile")));
						$profileSelect = new htmlSelect('profile', $profiles, array($_SESSION['config']->getName()));
						$profileSelect->setOnchangeEvent('loginProfileChanged(this)');
						$row->addField($profileSelect);

						parseHtml(null, $row, array(), true, $tabindex, 'user');
					?>
					</form>
				</td>
				<td class="loginRightBox hide-for-small" style="border-style:none">
				</td>
			</tr>
		</table>
		</div>
		</div>
		<?php
		}
		?>
		<br><br>
		<?PHP
			if (isLAMProVersion() && $licenseValidator->isExpiringSoon()) {
				$licenseMessage = sprintf(_('Your licence expires on %s. You need to purchase a new licence to be able to use LAM Pro after this date.'), $licenseValidator->getLicense()->getExpirationDate()->format('Y-m-d'));
				StatusMessage('WARN', $licenseMessage);
			}
		?>
		<br><br>
	</body>
</html>
<?php
}

// checking if the submitted username/password is correct.
if(isset($_POST['checklogin'])) {
	include_once(__DIR__ . "/../lib/ldap.inc"); // Include ldap.php which provides Ldap class

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
			setcookie('lam_login_name', $_POST['username'], time() + 60*60*24*365, '/', null, null, true);
		}
		else if (isset($_COOKIE['lam_login_name']) && ($_SESSION['config']->getLoginMethod() == LAMConfig::LOGIN_SEARCH)) {
			setcookie('lam_login_name', '', time() + 60*60*24*365, '/', null, null, true);
		}
		if($_POST['passwd'] == "") {
			logNewMessage(LOG_DEBUG, "Empty password for login");
			$error_message = _("Empty password submitted. Please try again.");
			display_LoginPage($licenseValidator, $error_message); // Empty password submitted. Return to login page.
			exit();
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
		$configLoginSearchDn = $_SESSION['config']->getLoginSearchDN();
		if (!empty($configLoginSearchDn)) {
			$searchDN = $configLoginSearchDn;
			$searchPassword = $_SESSION['config']->getLoginSearchPassword();
		}
		$searchSuccess = true;
		$searchError = '';
		$searchLDAP = new Ldap($_SESSION['config']);
		$searchLDAPResult = $searchLDAP->connect($searchDN, $searchPassword, true);
		if (! ($searchLDAPResult == 0)) {
			$searchSuccess = false;
			$searchError = _('Cannot connect to specified LDAP server. Please try again.') . ' ' . getDefaultLDAPErrorString($searchLDAP->server());
		}
		else {
			$searchResult = ldap_search($searchLDAP->server(), $_SESSION['config']->getLoginSearchSuffix(), $searchFilter, array('dn'), 0, 0, 0, LDAP_DEREF_NEVER);
			if ($searchResult) {
				$searchInfo = ldap_get_entries($searchLDAP->server(), $searchResult);
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
					if (ldap_errno($searchLDAP->server()) != 0) {
						$searchError .= ' ' . getDefaultLDAPErrorString($searchLDAP->server());
					}
				}
			}
			else {
				$searchSuccess = false;
				$searchError = _('Unable to find the user name in LDAP.');
				if (ldap_errno($searchLDAP->server()) != 0) {
					$searchError .= ' ' . getDefaultLDAPErrorString($searchLDAP->server());
				}
			}
		}
		if (!$searchSuccess) {
			$error_message = $searchError;
			logNewMessage(LOG_ERR, 'User ' . $username . ' (' . $clientSource . ') failed to log in. ' . $searchError . '');
			$searchLDAP->close();
			display_LoginPage($licenseValidator, $error_message);
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
		addSecurityTokenToSession();
		// logging
		logNewMessage(LOG_NOTICE, 'User ' . $username . ' (' . $clientSource . ') successfully logged in.');
		// Load main frame or 2 factor page
		if ($_SESSION['config']->getTwoFactorAuthentication() == TwoFactorProviderService::TWO_FACTOR_NONE) {
			metaRefresh("./main.php");
		}
		else {
			$_SESSION['2factorRequired'] = true;
			metaRefresh("./login2Factor.php");
		}
		die();
	}
	else {
		if (($result === False)
				|| ($result == 81)) {
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
		display_LoginPage($licenseValidator, $error_message);
		exit();
	}
}

//displays the login window
display_LoginPage($licenseValidator, $error_message);
?>
