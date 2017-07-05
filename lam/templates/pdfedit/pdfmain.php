<?php
namespace LAM\TOOLS\PDF_EDITOR;
use \htmlTable;
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
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2005 - 2017  Roland Gruber

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
include_once("../../lib/security.inc");
/** access to PDF configuration files */
include_once("../../lib/pdfstruct.inc");
/** LDAP object */
include_once("../../lib/ldap.inc");
/** for language settings */
include_once("../../lib/config.inc");
/** module functions */
include_once("../../lib/modules.inc");

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

$typeManager = new \LAM\TYPES\TypeManager();
$types = $typeManager->getConfiguredTypes();
$sortedTypes = array();
foreach ($types as $type) {
	if ($type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
		continue;
	}
	$sortedTypes[$type->getId()] = $type->getAlias();
}
natcasesort($sortedTypes);

$container = new htmlTable();
$container->addElement(new htmlTitle(_('PDF editor')), true);

if (isset($_POST['deleteProfile']) && ($_POST['deleteProfile'] == 'true')) {
	$typeToDelete = $typeManager->getConfiguredType($_POST['profileDeleteType']);
	// delete structure
	if (\LAM\PDF\deletePDFStructure($_POST['profileDeleteType'], $_POST['profileDeleteName'])) {
		$message = new htmlStatusMessage('INFO', _('Deleted PDF structure.'), $typeToDelete->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$message->colspan = 10;
		$container->addElement($message, true);
	}
	else {
		$message = new htmlStatusMessage('ERROR', _('Unable to delete PDF structure!'), $typeToDelete->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$message->colspan = 10;
		$container->addElement($message, true);
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
	if ($errMessage != null) {
		$errMessage->colspan = 10;
		$container->addElement($errMessage, true);
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
	if ($errMessage != null) {
		$errMessage->colspan = 10;
		$container->addElement($errMessage, true);
	}
}

// upload logo file
if (isset($_POST['uploadLogo']) && !empty($_FILES['logoUpload']) && !empty($_FILES['logoUpload']['size'])) {
	$file = $_FILES['logoUpload']['tmp_name'];
	$filename = $_FILES['logoUpload']['name'];
	$container->addElement(\LAM\PDF\uploadPDFLogo($file, $filename), true);
}

// delete logo file
if (isset($_POST['delLogo'])) {
	$toDel = $_POST['logo'];
	$container->addElement(\LAM\PDF\deletePDFLogo($toDel), true);
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
		'templates' => "");
	$availableTypes[$title] = $type->getId();
}
// get list of templates for each account type
for ($i = 0; $i < sizeof($templateClasses); $i++) {
	$templateClasses[$i]['templates'] = \LAM\PDF\getPDFStructures($templateClasses[$i]['typeId']);
}

// check if a template should be edited
for ($i = 0; $i < sizeof($templateClasses); $i++) {
	if (isset($_POST['editTemplate_' . $templateClasses[$i]['typeId']]) || isset($_POST['editTemplate_' . $templateClasses[$i]['typeId'] . '_x'])) {
		metaRefresh('pdfpage.php?type=' . htmlspecialchars($templateClasses[$i]['typeId']) . '&edit=' . htmlspecialchars($_POST['template_' . $templateClasses[$i]['typeId']]));
		exit;
	}
}

include '../main_header.php';
?>
<div class="user-bright smallPaddingContent">
<form enctype="multipart/form-data" action="pdfmain.php" method="post" name="pdfmainForm" >
<input type="hidden" name="<?php echo getSecurityTokenName(); ?>" value="<?php echo getSecurityTokenValue(); ?>">
	<?php
		if (isset($_GET['savedSuccessfully'])) {
			$message = new htmlStatusMessage("INFO", _("PDF structure was successfully saved."), htmlspecialchars($_GET['savedSuccessfully']));
			$message->colspan = 10;
			$container->addElement($message, true);
		}

		if (isset($_GET['loadFailed'])) {
			$message = new htmlStatusMessage("ERROR", _("Unable to read PDF structure."), htmlspecialchars($_GET['name']));
			$message->colspan = 10;
			$container->addElement($message, true);
		}

		// new template
		if (!empty($availableTypes)) {
			$container->addElement(new htmlSubTitle(_('Create a new PDF structure')), true);
			$newPDFContainer = new htmlTable();
			$newProfileSelect = new htmlSelect('typeId', $availableTypes);
			$newProfileSelect->setHasDescriptiveElements(true);
			$newProfileSelect->setWidth('15em');
			$newPDFContainer->addElement($newProfileSelect);
			$newPDFContainer->addElement(new htmlSpacer('10px', null));
			$newPDFContainer->addElement(new htmlButton('createNewTemplate', _('Create')));
			$container->addElement($newPDFContainer, true);
			$container->addElement(new htmlSpacer(null, '10px'), true);
		}

		// existing templates
		$container->addElement(new htmlSubTitle(_("Manage existing PDF structures")), true);
		$existingContainer = new htmlTable();
		for ($i = 0; $i < sizeof($templateClasses); $i++) {
			if ($i > 0) {
				$existingContainer->addElement(new htmlSpacer(null, '10px'), true);
			}

			$existingContainer->addElement(new htmlImage('../../graphics/' . $templateClasses[$i]['icon']));
			$existingContainer->addElement(new htmlSpacer('3px', null));
			$existingContainer->addElement(new htmlOutputText($templateClasses[$i]['title']));
			$existingContainer->addElement(new htmlSpacer('3px', null));
			$select = new htmlSelect('template_' . $templateClasses[$i]['typeId'], $templateClasses[$i]['templates']);
			$select->setWidth('15em');
			$existingContainer->addElement($select);
			$existingContainer->addElement(new htmlSpacer('3px', null));
			$exEditButton = new htmlButton('editTemplate_' . $templateClasses[$i]['typeId'], 'edit.png', true);
			$exEditButton->setTitle(_('Edit'));
			$existingContainer->addElement($exEditButton);
			$deleteLink = new htmlLink(null, '#', '../../graphics/delete.png');
			$deleteLink->setTitle(_('Delete'));
			$deleteLink->setOnClick("profileShowDeleteDialog('" . _('Delete') . "', '" . _('Ok') . "', '" . _('Cancel') . "', '" . $templateClasses[$i]['typeId'] . "', '" . 'template_' . $templateClasses[$i]['typeId'] . "');");
			$existingContainer->addElement($deleteLink);

			if (count($configProfiles) > 1) {
				$importLink = new htmlLink(null, '#', '../../graphics/import.png');
				$importLink->setTitle(_('Import PDF structures'));
				$importLink->setOnClick("showDistributionDialog('" . _("Import PDF structures") . "', '" .
										_('Ok') . "', '" . _('Cancel') . "', '" . $templateClasses[$i]['typeId'] . "', 'import');");
				$existingContainer->addElement($importLink);
			}
			$exportLink = new htmlLink(null, '#', '../../graphics/export.png');
			$exportLink->setTitle(_('Export PDF structure'));
			$exportLink->setOnClick("showDistributionDialog('" . _("Export PDF structure") . "', '" .
									_('Ok') . "', '" . _('Cancel') . "', '" . $templateClasses[$i]['typeId'] . "', 'export', '" . 'template_' . $templateClasses[$i]['typeId'] . "', '" . $_SESSION['config']->getName() . "');");
			$existingContainer->addElement($exportLink);
			$existingContainer->addNewLine();
		}
		$container->addElement($existingContainer, true);

		// manage logos
		$logoContainer = new htmlTable();
		$logoContainer->addElement(new htmlSpacer(null, '30px'), true);
		$logoContainer->addElement(new htmlSubTitle(_('Manage logos')), true);
		$logos = \LAM\PDF\getAvailableLogos();
		$logoOptions = array();
		foreach ($logos as $logo) {
			$file = $logo['filename'];
			$label = $file . ' (' . $logo['infos'][0] . ' x ' . $logo['infos'][1] . ")";
			$logoOptions[$label] = $file;
		}
		$logoSelect = new htmlSelect('logo', $logoOptions, null);
		$logoSelect->setHasDescriptiveElements(true);
		$logoContainer->addElement($logoSelect);
		$delLogo = new htmlButton('delLogo', _('Delete'));
		$delLogo->setIconClass('deleteButton');
		$logoContainer->addElement($delLogo, true);
		$logoContainer->addElement(new htmlInputFileUpload('logoUpload'));
		$logoUpload = new htmlButton('uploadLogo', _('Upload'));
		$logoUpload->setIconClass('upButton');
		$logoContainer->addElement($logoUpload);
		$container->addElement($logoContainer, true);

		$container->addElement(new htmlSpacer(null, '10px'), true);
		// generate content
		$tabindex = 1;
		parseHtml(null, $container, array(), false, $tabindex, 'user');

		echo "</form>\n";
		echo "</div>\n";

		for ($i = 0; $i < sizeof($templateClasses); $i++) {
			$typeId = $templateClasses[$i]['typeId'];
			$scope = $templateClasses[$i]['scope'];
			$importOptions = array();
			foreach ($configProfiles as $profile) {
				$typeManagerImport = new \LAM\TYPES\TypeManager($serverProfiles[$profile]);
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

			$container = new htmlTable();
			$container->addElement(new htmlOutputText(_('PDF structures')), true);

			$select = new htmlSelect('importProfiles_' . $typeId, $importOptions, array(), count($importOptions, 1) < 15 ? count($importOptions, 1) : 15);
			$select->setMultiSelect(true);
			$select->setHasDescriptiveElements(true);
			$select->setContainsOptgroups(true);
			$select->setWidth('290px');

			$container->addElement($select);
			$container->addElement(new htmlHelpLink('408'), true);

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
			echo "<form id=\"exportDialogForm_$typeId\" method=\"post\" action=\"pdfmain.php\">\n";

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

			$select = new htmlSelect('exportProfiles_' . $typeId, $exportOptions, array(), count($exportOptions) < 10 ? count($exportOptions, 1) : 10);
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
echo '<div id="deleteProfileDialog" class="hidden"><form id="deleteProfileForm" action="pdfmain.php" method="post">';
	echo _("Do you really want to delete this PDF structure?");
	echo '<br><br><div class="nowrap">';
	echo _("Structure name") . ': <div id="deleteText" style="display: inline;"></div></div>';
	echo '<input id="profileDeleteType" type="hidden" name="profileDeleteType" value="">';
	echo '<input id="profileDeleteName" type="hidden" name="profileDeleteName" value="">';
	echo '<input type="hidden" name="deleteProfile" value="true">';
	echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';
echo '</form></div>';

include '../main_footer.php';


/**
 * Imports the selected PDF structures.
 *
 * @param string $typeId type id
 * @param array $options options
 * @param \LAMConfig[] $serverProfiles server profiles (name => profile object)
 * @param \LAM\TYPES\TypeManager $typeManager type manager
 * @return \htmlStatusMessage message or null
 */
function importStructures($typeId, $options, &$serverProfiles, &$typeManager) {
	foreach ($options as $option) {
		$sourceConfName = $option['conf'];
		$sourceTypeId = $option['typeId'];
		$sourceName = $option['name'];
		$sourceTypeManager = new \LAM\TYPES\TypeManager($serverProfiles[$sourceConfName]);
		$sourceType = $sourceTypeManager->getConfiguredType($sourceTypeId);
		$targetType = $typeManager->getConfiguredType($typeId);
		if (($sourceType != null) && ($targetType != null)) {
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
 * @param \LAM\TYPES\TypeManager $typeManager type manager
 * @return \htmlStatusMessage message or null
 */
function exportStructures($typeId, $name, $options, &$serverProfiles, &$typeManager) {
	$sourceType = $typeManager->getConfiguredType($typeId);
	if ($sourceType == null) {
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
			$targetTypeManager = new \LAM\TYPES\TypeManager($serverProfiles[$targetConfName]);
			$targetType = $targetTypeManager->getConfiguredType($targetTypeId);
			if ($targetType != null) {
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
