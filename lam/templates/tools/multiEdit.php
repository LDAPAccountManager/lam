<?php
namespace LAM\TOOLS\MULTI_EDIT;
use htmlProgressbar;
use htmlTable;
use htmlTitle;
use htmlSelect;
use htmlOutputText;
use htmlInputField;
use htmlSubTitle;
use htmlButton;
use htmlStatusMessage;
use htmlSpacer;
use htmlHiddenInput;
use htmlGroup;
use htmlDiv;
use htmlJavaScript;
use htmlLink;
use htmlInputTextarea;
use htmlResponsiveRow;
use htmlResponsiveSelect;
use htmlResponsiveInputField;
use htmlResponsiveTable;
use LAM\TOOLS\TREEVIEW\TreeViewTool;
use LAM\TYPES\TypeManager;
use LAMException;
use LamTemporaryFilesManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2026  Roland Gruber

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
include_once __DIR__ . "/../../lib/security.inc";
/** access to configuration data */
include_once __DIR__ . "/../../lib/config.inc";
/** access LDAP server */
include_once __DIR__ . "/../../lib/ldap.inc";
/** used to print status messages */
include_once __DIR__ . "/../../lib/status.inc";
/** multi edit functions */
include_once __DIR__ . "/../../lib/multiEditTool.inc";

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) {
	die();
}

checkIfToolIsActive('toolMultiEdit');

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

define('ADD', 'add');
define('MOD', 'mod');
define('DEL', 'del');

define('STAGE_START', 'start');
define('STAGE_READ_FINISHED', 'readFinished');
define('STAGE_ACTIONS_CALCULATED', 'actionsCalculated');
define('STAGE_WRITING', 'writing');
define('STAGE_FINISHED', 'finished');

if (isset($_GET['ajaxStatus'])) {
	runAjaxActions();
}
else {
	displayStartPage();
}

/**
 * Displays the main page of the multi edit tool.
 */
function displayStartPage(): void {
	// display main page
	include_once __DIR__ . '/../../lib/adminHeader.inc';
	echo '<div class="smallPaddingContent">';
	echo "<form action=\"multiEdit.php\" method=\"post\">\n";
	$errors = [];
	$container = new htmlResponsiveRow();
	$container->add(new htmlTitle(_("Multi edit")));
	// LDAP suffix
	$showRules = ['-' => ['otherSuffix']];
	$hideRules = [];
	$typeManager = new TypeManager();
	$types = $typeManager->getConfiguredTypes();
	$suffixes = [];
	foreach ($types as $type) {
		if ($type->isHidden()) {
			continue;
		}
		$suffixes[$type->getAlias()] = $type->getSuffix();
		$hideRules[$type->getSuffix()] = ['otherSuffix'];
	}
	$treeSuffixes = TreeViewTool::getRootDns();
	if (!empty($treeSuffixes)) {
		if (count($treeSuffixes) === 1) {
			$suffixes[_('Tree view')] = $treeSuffixes[0];
			$hideRules[$treeSuffixes[0]] = ['otherSuffix'];
		}
		else {
			foreach ($treeSuffixes as $treeSuffix) {
				$suffixes[_('Tree view') . ' (' . getAbstractDN($treeSuffix) . ')'] = $treeSuffix;
				$hideRules[$treeSuffix] = ['otherSuffix'];
			}
		}
	}
	$suffixes = array_flip($suffixes);
	natcasesort($suffixes);
	$suffixes = array_flip($suffixes);
	$suffixes[_('Other')] = '-';
	$suffixValues = array_values($suffixes);
	$valSuffix = empty($_POST['suffix']) ? $suffixValues[0] : $_POST['suffix'];
	$suffixSelect = new htmlResponsiveSelect('suffix', $suffixes, [$valSuffix], _('LDAP suffix'), '700');
	$suffixSelect->setHasDescriptiveElements(true);
	$suffixSelect->setSortElements(false);
	$suffixSelect->setTableRowsToShow($showRules);
	$suffixSelect->setTableRowsToHide($hideRules);
	$container->add($suffixSelect);
	$valOtherSuffix = empty($_POST['otherSuffix']) ? '' : (string) $_POST['otherSuffix'];
	$container->add(new htmlResponsiveInputField(_('Other'), 'otherSuffix', $valOtherSuffix));
	// LDAP filter
	$valFilter = empty($_POST['filter']) ? '(objectClass=inetOrgPerson)' : (string) $_POST['filter'];
	$container->add(new htmlResponsiveInputField(_('LDAP filter'), 'filter', $valFilter, '701'));
	// operation fields
	$operationsTitle = new htmlSubTitle(_('Operations'));
	$operationsTitle->setHelpId('702');
	$container->add($operationsTitle);
	$operationsTitles = [_('Type'), _('Attribute name'), _('Value')];
	$data = [];
	$opCount = empty($_POST['opcount']) ? '3' : $_POST['opcount'];
	if (isset($_POST['addFields'])) {
		$opCount += 3;
	}
	$operations = [_('Add') => ADD, _('Modify') => MOD, _('Delete') => DEL];
	for ($i = 0; $i < $opCount; $i++) {
		// operation type
		$selOp = empty($_POST['op_' . $i]) ? ADD : $_POST['op_' . $i];
		$opSelect = new htmlSelect('op_' . $i, $operations, [$selOp]);
		$opSelect->setHasDescriptiveElements(true);
		$data[$i][] = $opSelect;
		// attribute name
		$attrVal = empty($_POST['attr_' . $i]) ? '' : (string) $_POST['attr_' . $i];
		$data[$i][] = new htmlInputField('attr_' . $i, $attrVal);
		$valVal = empty($_POST['val_' . $i]) ? '' : (string) $_POST['val_' . $i];
		$data[$i][] = new htmlInputField('val_' . $i, $valVal);
		// check input
		if (($selOp == ADD) && !empty($attrVal) && empty($valVal)) {
			$errors[] = new htmlStatusMessage('ERROR', _('Please enter a value to add.'), htmlspecialchars($attrVal));
		}
		if (($selOp == MOD) && !empty($attrVal) && empty($valVal)) {
			$errors[] = new htmlStatusMessage('ERROR', _('Please enter a value to modify.'), htmlspecialchars($attrVal));
		}
	}
	$operationsTable = new htmlResponsiveTable($operationsTitles, $data);
	$container->add($operationsTable);
	// add more fields
	$container->addVerticalSpacer('1rem');
	$container->add(new htmlButton('addFields', _('Add more fields')));
	$container->add(new htmlHiddenInput('opcount', $opCount));
	// error messages
	if ($errors !== []) {
		$container->addVerticalSpacer('5rem');
		foreach ($errors as $error) {
			$error->colspan = 5;
			$container->add($error);
		}
	}
	// action buttons
	$container->addVerticalSpacer('2rem');
	$buttonGroup = new htmlGroup();
	$buttonGroup->colspan = 3;
	$applyButton = new htmlButton('applyChanges', _('Apply changes'));
	$applyButton->setCSSClasses(['lam-primary']);
	$buttonGroup->addElement($applyButton);
	$buttonGroup->addElement(new htmlSpacer('10px', null));
	$dryRunButton = new htmlButton('dryRun', _('Dry run'));
	$dryRunButton->setCSSClasses(['lam-secondary']);
	$buttonGroup->addElement($dryRunButton);
	$container->add($buttonGroup);
	$container->addVerticalSpacer('1rem');

	// run actions
	if ((count($errors) === 0) && (isset($_POST['dryRun']) || isset($_POST['applyChanges']))) {
		runActions($container);
	}

	addSecurityTokenToMetaHTML($container);

	parseHtml(null, $container, [], false);
	echo "</form>\n";
	echo '</div>';
	include_once __DIR__ . '/../../lib/adminFooter.inc';
}

/**
 * Runs the dry run and change actions.
 *
 * @param htmlResponsiveRow $container container
 */
function runActions(htmlResponsiveRow $container): void {
	// LDAP suffix
	$suffix = ($_POST['suffix'] === '-') ? trim($_POST['otherSuffix']) : $_POST['suffix'];
	if (empty($suffix)) {
		$error = new htmlStatusMessage('ERROR', _('LDAP Suffix is invalid!'));
		$error->colspan = 5;
		$container->add($error);
		return;
	}
	// LDAP filter
	$filter = trim($_POST['filter']);
	// operations
	$operations = [];
	for ($i = 0; $i < $_POST['opcount']; $i++) {
		if (!empty($_POST['attr_' . $i])) {
			$operations[] = [$_POST['op_' . $i], strtolower(trim($_POST['attr_' . $i])), trim($_POST['val_' . $i])];
		}
	}
	if (count($operations) === 0) {
		$error = new htmlStatusMessage('ERROR', _('Please specify at least one operation.'));
		$error->colspan = 5;
		$container->add($error);
		return;
	}
	$_SESSION['multiEdit_suffix'] = $suffix;
	$_SESSION['multiEdit_filter'] = $filter;
	$_SESSION['multiEdit_operations'] = $operations;
	$_SESSION['multiEdit_status'] = ['stage' => STAGE_START];
	$_SESSION['multiEdit_dryRun'] = isset($_POST['dryRun']);
	// progress area
	$container->add(new htmlSubTitle(_('Progress')));
	$container->add(new htmlProgressbar('progressBar'));
	$progressDiv = new htmlDiv('progressArea', new htmlOutputText(''));
	$container->add($progressDiv);
	// JS block for AJAX status update
	$ajaxBlock = '
		window.lam.multiedit.runActions();
	';
	$container->add(new htmlJavaScript($ajaxBlock));
}

/**
 * Performs the modify operations.
 */
function runAjaxActions(): void {
	$jsonReturn = [
		'status' => STAGE_START,
		'progress' => 0,
		'content' => ''
	];
	switch ($_SESSION['multiEdit_status']['stage']) {
		case STAGE_START:
			$jsonReturn = readLDAPData();
			break;
		case STAGE_READ_FINISHED:
			$jsonReturn = generateActions();
			break;
		case STAGE_ACTIONS_CALCULATED:
		case STAGE_WRITING:
			$jsonReturn = $_SESSION['multiEdit_dryRun'] ? dryRun() : doModify();
			break;
	}
	echo json_encode($jsonReturn, JSON_THROW_ON_ERROR);
}

/**
 * Reads the LDAP entries from the directory.
 *
 * @return array<mixed> status
 */
function readLDAPData(): array {
	$suffix = $_SESSION['multiEdit_suffix'];
	$filter = $_SESSION['multiEdit_filter'];
	if (empty($filter)) {
		$filter = '(objectClass=*)';
	}
	$operations = $_SESSION['multiEdit_operations'];
	$attributes = [];
	foreach ($operations as $op) {
		if (!in_array(strtolower($op[1]), $attributes)) {
			$attributes[] = strtolower($op[1]);
			$attributes = array_merge($attributes, extractWildcards($op[2]));
		}
	}
	$attributes = array_values(array_unique($attributes));
	// run LDAP query
	$results = searchLDAP($suffix, $filter, $attributes);
	// print error message if no data returned
	if (empty($results)) {
		$code = ldap_errno($_SESSION['ldap']->server());
		if ($code !== 0) {
			$msg = new htmlStatusMessage('ERROR', _('Encountered an error while performing search.'), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
		}
		else {
			$msg = new htmlStatusMessage('ERROR', _('No objects found!'));
		}
		$content = getMessageHTML($msg);
		return [
			'status' => STAGE_FINISHED,
			'progress' => 100,
			'content' => $content
		];
	}
	// save LDAP data
	$_SESSION['multiEdit_status']['entries'] = $results;
	$_SESSION['multiEdit_status']['stage'] = STAGE_READ_FINISHED;
	return [
		'status' => STAGE_READ_FINISHED,
		'progress' => 10,
		'content' => ''
	];
}

/**
 * Generates the required actions based on the read LDAP data.
 *
 * @return array<mixed> status
 */
function generateActions(): array {
	$actions = [];
	foreach ($_SESSION['multiEdit_status']['entries'] as $oldEntry) {
		$dn = $oldEntry['dn'];
		$newEntry = $oldEntry;
		foreach ($_SESSION['multiEdit_operations'] as $op) {
			$opType = $op[0];
			$attr = $op[1];
			$val = replaceWildcards($op[2], $oldEntry);
			switch ($opType) {
				case ADD:
					if (empty($oldEntry[$attr]) || !in_array_ignore_case($val, $oldEntry[$attr])) {
						$newEntry[$attr][] = $val;
					}
					break;
				case MOD:
					if (empty($oldEntry[$attr]) || !in_array_ignore_case($val, $oldEntry[$attr])) {
						// attribute not yet exists, add it
						$newEntry[$attr] = [$val];
					}
					break;
				case DEL:
					if (empty($val) && !empty($oldEntry[$attr])) {
						unset($newEntry[$attr]);
					}
					elseif (!empty($val) && isset($oldEntry[$attr]) && in_array($val, $oldEntry[$attr])) {
						$newEntry[$attr] = array_delete([$val], $newEntry[$attr]);
					}
					break;
			}
		}
		unset($oldEntry['dn']);
		unset($newEntry['dn']);
		// cleanup
		foreach ($newEntry as $name => &$values) {
			// remove empty values
			$values = array_values($values);
			$count = count($values);
			for ($i = 0; $i < $count; $i++) {
				if ($values[$i] === '') {
					unset($values[$i]);
				}
			}
			$values = array_values($values);
			// remove empty list of values
			if (count($values) === 0) {
				unset($newEntry[$name]);
			}
		}
		// find deleted attributes (in $oldEntry but no longer in $newEntry)
		foreach ($oldEntry as $name => $value) {
			if (!isset($newEntry[$name])) {
				$actions[$dn][$name] = [];
			}
		}
		// find changed attributes
		foreach ($newEntry as $name => $value) {
			if (!isset($oldEntry[$name]) || !areArrayContentsEqual($value, $oldEntry[$name])) {
				$actions[$dn][$name] = $value;
			}
		}
	}
	// save actions
	$_SESSION['multiEdit_status']['actions'] = $actions;
	$_SESSION['multiEdit_status']['stage'] = STAGE_ACTIONS_CALCULATED;
	return [
		'status' => STAGE_ACTIONS_CALCULATED,
		'progress' => 20,
		'content' => ''
	];
}

/**
 * Prints the dryRun output.
 *
 * @return array<mixed> status
 * @throws LAMException error simulating actions
 */
function dryRun(): array {
	$pro = isLAMProVersion() ? ' Pro' : '';
	$ldif = '# LDAP Account Manager' . $pro . ' ' . LAMVersion() . "\n\nversion: 1\n\n";
	$log = '';
	// fill LDIF and log file
	foreach ($_SESSION['multiEdit_status']['actions'] as $dn => $changes) {
		$log .= $dn . "\r\n";
		$ldif .= 'dn: ' . $dn . "\n";
		$ldif .= 'changetype: modify' . "\n";
		$isFirstChange = true;
		foreach ($changes as $attr => $values) {
			$log .= '* ' . $attr . '=' . implode(', ', $values) . "\r\n";
			if (!$isFirstChange) {
				$ldif .= "-\n";
			}
			$ldif .= 'replace: ' . $attr . "\n";
			foreach ($values as $value) {
				$ldif .= $attr . ': ' . $value . "\n";
			}
			$isFirstChange = false;
		}
		$ldif .= "\n";
		$log .= "\r\n";
	}
	// build meta HTML
	$container = new htmlTable();
	$container->addElement(new htmlOutputText(_('Dry run finished.')), true);
	$container->addVerticalSpace('20px');
	// store LDIF
	$tempFilesManager = new LamTemporaryFilesManager();
	$fileName = $tempFilesManager->registerTemporaryFile('.ldif', 'ldif_');
	$out = $tempFilesManager->openTemporaryFileForWrite($fileName);
	fwrite($out, $ldif);
	$container->addElement(new htmlOutputText(_('LDIF file')), true);
	$ldifLink = new htmlLink($fileName, $tempFilesManager->getDownloadLink($fileName));
	$ldifLink->setTargetWindow('_blank');
	$container->addElement($ldifLink, true);
	$container->addVerticalSpace('20px');
	$container->addElement(new htmlOutputText(_('Log output')), true);
	$container->addElement(new htmlInputTextarea('log', $log, 100, 30), true);
	// generate HTML
	fclose ($out);
	ob_start();
	parseHtml(null, $container, [], true);
	$content = ob_get_contents();
	ob_end_clean();
	return [
		'status' => STAGE_FINISHED,
		'progress' => 100,
		'content' => $content
	];
}

/**
 * Error handler
 *
 * @param int $errno error number
 * @param string $errstr error message
 * @param string $errfile error file
 * @param int $errline error line
 * @return bool stop internal error handler
 */
function multiEditLdapErrorHandler($errno, $errstr, $errfile, $errline): bool {
	if (($errno === E_USER_ERROR) || ($errno === E_ERROR)) {
		logNewMessage(LOG_ERR, 'Error occurred: ' . $errstr . " ($errfile: $errline)");
		$_REQUEST['multiEdit_error'] = true;
	}
	elseif (($errno === E_USER_WARNING) || ($errno === E_WARNING)) {
		logNewMessage(LOG_WARNING, 'Error occurred: ' . $errstr . " ($errfile: $errline)");
		$_REQUEST['multiEdit_error'] = true;
	}
	return true;
}

/**
 * Runs the actual modifications.
 *
 * @return array<mixed> status
 */
function doModify(): array {
	set_error_handler(\LAM\TOOLS\MULTI_EDIT\multiEditLdapErrorHandler(...));
	// initial action index
	if (!isset($_SESSION['multiEdit_status']['index'])) {
		$_SESSION['multiEdit_status']['index'] = 0;
		$_SESSION['multiEdit_status']['dnList'] = array_keys($_SESSION['multiEdit_status']['actions']);
	}
	// initial content
	$_SESSION['multiEdit_status']['modContent'] ??= '';
	// run 10 modifications in each call
	$localCount = 0;
	while (($localCount < 10) && ($_SESSION['multiEdit_status']['index'] < count($_SESSION['multiEdit_status']['dnList']))) {
		$dn = $_SESSION['multiEdit_status']['dnList'][$_SESSION['multiEdit_status']['index']];
		$changes = $_SESSION['multiEdit_status']['actions'][$dn];
		$_SESSION['multiEdit_status']['modContent'] .= htmlspecialchars($dn) . "<br>";
		// run LDAP commands
		$success = ldapModifyAttributes($_SESSION['ldap']->server(), $dn, $changes);
		if (!$success || isset($_REQUEST['multiEdit_error'])) {
			$msg = new htmlStatusMessage('ERROR', getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			$_SESSION['multiEdit_status']['modContent'] .= getMessageHTML($msg);
		}
		$localCount++;
		$_SESSION['multiEdit_status']['index']++;
	}
	// check if finished
	if ($_SESSION['multiEdit_status']['index'] == count($_SESSION['multiEdit_status']['actions'])) {
		$_SESSION['multiEdit_status']['modContent'] .= '<br><br>' . _('Finished all operations.');
		return [
			'status' => STAGE_FINISHED,
			'progress' => 100,
			'content' => $_SESSION['multiEdit_status']['modContent']
		];
	}
	// return current status
	return [
		'status' => STAGE_WRITING,
		'progress' => 20 + (($_SESSION['multiEdit_status']['index'] / count($_SESSION['multiEdit_status']['actions'])) * 80),
		'content' => $_SESSION['multiEdit_status']['modContent']
	];
}

/**
 * Returns the HTML code for a htmlStatusMessage
 *
 * @param htmlStatusMessage $msg message
 * @return string HTML code
 */
function getMessageHTML(htmlStatusMessage $msg): string {
	ob_start();
	parseHtml(null, $msg, [], true);
	$content = ob_get_contents();
	ob_end_clean();
	return $content;
}
