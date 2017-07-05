<?php
namespace LAM\TOOLS\PROFILE_EDITOR;
use \htmlTable;
use \htmlTitle;
use \htmlStatusMessage;
use \LAMCfgMain;
use \htmlSubTitle;
use \htmlSpacer;
use \htmlSelect;
use \htmlButton;
use \htmlImage;
use \htmlLink;
use \htmlOutputText;
use \htmlHelpLink;
use \htmlHiddenInput;
use \htmlInputField;
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
* This is the main window of the profile editor.
*
* @package profiles
* @author Roland Gruber
*/

/** security functions */
include_once("../../lib/security.inc");
/** helper functions for profiles */
include_once("../../lib/profiles.inc");
/** access to LDAP server */
include_once("../../lib/ldap.inc");
/** access to configuration options */
include_once("../../lib/config.inc");

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

$typeManager = new \LAM\TYPES\TypeManager();
$types = $typeManager->getConfiguredTypes();
$profileClasses = array();
$profileClassesTemp = array();
foreach ($types as $type) {
	if ($type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
		continue;
	}
	$profileClassesTemp[$type->getAlias()] = array(
		'typeId' => $type->getId(),
		'scope' => $type->getScope(),
		'title' => $type->getAlias(),
		'icon' => $type->getIcon(),
		'profiles' => "");
}
$profileClassesKeys = array_keys($profileClassesTemp);
natcasesort($profileClassesKeys);
$profileClassesKeys = array_values($profileClassesKeys);
for ($i = 0; $i < sizeof($profileClassesKeys); $i++) {
	$profileClasses[] = $profileClassesTemp[$profileClassesKeys[$i]];
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// check if new profile should be created
elseif (isset($_POST['createProfileButton'])) {
	metaRefresh("profilepage.php?type=" . htmlspecialchars($_POST['createProfile']));
	exit;
}
// check if a profile should be edited
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	if (isset($_POST['editProfile_' . $profileClasses[$i]['typeId']]) || isset($_POST['editProfile_' . $profileClasses[$i]['typeId'] . '_x'])) {
		metaRefresh("profilepage.php?type=" . htmlspecialchars($profileClasses[$i]['typeId']) .
					"&amp;edit=" . htmlspecialchars($_POST['profile_' . $profileClasses[$i]['typeId']]));
		exit;
	}
}

include '../main_header.php';
echo "<div class=\"user-bright smallPaddingContent\">\n";
echo "<form name=\"profilemainForm\" action=\"profilemain.php\" method=\"post\">\n";
echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';

$container = new htmlTable();
$container->addElement(new htmlTitle(_("Profile editor")), true);

if (isset($_POST['deleteProfile']) && ($_POST['deleteProfile'] == 'true')) {
	$type = $typeManager->getConfiguredType($_POST['profileDeleteType']);
	if ($type->isHidden()) {
		logNewMessage(LOG_ERR, 'User tried to delete hidden account type profile: ' . $_POST['profileDeleteType']);
		die();
	}
	// delete profile
	if (\LAM\PROFILES\delAccountProfile($_POST['profileDeleteName'], $_POST['profileDeleteType'])) {
		$message = new htmlStatusMessage('INFO', _('Deleted profile.'), $type->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$message->colspan = 10;
		$container->addElement($message, true);
	}
	else {
		$message = new htmlStatusMessage('ERROR', _('Unable to delete profile!'), $type->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$message->colspan = 10;
		$container->addElement($message, true);
	}
}

$configProfiles = getConfigProfiles();
$serverProfiles = array();
foreach ($configProfiles as $profileName) {
	$serverProfiles[$profileName] = new \LAMConfig($profileName);
}

// import profiles
if (!empty($_POST['import'])) {
	$cfg = new LAMCfgMain();
	// check master password
	$errMessage = null;
	if (!$cfg->checkPassword($_POST['passwd_i_' . $_POST['typeId']])) {
		$errMessage = new htmlStatusMessage('ERROR', _('Master password is wrong!'));
	}
	elseif (!empty($_POST['importProfiles'])) {
		$options = array();
		foreach ($_POST['importProfiles'] as $importProfiles) {
			$parts = explode('##', $importProfiles);
			$options[] = array('conf' => $parts[0], 'typeId' => $parts[1], 'name' => $parts[2]);
		}
		$errMessage = importProfiles($_POST['typeId'], $options, $serverProfiles, $typeManager);
	}
	if ($errMessage != null) {
		$errMessage->colspan = 10;
		$container->addElement($errMessage, true);
	}
}
// export profiles
if (!empty($_POST['export'])) {
	$cfg = new LAMCfgMain();
	// check master password
	$errMessage = null;
	if (!$cfg->checkPassword($_POST['passwd_e_' . $_POST['typeId']])) {
		$errMessage = new htmlStatusMessage('ERROR', _('Master password is wrong!'));
	}
	elseif (!empty($_POST['exportProfiles'])) {
		$options = array();
		foreach ($_POST['exportProfiles'] as $importProfiles) {
			$parts = explode('##', $importProfiles);
			$options[] = array('conf' => $parts[0], 'typeId' => $parts[1]);
		}
		$typeId = $_POST['typeId'];
		$name = $_POST['name_' . $typeId];
		$errMessage = exportProfiles($typeId, $name, $options, $serverProfiles, $typeManager);
	}
	if ($errMessage != null) {
		$errMessage->colspan = 10;
		$container->addElement($errMessage, true);
	}
}

// get list of profiles for each account type
for ($i = 0; $i < sizeof($profileClasses); $i++) {
	$profileList = \LAM\PROFILES\getAccountProfiles($profileClasses[$i]['typeId']);
	natcasesort($profileList);
	$profileClasses[$i]['profiles'] = $profileList;
}

if (isset($_GET['savedSuccessfully'])) {
	$message = new htmlStatusMessage("INFO", _("Profile was saved."), htmlspecialchars($_GET['savedSuccessfully']));
	$message->colspan = 10;
	$container->addElement($message, true);
}

// new profile
if (!empty($profileClasses)) {
	$container->addElement(new htmlSubTitle(_('Create a new profile')), true);
	$sortedTypes = array();
	for ($i = 0; $i < sizeof($profileClasses); $i++) {
		$sortedTypes[$profileClasses[$i]['title']] = $profileClasses[$i]['typeId'];
	}
	natcasesort($sortedTypes);
	$newContainer = new htmlTable();
	$newProfileSelect = new htmlSelect('createProfile', $sortedTypes);
	$newProfileSelect->setHasDescriptiveElements(true);
	$newProfileSelect->setWidth('15em');
	$newContainer->addElement($newProfileSelect);
	$newContainer->addElement(new htmlSpacer('10px', null));
	$newContainer->addElement(new htmlButton('createProfileButton', _('Create')), true);
	$container->addElement($newContainer, true);
}

$container->addElement(new htmlSpacer(null, '10px'), true);

// existing profiles
$container->addElement(new htmlSubTitle(_('Manage existing profiles')), true);
$existingContainer = new htmlTable();
$existingContainer->colspan = 5;

for ($i = 0; $i < sizeof($profileClasses); $i++) {
	if ($i > 0) {
		$existingContainer->addElement(new htmlSpacer(null, '10px'), true);
	}

	$existingContainer->addElement(new htmlImage('../../graphics/' . $profileClasses[$i]['icon']));
	$existingContainer->addElement(new htmlSpacer('3px', null));
	$existingContainer->addElement(new htmlOutputText($profileClasses[$i]['title']));
	$existingContainer->addElement(new htmlSpacer('3px', null));
	$select = new htmlSelect('profile_' . $profileClasses[$i]['typeId'], $profileClasses[$i]['profiles']);
	$select->setWidth('15em');
	$existingContainer->addElement($select);
	$existingContainer->addElement(new htmlSpacer('3px', null));
	$editButton = new htmlButton('editProfile_' . $profileClasses[$i]['typeId'], 'edit.png', true);
	$editButton->setTitle(_('Edit'));
	$existingContainer->addElement($editButton);
	$deleteLink = new htmlLink(null, '#', '../../graphics/delete.png');
	$deleteLink->setTitle(_('Delete'));
	$deleteLink->setOnClick("profileShowDeleteDialog('" . _('Delete') . "', '" . _('Ok') . "', '" . _('Cancel') . "', '" . $profileClasses[$i]['typeId'] . "', '" . 'profile_' . $profileClasses[$i]['typeId'] . "');");
	$existingContainer->addElement($deleteLink);
	if (count($configProfiles) > 1) {
		$importLink = new htmlLink(null, '#', '../../graphics/import.png');
		$importLink->setTitle(_('Import profiles'));
		$importLink->setOnClick("showDistributionDialog('" . _("Import profiles") . "', '" .
								_('Ok') . "', '" . _('Cancel') . "', '" . $profileClasses[$i]['typeId'] . "', 'import');");
		$existingContainer->addElement($importLink);
	}
	$exportLink = new htmlLink(null, '#', '../../graphics/export.png');
	$exportLink->setTitle(_('Export profile'));
	$exportLink->setOnClick("showDistributionDialog('" . _("Export profile") . "', '" .
							_('Ok') . "', '" . _('Cancel') . "', '" . $profileClasses[$i]['typeId'] . "', 'export', '" . 'profile_' . $profileClasses[$i]['typeId'] . "');");
	$existingContainer->addElement($exportLink);
	$existingContainer->addNewLine();
}
$container->addElement($existingContainer);
$container->addElement(new htmlSpacer(null, '10px'), true);

// generate content
$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');

echo "</form>\n";
echo "</div>\n";

for ($i = 0; $i < sizeof($profileClasses); $i++) {
	$typeId = $profileClasses[$i]['typeId'];
	$scope = $profileClasses[$i]['scope'];
	$importOptions = array();
	foreach ($configProfiles as $profile) {
		$typeManagerImport = new \LAM\TYPES\TypeManager($serverProfiles[$profile]);
		$typesImport = $typeManagerImport->getConfiguredTypesForScope($scope);
		foreach ($typesImport as $typeImport) {
			if (($profile != $_SESSION['config']->getName()) || ($typeImport->getId() != $typeId)) {
				$accountProfiles = \LAM\PROFILES\getAccountProfiles($typeImport->getId(), $profile);
				if (!empty($accountProfiles)) {
					for ($p = 0; $p < sizeof($accountProfiles); $p++) {
						$importOptions[$profile][$typeImport->getAlias() . ': ' . $accountProfiles[$p]] = $profile . '##' . $typeImport->getId() . '##' . $accountProfiles[$p];
					}
				}
			}
		}
	}

	//import dialog
	echo "<div id=\"importDialog_$typeId\" class=\"hidden\">\n";
	echo "<form id=\"importDialogForm_$typeId\" method=\"post\" action=\"profilemain.php\">\n";

	$container = new htmlTable();
	$container->addElement(new htmlOutputText(_('Profiles')), true);

	$select = new htmlSelect('importProfiles', $importOptions, array(), count($importOptions, 1) < 15 ? count($importOptions, 1) : 15);
	$select->setMultiSelect(true);
	$select->setHasDescriptiveElements(true);
	$select->setContainsOptgroups(true);
	$select->setWidth('290px');

	$container->addElement($select);
	$container->addElement(new htmlHelpLink('362'), true);

	$container->addElement(new htmlSpacer(null, '10px'), true);

	$container->addElement(new htmlOutputText(_("Master password")), true);
	$exportPasswd = new htmlInputField('passwd_i_' . $typeId);
	$exportPasswd->setIsPassword(true);
	$container->addElement($exportPasswd);
	$container->addElement(new htmlHelpLink('236'));
	$container->addElement(new htmlHiddenInput('import', '1'));
	$container->addElement(new htmlHiddenInput('typeId', $typeId), true);
	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');

	echo '</form>';
	echo "</div>\n";

	//export dialog
	echo "<div id=\"exportDialog_$typeId\" class=\"hidden\">\n";
	echo "<form id=\"exportDialogForm_$typeId\" method=\"post\" action=\"profilemain.php\">\n";

	$container = new htmlTable();

	$container->addElement(new htmlOutputText(_("Target server profile")), true);
	$exportOptions = array();
	foreach ($configProfiles as $profile) {
		$typeManagerExport = new \LAM\TYPES\TypeManager($serverProfiles[$profile]);
		$typesExport = $typeManagerExport->getConfiguredTypesForScope($scope);
		foreach ($typesExport as $typeExport) {
			if (($profile != $_SESSION['config']->getName()) || ($typeExport->getId() != $typeId)) {
				$exportOptions[$typeManagerExport->getConfig()->getName()][$typeExport->getAlias()] = $profile . '##' . $typeExport->getId();
			}
		}
	}
	$exportOptions['*' . _('Global templates')][_('Global templates')] = 'templates*##';

	$select = new htmlSelect('exportProfiles', $exportOptions, array(), count($exportOptions) < 10 ? count($exportOptions, 1) : 10);
	$select->setHasDescriptiveElements(true);
	$select->setContainsOptgroups(true);
	$select->setMultiSelect(true);

	$container->addElement($select);
	$container->addElement(new htmlHelpLink('363'), true);

	$container->addElement(new htmlSpacer(null, '10px'), true);

	$container->addElement(new htmlOutputText(_("Master password")), true);
	$exportPasswd = new htmlInputField('passwd_e_' . $typeId);
	$exportPasswd->setIsPassword(true);
	$container->addElement($exportPasswd);
	$container->addElement(new htmlHelpLink('236'));
	$container->addElement(new htmlHiddenInput('export', '1'), true);
	$container->addElement(new htmlHiddenInput('typeId', $typeId), true);
	$container->addElement(new htmlHiddenInput('name_' . $typeId, '_'), true);
	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');

	echo '</form>';
	echo "</div>\n";

}

// form for delete action
echo '<div id="deleteProfileDialog" class="hidden"><form id="deleteProfileForm" action="profilemain.php" method="post">';
	echo _("Do you really want to delete this profile?");
	echo '<br><br><div class="nowrap">';
	echo _("Profile name") . ': <div id="deleteText" style="display: inline;"></div></div>';
	echo '<input id="profileDeleteType" type="hidden" name="profileDeleteType" value="">';
	echo '<input id="profileDeleteName" type="hidden" name="profileDeleteName" value="">';
	echo '<input type="hidden" name="deleteProfile" value="true">';
	echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';
echo '</form></div>';

include '../main_footer.php';

/**
 * Imports the selected account profiles.
 *
 * @param string $typeId type id
 * @param array $options options
 * @param \LAMConfig[] $serverProfiles server profiles (name => profile object)
 * @param \LAM\TYPES\TypeManager $typeManager type manager
 * @return \htmlStatusMessage message or null
 */
function importProfiles($typeId, $options, &$serverProfiles, &$typeManager) {
	foreach ($options as $option) {
		$sourceConfName = $option['conf'];
		$sourceTypeId = $option['typeId'];
		$sourceName = $option['name'];
		$sourceTypeManager = new \LAM\TYPES\TypeManager($serverProfiles[$sourceConfName]);
		$sourceType = $sourceTypeManager->getConfiguredType($sourceTypeId);
		$targetType = $typeManager->getConfiguredType($typeId);
		if (($sourceType != null) && ($targetType != null)) {
			try {
				\LAM\PROFILES\copyAccountProfile($sourceType, $sourceName, $targetType);
			}
			catch (\LAMException $e) {
				return new \htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
			}
		}
	}
	return new \htmlStatusMessage('INFO', _('Import successful'));
}

/**
 * Exports the selected account profile.
 *
 * @param string $typeId source type id
 * @param string $name profile name
 * @param array $options options
 * @param \LAMConfig[] $serverProfiles server profiles (name => profile object)
 * @param \LAM\TYPES\TypeManager $typeManager type manager
 * @return \htmlStatusMessage message or null
 */
function exportProfiles($typeId, $name, $options, &$serverProfiles, &$typeManager) {
	$sourceType = $typeManager->getConfiguredType($typeId);
	if ($sourceType == null) {
		return null;
	}
	foreach ($options as $option) {
		$targetConfName = $option['conf'];
		if ($targetConfName == 'templates*') {
			try {
				\LAM\PROFILES\copyAccountProfileToTemplates($sourceType, $name);
			}
			catch (\LAMException $e) {
				return new \htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
			}
		}
		else {
			$targetTypeId = $option['typeId'];
			$targetTypeManager = new \LAM\TYPES\TypeManager($serverProfiles[$targetConfName]);
			$targetType = $targetTypeManager->getConfiguredType($targetTypeId);
			if ($targetType != null) {
				try {
					\LAM\PROFILES\copyAccountProfile($sourceType, $name, $targetType);
				}
				catch (\LAMException $e) {
					return new \htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
				}
			}
		}
	}
	return new \htmlStatusMessage('INFO', _('Export successful'));
}

?>
