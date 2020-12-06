<?php
namespace LAM\CONFIG;
use \LAM\LIB\TWO_FACTOR\TwoFactorProviderService;
use \LAMConfig;
use \htmlTable;
use \htmlAccordion;
use \htmlSpacer;
use \DateTimeZone;
use \htmlStatusMessage;
use \htmlOutputText;
use \htmlInputCheckbox;
use \htmlHelpLink;
use \htmlElement;
use \htmlSubTitle;
use \htmlButton;
use \htmlResponsiveRow;
use \htmlResponsiveInputField;
use \htmlResponsiveSelect;
use \htmlResponsiveInputCheckbox;
use \htmlResponsiveInputTextarea;
use \htmlGroup;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2020  Roland Gruber

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
include_once(__DIR__ . "/../../lib/config.inc");
/** access to module settings */
include_once(__DIR__ . "/../../lib/modules.inc");
/** access to tools */
include_once(__DIR__ . "/../../lib/tools.inc");
/** 2-factor */
include_once __DIR__ . '/../../lib/2factor.inc';
/** common functions */
include_once __DIR__ . '/../../lib/configPages.inc';

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

// get password
if (isset($_POST['passwd'])) {
	$passwd = $_POST['passwd'];
}

// check if password was entered
// if not: load login page
if (!isset($passwd) && !(isset($_SESSION['conf_isAuthenticated']) && isset($_SESSION['conf_config']))) {
	$_SESSION['conf_message'] = new htmlStatusMessage('ERROR', _("No password was entered!"));
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
		if (substr($sessionKeys[$i], 0, 5) == "conf_") {
			unset($_SESSION[$sessionKeys[$i]]);
		}
	}
	$_SESSION['conf_message'] = new htmlStatusMessage('ERROR', _("The password is invalid! Please try again."));
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
printHeaderContents(_("LDAP Account Manager Configuration"), '../..');
echo "<body class=\"admin\">\n";
// include all JavaScript files
printJsIncludes('../..');
printConfigurationPageHeaderBar($conf);

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
echo "<form enctype=\"multipart/form-data\" action=\"confmain.php\" method=\"post\" autocomplete=\"off\">\n";

printConfigurationPageTabs(ConfigurationPageTab::GENERAL);

?>
<input type="text" name="hiddenPreventAutocomplete" autocomplete="false" class="hidden" value="">
<input type="password" name="hiddenPreventAutocompletePwd1" autocomplete="false" class="hidden" value="123">
<input type="password" name="hiddenPreventAutocompletePwd2" autocomplete="false" class="hidden" value="321">
<?php

$row = new htmlResponsiveRow();

$serverSettings = new htmlSubTitle(_("Server settings"), '../../graphics/profiles.png', null, true);
$row->add($serverSettings, 12);
// server URL
$urlInput = new htmlResponsiveInputField(_("Server address"), 'serverurl', $conf->get_ServerURL(), '201');
$urlInput->setRequired(true);
$row->add($urlInput, 12);
// use TLS
$tlsOptions = array(_("yes") => 'yes', _("no") => 'no');
$tlsSelect = new htmlResponsiveSelect('useTLS', $tlsOptions, array($conf->getUseTLS()), _("Activate TLS"), '201');
$tlsSelect->setHasDescriptiveElements(true);
$row->add($tlsSelect, 12);
// tree suffix
$row->add(new htmlResponsiveInputField(_("Tree suffix"), 'sufftree', $conf->get_Suffix('tree'), '203'), 12);
// LDAP search limit
$searchLimitOptions = array(
'-' => 0,		100 => 100,		500 => 500,
1000 => 1000,	5000 => 5000,	10000 => 10000,
50000 => 50000,	100000 => 100000);
$limitSelect = new htmlResponsiveSelect('searchLimit', $searchLimitOptions, array($conf->get_searchLimit()), _("LDAP search limit"), '222');
$limitSelect->setHasDescriptiveElements(true);
$row->add($limitSelect, 12);
// DN part to hide
$urlInput = new htmlResponsiveInputField(_("DN part to hide"), 'hideDnPart', $conf->getHideDnPart(), '292');
$row->add($urlInput, 12);

// access level is only visible in Pro version
if (isLAMProVersion()) {
	$accessOptions = array(
		_('Write access') => LAMConfig::ACCESS_ALL,
		_('Change passwords') => LAMConfig::ACCESS_PASSWORD_CHANGE,
		_('Read-only') => LAMConfig::ACCESS_READ_ONLY
	);
	$accessSelect = new htmlResponsiveSelect('accessLevel', $accessOptions, array($conf->getAccessLevel()), _("Access level"), '215');
	$accessSelect->setHasDescriptiveElements(true);
	$row->add($accessSelect, 12);
}

// advanced options
$advancedOptionsContent = new htmlResponsiveRow();
// display name
$advancedOptionsContent->add(new htmlResponsiveInputField(_('Display name'), 'serverDisplayName', $conf->getServerDisplayName(), '268'), 12);
// referrals
$followReferrals = ($conf->getFollowReferrals() === 'true');
$advancedOptionsContent->add(new htmlResponsiveInputCheckbox('followReferrals', $followReferrals , _('Follow referrals'), '205'), 12);
// paged results
$pagedResults = ($conf->getPagedResults() === 'true');
$advancedOptionsContent->add(new htmlResponsiveInputCheckbox('pagedResults', $pagedResults , _('Paged results'), '266'), 12);
// referential integrity overlay
$referentialIntegrity = ($conf->isReferentialIntegrityOverlayActive());
$advancedOptionsContent->add(new htmlResponsiveInputCheckbox('referentialIntegrityOverlay', $referentialIntegrity , _('Referential integrity overlay'), '269'), 12);
// hide password prompt for expired passwords
$hidePasswordPromptForExpiredPasswords = ($conf->isHidePasswordPromptForExpiredPasswords());
$advancedOptionsContent->add(new htmlResponsiveInputCheckbox('hidePasswordPromptForExpiredPasswords', $hidePasswordPromptForExpiredPasswords, _('Hide password prompt for expired password'), '291'), 12);

// build advanced options box
$advancedOptions = new htmlAccordion('advancedOptions_server', array(_('Advanced options') => $advancedOptionsContent), false);
$advancedOptions->colspan = 15;
$row->add($advancedOptions, 12);

$row->addVerticalSpacer('2rem');

// language
$row->add(new htmlSubTitle(_("Language settings"), '../../graphics/language.png', null, true), 12);
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
	$languageSelect = new htmlResponsiveSelect('lang', $languages, $defaultLanguage, _("Default language"), '209');
	$languageSelect->setHasDescriptiveElements(true);
	$row->add($languageSelect, 12);
}
else {
	$row->add(new htmlStatusMessage('ERROR', "Unable to load available languages. Setting English as default language."), 12);
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
$row->add(new htmlResponsiveSelect('timeZone', $timezones, array($conf->getTimeZone()), _('Time zone'), '213'), 12);

$row->addVerticalSpacer('2rem');

// lamdaemon settings
$row->add(new htmlSubTitle(_("Lamdaemon settings"), '../../graphics/lamdaemon.png', null, true), 12);
$row->add(new htmlResponsiveInputField(_("Server list"), 'scriptservers', $conf->get_scriptServers(), '218'), 12);
$row->add(new htmlResponsiveInputField(_("Path to external script"), 'scriptpath', $conf->get_scriptPath(), '210'), 12);

$row->add(new htmlResponsiveInputField(_('User name'), 'scriptuser', $conf->getScriptUserName(), '284'), 12);
$row->add(new htmlResponsiveInputField(_('SSH key file'), 'scriptkey', $conf->getScriptSSHKey(), '285'), 12);
$sshKeyPassword = new htmlResponsiveInputField(_('SSH key password'), 'scriptkeypassword', $conf->getScriptSSHKeyPassword(), '286');
$sshKeyPassword->setIsPassword(true);
$row->add($sshKeyPassword, 12);

$row->addVerticalSpacer('0.5rem');
$lamdaemonRightsLabel = new htmlGroup();
$lamdaemonRightsLabel->addElement(new htmlOutputText(_("Rights for the home directory")));
$lamdaemonRightsLabel->addElement(new htmlSpacer('0.2rem', null));
$lamdaemonRightsLabel->addElement(new htmlHelpLink('219'));
$row->addLabel($lamdaemonRightsLabel, 12, 6);
$chmod = $conf->get_scriptRights();
$rightsTable = new htmlTable();
$rightsTable->setCSSClasses(array('padding5'));
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
$row->addField($rightsTable, 12, 6);

$row->addVerticalSpacer('2rem');

// LAM Pro settings
if (isLAMProVersion()) {
	// password reset page
	$row->add(new htmlSubTitle(_("Password reset page settings"), '../../graphics/keyBig.png', null, true), 12);

	$pwdResetAllowSpecific = true;
	if ($conf->getPwdResetAllowSpecificPassword() == 'false') {
		$pwdResetAllowSpecific = false;
	}
	$row->add(new htmlResponsiveInputCheckbox('pwdResetAllowSpecificPassword', $pwdResetAllowSpecific , _('Allow setting specific passwords'), '280'), 12);

	$pwdResetAllowScreenPassword = true;
	if ($conf->getPwdResetAllowScreenPassword() == 'false') {
		$pwdResetAllowScreenPassword = false;
	}
	$row->add(new htmlResponsiveInputCheckbox('pwdResetAllowScreenPassword', $pwdResetAllowScreenPassword , _('Allow to display password on screen'), '281'), 12);

	$pwdResetForcePasswordChange = true;
	if ($conf->getPwdResetForcePasswordChange() == 'false') {
		$pwdResetForcePasswordChange = false;
	}
	$row->add(new htmlResponsiveInputCheckbox('pwdResetForcePasswordChange', $pwdResetForcePasswordChange , _('Force password change by default'), '283'), 12);

	$pwdResetDefaultPasswordOutputOptions = array(
		_('Display on screen') => LAMConfig::PWDRESET_DEFAULT_SCREEN,
		_('Send via mail') => LAMConfig::PWDRESET_DEFAULT_MAIL,
		_('Both') => LAMConfig::PWDRESET_DEFAULT_BOTH
	);
	$pwdResetDefaultPasswordOutputSelect = new htmlResponsiveSelect('pwdResetDefaultPasswordOutput', $pwdResetDefaultPasswordOutputOptions, array($conf->getPwdResetDefaultPasswordOutput()), _("Default password output"), '282');
	$pwdResetDefaultPasswordOutputSelect->setHasDescriptiveElements(true);
	$row->add($pwdResetDefaultPasswordOutputSelect, 12);

	$row->addVerticalSpacer('2rem');

	// mail settings
	$row->add(new htmlSubTitle(_("Password mail settings"), '../../graphics/mailBig.png', null, true), 12);

	$pwdMailFrom = new htmlResponsiveInputField(_('From address'), 'pwdResetMail_from', $conf->getLamProMailFrom(), '550', true);
	$row->add($pwdMailFrom, 12);

	$pwdMailReplyTo = new htmlResponsiveInputField(_('Reply-to address'), 'pwdResetMail_replyTo', $conf->getLamProMailReplyTo(), '554');
	$row->add($pwdMailReplyTo, 12);

	$pwdMailSubject = new htmlResponsiveInputField(_('Subject'), 'pwdResetMail_subject', $conf->getLamProMailSubject(), '551');
	$row->add($pwdMailSubject, 12);

	$pwdMailIsHTML = false;
	if ($conf->getLamProMailIsHTML() == 'true') {
		$pwdMailIsHTML = true;
	}
	$row->add(new htmlResponsiveInputCheckbox('pwdResetMail_isHTML',$pwdMailIsHTML , _('HTML format'), '553'), 12);

	$pwdMailAllowAlternate = true;
	if ($conf->getLamProMailAllowAlternateAddress() == 'false') {
		$pwdMailAllowAlternate = false;
	}
	$row->add(new htmlResponsiveInputCheckbox('pwdResetMail_allowAlternate',$pwdMailAllowAlternate , _('Allow alternate address'), '555'), 12);

	$pwdMailBody = new htmlResponsiveInputTextarea('pwdResetMail_body', $conf->getLamProMailText(), 50, 4, _('Text'), '552');
	$row->add($pwdMailBody, 12);

	$row->addVerticalSpacer('2rem');
}

// tool settings
$row->add(new htmlSubTitle(_("Tool settings"), '../../graphics/bigTools.png',null, true), 12);
$toolSettings = $conf->getToolSettings();
$row->add(new htmlOutputText(_('Hidden tools')), 12);
$row->addVerticalSpacer('0.5rem');
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
$toolSettingsContent = new htmlResponsiveRow();
$toolsSize = sizeof($tools);
for ($r = 0; $r < $toolsSize; $r++) {
	$tool = $tools[$r];
	$toolClass = get_class($tool);
	$toolName = substr($toolClass, strrpos($toolClass, '\\') + 1);
	$selected = false;
	if (isset($toolSettings['tool_hide_' . $toolName]) && ($toolSettings['tool_hide_' . $toolName] === 'true')) {
		$selected = true;
	}
	$toolSettingsContent->add(new htmlResponsiveInputCheckbox('tool_hide_' . $toolName, $selected, $tool->getName(), null, false), 12, 4);
}
for ($i = $toolsSize % 3; $i < 3; $i++) {
	$toolSettingsContent->add(new htmlOutputText(''), 0, 4);
}
$row->add($toolSettingsContent, 12);

$row->addVerticalSpacer('2rem');

// security setings
$row->add(new htmlSubTitle(_("Security settings"), '../../graphics/security.png', null, true), 12);
// login method
$loginOptions = array(
	_('Fixed list') => LAMConfig::LOGIN_LIST,
	_('LDAP search') => LAMConfig::LOGIN_SEARCH
);
$loginSelect = new htmlResponsiveSelect('loginMethod', $loginOptions, array($conf->getLoginMethod()), _("Login method"), '220');
$loginSelect->setHasDescriptiveElements(true);
$loginSelect->setTableRowsToHide(array(
	LAMConfig::LOGIN_LIST => array('loginSearchSuffix', 'loginSearchFilter', 'loginSearchDN', 'loginSearchPassword', 'httpAuthentication'),
	LAMConfig::LOGIN_SEARCH => array('admins')
));
$loginSelect->setTableRowsToShow(array(
	LAMConfig::LOGIN_LIST => array('admins'),
	LAMConfig::LOGIN_SEARCH => array('loginSearchSuffix', 'loginSearchFilter', 'loginSearchDN', 'loginSearchPassword', 'httpAuthentication')
));
$row->add($loginSelect, 12);
// admin list
$adminText = implode("\n", explode(";", $conf->get_Adminstring()));
$adminTextInput = new htmlResponsiveInputTextarea('admins', $adminText, '50', '3', _("List of valid users"), '207');
$adminTextInput->setRequired(true);
$row->add($adminTextInput, 12);
// search suffix
$searchSuffixInput = new htmlResponsiveInputField(_("LDAP suffix"), 'loginSearchSuffix', $conf->getLoginSearchSuffix(), '221');
$searchSuffixInput->setRequired(true);
$row->add($searchSuffixInput, 12);
// login search filter
$searchFilterInput = new htmlResponsiveInputField(_("LDAP filter"), 'loginSearchFilter', $conf->getLoginSearchFilter(), '221');
$searchFilterInput->setRequired(true);
$row->add($searchFilterInput, 12);
// login search bind user
$row->add(new htmlResponsiveInputField(_("Bind user"), 'loginSearchDN', $conf->getLoginSearchDN(), '224'), 12);
// login search bind password
$searchPasswordInput = new htmlResponsiveInputField(_("Bind password"), 'loginSearchPassword', $conf->getLoginSearchPassword(), '224');
$searchPasswordInput->setIsPassword(true);
$row->add($searchPasswordInput, 12);
// HTTP authentication
$row->add(new htmlResponsiveInputCheckbox('httpAuthentication', ($conf->getHttpAuthentication() == 'true'), _('HTTP authentication'), '223'), 12);
$row->addVerticalSpacer('1rem');

// 2factor authentication
if (extension_loaded('curl')) {
	$row->add(new htmlSubTitle(_("2-factor authentication"), '../../graphics/lock.png'), 12);
	$twoFactorOptions = array(
			_('None') => TwoFactorProviderService::TWO_FACTOR_NONE,
			'privacyIDEA' => TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA,
			'YubiKey' => TwoFactorProviderService::TWO_FACTOR_YUBICO,
			'Duo' => TwoFactorProviderService::TWO_FACTOR_DUO,
    		'Okta' => TwoFactorProviderService::TWO_FACTOR_OKTA,
            'WebAuthn' => TwoFactorProviderService::TWO_FACTOR_WEBAUTHN
	);
	$twoFactorSelect = new htmlResponsiveSelect('twoFactor', $twoFactorOptions, array($conf->getTwoFactorAuthentication()), _('Provider'), '514');
	$twoFactorSelect->setHasDescriptiveElements(true);
	$twoFactorSelect->setTableRowsToHide(array(
		TwoFactorProviderService::TWO_FACTOR_NONE => array('twoFactorURL', 'twoFactorURLs', 'twoFactorInsecure', 'twoFactorLabel',
			'twoFactorOptional', 'twoFactorCaption', 'twoFactorClientId', 'twoFactorSecretKey', 'twoFactorAttribute', 'twoFactorDomain'),
		TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA => array('twoFactorURLs', 'twoFactorClientId', 'twoFactorSecretKey', 'twoFactorDomain'),
		TwoFactorProviderService::TWO_FACTOR_YUBICO => array('twoFactorURL', 'twoFactorAttribute', 'twoFactorDomain'),
		TwoFactorProviderService::TWO_FACTOR_DUO => array('twoFactorURLs', 'twoFactorOptional', 'twoFactorInsecure', 'twoFactorLabel', 'twoFactorDomain'),
		TwoFactorProviderService::TWO_FACTOR_OKTA => array('twoFactorURLs', 'twoFactorOptional', 'twoFactorInsecure', 'twoFactorLabel', 'twoFactorDomain'),
		TwoFactorProviderService::TWO_FACTOR_WEBAUTHN => array('twoFactorURL', 'twoFactorURLs', 'twoFactorInsecure', 'twoFactorLabel',
			'twoFactorCaption', 'twoFactorClientId', 'twoFactorSecretKey', 'twoFactorAttribute'),
	));
	$twoFactorSelect->setTableRowsToShow(array(
		TwoFactorProviderService::TWO_FACTOR_PRIVACYIDEA => array('twoFactorURL', 'twoFactorInsecure', 'twoFactorLabel',
			'twoFactorOptional', 'twoFactorCaption', 'twoFactorAttribute'),
		TwoFactorProviderService::TWO_FACTOR_YUBICO => array('twoFactorURLs', 'twoFactorInsecure', 'twoFactorLabel',
			'twoFactorOptional', 'twoFactorCaption', 'twoFactorClientId', 'twoFactorSecretKey'),
		TwoFactorProviderService::TWO_FACTOR_DUO => array('twoFactorURL', 'twoFactorLabel',
			'twoFactorCaption', 'twoFactorClientId', 'twoFactorSecretKey', 'twoFactorAttribute'),
		TwoFactorProviderService::TWO_FACTOR_OKTA => array('twoFactorURL', 'twoFactorLabel',
			'twoFactorCaption', 'twoFactorClientId', 'twoFactorSecretKey', 'twoFactorAttribute'),
		TwoFactorProviderService::TWO_FACTOR_WEBAUTHN => array('twoFactorDomain', 'twoFactorOptional')
	));
	$row->add($twoFactorSelect, 12);
	$twoFactorAttribute = new htmlResponsiveInputField(_("User name attribute"), 'twoFactorAttribute', $conf->getTwoFactorAuthenticationAttribute(), '528');
	$row->add($twoFactorAttribute, 12);
	$twoFactorUrl = new htmlResponsiveInputField(_("Base URL"), 'twoFactorURL', $conf->getTwoFactorAuthenticationURL(), '515');
	$twoFactorUrl->setRequired(true);
	$row->add($twoFactorUrl, 12);
	$twoFactorUrl = new htmlResponsiveInputTextarea('twoFactorURLs', $conf->getTwoFactorAuthenticationURL(), '80', '4', _("Base URLs"), '515a');
	$twoFactorUrl->setRequired(true);
	$row->add($twoFactorUrl, 12);
	$twoFactorClientId = new htmlResponsiveInputField(_("Client id"), 'twoFactorClientId', $conf->getTwoFactorAuthenticationClientId(), '524');
	$row->add($twoFactorClientId, 12);
	$twoFactorSecretKey = new htmlResponsiveInputField(_("Secret key"), 'twoFactorSecretKey', $conf->getTwoFactorAuthenticationSecretKey(), '528');
	$row->add($twoFactorSecretKey, 12);
	$twoFactorDomain = new htmlResponsiveInputField(_("Domain"), 'twoFactorDomain', $conf->getTwoFactorAuthenticationDomain(), '529');
	$row->add($twoFactorDomain, 12);
	$twoFactorLabel = new htmlResponsiveInputField(_("Label"), 'twoFactorLabel', $conf->getTwoFactorAuthenticationLabel(), '517');
	$row->add($twoFactorLabel, 12);
	$row->add(new htmlResponsiveInputCheckbox('twoFactorOptional', $conf->getTwoFactorAuthenticationOptional(), _('Optional'), '519'), 12);
	$row->add(new htmlResponsiveInputCheckbox('twoFactorInsecure', $conf->getTwoFactorAuthenticationInsecure(), _('Disable certificate check'), '516'), 12);
	$twoFactorCaption = new htmlResponsiveInputTextarea('twoFactorCaption', $conf->getTwoFactorAuthenticationCaption(), '80', '4', _("Caption"));
	$twoFactorCaption->setIsRichEdit(true);
	$twoFactorCaption->alignment = htmlElement::ALIGN_TOP;
	$row->add($twoFactorCaption, 12);
}

// new password
$row->add(new htmlSubTitle(_("Profile password"), '../../graphics/keyBig.png', null, true), 12);
$password1 = new htmlResponsiveInputField(_("New password"), 'passwd1', null, '212');
$password1->setIsPassword(true, false, true);
$password2 = new htmlResponsiveInputField(_("Reenter password"), 'passwd2');
$password2->setIsPassword(true, false, true);
$password2->setSameValueFieldID('passwd1');
$row->add($password1, 12);
$row->add($password2, 12);

$row->addVerticalSpacer('2rem');

parseHtml(null, $row, array(), false, $tabindex, 'user');

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
	if (isset($_POST['referentialIntegrityOverlay']) && ($_POST['referentialIntegrityOverlay'] == 'on')) {
		$conf->setReferentialIntegrityOverlay('true');
	}
	else {
		$conf->setReferentialIntegrityOverlay('false');
	}
    if (isset($_POST['hidePasswordPromptForExpiredPasswords']) && ($_POST['hidePasswordPromptForExpiredPasswords'] == 'on')) {
        $conf->setHidePasswordPromptForExpiredPasswords('true');
    }
    else {
        $conf->setHidePasswordPromptForExpiredPasswords('false');
    }
	/*	if (!$conf->set_cacheTimeout($_POST['cachetimeout'])) {
			$errors[] = array("ERROR", _("Cache timeout is invalid!"));
		}*/
	$conf->set_searchLimit($_POST['searchLimit']);
	$conf->setHideDnPart($_POST['hideDnPart']);
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
		if (!empty($_POST['pwdResetMail_subject']) && empty($_POST['pwdResetMail_from'])) {
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
	if (isset($_POST['chmod_owr']) && ($_POST['chmod_owr'] == 'on')) {
		$chmodOwner += 4;
	}
	if (isset($_POST['chmod_oww']) && ($_POST['chmod_oww'] == 'on')) {
		$chmodOwner += 2;
	}
	if (isset($_POST['chmod_owe']) && ($_POST['chmod_owe'] == 'on')) {
		$chmodOwner += 1;
	}
	if (isset($_POST['chmod_grr']) && ($_POST['chmod_grr'] == 'on')) {
		$chmodGroup += 4;
	}
	if (isset($_POST['chmod_grw']) && ($_POST['chmod_grw'] == 'on')) {
		$chmodGroup += 2;
	}
	if (isset($_POST['chmod_gre']) && ($_POST['chmod_gre'] == 'on')) {
		$chmodGroup += 1;
	}
	if (isset($_POST['chmod_otr']) && ($_POST['chmod_otr'] == 'on')) {
		$chmodOther += 4;
	}
	if (isset($_POST['chmod_otw']) && ($_POST['chmod_otw'] == 'on')) {
		$chmodOther += 2;
	}
	if (isset($_POST['chmod_ote']) && ($_POST['chmod_ote'] == 'on')) {
		$chmodOther += 1;
	}
	$chmod = $chmodOwner . $chmodGroup . $chmodOther;
	if (!$conf->set_scriptrights($chmod)) {
		$errors[] = array("ERROR", _("Script rights are invalid!"));
	}
	$conf->setScriptUserName($_POST['scriptuser']);
	$conf->setScriptSSHKey($_POST['scriptkey']);
	$conf->setScriptSSHKeyPassword($_POST['scriptkeypassword']);
	if (!empty($_POST['scriptkey'])) {
		include_once '../../lib/remote.inc';
		$remote = new \LAM\REMOTE\Remote();
		try {
			$remote->loadKey($conf->getScriptSSHKey(), $conf->getScriptSSHKeyPassword());
		}
		catch (\LAMException $e) {
			$errors[] = array('ERROR', _('SSH key file'), $e->getTitle());
		}
	}
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
		if ($_POST['twoFactor'] === TwoFactorProviderService::TWO_FACTOR_YUBICO) {
			$conf->setTwoFactorAuthenticationURL($_POST['twoFactorURLs']);
		}
		else {
			$conf->setTwoFactorAuthenticationURL($_POST['twoFactorURL']);
		}
		$conf->setTwoFactorAuthenticationAttribute($_POST['twoFactorAttribute']);
		$conf->setTwoFactorAuthenticationClientId($_POST['twoFactorClientId']);
		$conf->setTwoFactorAuthenticationSecretKey($_POST['twoFactorSecretKey']);
		$conf->setTwoFactorAuthenticationDomain($_POST['twoFactorDomain']);
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

