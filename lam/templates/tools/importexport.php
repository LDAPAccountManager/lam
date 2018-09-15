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
use \LAMException;
use \htmlLink;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2018  Roland Gruber

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
include_once("../../lib/security.inc");
/** access to configuration data */
include_once("../../lib/config.inc");
/** access LDAP server */
include_once("../../lib/ldap.inc");
/** used to print status messages */
include_once("../../lib/status.inc");
/** import class */
include_once("../../lib/import.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

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

include '../../lib/adminHeader.inc';
	$tabindex = 1;
?>

<script>
  $(function() {
    $("#tabs").tabs();
  });
</script>

<div class="user-bright smallPaddingContent">
	<div id="tabs">
		<ul>
			<li id="tab_import">
				<a href="#tab-import"><img alt="import" src="../../graphics/import.png"> <?php echo _('Import') ?> </a>
			</li>
			<li id="tab_export">
				<a href="#tab-export"><img alt="export" src="../../graphics/export.png"> <?php echo _('Export') ?> </a>
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
		</div>
	</div>
</div>

<?php

/**
 * Prints the content area for the import tab.
 *
 * @param int $tabindex tabindex
 */
function printImportTabContent(&$tabindex) {
	echo "<form enctype=\"multipart/form-data\" action=\"importexport.php\" method=\"post\">\n";
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Import")), 12);
	$sources = array(
		_('File') => 'file',
		_('Text input') => 'text'
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

	$container->addVerticalSpacer('3rem');
	$button = new htmlButton('submitImport', _('Submit'));
	$container->add($button, 12, 12, 12, 'text-center');

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo ("</form>\n");
}

/**
 * Prints the content area for the import tab during processing state.
 *
 * @param int $tabindex tabindex
 */
function printImportTabProcessing(&$tabindex) {
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
	echo "<form enctype=\"multipart/form-data\" action=\"importexport.php\" method=\"post\">\n";
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
			'window.lam.import.startImport(\'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');'
		), 12);

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo ("</form>\n");
}

/**
 * Checks if the import data is ok.
 *
 * @throws LAMException error message if not valid
 */
function checkImportData() {
	$source = $_POST['source'];
	$ldif = '';
	if ($source == 'text') {
		$ldif = $_POST['text'];
	}
	else {
		$handle = fopen($_FILES['file']['tmp_name'], "r");
		$ldif = fread($handle, 100000000);
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
}

include '../../lib/adminFooter.inc';
