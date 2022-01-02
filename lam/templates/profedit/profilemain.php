<?php
namespace LAM\TOOLS\PROFILE_EDITOR;
use htmlDiv;
use htmlForm;
use htmlResponsiveInputField;
use htmlResponsiveSelect;
use \htmlTitle;
use \htmlStatusMessage;
use LAM\PROFILES\AccountProfilePersistenceManager;
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
use \htmlResponsiveRow;
use \htmlGroup;
use \LAM\TYPES\TypeManager;
use LAMException;
use ServerProfilePersistenceManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2021  Roland Gruber

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
include_once(__DIR__ . "/../../lib/security.inc");
/** helper functions for profiles */
include_once(__DIR__ . "/../../lib/profiles.inc");
/** access to LDAP server */
include_once(__DIR__ . "/../../lib/ldap.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) {
	die();
}

checkIfToolIsActive('toolProfileEditor');

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

$typeManager = new TypeManager();
$types = $typeManager->getConfiguredTypes();

$container = new htmlResponsiveRow();
$container->add(new htmlTitle(_("Profile editor")), 12);

$accountProfilePersistenceManager = new AccountProfilePersistenceManager();

if (isset($_POST['deleteProfile']) && ($_POST['deleteProfile'] == 'true')) {
	$type = $typeManager->getConfiguredType($_POST['profileDeleteType']);
	if ($type->isHidden()) {
		logNewMessage(LOG_ERR, 'User tried to delete hidden account type profile: ' . $_POST['profileDeleteType']);
		die();
	}
	// delete profile
	try {
		$accountProfilePersistenceManager->deleteAccountProfile($_POST['profileDeleteType'], $_POST['profileDeleteName'], $_SESSION['config']->getName());
		$message = new htmlStatusMessage('INFO', _('Deleted profile.'), $type->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$container->add($message, 12);
	}
	catch (LAMException $e) {
		$message = new htmlStatusMessage('ERROR', $e->getTitle(), $type->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$container->add($message, 12);
	}
}

// delete global template
if (isset($_POST['deleteGlobalTemplate']) && !empty($_POST['globalTemplatesDelete'])) {
	$cfg = new LAMCfgMain();
	if (empty($_POST['globalTemplateDeletePassword']) || !$cfg->checkPassword($_POST['globalTemplateDeletePassword'])) {
		$container->add(new htmlStatusMessage('ERROR', _('Master password is wrong!')), 12);
	}
	else {
		$selectedOptions = explode(':', $_POST['globalTemplatesDelete']);
		$selectedScope = $selectedOptions[0];
		$selectedName = $selectedOptions[1];
		try {
			$accountProfilePersistenceManager->deleteAccountProfileTemplate($selectedScope, $selectedName);
			$container->add(new htmlStatusMessage('INFO', _('Deleted profile.'), $selectedName), 12);
		} catch (LAMException $e) {
			$container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
		}
	}
}

$serverProfilePersistenceManager = new ServerProfilePersistenceManager();
$serverProfiles = array();
$configProfiles = array();
try {
	$configProfiles = $serverProfilePersistenceManager->getProfiles();
	foreach ($configProfiles as $profileName) {
		$serverProfiles[$profileName] = $serverProfilePersistenceManager->loadProfile($profileName);
	}
} catch (LAMException $e) {
	logNewMessage(LOG_ERR, 'Unable to read server profiles: ' . $e->getTitle());
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
	if ($errMessage !== null) {
		$container->add($errMessage, 12);
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
	if ($errMessage !== null) {
		$container->add($errMessage, 12);
	}
}

$profileClasses = array();
$profileClassesTemp = array();
foreach ($types as $type) {
	if ($type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
		continue;
	}
	$profileList = $accountProfilePersistenceManager->getAccountProfileNames($type->getId(), $_SESSION['config']->getName());
	natcasesort($profileList);
	$profileClassesTemp[$type->getAlias()] = array(
		'typeId' => $type->getId(),
		'scope' => $type->getScope(),
		'title' => $type->getAlias(),
		'icon' => $type->getIcon(),
		'profiles' => $profileList);
}
$profileClassesKeys = array_keys($profileClassesTemp);
natcasesort($profileClassesKeys);
$profileClassesKeys = array_values($profileClassesKeys);
foreach ($profileClassesKeys as $profileClassesKey) {
	$profileClasses[] = $profileClassesTemp[$profileClassesKey];
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
foreach ($profileClasses as $profileClass) {
	if (isset($_POST['editProfile_' . $profileClass['typeId']]) || isset($_POST['editProfile_' . $profileClass['typeId'] . '_x'])) {
		metaRefresh("profilepage.php?type=" . htmlspecialchars($profileClass['typeId']) .
					"&edit=" . htmlspecialchars($_POST['profile_' . $profileClass['typeId']]));
		exit;
	}
}

include __DIR__ . '/../../lib/adminHeader.inc';
echo "<div class=\"smallPaddingContent\">\n";
echo "<form name=\"profilemainForm\" action=\"profilemain.php\" method=\"post\">\n";
echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';

if (isset($_GET['savedSuccessfully'])) {
	$message = new htmlStatusMessage("INFO", _("Profile was saved."), htmlspecialchars($_GET['savedSuccessfully']));
	$container->add($message, 12);
}

// new profile
if (!empty($profileClasses)) {
	$container->add(new htmlSubTitle(_('Create a new profile')), 12);
	$sortedTypes = array();
	foreach ($profileClasses as $profileClass) {
		$sortedTypes[$profileClass['title']] = $profileClass['typeId'];
	}
	natcasesort($sortedTypes);
	$newProfileSelect = new htmlSelect('createProfile', $sortedTypes);
	$newProfileSelect->setHasDescriptiveElements(true);
	$container->addLabel($newProfileSelect);
	$createButton = new htmlButton('createProfileButton', _('Create'));
	$createButton->setCSSClasses(array('lam-primary'));
	$container->addField($createButton);
}

$container->addVerticalSpacer('1rem');

// existing profiles
$container->add(new htmlSubTitle(_('Manage existing profiles')), 12);

foreach ($profileClasses as $profileClass) {
	$labelGroup = new htmlGroup();
	$labelGroup->addElement(new htmlImage('../../graphics/' . $profileClass['icon']));
	$labelGroup->addElement(new htmlSpacer('3px', null));
	$labelGroup->addElement(new htmlOutputText($profileClass['title']));
	$container->add($labelGroup, 12, 4);
	$select = new htmlSelect('profile_' . $profileClass['typeId'], $profileClass['profiles']);
	$container->add($select, 12, 4);
	$buttonGroup = new htmlGroup();
	$editButton = new htmlButton('editProfile_' . $profileClass['typeId'], 'edit.png', true);
	$editButton->setTitle(_('Edit'));
	$buttonGroup->addElement($editButton);
	$deleteLink = new htmlLink(null, '#', '../../graphics/delete.png');
	$deleteLink->setTitle(_('Delete'));
	$deleteLink->setOnClick("profileShowDeleteDialog('" . _('Delete') . "', '" . _('Ok') . "', '" . _('Cancel') . "', '" . $profileClass['typeId'] . "', '" . 'profile_' . $profileClass['typeId'] . "'); return false;");
	$deleteLink->setCSSClasses(array('margin3'));
	$buttonGroup->addElement($deleteLink);
	if (count($configProfiles) > 1) {
		$importLink = new htmlLink(null, '#', '../../graphics/import.svg');
		$importLink->setTitle(_('Import profiles'));
		$importLink->setOnClick("window.lam.profilePdfEditor.showDistributionDialog('" . _("Import profiles") . "', '" .
								_('Ok') . "', '" . _('Cancel') . "', '" . $profileClass['typeId'] . "', 'import'); return false;");
		$importLink->setCSSClasses(array('margin3'));
		$buttonGroup->addElement($importLink);
	}
	$exportLink = new htmlLink(null, '#', '../../graphics/export.svg');
	$exportLink->setTitle(_('Export profile'));
	$exportLink->setOnClick("window.lam.profilePdfEditor.showDistributionDialog('" . _("Export profile") . "', '" .
							_('Ok') . "', '" . _('Cancel') . "', '" . $profileClass['typeId'] . "', 'export', '" . 'profile_' . $profileClass['typeId'] . "'); return false;");
	$exportLink->setCSSClasses(array('margin3'));
	$buttonGroup->addElement($exportLink);
	$container->add($buttonGroup, 12, 4);
	$container->addVerticalSpacer('1rem');
}
$container->addVerticalSpacer('1rem');

// generate content
$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, 'user');

echo "</form>\n";

// delete global templates
$globalDeletableTemplates = array();
try {
	$globalTemplates = $accountProfilePersistenceManager->getAccountProfileTemplateNames();
	foreach ($globalTemplates as $typeId => $availableTemplates) {
		if (empty($availableTemplates)) {
			continue;
		}
		foreach ($availableTemplates as $availableTemplate) {
			if ($availableTemplate !== 'default') {
				$globalDeletableTemplates[$typeId][$availableTemplate] = $typeId . ':' . $availableTemplate;
			}
		}
	}
}
catch (LAMException $e) {
	logNewMessage(LOG_ERR, $e->getTitle());
}

if (!empty($globalDeletableTemplates)) {
	$container = new htmlResponsiveRow();
	$globalTemplatesSubtitle = new htmlSubTitle(_('Global templates'));
	$globalTemplatesSubtitle->setHelpId('364');
	$container->add($globalTemplatesSubtitle, 12);
	$globalTemplatesSelect = new htmlResponsiveSelect('globalTemplatesDelete', $globalDeletableTemplates, array(), _('Delete'));
	$globalTemplatesSelect->setContainsOptgroups(true);
	$globalTemplatesSelect->setHasDescriptiveElements(true);
	$container->add($globalTemplatesSelect, 12);
	$globalTemplateDeleteDialogPassword = new htmlResponsiveInputField(_("Master password"), 'globalTemplateDeletePassword', null, '236');
	$globalTemplateDeleteDialogPassword->setIsPassword(true);
	$globalTemplateDeleteDialogPassword->setRequired(true);
	$container->add($globalTemplateDeleteDialogPassword, 12);
	$container->addVerticalSpacer('1rem');
	$globalTemplateDeleteButton = new htmlButton('deleteGlobalProfileButton', _('Delete'));
	$globalTemplateDeleteButton->setCSSClasses(array('lam-danger'));
	$globalTemplateDeleteButton->setOnClick("showConfirmationDialog('" . _("Delete") . "', '" .
		_('Ok') . "', '" . _('Cancel') . "', 'globalTemplateDeleteDialog', 'deleteGlobalTemplatesForm', undefined); return false;");
	$container->addLabel(new htmlOutputText('&nbsp;', false));
	$container->addField($globalTemplateDeleteButton, 12);
	addSecurityTokenToMetaHTML($container);
	$globalTemplateDeleteDialogContent = new htmlResponsiveRow();
	$globalTemplateDeleteDialogContent->add(new htmlOutputText(_('Do you really want to delete this profile?')), 12);
	$globalTemplateDeleteDialogContent->add(new htmlHiddenInput('deleteGlobalTemplate', 'true'), 12);
	$globalTemplateDeleteDialogDiv = new htmlDiv('globalTemplateDeleteDialog', $globalTemplateDeleteDialogContent, array('hidden'));
	$container->add($globalTemplateDeleteDialogDiv, 12);
	$container->addVerticalSpacer('1rem');
	$globalTemplateDeleteForm = new htmlForm('deleteGlobalTemplatesForm', 'profilemain.php', $container);
	parseHtml(null, $globalTemplateDeleteForm, array(), false, $tabindex, 'user');
}

echo "</div>\n";

foreach ($profileClasses as $profileClass) {
	$typeId = $profileClass['typeId'];
	$scope = $profileClass['scope'];
	$importOptions = array();
	foreach ($configProfiles as $profile) {
		$typeManagerImport = new TypeManager($serverProfiles[$profile]);
		$typesImport = $typeManagerImport->getConfiguredTypesForScope($scope);
		foreach ($typesImport as $typeImport) {
			if (($profile != $_SESSION['config']->getName()) || ($typeImport->getId() != $typeId)) {
				$accountProfiles = $accountProfilePersistenceManager->getAccountProfileNames($typeImport->getId(), $profile);
				if (!empty($accountProfiles)) {
					foreach ($accountProfiles as $accountProfile) {
						$importOptions[$profile][$typeImport->getAlias() . ': ' . $accountProfile] = $profile . '##' . $typeImport->getId() . '##' . $accountProfile;
					}
				}
			}
		}
	}

	//import dialog
	echo "<div id=\"importDialog_$typeId\" class=\"hidden\">\n";
	echo "<form id=\"importDialogForm_$typeId\" method=\"post\" action=\"profilemain.php\">\n";

	$containerProfiles = new htmlResponsiveRow();
	$containerProfiles->add(new htmlOutputText(_('Profiles')), 12);

	$select = new htmlSelect('importProfiles', $importOptions, array(), count($importOptions, 1) < 15 ? count($importOptions, 1) : 15);
	$select->setMultiSelect(true);
	$select->setHasDescriptiveElements(true);
	$select->setContainsOptgroups(true);

	$containerProfiles->add($select, 11);
	$containerProfiles->add(new htmlHelpLink('362'), 1);

	$containerProfiles->addVerticalSpacer('2rem');

	$containerProfiles->add(new htmlOutputText(_("Master password")), 12);
	$exportPasswd = new htmlInputField('passwd_i_' . $typeId);
	$exportPasswd->setIsPassword(true);
	$containerProfiles->add($exportPasswd, 11);
	$containerProfiles->add(new htmlHelpLink('236'), 1);
	$containerProfiles->add(new htmlHiddenInput('import', '1'), 0);
	$containerProfiles->add(new htmlHiddenInput('typeId', $typeId), 0);
	addSecurityTokenToMetaHTML($containerProfiles);

	parseHtml(null, $containerProfiles, array(), false, $tabindex, 'user');

	echo '</form>';
	echo "</div>\n";

	//export dialog
	echo "<div id=\"exportDialog_$typeId\" class=\"hidden\">\n";
	echo "<form id=\"exportDialogForm_$typeId\" method=\"post\" action=\"profilemain.php\">\n";

	$containerTarget = new htmlResponsiveRow();

	$containerTarget->add(new htmlOutputText(_("Target server profile")), 12);
	$exportOptions = array();
	foreach ($configProfiles as $profile) {
		$typeManagerExport = new TypeManager($serverProfiles[$profile]);
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

	$containerTarget->add($select, 11);
	$containerTarget->add(new htmlHelpLink('363'), 1);

	$containerTarget->addVerticalSpacer('2rem');

	$containerTarget->add(new htmlOutputText(_("Master password")), 12);
	$exportPasswd = new htmlInputField('passwd_e_' . $typeId);
	$exportPasswd->setIsPassword(true);
	$containerTarget->add($exportPasswd, 11);
	$containerTarget->add(new htmlHelpLink('236'), 1);
	$containerTarget->add(new htmlHiddenInput('export', '1'), 0);
	$containerTarget->add(new htmlHiddenInput('typeId', $typeId), 0);
	$containerTarget->add(new htmlHiddenInput('name_' . $typeId, '_'), 0);
	addSecurityTokenToMetaHTML($containerTarget);

	parseHtml(null, $containerTarget, array(), false, $tabindex, 'user');

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

include __DIR__ . '/../../lib/adminFooter.inc';

/**
 * Imports the selected account profiles.
 *
 * @param string $typeId type id
 * @param array $options options
 * @param \LAMConfig[] $serverProfiles server profiles (name => profile object)
 * @param TypeManager $typeManager type manager
 * @return \htmlStatusMessage message or null
 */
function importProfiles($typeId, $options, &$serverProfiles, TypeManager &$typeManager) {
	$accountProfilesPersistenceManager = new AccountProfilePersistenceManager();
	foreach ($options as $option) {
		$sourceConfName = $option['conf'];
		$sourceTypeId = $option['typeId'];
		$sourceName = $option['name'];
		$sourceTypeManager = new TypeManager($serverProfiles[$sourceConfName]);
		$sourceType = $sourceTypeManager->getConfiguredType($sourceTypeId);
		$targetType = $typeManager->getConfiguredType($typeId);
		if (($sourceType !== null) && ($targetType !== null)) {
			try {
				$data = $accountProfilesPersistenceManager->loadAccountProfile($sourceType->getId(), $sourceName, $sourceConfName);
				$accountProfilesPersistenceManager->writeAccountProfile($typeId, $sourceName,
					$targetType->getTypeManager()->getConfig()->getName(), $data);
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
 * @param TypeManager $typeManager type manager
 * @return htmlStatusMessage message or null
 */
function exportProfiles($typeId, $name, $options, &$serverProfiles, TypeManager &$typeManager): ?htmlStatusMessage {
	$sourceType = $typeManager->getConfiguredType($typeId);
	if ($sourceType === null) {
		return null;
	}
	$accountProfilePersistenceManager = new AccountProfilePersistenceManager();
	foreach ($options as $option) {
		$targetConfName = $option['conf'];
		if ($targetConfName == 'templates*') {
			try {
				$data = $accountProfilePersistenceManager->loadAccountProfile($sourceType->getId(), $name, $_SESSION['config']->getName());
				$accountProfilePersistenceManager->writeAccountProfileTemplate($sourceType->getScope(), $name, $data);
			}
			catch (LAMException $e) {
				return new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
			}
		}
		else {
			$targetTypeId = $option['typeId'];
			$targetTypeManager = new TypeManager($serverProfiles[$targetConfName]);
			$targetType = $targetTypeManager->getConfiguredType($targetTypeId);
			if ($targetType !== null) {
				try {
					$data = $accountProfilePersistenceManager->loadAccountProfile($sourceType->getId(), $name, $_SESSION['config']->getName());
					$accountProfilePersistenceManager->writeAccountProfile($targetType->getId(), $name, $targetConfName, $data);
				}
				catch (LAMException $e) {
					return new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
				}
			}
		}
	}
	return new htmlStatusMessage('INFO', _('Export successful'));
}
