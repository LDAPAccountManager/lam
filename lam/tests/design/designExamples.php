<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021  Roland Gruber

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
* Displays a list of input elements.
*
* @author Roland Gruber
* @package tools
*/

include_once(__DIR__ . "/../../lib/account.inc");
include_once(__DIR__ . "/../../lib/modules.inc");
include_once(__DIR__ . "/../../lib/html.inc");

echo '<head>';
$prefix = '../..';
printHeaderContents("Design Examples", $prefix);
echo "</head><body>\n";

// include all JavaScript files
printJsIncludes($prefix);

$row = new htmlResponsiveRow();

$row->add(new htmlTitle('Design'));

$row->add(new htmlSubTitle('Buttons'));

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlButton('name1', 'Click me'));

$row->addLabel(new htmlOutputText('Primary'));
$primaryButton = new htmlButton('name2', 'Click me');
$primaryButton->setCSSClasses(array('lam-primary'));
$row->addField($primaryButton);

$row->addLabel(new htmlOutputText('Secondary'));
$primaryButton = new htmlButton('name2a', 'Click me');
$primaryButton->setCSSClasses(array('lam-secondary'));
$row->addField($primaryButton);

$row->addLabel(new htmlOutputText('Danger'));
$primaryButton = new htmlButton('name3', 'Click me');
$primaryButton->setCSSClasses(array('lam-danger'));
$row->addField($primaryButton);

$row->addLabel(new htmlOutputText('Disabled'));
$buttonDisabled = new htmlButton('name3', 'Click me');
$buttonDisabled->setIsEnabled(false);
$row->addField($buttonDisabled);

$row->add(new htmlSubTitle('Tables'));
$tableTitles = array('text 1', 'text 2', 'text 3', 'text 4', 'text 5');
$tableData = array();
for ($rowNumber = 0; $rowNumber < 10; $rowNumber++) {
    $tableRow = array();
    for ($column = 0; $column < 5; $column++) {
        $tableRow[] = new htmlOutputText('value' . $column);
    }
    $tableData[] = $tableRow;
}
$table = new htmlResponsiveTable($tableTitles, $tableData);
$row->add($table);

$row->addVerticalSpacer('2rem');
$row->add(new htmlOutputText('Account list'));
$table = new htmlResponsiveTable($tableTitles, $tableData);
$table->setCSSClasses(array('accountlist'));
$row->add($table);

$row->add(new htmlSubTitle('Input fields'));

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlInputField('text1', 'Some text'));

$row->addLabel(new htmlOutputText('Autocomplete'));
$autocompleteInput = new htmlInputField('text1a', 'Some text');
$autocompleteInput->enableAutocompletion(array('Some text', 'Some text2', 'Some text3', 'Some text4'));
$row->addField($autocompleteInput);

$row->addLabel(new htmlOutputText('Disabled'));
$textDisabled = new htmlInputField('text2', 'Some text');
$textDisabled->setIsEnabled(false);
$row->addField($textDisabled);

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlInputTextarea('textarea1', 'Some text', 50, 5));

$row->addLabel(new htmlOutputText('Disabled'));
$textAreaDisabled = new htmlInputTextarea('textarea2', 'Some text', 50, 5);
$textAreaDisabled->setIsEnabled(false);
$row->addField($textAreaDisabled);

$checkRow1 = new htmlResponsiveRow();
$checkRow1->add(new htmlResponsiveInputCheckbox('check1', true, 'Default'));
$row->add($checkRow1);

$checkRow2 = new htmlResponsiveRow();
$checkboxDisabled = new htmlResponsiveInputCheckbox('check2', true, 'Disabled');
$checkboxDisabled->setIsEnabled(false);
$checkRow2->add($checkboxDisabled);
$row->add($checkRow2);

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlRadio('radio1', array('label1' => 'value', 'label2' => 'value2'), 'value'));

$row->addLabel(new htmlOutputText('Disabled'));
$radioDisabled = new htmlRadio('radio2', array('label1' => 'value', 'label2' => 'value2'), 'value');
$radioDisabled->setIsEnabled(false);
$row->addField($radioDisabled);

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlInputFileUpload('file1'));

$row->addLabel(new htmlOutputText('Disabled'));
$uploadDisabled = new htmlInputFileUpload('file2');
$uploadDisabled->setIsEnabled(false);
$row->addField($uploadDisabled);

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlInputColorPicker('color1'));

$row->addLabel(new htmlOutputText('Disabled'));
$colorDisabled = new htmlInputColorPicker('color2');
$colorDisabled->setIsEnabled(false);
$row->addField($colorDisabled);

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlSelect('select1', array(1, 2, 3), array(2)));

$row->addLabel(new htmlOutputText('Disabled'));
$selectDisabled = new htmlSelect('select2', array(1, 2, 3), array(2));
$selectDisabled->setIsEnabled(false);
$row->addField($selectDisabled);

$row->addLabel(new htmlOutputText('Default'));
$multiSelect1 = new htmlSelect('select1', array("1", "2", "3"), array("1", "3"), 5);
$multiSelect1->setMultiSelect(true);
$row->addField($multiSelect1);

$row->addLabel(new htmlOutputText('Disabled'));
$multiSelect2 = new htmlSelect('select2', array("1", "2", "3"), array("1", "3"), 5);
$multiSelect2->setIsEnabled(false);
$multiSelect2->setMultiSelect(true);
$row->addField($multiSelect2);


$row->add(new htmlSubTitle('Messages'));

$row->add(new htmlStatusMessage('INFO', 'Title', 'Text'));
$row->add(new htmlStatusMessage('INFO', 'Title'));
$row->add(new htmlStatusMessage('WARN', 'Title', 'Text'));
$row->add(new htmlStatusMessage('WARN', 'Title'));
$row->add(new htmlStatusMessage('ERROR', 'Title', 'Text'));
$row->add(new htmlStatusMessage('ERROR', 'Title'));


$row->add(new htmlSubTitle('Links'));

$row->addLabel(new htmlOutputText('Link'));
$row->addField(new htmlLink('linked text', 'designExamples.php'));

$row->add(new htmlSubTitle('Line'));

$row->add(new htmlHorizontalLine());


$row->add(new htmlSpacer(null, '5rem'));


$row->add(new htmlSubTitle('Progress bar'));

$row->add(new htmlProgressbar('progressBar', 33));


$row->add(new htmlSpacer(null, '5rem'));

$row->add(new htmlSubTitle('Accordion'));

$accordionElementsSingle = array();
$accordionElementsSingleContent1 = new htmlResponsiveRow();
$accordionElementsSingleContent1->add(new htmlResponsiveInputField('Input 1', 'acc1i1'));
$accordionElementsSingleContent1->add(new htmlResponsiveInputField('Input 2', 'acc1i2'));
$accordionElementsSingleContent1->add(new htmlResponsiveInputTextarea('acc1i3', '', 20, 3, 'Text area'));
$accordionElementsSingle['Accordion'] = $accordionElementsSingleContent1;
$row->add(new htmlAccordion('acc_single', $accordionElementsSingle));

$row->add(new htmlSpacer(null, '5rem'));

$accordionElementsSingleClosed = array();
$accordionElementsSingleContentClosed = new htmlResponsiveRow();
$accordionElementsSingleContentClosed->add(new htmlResponsiveInputField('Input 1', 'acc1ai1'));
$accordionElementsSingleContentClosed->add(new htmlResponsiveInputField('Input 2', 'acc1ai2'));
$accordionElementsSingleContentClosed->add(new htmlResponsiveInputTextarea('acc1ai3', '', 20, 3, 'Text area'));
$accordionElementsSingleClosed['Accordion - initially closed'] = $accordionElementsSingleContentClosed;
$row->add(new htmlAccordion('acc_singleClosed', $accordionElementsSingleClosed, false));

$row->add(new htmlSpacer(null, '5rem'));

$accordionElementsMulti = array();
for ($i = 0; $i < 5; $i++) {
	$accordionElementsContent = new htmlResponsiveRow();
	$accordionElementsContent->add(new htmlResponsiveInputField('Input 1', 'acc1i1' . $i));
	$accordionElementsContent->add(new htmlResponsiveInputField('Input 2', 'acc1i2' . $i));
	$accordionElementsContent->add(new htmlResponsiveInputTextarea('acc1i3' . $i, '', 20, 3, 'Text area'));
	$accordionElementsMulti['Accordion ' . $i] = $accordionElementsContent;
}
$row->add(new htmlAccordion('acc_multi', $accordionElementsMulti));

$row->add(new htmlSpacer(null, '5rem'));

$row->add(new htmlSubTitle('Sortable list'));

$sortableList1 = new htmlSortableList(array(
	'text 1',
	'text 2',
	'text 3',
	'text 4',
	'text 5',
), 'sortableList1');
$sortableList1->setCSSClasses(array('module-list'));
$row->add($sortableList1);

$row->add(new htmlSpacer(null, '2rem'));

$listElements = array();
for ($i = 0; $i < 8; $i++) {
	$el = new htmlTable('100%');
	$image = new htmlImage('../../graphics/tux.svg');
	$image->setCSSClasses(array('size16', 'margin-right5-mobile-only'));
	$el->addElement($image);
	$el->addElement(new htmlOutputText("Text " . $i));
	$image2 = new htmlImage('../../graphics/del.svg');
	$image2->setCSSClasses(array('size16', 'margin-right5-mobile-only'));
	$image2->alignment = htmlElement::ALIGN_RIGHT;
	$el->addElement($image2);
	$listElements[] = $el;
}
$sortableList2 = new htmlSortableList($listElements, 'sortableList2');
$sortableList2->setCSSClasses(array('module-list'));
$row->add($sortableList2);


$row->add(new htmlSpacer(null, '20rem'));


$tabindex = 1;
parseHtml(null, $row, array(), false, $tabindex, 'user');

?>
</body>
</html>
