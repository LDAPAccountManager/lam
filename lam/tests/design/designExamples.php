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
    for ($colum = 0; $colum < 5; $colum++) {
        $tableRow[] = new htmlOutputText('value' . $colum);
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
$autocompleteInput->enableAutocompletion(array('Some text', 'Some text2', 'Some text3', 'Some text4'), 2);
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
$checkRow1->addLabel(new htmlOutputText('Default'));
$checkRow1->addField(new htmlInputCheckbox('check1', true));
$row->add($checkRow1);

$checkRow2 = new htmlResponsiveRow();
$checkRow2->addLabel(new htmlOutputText('Disabled'));
$checkboxDisabled = new htmlInputCheckbox('check2', true);
$checkboxDisabled->setIsEnabled(false);
$checkRow2->addField($checkboxDisabled);
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


$tabindex = 1;
parseHtml(null, $row, array(), false, $tabindex, 'user');

?>
</body>
</html>
