<?php
namespace LAM\TOOLS\PROFILE_EDITOR;
use \htmlResponsiveRow;
use \htmlTitle;
use \htmlResponsiveInputField;
use \htmlResponsiveSelect;
use \htmlButton;
use \htmlHiddenInput;
use \htmlSubTitle;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2019  Roland Gruber

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
* Manages creating/changing of profiles.
*
* @package profiles
* @author Roland Gruber
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** helper functions for profiles */
include_once(__DIR__ . "/../../lib/profiles.inc");
/** access to LDAP server */
include_once(__DIR__ . "/../../lib/ldap.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");
/** access to account modules */
include_once(__DIR__ . "/../../lib/modules.inc");
/** Used to display status messages */
include_once(__DIR__ . "/../../lib/status.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolProfileEditor');

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// copy type and profile name from POST to GET
if (isset($_POST['profname'])) {
	$_GET['edit'] = $_POST['profname'];
}
if (isset($_POST['accounttype'])) {
	$_GET['type'] = $_POST['accounttype'];
}

$typeManager = new \LAM\TYPES\TypeManager();
$type = $typeManager->getConfiguredType($_GET['type']);
if ($type->isHidden() || !checkIfWriteAccessIsAllowed($_GET['type'])) {
	logNewMessage(LOG_ERR, 'User tried to access hidden account type profile: ' . $_GET['type']);
	die();
}

// abort button was pressed
// back to profile editor
if (isset($_POST['abort'])) {
	metaRefresh("profilemain.php");
	exit;
}

$errors = array();

// save button was presed
if (isset($_POST['save'])) {
	// create option array to check and save
	$options = array();
	$opt_keys = array_keys($_SESSION['profile_types']);
	foreach ($opt_keys as $element) {
		// text fields
		if ($_SESSION['profile_types'][$element] == "text") {
			$options[$element] = array($_POST[$element]);
		}
		// checkboxes
		elseif ($_SESSION['profile_types'][$element] == "checkbox") {
			if (isset($_POST[$element]) && ($_POST[$element] == "on")) $options[$element] = array('true');
			else $options[$element] = array('false');
		}
		// dropdownbox
		elseif ($_SESSION['profile_types'][$element] == "select") {
			$options[$element] = array($_POST[$element]);
		}
		// multiselect
		elseif ($_SESSION['profile_types'][$element] == "multiselect") {
			if (isset($_POST[$element])) $options[$element] = $_POST[$element];  // value is already an array
			else $options[$element] = array();
		}
		// textareas
		if ($_SESSION['profile_types'][$element] == "textarea") {
			$options[$element] = explode("\r\n", $_POST[$element]);
		}
	}

	// check options
	$errors = checkProfileOptions($_POST['accounttype'], $options);
	if (sizeof($errors) == 0) {  // input data is valid, save profile
		// save profile
		if (\LAM\PROFILES\saveAccountProfile($options, $_POST['profname'], $_POST['accounttype'])) {
			metaRefresh('profilemain.php?savedSuccessfully=' . $_POST['profname']);
			exit();
		}
		else {
			$errors[] = array("ERROR", _("Unable to save profile!"), $_POST['profname']);
		}
	}
}

// print header
include __DIR__ . '/../../lib/adminHeader.inc';
echo '<div class="user-bright smallPaddingContent">';

// print error messages if any
if (sizeof($errors) > 0) {
	echo "<br>\n";
	foreach ($errors as $error) {
		call_user_func_array('StatusMessage', $error);
	}
}

// empty list of attribute types
$_SESSION['profile_types'] = array();

// get module options
$options = getProfileOptions($type->getId());

// load old profile or POST values if needed
$old_options = array();
if (isset($_POST['save'])) {
	foreach ($_POST as $key => $value) {
		if (!is_array($value)) {
			$old_options[$key] = array($value);
		}
		else {
			$old_options[$key] = $value;
		}
	}
}
elseif (isset($_GET['edit'])) {
	$old_options = \LAM\PROFILES\loadAccountProfile($_GET['edit'], $type->getId());
}

// display formular
echo "<form id=\"profilepage\" action=\"profilepage.php?type=" . $type->getId() . "\" method=\"post\">\n";
echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';

$profName = '';
if (isset($_GET['edit'])) {
	$profName = $_GET['edit'];
}

$tabindex = 1;

$container = new htmlResponsiveRow();
$container->add(new htmlTitle(_("Profile editor")), 12);

// general options
$container->add(new htmlSubTitle(_("General settings"), '../../graphics/logo32.png', null, true), 12);
$container->add(new htmlResponsiveInputField(_("Profile name") . '*', 'profname', $profName, '360'), 12);
$container->addVerticalSpacer('1rem');
// suffix box
// get root suffix
$rootsuffix = $type->getSuffix();
// get subsuffixes
$suffixes = array('-' => '-');
$possibleSuffixes = $type->getSuffixList();
foreach ($possibleSuffixes as $suffix) {
	$suffixes[getAbstractDN($suffix)] = $suffix;
}
$selectedSuffix = array();
if (isset($old_options['ldap_suffix'][0])) {
	$selectedSuffix[] = $old_options['ldap_suffix'][0];
}
$suffixSelect = new htmlResponsiveSelect('ldap_suffix', $suffixes, $selectedSuffix, _("LDAP suffix"), '361');
$suffixSelect->setHasDescriptiveElements(true);
$suffixSelect->setSortElements(false);
$suffixSelect->setRightToLeftTextDirection(true);
$container->add($suffixSelect, 12);
// RDNs
$rdns = getRDNAttributes($type->getId());
$selectedRDN = array();
if (isset($old_options['ldap_rdn'][0])) {
	$selectedRDN[] = $old_options['ldap_rdn'][0];
}
$container->add(new htmlResponsiveSelect('ldap_rdn', $rdns, $selectedRDN, _("RDN identifier"), '301'), 12);

$container->addVerticalSpacer('2rem');

$_SESSION['profile_types'] = parseHtml(null, $container, $old_options, false, $tabindex, $type->getScope());

// display module options
foreach ($options as $moduleName => $moduleOptions) {
	// ignore modules without options
	if (empty($moduleOptions)) {
		continue;
	}
	$module = new $moduleName($type->getScope());
	$icon = $module->getIcon();
	if (!empty($icon) && !(strpos($icon, 'http') === 0) && !(strpos($icon, '/') === 0)) {
		$icon = '../../graphics/' . $icon;
	}
	$modContainer = new htmlResponsiveRow();
	$modContainer->add(new htmlSubTitle(getModuleAlias($moduleName, $type->getScope()), $icon, null, true), 12);
	$modContainer->add($moduleOptions, 12);
	$modContainer->addVerticalSpacer('2rem');
	$_SESSION['profile_types'] = array_merge($_SESSION['profile_types'], parseHtml($moduleName, $modContainer, $old_options, false, $tabindex, $type->getScope()));
}

// profile name and submit/abort buttons
$buttonTable = new htmlResponsiveRow();
$saveButton = new htmlButton('save', _('Save'));
$saveButton->setIconClass('saveButton');
$buttonTable->addLabel($saveButton);
$cancelButton = new htmlButton('abort', _('Cancel'));
$cancelButton->setIconClass('cancelButton');
$buttonTable->addField($cancelButton);
$buttonTable->add(new htmlHiddenInput('accounttype', $type->getId()), 0);

$_SESSION['profile_types'] = array_merge($_SESSION['profile_types'], parseHtml(null, $buttonTable, $old_options, false, $tabindex, $type->getScope()));

?>
<script type="text/javascript">
	jQuery("#profilepage").validationEngine({promptPosition: "topLeft", addFailureCssClassToField: "lam-input-error", autoHidePrompt: true, autoHideDelay: 5000});
</script>
</form>
</div>
<?php
include __DIR__ . '/../../lib/adminFooter.inc';

?>
