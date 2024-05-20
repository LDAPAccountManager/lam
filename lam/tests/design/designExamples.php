<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021 - 2024  Roland Gruber

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

echo "<!DOCTYPE html>\n";
echo '<head>';
$prefix = '../..';
printHeaderContents("Design Examples", $prefix);
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"$prefix/templates/lib/extra/jodit/jodit.css\">\n";
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
$primaryButton->setCSSClasses(['lam-primary']);
$row->addField($primaryButton);

$row->addLabel(new htmlOutputText('Secondary'));
$primaryButton = new htmlButton('name2a', 'Click me');
$primaryButton->setCSSClasses(['lam-secondary']);
$row->addField($primaryButton);

$row->addLabel(new htmlOutputText('Danger'));
$primaryButton = new htmlButton('name3', 'Click me');
$primaryButton->setCSSClasses(['lam-danger']);
$row->addField($primaryButton);

$row->addLabel(new htmlOutputText('Disabled'));
$buttonDisabled = new htmlButton('name3', 'Click me');
$buttonDisabled->setIsEnabled(false);
$row->addField($buttonDisabled);

$row->add(new htmlSubTitle('Tables'));
$tableTitles = ['text 1', 'text 2', 'text 3', 'text 4', 'text 5'];
$tableData = [];
for ($rowNumber = 0; $rowNumber < 10; $rowNumber++) {
    $tableRow = [];
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
$table->setCSSClasses(['accountlist']);
$row->add($table);

$row->add(new htmlSubTitle('Data table'));
$dataTableColumns = [
    new htmlDataTableColumn('First Name', 'first'),
	new htmlDataTableColumn('Last Name', 'last'),
	new htmlDataTableColumn('User Name', 'uid'),
    new htmlDataTableColumn('Description', 'desc'),
    new htmlDataTableColumn('Group', 'group')
];
$row->add(new htmlDataTable('datatable', $dataTableColumns, 300));
$row->add(new htmlJavaScript('
    let data = [];
    for (let i = 0; i < 1000; i++) {
        const entry = {
            first: "first " + i,
            last: "last " + i,
            uid: "" + i,
            desc: "description " + i,
            group: "group" + i,
        }
        data.push(entry);
    }
    window.lam.datatable.setData("datatable", data);
'));

$row->add(new htmlSubTitle('Input fields'));

$row->addLabel(new htmlOutputText('Default'));
$row->addField(new htmlInputField('text1', 'Some text'));

$row->addLabel(new htmlOutputText('Autocomplete'));
$autocompleteInput = new htmlInputField('text1a', 'Some text');
$autocompleteInput->enableAutocompletion(['Some text', 'Some text2', 'Some text3', 'Some text4']);
$row->addField($autocompleteInput);

$row->addLabel(new htmlOutputText('Disabled'));
$textDisabled = new htmlInputField('text2', 'Some text');
$textDisabled->setIsEnabled(false);
$row->addField($textDisabled);

$row->addLabel(new htmlOutputText('Date select'));
$textDateSelect = new htmlInputField('textDateSelect', '2023-05-30');
$textDateSelect->showCalendar('Y-m-d');
$row->addField($textDateSelect);

$row->addLabel(new htmlOutputText('Time select'));
$textTimeSelect = new htmlInputField('textTimeSelect', '2023-05-30 23:45:50');
$textTimeSelect->showCalendar('Y-m-d H:i:S', true, true);
$row->addField($textTimeSelect);

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
$row->addField(new htmlRadio('radio1', ['label1' => 'value', 'label2' => 'value2'], 'value'));

$row->addLabel(new htmlOutputText('Disabled'));
$radioDisabled = new htmlRadio('radio2', ['label1' => 'value', 'label2' => 'value2'], 'value');
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

$row->addLabel(new htmlOutputText('Default select'));
$row->addField(new htmlSelect('select1', [1, 2, 3], [2]));

$row->addLabel(new htmlOutputText('Disabled select'));
$selectDisabled = new htmlSelect('select2', [1, 2, 3], [2]);
$selectDisabled->setIsEnabled(false);
$row->addField($selectDisabled);

$row->addLabel(new htmlOutputText('Default multiselect'));
$multiSelect1 = new htmlSelect('select1', ["1", "2", "3"], ["1", "3"], 5);
$multiSelect1->setMultiSelect(true);
$row->addField($multiSelect1);

$row->addLabel(new htmlOutputText('Disabled multiselect'));
$multiSelect2 = new htmlSelect('select2', ["1", "2", "3"], ["1", "3"], 5);
$multiSelect2->setIsEnabled(false);
$multiSelect2->setMultiSelect(true);
$row->addField($multiSelect2);

$row->add(new htmlSpacer(null, '5rem'));

$row->addLabel(new htmlOutputText('Default select filter'));
$defaultSelectElements = [];
for ($i = 0; $i < 1000; $i++) {
	$text = str_pad($i, 4, '0', STR_PAD_LEFT);
	$defaultSelectElements['Text ' . $text] = $text;
}
$row->addField(new htmlSelect('selectWithFilter', $defaultSelectElements, ['0002']));
$row->addLabel(new htmlOutputText('Filter'));
$selectFilter = new htmlInputField('selectFilter');
$selectFilter->filterSelectBox('selectWithFilter');
$row->addField($selectFilter);

$row->add(new htmlSpacer(null, '5rem'));

$row->addLabel(new htmlOutputText('Default multiselect filter'));
$defaultScrollElements = [];
for ($i = 0; $i < 1000; $i++) {
    $text = str_pad($i, 4, '0', STR_PAD_LEFT);
	$defaultScrollElements['Text ' . $text] = $text;
}
$scrollSelect = new htmlSelect('scrollSelect', $defaultScrollElements, ["0001", "0003", "0999"], 10);
$scrollSelect->setHasDescriptiveElements(true);
$scrollSelect->setMultiSelect(true);
$row->addField($scrollSelect);
$row->addLabel(new htmlOutputText('Filter'));
$scrollFilter = new htmlInputField('scrollSelectFilter');
$scrollFilter->filterSelectBox('scrollSelect');
$row->addField($scrollFilter);

$row->add(new htmlSpacer(null, '5rem'));

$row->addLabel(new htmlOutputText('Dynamic multiselect filter'));
$dynamicScrollElements = [];
for ($i = 0; $i < 100000; $i++) {
	$text = str_pad($i, 6, '0', STR_PAD_LEFT);
	$dynamicScrollElements['Text ' . $text] = $text;
}
$multiSelect3 = new htmlSelect('dynamicSelect', $dynamicScrollElements, ["000001", "000003", "099999"], 10);
$multiSelect3->setHasDescriptiveElements(true);
$multiSelect3->setMultiSelect(true);
$multiSelect3->enableDynamicScrolling();
$row->addField($multiSelect3);
$row->addLabel(new htmlOutputText('Filter'));
$dynamicScrollFilter = new htmlInputField('dynamicSelectFilter');
$dynamicScrollFilter->filterSelectBox('dynamicSelect');
$row->addField($dynamicScrollFilter);



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

$accordionElementsSingle = [];
$accordionElementsSingleContent1 = new htmlResponsiveRow();
$accordionElementsSingleContent1->add(new htmlResponsiveInputField('Input 1', 'acc1i1'));
$accordionElementsSingleContent1->add(new htmlResponsiveInputField('Input 2', 'acc1i2'));
$accordionElementsSingleContent1->add(new htmlResponsiveInputTextarea('acc1i3', '', 20, 3, 'Text area'));
$accordionElementsSingle['Accordion'] = $accordionElementsSingleContent1;
$row->add(new htmlAccordion('acc_single', $accordionElementsSingle));

$row->add(new htmlSpacer(null, '5rem'));

$accordionElementsSingleClosed = [];
$accordionElementsSingleContentClosed = new htmlResponsiveRow();
$accordionElementsSingleContentClosed->add(new htmlResponsiveInputField('Input 1', 'acc1ai1'));
$accordionElementsSingleContentClosed->add(new htmlResponsiveInputField('Input 2', 'acc1ai2'));
$accordionElementsSingleContentClosed->add(new htmlResponsiveInputTextarea('acc1ai3', '', 20, 3, 'Text area'));
$accordionElementsSingleClosed['Accordion - initially closed'] = $accordionElementsSingleContentClosed;
$row->add(new htmlAccordion('acc_singleClosed', $accordionElementsSingleClosed, false));

$row->add(new htmlSpacer(null, '5rem'));

$accordionElementsMulti = [];
for ($i = 0; $i < 5; $i++) {
	$accordionElementsContent = new htmlResponsiveRow();
	$accordionElementsContent->add(new htmlResponsiveInputField('Input 1', 'acc1i1' . $i));
	$accordionElementsContent->add(new htmlResponsiveInputField('Input 2', 'acc1i2' . $i));
	$accordionElementsContent->add(new htmlResponsiveInputTextarea('acc1i3' . $i, '', 20, 3, 'Text area'));
	$accordionElementsMulti['Accordion ' . $i] = $accordionElementsContent;
}
$row->add(new htmlAccordion('acc_multi', $accordionElementsMulti, 2));

$row->add(new htmlSpacer(null, '5rem'));

$accordionElementsSaveState = [];
for ($i = 0; $i < 5; $i++) {
	$accordionElementsSaveStateContent = new htmlResponsiveRow();
	$accordionElementsSaveStateContent->add(new htmlResponsiveInputField('Input 1', 'accSi1' . $i));
	$accordionElementsSaveStateContent->add(new htmlResponsiveInputField('Input 2', 'accSi2' . $i));
	$accordionElementsSaveStateContent->add(new htmlResponsiveInputTextarea('accSi3' . $i, '', 20, 3, 'Text area'));
	$accordionElementsSaveState['Accordion with saved state ' . $i] = $accordionElementsSaveStateContent;
}
$saveStateAccordion = new htmlAccordion('acc_save', $accordionElementsSaveState, 2);
$saveStateAccordion->saveState();
$row->add($saveStateAccordion);

$row->add(new htmlSpacer(null, '5rem'));

$row->add(new htmlSubTitle('Sortable list'));

$sortableList1 = new htmlSortableList([
	'text 1',
	'text 2',
	'text 3',
	'text 4',
	'text 5',
], 'sortableList1');
$sortableList1->setCSSClasses(['module-list']);
$row->add($sortableList1);

$row->add(new htmlSpacer(null, '2rem'));

$listElements = [];
for ($i = 0; $i < 8; $i++) {
	$el = new htmlTable('100%');
	$image = new htmlImage('../../graphics/tux.svg');
	$image->setCSSClasses(['size16', 'margin-right5-mobile-only']);
	$el->addElement($image);
	$el->addElement(new htmlOutputText("Text " . $i));
	$image2 = new htmlImage('../../graphics/del.svg');
	$image2->setCSSClasses(['size16', 'margin-right5-mobile-only']);
	$image2->alignment = htmlElement::ALIGN_RIGHT;
	$el->addElement($image2);
	$listElements[] = $el;
}
$sortableList2 = new htmlSortableList($listElements, 'sortableList2');
$sortableList2->setCSSClasses(['module-list']);
$row->add($sortableList2);

$row->add(new htmlSubTitle('Alert'));

$row->addLabel(new htmlOutputText('Info dialog'));
$infoDialog = new htmlButton('infoDialog', 'Click me');
$infoDialog->setOnClick('window.lam.dialog.showInfo(\'Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna\', \'Ok\');');
$row->addField($infoDialog);

$row->addLabel(new htmlOutputText('Error dialog'));
$infoDialog = new htmlButton('errorDialog', 'Click me');
$infoDialog->setOnClick('window.lam.dialog.showError(\'Some title\', \'Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna\', \'Ok\');');
$row->addField($infoDialog);

$row->add(new htmlSubTitle('WYSIWYG editor'));

$row->addLabel(new htmlOutputText('WYSIWYG'));
$textAreaWysiwyg = new htmlInputTextarea('textareaWysiwyg', 'This is a <b>test</b>', 50, 5);
$textAreaWysiwyg->setIsRichEdit(true);
$row->addField($textAreaWysiwyg);

$row->add(new htmlSubTitle('Show/hide'));

$showHideOptions = [
    'Input1' => 'input1',
	'Input2' => 'input2',
	'Both' => 'both',
];
$showHideSelect = new htmlResponsiveSelect('showHideSelect', $showHideOptions, ['input1'], 'Show/hide select');
$showHideSelect->setHasDescriptiveElements(true);
$showHideSelect->setTableRowsToShow([
    'input1' => ['showHideSelect_i1', 'invalid'],
	'input2' => ['showHideSelect_i2'],
	'both' => ['showHideSelect_i1', 'showHideSelect_i2'],
]);
$showHideSelect->setTableRowsToHide([
	'input1' => ['showHideSelect_i2', 'invalid'],
	'input2' => ['showHideSelect_i1'],
	'both' => [],
]);
$row->add($showHideSelect);
$row->add(new htmlResponsiveInputField('Input 1', 'showHideSelect_i1', ''));
$row->add(new htmlResponsiveInputField('Input 2', 'showHideSelect_i2', ''));

$row->add(new htmlSpacer(null, '3rem'));

$showHideRadio = new htmlResponsiveRadio('Show/hide radio', 'showHideRadio', $showHideOptions, 'input2');
$showHideRadio->setTableRowsToShow([
	'input1' => ['showHideRadio_i1'],
	'input2' => ['showHideRadio_i2', 'invalid'],
	'both' => ['showHideRadio_i1', 'showHideRadio_i2'],
]);
$showHideRadio->setTableRowsToHide([
	'input1' => ['showHideRadio_i2'],
	'input2' => ['showHideRadio_i1', 'invalid'],
	'both' => [],
]);
$row->add($showHideRadio);
$row->add(new htmlResponsiveInputField('Input 1', 'showHideRadio_i1', ''));
$row->add(new htmlResponsiveInputField('Input 2', 'showHideRadio_i2', ''));

$row->add(new htmlSpacer(null, '3rem'));

$showHideCheckbox = new htmlResponsiveInputCheckbox('showHideCheckbox', false, 'Show/hide checkbox');
$showHideCheckbox->setTableRowsToShow(['showHideCheckbox_i1']);
$showHideCheckbox->setTableRowsToHide(['showHideCheckbox_i2', 'invalid']);
$showHideCheckbox->setElementsToEnable(['showHideCheckbox_ie']);
$showHideCheckbox->setElementsToDisable(['showHideCheckbox_id']);
$row->add($showHideCheckbox);
$row->add(new htmlResponsiveInputField('Input 1', 'showHideCheckbox_i1', ''));
$row->add(new htmlResponsiveInputField('Input 2', 'showHideCheckbox_i2', ''));
$row->addVerticalSpacer('0.5rem');
$row->add(new htmlResponsiveInputField('Enabled on checked', 'showHideCheckbox_ie', ''));
$row->add(new htmlResponsiveInputField('Disabled on checked', 'showHideCheckbox_id', ''));

$row->add(new htmlSpacer(null, '3rem'));

$row->add(new htmlSpacer(null, '20rem'));

$form = new htmlForm('formname', 'designExamples.php', $row);

parseHtml(null, $form, [], false, 'user');

?>
<script type="text/javascript" src="../../templates/lib/extra/jodit/jodit.js"></script>
</body>
</html>
