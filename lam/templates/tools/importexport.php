<?php
namespace LAM\TOOLS\IMPORT_EXPORT;
use htmlProgressbar;
use htmlTitle;
use htmlResponsiveRadio;
use htmlResponsiveRow;
use htmlResponsiveInputFileUpload;
use htmlResponsiveInputTextarea;
use htmlButton;
use htmlStatusMessage;
use htmlDiv;
use htmlOutputText;
use htmlJavaScript;
use LAM\TOOLS\TREEVIEW\TreeViewTool;
use LAMException;
use htmlLink;
use htmlResponsiveInputCheckbox;
use htmlResponsiveSelect;
use htmlResponsiveInputField;
use htmlHiddenInput;
use LAM\TYPES\TypeManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018 - 2026  Roland Gruber

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

$tabImportClass = 'lam-tab-active';
$tabExportClass = '';
if (!empty($_GET['tab']) && ($_GET['tab'] === 'export')) {
	$tabImportClass = '';
	$tabExportClass = 'lam-tab-active';
}

?>

<div class="smallPaddingContent">
	<div id="tabs" class="lam-tab-container">
		<ul class="lam-tab-navigation">
			<li id="tab_import" data-tabid="1" class="lam-tab <?php echo $tabImportClass; ?>">
				<a class="lam-tab-anchor" href="#tab-import"><img alt="import" src="../../graphics/import.svg"> <?php echo _('Import') ?> </a>
			</li>
			<li id="tab_export" data-tabid="2" class="lam-tab <?php echo $tabExportClass; ?>">
				<a class="lam-tab-anchor" href="#tab-export"><img alt="export" src="../../graphics/export.svg"> <?php echo _('Export') ?> </a>
			</li>
		</ul>
		<div class="lam-tab-content <?php echo $tabImportClass; ?>" id="tab-import" data-tabid="1">
			<?php
				if (isset($_POST['submitImport'])) {
					printImportTabProcessing();
				}
				else {
					printImportTabContent();
				}
			?>
		</div>
		<div class="lam-tab-content <?php echo $tabExportClass; ?>" id="tab-export" data-tabid="2">
			<?php
				if (isset($_POST['submitExport'])) {
					printExportTabProcessing();
				}
				else {
					printExportTabContent();
				}
			?>
		</div>
	</div>
</div>

<?php

/**
 * Prints the content area for the import tab.
 */
function printImportTabContent(): void {
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Import")));
	$sources = [
		_('Text input') => 'text',
		_('File') => 'file',
	];
	$sourceRadio = new htmlResponsiveRadio(_('Source'), 'source', $sources, 'text');
	$sourceRadio->setTableRowsToHide(
		[
			'file' => ['text'],
			'text' => ['file']
		]
	);
	$sourceRadio->setTableRowsToShow(
		[
			'text' => ['text'],
			'file' => ['file']
		]
	);
	$container->add($sourceRadio);
	$container->addVerticalSpacer('1rem');
	$container->add(new htmlResponsiveInputFileUpload('file', _('File'), '750'));
    $ldifValue = $_POST['text'] ?? '';
	$container->add(new htmlResponsiveInputTextarea('text', $ldifValue, 60, 20, _('LDIF data'), '750'));
	$container->add(new htmlResponsiveInputCheckbox('noStop', false, _('Don\'t stop on errors')));

	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitImport', _('Submit'));
	$container->add($button, 12, 12, 12, 'text-center');

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, [], false);
	echo "</form>\n";
}

/**
 * Prints the content area for the import tab during processing state.
 */
function printImportTabProcessing(): void {
	try {
		checkImportData();
	}
	catch (LAMException $e) {
		$container = new htmlResponsiveRow();
		$container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()));
		parseHtml(null, $container, [], false);
		printImportTabContent();
		return;
	}
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Import")));

	$container->add(new htmlDiv('statusImportInprogress', new htmlOutputText(_('Status') . ': ' . _('in progress'))));
	$container->add(new htmlDiv('statusImportDone', new htmlOutputText(_('Status') . ': ' . _('done')), ['hidden']));
	$container->add(new htmlDiv('statusImportFailed', new htmlOutputText(_('Status') . ': ' . _('failed')), ['hidden']));
	$container->addVerticalSpacer('1rem');
	$container->add(new htmlProgressbar('progressbarImport'));
	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitImportCancel', _('Cancel'));
	$container->add($button, 12, 12, 12, 'text-center');

	$newImportButton = new htmlLink(_('New import'), '');
	$container->add($newImportButton, 12, 12, 12, 'text-center hidden newimport');

	$container->addVerticalSpacer('3rem');

	$container->add(new htmlDiv('importResults', new htmlOutputText('')));
	$container->add(new htmlJavaScript(
			'window.lam.importexport.startImport(\'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');'
		));

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, [], false);
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
		$ldif = fread($handle, 100_000_000);
		if ($ldif === false) {
			throw new LAMException(_('Unable to create temporary file.'));
		}
		fclose($handle);
	}
	if (empty($ldif)) {
		throw new LAMException(_('You must either upload a file or provide an import in the text box.'));
	}
	$lines = preg_split("/\n|\r\n|\r/", $ldif);
    if ($lines === false) {
		throw new LAMException(_('You must either upload a file or provide an import in the text box.'));
    }
	$importer = new Importer();
	$tasks = $importer->getTasks($lines);
	$_SESSION[Importer::SESSION_KEY_TASKS] = $tasks;
	$_SESSION[Importer::SESSION_KEY_COUNT] = count($tasks);
	$_SESSION[Importer::SESSION_KEY_STOP_ON_ERROR] = (!isset($_POST['noStop']) || ($_POST['noStop'] != 'on'));
}

/**
 * Prints the content area for the export tab.
 */
function printExportTabContent(): void {
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php?tab=export\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Export")));

	$baseDn = getDefaultBaseDn();
	if (!empty($_GET['dn'])) {
	    $preSetDn = base64_decode($_GET['dn']);
	    if (isValidExportDn($preSetDn)) {
	        $baseDn = $preSetDn;
        }
    }
	$baseDnField = new htmlResponsiveInputField(_('Base DN'), 'baseDn', $baseDn, '751', true);
	$baseDnField->showDnSelection();
	$container->add($baseDnField);

	$searchScopes = [
		_('Base (base dn only)') => 'base',
		_('One (one level beneath base)') => 'one',
		_('Sub (entire subtree)') => 'sub'
	];
	$searchScopeSelect = new htmlResponsiveSelect('searchScope', $searchScopes, ['sub'], _('Search scope'));
	$searchScopeSelect->setHasDescriptiveElements(true);
	$searchScopeSelect->setSortElements(false);
	$container->add($searchScopeSelect);
	$container->add(new htmlResponsiveInputField(_('Search filter'), 'filter', '(objectClass=*)', '752'));
	$container->add(new htmlResponsiveInputField(_('Attributes'), 'attributes', '*', '753'));
	$container->add(new htmlResponsiveInputCheckbox('includeSystem', false, _('Include system attributes'), '754'));
	$container->add(new htmlResponsiveInputCheckbox('saveAsFile', false, _('Save as file')));

	$formats = [
		'CSV' => 'csv',
		'LDIF' => 'ldif'
	];
	$formatSelect = new htmlResponsiveSelect('format', $formats, ['ldif'], _('Export format'));
	$formatSelect->setHasDescriptiveElements(true);
	$formatSelect->setSortElements(false);
	$container->add($formatSelect);

	$endings = [
		'Windows' => 'windows',
		'Unix' => 'unix'
	];
	$endingsSelect = new htmlResponsiveSelect('ending', $endings, ['unix'], _('End of line'));
	$endingsSelect->setHasDescriptiveElements(true);
	$endingsSelect->setSortElements(false);
	$container->add($endingsSelect);

	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitExport', _('Submit'));
	$container->add($button, 12, 12, 12, 'text-center');

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, [], false);
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
		if (str_ends_with($dn, $suffix)) {
			return true;
		}
	}
	if ($_SESSION['config']->isToolActive('TreeViewTool')) {
	    $treeSuffixes = TreeViewTool::getRootDns();
	    foreach ($treeSuffixes as $treeSuffix) {
	        $treeSuffix = strtolower($treeSuffix);
		    if (str_ends_with($dn, $treeSuffix)) {
			    return true;
		    }
        }
	}
    return false;
}

/**
 * Prints the content area for the export tab during processing state.
 */
function printExportTabProcessing(): void {
	try {
		checkExportData();
	}
	catch (LAMException $e) {
		$container = new htmlResponsiveRow();
		$container->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()));
		parseHtml(null, $container, [], false);
		printExportTabContent();
		return;
	}
	echo "<form class=\"inputForm\" enctype=\"multipart/form-data\" action=\"importexport.php?tab=export\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Export")));

	$container->add(new htmlHiddenInput('baseDn', $_POST['baseDn']));
	$container->add(new htmlHiddenInput('searchScope', $_POST['searchScope']));
	$container->add(new htmlHiddenInput('filter', $_POST['filter']));
	$container->add(new htmlHiddenInput('attributes', $_POST['attributes']));
	$container->add(new htmlHiddenInput('format', $_POST['format']));
	$container->add(new htmlHiddenInput('ending', $_POST['ending']));
	$container->add(new htmlHiddenInput('includeSystem', isset($_POST['includeSystem']) && ($_POST['includeSystem'] === 'on') ? 'true' : 'false'));
	$container->add(new htmlHiddenInput('saveAsFile', isset($_POST['saveAsFile']) && ($_POST['saveAsFile'] === 'on') ? 'true' : 'false'));

	$container->add(new htmlDiv('statusExportInprogress', new htmlOutputText(_('Status') . ': ' . _('in progress'))));
	$container->add(new htmlDiv('statusExportDone', new htmlOutputText(_('Status') . ': ' . _('done')), ['hidden']));
	$container->add(new htmlDiv('statusExportFailed', new htmlOutputText(_('Status') . ': ' . _('failed')), ['hidden']));
	$container->addVerticalSpacer('1rem');
	$container->add(new htmlProgressbar('progressbarExport'));
	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitExportCancel', _('Cancel'));
	$container->add($button, 12, 12, 12, 'text-center');

	$newExportButton = new htmlLink(_('New export'), '');
	$container->add($newExportButton, 12, 12, 12, 'text-center hidden newexport');

	$container->addVerticalSpacer('3rem');

	$exportText = new htmlOutputText('');
	$exportText->setPreformatted();
	$container->add(new htmlDiv('exportResults', $exportText));
	$container->add(new htmlJavaScript(
			'window.lam.importexport.startExport(\'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');'
		));

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, [], false);
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
