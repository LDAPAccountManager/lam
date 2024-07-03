<?php
namespace LAM\TOOLS\PROFILE_EDITOR;
use \htmlResponsiveRow;
use \htmlTitle;
use \htmlResponsiveInputField;
use \htmlResponsiveSelect;
use \htmlButton;
use \htmlHiddenInput;
use \htmlSubTitle;
use LAM\PROFILES\AccountProfilePersistenceManager;
use LAMException;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2024  Roland Gruber

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

$accountProfilePersistenceManager = new AccountProfilePersistenceManager();

$errors = [];

// save button was pressed
if (isset($_POST['save'])) {
	// create option array to check and save
	$options = [];
	$opt_keys = array_keys($_SESSION['profile_types']);
	foreach ($opt_keys as $element) {
		// text fields
		if ($_SESSION['profile_types'][$element] == "text") {
			$options[$element] = [$_POST[$element]];
		}
		// checkboxes
		elseif ($_SESSION['profile_types'][$element] == "checkbox") {
			if (isset($_POST[$element]) && ($_POST[$element] == "on")) $options[$element] = ['true'];
			else $options[$element] = ['false'];
		}
		// dropdownbox
		elseif ($_SESSION['profile_types'][$element] == "select") {
			$options[$element] = [$_POST[$element]];
		}
		// multiselect
		elseif ($_SESSION['profile_types'][$element] == "multiselect") {
			if (isset($_POST[$element])) $options[$element] = $_POST[$element];  // value is already an array
			else $options[$element] = [];
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
        try {
	        $accountProfilePersistenceManager->writeAccountProfile($_POST['accounttype'], $_POST['profname'], $_SESSION['config']->getName(), $options);
	        metaRefresh('profilemain.php?savedSuccessfully=' . $_POST['profname']);
	        exit();
        }
        catch (LAMException $e) {
            logNewMessage(LOG_ERR, $e->getTitle());
	        $errors[] = ["ERROR", _("Unable to save profile!"), $_POST['profname']];
        }
	}
}

// print header
include __DIR__ . '/../../lib/adminHeader.inc';
echo '<div class="smallPaddingContent">';

// print error messages if any
if (sizeof($errors) > 0) {
	echo "<br>\n";
	foreach ($errors as $error) {
		call_user_func_array(StatusMessage(...), $error);
	}
}

// empty list of attribute types
$_SESSION['profile_types'] = [];

// get module options
$options = getProfileOptions($type->getId());

// load old profile or POST values if needed
$old_options = [];
if (isset($_POST['save'])) {
	foreach ($_POST as $key => $value) {
		if (!is_array($value)) {
			$old_options[$key] = [$value];
		}
		else {
			$old_options[$key] = $value;
		}
	}
}
elseif (isset($_GET['edit'])) {
	try {
		$old_options = $accountProfilePersistenceManager->loadAccountProfile($type->getId(), $_GET['edit'], $_SESSION['config']->getName());
	} catch (LAMException $e) {
	    StatusMessage('ERROR', $e->getTitle(), $e->getMessage());
	}
}

// display formular
echo "<form id=\"profilepage\" action=\"profilepage.php?type=" . $type->getId() . "\" method=\"post\">\n";
echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';

$profName = '';
if (isset($_GET['edit'])) {
	$profName = $_GET['edit'];
}

$container = new htmlResponsiveRow();
$container->add(new htmlTitle(_("Profile editor")), 12);

// general options
$container->add(new htmlSubTitle(_("General settings"), '../../graphics/logo32.png', null, true), 12);
$profileNameField = new htmlResponsiveInputField(_("Profile name"), 'profname', $profName, '360', true);
$profileNameField->setTransient(true);
$container->add($profileNameField, 12);
$container->addVerticalSpacer('1rem');
// suffix box
// get root suffix
$rootsuffix = $type->getSuffix();
// get subsuffixes
$suffixes = ['-' => '-'];
$possibleSuffixes = $type->getSuffixList();
foreach ($possibleSuffixes as $suffix) {
	$suffixes[getAbstractDN($suffix)] = $suffix;
}
$selectedSuffix = [];
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
$selectedRDN = [];
if (isset($old_options['ldap_rdn'][0])) {
	$selectedRDN[] = $old_options['ldap_rdn'][0];
}
$rdnSelect = new htmlResponsiveSelect('ldap_rdn', $rdns, $selectedRDN, _("RDN identifier"), '301');
$rdnSelect->setSortElements(false);
$container->add($rdnSelect);

$container->addVerticalSpacer('2rem');

$_SESSION['profile_types'] = parseHtml(null, $container, $old_options, false, $type->getScope());

// display module options
foreach ($options as $moduleName => $moduleOptions) {
	// ignore modules without options
	if (empty($moduleOptions)) {
		continue;
	}
	$module = new $moduleName($type->getScope());
	$icon = $module->getIcon();
	if (!empty($icon) && !(str_starts_with($icon, 'http')) && !(str_starts_with($icon, '/'))) {
		$icon = '../../graphics/' . $icon;
	}
	$modContainer = new htmlResponsiveRow();
	$modContainer->add(new htmlSubTitle(getModuleAlias($moduleName, $type->getScope()), $icon, null, true), 12);
	$modContainer->add($moduleOptions, 12);
	$modContainer->addVerticalSpacer('2rem');
	$_SESSION['profile_types'] = array_merge($_SESSION['profile_types'], parseHtml($moduleName, $modContainer, $old_options, false, $type->getScope()));
}

// profile name and submit/abort buttons
$buttonTable = new htmlResponsiveRow();
$saveButton = new htmlButton('save', _('Save'));
$saveButton->setCSSClasses(['lam-primary fullwidth-mobile-only']);
$buttonTable->addLabel($saveButton);
$cancelButton = new htmlButton('abort', _('Cancel'));
$cancelButton->setCSSClasses(['fullwidth-mobile-only']);
$cancelButton->disableFormValidation();
$buttonTable->addField($cancelButton);
$buttonTable->add(new htmlHiddenInput('accounttype', $type->getId()), 0);

$_SESSION['profile_types'] = array_merge($_SESSION['profile_types'], parseHtml(null, $buttonTable, $old_options, false, $type->getScope()));

?>
</form>
</div>
<?php
include __DIR__ . '/../../lib/adminFooter.inc';

?>
