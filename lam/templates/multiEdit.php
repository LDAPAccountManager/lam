<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013  Roland Gruber

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
include_once("../lib/security.inc");
/** access to configuration data */
include_once("../lib/config.inc");
/** access LDAP server */
include_once("../lib/ldap.inc");
/** used to print status messages */
include_once("../lib/status.inc");

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolMultiEdit');

setlanguage();

const ADD = 'add';
const MOD = 'mod';
const DEL = 'del';

const STAGE_START = 'start';
const STAGE_READ_FINISHED = 'readFinished';
const STAGE_ACTIONS_CALCULATED = 'actionsCalculated';
const STAGE_WRITING = 'writing';
const STAGE_FINISHED = 'finished';

if (isset($_GET['ajaxStatus'])) {
	runAjaxActions();
}
else {
	displayStartPage();
}

/**
 * Displays the main page of the multi edit tool.
 */
function displayStartPage() {
	// display main page
	include 'main_header.php';
	echo '<div class="user-bright smallPaddingContent">';
	echo ("<form action=\"multiEdit.php\" method=\"post\">\n");
	$errors = array();
	$tabindex = 1;
	$container = new htmlTable();
	$container->addElement(new htmlTitle(_("Multi edit")), true);
	// LDAP suffix
	$showRules = array('-' => array('otherSuffix'));
	$hideRules = array();
	$container->addElement(new htmlOutputText(_('LDAP suffix')));
	$suffixGroup = new htmlTable();
	$types = $_SESSION['config']->get_ActiveTypes();
	$suffixes = array();
	foreach ($types as $type) {
		$suffixes[getTypeAlias($type)] = $_SESSION['config']->get_Suffix($type);
		$hideRules[$_SESSION['config']->get_Suffix($type)] = array('otherSuffix');
	}
	$treeSuffix = $_SESSION['config']->get_Suffix('tree');
	if (!empty($treeSuffix)) {
		$suffixes[_('Tree view')] = $_SESSION['config']->get_Suffix('tree');
		$hideRules[$_SESSION['config']->get_Suffix('tree')] = array('otherSuffix');
	}
	$suffixes = array_flip($suffixes);
	natcasesort($suffixes);
	$suffixes = array_flip($suffixes);
	$suffixes[_('other')] = '-';
	$suffixValues = array_values($suffixes);
	$valSuffix = empty($_POST['suffix']) ? $suffixValues[0] : $_POST['suffix'];
	$suffixSelect = new htmlSelect('suffix', $suffixes, array($valSuffix));
	$suffixSelect->setHasDescriptiveElements(true);
	$suffixSelect->setSortElements(false);
	$suffixSelect->setTableRowsToShow($showRules);
	$suffixSelect->setTableRowsToHide($hideRules);
	$suffixGroup->addElement($suffixSelect);
	$otherSuffixTable = new htmlTable();
	$valOtherSuffix = empty($_POST['otherSuffix']) ? '' : $_POST['otherSuffix'];
	$otherSuffixTable->addElement(new htmlInputField('otherSuffix'));
	$suffixGroup->addElement($otherSuffixTable);
	$container->addElement($suffixGroup);
	$container->addElement(new htmlHelpLink('700'), true);
	// LDAP filter
	$valFilter = empty($_POST['filter']) ? '' : $_POST['filter'];
	$container->addElement(new htmlTableExtendedInputField(_('LDAP filter'), 'filter', $valFilter, '701'), true);
	// operation fields
	$container->addElement(new htmlSubTitle(_('Operations')), true);
	$container->addElement(new htmlOutputText(_('Type')));
	$container->addElement(new htmlOutputText(_('Attribute name')));
	$container->addElement(new htmlOutputText(_('Value')));
	$container->addElement(new htmlHelpLink('702'), true);
	$opCount = empty($_POST['opcount']) ? '3' : $_POST['opcount'];
	if (isset($_POST['addFields'])) {
		$opCount += 3;
	}
	$operations = array(_('Add') => ADD, _('Modify') => MOD, _('Delete') => DEL);
	for ($i = 0; $i < $opCount; $i++) {
		// operation type
		$selOp = empty($_POST['op_' . $i]) ? ADD : $_POST['op_' . $i];
		$opSelect = new htmlSelect('op_' . $i, $operations, array($selOp));
		$opSelect->setHasDescriptiveElements(true);
		$container->addElement($opSelect);
		// attribute name
		$attrVal = empty($_POST['attr_' . $i]) ? '' : $_POST['attr_' . $i];
		$container->addElement(new htmlInputField('attr_' . $i, $attrVal));
		$valVal = empty($_POST['val_' . $i]) ? '' : $_POST['val_' . $i];
		$container->addElement(new htmlInputField('val_' . $i, $valVal), true);
		// check input
		if (($selOp == ADD) && !empty($attrVal) && empty($valVal)) {
			$errors[] = new htmlStatusMessage('ERROR', _('Please enter a value to add.'), htmlspecialchars($attrVal));
		}
		if (($selOp == MOD) && !empty($attrVal) && empty($valVal)) {
			$errors[] = new htmlStatusMessage('ERROR', _('Please enter a value to modify.'), htmlspecialchars($attrVal));
		}
	}
	// add more fields
	$container->addVerticalSpace('5px');
	$container->addElement(new htmlButton('addFields', _('Add more fields')));
	$container->addElement(new htmlHiddenInput('opcount', $opCount), true);
	// error messages
	if (sizeof($errors) > 0) {
		$container->addVerticalSpace('20px');
		foreach ($errors as $error) {
			$error->colspan = 5;
			$container->addElement($error, true);
		}
	}
	// action buttons
	$container->addVerticalSpace('20px');
	$buttonGroup = new htmlGroup();
	$buttonGroup->colspan = 3;
	$dryRunButton = new htmlButton('dryRun', _('Dry run'));
	$dryRunButton->setIconClass('dryRunButton');
	$buttonGroup->addElement($dryRunButton);
	$buttonGroup->addElement(new htmlSpacer('10px', null));
	$applyButton = new htmlButton('applyChanges', _('Apply changes'));
	$applyButton->setIconClass('saveButton');
	$buttonGroup->addElement($applyButton);
	$container->addElement($buttonGroup, true);
	$container->addVerticalSpace('10px');
	
	// run actions
	if ((sizeof($errors) == 0) && (isset($_POST['dryRun']) || isset($_POST['applyChanges']))) {
		runActions($container);
	}
	
	
	parseHtml(null, $container, array(), false, $tabindex, 'user');
	echo ("</form>\n");
	echo '</div>';
	include 'main_footer.php';
}

/**
 * Runs the dry run and change actions.
 * 
 * @param htmlTable $container container
 */
function runActions(&$container) {
	// LDAP suffix
	if ($_POST['suffix'] == '-') {
		$suffix = trim($_POST['otherSuffix']);
	}
	else {
		$suffix = $_POST['suffix'];
	}
	if (empty($suffix)) {
		$error = new htmlStatusMessage('ERROR', _('LDAP Suffix is invalid!'));
		$error->colspan = 5;
		$container->addElement($error);
		return;
	}
	// LDAP filter
	$filter = trim($_POST['filter']);
	// operations
	$operations = array();
	for ($i = 0; $i < sizeof($_POST['opcount']); $i++) {
		if (!empty($_POST['attr_' . $i])) {
			$operations[] = array($_POST['op_' . $i], strtolower(trim($_POST['attr_' . $i])), trim($_POST['val_' . $i]));
		}
	}
	if (sizeof($operations) == 0) {
		$error = new htmlStatusMessage('ERROR', _('Please specify at least one operation.'));
		$error->colspan = 5;
		$container->addElement($error);
		return;
	}
	$_SESSION['multiEdit_suffix'] = $suffix;
	$_SESSION['multiEdit_filter'] = $filter;
	$_SESSION['multiEdit_operations'] = $operations;
	$_SESSION['multiEdit_status'] = array('stage' => STAGE_START);
	$_SESSION['multiEdit_dryRun'] = isset($_POST['dryRun']);
	// disable all input elements
	$jsContent = '
		jQuery(\'input\').attr(\'disabled\', true);
		jQuery(\'select\').attr(\'disabled\', true);
		jQuery(\'button\').attr(\'disabled\', true);
	';
	$container->addElement(new htmlJavaScript($jsContent), true);
	// progress area
	$container->addElement(new htmlSubTitle(_('Progress')), true);
	$progressBarDiv = new htmlDiv('progressBar', '');
	$progressBarDiv->colspan = 5;
	$container->addElement($progressBarDiv, true);
	$progressDiv = new htmlDiv('progressArea', '');
	$progressDiv->colspan = 5;
	$container->addElement($progressDiv, true);
	// JS block for AJAX status update
	$ajaxBlock = '
		jQuery.get(\'multiEdit.php?ajaxStatus\', null, function(data) {handleReply(data);}, \'json\');
		
		function handleReply(data) {
			jQuery(\'#progressBar\').progressbar({value: data.progress, max: 120});
			jQuery(\'#progressArea\').html(data.content);
			if (data.status != "finished") {
				jQuery.get(\'multiEdit.php?ajaxStatus\', null, function(data) {handleReply(data);}, \'json\');
			}
			else {
				jQuery(\'input\').removeAttr(\'disabled\');
				jQuery(\'select\').removeAttr(\'disabled\');
				jQuery(\'button\').removeAttr(\'disabled\');
				jQuery(\'#progressBar\').hide();
			}
		}
	';
	$container->addElement(new htmlJavaScript($ajaxBlock), true);
}

/**
 * Performs the modify operations.
 */
function runAjaxActions() {
	$jsonReturn = array(
		'status' => STAGE_START,
		'progress' => 0,
		'content' => ''
	);
	switch ($_SESSION['multiEdit_status']['stage']) {
		case STAGE_START:
			$jsonReturn = readLDAPData();
			break;
		case STAGE_READ_FINISHED:
			$jsonReturn = generateActions();
			break;
		case STAGE_ACTIONS_CALCULATED:
			if ($_SESSION['multiEdit_dryRun']) {
				$jsonReturn = dryRun();
			}
			else {
				$jsonReturn = doModify();
			}
			break;
	}
	echo json_encode($jsonReturn);
}

/**
 * Reads the LDAP entries from the directory.
 * 
 * @return array status
 */
function readLDAPData() {
	$suffix = $_SESSION['multiEdit_suffix'];
	$filter = $_SESSION['multiEdit_filter'];
	if (empty($filter)) {
		$filter = '(objectClass=*)';
	}
	$operations = $_SESSION['multiEdit_operations'];
	$attributes = array();
	foreach ($operations as $op) {
		if (!in_array(strtolower($op[1]), $attributes)) {
			$attributes[] = strtolower($op[1]);
		}
	}
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
		$tabindex = 0;
		ob_start();
		parseHtml(null, $msg, array(), true, $tabindex, 'user');
		$content = ob_get_contents();
		ob_end_clean();
		return array(
			'status' => STAGE_FINISHED,
			'progress' => 120,
			'content' => $content
		);
	}
	// save LDAP data
	$_SESSION['multiEdit_status']['entries'] = $results;
	$_SESSION['multiEdit_status']['stage'] = STAGE_READ_FINISHED;
	return array(
		'status' => STAGE_READ_FINISHED,
		'progress' => 10,
		'content' => ''
	);
}

/**
 * Generates the required actions based on the read LDAP data.
 * 
 * @return array status
 */
function generateActions() {
	$actions = array();
	foreach ($_SESSION['multiEdit_status']['entries'] as $entry) {
		foreach ($_SESSION['multiEdit_operations'] as $op) {
			$opType = $op[0];
			$attr = $op[1];
			$val = $op[2];
			switch ($opType) {
				case ADD:
					if (empty($entry[$attr]) || !in_array_ignore_case($val, $entry[$attr])) {
						$actions[] = array(ADD, $attr, $val);
					}
					break;
				case MOD:
					if (empty($entry[$attr]) || !in_array_ignore_case($val, $entry[$attr])) {
						$actions[] = array(ADD, $attr, $val);
					}
					break;
				case DEL:
					if (empty($val) && !empty($entry[$attr])) {
						$actions[] = array(MOD, $attr, array());
					}
					elseif (!empty($val) && in_array($val, $entry[$attr])) {
						$actions[] = array(DEL, $attr, array($val));
					}
					break;
			}
		}
	}
	// save actions
	$_SESSION['multiEdit_status']['actions'] = $actions;
	$_SESSION['multiEdit_status']['stage'] = STAGE_ACTIONS_CALCULATED;
	return array(
		'status' => STAGE_ACTIONS_CALCULATED,
		'progress' => 20,
		'content' => ''
	);
}

