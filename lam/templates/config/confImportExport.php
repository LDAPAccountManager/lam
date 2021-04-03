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
  Copyright (C) 2020 - 2021  Roland Gruber

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

// start session
if (strtolower(session_module_name()) == 'files') {
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
		$zipFile = stream_get_meta_data($zipTmpFile)['uri'];
		fclose($zipTmpFile);
		$zip->open($zipFile, ZipArchive::CREATE);
		$json = $exporter->exportAsJson();
		$zip->addFromString('lam-config.json', $json);
		$zip->close();
        $handle = fopen($zipFile, "r");
        $contents = fread($handle, filesize($zipFile));
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
	<body class="admin">
    <?php
        // include all JavaScript files
        printJsIncludes('../..');
    ?>
    <table class="lamTop ui-corner-all">
        <tr>
            <td align="left">
                <a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">
                    <span class="hide-on-tablet">&nbsp;</span>
                    <span class="hide-on-mobile">
                        <?php echo getLAMVersionText(); ?>
                    </span>
                </a>
            </td>
            <td align="right">
		        <?php
		        if (is_dir(__DIR__ . '/../../docs/manual')) {
                ?>
                    <a target="_blank" href="../../docs/manual/index.html"><img class="align-middle" width="16" height="16" alt="help" src="../../graphics/help.png">
                        <span class="hide-on-tablet">&nbsp;</span>
                        <span class="hide-on-mobile">
                            <?php echo _("Help") ?>&nbsp;
                        </span>
                    </a>
                <?php
		        }
		        ?>
            </td>
        </tr>
    </table>
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
	function showLoginDialog($message = null) {
    	$tabindex = 0;
		$content = new htmlResponsiveRow();
		$loginContent = new htmlResponsiveRow();
		$loginContent->setCSSClasses(array('maxrow fullwidth roundedShadowBox spacing5'));
		if ($message !== null) {
		    $loginContent->add($message, 12);
        }
		$pwdInput = new htmlResponsiveInputField(_("Master password"), 'password', '', '236');
		$pwdInput->setIsPassword(true);
		$pwdInput->setCSSClasses(array('lam-initial-focus'));
		$loginContent->add($pwdInput, 12);
		$loginContent->addLabel(new htmlOutputText('&nbsp;', false));
		$loginContent->addField(new htmlButton('submitLogin', _("Ok")));

		$content->add($loginContent, 12);

		parseHtml(null, $content, array(), false, $tabindex, null);
		renderBackLink();
	}

    /**
     * Renders the link back to login page.
     */
	function renderBackLink() {
		$tabindex = 0;
		$content = new htmlResponsiveRow();
        $content->addVerticalSpacer('2rem');
        $content->add(new htmlLink(_('Back to login'), '../login.php', '../../graphics/undo.png'), 12);
		$content->addVerticalSpacer('1rem');
		parseHtml(null, $content, array(), false, $tabindex, null);
    }

    /**
     * Checks the login password.
     *
     * @param LAMCfgMain $cfg main config
     * @return bool login ok
     */
	function checkLogin($cfg) {
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
    function displayImportExport() {
	    $tabindex = 0;
	    $content = new htmlResponsiveRow();

	    $content->add(new htmlSubTitle(_('Export')), 12);
	    $content->add(new htmlButton('exportConfig', _('Export')), 12);

	    $content->add(new htmlSubTitle(_('Import')), 12);
	    renderImportPart($content);

	    parseHtml(null, $content, array(), false, $tabindex, null);
	    renderBackLink();
    }

    /**
     * Renders the import area.
     *
     * @param htmlResponsiveRow $content content where to add import part
     */
    function renderImportPart($content) {
        $validUpload = false;
        $importSteps = array();
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
		            $data = fread($handle, 100000000);
		            fclose($handle);
                }
	            $importer = new ConfigDataImporter();
		        $importSteps = $importer->getPossibleImportSteps($data);
		        $tmpFile = __DIR__ . '/../../tmp/internal/import_' . getRandomNumber() . '.tmp';
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
                $content->add(new htmlStatusMessage('ERROR', htmlspecialchars($e->getTitle()), htmlspecialchars($e->getMessage())), 12);
            }
        }
        if (!isset($_POST['importConfigConfirm']) && !$validUpload) {
	        $content->add(new htmlInputFileUpload('import-file'), 12);
	        $content->add(new htmlButton('importConfig', _('Submit')), 12);
        }
        elseif (isset($_POST['importConfig'])) {
            $content->add(new htmlOutputText(_('Import steps')), 12);
            foreach ($importSteps as $importStep) {
                $stepKey = 'step_' . $importStep->getKey();
                $stepCheckbox = new htmlResponsiveInputCheckbox($stepKey, true, $importStep->getLabel());
                $stepCheckbox->setLabelAfterCheckbox();
                $stepCheckbox->setCSSClasses(array('bold'));
                $subStepIds = array();
                $content->add($stepCheckbox, 12);
	            $content->addVerticalSpacer('0.3rem');
                foreach ($importStep->getSubSteps() as $subStep) {
                    $subStepKey = 'step_' . $subStep->getKey();
                    $subStepIds[] = $subStepKey;
	                $subStepCheckbox = new htmlResponsiveInputCheckbox($subStepKey, true, $subStep->getLabel());
	                $subStepCheckbox->setLabelAfterCheckbox();
	                $content->add($subStepCheckbox, 12);
                }
                $stepCheckbox->setTableRowsToShow($subStepIds);
                $content->addVerticalSpacer('1rem');
            }
            $buttonGroup = new htmlGroup();
	        $buttonGroup->addElement(new htmlButton('importConfigConfirm', _('Import')));
	        $buttonGroup->addElement(new htmlButton('importCancel', _('Cancel')));
	        $content->add($buttonGroup, 12);
        }
        elseif (isset($_POST['importConfigConfirm'])) {
			$handle = fopen($_SESSION['configImportFile'], "r");
	        $data = fread($handle, 100000000);
	        fclose($handle);
	        try {
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
		        $content->add(new htmlStatusMessage('INFO', _('Configuration import ended successful.')), 12);
		        $content->add(new htmlButton('importNew', _('New import')), 12);
	        }
	        catch (LAMException $e) {
		        $content->add(new htmlStatusMessage('ERROR', htmlspecialchars($e->getTitle()), htmlspecialchars($e->getMessage())), 12);
		        $content->add(new htmlButton('importCancel', _('Back')), 12);
	        }
        }
    }

	?>
    </form>
	</body>
</html>
