<?php
namespace LAM\TOOLS\PDF_EDITOR;
use htmlDiv;
use htmlForm;
use htmlResponsiveInputField;
use htmlResponsiveSelect;
use \htmlTitle;
use \htmlStatusMessage;
use LAM\PDF\PdfStructurePersistenceManager;
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
use LAMException;
use ServerProfilePersistenceManager;
use function LAM\PDF\getPDFStructures;

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
if (!checkIfWriteAccessIsAllowed()) {
    die();
}

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

$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();

$container = new htmlResponsiveRow();
$container->add(new htmlTitle(_('PDF editor')), 12);

if (isset($_POST['deleteProfile']) && ($_POST['deleteProfile'] == 'true')) {
	$typeToDelete = $typeManager->getConfiguredType($_POST['profileDeleteType']);
	// delete structure
    try {
        $pdfStructurePersistenceManager->deletePdfStructure($_SESSION['config']->getName(), $_POST['profileDeleteType'], $_POST['profileDeleteName']);
	    $message = new htmlStatusMessage('INFO', _('Deleted PDF structure.'), $typeToDelete->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
	    $container->add($message, 12);
    }
    catch (LAMException $e) {
	    $message = new htmlStatusMessage('ERROR', _('Unable to delete PDF structure!'), $typeToDelete->getAlias() . ': ' . htmlspecialchars($_POST['profileDeleteName']));
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
			$pdfStructurePersistenceManager->deletePdfStructureTemplate($selectedScope, $selectedName);
			$container->add(new htmlStatusMessage('INFO', _('Deleted profile.'), $selectedName), 12);
		} catch (LAMException $e) {
			$container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
		}
	}
}

// delete global logo
if (isset($_POST['deleteGlobalLogo']) && !empty($_POST['globalLogoDelete'])) {
	$cfg = new LAMCfgMain();
	if (empty($_POST['globalLogoDeletePassword']) || !$cfg->checkPassword($_POST['globalLogoDeletePassword'])) {
		$container->add(new htmlStatusMessage('ERROR', _('Master password is wrong!')), 12);
	}
	else {
		$selectedLogo = $_POST['globalLogoDelete'];
		try {
		    $pdfStructurePersistenceManager->deletePdfTemplateLogo($selectedLogo);
			$container->add(new htmlStatusMessage('INFO', _('Logo file deleted.'), $selectedLogo), 12);
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
	$handle = fopen($file, "r");
	$data = fread($handle, 100000000);
	fclose($handle);
	$filename = $_FILES['logoUpload']['name'];
	try {
		$pdfStructurePersistenceManager->savePdfLogo($_SESSION['config']->getName(), $filename, $data);
		$container->add(new htmlStatusMessage('INFO', _('Uploaded logo file.'), $filename), 12);
    }
    catch (LAMException $e) {
	    $container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
    }
}

// delete logo file
if (isset($_POST['delLogo'])) {
	$toDel = $_POST['logo'];
	try {
	    $pdfStructurePersistenceManager->deletePdfLogo($_SESSION['config']->getName(), $toDel);
		$container->add(new htmlStatusMessage('INFO', _('Logo file deleted.'), $toDel), 12);
    }
    catch (LAMException $e) {
	    $container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
    }
}

// export logo
if (!empty($_POST['exportLogoTargetProfile'])) {
	$cfg = new LAMCfgMain();
	if (!$cfg->checkPassword($_POST['exportLogoPassword'])) {
		$container->add(new htmlStatusMessage('ERROR', _('Master password is wrong!')), 12);
	}
	else {
		try {
	        foreach ($_POST['exportLogoTargetProfile'] as $targetProfile) {
	            $fileName = $_POST['exportLogoName'];
	            $binary = $pdfStructurePersistenceManager->getPdfLogoBinary($_SESSION['config']->getName(), $fileName);
	            if ($targetProfile === 'templates*') {
	                $pdfStructurePersistenceManager->savePdfTemplateLogo($fileName, $binary);
                }
	            else {
	                $pdfStructurePersistenceManager->savePdfLogo($targetProfile, $fileName, $binary);
                }
            }
			$container->add(new htmlStatusMessage('INFO', _('Exported logo file.')), 12);
		}
		catch (LAMException $e) {
			$container->add(new htmlStatusMessage($e->getTitle(), $e->getMessage()), 12);
		}
    }
}

// import logo
if (!empty($_POST['importLogoSourceProfile'])) {
	$cfg = new LAMCfgMain();
	if (!$cfg->checkPassword($_POST['importLogoPassword'])) {
		$container->add(new htmlStatusMessage('ERROR', _('Master password is wrong!')), 12);
	}
	else {
		try {
			foreach ($_POST['importLogoSourceProfile'] as $sourceLogo) {
				$parts = explode('##', $sourceLogo);
				if (sizeof($parts) !== 2) {
				    continue;
                }
				$profileName = $parts[0];
				$fileName = $parts[1];
				$binary = $pdfStructurePersistenceManager->getPdfLogoBinary($profileName, $fileName);
				$pdfStructurePersistenceManager->savePdfLogo($_SESSION['config']->getName(), $fileName, $binary);
			}
			$container->add(new htmlStatusMessage('INFO', _('Logo import successful.')), 12);
		}
		catch (LAMException $e) {
			$container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
		}
	}
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
		'templates' => $pdfStructurePersistenceManager->getPDFStructures($_SESSION['config']->getName(), $type->getId()));
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
<div class="smallPaddingContent">
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
			$createButton = new htmlButton('createNewTemplate', _('Create'));
			$createButton->setCSSClasses(array('lam-primary'));
			$container->addField($createButton);
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
    	$logoOptions = array();
        try {
            $logos = $pdfStructurePersistenceManager->getPdfLogos($_SESSION['config']->getName(), true);
	        foreach ($logos as $logo) {
		        $file = $logo->getName();
		        $label = $file . ' (' . $logo->getWidth() . ' x ' . $logo->getHeight() . ")";
		        $logoOptions[$label] = $file;
	        }
        } catch (LAMException $e) {
            $container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()));
        }
		$logoSelect = new htmlSelect('logo', $logoOptions, null);
		$logoSelect->setHasDescriptiveElements(true);
		$container->addLabel($logoSelect);
		$logoButtonGroup = new htmlGroup();
		$delLogo = new htmlButton('delLogo', 'delete.png', true);
		$delLogo->setTitle(_('Delete'));
	    $logoButtonGroup->addElement($delLogo);
        $importLogoLink = new htmlLink(null, '#', '../../graphics/import.png');
	    $importLogoLink->setTitle(_('Import logo'));
	    $importLogoLink->setOnClick("window.lam.profilePdfEditor.showPdfLogoImportDialog('" . _("Import logo") . "', '" .
            _('Ok') . "', '" . _('Cancel') . "'); return false;");
    	$importLogoLink->setCSSClasses(array('margin3'));
        $logoButtonGroup->addElement($importLogoLink);
        $exportLogoLink = new htmlLink(null, '#', '../../graphics/export.png');
	    $exportLogoLink->setTitle(_('Export logo'));
    	$exportLogoLink->setOnClick("window.lam.profilePdfEditor.showPdfLogoExportDialog('" . _("Export logo") . "', '" .
            _('Ok') . "', '" . _('Cancel') . "'); return false;");
	    $exportLogoLink->setCSSClasses(array('margin3'));
	    $logoButtonGroup->addElement($exportLogoLink);
    	$container->addField($logoButtonGroup);
		$container->addVerticalSpacer('2rem');
		$container->addLabel(new htmlInputFileUpload('logoUpload'));
		$logoUpload = new htmlButton('uploadLogo', _('Upload'));
		$logoUpload->setCSSClasses(array('lam-secondary'));
		$container->addField($logoUpload);

		$container->addVerticalSpacer('4rem');
		// generate content
		$tabindex = 1;
		parseHtml(null, $container, array(), false, $tabindex, 'user');

		echo "</form>\n";

		// export logo form
    	$container = new htmlResponsiveRow();
        $logoExportFormContent = new htmlResponsiveRow();
        $exportOptions = array();
        foreach ($configProfiles as $profile) {
            if ($profile != $_SESSION['config']->getName()) {
                $exportOptions[$profile] = $profile;
            }
        }
        asort($exportOptions);
        $exportOptions['*' . _('Global templates')] = 'templates*';
        $logoExportConfigSelect = new htmlResponsiveSelect('exportLogoTargetProfile', $exportOptions, array(), _('Target server profile'), null, 5);
        $logoExportConfigSelect->setHasDescriptiveElements(true);
        $logoExportConfigSelect->setSortElements(false);
        $logoExportConfigSelect->setMultiSelect(true);
        $logoExportFormContent->add($logoExportConfigSelect, 12);
        $logoExportFormContent->addVerticalSpacer('1rem');
        $logoExportFormContent->addLabel(new htmlOutputText(''));
        $logoExportFormContent->addField(new htmlHiddenInput('exportLogoName', null));
        $logoExportFormPwd = new htmlResponsiveInputField(_("Master password"), 'exportLogoPassword', null, '236');
        $logoExportFormPwd->setIsPassword(true);
        $logoExportFormContent->add($logoExportFormPwd, 12);
        addSecurityTokenToMetaHTML($logoExportFormContent);
        $logoExportForm = new htmlForm('logoExportForm', 'pdfmain.php', $logoExportFormContent);
        $logoExportDialog = new htmlDiv('logoExportDiv', $logoExportForm, array('hidden'));
        $container->add($logoExportDialog, 12);
    	parseHtml(null, $container, array(), false, $tabindex, 'user');

        // import logo form
        $container = new htmlResponsiveRow();
        $logoImportFormContent = new htmlResponsiveRow();
        $importOptions = array();
        foreach ($configProfiles as $profileName) {
            if ($profileName != $_SESSION['config']->getName()) {
                $availableLogos = $pdfStructurePersistenceManager->getPdfLogos($profileName);
                foreach ($availableLogos as $availableLogo) {
                    $fileName = $availableLogo->getName();
	                $importOptions[$profileName][$fileName] = $profileName . '##' . $fileName;
                }
            }
        }
        $logoImportConfigSelect = new htmlResponsiveSelect('importLogoSourceProfile', $importOptions, array(), _('PDF structures'), null, 5);
        $logoImportConfigSelect->setHasDescriptiveElements(true);
        $logoImportConfigSelect->setContainsOptgroups(true);
        $logoImportConfigSelect->setMultiSelect(true);
        $logoImportFormContent->add($logoImportConfigSelect, 12);
        $logoImportFormContent->addVerticalSpacer('1rem');
        $logoImportFormContent->addLabel(new htmlOutputText(''));
        $logoImportFormPwd = new htmlResponsiveInputField(_("Master password"), 'importLogoPassword', null, '236');
	    $logoImportFormPwd->setIsPassword(true);
    	$logoImportFormContent->add($logoImportFormPwd, 12);
        addSecurityTokenToMetaHTML($logoImportFormContent);
        $logoImportForm = new htmlForm('logoImportForm', 'pdfmain.php', $logoImportFormContent);
        $logoImportDialog = new htmlDiv('logoImportDiv', $logoImportForm, array('hidden'));
        $container->add($logoImportDialog, 12);
        parseHtml(null, $container, array(), false, $tabindex, 'user');

	    foreach ($templateClasses as $templateClass) {
			$typeId = $templateClass['typeId'];
			$scope = $templateClass['scope'];
			$importOptions = array();
			foreach ($configProfiles as $profile) {
				$typeManagerImport = new TypeManager($serverProfiles[$profile]);
				$typesImport = $typeManagerImport->getConfiguredTypesForScope($scope);
				foreach ($typesImport as $typeImport) {
					if (($profile != $_SESSION['config']->getName()) || ($typeImport->getId() != $typeId)) {
						$accountProfiles = $pdfStructurePersistenceManager->getPDFStructures($profile, $typeImport->getId());
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

// delete global templates
$globalTemplates = $pdfStructurePersistenceManager->getPdfStructureTemplateNames();
$globalDeletableTemplates = array();
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
    $globalTemplateDeleteForm = new htmlForm('deleteGlobalTemplatesForm', 'pdfmain.php', $container);
    parseHtml(null, $globalTemplateDeleteForm, array(), false, $tabindex, 'user');
}

// delete global PDF logos
$globalPdfLogos = $pdfStructurePersistenceManager->getPdfTemplateLogoNames();
if (!empty($globalPdfLogos)) {
	$container = new htmlResponsiveRow();
	$globalLogosSubtitle = new htmlSubTitle(_('Global template logos'));
	$globalLogosSubtitle->setHelpId('365');
	$container->add($globalLogosSubtitle, 12);
	$globalTemplateLogosSelect = new htmlResponsiveSelect('globalLogoDelete', $globalPdfLogos, array(), _('Delete'));
	$container->add($globalTemplateLogosSelect, 12);
	$globalLogoDeleteDialogPassword = new htmlResponsiveInputField(_("Master password"), 'globalLogoDeletePassword', null, '236');
	$globalLogoDeleteDialogPassword->setIsPassword(true);
	$globalLogoDeleteDialogPassword->setRequired(true);
	$container->add($globalLogoDeleteDialogPassword, 12);
	$container->addVerticalSpacer('1rem');
	$globalLogoDeleteButton = new htmlButton('deleteGlobalLogoButton', _('Delete'));
	$globalLogoDeleteButton->setCSSClasses(array('lam-danger'));
	$globalLogoDeleteButton->setOnClick("showConfirmationDialog('" . _("Delete") . "', '" .
		_('Ok') . "', '" . _('Cancel') . "', 'globalLogoDeleteDialog', 'deleteGlobalLogoForm', undefined); return false;");
	$container->addLabel(new htmlOutputText('&nbsp;', false));
	$container->addField($globalLogoDeleteButton, 12);
	addSecurityTokenToMetaHTML($container);
	$globalLogoDeleteDialogContent = new htmlResponsiveRow();
	$globalLogoDeleteDialogContent->add(new htmlOutputText(_('Do you really want to delete this logo?')), 12);
	$globalLogoDeleteDialogContent->add(new htmlHiddenInput('deleteGlobalLogo', 'true'), 12);
	$globalLogoDeleteDialogDiv = new htmlDiv('globalLogoDeleteDialog', $globalLogoDeleteDialogContent, array('hidden'));
	$container->add($globalLogoDeleteDialogDiv, 12);
	$container->addVerticalSpacer('1rem');
	$globalLogoDeleteForm = new htmlForm('deleteGlobalLogoForm', 'pdfmain.php', $container);
	parseHtml(null, $globalLogoDeleteForm, array(), false, $tabindex, 'user');
}

echo "</div>\n";

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
	$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
	foreach ($options as $option) {
		$sourceConfName = $option['conf'];
		$sourceTypeId = $option['typeId'];
		$sourceName = $option['name'];
		$sourceTypeManager = new TypeManager($serverProfiles[$sourceConfName]);
		$sourceType = $sourceTypeManager->getConfiguredType($sourceTypeId);
		$targetType = $typeManager->getConfiguredType($typeId);
		if (($sourceType !== null) && ($targetType !== null)) {
			try {
			    $structure = $pdfStructurePersistenceManager->readPdfStructure($sourceConfName, $sourceTypeId, $sourceName);
			    $pdfStructurePersistenceManager->savePdfStructure($_SESSION['config']->getName(), $sourceTypeId, $sourceName, $structure);
			}
			catch (LAMException $e) {
				return new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
			}
		}
	}
	return new htmlStatusMessage('INFO', _('Import successful'));
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
function exportStructures($typeId, $name, $options, &$serverProfiles, TypeManager &$typeManager): ?htmlStatusMessage {
	$sourceType = $typeManager->getConfiguredType($typeId);
	if ($sourceType === null) {
		return null;
	}
	$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();
	foreach ($options as $option) {
		$targetConfName = $option['conf'];
		if ($targetConfName == 'templates*') {
			try {
				$structure = $pdfStructurePersistenceManager->readPdfStructure($_SESSION['config']->getName(), $typeId, $name);
				$pdfStructurePersistenceManager->savePdfStructureTemplate($sourceType->getScope(), $name, $structure);
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
				    $structure = $pdfStructurePersistenceManager->readPdfStructure($_SESSION['config']->getName(), $typeId, $name);
				    $pdfStructurePersistenceManager->savePdfStructure($targetConfName, $targetTypeId, $name, $structure);
				}
				catch (LAMException $e) {
					return new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
				}
			}
		}
	}
	return new htmlStatusMessage('INFO', _('Export successful'));
}
