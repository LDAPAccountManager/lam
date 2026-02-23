<?php

namespace LAM\TOOLS\PDF_EDITOR;

use htmlJavaScript;
use htmlResponsiveRow;
use htmlResponsiveSelect;
use htmlResponsiveInputField;
use htmlTitle;
use htmlButton;
use htmlOutputText;
use htmlGroup;
use htmlSelect;
use htmlInputField;
use htmlSubTitle;
use htmlResponsiveInputTextarea;
use htmlHiddenInput;
use htmlSpacer;
use LAM\PDF\PdfLogo;
use LAM\PDF\PdfStructurePersistenceManager;
use LAM\PDF\PDFTextSection;
use LAM\PDF\PDFEntrySection;
use LAM\PDF\PDFStructure;
use LAM\PDF\PDFSectionEntry;
use LAM\TYPES\TypeManager;
use LAMException;

/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2007 - 2025  Roland Gruber

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

  Manages creating/changing of pdf structures.
*/

/**
 * Displays the main page of the PDF editor where the user can select the displayed entries.
 *
 * @author Michael Duergner
 * @author Roland Gruber
 * @package PDF
 */

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to PDF configuration files */
include_once(__DIR__ . '/../../lib/pdfstruct.inc');
/** LDAP object */
include_once(__DIR__ . '/../../lib/ldap.inc');
/** LAM configuration */
include_once(__DIR__ . '/../../lib/config.inc');
/** module functions */
include_once(__DIR__ . '/../../lib/modules.inc');

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) {
	die();
}

checkIfToolIsActive('toolPDFEditor');

setlanguage();

if (!empty($_POST)) {
	validateSecurityToken();
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// Write $_POST variables to $_GET when the form was submitted via post
if (isset($_POST['type'])) {
	$_GET = $_POST;
}

$typeManager = new TypeManager();
$type = $typeManager->getConfiguredType($_GET['type']);
if (($type === null) || $type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
	logNewMessage(LOG_ERR, 'User tried to access hidden PDF structure: ' . $_GET['type']);
	die();
}


// Abort and go back to the main PDF structure page
if (isset($_GET['abort'])) {
	metarefresh('pdfmain.php');
	exit;
}

$pdfStructurePersistenceManager = new PdfStructurePersistenceManager();

// Load PDF structure from the file if it is not defined in session
if (!isset($_SESSION['currentPDFStructure'])) {
	// Load structure file to be edited
	try {
		if (isset($_GET['edit'])) {
			$_SESSION['currentPDFStructure'] = $pdfStructurePersistenceManager->readPdfStructure($_SESSION['config']->getName(),
				$type->getId(), $_GET['edit']);
		}
		// Load the default structure file when creating a new one
		else {
			$structureNames = $pdfStructurePersistenceManager->getPDFStructures($_SESSION['config']->getName(),
				$type->getId());
			if (in_array('default', $structureNames)) {
				$_SESSION['currentPDFStructure'] = $pdfStructurePersistenceManager->readPdfStructure($_SESSION['config']->getName(),
					$type->getId(), 'default');
			}
			else {
				$_SESSION['currentPDFStructure'] = new PDFStructure();
			}
		}
	}
	catch (LAMException $e) {
		logNewMessage(LOG_ERR, $e->getTitle());
		metaRefresh('pdfmain.php?loadFailed=1&name=' . $_GET['edit']);
		exit;
	}
}

$logoFiles = $pdfStructurePersistenceManager->getPdfLogos($_SESSION['config']->getName(), true);
if (!empty($_POST['form_submit'])) {
	updateBasicSettings($_SESSION['currentPDFStructure'], $logoFiles);
	updateSectionTitles($_SESSION['currentPDFStructure']);
	addSection($_SESSION['currentPDFStructure']);
	addSectionEntry($_SESSION['currentPDFStructure']);
	removeItem($_SESSION['currentPDFStructure']);
	moveUp($_SESSION['currentPDFStructure']);
	moveDown($_SESSION['currentPDFStructure']);
}

// Check if pdfname is valid, then save current structure to file and go to
// main pdf structure page
$saveErrors = [];
if (isset($_GET['submit'])) {
	try {
		$pdfStructurePersistenceManager->savePdfStructure($_SESSION['config']->getName(), $type->getId(), $_POST['pdfname'], $_SESSION['currentPDFStructure']);
		unset($_SESSION['currentPDFStructure']);
		metaRefresh('pdfmain.php?savedSuccessfully=' . $_POST['pdfname']);
		exit;
	}
	catch (LAMException $e) {
		$saveErrors[] = ['ERROR', $e->getTitle(), $e->getMessage()];
	}
}

$availablePDFFields = getAvailablePDFFields($type->getId());

// Create the values for the dropdown boxes for section headline defined by
// value entries and fetch all available modules
$modules = [];
$section_items_array = [];
$section_items = '';
$sortedModules = [];
foreach ($availablePDFFields as $module => $fields) {
	$title = ($module != 'main') ? getModuleAlias($module, $type->getScope()) : _('Main');
	$sortedModules[$module] = $title;
}
natcasesort($sortedModules);
foreach ($sortedModules as $module => $title) {
	$values = $availablePDFFields[$module];
	if (count($values) < 1) {
		continue;
	}
	$modules[] = $module;
	$section_items .= "<optgroup label=\"" . $title . "\"\n>";
	natcasesort($values);
	foreach ($values as $attribute => $attributeLabel) {
		$section_items_array[] = $module . '_' . $attribute;
		$section_items .= "<option value=\"" . $module . '_' . $attribute . "\">" . $attributeLabel . "</option>\n";
	}
	$section_items .= "</optgroup>\n";
}
$modules = implode(',', $modules);

// print header
include __DIR__ . '/../../lib/adminHeader.inc';
?>
    <div class="smallPaddingContent">
	<?php

// print error messages if any
if ($saveErrors !== []) {
	foreach ($saveErrors as $saveError) {
		call_user_func_array(StatusMessage(...), $saveError);
	}
	echo "<br>\n";
}

$newFieldFieldElements = [];
foreach ($sortedModules as $module => $title) {
	$fields = $availablePDFFields[$module];
	if ($fields !== []) {
		$moduleFields = [];
		foreach ($fields as $field => $fieldLabel) {
			$moduleFields[$fieldLabel] = $module . "_" . $field;
		}
		$newFieldFieldElements[$title] = $moduleFields;
	}
}

// structure name
$structureName = '';
if (isset($_GET['edit'])) {
	$structureName = $_GET['edit'];
}
elseif (isset($_POST['pdfname'])) {
	$structureName = $_POST['pdfname'];
}
// headline
$headline = $_SESSION['currentPDFStructure']->getTitle();
// logo
$logos = [_('No logo') => 'none'];
foreach ($logoFiles as $logoFile) {
	$logos[$logoFile->getName() . ' (' . $logoFile->getWidth() . ' x ' . $logoFile->getHeight() . ")"] = $logoFile->getName();
}
$selectedLogo = ['printLogo.jpg'];
if ($_SESSION['currentPDFStructure']->getLogo() !== null) {
	$selectedLogo = [$_SESSION['currentPDFStructure']->getLogo()];
}

?>
    <form id="inputForm" action="pdfpage.php" method="post"
          onSubmit="window.lam.utility.saveScrollPosition('inputForm')">
	<?php
$sectionElements = [];
$nonTextSectionElements = [];

$container = new htmlResponsiveRow();
$container->add(new htmlTitle(_('PDF editor')));

// main content
$mainContent = new htmlResponsiveRow();
$structureNameInput = new htmlResponsiveInputField(_("Structure name"), 'pdfname', $structureName, '360');
$structureNameInput->setRequired(true);
$mainContent->add($structureNameInput);
$mainContent->add(new htmlResponsiveInputField(_('Headline'), 'headline', $headline));
$logoSelect = new htmlResponsiveSelect('logoFile', $logos, $selectedLogo, _('Logo'));
$logoSelect->setHasDescriptiveElements(true);
$mainContent->add($logoSelect);
$foldingMarks = PDFStructure::FOLDING_NONE;
if ($_SESSION['currentPDFStructure']->getFoldingMarks() !== null) {
	$foldingMarks = $_SESSION['currentPDFStructure']->getFoldingMarks();
}
$possibleFoldingMarks = [
	_('No') => PDFStructure::FOLDING_NONE,
	_('Yes') => PDFStructure::FOLDING_STANDARD
];
$foldingMarksSelect = new htmlResponsiveSelect('foldingmarks', $possibleFoldingMarks, [$foldingMarks], _('Folding marks'));
$foldingMarksSelect->setHasDescriptiveElements(true);
$mainContent->add($foldingMarksSelect);
$mainContent->addVerticalSpacer('3rem');
// PDF structure
$structure = $_SESSION['currentPDFStructure'];
// print every entry in the current structure
$structureContent = new htmlResponsiveRow();
$sections = $structure->getSections();
foreach ($sections as $key => $section) {
	// create the up/down/remove links
	$linkUp = new htmlButton('up_section_' . $key, 'up.svg', true);
	$linkUp->setTitle(_("Up"));
	$linkDown = new htmlButton('down_section_' . $key, 'down.svg', true);
	$linkDown->setTitle(_("Down"));
	$linkRemove = new htmlButton('remove_section_' . $key, 'del.svg', true);
	$linkRemove->setTitle(_("Remove"));
	$emptyBox = new htmlSpacer('19px', null);
	// We have a new section to start
	if ($section instanceof PDFEntrySection) {
		if ($section->isAttributeTitle()) {
			$section_headline = translateFieldIDToName($section->getPdfKey(), $type->getScope(), $availablePDFFields);
			if ($section_headline === null) {
				continue;
			}
		}
		else {
			$section_headline = $section->getTitle();
		}
		$nonTextSectionElements[$section_headline] = $key;
		$sectionElements[$section_headline] = $key;
		$structureContent->addVerticalSpacer('2rem');
		// Section headline is a value entry
		if ($section->isAttributeTitle()) {
			$headlineElements = [];
			foreach ($section_items_array as $item) {
				$headlineElements[translateFieldIDToName($item, $type->getScope(), $availablePDFFields)] = '_' . $item;
			}
			$sectionHeadlineSelect = new htmlSelect('section_' . $key, $headlineElements, ['_' . $section->getPdfKey()]);
			$sectionHeadlineSelect->setHasDescriptiveElements(true);
			$structureContent->addLabel($sectionHeadlineSelect);
		}
		// Section headline is a user text
		else {
			$sectionHeadlineInput = new htmlInputField('section_' . $key, $section_headline);
			$structureContent->addLabel($sectionHeadlineInput);
		}
		$actionGroup = new htmlGroup();
		if ($key != 0) {
			$actionGroup->addElement($linkUp);
		}
		else {
			$actionGroup->addElement($emptyBox);
		}
		$hasAdditionalSections = $key < (count($sections) - 1);
		if ($hasAdditionalSections) {
			$actionGroup->addElement($linkDown);
		}
		else {
			$actionGroup->addElement($emptyBox);
		}
		$actionGroup->addElement($linkRemove);
		$structureContent->addField($actionGroup);
		// add section entries
		$sectionEntries = $section->getEntries();
		foreach ($sectionEntries as $e => $sectionEntry) {
			$fieldLabel = translateFieldIDToName($sectionEntry->getKey(), $type->getScope(), $availablePDFFields);
			if ($fieldLabel === null) {
				continue;
			}
			$structureContent->addVerticalSpacer('0.5rem');
			$fieldOutput = new htmlOutputText($fieldLabel);
			$structureContent->addLabel($fieldOutput);
			$actionGroup = new htmlGroup();
			if ($e != 0) {
				$entryLinkUp = new htmlButton('up_entry_' . $key . '_' . $e, 'up.svg', true);
				$entryLinkUp->setTitle(_("Up"));
				$actionGroup->addElement($entryLinkUp);
			}
			else {
				$actionGroup->addElement($emptyBox);
			}
			if ($e < (count($sectionEntries) - 1)) {
				$linkDown = new htmlButton('down_entry_' . $key . '_' . $e, 'down.svg', true);
				$linkDown->setTitle(_("Down"));
				$actionGroup->addElement($linkDown);
			}
			else {
				$actionGroup->addElement($emptyBox);
			}
			$entryLinkRemove = new htmlButton('remove_entry_' . $key . '_' . $e, 'del.svg', true);
			$entryLinkRemove->setTitle(_("Remove"));
			$actionGroup->addElement($entryLinkRemove);
			$structureContent->addField($actionGroup);
		}
	}
	// We have to include a static text.
    elseif ($section instanceof PDFTextSection) {
		$structureContent->addVerticalSpacer('2rem');
		// Add current satic text for dropdown box needed for the position when inserting a new
		// section or static text entry
		$textSnippet = $section->getText();
		$textSnippet = str_replace(["\n", "\r"], [" ", " "], $textSnippet);
		$textSnippet = trim($textSnippet);
		if (strlen($textSnippet) > 15) {
			$textSnippet = substr($textSnippet, 0, 15) . '...';
		}
		$textSnippet = htmlspecialchars($textSnippet);
		$sectionElements[_('Static text') . ': ' . $textSnippet] = $key;
		$sectionHeadlineOutput = new htmlOutputText(_('Static text'));
		$structureContent->add($sectionHeadlineOutput, 12, 6);
		$actionGroup = new htmlGroup();
		if ($key != 0) {
			$actionGroup->addElement($linkUp);
		}
		else {
			$actionGroup->addElement($emptyBox);
		}
		if ($key != count($sections) - 1) {
			$actionGroup->addElement($linkDown);
		}
		else {
			$actionGroup->addElement($emptyBox);
		}
		$actionGroup->addElement($linkRemove);
		$structureContent->addField($actionGroup);
		$structureContent->addVerticalSpacer('1rem');
		$staticTextOutput = new htmlOutputText($section->getText());
		$staticTextOutput->setPreformatted();
		$structureContent->add($staticTextOutput);
	}
}
$sectionElements[_('End')] = count($structure->getSections());
$mainContent->add($structureContent);
$container->add($mainContent);
$container->addVerticalSpacer('2rem');

// new field
if (!empty($nonTextSectionElements)) {
	$newFieldContainer = new htmlResponsiveRow();
	$newFieldContainer->add(new htmlSubTitle(_('New field')));
	$newFieldFieldSelect = new htmlResponsiveSelect('new_field', $newFieldFieldElements, [], _('Field'));
	$newFieldFieldSelect->setHasDescriptiveElements(true);
	$newFieldFieldSelect->setContainsOptgroups(true);
	$newFieldContainer->add($newFieldFieldSelect);
	$newFieldSectionSelect = new htmlResponsiveSelect('add_field_position', $nonTextSectionElements, [], _('Position'));
	$newFieldSectionSelect->setHasDescriptiveElements(true);
	$newFieldContainer->add($newFieldSectionSelect);
	$newFieldContainer->addLabel(new htmlOutputText('&nbsp;', false));
	$newFieldContainer->addField(new htmlButton('add_new_field', _('Add')));
	$container->add($newFieldContainer);
}

// new section
$container->addVerticalSpacer('1rem');
$newSectionContent = new htmlResponsiveRow();
$newSectionContent->add(new htmlSubTitle(_('New section')));
// add new section with text title
$newSectionContent->add(new htmlResponsiveInputField(_("Headline"), 'new_section_text'));
$newSectionPositionSelect1 = new htmlResponsiveSelect('add_sectionText_position', $sectionElements, [], _('Position'));
$newSectionPositionSelect1->setHasDescriptiveElements(true);
$newSectionPositionSelect1->setSortElements(false);
$newSectionContent->add($newSectionPositionSelect1);
$newSectionContent->addLabel(new htmlOutputText('&nbsp;', false));
$newSectionContent->addField(new htmlButton('add_sectionText', _('Add')));
$newSectionContent->addVerticalSpacer('2rem');
// add new section with field title
$newSectionFieldSelect = new htmlResponsiveSelect('new_section_item', $newFieldFieldElements, [], _("Headline"));
$newSectionFieldSelect->setHasDescriptiveElements(true);
$newSectionFieldSelect->setContainsOptgroups(true);
$newSectionContent->add($newSectionFieldSelect);
$newSectionPositionSelect2 = new htmlResponsiveSelect('add_section_position', $sectionElements, [], _('Position'));
$newSectionPositionSelect2->setHasDescriptiveElements(true);
$newSectionPositionSelect2->setSortElements(false);
$newSectionContent->add($newSectionPositionSelect2);
$newSectionContent->addLabel(new htmlOutputText('&nbsp;', false));
$newSectionContent->addField(new htmlButton('add_section', _('Add')));

// new text area
$container->add($newSectionContent);
$container->addVerticalSpacer('1rem');
$newTextFieldContent = new htmlResponsiveRow();
$newTextFieldContent->add(new htmlSubTitle(_('New text area')));
$newTextFieldContent->add(new htmlResponsiveInputTextarea('text_text', '', 40, 3, _('Static text')));
$newTextFieldPositionSelect = new htmlResponsiveSelect('add_text_position', $sectionElements, [], _('Position'));
$newTextFieldPositionSelect->setHasDescriptiveElements(true);
$newTextFieldPositionSelect->setSortElements(false);
$newTextFieldContent->add($newTextFieldPositionSelect);
$newTextFieldContent->addLabel(new htmlOutputText('&nbsp;', false));
$newTextFieldContent->addField(new htmlButton('add_text', _('Add')));
$newTextFieldContent->addVerticalSpacer('2rem');
$container->add($newTextFieldContent);

// buttons
$buttonContainer = new htmlResponsiveRow();
$saveButton = new htmlButton('submit', _("Save"));
$saveButton->setCSSClasses(['lam-primary']);
$cancelButton = new htmlButton('abort', _("Cancel"));
$cancelButton->disableFormValidation();
$buttonGroup = new htmlGroup();
$buttonGroup->addElement($saveButton);
$buttonGroup->addElement($cancelButton);
$buttonContainer->add($buttonGroup);
$buttonContainer->add(new htmlHiddenInput('modules', $modules), 4);
$buttonContainer->add(new htmlHiddenInput('type', $type->getId()), 4);
$buttonContainer->add(new htmlHiddenInput('form_submit', 'true'), 4);

$container->add($buttonContainer);
addSecurityTokenToMetaHTML($container);

if ((count($saveErrors) === 0) && isset($_POST['scrollPositionTop']) && get_preg($_POST['scrollPositionTop'], 'digit')
        && isset($_POST['scrollPositionLeft']) && get_preg($_POST['scrollPositionLeft'], 'digit')) {
	// scroll to last position
	$container->add(new htmlJavaScript('window.lam.utility.restoreScrollPosition(' . $_POST['scrollPositionTop'] . ', ' . $_POST['scrollPositionLeft'] . ')'));
}

parseHtml(null, $container, [], false, $type->getScope());

echo '</form></div>';
include __DIR__ . '/../../lib/adminFooter.inc';


/**
 * Translates a given field ID (e.g. inetOrgPerson_givenName) to its descriptive name.
 *
 * @param String $id field ID
 * @param String $scope account type
 * @param array<mixed> $availablePDFFields available PDF fields
 * @return string|null field label or null if no matching module found
 */
function translateFieldIDToName($id, $scope, $availablePDFFields): ?string {
	foreach ($availablePDFFields as $module => $fields) {
		if (!(str_starts_with($id, $module . '_'))) {
			continue;
		}
		foreach ($fields as $name => $label) {
			if ($id === $module . '_' . $name) {
				if ($module == 'main') {
					return _('Main') . ': ' . $label;
				}
				else {
					return getModuleAlias($module, $scope) . ': ' . $label;
				}
			}
		}
	}
	return null;
}

/**
 * Updates basic settings such as logo and head line.
 *
 * @param PDFStructure $structure PDF structure
 * @param PdfLogo[] $logoFiles logos
 */
function updateBasicSettings(PDFStructure &$structure, array $logoFiles): void {
	// set headline
	if (isset($_POST['headline'])) {
		$structure->setTitle(str_replace(['<', '>'], ['', ''], $_POST['headline']));
	}
	// set logo
	if (isset($_POST['logoFile'])) {
		$fileName = $_POST['logoFile'];
		if ($fileName !== PDFStructure::NO_LOGO) {
			$found = false;
			foreach ($logoFiles as $logoFile) {
				if ($logoFile->getName() === $fileName) {
					$found = true;
				}
			}
			if (!$found) {
				logNewMessage(LOG_ERR, 'Invalid PDF logo file: ' . $fileName);
				return;
			}
		}
		$structure->setLogo($fileName);
	}
	// set folding marks
	if (isset($_POST['foldingmarks'])) {
		$structure->setFoldingMarks($_POST['foldingmarks']);
	}
}

/**
 * Updates section titles.
 *
 * @param PDFStructure $structure PDF structure
 */
function updateSectionTitles(PDFStructure $structure): void {
	$sections = $structure->getSections();
	foreach ($_POST as $key => $value) {
		if (str_starts_with($key, 'section_')) {
			$pos = substr($key, strlen('section_'));
            if ($sections[$pos] instanceof PDFEntrySection) {
				$sections[$pos]->setTitle($value);
			}
		}
	}
}

/**
 * Adds a new section if requested.
 *
 * @param PDFStructure $structure PDF structure
 */
function addSection(PDFStructure &$structure): void {
	$sections = $structure->getSections();
	// add a new text field
	if (isset($_POST['add_text'])) {
		// Check if text for static text field is specified
		if (empty($_POST['text_text'])) {
			StatusMessage('ERROR', _('No static text specified'), _('The static text must contain at least one character.'));
		}
		else {
			$section = new PDFTextSection(str_replace("\r", "", $_POST['text_text']));
			array_splice($sections, $_POST['add_text_position'], 0, [$section]);
			$structure->setSections($sections);
		}
	}
	// add a new section with text headline
    elseif (isset($_POST['add_sectionText'])) {
		// Check if name for new section is specified when needed
		if (empty($_POST['new_section_text'])) {
			StatusMessage('ERROR', _('No section text specified'), _('The headline for a new section must contain at least one character.'));
		}
		else {
			$section = new PDFEntrySection($_POST['new_section_text']);
			array_splice($sections, $_POST['add_sectionText_position'], 0, [$section]);
			$structure->setSections($sections);
		}
	}
	// Add a new section with item as headline
    elseif (isset($_POST['add_section'])) {
		$section = new PDFEntrySection('_' . $_POST['new_section_item']);
		array_splice($sections, $_POST['add_section_position'], 0, [$section]);
		$structure->setSections($sections);
	}
}

/**
 * Adds a new entry to a section if requested.
 *
 * @param PDFStructure $structure PDF structure
 */
function addSectionEntry(PDFStructure $structure): void {
	if (isset($_POST['add_new_field'])) {
		$field = new PDFSectionEntry($_POST['new_field']);
		$sections = $structure->getSections();
		$pos = $_POST['add_field_position'];
        if ($sections[$pos] instanceof PDFEntrySection) {
			$entries = $sections[$pos]->getEntries();
			$entries[] = $field;
			$sections[$pos]->setEntries($entries);
			$structure->setSections($sections);
		}
	}
}

/**
 * Removes a section or entry if requested.
 *
 * @param PDFStructure $structure PDF structure
 */
function removeItem(PDFStructure &$structure): void {
	$sections = $structure->getSections();
	foreach (array_keys($_POST) as $key) {
		// remove section
		if (str_starts_with($key, 'remove_section_')) {
			$pos = substr($key, strlen('remove_section_'));
			unset($sections[$pos]);
			$sections = array_values($sections);
			$structure->setSections($sections);
		}
		// remove section entry
		if (str_starts_with($key, 'remove_entry_')) {
			$parts = substr($key, strlen('remove_entry_'));
			$parts = explode('_', $parts);
			$sectionPos = $parts[0];
			$entryPos = $parts[1];
            if ($sections[$sectionPos] instanceof PDFEntrySection) {
				$entries = $sections[$sectionPos]->getEntries();
				unset($entries[$entryPos]);
				$entries = array_values($entries);
				$sections[$sectionPos]->setEntries($entries);
				$structure->setSections($sections);
			}
		}
	}
}

/**
 * Moves up a section or entry if requested.
 *
 * @param PDFStructure $structure PDF structure
 */
function moveUp(PDFStructure &$structure): void {
	$sections = $structure->getSections();
	foreach (array_keys($_POST) as $key) {
		// move section
		if (str_starts_with($key, 'up_section_')) {
			$pos = intval(substr($key, strlen('up_section_')));
			$sectionTmp = $sections[$pos - 1];
			$sections[$pos - 1] = $sections[$pos];
			$sections[$pos] = $sectionTmp;
			$structure->setSections($sections);
		}
		// move section entry
		if (str_starts_with($key, 'up_entry_')) {
			$parts = substr($key, strlen('up_entry_'));
			$parts = explode('_', $parts);
			$sectionPos = $parts[0];
			$entryPos = intval($parts[1]);
            if ($sections[$sectionPos] instanceof PDFEntrySection) {
				$entries = $sections[$sectionPos]->getEntries();
				$entryTmp = $entries[$entryPos - 1];
				$entries[$entryPos - 1] = $entries[$entryPos];
				$entries[$entryPos] = $entryTmp;
				$sections[$sectionPos]->setEntries($entries);
				$structure->setSections($sections);
			}
		}
	}
}

/**
 * Moves down a section or entry if requested.
 *
 * @param PDFStructure $structure PDF structure
 */
function moveDown(PDFStructure &$structure): void {
	$sections = $structure->getSections();
	foreach (array_keys($_POST) as $key) {
		// move section
		if (str_starts_with($key, 'down_section_')) {
			$pos = intval(substr($key, strlen('down_section_')));
			$sectionTmp = $sections[$pos + 1];
			$sections[$pos + 1] = $sections[$pos];
			$sections[$pos] = $sectionTmp;
			$structure->setSections($sections);
		}
		// move section entry
		if (str_starts_with($key, 'down_entry_')) {
			$parts = substr($key, strlen('down_entry_'));
			$parts = explode('_', $parts);
			$sectionPos = $parts[0];
			$entryPos = intval($parts[1]);
            if ($sections[$sectionPos] instanceof PDFEntrySection) {
				$entries = $sections[$sectionPos]->getEntries();
				$entryTmp = $entries[$entryPos + 1];
				$entries[$entryPos + 1] = $entries[$entryPos];
				$entries[$entryPos] = $entryTmp;
				$sections[$sectionPos]->setEntries($entries);
				$structure->setSections($sections);
			}
		}
	}
}
