<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2007 - 2010  Roland Gruber

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
/** XML functions */
include_once('../../lib/xml_parser.inc');

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// Write $_POST variables to $_GET when form was submitted via post
if(isset($_POST['type'])) {
	$_GET = $_POST;
	if($_POST['pdfname'] == '') {
		unset($_GET['pdfname']);
	}
}


// Abort and go back to main pdf structure page
if(isset($_GET['abort'])) {
	metarefresh('pdfmain.php');
	exit;
}

// set new logo and headline
if ((isset($_GET['headline'])) && ($_GET['logoFile'] != $_SESSION['currentPageDefinitions']['filename'])) {
	$_SESSION['currentPageDefinitions']['filename'] = $_GET['logoFile'];
}
if ((isset($_GET['headline'])) && ($_GET['headline'] != $_SESSION['currentPageDefinitions']['headline'])) {
	$_SESSION['currentPageDefinitions']['headline'] = str_replace('<','',str_replace('>','',$_GET['headline']));
}

// Check if pdfname is valid, then save current structure to file and go to
// main pdf structure page
$saveErrors = array();
if(isset($_GET['submit'])) {
	if(!isset($_GET['pdfname']) || !preg_match('/[a-zA-Z0-9\-\_]+/',$_GET['pdfname'])) {
		$saveErrors[] = array('ERROR', _('PDF structure name not valid'), _('The name for that PDF-structure you submitted is not valid. A valid name must consist of the following characters: \'a-z\',\'A-Z\',\'0-9\',\'_\',\'-\'.'));
	}
	else {
		$return = savePDFStructureDefinitions($_GET['type'],$_GET['pdfname']);
		if($return == 'ok') {
			metaRefresh('pdfmain.php?savedSuccessfully=' . $_GET['pdfname']);
			exit;
		} 
		elseif($return == 'no perms'){
			$saveErrors[] = array('ERROR', _("Could not save PDF structure, access denied."), $_GET['pdfname']);
		}
	}
}
// add a new text field
elseif(isset($_GET['add_text'])) {
	// Check if text for static text field is specified
	if(!isset($_GET['text_text']) || ($_GET['text_text'] == '')) {
		StatusMessage('ERROR',_('No static text specified'),_('The static text must contain at least one character.'));
	}
	else {
		$entry = array(array('tag' => 'TEXT','type' => 'complete','level' => '2','value' => $_GET['text_text']));
		// Insert new field in structure
		array_splice($_SESSION['currentPDFStructure'],$_GET['add_text_position'],0,$entry);
	}
}
// add a new section with text headline
elseif(isset($_GET['add_sectionText'])) {
	// Check if name for new section is specified when needed
	if(!isset($_GET['section_text']) || ($_GET['section_text'] == '')) {
		StatusMessage('ERROR',_('No section text specified'),_('The headline for a new section must contain at least one character.'));
	}
	else {
		$attributes = array();		
		$attributes['NAME'] = $_GET['section_text'];
		$entry = array(array('tag' => 'SECTION','type' => 'open','level' => '2','attributes' => $attributes),array('tag' => 'SECTION','type' => 'close','level' => '2'));
		// Insert new field in structure
		array_splice($_SESSION['currentPDFStructure'],$_GET['add_sectionText_position'],0,$entry);
	}
}
// Add a new section with item as headline
elseif(isset($_GET['add_section'])) {
		$attributes = array();
		$attributes['NAME'] = '_' . $_GET['section_item'];
		$entry = array(array('tag' => 'SECTION','type' => 'open','level' => '2','attributes' => $attributes),array('tag' => 'SECTION','type' => 'close','level' => '2'));
		// Insert new field in structure
		array_splice($_SESSION['currentPDFStructure'],$_GET['add_section_position'],0,$entry);
}
// Add a new value field
elseif(isset($_GET['add_new_field'])) {
	$field = array('tag' => 'ENTRY','type' => 'complete','level' => '3','attributes' => array('NAME' => $_GET['new_field']));
	$pos = 0;
	// Find begin section to insert into
	while($pos < $_GET['add_field_position']) {
		next($_SESSION['currentPDFStructure']);
		$pos++;
	}
	$current = next($_SESSION['currentPDFStructure']);
	$pos++;
	// End of section to insert into
	while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
		$current = next($_SESSION['currentPDFStructure']);
		$pos++;
	}
	// Insert new entry before closing section tag
	array_splice($_SESSION['currentPDFStructure'],$pos,0,array($field));
}
// Remove section, static text or value entry from structure
elseif(isset($_GET['remove'])) {
	$start = 0;
	// Find element to remove
	while($start < $_GET['remove']) {
		next($_SESSION['currentPDFStructure']);
		$start++;
	}
	$remove = current($_SESSION['currentPDFStructure']);
	// We have a section to remove
	if($remove['tag'] == "SECTION") {
		$end = $start;
		$current = next($_SESSION['currentPDFStructure']);
		$end++;
		// Find end of section to remove
		while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
			$current = next($_SESSION['currentPDFStructure']);
			$end++;
		}
		// Remove complete section with all value entries in it from structure
		array_splice($_SESSION['currentPDFStructure'],$start,$end - $start + 1);
	}
	// We have a value entry to remove
	elseif($remove['tag'] == "ENTRY") {
		array_splice($_SESSION['currentPDFStructure'],$start,1);
	}
	// We hava a static text to remove
	elseif($remove['tag'] == "TEXT") {
		array_splice($_SESSION['currentPDFStructure'],$start,1);
	}
}
// Move a section, static text or value entry upwards
elseif(isset($_GET['up'])) {
	$tmp = $_SESSION['currentPDFStructure'][$_GET['up']];
	$prev = $_SESSION['currentPDFStructure'][$_GET['up'] - 1];
	// We have a section or static text to move
	if($tmp['tag'] == 'SECTION' || $tmp['tag'] == 'TEXT') {
		$pos = 0;
		$borders = array();
		$current = current($_SESSION['currentPDFStructure']);
		// Add borders of sections and static text entry to array
		if($current['tag'] == 'SECTION') {
			$borders[$current['type']][] = $pos;
		}
		elseif($current['tag'] == 'TEXT') {
			$borders['open'][] = $pos;
			$borders['close'][] = $pos;
		}
		// Find all sections and statci text fields before the section or static
		// text entry to move upwards
		while($pos < $_GET['up']) {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
			if($current['tag'] == 'SECTION') {
				$borders[$current['type']][] = $pos;
			}
			elseif($current['tag'] == 'TEXT') {
				$borders['open'][] = $pos;
				$borders['close'][] = $pos;
			}
		}
		// Move only when not topmost element
		if(count($borders['close']) > 0) {
			// We have a section to move up
			if($current['tag'] == 'SECTION') {
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
				// Find end of section to move
				while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
					$current = next($_SESSION['currentPDFStructure']);
					$pos++;
				}
				$borders['close'][] = $pos;
			}
			// Calculate the entries to move and move them
			$cut_start = $borders['open'][count($borders['open']) - 1];
			$cut_count = $borders['close'][count($borders['close']) - 1] - $borders['open'][count($borders['open']) - 1] + 1;
			$insert_pos = $borders['open'][count($borders['open']) - 2];
			$tomove = array_splice($_SESSION['currentPDFStructure'],$cut_start,$cut_count);
			array_splice($_SESSION['currentPDFStructure'],$insert_pos,0,$tomove);
		}
	}
	// We have a value entry to move; move it only if its not the topmost
	// entry in this section
	elseif($tmp['tag'] == 'ENTRY' && $prev['tag'] == 'ENTRY') {
		$_SESSION['currentPDFStructure'][$_GET['up']] = $prev;
		$_SESSION['currentPDFStructure'][$_GET['up'] - 1] = $tmp;
	}
}
// Move a section, static text field or value entry downwards
elseif(isset($_GET['down'])) {
	$tmp = $_SESSION['currentPDFStructure'][$_GET['down']];
	$next = $_SESSION['currentPDFStructure'][$_GET['down'] + 1];
	// We have a section or static text to move
	if($tmp['tag'] == 'SECTION' || $tmp['tag'] == 'TEXT') {
		$pos = 0;
		$current = current($_SESSION['currentPDFStructure']);
		// Find section or static text entry to move
		while($pos < $_GET['down']) {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		$borders = array();
		// We have a section to move
		if($current['tag'] == 'SECTION'){
			$borders[$current['type']][] = $pos;
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
			// Find end of section to move
			while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
			}
			$borders['close'][] = $pos;
		}
		// We have a static text entry to move
		elseif($current['tag'] == 'TEXT') {
			$borders['open'][] = $pos;
			$borders['close'][] = $pos;
		}
		$current = next($_SESSION['currentPDFStructure']);
		$pos++;
		// Find next section or static text entry in structure
		if($current) {
			// Next is a section
			if($current['tag'] == 'SECTION') {
				$borders[$current['type']][] = $pos;
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
				// Find end of this section
				while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
					if($current['tag'] == 'SECTION') {
						$borders[$current['type']][] = $pos;
					}
					$current = next($_SESSION['currentPDFStructure']);
					$pos++;
				}
			}
			// Next is static text entry
			elseif($current['tag'] == 'TEXT') {
				$borders['open'][] = $pos;
			}
			$borders['close'][] = $pos;
		}
		// Move only downwars if not bottenmost element of this structure
		if(count($borders['open']) > 1) {
			// Calculate entries to move and move them
			$cut_start = $borders['open'][count($borders['open']) - 1];
			$cut_count = $borders['close'][count($borders['close']) - 1] - $borders['open'][count($borders['open']) - 1] + 1;
			$insert_pos = $borders['open'][count($borders['open']) - 2];
			$tomove = array_splice($_SESSION['currentPDFStructure'],$cut_start,$cut_count);
			array_splice($_SESSION['currentPDFStructure'],$insert_pos,0,$tomove);
		}
	}
	// We have a value entry to move; move it only if it is not the bottmmost
	// element of this section.
	elseif($tmp['tag'] == 'ENTRY' && $next['tag'] == 'ENTRY') {
		$_SESSION['currentPDFStructure'][$_GET['down']] = $_SESSION['currentPDFStructure'][$_GET['down'] + 1];
		$_SESSION['currentPDFStructure'][$_GET['down'] + 1] = $tmp;
	}
}

// Change section headline
foreach ($_GET as $key => $value) {
	if(strpos($key, 'change_') === 0) {
		$alter = explode('_', $key);
		$newvalue = $_GET['section_' . $alter[1]];
		$_SESSION['currentPDFStructure'][$alter[1]]['attributes']['NAME'] = $newvalue;
	}
}

// Load PDF structure from file if it is not defined in session
if(!isset($_SESSION['currentPDFStructure'])) {
	// Load structure file to be edit
	if(isset($_GET['edit'])) {
		$load = loadPDFStructureDefinitions($_GET['type'],$_GET['edit']);
		$_SESSION['currentPDFStructure'] = $load['structure'];
		$_SESSION['currentPageDefinitions'] = $load['page_definitions'];
		$_GET['pdfname'] = $_GET['edit'];
	}
	// Load default structure file when creating a new one
	else {
		$load = loadPDFStructureDefinitions($_GET['type']);
		$_SESSION['currentPDFStructure'] = $load['structure'];
		$_SESSION['currentPageDefinitions'] = $load['page_definitions'];
	}
}

// Load available fields from modules when not set in session
if(!isset($_SESSION['availablePDFFields'])) {
	$_SESSION['availablePDFFields'] = getAvailablePDFFields($_GET['type']);
}

// Create the values for the dropdown boxes for section headline defined by
// value entries and fetch all available modules
$modules = array();
$section_items_array = array();
$section_items = '';
$sortedModules = array();
foreach($_SESSION['availablePDFFields'] as $module => $fields) {
	if ($module != 'main') {
		$title = getModuleAlias($module, $_GET['type']);
	}
	else {
		$title = _('Main');
	}
	$sortedModules[$module] = $title;
}
natcasesort($sortedModules);
foreach($sortedModules as $module => $title) {
	$values = $_SESSION['availablePDFFields'][$module];
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
	$fields = $_SESSION['availablePDFFields'][$module];
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
elseif (isset($_GET['pdfname'])) {
	$structureName = $_GET['pdfname'];
}
else if (isset($_POST['pdfname'])) {
	$structureName = $_POST['pdfname'];
}
// headline
$headline = 'LDAP Account Manager';
if (isset($_SESSION['currentPageDefinitions']['headline'])) {
	$headline = $_SESSION['currentPageDefinitions']['headline'];
}
// logo
$logoFiles = getAvailableLogos();
$logos = array(_('No logo') => 'none');
foreach($logoFiles as $logoFile) {
	$logos[$logoFile['filename'] . ' (' . $logoFile['infos'][0] . ' x ' . $logoFile['infos'][1] . ")"] = $logoFile['filename'];  
}
$selectedLogo = array('printLogo.jpg');
if (isset($_SESSION['currentPageDefinitions']['filename'])) {
	$selectedLogo = array($_SESSION['currentPageDefinitions']['filename']);
}

?>
	<form action="pdfpage.php" method="post">
<?php
$sectionElements = array(_('Beginning') => 0);
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
$mainContent->addElement(new htmlSpacer(null, '30px'), true);
// PDF structure
// print every entry in the current structure
$structureContent = new htmlTable();
foreach($_SESSION['currentPDFStructure'] as $key => $entry) {
	// create the up/down/remove links
	$linkBase = 'pdfpage.php?type=' . $_GET['type'] . '&pdfname=' . $structureName . '&headline=' . $headline . '&logoFile=' . $selectedLogo[0];
	$linkUp = new htmlLink(null, $linkBase . '&up=' . $key, '../../graphics/up.gif');
	$linkUp->setTitle(_("Up"));
	$linkDown = new htmlLink(null, $linkBase . '&down=' . $key, '../../graphics/down.gif');
	$linkDown->setTitle(_("Down"));
	$linkRemove = new htmlLink(null, $linkBase . '&remove=' . $key, '../../graphics/delete.gif');
	$linkRemove->setTitle(_("Remove"));
	// We have a new section to start
	if($entry['tag'] == "SECTION" && $entry['type'] == "open") {
		$name = $entry['attributes']['NAME'];
		if(preg_match("/^_[a-zA-Z0-9_]+_[a-zA-Z0-9_]+/",$name)) {
			$section_headline = translateFieldIDToName(substr($name,1), $_GET['type']);
		}
		else {
			$section_headline = $name;
		}
		$nonTextSectionElements[$section_headline] = $key;
		$sectionElements[$section_headline] = $key;
		$structureContent->addElement(new htmlSpacer(null, '15px'), true);
		// Section headline is a value entry
		if(preg_match("/^_[a-zA-Z0-9_]+_[a-zA-Z0-9_]+/",$name)) {
			$headlineElements = array();
			foreach($section_items_array as $item) {
				$headlineElements[translateFieldIDToName($item, $_GET['type'])] = '_' . $item;
			}
			$sectionHeadlineSelect = new htmlSelect('section_' . $key, $headlineElements, array($name));
			$sectionHeadlineSelect->setHasDescriptiveElements(true);
			$sectionHeadlineGroup = new htmlGroup();
			$sectionHeadlineGroup->addElement($sectionHeadlineSelect);
			$sectionHeadlineGroup->addElement(new htmlButton('change_' . $key, _('Change')));
			$sectionHeadlineGroup->colspan = 2;
			$structureContent->addElement($sectionHeadlineGroup);
		}
		// Section headline is a user text
		else {
			$sectionHeadlineInput = new htmlInputField('section_' . $key, $section_headline);
			$sectionHeadlineGroup = new htmlGroup();
			$sectionHeadlineGroup->addElement($sectionHeadlineInput);
			$sectionHeadlineGroup->addElement(new htmlButton('change_' . $key, _('Change')));
			$sectionHeadlineGroup->colspan = 2;
			$structureContent->addElement($sectionHeadlineGroup);
		}
		$structureContent->addElement($linkUp);
		$structureContent->addElement($linkDown);
		$structureContent->addElement($linkRemove, true);
	}
	// We have to include a static text.
	elseif($entry['tag'] == "TEXT") {
		// Add current satic text for dropdown box needed for the position when inserting a new
		// section or static text entry
		$sectionElements[_('Static text')] = $key + 1;
		$structureContent->addElement(new htmlSpacer(null, '15px'), true);
		$sectionHeadlineOutput = new htmlOutputText(_('Static text'));
		$sectionHeadlineOutput->colspan = 2;
		$structureContent->addElement($sectionHeadlineOutput);
		$structureContent->addElement($linkUp);
		$structureContent->addElement($linkDown);
		$structureContent->addElement($linkRemove, true);
		$structureContent->addElement(new htmlSpacer('10px', null));
		$staticTextOutput = new htmlOutputText($entry['value']);
		$structureContent->addElement($staticTextOutput, true);
	}
	// We have to include an entry from the account
	elseif($entry['tag'] == "ENTRY") {
		// Get name of current entry
		$name = $entry['attributes']['NAME'];
		$structureContent->addElement(new htmlSpacer('10px', null));
		$fieldOutput = new htmlOutputText(translateFieldIDToName($name, $_GET['type']));
		$structureContent->addElement($fieldOutput);
		$structureContent->addElement($linkUp);
		$structureContent->addElement($linkDown);
		$structureContent->addElement($linkRemove, true);
	}
}
$structureContent->colspan = 3;
$mainContent->addElement($structureContent);
$container->addElement(new htmlFieldset($mainContent), true);
$container->addElement(new htmlSpacer(null, '15px'), true);

// new section
$container->addElement(new htmlSubTitle(_('New section')), true);
$newSectionContent = new htmlTable();
// add new section with text title
$newSectionContent->addElement(new htmlTableExtendedInputField(_("Headline"), 'section_text'));
$newSectionPositionSelect1 = new htmlTableExtendedSelect('add_sectionText_position', $sectionElements, array(), _('Position'));
$newSectionPositionSelect1->setHasDescriptiveElements(true);
$newSectionPositionSelect1->setSortElements(false);
$newSectionContent->addElement($newSectionPositionSelect1);
$newSectionContent->addElement(new htmlButton('add_sectionText', _('Add')), true);
// add new section with field title
$newSectionFieldSelect = new htmlTableExtendedSelect('section_item', $newFieldFieldElements, array(), _("Headline"));
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

// buttons
$buttonContainer = new htmlTable();
$saveButton = new htmlButton('submit', _("Save"));
$saveButton->setIconClass('saveButton');
$cancelButton = new htmlButton('abort', _("Cancel"));
$cancelButton->setIconClass('cancelButton');
$buttonContainer->addElement($saveButton);
$buttonContainer->addElement($cancelButton);
$buttonContainer->addElement(new htmlHiddenInput('modules', $modules));
$buttonContainer->addElement(new htmlHiddenInput('type', $_GET['type']));

$container->addElement($buttonContainer, true);

$tabindex = 1;
parseHtml(null, $container, array(), false, $tabindex, $_GET['type']);

echo '</form>';
include '../main_footer.php';


/**
 * Translates a given field ID (e.g. inetOrgPerson_givenName) to its descriptive name.
 *
 * @param String $id field ID
 * @param String $scope account type
 */
function translateFieldIDToName($id, $scope) {
	foreach ($_SESSION['availablePDFFields'] as $module => $fields) {
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

?>
