<?php
namespace LAM\CONFIG;
use htmlButton;
use htmlGroup;
use htmlInputFileUpload;
use htmlLink;
use htmlOutputText;
use htmlResponsiveInputCheckbox;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlStatusMessage;
use htmlSubTitle;
use LAM\PERSISTENCE\ConfigDataExporter;
use LAM\PERSISTENCE\ConfigDataImporter;
use LAMCfgMain;
use LAMException;
use ZipArchive;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020 - 2024  Roland Gruber

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
* Import and export functions for LAM configuration.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to persistence functions */
include_once('../../lib/persistence.inc');
/** account modules */
include_once('../../lib/modules.inc');

// start session
if (isFileBasedSession()) {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

if (!isset($_SESSION['cfgMain'])) {
	$cfg = new LAMCfgMain();
	$_SESSION['cfgMain'] = $cfg;
}
$cfg = &$_SESSION['cfgMain'];

// export
if (isset($_POST['exportConfig']) && $cfg->checkPassword($_SESSION["mainconf_password"])) {
	$exporter = new ConfigDataExporter();
	if (!headers_sent()) {
		header('Content-Type: application/zip; charset=utf-8');
		header('Content-disposition: attachment; filename=lam-config.zip');
	}
	try {
		$zip = new ZipArchive();
		$zipTmpFile = tmpfile();
		if ($zipTmpFile === false) {
		    throw new LAMException(_('Unable to create temporary file.'));
        }
        $metaData = stream_get_meta_data($zipTmpFile);
        if (empty($metaData['uri'])) {
            throw new LAMException(_('Unable to create temporary file.'));
        }
		$zipFile = $metaData['uri'];
		fclose($zipTmpFile);
		$zip->open($zipFile, ZipArchive::CREATE);
		$json = $exporter->exportAsJson();
		$zip->addFromString('lam-config.json', $json);
		$zip->close();
        $handle = fopen($zipFile, "r");
        if ($handle === false) {
			throw new LAMException(_('Unable to create temporary file.'));
        }
        $fileSize = filesize($zipFile);
		if ($fileSize === false) {
			throw new LAMException(_('Unable to create temporary file.'));
		}
        $contents = fread($handle, $fileSize);
        fclose($handle);
		unlink($zipFile);
        echo $contents;
    }
	catch (LAMException $e) {
	    logNewMessage(LOG_ERR, $e->getTitle() . ' ' . $e->getMessage());
    }
	exit;
}

echo $_SESSION['header'];
printHeaderContents(_("Import and export configuration"), '../..');

?>
	</head>
	<body>
    <?php
        // include all JavaScript files
        printJsIncludes('../..');
    ?>
    <div id="lam-topnav" class="lam-header">
        <div class="lam-header-left lam-menu-stay">
            <a href="https://www.ldap-account-manager.org/" target="new_window">
                <img class="align-middle" width="24" height="24" alt="help" src="../../graphics/logo24.png">
                <span class="hide-on-mobile">
                        <?php
                        echo getLAMVersionText();
                        ?>
                    </span>
            </a>
        </div>
	    <?php
	    if (is_dir(__DIR__ . '/../../docs/manual')) {
		    ?>
            <a class="lam-header-right lam-menu-icon hide-on-tablet" href="javascript:void(0);" class="icon" onclick="window.lam.topmenu.toggle();">
                <img class="align-middle" width="16" height="16" alt="menu" src="../../graphics/menu.svg">
                <span class="padding0">&nbsp;</span>
            </a>
            <a class="lam-header-right lam-menu-entry" target="_blank" href="../../docs/manual/index.html">
                <span class="padding0">&nbsp;<?php echo _("Help") ?></span>
            </a>
		    <?php
	    }
	    ?>
    </div>
    <form action="confImportExport.php" method="post" autocomplete="off" enctype="multipart/form-data">
    <br><br>
    <?php

    // check if user is logged in
    if (!isset($_POST['submitLogin']) && !isset($_SESSION["mainconf_password"])) {
        showLoginDialog();
        exit();
    }

    // check login
    if (isset($_POST['submitLogin']) && !checkLogin($cfg)) {
	    exit();
    }

    displayImportExport();

    /**
     * Shows the login dialog for the configuration master password.
     *
     * @param htmlStatusMessage $message message to show if any error occurred
     */
	function showLoginDialog($message = null): void {
		$content = new htmlResponsiveRow();
		$loginContent = new htmlResponsiveRow();
		$loginContent->setCSSClasses(['maxrow fullwidth roundedShadowBox spacing5']);
		if ($message !== null) {
		    $loginContent->add($message, 12);
        }
		$pwdInput = new htmlResponsiveInputField(_("Master password"), 'password', '', '236');
		$pwdInput->setIsPassword(true);
		$pwdInput->setCSSClasses(['lam-initial-focus']);
		$loginContent->add($pwdInput, 12);
		$loginContent->addLabel(new htmlOutputText('&nbsp;', false));
		$loginButton = new htmlButton('submitLogin', _("Ok"));
		$loginButton->setCSSClasses(['lam-primary']);
		$loginContent->addField($loginButton);

		$content->add($loginContent, 12);

		parseHtml(null, $content, [], false, null);
		renderBackLink();
	}

    /**
     * Renders the link back to login page.
     */
	function renderBackLink(): void {
		$content = new htmlResponsiveRow();
        $content->addVerticalSpacer('2rem');
        $content->add(new htmlLink(_('Back to login'), '../login.php'), 12);
		$content->addVerticalSpacer('1rem');
		parseHtml(null, $content, [], false, null);
    }

    /**
     * Checks the login password.
     *
     * @param LAMCfgMain $cfg main config
     * @return bool login ok
     */
	function checkLogin($cfg): bool {
        $password = $_POST['password'];
        if ($cfg->checkPassword($password)) {
	        $_SESSION["mainconf_password"] = $password;
            return true;
        }
        showLoginDialog(new htmlStatusMessage('ERROR', _('The password is invalid! Please try again.')));
        return false;
    }

    /**
     * Displays the import/export functions.
     */
    function displayImportExport(): void {
	    $content = new htmlResponsiveRow();

	    $content->add(new htmlSubTitle(_('Export')), 12);
	    $exportButton = new htmlButton('exportConfig', _('Export'));
	    $exportButton->setCSSClasses(['lam-primary']);
	    $content->add($exportButton);

	    $content->add(new htmlSubTitle(_('Import')), 12);
	    renderImportPart($content);

	    parseHtml(null, $content, [], false, null);
	    renderBackLink();
    }

    /**
     * Renders the import area.
     *
     * @param htmlResponsiveRow $content content where to add import part
     */
    function renderImportPart($content): void {
        $validUpload = false;
        $importSteps = [];
        if (isset($_POST['importConfig'])) {
	        try {
	            if (empty($_FILES['import-file']['tmp_name'])) {
	                throw new LAMException(_('The file you uploaded is too large. Please check php.ini, upload_max_filesize setting.'));
                }
	            $uploadFileName = $_FILES['import-file']['name'];
	            $tmpFileName = $_FILES['import-file']['tmp_name'];
	            if (preg_match('/\\.zip$/', $uploadFileName)) {
                    $zip = new ZipArchive();
                    $zip->open($tmpFileName);
                    $data = $zip->getFromName('lam-config.json');
                    if ($data === false) {
	                    throw new LAMException(_('Unable to read import file.'));
                    }
                    $zip->close();
                }
	            else {
		            $handle = fopen($tmpFileName, "r");
		            if ($handle === false) {
						throw new LAMException(_('Unable to read import file.'));
                    }
		            $data = fread($handle, 100_000_000);
					if ($data === false) {
						throw new LAMException(_('Unable to read import file.'));
					}
		            fclose($handle);
                }
	            $importer = new ConfigDataImporter();
		        $importSteps = $importer->getPossibleImportSteps($data);
		        $tmpFile = __DIR__ . '/../../tmp/internal/import_' . generateRandomText() . '.tmp';
		        $file = @fopen($tmpFile, "w");
		        if ($file) {
			        fputs($file, $data);
			        fclose($file);
			        chmod($tmpFile, 0600);
		        }
		        $_SESSION['configImportFile'] = $tmpFile;
	            $validUpload = true;
            }
            catch (LAMException $e) {
                $content->add(new htmlStatusMessage('ERROR', htmlspecialchars($e->getTitle()), htmlspecialchars($e->getMessage())));
            }
        }
        if (!isset($_POST['importConfigConfirm']) && !$validUpload) {
	        $content->add(new htmlInputFileUpload('import-file'), 12);
	        $submitButton = new htmlButton('importConfig', _('Submit'));
	        $submitButton->setCSSClasses(['lam-secondary']);
	        $content->add($submitButton);
        }
        elseif (isset($_POST['importConfig'])) {
            $content->add(new htmlOutputText(_('Import steps')), 12);
            foreach ($importSteps as $importStep) {
                $stepKey = 'step_' . $importStep->getKey();
                $stepCheckbox = new htmlResponsiveInputCheckbox($stepKey, true, $importStep->getLabel());
                $stepCheckbox->setLabelAfterCheckbox();
                $stepCheckbox->setCSSClasses(['bold']);
                $subStepIds = [];
                $content->add($stepCheckbox);
	            $content->addVerticalSpacer('0.3rem');
                foreach ($importStep->getSubSteps() as $subStep) {
                    $subStepKey = 'step_' . $subStep->getKey();
                    $subStepIds[] = $subStepKey;
	                $subStepCheckbox = new htmlResponsiveInputCheckbox($subStepKey, true, $subStep->getLabel());
	                $subStepCheckbox->setLabelAfterCheckbox();
	                $content->add($subStepCheckbox);
                }
                $stepCheckbox->setTableRowsToShow($subStepIds);
                $content->addVerticalSpacer('1rem');
            }
            $buttonGroup = new htmlGroup();
            $importButton = new htmlButton('importConfigConfirm', _('Import'));
            $importButton->setCSSClasses(['lam-secondary']);
	        $buttonGroup->addElement($importButton);
	        $buttonGroup->addElement(new htmlButton('importCancel', _('Cancel')));
	        $content->add($buttonGroup);
        }
        elseif (isset($_POST['importConfigConfirm'])) {
	        try {
				$handle = fopen($_SESSION['configImportFile'], "r");
				if ($handle === false) {
					throw new LAMException(_('Unable to read import file.'));
				}
				$data = fread($handle, 100_000_000);
				if ($data === false) {
					throw new LAMException(_('Unable to read import file.'));
				}
				fclose($handle);
		        $importer = new ConfigDataImporter();
		        $importSteps = $importer->getPossibleImportSteps($data);
		        foreach ($importSteps as $importStep) {
			        $importStep->setActive(isset($_POST['step_' . $importStep->getKey()]));
			        foreach ($importStep->getSubSteps() as $subStep) {
				        $subStep->setActive(isset($_POST['step_' . $subStep->getKey()]));
                    }
		        }
		        $importer->runImport($importSteps);
		        unlink($_SESSION['configImportFile']);
		        $content->add(new htmlStatusMessage('INFO', _('Configuration import ended successful.')));
		        $content->add(new htmlButton('importNew', _('New import')));
	        }
	        catch (LAMException $e) {
		        $content->add(new htmlStatusMessage('ERROR', htmlspecialchars($e->getTitle()), htmlspecialchars($e->getMessage())));
		        $content->add(new htmlButton('importCancel', _('Back')));
	        }
        }
    }

	?>
    </form>
	</body>
</html>
