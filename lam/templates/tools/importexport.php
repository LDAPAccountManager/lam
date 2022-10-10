<?php
namespace LAM\TOOLS\IMPORT_EXPORT;
use \htmlTitle;
use \htmlResponsiveRadio;
use \htmlResponsiveRow;
use \htmlResponsiveInputFileUpload;
use \htmlResponsiveInputTextarea;
use \htmlButton;
use \htmlStatusMessage;
use \htmlDiv;
use \htmlOutputText;
use \htmlJavaScript;
use LAM\TOOLS\TREEVIEW\TreeViewTool;
use \LAMException;
use \htmlLink;
use \htmlResponsiveInputCheckbox;
use \htmlResponsiveSelect;
use \htmlResponsiveInputField;
use \htmlHiddenInput;
use LAM\TYPES\TypeManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018 - 2022  Roland Gruber

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
* Multi edit tool that allows LDAP operations on multiple entries.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration data */
include_once(__DIR__ . "/../../lib/config.inc");
/** access LDAP server */
include_once(__DIR__ . "/../../lib/ldap.inc");
/** used to print status messages */
include_once(__DIR__ . "/../../lib/status.inc");
/** import class */
include_once(__DIR__ . "/../../lib/import.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) {
	die();
}

checkIfToolIsActive('ImportExport');

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

// clean old data
if (isset($_SESSION[Importer::SESSION_KEY_TASKS])) {
	unset($_SESSION[Importer::SESSION_KEY_TASKS]);
}
if (isset($_SESSION[Importer::SESSION_KEY_COUNT])) {
	unset($_SESSION[Importer::SESSION_KEY_COUNT]);
}
if (isset($_SESSION[Importer::SESSION_KEY_STOP_ON_ERROR])) {
	unset($_SESSION[Importer::SESSION_KEY_STOP_ON_ERROR]);
}

include __DIR__ . '/../../lib/adminHeader.inc';
$tabindex = 1;
$activeTab = 0;
if (!empty($_GET['tab']) && ($_GET['tab'] === 'export')) {
	$activeTab = 1;
}

?>

<script>
  $(function() {
	jQuery("#tabs").tabs({
        active: <?php echo $activeTab; ?>
    });
  });
</script>

<div class="smallPaddingContent">
	<div id="tabs">
		<ul>
			<li id="tab_import">
				<a href="#tab-import"><img alt="import" src="../../graphics/import.svg"> <?php echo _('Import') ?> </a>
			</li>
			<li id="tab_export">
				<a href="#tab-export"><img alt="export" src="../../graphics/export.svg"> <?php echo _('Export') ?> </a>
			</li>
		</ul>
		<div id="tab-import">
			<?php
				if (isset($_POST['submitImport'])) {
					printImportTabProcessing($tabindex);
				}
				else {
					printImportTabContent($tabindex);
				}
			?>
		</div>
		<div id="tab-export">
			<?php
				if (isset($_POST['submitExport'])) {
					printExportTabProcessing($tabindex);
				}
				else {
					printExportTabContent($tabindex);
				}
			?>
		</div>
	</div>
</div>

<?php

/**
 * Prints the content area for the import tab.
 *
 * @param int $tabindex tabindex
 */
function printImportTabContent(&$tabindex): void {
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Import")), 12);
	$sources = array(
		_('Text input') => 'text',
		_('File') => 'file',
	);
	$sourceRadio = new htmlResponsiveRadio(_('Source'), 'source', $sources, 'text');
	$sourceRadio->setTableRowsToHide(
		array(
			'file' => array('text'),
			'text' => array('file')
		)
	);
	$sourceRadio->setTableRowsToShow(
		array(
			'text' => array('text'),
			'file' => array('file')
		)
	);
	$container->add($sourceRadio, 12);
	$container->addVerticalSpacer('1rem');
	$container->add(new htmlResponsiveInputFileUpload('file', _('File'), '750'), 12);
	$container->add(new htmlResponsiveInputTextarea('text', '', '60', '20', _('LDIF data'), '750'), 12);
	$container->add(new htmlResponsiveInputCheckbox('noStop', false, _('Don\'t stop on errors')), 12);

	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitImport', _('Submit'));
	$container->add($button, 12, 12, 12, 'text-center');

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo "</form>\n";
}

/**
 * Prints the content area for the import tab during processing state.
 *
 * @param int $tabindex tabindex
 */
function printImportTabProcessing(&$tabindex): void {
	try {
		checkImportData();
	}
	catch (LAMException $e) {
		$container = new htmlResponsiveRow();
		$container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
		parseHtml(null, $container, array(), false, $tabindex, 'user');
		printImportTabContent($tabindex);
		return;
	}
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Import")), 12);

	$container->add(new htmlDiv('statusImportInprogress', new htmlOutputText(_('Status') . ': ' . _('in progress'))), 12);
	$container->add(new htmlDiv('statusImportDone', new htmlOutputText(_('Status') . ': ' . _('done')), array('hidden')), 12);
	$container->add(new htmlDiv('statusImportFailed', new htmlOutputText(_('Status') . ': ' . _('failed')), array('hidden')), 12);
	$container->addVerticalSpacer('1rem');
	$container->add(new htmlDiv('progressbarImport', new htmlOutputText('')), 12);
	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitImportCancel', _('Cancel'));
	$container->add($button, 12, 12, 12, 'text-center');

	$newImportButton = new htmlLink(_('New import'), null, null, true);
	$container->add($newImportButton, 12, 12, 12, 'text-center hidden newimport');

	$container->addVerticalSpacer('3rem');

	$container->add(new htmlDiv('importResults', new htmlOutputText('')), 12);
	$container->add(new htmlJavaScript(
			'window.lam.importexport.startImport(\'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');'
		), 12);

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo "</form>\n";
}

/**
 * Checks if the import data is ok.
 *
 * @throws LAMException error message if not valid
 */
function checkImportData(): void {
	$source = $_POST['source'];
	if ($source == 'text') {
		$ldif = $_POST['text'];
	}
	else {
		$handle = fopen($_FILES['file']['tmp_name'], "r");
		if ($handle === false) {
			throw new LAMException(_('Unable to create temporary file.'));
		}
		$ldif = fread($handle, 100000000);
		if ($ldif === false) {
			throw new LAMException(_('Unable to create temporary file.'));
		}
		fclose($handle);
	}
	if (empty($ldif)) {
		throw new LAMException(_('You must either upload a file or provide an import in the text box.'));
	}
	$lines = preg_split("/\n|\r\n|\r/", $ldif);
	$importer = new Importer();
	$tasks = $importer->getTasks($lines);
	$_SESSION[Importer::SESSION_KEY_TASKS] = $tasks;
	$_SESSION[Importer::SESSION_KEY_COUNT] = sizeof($tasks);
	$_SESSION[Importer::SESSION_KEY_STOP_ON_ERROR] = (!isset($_POST['noStop']) || ($_POST['noStop'] != 'on'));
}

/**
 * Prints the content area for the export tab.
 *
 * @param int $tabindex tabindex
 */
function printExportTabContent(&$tabindex): void {
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php?tab=export\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Export")), 12);

	$baseDn = getDefaultBaseDn();
	if (!empty($_GET['dn'])) {
	    $preSetDn = base64_decode($_GET['dn']);
	    if (isValidExportDn($preSetDn)) {
	        $baseDn = $preSetDn;
        }
    }
	$baseDnField = new htmlResponsiveInputField(_('Base DN'), 'baseDn', $baseDn, '751', true);
	$baseDnField->showDnSelection();
	$container->add($baseDnField, 12);

	$searchScopes = array(
		_('Base (base dn only)') => 'base',
		_('One (one level beneath base)') => 'one',
		_('Sub (entire subtree)') => 'sub'
	);
	$searchScopeSelect = new htmlResponsiveSelect('searchScope', $searchScopes, array('sub'), _('Search scope'));
	$searchScopeSelect->setHasDescriptiveElements(true);
	$searchScopeSelect->setSortElements(false);
	$container->add($searchScopeSelect, 12);
	$container->add(new htmlResponsiveInputField(_('Search filter'), 'filter', '(objectClass=*)', '752'), 12);
	$container->add(new htmlResponsiveInputField(_('Attributes'), 'attributes', '*', '753'), 12);
	$container->add(new htmlResponsiveInputCheckbox('includeSystem', false, _('Include system attributes'), '754'), 12);
	$container->add(new htmlResponsiveInputCheckbox('saveAsFile', false, _('Save as file')), 12);

	$formats = array(
		'CSV' => 'csv',
		'LDIF' => 'ldif'
	);
	$formatSelect = new htmlResponsiveSelect('format', $formats, array('ldif'), _('Export format'));
	$formatSelect->setHasDescriptiveElements(true);
	$formatSelect->setSortElements(false);
	$container->add($formatSelect, 12);

	$endings = array(
		'Windows' => 'windows',
		'Unix' => 'unix'
	);
	$endingsSelect = new htmlResponsiveSelect('ending', $endings, array('unix'), _('End of line'));
	$endingsSelect->setHasDescriptiveElements(true);
	$endingsSelect->setSortElements(false);
	$container->add($endingsSelect, 12);

	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitExport', _('Submit'));
	$container->add($button, 12, 12, 12, 'text-center');

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo "</form>\n";
}

/**
 * Returns the default base DN.
 *
 * @return string base DN
 */
function getDefaultBaseDn(): string {
	$typeManager = new TypeManager();
	$baseDn = '';
	foreach ($typeManager->getConfiguredTypes() as $type) {
		$suffix = $type->getSuffix();
		if (empty($baseDn) || (!empty($suffix) && (strlen($suffix) < strlen($baseDn)))) {
			$baseDn = $suffix;
		}
	}
	if ($_SESSION['config']->isToolActive('TreeViewTool')) {
		$treeSuffixes = TreeViewTool::getRootDns();
        if (empty($baseDn) || (!empty($treeSuffixes) && (strlen($treeSuffixes[0]) < strlen($baseDn)))) {
            $baseDn = $treeSuffixes[0];
        }
    }
	return $baseDn;
}

/**
 * Checks if the given DN is valid for exporting.
 *
 * @param string $dn DN
 * @return bool valid
 */
function isValidExportDn(string $dn): bool {
    $dn = strtolower($dn);
	$typeManager = new TypeManager();
	foreach ($typeManager->getConfiguredTypes() as $type) {
		$suffix = strtolower($type->getSuffix());
		if (substr($dn, -1 * strlen($suffix)) === $suffix) {
			return true;
		}
	}
	if ($_SESSION['config']->isToolActive('TreeViewTool')) {
	    $treeSuffixes = TreeViewTool::getRootDns();
	    foreach ($treeSuffixes as $treeSuffix) {
	        $treeSuffix = strtolower($treeSuffix);
		    if (substr($dn, -1 * strlen($treeSuffix)) === $treeSuffix) {
			    return true;
		    }
        }
	}
    return false;
}

/**
 * Prints the content area for the export tab during processing state.
 *
 * @param int $tabindex tabindex
 */
function printExportTabProcessing(&$tabindex): void {
	try {
		checkExportData();
	}
	catch (LAMException $e) {
		$container = new htmlResponsiveRow();
		$container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()), 12);
		parseHtml(null, $container, array(), false, $tabindex, 'user');
		printExportTabContent($tabindex);
		return;
	}
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php?tab=export\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Export")), 12);

	$container->add(new htmlHiddenInput('baseDn', $_POST['baseDn']), 12);
	$container->add(new htmlHiddenInput('searchScope', $_POST['searchScope']), 12);
	$container->add(new htmlHiddenInput('filter', $_POST['filter']), 12);
	$container->add(new htmlHiddenInput('attributes', $_POST['attributes']), 12);
	$container->add(new htmlHiddenInput('format', $_POST['format']), 12);
	$container->add(new htmlHiddenInput('ending', $_POST['ending']), 12);
	$container->add(new htmlHiddenInput('includeSystem', isset($_POST['includeSystem']) && ($_POST['includeSystem'] === 'on') ? 'true' : 'false'), 12);
	$container->add(new htmlHiddenInput('saveAsFile', isset($_POST['saveAsFile']) && ($_POST['saveAsFile'] === 'on') ? 'true' : 'false'), 12);

	$container->add(new htmlDiv('statusExportInprogress', new htmlOutputText(_('Status') . ': ' . _('in progress'))), 12);
	$container->add(new htmlDiv('statusExportDone', new htmlOutputText(_('Status') . ': ' . _('done')), array('hidden')), 12);
	$container->add(new htmlDiv('statusExportFailed', new htmlOutputText(_('Status') . ': ' . _('failed')), array('hidden')), 12);
	$container->addVerticalSpacer('1rem');
	$container->add(new htmlDiv('progressbarExport', new htmlOutputText('')), 12);
	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitExportCancel', _('Cancel'));
	$container->add($button, 12, 12, 12, 'text-center');

	$newExportButton = new htmlLink(_('New export'), null, null, true);
	$container->add($newExportButton, 12, 12, 12, 'text-center hidden newexport');

	$container->addVerticalSpacer('3rem');

	$exportText = new htmlOutputText('');
	$exportText->setPreformatted(true);
	$container->add(new htmlDiv('exportResults', $exportText), 12);
	$container->add(new htmlJavaScript(
			'window.lam.importexport.startExport(\'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');'
		), 12);

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo "</form>\n";
}

/**
 * Checks if the export data is ok.
 *
 * @throws LAMException error message if not valid
 */
function checkExportData(): void {
	if (empty($_POST['baseDn'])) {
		throw new LAMException(_('This field is required.'), _('Base DN'));
	}
}

include __DIR__ . '/../../lib/adminFooter.inc';
