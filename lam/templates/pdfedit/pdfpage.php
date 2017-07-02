<?php
namespace LAM\TOOLS\PDF_EDITOR;
use \htmlTable;
use \htmlTitle;
use \htmlTableExtendedInputField;
use \htmlSpacer;
use \htmlTableExtendedSelect;
use \htmlButton;
use \htmlOutputText;
use \htmlGroup;
use \htmlSelect;
use \htmlInputField;
use \htmlSubTitle;
use \htmlFieldset;
use \htmlInputTextarea;
use \htmlHiddenInput;
use LAM\PDF\PDFStructureReader;
use LAM\PDF\PDFTextSection;
use LAM\PDF\PDFEntrySection;
use LAM\PDF\PDFStructure;
use LAM\PDF\PDFSectionEntry;
use LAM\PDF\PDFStructureWriter;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2007 - 2017  Roland Gruber

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
include_once("../../lib/security.inc");
/** access to PDF configuration files */
include_once('../../lib/pdfstruct.inc');
/** LDAP object */
include_once('../../lib/ldap.inc');
/** LAM configuration */
include_once('../../lib/config.inc');
/** module functions */
include_once('../../lib/modules.inc');

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

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

// Write $_POST variables to $_GET when form was submitted via post
if (isset($_POST['type'])) {
	$_GET = $_POST;
}

$typeManager = new \LAM\TYPES\TypeManager();
$type = $typeManager->getConfiguredType($_GET['type']);
if ($type->isHidden() || !checkIfWriteAccessIsAllowed($type->getId())) {
	logNewMessage(LOG_ERR, 'User tried to access hidden PDF structure: ' . $type->getId());
	die();
}


// Abort and go back to main pdf structure page
if(isset($_GET['abort'])) {
	metarefresh('pdfmain.php');
	exit;
}

// Load PDF structure from file if it is not defined in session
if(!isset($_SESSION['currentPDFStructure'])) {
	// Load structure file to be edit
	$reader = new PDFStructureReader();
	try {
		if(isset($_GET['edit'])) {
			$_SESSION['currentPDFStructure'] = $reader->read($type->getId(), $_GET['edit']);
		}
		// Load default structure file when creating a new one
		else {
			$_SESSION['currentPDFStructure'] = $reader->read($type->getId(), 'default');
		}
	}
	catch (\LAMException $e) {
		metaRefresh('pdfmain.php?loadFailed=1&name=' . $_GET['edit']);
		exit;
	}
}

if (!empty($_POST['form_submit'])) {
	updateBasicSettings($_SESSION['currentPDFStructure']);
	updateSectionTitles($_SESSION['currentPDFStructure']);
	addSection($_SESSION['currentPDFStructure']);
	addSectionEntry($_SESSION['currentPDFStructure']);
	removeItem($_SESSION['currentPDFStructure']);
	moveUp($_SESSION['currentPDFStructure']);
	moveDown($_SESSION['currentPDFStructure']);
}

// Check if pdfname is valid, then save current structure to file and go to
// main pdf structure page
$saveErrors = array();
if(isset($_GET['submit'])) {
	$writer = new PDFStructureWriter();
	try {
		$writer->write($type->getId(), $_POST['pdfname'], $_SESSION['currentPDFStructure']);
		unset($_SESSION['currentPDFStructure']);
		metaRefresh('pdfmain.php?savedSuccessfully=' . $_POST['pdfname']);
		exit;
	}
	catch (\LAMException $e) {
		$saveErrors[] = array('ERROR', $e->getTitle(), $e->getMessage());
	}
}

$availablePDFFields = getAvailablePDFFields($type->getId());

// Create the values for the dropdown boxes for section headline defined by
// value entries and fetch all available modules
$modules = array();
$section_items_array = array();
$section_items = '';
$sortedModules = array();
foreach($availablePDFFields as $module => $fields) {
	if ($module != 'main') {
		$title = getModuleAlias($module, $type->getScope());
	}
	else {
		$title = _('Main');
	}
	$sortedModules[$module] = $title;
}
natcasesort($sortedModules);
foreach($sortedModules as $module => $title) {
	$values = $availablePDFFields[$module];
	if (!is_array($values) || (sizeof($values) < 1)) {
		continue;
	}
	$modules[] = $module;
	$section_items .= "<optgroup label=\"" . $title . "\"\n>";
	natcasesort($values);
	foreach($values as $attribute => $attributeLabel) {
		$section_items_array[] = $module . '_' . $attribute;
		$section_items .= "<option value=\"" . $module . '_' . $attribute . "\">" . $attributeLabel . "</option>\n";
	}
	$section_items .= "</optgroup>\n";
}
$modules = join(',',$modules);

// print header
include '../main_header.php';

// print error messages if any
if (sizeof($saveErrors) > 0) {
	for ($i = 0; $i < sizeof($saveErrors); $i++) {
		call_user_func_array('StatusMessage', $saveErrors[$i]);
	}
	echo "<br>\n";
}

$newFieldFieldElements = array();
foreach($sortedModules as $module => $title) {
	$fields = $availablePDFFields[$module];
	if (isset($fields) && is_array($fields) && (sizeof($fields) > 0)) {
		$moduleFields = array();
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
else if (isset($_POST['pdfname'])) {
	$structureName = $_POST['pdfname'];
}
// headline
$headline = $_SESSION['currentPDFStructure']->getTitle();
// logo
$logoFiles = \LAM\PDF\getAvailableLogos();
$logos = array(_('No logo') => 'none');
foreach($logoFiles as $logoFile) {
	$logos[$logoFile['filename'] . ' (' . $logoFile['infos'][0] . ' x ' . $logoFile['infos'][1] . ")"] = $logoFile['filename'];
}
$selectedLogo = array('printLogo.jpg');
if (isset($_SESSION['currentPDFStructure'])) {
	$selectedLogo = array($_SESSION['currentPDFStructure']->getLogo());
}

?>
	<form id="inputForm" action="pdfpage.php" method="post" onSubmit="saveScrollPosition('inputForm')">
<?php
$sectionElements = array();
$nonTextSectionElements = array();

$container = new htmlTable();
$container->addElement(new htmlTitle(_('PDF editor')), true);

// main content
$mainContent = new htmlTable();
$structureNameInput = new htmlTableExtendedInputField(_("Structure name"), 'pdfname', $structureName, '360');
$structureNameInput->setRequired(true);
$mainContent->addElement($structureNameInput, true);
$mainContent->addElement(new htmlTableExtendedInputField(_('Headline'), 'headline', $headline), true);
$logoSelect = new htmlTableExtendedSelect('logoFile', $logos, $selectedLogo, _('Logo'));
$logoSelect->setHasDescriptiveElements(true);
$mainContent->addElement($logoSelect, true);
$foldingMarks = 'no';
if (isset($_SESSION['currentPDFStructure'])) {
	$foldingMarks = $_SESSION['currentPDFStructure']->getFoldingMarks();
}
$possibleFoldingMarks = array(_('No') => 'no', _('Yes') => 'standard');
$foldingMarksSelect = new htmlTableExtendedSelect('foldingmarks', $possibleFoldingMarks, array($foldingMarks), _('Folding marks'));
$foldingMarksSelect->setHasDescriptiveElements(true);
$mainContent->addElement($foldingMarksSelect, true);
$mainContent->addElement(new htmlSpacer(null, '30px'), true);
// PDF structure
$structure = $_SESSION['currentPDFStructure'];
// print every entry in the current structure
$structureContent = new htmlTable();
$sections = $structure->getSections();
for ($key = 0; $key < sizeof($sections); $key++) {
	$section = $sections[$key];
	// create the up/down/remove links
	$linkUp = new htmlButton('up_section_' . $key, 'up.gif', true);
	$linkUp->setTitle(_("Up"));
	$linkDown = new htmlButton('down_section_' . $key, 'down.gif', true);
	$linkDown->setTitle(_("Down"));
	$linkRemove = new htmlButton('remove_section_' . $key, 'delete.gif', true);
	$linkRemove->setTitle(_("Remove"));
	$emptyBox = new htmlOutputText('');
	// We have a new section to start
	if($section instanceof PDFEntrySection) {
		if($section->isAttributeTitle()) {
			$section_headline = translateFieldIDToName($section->getPdfKey(), $type->getScope(), $availablePDFFields);
		}
		else {
			$section_headline = $section->getTitle();
		}
		$nonTextSectionElements[$section_headline] = $key;
		$sectionElements[$section_headline] = $key;
		$structureContent->addElement(new htmlSpacer(null, '15px'), true);
		// Section headline is a value entry
		if($section->isAttributeTitle()) {
			$headlineElements = array();
			foreach($section_items_array as $item) {
				$headlineElements[translateFieldIDToName($item, $type->getScope(), $availablePDFFields)] = '_' . $item;
			}
			$sectionHeadlineSelect = new htmlSelect('section_' . $key, $headlineElements, array('_' . $section->getPdfKey()));
			$sectionHeadlineSelect->setHasDescriptiveElements(true);
			$sectionHeadlineGroup = new htmlGroup();
			$sectionHeadlineGroup->addElement($sectionHeadlineSelect);
			$sectionHeadlineGroup->colspan = 2;
			$structureContent->addElement($sectionHeadlineGroup);
		}
		// Section headline is a user text
		else {
			$sectionHeadlineInput = new htmlInputField('section_' . $key, $section_headline);
			$sectionHeadlineGroup = new htmlGroup();
			$sectionHeadlineGroup->addElement($sectionHeadlineInput);
			$sectionHeadlineGroup->colspan = 2;
			$structureContent->addElement($sectionHeadlineGroup);
		}
		if ($key != 0) {
			$structureContent->addElement($linkUp);
		}
		else {
			$structureContent->addElement($emptyBox);
		}
		$hasAdditionalSections = $key < (sizeof($sections) - 1);
		if ($hasAdditionalSections) {
			$structureContent->addElement($linkDown);
		}
		else {
			$structureContent->addElement($emptyBox);
		}
		$structureContent->addElement($linkRemove, true);
		// add section entries
		$sectionEntries = $section->getEntries();
		for ($e = 0; $e < sizeof($sectionEntries); $e++) {
			$sectionEntry = $sectionEntries[$e];
			$structureContent->addElement(new htmlSpacer('10px', null));
			$fieldOutput = new htmlOutputText(translateFieldIDToName($sectionEntry->getKey(), $type->getScope(), $availablePDFFields));
			$structureContent->addElement($fieldOutput);
			if ($e != 0) {
				$entryLinkUp = new htmlButton('up_entry_' . $key . '_' . $e, 'up.gif', true);
				$entryLinkUp->setTitle(_("Up"));
				$structureContent->addElement($entryLinkUp);
			}
			else {
				$structureContent->addElement($emptyBox);
			}
			if ($e < (sizeof($sectionEntries) - 1)) {
				$linkDown = new htmlButton('down_entry_' . $key . '_' . $e, 'down.gif', true);
				$linkDown->setTitle(_("Down"));
				$structureContent->addElement($linkDown);
			}
			else {
				$structureContent->addElement($emptyBox);
			}
			$entryLinkRemove = new htmlButton('remove_entry_' . $key . '_' . $e, 'delete.gif', true);
			$entryLinkRemove->setTitle(_("Remove"));
			$structureContent->addElement($entryLinkRemove, true);
		}
	}
	// We have to include a static text.
	elseif($section instanceof PDFTextSection) {
		// Add current satic text for dropdown box needed for the position when inserting a new
		// section or static text entry
		$textSnippet = $section->getText();
		$textSnippet = str_replace(array("\n", "\r"), array(" ", " "), $textSnippet);
		$textSnippet = trim($textSnippet);
		if (strlen($textSnippet) > 15) {
			$textSnippet = substr($textSnippet, 0, 15) . '...';
		}
		$textSnippet = htmlspecialchars($textSnippet);
		$sectionElements[_('Static text') . ': ' . $textSnippet] = $key;
		$structureContent->addElement(new htmlSpacer(null, '15px'), true);
		$sectionHeadlineOutput = new htmlOutputText(_('Static text'));
		$sectionHeadlineOutput->colspan = 2;
		$structureContent->addElement($sectionHeadlineOutput);
		if ($key != 0) {
			$structureContent->addElement($linkUp);
		}
		else {
			$structureContent->addElement($emptyBox);
		}
		if ($key != sizeof($sections) - 1) {
			$structureContent->addElement($linkDown);
		}
		else {
			$structureContent->addElement($emptyBox);
		}
		$structureContent->addElement($linkRemove, true);
		$structureContent->addElement(new htmlSpacer('10px', null));
		$staticTextOutput = new htmlOutputText($section->getText());
		$staticTextOutput->setPreformatted();
		$structureContent->addElement($staticTextOutput, true);
	}
}
$sectionElements[_('End')] = sizeof($structure->getSections());
$structureContent->colspan = 3;
$mainContent->addElement($structureContent);
$container->addElement(new htmlFieldset($mainContent), true);
$container->addElement(new htmlSpacer(null, '15px'), true);

// new section
$container->addElement(new htmlSubTitle(_('New section')), true);
$newSectionContent = new htmlTable();
// add new section with text title
$newSectionContent->addElement(new htmlTableExtendedInputField(_("Headline"), 'new_section_text'));
$newSectionPositionSelect1 = new htmlTableExtendedSelect('add_sectionText_position', $sectionElements, array(), _('Position'));
$newSectionPositionSelect1->setHasDescriptiveElements(true);
$newSectionPositionSelect1->setSortElements(false);
$newSectionContent->addElement($newSectionPositionSelect1);
$newSectionContent->addElement(new htmlButton('add_sectionText', _('Add')), true);
// add new section with field title
$newSectionFieldSelect = new htmlTableExtendedSelect('new_section_item', $newFieldFieldElements, array(), _("Headline"));
$newSectionFieldSelect->setHasDescriptiveElements(true);
$newSectionFieldSelect->setContainsOptgroups(true);
$newSectionContent->addElement($newSectionFieldSelect);
$newSectionPositionSelect2 = new htmlTableExtendedSelect('add_section_position', $sectionElements, array(), _('Position'));
$newSectionPositionSelect2->setHasDescriptiveElements(true);
$newSectionPositionSelect2->setSortElements(false);
$newSectionContent->addElement($newSectionPositionSelect2);
$newSectionContent->addElement(new htmlButton('add_section', _('Add')));

$container->addElement(new htmlFieldset($newSectionContent, _("Section")), true);
$container->addElement(new htmlSpacer(null, '10px'), true);
$newTextFieldContent = new htmlTable();
$newTextFieldContent->addElement(new htmlInputTextarea('text_text', '', 40, 3));
$newTextFieldPositionSelect = new htmlTableExtendedSelect('add_text_position', $sectionElements, array(), _('Position'));
$newTextFieldPositionSelect->setHasDescriptiveElements(true);
$newTextFieldPositionSelect->setSortElements(false);
$newTextFieldContent->addElement($newTextFieldPositionSelect);
$newTextFieldContent->addElement(new htmlButton('add_text', _('Add')));
$container->addElement(new htmlFieldset($newTextFieldContent, _("Text field")), true);

// new field
if (!empty($nonTextSectionElements)) {
	$container->addElement(new htmlSubTitle(_('New field')), true);
	$newFieldContainer = new htmlTable();
	$newFieldFieldSelect = new htmlSelect('new_field', $newFieldFieldElements);
	$newFieldFieldSelect->setHasDescriptiveElements(true);
	$newFieldFieldSelect->setContainsOptgroups(true);
	$newFieldContainer->addElement($newFieldFieldSelect);
	$newFieldContainer->addElement(new htmlSpacer('10px', null));
	$newFieldSectionSelect = new htmlTableExtendedSelect('add_field_position', $nonTextSectionElements, array(), _('Position'));
	$newFieldSectionSelect->setHasDescriptiveElements(true);
	$newFieldContainer->addElement($newFieldSectionSelect);
	$newFieldContainer->addElement(new htmlButton('add_new_field', _('Add')));
	$container->addElement(new htmlFieldset($newFieldContainer), true);
	$container->addElement(new htmlSpacer(null, '20px'), true);
}

// buttons
$buttonContainer = new htmlTable();
$saveButton = new htmlButton('submit', _("Save"));
$saveButton->setIconClass('saveButton');
$cancelButton = new htmlButton('abort', _("Cancel"));
$cancelButton->setIconClass('cancelButton');
$buttonContainer->addElement($saveButton);
$buttonContainer->addElement($cancelButton);
$buttonContainer->addElement(new htmlHiddenInput('modules', $modules));
$buttonContainer->addElement(new htmlHiddenInput('type', $type->getId()));
$buttonContainer->addElement(new htmlHiddenInput('form_submit', 'true'));

$container->addElement($buttonContainer, true);
addSecurityTokenToMetaHTML($container);

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, $type->getScope());

if ((sizeof($saveErrors) == 0) && isset($_POST['scrollPositionTop']) && isset($_POST['scrollPositionLeft'])) {
	// scroll to last position
	echo '<script type="text/javascript">
		jQuery(document).ready(function() {
			jQuery(window).scrollTop(' . $_POST['scrollPositionTop'] . ');
			jQuery(window).scrollLeft('. $_POST['scrollPositionLeft'] . ');
	});
	</script>';
}

echo '</form>';
include '../main_footer.php';


/**
 * Translates a given field ID (e.g. inetOrgPerson_givenName) to its descriptive name.
 *
 * @param String $id field ID
 * @param String $scope account type
 * @param array $availablePDFFields available PDF fields
 */
function translateFieldIDToName($id, $scope, $availablePDFFields) {
	foreach ($availablePDFFields as $module => $fields) {
		if (!(strpos($id, $module . '_') === 0)) {
			continue;
		}
		foreach ($fields as $name => $label) {
			if ($id == $module . '_' . $name) {
				if ($module == 'main') {
					return _('Main') . ': ' . $label;
				}
				else  {
					return getModuleAlias($module, $scope) . ': ' . $label;
				}
			}
		}
	}
	return $id;
}

/**
 * Updates basic settings such as logo and head line.
 *
 * @param PDFStructure $structure
 */
function updateBasicSettings(&$structure) {
	// set headline
	if (isset($_POST['headline'])) {
		$structure->setTitle(str_replace('<', '', str_replace('>', '', $_POST['headline'])));
	}
	// set logo
	if (isset($_POST['logoFile'])) {
		$structure->setLogo($_POST['logoFile']);
	}
	// set folding marks
	if (isset($_POST['foldingmarks'])) {
		$structure->setFoldingMarks($_POST['foldingmarks']);
	}
}

/**
 * Updates section titles.
 *
 * @param PDFStructure $structure
 */
function updateSectionTitles(&$structure) {
	$sections = $structure->getSections();
	foreach ($_POST as $key => $value) {
		if (strpos($key, 'section_') === 0) {
			$pos = substr($key, strlen('section_'));
			$sections[$pos]->setTitle($value);
		}
	}
}

/**
 * Adds a new section if requested.
 *
 * @param PDFStructure $structure
 */
function addSection(&$structure) {
	$sections = $structure->getSections();
	// add a new text field
	if(isset($_POST['add_text'])) {
		// Check if text for static text field is specified
		if(empty($_POST['text_text'])) {
			StatusMessage('ERROR',_('No static text specified'),_('The static text must contain at least one character.'));
		}
		else {
			$section = new PDFTextSection(str_replace("\r", "", $_POST['text_text']));
			array_splice($sections, $_POST['add_text_position'], 0, array($section));
			$structure->setSections($sections);
		}
	}
	// add a new section with text headline
	elseif(isset($_POST['add_sectionText'])) {
		// Check if name for new section is specified when needed
		if(empty($_POST['new_section_text'])) {
			StatusMessage('ERROR',_('No section text specified'),_('The headline for a new section must contain at least one character.'));
		}
		else {
			$section = new PDFEntrySection($_POST['new_section_text']);
			array_splice($sections, $_POST['add_text_position'], 0, array($section));
			$structure->setSections($sections);
		}
	}
	// Add a new section with item as headline
	elseif(isset($_POST['add_section'])) {
		$section = new PDFEntrySection('_' . $_POST['new_section_item']);
		array_splice($sections, $_POST['add_text_position'], 0, array($section));
		$structure->setSections($sections);
	}
}

/**
 * Adds a new entry to a section if requested.
 *
 * @param PDFStructure $structure
 */
function addSectionEntry(&$structure) {
	if(isset($_POST['add_new_field'])) {
		$field = new PDFSectionEntry($_POST['new_field']);
		$sections = $structure->getSections();
		$pos = $_POST['add_field_position'];
		$entries = $sections[$pos]->getEntries();
		$entries[] = $field;
		$sections[$pos]->setEntries($entries);
		$structure->setSections($sections);
	}
}

/**
 * Removes a section or entry if requested.
 *
 * @param PDFStructure $structure
 */
function removeItem(&$structure) {
	$sections = $structure->getSections();
	foreach ($_POST as $key => $value) {
		// remove section
		if (strpos($key, 'remove_section_') === 0) {
			$pos = substr($key, strlen('remove_section_'));
			unset($sections[$pos]);
			$sections = array_values($sections);
			$structure->setSections($sections);
		}
		// remove section entry
		if (strpos($key, 'remove_entry_') === 0) {
			$parts = substr($key, strlen('remove_entry_'));
			$parts = explode('_', $parts);
			$sectionPos = $parts[0];
			$entryPos = $parts[1];
			$entries = $sections[$sectionPos]->getEntries();
			unset($entries[$entryPos]);
			$entries = array_values($entries);
			$sections[$sectionPos]->setEntries($entries);
			$structure->setSections($sections);
		}
	}
}

/**
 * Moves up a section or entry if requested.
 *
 * @param PDFStructure $structure
 */
function moveUp(&$structure) {
	$sections = $structure->getSections();
	foreach ($_POST as $key => $value) {
		// move section
		if (strpos($key, 'up_section_') === 0) {
			$pos = substr($key, strlen('up_section_'));
			$sectionTmp = $sections[$pos - 1];
			$sections[$pos - 1] = $sections[$pos];
			$sections[$pos] = $sectionTmp;
			$structure->setSections($sections);
		}
		// move section entry
		if (strpos($key, 'up_entry_') === 0) {
			$parts = substr($key, strlen('up_entry_'));
			$parts = explode('_', $parts);
			$sectionPos = $parts[0];
			$entryPos = $parts[1];
			$entries = $sections[$sectionPos]->getEntries();
			$entryTmp = $entries[$entryPos - 1];
			$entries[$entryPos - 1] = $entries[$entryPos];
			$entries[$entryPos] = $entryTmp;
			$sections[$sectionPos]->setEntries($entries);
			$structure->setSections($sections);
		}
	}
}

/**
 * Moves down a section or entry if requested.
 *
 * @param PDFStructure $structure
 */
function moveDown(&$structure) {
	$sections = $structure->getSections();
	foreach ($_POST as $key => $value) {
		// move section
		if (strpos($key, 'down_section_') === 0) {
			$pos = substr($key, strlen('down_section_'));
			$sectionTmp = $sections[$pos + 1];
			$sections[$pos + 1] = $sections[$pos];
			$sections[$pos] = $sectionTmp;
			$structure->setSections($sections);
		}
		// move section entry
		if (strpos($key, 'down_entry_') === 0) {
			$parts = substr($key, strlen('down_entry_'));
			$parts = explode('_', $parts);
			$sectionPos = $parts[0];
			$entryPos = $parts[1];
			$entries = $sections[$sectionPos]->getEntries();
			$entries = $sections[$sectionPos]->getEntries();
			$entryTmp = $entries[$entryPos + 1];
			$entries[$entryPos + 1] = $entries[$entryPos];
			$entries[$entryPos] = $entryTmp;
			$sections[$sectionPos]->setEntries($entries);
			$structure->setSections($sections);
		}
	}
}

?>
