<?php
namespace LAM\CONFIG;
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use \LAMConfig;
use \htmlTable;
use \htmlTableExtendedInputField;
use \htmlAccordion;
use \htmlTableExtendedSelect;
use \htmlFieldset;
use \htmlSpacer;
use \htmlTableExtendedInputCheckbox;
use \DateTimeZone;
use \htmlStatusMessage;
use \htmlOutputText;
use \htmlInputCheckbox;
use \htmlHelpLink;
use \htmlTableExtendedInputTextarea;
use \htmlElement;
use \htmlSubTitle;
use \htmlButton;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2017  Roland Gruber

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
* Main page of configuration
*
* @package configuration
* @author Roland Gruber
* @author Thomas Manninger
*/


/** Access to config functions */
include_once("../../lib/config.inc");
/** access to module settings */
include_once("../../lib/modules.inc");
/** access to tools */
include_once("../../lib/tools.inc");
/** 2-factor */
include_once '../../lib/2factor.inc';

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
@session_start();

setlanguage();

// get password
if (isset($_POST['passwd'])) $passwd = $_POST['passwd'];

// check if password was entered
// if not: load login page
if (!isset($passwd) && !(isset($_SESSION['conf_isAuthenticated']) && isset($_SESSION['conf_config']))) {
	$_SESSION['conf_message'] = _("No password was entered!");
	/** go back to login if password is empty */
	metaRefresh('conflogin.php');
	exit;
}

if (!isset($_SESSION['conf_config']) && isset($_POST['filename'])) {
	$_SESSION['conf_config'] = new LAMConfig($_POST['filename']);
}
$conf = &$_SESSION['conf_config'];

// check if password is valid
// if not: load login page
if ((!isset($_SESSION['conf_isAuthenticated']) || !($_SESSION['conf_isAuthenticated'] === $conf->getName())) && !$conf->check_Passwd($passwd)) {
	$sessionKeys = array_keys($_SESSION);
	for ($i = 0; $i < sizeof($sessionKeys); $i++) {
		if (substr($sessionKeys[$i], 0, 5) == "conf_") unset($_SESSION[$sessionKeys[$i]]);
	}
	$_SESSION['conf_message'] = _("The password is invalid! Please try again.");
	/** go back to login if password is invalid */
	metaRefresh('conflogin.php');
	exit;
}
$_SESSION['conf_isAuthenticated'] = $conf->getName();

// check if user canceled editing
if (isset($_POST['cancelSettings'])) {
	metaRefresh("../login.php");
	exit;
}

$errorsToDisplay = array();
if (isset($_SESSION['conf_messages']) && is_array($_SESSION['conf_messages'])) {
	$errorsToDisplay = array_merge($errorsToDisplay, $_SESSION['conf_messages']);
	unset($_SESSION['conf_messages']);
}

// check if button was pressed and if we have to save the settings or go to another tab
if (isset($_POST['saveSettings']) || isset($_POST['editmodules'])
	|| isset($_POST['edittypes']) || isset($_POST['generalSettingsButton'])
	|| isset($_POST['moduleSettings']) || isset($_POST['jobs'])) {
	$errorsToDisplay = checkInput();
	if (sizeof($errorsToDisplay) == 0) {
		// go to final page
		if (isset($_POST['saveSettings'])) {
			metaRefresh("confsave.php");
			exit;
		}
		// go to modules page
		elseif (isset($_POST['editmodules'])) {
			metaRefresh("confmodules.php");
			exit;
		}
		// go to types page
		elseif (isset($_POST['edittypes'])) {
			metaRefresh("conftypes.php");
			exit;
		}
		// go to module settings page
		elseif (isset($_POST['moduleSettings'])) {
			metaRefresh("moduleSettings.php");
			exit;
		}
		// go to jobs page
		elseif (isset($_POST['jobs'])) {
			metaRefresh("jobs.php");
			exit;
		}
	}
}


// index for tab order
$tabindex = 1;

echo $_SESSION['header'];

echo ("<title>" . _("LDAP Account Manager Configuration") . "</title>\n");

// include all CSS files
$cssDirName = dirname(__FILE__) . '/../../style';
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
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
}

echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../../graphics/favicon.ico\">\n";
echo "<link rel=\"icon\" href=\"../../graphics/logo136.png\">\n";
echo ("</head>\n");
echo ("<body>\n");
// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/../lib';
$jsDir = dir($jsDirName);
$jsFiles = array();
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	$jsFiles[] = $jsEntry;
}
sort($jsFiles);
foreach ($jsFiles as $jsEntry) {
	echo "<script type=\"text/javascript\" src=\"../lib/" . $jsEntry . "\"></script>\n";
}
?>
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">LDAP Account Manager</a>
				</td>
				<td align="right">
					<?php echo _('Server profile') . ': ' . $conf->getName(); ?>
					&nbsp;&nbsp;
				</td>
			</tr>
		</table>
		<br>
<?php

if (!$conf->isWritable()) {
	StatusMessage('WARN', _('The config file is not writable.'), _('Your changes cannot be saved until you make the file writable for the webserver user.'));
	echo "<br>";
}

// display error messages
if (sizeof($errorsToDisplay) > 0) {
	for ($i = 0; $i < sizeof($errorsToDisplay); $i++) {
		call_user_func_array('StatusMessage', $errorsToDisplay[$i]);
	}
	echo "<br>";
}

// display formular
echo ("<form enctype=\"multipart/form-data\" action=\"confmain.php\" method=\"post\" autocomplete=\"off\">\n");

// hidden submit buttons which are clicked by tabs
echo "<div style=\"display: none;\">\n";
	echo "<input name=\"generalSettingsButton\" type=\"submit\" value=\" \">";
	echo "<input name=\"edittypes\" type=\"submit\" value=\" \">";
	echo "<input name=\"editmodules\" type=\"submit\" value=\" \">";
	echo "<input name=\"moduleSettings\" type=\"submit\" value=\" \">";
	echo "<input name=\"jobs\" type=\"submit\" value=\" \">";
echo "</div>\n";

// tabs
echo '<div class="ui-tabs ui-widget ui-widget-content ui-corner-all">';

echo '<ul class="ui-tabs-nav ui-helper-reset ui-helper-clearfix ui-widget-header ui-corner-all">';
echo '<li id="generalSettingsButton" class="ui-state-default ui-corner-top">';
	echo '<a href="#" onclick="document.getElementsByName(\'generalSettingsButton\')[0].click();"><img src="../../graphics/tools.png" alt=""> ';
	echo _('General settings') . '</a>';
echo '</li>';
echo '<li id="edittypes" class="ui-state-default ui-corner-top" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
	echo '<a href="#" onclick="document.getElementsByName(\'edittypes\')[0].click();"><img src="../../graphics/gear.png" alt=""> ';
	echo _('Account types') . '</a>';
echo '</li>';
echo '<li id="editmodules" class="ui-state-default ui-corner-top" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
	echo '<a href="#" onclick="document.getElementsByName(\'editmodules\')[0].click();"><img src="../../graphics/modules.png" alt=""> ';
	echo _('Modules') . '</a>';
echo '</li>';
echo '<li id="moduleSettings" class="ui-state-default ui-corner-top" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
	echo '<a href="#" onclick="document.getElementsByName(\'moduleSettings\')[0].click();"><img src="../../graphics/modules.png" alt=""> ';
	echo _('Module settings') . '</a>';
echo '</li>';
if (isLAMProVersion()) {
	echo '<li id="jobs" class="ui-state-default ui-corner-top" onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
		echo '<a href="#" onclick="document.getElementsByName(\'jobs\')[0].click();"><img src="../../graphics/clock.png" alt=""> ';
		echo _('Jobs') . '</a>';
	echo '</li>';
}
echo '</ul>';

?>

<script type="text/javascript">
jQuery(document).ready(function() {
	jQuery('#generalSettingsButton').addClass('ui-tabs-active');
	jQuery('#generalSettingsButton').addClass('ui-state-active');
	jQuery('#generalSettingsButton').addClass('user-bright');
});
</script>

<div class="ui-tabs-panel ui-widget-content ui-corner-bottom user-bright">
<input type="text" name="hiddenPreventAutocomplete" autocomplete="false" class="hidden" value="">
<input type="password" name="hiddenPreventAutocompletePwd1" autocomplete="false" class="hidden" value="">
<input type="password" name="hiddenPreventAutocompletePwd2" autocomplete="false" class="hidden" value="">
<?php

$container = new htmlTable();

$serverSettingsContent = new htmlTable();
// server URL
$urlInput = new htmlTableExtendedInputField(_("Server address"), 'serverurl', $conf->get_ServerURL(), '201');
$urlInput->setRequired(true);
$serverSettingsContent->addElement($urlInput, true);
// use TLS
$tlsOptions = array(_("yes") => 'yes', _("no") => 'no');
$tlsSelect = new htmlTableExtendedSelect('useTLS', $tlsOptions, array($conf->getUseTLS()), _("Activate TLS"), '201');
$tlsSelect->setHasDescriptiveElements(true);
$serverSettingsContent->addElement($tlsSelect, true);
// tree suffix
$serverSettingsContent->addElement(new htmlTableExtendedInputField(_("Tree suffix"), 'sufftree', $conf->get_Suffix('tree'), '203'), true);
// LDAP search limit
$searchLimitOptions = array(
'-' => 0,		100 => 100,		500 => 500,
1000 => 1000,	5000 => 5000,	10000 => 10000,
50000 => 50000,	100000 => 100000);
$limitSelect = new htmlTableExtendedSelect('searchLimit', $searchLimitOptions, array($conf->get_searchLimit()), _("LDAP search limit"), '222');
$limitSelect->setHasDescriptiveElements(true);
$serverSettingsContent->addElement($limitSelect, true);

// access level is only visible in Pro version
if (isLAMProVersion()) {
	$accessOptions = array(
		_('Write access') => LAMConfig::ACCESS_ALL,
		_('Change passwords') => LAMConfig::ACCESS_PASSWORD_CHANGE,
		_('Read-only') => LAMConfig::ACCESS_READ_ONLY
	);
	$accessSelect = new htmlTableExtendedSelect('accessLevel', $accessOptions, array($conf->getAccessLevel()), _("Access level"), '215');
	$accessSelect->setHasDescriptiveElements(true);
	$serverSettingsContent->addElement($accessSelect, true);
}

// advanced options
$advancedOptionsContent = new htmlTable();
// display name
$advancedOptionsContent->addElement(new htmlTableExtendedInputField(_('Display name'), 'serverDisplayName', $conf->getServerDisplayName(), '268'), true);
// referrals
$followReferrals = ($conf->getFollowReferrals() === 'true');
$advancedOptionsContent->addElement(new htmlTableExtendedInputCheckbox('followReferrals', $followReferrals , _('Follow referrals'), '205'), true);
// paged results
$pagedResults = ($conf->getPagedResults() === 'true');
$advancedOptionsContent->addElement(new htmlTableExtendedInputCheckbox('pagedResults', $pagedResults , _('Paged results'), '266'), true);

// build advanced options box
$advancedOptions = new htmlAccordion('advancedOptions_server', array(_('Advanced options') => $advancedOptionsContent), false);
$advancedOptions->colspan = 15;
$serverSettingsContent->addElement($advancedOptions, true);

$serverSettings = new htmlFieldset($serverSettingsContent, _("Server settings"), '../../graphics/profiles.png');
$container->addElement($serverSettings, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

// language
$languageSettingsContent = new htmlTable();
// read available languages
$possibleLanguages = getLanguages();
$defaultLanguage = array('en_GB.utf8');
if(!empty($possibleLanguages)) {
	foreach ($possibleLanguages as $lang) {
		$languages[$lang->description] = $lang->code;
		if (strpos($conf->get_defaultLanguage(), $lang->code) === 0) {
			$defaultLanguage = array($lang->code);
		}
	}
	$languageSelect = new htmlTableExtendedSelect('lang', $languages, $defaultLanguage, _("Default language"), '209');
	$languageSelect->setHasDescriptiveElements(true);
	$languageSettingsContent->addElement($languageSelect, true);
}
else {
	$languageSettingsContent->addElement(new htmlStatusMessage('ERROR', "Unable to load available languages. Setting English as default language."));
}
$timezones = array();
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::AFRICA));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::AMERICA));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::ANTARCTICA));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::ARCTIC));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::ASIA));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::ATLANTIC));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::AUSTRALIA));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::EUROPE));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::INDIAN));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::PACIFIC));
$timezones = array_merge($timezones, DateTimeZone::listIdentifiers(DateTimeZone::UTC));
$languageSettingsContent->addElement(new htmlTableExtendedSelect('timeZone', $timezones, array($conf->getTimeZone()), _('Time zone'), '213'), true);
$languageSettings = new htmlFieldset($languageSettingsContent, _("Language settings"), '../../graphics/language.png');
$container->addElement($languageSettings, true);


$container->addElement(new htmlSpacer(null, '10px'), true);

// lamdaemon settings
$lamdaemonSettingsContent = new htmlTable();
$lamdaemonSettingsContent->addElement(new htmlTableExtendedInputField(_("Server list"), 'scriptservers', $conf->get_scriptServers(), '218'), true);
$lamdaemonSettingsContent->addElement(new htmlTableExtendedInputField(_("Path to external script"), 'scriptpath', $conf->get_scriptPath(), '210'), true);

$lamdaemonSettingsContent->addElement(new htmlTableExtendedInputField(_('User name'), 'scriptuser', $conf->getScriptUserName(), '284'), true);
$lamdaemonSettingsContent->addElement(new htmlTableExtendedInputField(_('SSH key file'), 'scriptkey', $conf->getScriptSSHKey(), '285'), true);
$sshKeyPassword = new htmlTableExtendedInputField(_('SSH key password'), 'scriptkeypassword', $conf->getScriptSSHKeyPassword(), '286');
$sshKeyPassword->setIsPassword(true);
$lamdaemonSettingsContent->addElement($sshKeyPassword, true);

$lamdaemonSettingsContent->addElement(new htmlSpacer(null, '5px'), true);
$lamdaemonSettingsContent->addElement(new htmlOutputText(_("Rights for the home directory")));
$chmod = $conf->get_scriptRights();
$rightsTable = new htmlTable();
$rightsTable->addElement(new htmlOutputText(''));
$rightsTable->addElement(new htmlOutputText(_("Read")));
$rightsTable->addElement(new htmlOutputText(_("Write")));
$rightsTable->addElement(new htmlOutputText(_("Execute")), true);
$rightsTable->addElement(new htmlOutputText(_("Owner")));
$rightsTable->addElement(new htmlInputCheckbox('chmod_owr', checkChmod("read","owner", $chmod)));
$rightsTable->addElement(new htmlInputCheckbox('chmod_oww', checkChmod("write","owner", $chmod)));
$rightsTable->addElement(new htmlInputCheckbox('chmod_owe', checkChmod("execute","owner", $chmod)), true);
$rightsTable->addElement(new htmlOutputText(_("Group")));
$rightsTable->addElement(new htmlInputCheckbox('chmod_grr', checkChmod("read","group", $chmod)));
$rightsTable->addElement(new htmlInputCheckbox('chmod_grw', checkChmod("write","group", $chmod)));
$rightsTable->addElement(new htmlInputCheckbox('chmod_gre', checkChmod("execute","group", $chmod)), true);
$rightsTable->addElement(new htmlOutputText(_("Other")));
$rightsTable->addElement(new htmlInputCheckbox('chmod_otr', checkChmod("read","other", $chmod)));
$rightsTable->addElement(new htmlInputCheckbox('chmod_otw', checkChmod("write","other", $chmod)));
$rightsTable->addElement(new htmlInputCheckbox('chmod_ote', checkChmod("execute","other", $chmod)), true);
$lamdaemonSettingsContent->addElement($rightsTable);
$lamdaemonSettingsContent->addElement(new htmlHelpLink('219'));
$lamdaemonSettings = new htmlFieldset($lamdaemonSettingsContent, _("Lamdaemon settings"), '../../graphics/lamdaemon.png');
$container->addElement($lamdaemonSettings, true);
$container->addElement(new htmlSpacer(null, '10px'), true);


// LAM Pro settings
if (isLAMProVersion()) {
	// password reset page
	$pwdResetContent = new htmlTable();

	$pwdResetAllowSpecific = true;
	if ($conf->getPwdResetAllowSpecificPassword() == 'false') {
		$pwdResetAllowSpecific = false;
	}
	$pwdResetContent->addElement(new htmlTableExtendedInputCheckbox('pwdResetAllowSpecificPassword', $pwdResetAllowSpecific , _('Allow setting specific passwords'), '280'));

	$pwdResetContent->addSpace('10px');

	$pwdResetAllowScreenPassword = true;
	if ($conf->getPwdResetAllowScreenPassword() == 'false') {
		$pwdResetAllowScreenPassword = false;
	}
	$pwdResetContent->addElement(new htmlTableExtendedInputCheckbox('pwdResetAllowScreenPassword', $pwdResetAllowScreenPassword , _('Allow to display password on screen'), '281'), true);

	$pwdResetDefaultPasswordOutputOptions = array(
		_('Display on screen') => LAMConfig::PWDRESET_DEFAULT_SCREEN,
		_('Send via mail') => LAMConfig::PWDRESET_DEFAULT_MAIL,
		_('Both') => LAMConfig::PWDRESET_DEFAULT_BOTH
	);
	$pwdResetDefaultPasswordOutputSelect = new htmlTableExtendedSelect('pwdResetDefaultPasswordOutput', $pwdResetDefaultPasswordOutputOptions, array($conf->getPwdResetDefaultPasswordOutput()), _("Default password output"), '282');
	$pwdResetDefaultPasswordOutputSelect->setHasDescriptiveElements(true);
	$pwdResetContent->addElement($pwdResetDefaultPasswordOutputSelect);

	$pwdResetContent->addSpace('10px');

	$pwdResetForcePasswordChange = true;
	if ($conf->getPwdResetForcePasswordChange() == 'false') {
		$pwdResetForcePasswordChange = false;
	}
	$pwdResetContent->addElement(new htmlTableExtendedInputCheckbox('pwdResetForcePasswordChange', $pwdResetForcePasswordChange , _('Force password change by default'), '283'), true);

	$pwdResetFieldset = new htmlFieldset($pwdResetContent, _("Password reset page settings"), '../../graphics/keyBig.png');
	$container->addElement($pwdResetFieldset, true);
	$container->addElement(new htmlSpacer(null, '10px'), true);

	// mail settings
	$pwdMailContent = new htmlTable();

	$pwdMailFrom = new htmlTableExtendedInputField(_('From address'), 'pwdResetMail_from', $conf->getLamProMailFrom(), '550');
	$pwdMailContent->addElement($pwdMailFrom, true);

	$pwdMailReplyTo = new htmlTableExtendedInputField(_('Reply-to address'), 'pwdResetMail_replyTo', $conf->getLamProMailReplyTo(), '554');
	$pwdMailContent->addElement($pwdMailReplyTo, true);

	$pwdMailSubject = new htmlTableExtendedInputField(_('Subject'), 'pwdResetMail_subject', $conf->getLamProMailSubject(), '551');
	$pwdMailContent->addElement($pwdMailSubject, true);

	$pwdMailIsHTML = false;
	if ($conf->getLamProMailIsHTML() == 'true') {
		$pwdMailIsHTML = true;
	}
	$pwdMailContent->addElement(new htmlTableExtendedInputCheckbox('pwdResetMail_isHTML',$pwdMailIsHTML , _('HTML format'), '553'), true);

	$pwdMailAllowAlternate = true;
	if ($conf->getLamProMailAllowAlternateAddress() == 'false') {
		$pwdMailAllowAlternate = false;
	}
	$pwdMailContent->addElement(new htmlTableExtendedInputCheckbox('pwdResetMail_allowAlternate',$pwdMailAllowAlternate , _('Allow alternate address'), '555'), true);

	$pwdMailBody = new htmlTableExtendedInputTextarea('pwdResetMail_body', $conf->getLamProMailText(), 50, 4, _('Text'), '552');
	$pwdMailContent->addElement($pwdMailBody, true);

	$pwdMailFieldset = new htmlFieldset($pwdMailContent, _("Password mail settings"), '../../graphics/mailBig.png');
	$container->addElement($pwdMailFieldset, true);
	$container->addElement(new htmlSpacer(null, '10px'), true);
}

// tool settings
$toolSettings = $conf->getToolSettings();
$toolSettingsContent = new htmlTable();
$toolsLabel = new htmlOutputText(_('Hidden tools'));
$toolsLabel->colspan = 5;
$toolSettingsContent->addElement($toolsLabel, true);
$tools = getTools();
for ($i = 0; $i < sizeof($tools); $i++) {
	$tool = new $tools[$i]();
	if ($tool->isHideable()) {
		$tools[$i] = $tool;
	}
	else {
		unset($tools[$i]);
		$i--;
		$tools = array_values($tools);
	}
}
for ($r = 0; $r < (sizeof($tools) / 4); $r++) {
	for ($c = 0; $c < 4; $c++) {
		if (!isset($tools[($r * 4) + $c])) {
			break;
		}
		$tool = $tools[($r * 4) + $c];
		$toolClass = get_class($tool);
		$toolName = substr($toolClass, strrpos($toolClass, '\\') + 1);
		$selected = false;
		if (isset($toolSettings['tool_hide_' . $toolName]) && ($toolSettings['tool_hide_' . $toolName] === 'true')) {
			$selected = true;
		}
		$toolSettingsContent->addElement(new htmlTableExtendedInputCheckbox('tool_hide_' . $toolName, $selected, $tool->getName(), null, false));
		$toolSettingsContent->addElement(new htmlSpacer('10px', null));
	}
	$toolSettingsContent->addNewLine();
}
$toolSettingsFieldset = new htmlFieldset($toolSettingsContent, _("Tool settings"), '../../graphics/bigTools.png');
$container->addElement($toolSettingsFieldset, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

// security setings
$securitySettingsContent = new htmlTable();
// login method
$loginOptions = array(
	_('Fixed list') => LAMConfig::LOGIN_LIST,
	_('LDAP search') => LAMConfig::LOGIN_SEARCH
);
$loginSelect = new htmlTableExtendedSelect('loginMethod', $loginOptions, array($conf->getLoginMethod()), _("Login method"), '220');
$loginSelect->setHasDescriptiveElements(true);
$loginSelect->setTableRowsToHide(array(
	LAMConfig::LOGIN_LIST => array('loginSearchSuffix', 'loginSearchFilter', 'loginSearchDN', 'loginSearchPassword', 'httpAuthentication'),
	LAMConfig::LOGIN_SEARCH => array('admins')
));
$loginSelect->setTableRowsToShow(array(
	LAMConfig::LOGIN_LIST => array('admins'),
	LAMConfig::LOGIN_SEARCH => array('loginSearchSuffix', 'loginSearchFilter', 'loginSearchDN', 'loginSearchPassword', 'httpAuthentication')
));
$securitySettingsContent->addElement($loginSelect, true);
// admin list
$adminText = implode("\n", explode(";", $conf->get_Adminstring()));
$adminTextInput = new htmlTableExtendedInputTextarea('admins', $adminText, '50', '3', _("List of valid users"), '207');
$adminTextInput->setRequired(true);
$securitySettingsContent->addElement($adminTextInput, true);
// search suffix
$searchSuffixInput = new htmlTableExtendedInputField(_("LDAP suffix"), 'loginSearchSuffix', $conf->getLoginSearchSuffix(), '221');
$searchSuffixInput->setRequired(true);
$securitySettingsContent->addElement($searchSuffixInput, true);
// login search filter
$searchFilterInput = new htmlTableExtendedInputField(_("LDAP filter"), 'loginSearchFilter', $conf->getLoginSearchFilter(), '221');
$searchFilterInput->setRequired(true);
$securitySettingsContent->addElement($searchFilterInput, true);
// login search bind user
$securitySettingsContent->addElement(new htmlTableExtendedInputField(_("Bind user"), 'loginSearchDN', $conf->getLoginSearchDN(), '224'), true);
// login search bind password
$searchPasswordInput = new htmlTableExtendedInputField(_("Bind password"), 'loginSearchPassword', $conf->getLoginSearchPassword(), '224');
$searchPasswordInput->setIsPassword(true);
$securitySettingsContent->addElement($searchPasswordInput, true);
// HTTP authentication
$securitySettingsContent->addElement(new htmlTableExtendedInputCheckbox('httpAuthentication', ($conf->getHttpAuthentication() == 'true'), _('HTTP authentication'), '223', true), true);
$securitySettingsContent->addElement(new htmlSpacer(null, '30px'), true);

// 2factor authentication
if (extension_loaded('curl')) {
	$securitySettingsContent->addElement(new htmlSubTitle(_("2-factor authentication")), true);
	$twoFactorOptions = array(
			_('None') => TwoFactorProviderService::TWO_FACTOR_NONE,
			'privacyIDEA' => TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA,
	);
	$twoFactorSelect = new htmlTableExtendedSelect('twoFactor', $twoFactorOptions, array($conf->getTwoFactorAuthentication()), _('Provider'), '514');
	$twoFactorSelect->setHasDescriptiveElements(true);
	$twoFactorSelect->setTableRowsToHide(array(
			TwoFactorProviderService::TWO_FACTOR_NONE => array('twoFactorURL', 'twoFactorInsecure', 'twoFactorLabel', 'twoFactorOptional', 'twoFactorCaption')
	));
	$twoFactorSelect->setTableRowsToShow(array(
			TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA => array('twoFactorURL', 'twoFactorInsecure', 'twoFactorLabel', 'twoFactorOptional', 'twoFactorCaption')
	));
	$securitySettingsContent->addElement($twoFactorSelect, true);
	$twoFactorUrl = new htmlTableExtendedInputField(_("Base URL"), 'twoFactorURL', $conf->getTwoFactorAuthenticationURL(), '515');
	$twoFactorUrl->setRequired(true);
	$securitySettingsContent->addElement($twoFactorUrl, true);
	$twoFactorLabel = new htmlTableExtendedInputField(_("Label"), 'twoFactorLabel', $conf->getTwoFactorAuthenticationLabel(), '517');
	$securitySettingsContent->addElement($twoFactorLabel, true);
	$securitySettingsContent->addElement(new htmlTableExtendedInputCheckbox('twoFactorOptional', $conf->getTwoFactorAuthenticationOptional(), _('Optional'), '519'), true);
	$securitySettingsContent->addElement(new htmlTableExtendedInputCheckbox('twoFactorInsecure', $conf->getTwoFactorAuthenticationInsecure(), _('Disable certificate check'), '516'), true);
	$securitySettingsContent->addElement(new htmlSpacer(null, '5px'), true);
	$twoFactorCaption = new htmlTableExtendedInputTextarea('twoFactorCaption', $conf->getTwoFactorAuthenticationCaption(), '80', '4', _("Caption"), '518');
	$twoFactorCaption->setIsRichEdit(true);
	$twoFactorCaption->alignment = htmlElement::ALIGN_TOP;
	$securitySettingsContent->addElement($twoFactorCaption, true);
}

// new password
$securitySettingsContent->addElement(new htmlSubTitle(_("Profile password")), true);
$password1 = new htmlTableExtendedInputField(_("New password"), 'passwd1', null, '212');
$password1->setIsPassword(true);
$password2 = new htmlTableExtendedInputField(_("Reenter password"), 'passwd2');
$password2->setIsPassword(true);
$password2->setSameValueFieldID('passwd1');
$securitySettingsContent->addElement($password1, true);
$securitySettingsContent->addElement($password2, true);
$securitySettings = new htmlFieldset($securitySettingsContent, _("Security settings"), '../../graphics/security.png');
$container->addElement($securitySettings, true);
$container->addElement(new htmlSpacer(null, '10px'), true);

parseHtml(null, $container, array(), false, $tabindex, 'user');

echo "</div></div>";

$buttonContainer = new htmlTable();
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
$saveButton = new htmlButton('saveSettings', _('Save'));
$saveButton->setIconClass('saveButton');
$buttonContainer->addElement($saveButton);
$cancelButton = new htmlButton('cancelSettings', _('Cancel'));
$cancelButton->setIconClass('cancelButton');
$buttonContainer->addElement($cancelButton, true);
$buttonContainer->addElement(new htmlSpacer(null, '10px'), true);
parseHtml(null, $buttonContainer, array(), false, $tabindex, 'user');

?>
</form>
<script type="text/javascript" src="../lib/extra/ckeditor/ckeditor.js"></script>
</body>
</html>
<?php

/**
 * Checks user input and saves the entered settings.
 *
 * @return array list of errors
 */
function checkInput() {
	$conf = &$_SESSION['conf_config'];
	$types = $conf->get_ActiveTypes();

	// remove double slashes if magic quotes are on
	if (get_magic_quotes_gpc() == 1) {
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (is_string($_POST[$postKeys[$i]])) $_POST[$postKeys[$i]] = stripslashes($_POST[$postKeys[$i]]);
		}
	}
	// check new preferences
	$errors = array();
	if (!$conf->set_ServerURL($_POST['serverurl'])) {
		$errors[] = array("ERROR", _("Server address is invalid!"));
	}
	$conf->setServerDisplayName($_POST['serverDisplayName']);
	$conf->setUseTLS($_POST['useTLS']);
	if ((strpos($_POST['serverurl'], 'ldaps://') !== false) && ($_POST['useTLS'] == 'yes')) {
		$errors[] = array("ERROR", _('You cannot use SSL and TLS encryption at the same time. Please use either "ldaps://" or TLS.'));
	}
	if (isset($_POST['followReferrals']) && ($_POST['followReferrals'] == 'on')) {
		$conf->setFollowReferrals('true');
	}
	else {
		$conf->setFollowReferrals('false');
	}
	if (isset($_POST['pagedResults']) && ($_POST['pagedResults'] == 'on')) {
		$conf->setPagedResults('true');
	}
	else {
		$conf->setPagedResults('false');
	}
/*	if (!$conf->set_cacheTimeout($_POST['cachetimeout'])) {
		$errors[] = array("ERROR", _("Cache timeout is invalid!"));
	}*/
	$conf->set_searchLimit($_POST['searchLimit']);
	if (isLAMProVersion()) {
		$conf->setAccessLevel($_POST['accessLevel']);
		if (isset($_POST['pwdResetAllowSpecificPassword']) && ($_POST['pwdResetAllowSpecificPassword'] == 'on')) {
			$conf->setPwdResetAllowSpecificPassword('true');
		}
		else {
			$conf->setPwdResetAllowSpecificPassword('false');
		}
		if (isset($_POST['pwdResetAllowScreenPassword']) && ($_POST['pwdResetAllowScreenPassword'] == 'on')) {
			$conf->setPwdResetAllowScreenPassword('true');
		}
		else {
			$conf->setPwdResetAllowScreenPassword('false');
		}
		if (isset($_POST['pwdResetForcePasswordChange']) && ($_POST['pwdResetForcePasswordChange'] == 'on')) {
			$conf->setPwdResetForcePasswordChange('true');
		}
		else {
			$conf->setPwdResetForcePasswordChange('false');
		}
		$conf->setPwdResetDefaultPasswordOutput($_POST['pwdResetDefaultPasswordOutput']);
		if (!$conf->setLamProMailFrom($_POST['pwdResetMail_from'])) {
			$errors[] = array("ERROR", _("From address for password mails is invalid."), htmlspecialchars($_POST['pwdResetMail_from']));
		}
		if (!$conf->setLamProMailReplyTo($_POST['pwdResetMail_replyTo'])) {
			$errors[] = array("ERROR", _("Reply-to address for password mails is invalid."), htmlspecialchars($_POST['pwdResetMail_replyTo']));
		}
		$conf->setLamProMailSubject($_POST['pwdResetMail_subject']);
		if (isset($_POST['pwdResetMail_isHTML']) && ($_POST['pwdResetMail_isHTML'] == 'on')) {
			$conf->setLamProMailIsHTML('true');
		}
		else {
			$conf->setLamProMailIsHTML('false');
		}
		if (isset($_POST['pwdResetMail_allowAlternate']) && ($_POST['pwdResetMail_allowAlternate'] == 'on')) {
			$conf->setLamProMailAllowAlternateAddress('true');
		}
		else {
			$conf->setLamProMailAllowAlternateAddress('false');
		}
		$conf->setLamProMailText($_POST['pwdResetMail_body']);
	}
	$adminText = $_POST['admins'];
	$adminText = explode("\n", $adminText);
	$adminTextNew = array();
	for ($i = 0; $i < sizeof($adminText); $i++) {
		if (trim($adminText[$i]) == "") continue;
		$adminTextNew[] = trim($adminText[$i]);
	}
	$conf->setLoginMethod($_POST['loginMethod']);
	$conf->setLoginSearchFilter($_POST['loginSearchFilter']);
	$conf->setLoginSearchSuffix($_POST['loginSearchSuffix']);
	$conf->setLoginSearchPassword($_POST['loginSearchPassword']);
	$conf->setLoginSearchDN($_POST['loginSearchDN']);
	if ($_POST['loginMethod'] == LAMConfig::LOGIN_SEARCH) { // check only if search method
		if (!$conf->setLoginSearchDN($_POST['loginSearchDN'])) {
			$errors[] = array("ERROR", _("Please enter a valid bind user."));
		}
	}
	if (isset($_POST['httpAuthentication']) && ($_POST['httpAuthentication'] == 'on')) {
		$conf->setHttpAuthentication('true');
	}
	else {
		$conf->setHttpAuthentication('false');
	}
	if (!$conf->set_Adminstring(implode(";", $adminTextNew))) {
		$errors[] = array("ERROR", _("List of admin users is empty or invalid!"));
	}
	if (!$conf->set_Suffix("tree", $_POST['sufftree'])) {
		$errors[] = array("ERROR", _("TreeSuffix is invalid!"));
	}
	if (!$conf->set_defaultLanguage($_POST['lang'])) {
		$errors[] = array("ERROR", _("Language is not defined!"));
	}
	$conf->setTimeZone($_POST['timeZone']);
	if (!$conf->set_scriptpath($_POST['scriptpath'])) {
		$errors[] = array("ERROR", _("Script path is invalid!"));
	}
	if (!$conf->set_scriptservers($_POST['scriptservers'])) {
		$errors[] = array("ERROR", _("Script server is invalid!"));
	}
	$chmodOwner = 0;
	$chmodGroup = 0;
	$chmodOther = 0;
	if (isset($_POST['chmod_owr']) && ($_POST['chmod_owr'] == 'on')) $chmodOwner += 4;
	if (isset($_POST['chmod_oww']) && ($_POST['chmod_oww'] == 'on')) $chmodOwner += 2;
	if (isset($_POST['chmod_owe']) && ($_POST['chmod_owe'] == 'on')) $chmodOwner += 1;
	if (isset($_POST['chmod_grr']) && ($_POST['chmod_grr'] == 'on')) $chmodGroup += 4;
	if (isset($_POST['chmod_grw']) && ($_POST['chmod_grw'] == 'on')) $chmodGroup += 2;
	if (isset($_POST['chmod_gre']) && ($_POST['chmod_gre'] == 'on')) $chmodGroup += 1;
	if (isset($_POST['chmod_otr']) && ($_POST['chmod_otr'] == 'on')) $chmodOther += 4;
	if (isset($_POST['chmod_otw']) && ($_POST['chmod_otw'] == 'on')) $chmodOther += 2;
	if (isset($_POST['chmod_ote']) && ($_POST['chmod_ote'] == 'on')) $chmodOther += 1;
	$chmod = $chmodOwner . $chmodGroup . $chmodOther;
	if (!$conf->set_scriptrights($chmod)) {
		$errors[] = array("ERROR", _("Script rights are invalid!"));
	}
	$conf->setScriptUserName($_POST['scriptuser']);
	$conf->setScriptSSHKey($_POST['scriptkey']);
	$conf->setScriptSSHKeyPassword($_POST['scriptkeypassword']);
	// tool settings
	$tools = getTools();
	$toolSettings = array();
	for ($i = 0; $i < sizeof($tools); $i++) {
	    $toolClass = $tools[$i];
	    $toolName = substr($toolClass, strrpos($toolClass, '\\') + 1);
		$toolConfigID = 'tool_hide_' . $toolName;
		if ((isset($_POST[$toolConfigID])) && ($_POST[$toolConfigID] == 'on')) {
			$toolSettings[$toolConfigID] = 'true';
		}
		else {
			$toolSettings[$toolConfigID] = 'false';
		}
	}
	$conf->setToolSettings($toolSettings);
	// 2-factor
	if (extension_loaded('curl')) {
		$conf->setTwoFactorAuthentication($_POST['twoFactor']);
		$conf->setTwoFactorAuthenticationURL($_POST['twoFactorURL']);
		$conf->setTwoFactorAuthenticationInsecure(isset($_POST['twoFactorInsecure']) && ($_POST['twoFactorInsecure'] == 'on'));
		$conf->setTwoFactorAuthenticationLabel($_POST['twoFactorLabel']);
		$conf->setTwoFactorAuthenticationOptional(isset($_POST['twoFactorOptional']) && ($_POST['twoFactorOptional'] == 'on'));
		$conf->setTwoFactorAuthenticationCaption(str_replace(array("\r", "\n"), array('', ''), $_POST['twoFactorCaption']));
	}
	// check if password was changed
	if (isset($_POST['passwd1']) && ($_POST['passwd1'] != '')) {
		if ($_POST['passwd1'] != $_POST['passwd2']) {
			$errors[] = array("ERROR", _("Passwords are different!"));
		}
		else {
			// set new password
			$conf->set_Passwd($_POST['passwd1']);
		}
	}

	return $errors;
}

?>

