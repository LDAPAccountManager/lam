<?php
namespace LAM\TOOLS\PDF_EDITOR;
use \htmlTitle;
use \htmlStatusMessage;
use \LAMCfgMain;
use \htmlSubTitle;
use \htmlSelect;
use \htmlImage;
use \htmlSpacer;
use \htmlButton;
use \htmlLink;
use \htmlOutputText;
use \htmlInputFileUpload;
use \htmlHelpLink;
use \htmlInputField;
use \htmlHiddenInput;
use \htmlResponsiveRow;
use \htmlGroup;
use \LAM\TYPES\TypeManager;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2005 - 2021  Roland Gruber

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
* This is the main window of the PDF structure editor.
*
* @author Michael Duergner
* @author Roland Gruber
* @package PDF
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to PDF configuration files */
include_once(__DIR__ . "/../../lib/pdfstruct.inc");
/** LDAP object */
include_once(__DIR__ . "/../../lib/ldap.inc");
/** for language settings */
include_once(__DIR__ . "/../../lib/config.inc");
/** module functions */
include_once(__DIR__ . "/../../lib/modules.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolPDFEditor');

if (!empty($_POST)) {
	validateSecurityToken();
}

setlanguage();

// Unset PDF structure definitions in session if set
if(isset($_SESSION['currentPDFStructure'])) {
	unset($_SESSION['currentPDFStructure']);
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// check if new template should be created
if(isset($_POST['createNewTemplate'])) {
	metaRefresh('pdfpage.php?type=' . htmlspecialchars($_POST['typeId']));
	exit();
}

$typeManager = new TypeManager();
$types = $typeManager->getConfiguredTypes();
$sortedTypes = array();
foreach ($types as $type) {
	if ($type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
		continue;
	}
	$sortedTypes[$type->getId()] = $type->getAlias();
}
natcasesort($sortedTypes);

$container = new htmlResponsiveRow();
$container->add(new htmlTitle(_('PDF editor')), 12);

if (isset($_POST['deleteProfile']) && ($_POST['deleteProfile'] == 'true')) {
	$typeToDelete = $typeManager->getConfiguredType($_POST['profileDeleteType']);
	// delete structure
	if (\LAM\PDF\deletePDFStructure($_POST['profileDeleteType'], $_POST['profileDeleteName'], $_SESSION['config']->getName())) {
		$message = new htmlStatusMessage('INFO', _('Deleted PDF structure.'), $typeToDelete->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$container->add($message, 12);
	}
	else {
		$message = new htmlStatusMessage('ERROR', _('Unable to delete PDF structure!'), $typeToDelete->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$container->add($message, 12);
	}
}

$configProfiles = getConfigProfiles();
$serverProfiles = array();
foreach ($configProfiles as $profileName) {
	$serverProfiles[$profileName] = new \LAMConfig($profileName);
}

// import structures
if (!empty($_POST['import'])) {
	$cfg = new LAMCfgMain();
	$typeId = $_POST['typeId'];
	// check master password
	$errMessage = null;
	if (!$cfg->checkPassword($_POST['passwd_i_' . $_POST['typeId']])) {
		$errMessage = new htmlStatusMessage('ERROR', _('Master password is wrong!'));
	}
	elseif (!empty($_POST['importProfiles_' . $typeId])) {
		$options = array();
		foreach ($_POST['importProfiles_' . $typeId] as $importProfiles) {
			$parts = explode('##', $importProfiles);
			$options[] = array('conf' => $parts[0], 'typeId' => $parts[1], 'name' => $parts[2]);
		}
		$errMessage = importStructures($_POST['typeId'], $options, $serverProfiles, $typeManager);
	}
	if ($errMessage !== null) {
		$container->add($errMessage, 12);
	}
}

// export structures
if (!empty($_POST['export'])) {
	$cfg = new LAMCfgMain();
	$typeId = $_POST['typeId'];
	// check master password
	$errMessage = null;
	if (!$cfg->checkPassword($_POST['passwd_e_' . $_POST['typeId']])) {
		$errMessage = new htmlStatusMessage('ERROR', _('Master password is wrong!'));
	}
	elseif (!empty($_POST['exportProfiles_' . $typeId])) {
		$options = array();
		foreach ($_POST['exportProfiles_' . $typeId] as $importProfiles) {
			$parts = explode('##', $importProfiles);
			$options[] = array('conf' => $parts[0], 'typeId' => $parts[1]);
		}
		$typeId = $_POST['typeId'];
		$name = $_POST['name_' . $typeId];
		$errMessage = exportStructures($typeId, $name, $options, $serverProfiles, $typeManager);
	}
	if ($errMessage !== null) {
		$container->add($errMessage, 12);
	}
}

// upload logo file
if (isset($_POST['uploadLogo']) && !empty($_FILES['logoUpload']) && !empty($_FILES['logoUpload']['size'])) {
	$file = $_FILES['logoUpload']['tmp_name'];
	$filename = $_FILES['logoUpload']['name'];
	$container->add(\LAM\PDF\uploadPDFLogo($file, $filename, $_SESSION['config']->getName()), 12);
}

// delete logo file
if (isset($_POST['delLogo'])) {
	$toDel = $_POST['logo'];
	$container->add(\LAM\PDF\deletePDFLogo($toDel, $_SESSION['config']->getName()), 12);
}

// get list of account types
$availableTypes = array();
$templateClasses = array();
foreach ($sortedTypes as $typeId => $title) {
	$type = $typeManager->getConfiguredType($typeId);
	$templateClasses[] = array(
		'typeId' => $type->getId(),
		'scope' => $type->getScope(),
		'title' => $title,
		'icon' => $type->getIcon(),
		'templates' => \LAM\PDF\getPDFStructures($type->getId(), $_SESSION['config']->getName()));
	$availableTypes[$title] = $type->getId();
}
// check if a template should be edited
foreach ($templateClasses as $templateClass) {
	if (isset($_POST['editTemplate_' . $templateClass['typeId']]) || isset($_POST['editTemplate_' . $templateClass['typeId'] . '_x'])) {
		metaRefresh('pdfpage.php?type=' . htmlspecialchars($templateClass['typeId']) . '&edit=' . htmlspecialchars($_POST['template_' . $templateClass['typeId']]));
		exit;
	}
}
include __DIR__ . '/../../lib/adminHeader.inc';

?>
<div class="user-bright smallPaddingContent">
<form enctype="multipart/form-data" action="pdfmain.php" method="post" name="pdfmainForm" >
<input type="hidden" name="<?php echo getSecurityTokenName(); ?>" value="<?php echo getSecurityTokenValue(); ?>">
	<?php
		if (isset($_GET['savedSuccessfully'])) {
			$message = new htmlStatusMessage("INFO", _("PDF structure was successfully saved."), htmlspecialchars($_GET['savedSuccessfully']));
			$container->add($message, 12);
		}

		if (isset($_GET['loadFailed'])) {
			$message = new htmlStatusMessage("ERROR", _("Unable to read PDF structure."), htmlspecialchars($_GET['name']));
			$container->add($message, 12);
		}

		// new template
		if (!empty($availableTypes)) {
			$container->add(new htmlSubTitle(_('Create a new PDF structure')), 12);
			$newProfileSelect = new htmlSelect('typeId', $availableTypes);
			$newProfileSelect->setHasDescriptiveElements(true);
			$container->addLabel($newProfileSelect);
			$container->addField(new htmlButton('createNewTemplate', _('Create')));
			$container->addVerticalSpacer('2rem');
		}

		// existing templates
		$container->add(new htmlSubTitle(_("Manage existing PDF structures")), 12);
		foreach ($templateClasses as $templateClass) {
			$labelGroup = new htmlGroup();
			$labelGroup->addElement(new htmlImage('../../graphics/' . $templateClass['icon']));
			$labelGroup->addElement(new htmlSpacer('3px', null));
			$labelGroup->addElement(new htmlOutputText($templateClass['title']));
			$container->add($labelGroup, 12, 4);
			$select = new htmlSelect('template_' . $templateClass['typeId'], $templateClass['templates']);
			$container->add($select, 12, 4);
			$buttonGroup = new htmlGroup();
			$exEditButton = new htmlButton('editTemplate_' . $templateClass['typeId'], 'edit.png', true);
			$exEditButton->setTitle(_('Edit'));
			$buttonGroup->addElement($exEditButton);
			$deleteLink = new htmlLink(null, '#', '../../graphics/delete.png');
			$deleteLink->setTitle(_('Delete'));
			$deleteLink->setOnClick("profileShowDeleteDialog('" . _('Delete') . "', '" . _('Ok') . "', '" .
										_('Cancel') . "', '" . $templateClass['typeId'] . "', '" . 'template_' .
										$templateClass['typeId'] . "'); return false;");
			$deleteLink->setCSSClasses(array('margin3'));
			$buttonGroup->addElement($deleteLink);

			if (count($configProfiles) > 1) {
				$importLink = new htmlLink(null, '#', '../../graphics/import.png');
				$importLink->setTitle(_('Import PDF structures'));
				$importLink->setOnClick("window.lam.profilePdfEditor.showDistributionDialog('" . _("Import PDF structures") . "', '" .
										_('Ok') . "', '" . _('Cancel') . "', '" . $templateClass['typeId'] .
										"', 'import'); return false;");
				$importLink->setCSSClasses(array('margin3'));
				$buttonGroup->addElement($importLink);
			}
			$exportLink = new htmlLink(null, '#', '../../graphics/export.png');
			$exportLink->setTitle(_('Export PDF structure'));
			$exportLink->setOnClick("window.lam.profilePdfEditor.showDistributionDialog('" . _("Export PDF structure") . "', '" .
									_('Ok') . "', '" . _('Cancel') . "', '" . $templateClass['typeId'] .
									"', 'export', '" . 'template_' . $templateClass['typeId'] . "', '" .
									$_SESSION['config']->getName() . "'); return false;");
			$exportLink->setCSSClasses(array('margin3'));
			$buttonGroup->addElement($exportLink);
			$container->add($buttonGroup, 12, 4);
			$container->addVerticalSpacer('1rem');
		}

		// manage logos
		$container->addVerticalSpacer('4rem');
		$container->add(new htmlSubTitle(_('Manage logos')), 12);
		$logos = \LAM\PDF\getAvailableLogos($_SESSION['config']->getName());
		$logoOptions = array();
		foreach ($logos as $logo) {
			$file = $logo['filename'];
			$label = $file . ' (' . $logo['infos'][0] . ' x ' . $logo['infos'][1] . ")";
			$logoOptions[$label] = $file;
		}
		$logoSelect = new htmlSelect('logo', $logoOptions, null);
		$logoSelect->setHasDescriptiveElements(true);
		$container->addLabel($logoSelect);
		$delLogo = new htmlButton('delLogo', _('Delete'));
		$delLogo->setIconClass('deleteButton');
		$container->addField($delLogo);
		$container->addVerticalSpacer('2rem');
		$container->addLabel(new htmlInputFileUpload('logoUpload'));
		$logoUpload = new htmlButton('uploadLogo', _('Upload'));
		$logoUpload->setIconClass('upButton');
		$container->addField($logoUpload);

		$container->addVerticalSpacer('2rem');
		// generate content
		$tabindex = 1;
		parseHtml(null, $container, array(), false, $tabindex, 'user');

		echo "</form>\n";
		echo "</div>\n";

		foreach ($templateClasses as $templateClass) {
			$typeId = $templateClass['typeId'];
			$scope = $templateClass['scope'];
			$importOptions = array();
			foreach ($configProfiles as $profile) {
				$typeManagerImport = new TypeManager($serverProfiles[$profile]);
				$typesImport = $typeManagerImport->getConfiguredTypesForScope($scope);
				foreach ($typesImport as $typeImport) {
					if (($profile != $_SESSION['config']->getName()) || ($typeImport->getId() != $typeId)) {
						$accountProfiles = \LAM\PDF\getPDFStructures($typeImport->getId(), $profile);
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
			echo "<form id=\"importDialogForm_$typeId\" method=\"post\" action=\"pdfmain.php\">\n";

			$containerStructures = new htmlResponsiveRow();
			$containerStructures->add(new htmlOutputText(_('PDF structures')), 12);

			$select = new htmlSelect('importProfiles_' . $typeId, $importOptions, array(), count($importOptions, 1) < 15 ? count($importOptions, 1) : 15);
			$select->setMultiSelect(true);
			$select->setHasDescriptiveElements(true);
			$select->setContainsOptgroups(true);

			$containerStructures->add($select, 11);
			$containerStructures->add(new htmlHelpLink('408'), 1);

			$containerStructures->addVerticalSpacer('2rem');

			$containerStructures->add(new htmlOutputText(_("Master password")), 12);
			$exportPasswd = new htmlInputField('passwd_i_' . $typeId);
			$exportPasswd->setIsPassword(true);
			$containerStructures->add($exportPasswd, 11);
			$containerStructures->add(new htmlHelpLink('236'), 1);
			$containerStructures->add(new htmlHiddenInput('import', '1'), 12);
			$containerStructures->add(new htmlHiddenInput('typeId', $typeId), 12);
			addSecurityTokenToMetaHTML($containerStructures);

			parseHtml(null, $containerStructures, array(), false, $tabindex, 'user');

			echo '</form>';
			echo "</div>\n";

			//export dialog
			echo "<div id=\"exportDialog_$typeId\" class=\"hidden\">\n";
			echo "<form id=\"exportDialogForm_$typeId\" method=\"post\" action=\"pdfmain.php\">\n";

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

			$exportSize = count($exportOptions) < 10 ? count($exportOptions, 1) : 10;
			$select = new htmlSelect('exportProfiles_' . $typeId, $exportOptions, array(), $exportSize);
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
			$containerTarget->add(new htmlHiddenInput('export', '1'), 12);
			$containerTarget->add(new htmlHiddenInput('typeId', $typeId), 12);
			$containerTarget->add(new htmlHiddenInput('name_' . $typeId, '_'), 12);
			addSecurityTokenToMetaHTML($containerTarget);

			parseHtml(null, $containerTarget, array(), false, $tabindex, 'user');

			echo '</form>';
			echo "</div>\n";
		}

// form for delete action
echo '<div id="deleteProfileDialog" class="hidden"><form id="deleteProfileForm" action="pdfmain.php" method="post">';
	echo _("Do you really want to delete this PDF structure?");
	echo '<br><br><div class="nowrap">';
	echo _("Structure name") . ': <div id="deleteText" style="display: inline;"></div></div>';
	echo '<input id="profileDeleteType" type="hidden" name="profileDeleteType" value="">';
	echo '<input id="profileDeleteName" type="hidden" name="profileDeleteName" value="">';
	echo '<input type="hidden" name="deleteProfile" value="true">';
	echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';
echo '</form></div>';

include __DIR__ . '/../../lib/adminFooter.inc';


/**
 * Imports the selected PDF structures.
 *
 * @param string $typeId type id
 * @param array $options options
 * @param \LAMConfig[] $serverProfiles server profiles (name => profile object)
 * @param TypeManager $typeManager type manager
 * @return \htmlStatusMessage message or null
 */
function importStructures($typeId, $options, &$serverProfiles, TypeManager &$typeManager) {
	foreach ($options as $option) {
		$sourceConfName = $option['conf'];
		$sourceTypeId = $option['typeId'];
		$sourceName = $option['name'];
		$sourceTypeManager = new TypeManager($serverProfiles[$sourceConfName]);
		$sourceType = $sourceTypeManager->getConfiguredType($sourceTypeId);
		$targetType = $typeManager->getConfiguredType($typeId);
		if (($sourceType !== null) && ($targetType !== null)) {
			try {
				\LAM\PDF\copyStructure($sourceType, $sourceName, $targetType);
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
 * @return \htmlStatusMessage message or null
 */
function exportStructures($typeId, $name, $options, &$serverProfiles, TypeManager &$typeManager) {
	$sourceType = $typeManager->getConfiguredType($typeId);
	if ($sourceType === null) {
		return null;
	}
	foreach ($options as $option) {
		$targetConfName = $option['conf'];
		if ($targetConfName == 'templates*') {
			try {
				\LAM\PDF\copyStructureToTemplates($sourceType, $name);
			}
			catch (\LAMException $e) {
				return new \htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
			}
		}
		else {
			$targetTypeId = $option['typeId'];
			$targetTypeManager = new TypeManager($serverProfiles[$targetConfName]);
			$targetType = $targetTypeManager->getConfiguredType($targetTypeId);
			if ($targetType !== null) {
				try {
					\LAM\PDF\copyStructure($sourceType, $name, $targetType);
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
