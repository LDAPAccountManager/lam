<?php
namespace LAM\TOOLS\TREEVIEW;
use htmlButton;
use htmlDiv;
use htmlElement;
use htmlForm;
use htmlGroup;
use htmlHiddenInput;
use htmlImage;
use htmlInputField;
use htmlInputFileUpload;
use htmlInputTextarea;
use htmlLink;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlResponsiveSelect;
use htmlResponsiveTable;
use htmlSelect;
use htmlStatusMessage;
use htmlSubTitle;
use htmlTable;
use htmlTitle;
use LAM\SCHEMA\AttributeType;
use LAM\SCHEMA\ObjectClass;
use LAMException;
use function LAM\SCHEMA\get_schema_attributes;
use function LAM\SCHEMA\get_schema_objectclasses;


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
* Tree view functions.
*
* @author Roland Gruber
*/

include_once 'account.inc';
include_once 'tools.inc';
include_once 'tools/treeview.inc';

/**
 * Tree view functions.
 *
 * @package LAM\TOOLS\TREEVIEW
 */
class TreeView {

	/**
	 * @var array schema attributes
	 */
	private $schemaAttributes = null;

	/**
	 * @var array schema object classes
	 */
	private $schemaObjectClasses = null;

	/**
	 * Returns the JSON to answer an AJAX request.
	 *
	 * @return string JSON data
	 */
	public function answerAjaxCall(): string {
		if (empty($_GET['command'])) {
			logNewMessage(LOG_ERR, 'No command given for tree view.');
			die();
		}
		$command = $_GET['command'];
		if (empty($_POST['dn'])) {
			logNewMessage(LOG_ERR, 'No dn for tree view.');
			die;
		}
		$dn = ($_POST['dn'] === '#') ? '#' : base64_decode($_POST['dn']);
		switch ($command) {
			case 'getNodes':
				return $this->getNodes($dn);
			case 'getNodeContent':
				$this->validateDn($dn);
				return $this->getNodeContent($dn);
			case 'getInternalAttributesContent':
				$this->validateDn($dn);
				return $this->getInternalAttributesContent($dn);
			case 'getPossibleNewAttributes':
				return $this->getPossibleNewAttributeNameOptionsJson();
			case 'saveAttributes':
				$this->ensureWriteAccess();
				$this->validateDn($dn);
				return $this->saveAttributes($dn);
			case 'createNewNode':
				$this->ensureWriteAccess();
				$this->validateDn($dn);
				return $this->createNewNode($dn);
			case 'deleteNode':
				$this->ensureWriteAccess();
				$this->validateDn($dn);
				return $this->deleteNode($dn);
			case 'search':
				$this->validateDn($dn);
				return $this->search($dn);
			case 'searchResults':
				$this->validateDn($dn);
				return $this->searchResults($dn);
			case 'paste':
				$this->ensureWriteAccess();
				$this->validateDn($dn);
				return $this->paste($dn);
			default:
				logNewMessage(LOG_ERR, 'Invalid command for tree view: ' . $command);
				die;
		}
		return json_encode(array('123'));
	}

	/**
	 * Returns the JSON for the possible new attributes select.
	 */
	private function getPossibleNewAttributeNameOptionsJson() {
		$objectClasses = $_POST['objectClasses'];
		$attributeNames = $this->getPossibleNewAttributeNameOptions($objectClasses, true);
		natcasesort($attributeNames);
		return json_encode(array('data' => $attributeNames));
	}

	/**
	 * Lists LDAP nodes.
	 *
	 * @param string $dn DN
	 * @return string JSON data
	 */
	private function getNodes(string $dn): string {
		$toolSettings = $_SESSION['config']->getToolSettings();
		$rootDn = $toolSettings[TreeViewTool::TREE_SUFFIX_CONFIG];
		if ($dn === '#') {
			return $this->getRootNodes($rootDn);
		}
		return $this->getSubNodes($dn, $rootDn);
	}

	/**
	 * Returns a list of root nodes for the tree view.
	 *
	 * @return string JSON data
	 */
	private function getRootNodes(string $rootDn): string {
		logNewMessage(LOG_DEBUG, 'Getting tree nodes for ' . $rootDn);
		$rootData = ldapGetDN($rootDn, array('objectClass'));
		$children = ldapListDN($rootDn, '(objectClass=*)', array('objectClass'));
		$jsonData = $this->createNodeData($rootData, true, true, $children);
		return json_encode($jsonData);
	}

	/**
	 * Returns the subnodes of the given DN.
	 *
	 * @param string $dn DN
	 * @param string $rootDn tree root DN
	 * @return string JSON data
	 */
	private function getSubNodes(string $dn, string $rootDn): string {
		$this->validateDn($dn);
		logNewMessage(LOG_DEBUG, 'Getting tree nodes for ' . $dn);
		$children = ldapListDN($dn, '(objectClass=*)', array('objectClass'));
		$childNodes = array();
		foreach ($children as $child) {
			$childNodes[] = $this->createNodeData($child);
		}
		$this->sortNodes($childNodes);
		return json_encode($childNodes);
	}

	/**
	 * Creates the node data for the tree view.
	 *
	 * @param array $attributes LDAP data
	 * @param bool $open open node
	 * @param array $children child LDAP data
	 * @return array nodes
	 */
	private function createNodeData(array $attributes, bool $open = false, $noShortenFirst = false, array $children = array()): array {
		$text = ($noShortenFirst) ? $attributes['dn'] : extractRDN($attributes['dn']);
		$data = array(
			'id' => base64_encode($attributes['dn']),
			'text' => unescapeLdapSpecialCharacters($text),
			'icon' => $this->getNodeIcon($attributes),
			'children' => true
		);
		if (!empty($children)) {
			$childData = array();
			foreach ($children as $child) {
				$childData[] = $this->createNodeData($child);
			}
			$this->sortNodes($childData);
			$data['children'] = $childData;
		}
		if ($open) {
			$data['state'] = array(
				'opened' => true
			);
		}
		return $data;
	}

	/**
	 * Returns the node's icon.
	 *
	 * @param array $attributes LDAP data
	 * @return string icon
	 */
	private function getNodeIcon(array $attributes): string {
		$base = '../../graphics/';
		$icon = 'object.png';
		$objectClasses = array_map('strtolower', $attributes['objectclass']);
		$rdn = extractRDNValue($attributes['dn']);
		if (in_array('sambaaccount', $objectClasses) &&
			'$' == $rdn[ strlen($rdn) - 1 ]) {
			$icon = 'nt_machine.png';
		}
		if (in_array('sambaaccount', $objectClasses)) {
			$icon = 'nt_user.png';
		}
		elseif (in_array('person', $objectClasses) ||
			in_array('organizationalperson', $objectClasses) ||
			in_array('inetorgperson', $objectClasses) ||
			in_array('account', $objectClasses) ||
			in_array('posixaccount', $objectClasses) ||
			in_array('organizationalrole', $objectClasses)) {
			$icon = 'uidSmall.png';
		}
		elseif (in_array('organization', $objectClasses)) {
			$icon = 'world-color.svg';
		}
		elseif (in_array('organizationalunit', $objectClasses)) {
			$icon = 'folder.svg';
		}
		elseif (in_array('dcobject', $objectClasses) ||
			in_array('domainrelatedobject', $objectClasses) ||
			in_array('domain', $objectClasses) ||
			in_array('builtindomain', $objectClasses)) {
			$icon = 'world-color.svg';
		}
		elseif (in_array('alias', $objectClasses)) {
			$icon = 'alias.png';
		}
		elseif (in_array('document', $objectClasses)) {
			$icon = 'txt.png';
		}
		elseif (in_array('country', $objectClasses)) {
			$icon = 'world-color.svg';
		}
		elseif (in_array('locality', $objectClasses)) {
			$icon = 'location.svg';
		}
		elseif (in_array('posixgroup', $objectClasses) ||
			in_array('groupofnames', $objectClasses) ||
			in_array('groupofuniquenames', $objectClasses) ||
			in_array('group', $objectClasses)) {
			$icon = 'group.png';
		}
		elseif (in_array('iphost', $objectClasses)) {
			$icon = 'host.png';
		}
		elseif (in_array('device', $objectClasses)) {
			$icon = 'device.png';
		}
		elseif (in_array('server', $objectClasses)) {
			$icon = 'host.png';
		}
		elseif (in_array('volume', $objectClasses)) {
			$icon = 'hard-drive.png';
		}
		elseif (in_array('container', $objectClasses)) {
			$icon = 'folder.svg';
		}
		return $base . $icon;
	}

	/**
	 * Sorts the given node array by DN.
	 *
	 * @param array $nodes nodes
	 */
	private function sortNodes(array &$nodes): void {
		usort($nodes, 'LAM\TOOLS\TREEVIEW\compareNodeByIdAsDn');
	}

	/**
	 * Returns the options for the drop-down to add a new attribute.
	 *
	 * @param array $objectClasses object classes
	 * @param bool $includeMustAttributes include required attributes
	 * @return array list of options
	 */
	private function getPossibleNewAttributeNameOptions(array $objectClasses, bool $includeMustAttributes = false): array {
		$schemaObjectClasses = $this->getSchemaObjectClasses();
		$schemaAttributes = $this->getSchemaAttributes();
		$possibleNewAttributes = array();
		foreach ($objectClasses as $objectClass) {
			$objectClass = strtolower($objectClass);
			if (empty($schemaObjectClasses[$objectClass])) {
				continue;
			}
			$attributes = $schemaObjectClasses[$objectClass]->getMayAttrs();
			if ($includeMustAttributes) {
				$attributes = array_merge($attributes, $schemaObjectClasses[$objectClass]->getMustAttrs());
			}
			foreach ($attributes as $attribute) {
				$attributeName = $attribute->getName();
				if (!isset($attributes[strtolower($attributeName)])) {
					$schemaAttribute = $schemaAttributes[strtolower($attributeName)];
					$single = $schemaAttribute->getIsSingleValue() ? 'single' : 'multi';
					$type = $this->isMultiLineAttribute($attributeName, $schemaAttribute) ? 'textarea' : 'input';
					if ($this->isPasswordAttribute($attributeName)) {
						$type = 'password';
					}
					elseif ($this->isJpegAttribute($attributeName, $schemaAttribute)) {
						$type = 'jpeg';
					}
					$possibleNewAttributes[$attributeName] = $attributeName . '__#__' . $single . '__#__' . $type;
				}
			}
		}
		ksort($possibleNewAttributes);
		return $possibleNewAttributes;
	}

	/**
	 * Returns the node content with the attribute listing.
	 *
	 * @param htmlStatusMessage|null $message message to display
	 * @return string JSON
	 */
	private function getNodeContent(string $dn, ?htmlStatusMessage $message = null): string {
		$row = new htmlResponsiveRow();
		if ($message !== null) {
			$row->add($message);
			$row->addVerticalSpacer('1rem');
		}
		$row->add(new htmlTitle(unescapeLdapSpecialCharacters($dn)), 12);
		$row->add(new htmlDiv('ldap_actionarea_messages', new htmlOutputText('')), 12);
		$row->add(new htmlSubTitle(_('Attributes')), 12);
		$attributes = ldapGetDN($dn, array('*'));
		unset($attributes['dn']);
		ksort($attributes);
		$schemaAttributes = $this->getSchemaAttributes();
		$objectClasses = $attributes['objectclass'];
		$highlighted = (empty($_POST['highlight'])) ? array() : $_POST['highlight'];
		foreach ($attributes as $attributeName => $values) {
			$schemaAttribute = null;
			if (isset($schemaAttributes[$attributeName])) {
				$schemaAttribute = $schemaAttributes[$attributeName];
				$attributeName = $schemaAttribute->getName();
			}
			$this->addAttributeContent($row, $attributeName, $values, $schemaAttribute, $objectClasses, $dn, $highlighted);
		}
		$row->addVerticalSpacer('1rem');

		// add new attributes
		$possibleNewAttributes = $this->getPossibleNewAttributeNameOptions($objectClasses);
		logNewMessage(LOG_DEBUG, 'Possible new attributes for ' . $dn . ': ' . implode('; ', $possibleNewAttributes));
		if (!empty($possibleNewAttributes)) {
			$possibleNewAttributes = array('' => '') + $possibleNewAttributes;
			$row->add(new htmlSubTitle(_('Add new attribute')), 12);
			$newAttributeSelect = new htmlResponsiveSelect('newAttribute', $possibleNewAttributes, array(), _('Attribute'));
			$newAttributeSelect->setHasDescriptiveElements(true);
			$newAttributeSelect->setTransformSingleSelect(false);
			$newAttributeSelect->setOnchangeEvent('window.lam.treeview.addAttributeField(event, this);');
			$row->add($newAttributeSelect, 12);
			$newAttributesContentSingleInput = new htmlResponsiveRow();
			$newAttributesContentSingleInput->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_INPUT_LABEL'));
			$newAttributesContentSingleInput->addField($this->getAttributeContentField('PLACEHOLDER_SINGLE_INPUT', array(''), false, false, false, null));
			$row->add(new htmlDiv('new-attributes-single-input', $newAttributesContentSingleInput, array('hidden')), 12);
			$newAttributesContentMultiInput = new htmlResponsiveRow();
			$newAttributesContentMultiInput->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_INPUT_LABEL'));
			$newAttributesContentMultiInput->addField($this->getAttributeContentField('PLACEHOLDER_MULTI_INPUT', array(''), false, true, false, null));
			$row->add(new htmlDiv('new-attributes-multi-input', $newAttributesContentMultiInput, array('hidden')), 12);
			$newAttributesContentSingleTextarea = new htmlResponsiveRow();
			$newAttributesContentSingleTextarea->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_TEXTAREA_LABEL'));
			$newAttributesContentSingleTextarea->addField($this->getAttributeContentField('PLACEHOLDER_SINGLE_TEXTAREA', array(''), false, false, true, null));
			$row->add(new htmlDiv('new-attributes-single-textarea', $newAttributesContentSingleTextarea, array('hidden')), 12);
			$newAttributesContentMultiTextarea = new htmlResponsiveRow();
			$newAttributesContentMultiTextarea->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_TEXTAREA_LABEL'));
			$newAttributesContentMultiTextarea->addField($this->getAttributeContentField('PLACEHOLDER_MULTI_TEXTAREA', array(''), false, true, true, null));
			$row->add(new htmlDiv('new-attributes-multi-textarea', $newAttributesContentMultiTextarea, array('hidden')), 12);
			$newAttributesContentSinglePassword = new htmlResponsiveRow();
			$newAttributesContentSinglePassword->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_PASSWORD_LABEL'));
			$newAttributesContentSinglePassword->addField($this->getAttributeContentField('userPassword', array(''), false, false, false, null));
			$row->add(new htmlDiv('new-attributes-single-password', $newAttributesContentSinglePassword, array('hidden')), 12);
			$newAttributesContentMultiPassword = new htmlResponsiveRow();
			$newAttributesContentMultiPassword->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_PASSWORD_LABEL'));
			$newAttributesContentMultiPassword->addField($this->getAttributeContentField('userPassword', array(''), false, true, true, null));
			$row->add(new htmlDiv('new-attributes-multi-password', $newAttributesContentMultiPassword, array('hidden')), 12);
			$newAttributesContentSingleJpeg = new htmlResponsiveRow();
			$newAttributesContentSingleJpeg->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_JPEG_LABEL'));
			$newAttributesContentSingleJpeg->addField($this->getAttributeContentField('jpegPhoto', array(''), false, false, false, null));
			$row->add(new htmlDiv('new-attributes-single-jpeg', $newAttributesContentSingleJpeg, array('hidden')), 12);
			$newAttributesContentMultiJpeg = new htmlResponsiveRow();
			$newAttributesContentMultiJpeg->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_JPEG_LABEL'));
			$newAttributesContentMultiJpeg->addField($this->getAttributeContentField('jpegPhoto', array(''), false, true, true, null));
			$row->add(new htmlDiv('new-attributes-multi-jpeg', $newAttributesContentMultiJpeg, array('hidden')), 12);
		}

		if (checkIfWriteAccessIsAllowed()) {
			$row->addVerticalSpacer('2rem');
			$saveButton = new htmlLink(_('Save'), '#', '../../graphics/save.svg', true);
			$saveButton->setId('savebutton');
			$saveButton->setOnClick('window.lam.treeview.saveAttributes(event,
			"' . getSecurityTokenName() . '",
			"' . getSecurityTokenValue() . '",
			"' . base64_encode($dn) . '");');
			$saveButton->setCSSClasses(array('icon'));
			$row->add($saveButton, 12, 12, 12, 'text-center');
		}

		$internalAttributesContent = new htmlResponsiveRow();
		$internalAttributesContent->add(new htmlSubTitle(_('Internal attributes')), 12);
		$internalAttributesButton = new htmlLink(_('Show internal attributes'), '#', null, true);
		$internalAttributesButton->setOnClick('window.lam.treeview.getInternalAttributesContent(event,
			"' . getSecurityTokenName() . '",
			"' . getSecurityTokenValue() . '",
			"' . base64_encode($dn) . '");');
		$internalAttributesButton->setId('internalAttributesButton');
		$internalAttributesContent->add($internalAttributesButton, 12);
		$internalAttributesDiv = new htmlDiv('actionarea-internal-attributes', $internalAttributesContent);
		$row->add($internalAttributesDiv, 12);
		$tabindex = 1;
		ob_start();
		parseHtml(null, $row, array(), false, $tabindex, 'none');
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(array('content' => $content));
	}

	/**
	 * Adds the content part for one attribute.
	 *
	 * @param htmlResponsiveRow $row container where to add content
	 * @param string $attributeName attribute name
	 * @param array $values values
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @param string[] $objectClasses object classes
	 * @param string|null $dn DN
	 * @param array $highlighted list of highlighted attribute names
	 */
	private function addAttributeContent(htmlResponsiveRow $row, string $attributeName, array $values,
	                                    ?AttributeType $schemaAttribute, array $objectClasses, ?string $dn,
										array $highlighted): void {
		$schemaObjectClasses = $this->getSchemaObjectClasses();
		$label = new htmlOutputText($attributeName);
		$rdnAttribute = ($dn !== null) ? strtolower(extractRDNAttribute($dn)) : '';
		$attributeNameLowerCase = strtolower($attributeName);
		$required = ($attributeNameLowerCase === $rdnAttribute) || ($attributeNameLowerCase === 'objectclass');
		if (($schemaAttribute !== null) && $this->isAttributeRequired($schemaObjectClasses, $schemaAttribute, $objectClasses)) {
			$required = true;
		}
		$label->setMarkAsRequired($required);
		$isHighlighted = in_array_ignore_case($attributeName, $highlighted);
		$cssClasses = ($isHighlighted) ? 'tree-highlight' : '';
		$row->addLabel($label, $cssClasses);
		if (($schemaAttribute !== null) && !empty($schemaAttribute->getDescription())) {
			$label->setTitle($schemaAttribute->getDescription());
		}
		$isMultiValue = $this->isMultiValueAttribute($values, $schemaAttribute);
		$isMultiLine = $this->isMultiLineAttribute($attributeName, $schemaAttribute);
		$row->addField($this->getAttributeContentField($attributeName, $values, $required, $isMultiValue, $isMultiLine, $schemaAttribute), $cssClasses);
		$row->addVerticalSpacer('0.5rem');
	}

	/**
	 * Returns if the given attribute is a multi-value one.
	 *
	 * @param array|null $values attribute values
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @return bool is multi-value
	 */
	private function isMultiValueAttribute(?array $values, ?AttributeType $schemaAttribute): bool {
		return (sizeof($values) > 1) || (($schemaAttribute !== null) && ($schemaAttribute->getIsSingleValue() !== true));
	}

	/**
	 * Returns the input fields for the attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param array $values values
	 * @param bool $required is required
	 * @param bool $isMultiValue multi-value attribute
	 * @param bool $isMultiLine textarea attribute
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @return htmlElement content
	 */
	private function getAttributeContentField(string $attributeName, array $values, bool $required, bool $isMultiValue,
	                                          bool $isMultiLine, ?AttributeType $schemaAttribute): htmlElement {
		if (!$isMultiValue) {
			$field = $this->getAttributeInputField($attributeName, $values[0], $required, $isMultiLine, true, $schemaAttribute, 0);
			return $this->addExtraAttributeContent($field, $attributeName, $schemaAttribute);
		}
		$valueTable = new htmlTable();
		$valueTable->setCSSClasses(array('fullwidth'));
		$autoCompleteValues = array();
		$onInput = null;
		if (strtolower($attributeName) === 'objectclass') {
			$schemaObjectClasses = $this->getSchemaObjectClasses();
			foreach ($schemaObjectClasses as $schemaObjectClass) {
				if (!in_array_ignore_case($schemaObjectClass->getName(), $values)) {
					$autoCompleteValues[] = $schemaObjectClass->getName();
				}
			}
			$onInput = "window.lam.treeview.updatePossibleNewAttributes('" . getSecurityTokenName() . "', '" . getSecurityTokenValue() . "');";
		}
		foreach ($values as $index => $value) {
			$inputField = $this->getAttributeInputField($attributeName, $value, $required, $isMultiLine, false, $schemaAttribute, $index);
			$cssClasses = $inputField->getCSSClasses();
			$cssClasses[] = 'lam-attr-' . htmlspecialchars(strtolower($attributeName));
			$inputField->setCSSClasses($cssClasses);
			if (!empty($autoCompleteValues) && ($inputField instanceof htmlInputField)) {
				$inputField->enableAutocompletion($autoCompleteValues);
				$inputField->setId($attributeName . $index);
			}
			if ($onInput !== null) {
				$inputField->setOnInput($onInput);
			}
			$valueTable->addElement($inputField);
			if (checkIfWriteAccessIsAllowed()) {
				if (!$this->isJpegAttribute($attributeName, $schemaAttribute)) {
					$addButton = new htmlLink(null, '#', '../../graphics/add.svg');
					$addButton->setCSSClasses(array('margin2'));
					$addButton->setOnClick('window.lam.treeview.addValue(event, this);');
					$valueTable->addElement($addButton);
				}
				if (!$this->isJpegAttribute($attributeName, $schemaAttribute) || ($value !== '')) {
					$clearButton = new htmlLink(null, '#', '../../graphics/del.svg');
					$clearButton->setCSSClasses(array('margin2'));
					$clearButton->setOnClick('window.lam.treeview.clearValue(event, this);');
					$valueTable->addElement($clearButton);
				}
			}
			$valueTable->addNewLine();
		}
		return $this->addExtraAttributeContent($valueTable, $attributeName, $schemaAttribute);
	}

	/**
	 * Adds any entry content to the element.
	 *
	 * @param htmlElement $element original element
	 * @param string $attributeName attribute name
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @return htmlElement enhanced element
	 */
	private function addExtraAttributeContent(htmlElement $element, string $attributeName, ?AttributeType $schemaAttribute): htmlElement {
		if ($this->isJpegAttribute($attributeName, $schemaAttribute)) {
			$row = new htmlResponsiveRow();
			$row->add($element);
			$upload = new htmlInputFileUpload('lam_attr_' . $attributeName);
			$upload->addDataAttribute('attr-name', $attributeName);
			$upload->setCSSClasses(array('image-upload'));

			$row->add($upload);
			return $row;
		}
		return $element;
	}

	/**
	 * Returns an input field for an LDAP attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param string $value value
	 * @param bool $required required
	 * @param bool $isMultiLine is multi-line attribute
	 * @param bool $isSingleValue is single value attribute
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @param int $index value position
	 * @return htmlElement input field
	 */
	private function getAttributeInputField(string $attributeName, string $value, bool $required, bool $isMultiLine,
	                                        bool $isSingleValue, ?AttributeType $schemaAttribute, int $index): htmlElement {
		if ($this->isPasswordAttribute($attributeName)) {
			return $this->getAttributePasswordInputField($attributeName, $value, $required, $isSingleValue);
		}
		if ($this->isJpegAttribute($attributeName, $schemaAttribute)) {
			return $this->getAttributeJpegInputField($attributeName, $value, $required, $index);
		}
		if ($isMultiLine) {
			$inputField = new htmlInputTextarea('lam_attr_' . $attributeName, $value, 50, 5);
		}
		else {
			$inputField = new htmlInputField('lam_attr_' . $attributeName, $value);
		}
		$inputField->addDataAttribute('value-orig', $value);
		$inputField->addDataAttribute('attr-name', $attributeName);
		$cssClass = ($isSingleValue) ? 'single-input' : 'multi-input';
		$inputField->setCSSClasses(array($cssClass));
		if ($required) {
			$inputField->setRequired(true);
		}
		return $inputField;
	}

	/**
	 * Returns if the given attribute is a (hashable) password.
	 *
	 * @param string $attributeName attribute name
	 * @return bool is password
	 */
	private function isPasswordAttribute(string $attributeName): bool {
		if (strtolower($attributeName) === 'userpassword') {
			return true;
		}
		return false;
	}

	/**
	 * Returns if the given attribute is a JPG image.
	 *
	 * @param string $attributeName attribute name
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @return bool is password
	 */
	private function isJpegAttribute(string $attributeName, ?AttributeType $schemaAttribute): bool {
		if (strtolower($attributeName) === 'jpegphoto') {
			return true;
		}
		if (($schemaAttribute !== null) && ($schemaAttribute->getType() === 'JPEG')) {
			return true;
		}
		return false;
	}

	/**
	 * Returns an input field for a password attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param string $value value
	 * @param bool $required required
	 * @param bool $isSingleValue is single value attribute
	 * @return htmlElement input field
	 */
	private function getAttributePasswordInputField(string $attributeName, string $value, bool $required, bool $isSingleValue): htmlElement {
		$inputField = new htmlInputField('lam_attr_' . $attributeName, $value);
		$inputField->addDataAttribute('value-orig', $value);
		$inputField->addDataAttribute('attr-name', $attributeName);
		$cssClass = ($isSingleValue) ? 'single-input' : 'multi-input';
		$inputField->setCSSClasses(array($cssClass));
		if ($required) {
			$inputField->setRequired(true);
		}
		$inputField->setIsPassword(true);
		$row = new htmlResponsiveRow();
		$row->add($inputField, 7);
		$hashes = getSupportedHashTypes();
		$selectedHash = array(getHashType($value));
		$hashSelect = new htmlSelect('lam_hash_' . $attributeName, $hashes, $selectedHash);
		$hashSelect->addDataAttribute('attr-name', $attributeName);
		$hashSelect->setCSSClasses(array('hash-select'));
		$row->add($hashSelect, 5);
		return $row;
	}

	/**
	 * Returns an input field for a JPG image attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param string $value value
	 * @param bool $required required
	 * @return htmlElement input field
	 */
	private function getAttributeJpegInputField(string $attributeName, string $value, bool $required, int $index): htmlElement {
		$imgNumber = getRandomNumber();
		$jpeg_filename = 'jpg' . $imgNumber . '.jpg';
		$outJpeg = @fopen(__DIR__ . '/../tmp/' . $jpeg_filename, "wb");
		fwrite($outJpeg, $value);
		fclose ($outJpeg);
		$photoFile = '../../tmp/' . $jpeg_filename;
		$image = new htmlImage($photoFile);
		$image->enableLightbox();
		$image->setCSSClasses(array('thumbnail', 'image-input'));
		$image->addDataAttribute('attr-name', $attributeName);
		$image->addDataAttribute('index', $index);
		return $image;
	}

	/**
	 * Returns if the given attribute is multi-line.
	 *
	 * @param string $attributeName attribute name
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @return bool is multi-line
	 */
	private function isMultiLineAttribute(string $attributeName, ?AttributeType $schemaAttribute): bool {
		$knownAttributes = array('postalAddress1', 'homePostalAddress', 'personalSignature', 'description', 'mailReplyText');
		if (in_array_ignore_case($attributeName, $knownAttributes)) {
			return true;
		}
		if ($schemaAttribute === null) {
			return false;
		}
		$knownSyntaxOIDs = array(
			// octet string syntax OID:
			'1.3.6.1.4.1.1466.115.121.1.40',
			// postal address syntax OID:
			'1.3.6.1.4.1.1466.115.121.1.41');
		return in_array($schemaAttribute->getSyntaxOID(), $knownSyntaxOIDs);
	}

	/**
	 * Returns if the attribute is required for the given list of object classes.
	 *
	 * @param ObjectClass[] $schemaObjectClasses object class definitions
	 * @param AttributeType $schemaAttribute schema attribute
	 * @param array $objectClasses list of object classes
	 * @return bool is required
	 */
	private function isAttributeRequired(array $schemaObjectClasses, AttributeType $schemaAttribute, array $objectClasses): bool {
		foreach ($objectClasses as $objectClass) {
			$objectClass = strtolower($objectClass);
			if (!isset($schemaObjectClasses[$objectClass])) {
				continue;
			}
			$schemaObjectClass = $schemaObjectClasses[$objectClass];
			$requiredAttributes = $schemaObjectClass->getMustAttrNames();
			if (in_array_ignore_case($schemaAttribute->getName(), $requiredAttributes)) {
				return true;
			}
			if (!empty($schemaObjectClass->getSupClasses())) {
				if ($this->isAttributeRequired($schemaObjectClasses, $schemaAttribute, $schemaObjectClass->getSupClasses())) {
					return true;
				}
			}
		}
		return false;
	}

	/**
	 * Returns the internal attributes.
	 *
	 * @param string $dn DN
	 * @return string JSON
	 */
	private function getInternalAttributesContent(string $dn): string {
		$row = new htmlResponsiveRow();
		$row->add(new htmlSubTitle(_('Internal attributes')), 12);
		$attributes = ldapGetDN($dn, array('+', 'creatorsName', 'createTimestamp', 'modifiersName',
			'modifyTimestamp', 'hasSubordinates', 'pwdChangedTime'));
		unset($attributes['dn']);
		ksort($attributes);
		foreach ($attributes as $attributeName => $values) {
			$row->addLabel(new htmlOutputText($this->getProperAttributeName($attributeName)));
			$row->addField(new htmlOutputText(implode(', ', $values)));
			$row->addVerticalSpacer('0.5rem');
		}
		$tabindex = 1;
		ob_start();
		parseHtml(null, $row, array(), false, $tabindex, 'none');
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(array('content' => $content));
	}

	/**
	 * Returns the node content with the attribute listing.
	 *
	 * @return string JSON
	 */
	private function saveAttributes(string $dn): string {
		$schemaAttributes = $this->getSchemaAttributes();
		$attributes = ldapGetDN($dn, array('*'));
		unset($attributes['dn']);
		$attributes = array_change_key_case($attributes,CASE_LOWER);
		$changes = json_decode($_POST['changes'], true);
		$changes = array_change_key_case($changes, CASE_LOWER);
		logNewMessage(LOG_DEBUG, 'LDAP changes for ' . $dn . ': ' . print_r($changes, true));
		$ldapChanges = array();
		if (!empty($changes)) {
			foreach ($changes as $attrName => $change) {
				if (isset($change['new'])) {
					$newValues = $change['new'];
					if (isset($change['hash'])) {
						$newValues = $this->applyPasswordHash($newValues, $change['hash']);
					}
					$ldapChanges[$attrName] = $newValues;
				}
				if (isset($change['delete']) && isset($attributes[$attrName])) {
					$oldValues = $attributes[$attrName];
					$changed = false;
					foreach ($change['delete'] as $index) {
						if (isset($oldValues[$index])) {
							unset($oldValues[$index]);
							$changed = true;
						}
					}
					if ($changed) {
						$ldapChanges[$attrName] = array_values($oldValues);
					}
				}
				if (!empty($change['upload'])) {
					if (empty($ldapChanges[$attrName]) && !empty($attributes[$attrName])) {
						$ldapChanges[$attrName] = $attributes[$attrName];
					}
					$oldValues = !empty($attributes[$attrName]) ? $attributes[$attrName] : array();
					$schemaAttribute = isset($schemaAttributes[$attrName]) ? $schemaAttributes[$attrName] : null;
					if ($this->isMultiValueAttribute($oldValues, $schemaAttribute)) {
						$ldapChanges[$attrName][] = base64_decode($change['upload']);
					}
					else {
						$ldapChanges[$attrName][0] = base64_decode($change['upload']);
					}
				}
			}
		}
		$message = new htmlStatusMessage('INFO', _('You made no changes'));
		$jsonData = array();
		// rename DN if RDN attribute changed
		$rdnAttribute = strtolower(extractRDNAttribute($dn));
		$rdnValue = extractRDNValue($dn);
		if (isset($ldapChanges[$rdnAttribute]) && isset($ldapChanges[$rdnAttribute][0]) && !in_array($rdnValue, $ldapChanges[$rdnAttribute])) {
			$pos = 0;
			$oldPos = array_search($rdnValue, $changes[$rdnAttribute]['old']);
			if (($oldPos !== false) && isset($ldapChanges[$rdnAttribute][$oldPos])) {
				$pos = $oldPos;
			}
			$newRdnValue = $ldapChanges[$rdnAttribute][$pos];
			$newRdn = $rdnAttribute . '=' . ldap_escape($newRdnValue, '', LDAP_ESCAPE_DN);
			$parent = extractDNSuffix($dn);
			$renameOk = ldap_rename($_SESSION['ldap']->server(), $dn, $newRdn, $parent, true);
			$newDn = $newRdn . ',' . $parent;
			if ($renameOk) {
				logNewMessage(LOG_DEBUG, 'Renamed ' . $dn . ' to ' . $newDn);
				$dn = $newDn;
				$jsonData['newDn'] = base64_encode($newDn);
				$jsonData['parent'] = base64_encode($parent);
			}
			else {
				logNewMessage(LOG_ERR, 'Renaming ' . $dn . ' to ' . $newDn . ' failed: ' . getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
				$message = new htmlStatusMessage('ERROR', sprintf(_('Was unable to rename DN: %s.'), $dn), getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
				// skip other changes
				$ldapChanges = array();
			}
		}
		if (!empty($ldapChanges)) {
			$saved = ldap_modify($_SESSION['ldap']->server(), $dn, $ldapChanges);
			if ($saved) {
				$message = new htmlStatusMessage('INFO', _('All changes were successful.'));
			}
			else {
				$message = new htmlStatusMessage('ERROR', sprintf(_('Was unable to modify attributes of DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
		}
		$tabIndex = 1;
		ob_start();
		parseHtml(null, $message, array(), true, $tabIndex, 'none');
		$messageContent = ob_get_contents();
		ob_end_clean();
		$jsonData['result'] = $messageContent;
		return json_encode($jsonData);
	}

	/**
	 * Applies password hashing on the provided values.
	 *
	 * @param array $values values
	 * @param array $hash hash types
	 * @return array hashed values
	 */
	private function applyPasswordHash(array $values, array $hash): array {
		$result = array();
		for ($i = 0; $i < sizeof($values); $i++) {
			$oldType = getHashType($values[$i]);
			if (($oldType === 'PLAIN') && ($hash[$i] !== 'PLAIN')) {
				$result[] = pwd_hash($values[$i], true, $hash[$i]);
			}
			else {
				$result[] = $values[$i];
			}
		}
		return $result;
	}

	/**
	 * Displays the content to create a new subnode.
	 *
	 * @param string $dn DN
	 * @return string JSON data
	 */
	private function createNewNode(string $dn): string {
		$step = $_GET['step'];
		switch ($step) {
			case 'getObjectClasses':
				return $this->createNewNodeGetObjectClassesStep($dn);
			case 'checkObjectClasses':
				return $this->createNewNodeCheckObjectClassesStep($dn);
			case 'checkAttributes':
				return $this->createNewNodeCheckAttributesStep($dn);
		}
		logNewMessage(LOG_ERR, 'Invalid create new node step: ' . $step);
	}

	/**
	 * Returns the content to select the object classes.
	 *
	 * @param string $dn DN
	 * @param string|null $errorMessage error if any
	 * @return string JSON data
	 */
	private function createNewNodeGetObjectClassesStep(string $dn, string $errorMessage = null): string {
		$row = new htmlResponsiveRow();
		$row->add(new htmlTitle(_('Create a child entry')));
		if ($errorMessage !== null) {
			$row->add(new htmlStatusMessage('ERROR', $errorMessage));
		}
		$row->addLabel(new htmlOutputText(_('Parent')));
		$row->addField(new htmlOutputText($dn));
		$row->addVerticalSpacer('1rem');
		$schemaObjectClasses = $this->getSchemaObjectClasses();
		$objectClassOptions = array();
		$selectCssClasses = array();
		foreach ($schemaObjectClasses as $schemaObjectClass) {
			$name = $schemaObjectClass->getName();
			$objectClassOptions[$name] = $name;
			if ($schemaObjectClass->getType() === 'structural') {
				$selectCssClasses[$name] = 'bold';
			}
		}
		$objectClassSelect = new htmlResponsiveSelect('objectClasses', $objectClassOptions, array(), _('Object classes'), null, 10);
		$objectClassSelect->setHasDescriptiveElements(true);
		$objectClassSelect->setMultiSelect(true);
		$objectClassSelect->setOptionCssClasses($selectCssClasses);
		$row->add($objectClassSelect, 12);
		$row->addVerticalSpacer('0.5rem');
		$filterGroup = new htmlGroup();
		$filterGroup->addElement(new htmlOutputText(_('Filter') . ' '));
		$filterInput = new htmlInputField('filter', '');
		$filterInput->filterSelectBox('objectClasses');
		$filterGroup->addElement($filterInput);
		$row->addLabel(new htmlOutputText('&nbsp;', false));
		$row->addField($filterGroup);
		$row->addVerticalSpacer('1rem');
		$nextButton = new htmlButton('next', _('Next'));
		$nextButton->setOnClick('window.lam.treeview.createNodeSelectObjectClassesStep(event, \'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');');
		$row->add($nextButton, 12, 12, 12, 'text-center');
		$row->addVerticalSpacer('2rem');
		$row->add(new htmlOutputText(_('Hint: You must choose exactly one structural object class (shown in bold above)')), 12);
		$row->add(new htmlHiddenInput('parentDn', base64_encode($dn)), 12);
		ob_start();
		$tabIndex = 1;
		parseHtml(null, $row, array(), false, $tabIndex, '');
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(array('content' => $content));
	}

	/**
	 * Returns the content to select the object classes.
	 *
	 * @param string $dn DN
	 * @param string|null $errorMessage error if any
	 * @param string|null $rdnAttribute RDN attribute name
	 * @param array|null $attributes attribute values
	 * @return string JSON data
	 */
	private function createNewNodeCheckObjectClassesStep(string $dn, ?string $errorMessage = null, ?string $rdnAttribute = null, ?array $attributes = null): string {
		if ($attributes === null) {
			$objectClasses = $_POST['objectClasses'];
		}
		else {
			$objectClasses = $attributes['objectClass'];
		}
		$structuralObjectClassesCount = 0;
		$schemaObjectClasses = $this->getSchemaObjectClasses();
		foreach ($objectClasses as $objectClass) {
			$objectClassLower = strtolower($objectClass);
			if (!isset($schemaObjectClasses[$objectClassLower])) {
				logNewMessage(LOG_ERR, 'Tree view new node, invalid object class: ' . $objectClass);
				return $this->createNewNodeGetObjectClassesStep($dn, _('Invalid object class.'));
			}
			if ($schemaObjectClasses[$objectClassLower]->getType() === 'structural') {
				$structuralObjectClassesCount++;
			}
		}
		if ($structuralObjectClassesCount === 0) {
			return $this->createNewNodeGetObjectClassesStep($dn, _('No structural object class selected.'));
		}
		elseif ($structuralObjectClassesCount > 1) {
			return $this->createNewNodeGetObjectClassesStep($dn, _('Multiple structural object classes selected.'));
		}
		$row = new htmlResponsiveRow();
		$row->add(new htmlTitle(_('Create a child entry')));
		if ($errorMessage !== null) {
			$row->add(new htmlStatusMessage('ERROR', $errorMessage));
			$row->addVerticalSpacer('0.5rem');
		}
		$row->addLabel(new htmlOutputText(_('Parent')));
		$row->addField(new htmlOutputText($dn));
		$row->addVerticalSpacer('1rem');
		$row->addLabel(new htmlOutputText(_('Object classes')));
		$row->addField(new htmlOutputText(implode(', ', $objectClasses)));
		$row->addVerticalSpacer('1rem');
		$schemaAttributes = $this->getSchemaAttributes();
		$mustAttributes = array();
		$mayAttributes = array();
		foreach ($objectClasses as $objectClass) {
			$classMustAttributeNames = $this->getMustAttributeNamesRecursive($schemaObjectClasses, $objectClass);
			foreach ($classMustAttributeNames as $classMustAttributeName) {
				$attrNameLower = strtolower($classMustAttributeName);
				if (!isset($schemaAttributes[$attrNameLower])) {
					logNewMessage(LOG_ERR, 'Tree view new node, invalid attribute: ' . $classMustAttributeName);
					return $this->createNewNodeGetObjectClassesStep($dn, _('Invalid object class.'));
				}
				$mustAttributes[$attrNameLower] = $schemaAttributes[$attrNameLower];
			}
		}
		foreach ($objectClasses as $objectClass) {
			$classMayAttributeNames = $this->getMayAttributeNamesRecursive($schemaObjectClasses, $objectClass);
			foreach ($classMayAttributeNames as $classMayAttributeName) {
				$attrNameLower = strtolower($classMayAttributeName);
				if (!isset($schemaAttributes[$attrNameLower])) {
					logNewMessage(LOG_ERR, 'Tree view new node, invalid attribute: ' . $classMayAttributeName);
					return $this->createNewNodeGetObjectClassesStep($dn, _('Invalid object class.'));
				}
				if (array_key_exists($attrNameLower, $mustAttributes)) {
					continue;
				}
				$mayAttributes[$attrNameLower] = $schemaAttributes[$attrNameLower];
			}
		}
		if (isset($mustAttributes['objectclass'])) {
			unset($mustAttributes['objectclass']);
		}
		ksort($mustAttributes);
		ksort($mayAttributes);
		$allAttributes = array_merge($mustAttributes, $mayAttributes);
		ksort($allAttributes);
		$rdnOptions = array();
		foreach ($allAttributes as $attribute) {
			$rdnOptions[] = $attribute->getName();
		}
		$rdnOptionsSelected = array();
		if ($rdnAttribute !== null) {
			$rdnOptionsSelected[] = $rdnAttribute;
		}
		elseif (isset($allAttributes['cn'])) {
			$rdnOptionsSelected[] = $allAttributes['cn']->getName();
		}
		elseif (isset($allAttributes['ou'])) {
			$rdnOptionsSelected[] = $allAttributes['ou']->getName();
		}
		$row->add(new htmlResponsiveSelect('rdn', $rdnOptions, $rdnOptionsSelected, _('RDN identifier')));
		if (!empty($mustAttributes)) {
			$row->add(new htmlSubTitle(_('Required attributes')));
		}
		foreach ($mustAttributes as $mustAttribute) {
			$values = empty($attributes[$mustAttribute->getName()]) ? array('') : $attributes[$mustAttribute->getName()];
			$this->addAttributeContent($row, $mustAttribute->getName(), $values, $mustAttribute, $objectClasses, null, array());
		}
		if (!empty($mayAttributes)) {
			$row->add(new htmlSubTitle(_('Optional attributes')));
		}
		foreach ($mayAttributes as $mayAttribute) {
			$values = empty($attributes[$mayAttribute->getName()]) ? array('') : $attributes[$mayAttribute->getName()];
			$this->addAttributeContent($row, $mayAttribute->getName(), $values, $mayAttribute, $objectClasses, null, array());
		}
		$row->addVerticalSpacer('1rem');
		$nextButton = new htmlButton('save', _('Create'));
		$nextButton->setOnClick('window.lam.treeview.createNodeEnterAttributesStep(event, \'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');');
		$row->add($nextButton, 12, 12, 12, 'text-center');
		$row->add(new htmlHiddenInput('objectClasses', implode(',', $objectClasses)));
		$row->add(new htmlHiddenInput('parentDn', base64_encode($dn)));
		ob_start();
		$tabIndex = 1;
		parseHtml(null, $row, array(), false, $tabIndex, '');
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(array('content' => $content));
	}

	/**
	 * Gets a recursive list of must attribute names.
	 *
	 * @param ObjectClass[] $objectClasses schema object classes
	 * @param string $objectClass object class
	 * @return array attribute names
	 */
	private function getMustAttributeNamesRecursive(array $objectClasses, string $objectClass): array {
		$objectClassLower = strtolower($objectClass);
		$objectClassObject = $objectClasses[$objectClassLower];
		$attributeNames = $objectClassObject->getMustAttrNames();
		if ($attributeNames === null) {
			$attributeNames = array();
		}
		if (!empty($objectClassObject->getSupClasses())) {
			foreach ($objectClassObject->getSupClasses() as $superClass) {
				$attributeNames = array_merge($attributeNames, $this->getMustAttributeNamesRecursive($objectClasses, $superClass));
			}
		}
		$attributeNames = array_map('strtolower', $attributeNames);
		$attributeNames = array_unique($attributeNames);
		return $attributeNames;
	}

	/**
	 * Gets a recursive list of may attribute names.
	 *
	 * @param ObjectClass[] $objectClasses schema object classes
	 * @param string $objectClass object class
	 * @return array attribute names
	 */
	private function getMayAttributeNamesRecursive(array $objectClasses, string $objectClass): array {
		$objectClassLower = strtolower($objectClass);
		$objectClassObject = $objectClasses[$objectClassLower];
		$attributeNames = $objectClassObject->getMayAttrNames();
		if ($attributeNames === null) {
			$attributeNames = array();
		}
		if (!empty($objectClassObject->getSupClasses())) {
			foreach ($objectClassObject->getSupClasses() as $superClass) {
				$attributeNames = array_merge($attributeNames, $this->getMayAttributeNamesRecursive($objectClasses, $superClass));
			}
		}
		$attributeNames = array_map('strtolower', $attributeNames);
		$attributeNames = array_unique($attributeNames);
		return $attributeNames;
	}

	/**
	 * Returns the content for the check attributes step of node creation.
	 *
	 * @param string $dn DN
	 * @param string|null $errorMessage error if any
	 * @return string JSON data
	 */
	private function createNewNodeCheckAttributesStep(string $dn, string $errorMessage = null): string {
		$objectClasses = $_POST['objectClasses'];
		$rdnAttribute = $_POST['rdn'];
		$attributeChanges = json_decode($_POST['attributes'], true);
		$attributes = array('objectClass' => explode(',', $objectClasses));
		foreach ($attributeChanges as $attributeName => $attributeChange) {
			if (isset($attributeChange['new'])) {
				$attributes[$attributeName] = $attributeChange['new'];
				if (isset($attributeChange['hash'])) {
					$attributes[$attributeName] = $this->applyPasswordHash($attributes[$attributeName], $attributeChange['hash']);
				}
			}
			if (isset($attributeChange['upload'])) {
				$attributes[$attributeName][] = base64_decode($attributeChange['upload']);
			}
		}
		if (!isset($attributes[$rdnAttribute][0])) {
			return $this->createNewNodeCheckObjectClassesStep($dn, _('The RDN field is empty.'), $rdnAttribute, $attributes);
		}
		$rdn = $rdnAttribute . '=' . ldap_escape($attributes[$rdnAttribute][0], '', LDAP_ESCAPE_DN);
		$newDn = $rdn . ',' . $dn;
		$success = ldap_add($_SESSION['ldap']->server(), $newDn, $attributes);
		if (!$success) {
			return $this->createNewNodeCheckObjectClassesStep($dn, getExtendedLDAPErrorMessage($_SESSION['ldap']->server()), $rdnAttribute, $attributes);
		}
		return $this->getNodeContent($newDn, new htmlStatusMessage('INFO',
			sprintf(_('Creation successful. DN <b>%s</b> has been created.'),
				htmlspecialchars(unescapeLdapSpecialCharacters($newDn)))));
	}

	/**
	 * Deletes a node in LDAP.
	 *
	 * @param string $dn DN
	 * @return string JSON
	 */
	private function deleteNode(string $dn): string {
		$errors = deleteDN($dn, true);
		foreach ($errors as $error) {
			logNewMessage(LOG_ERR, 'Tree view delete node failed: ' . $error[0] . ' ' . $error[1]);
		}
		if (!empty($errors)) {
			return json_encode(array('errors' => $errors));
		}
		return json_encode(array());
	}

	/**
	 * Stops processing if DN is invalid.
	 *
	 * @param string $dn DN
	 */
	private function validateDn(string $dn): void {
		$dn = strtolower($dn);
		$toolSettings = $_SESSION['config']->getToolSettings();
		$rootDn = strtolower($toolSettings[TreeViewTool::TREE_SUFFIX_CONFIG]);
		if (substr($dn, -1 * strlen($rootDn)) !== $rootDn) {
			logNewMessage(LOG_ERR, 'Invalid DN for tree view: ' . $dn);
			die();
		}
	}

	/**
	 * Returns the proper spelling of the attribute name.
	 *
	 * @param string $attributeName attribute name in lower-case
	 * @return string proper attribute name
	 */
	private function getProperAttributeName(string $attributeName): string {
		$schemaAttributes = $this->getSchemaAttributes();
		if (isset($schemaAttributes[$attributeName])) {
			return $schemaAttributes[$attributeName]->getName();
		}
		return $attributeName;
	}

	/**
	 * Returns the schema attributes.
	 *
	 * @return array attributes
	 */
	private function getSchemaAttributes(): array {
		if ($this->schemaAttributes === null) {
			$this->schemaAttributes = get_schema_attributes(null);
		}
		return $this->schemaAttributes;
	}

	/**
	 * Returns the schema object classes.
	 *
	 * @return ObjectClass[] object classes
	 */
	private function getSchemaObjectClasses(): array {
		if ($this->schemaObjectClasses === null) {
			$this->schemaObjectClasses = get_schema_objectclasses();
		}
		return $this->schemaObjectClasses;
	}

	/**
	 * Stops processing if no write access is allowed.
	 */
	private function ensureWriteAccess(): void {
		if (!checkIfWriteAccessIsAllowed()) {
			logNewMessage(LOG_ERR, 'Write operation denied for tree view.');
			die();
		}
	}

	/**
	 * Renders the search mask.
	 *
	 * @param string $dn DN
	 * @return string JSON
	 */
	private function search(string $dn): string {
		$row = new htmlResponsiveRow();
		$row->add(new htmlTitle(_('Search')));
		$row->addLabel(new htmlOutputText(_('Base DN')));
		$row->addField(new htmlOutputText(unescapeLdapSpecialCharacters($dn)));
		$row->addVerticalSpacer('1rem');
		$scopeOptions = array(
			_('Sub (entire subtree)') => 'sub',
			_('One (one level beneath base)') => 'one',
		);
		$scopeSelect = new htmlResponsiveSelect('scope', $scopeOptions, array(), _('Search scope'));
		$scopeSelect->setSortElements(false);
		$scopeSelect->setHasDescriptiveElements(true);
		$row->add($scopeSelect);
		$filterInput = new htmlResponsiveInputField(_('Search filter'), 'filter', '(objectClass=*)', null, true);
		$row->add($filterInput);
		$row->add(new htmlResponsiveInputField(_('Attributes'), 'attributes', 'cn, givenName, sn, uid', null, true));
		$row->add(new htmlResponsiveInputField(_('Order by'), 'orderBy', 'dn'));
		$resultCountInput = new htmlResponsiveInputField(_('LDAP search limit'), 'limit', '50');
		$resultCountInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$row->add($resultCountInput);
		$displayFormat = array(
			_('List') => 'list',
			_('Table') => 'table'
		);
		$formatSelect = new htmlResponsiveSelect('format', $displayFormat, array('list'), _('Display format'));
		$formatSelect->setHasDescriptiveElements(true);
		$row->add($formatSelect);
		$row->addVerticalSpacer('2rem');
		$nextButton = new htmlButton('search', _('Search'));
		$nextButton->setOnClick('window.lam.treeview.searchResults(event, \'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
		$row->add($nextButton, 12, 12, 12, 'text-center');
		ob_start();
		$tabIndex = 1;
		parseHtml(null, $row, array(), false, $tabIndex, '');
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(array('content' => $content));
	}

	/**
	 * Renders the search results.
	 *
	 * @param string $dn DN
	 * @return string JSON
	 */
	private function searchResults(string $dn): string {
		$scope = $_POST['scope'];
		if (!in_array($scope, array('sub', 'one'))) {
			logNewMessage(LOG_ERR, 'Invalid search scope: ' . $scope);
			die();
		}
		$format = $_POST['format'];
		if (!in_array($format, array('list', 'table'))) {
			logNewMessage(LOG_ERR, 'Invalid search format: ' . $format);
			die();
		}
		$filter = empty($_POST['filter']) ? '(objectClass=*)' : $_POST['filter'];
		$attributes = preg_split('/,[ ]*/', $_POST['attributes']);
		$searchAttributes = $attributes;
		if (!in_array_ignore_case('objectClass', $searchAttributes)) {
			$searchAttributes[] = 'objectClass';
		}
		global $lamOrderByAttribute;
		$lamOrderByAttribute = empty($_POST['orderBy']) ? 'dn' : strtolower($_POST['orderBy']);
		$limit = empty($_POST['limit']) ? 0 : intval($_POST['limit']);
		$results = array();
		switch ($scope) {
			case 'sub':
				$results = searchLDAP($dn, $filter, $searchAttributes, $limit);
				break;
			case 'one':
				$results = ldapListDN($dn, $filter, $searchAttributes, null, $limit);
				break;
		}
		usort($results, 'LAM\TOOLS\TREEVIEW\compareByAttributes');
		$row = $this->searchResultsHeader($dn, $filter);
		if ($format === 'list') {
			return $this->searchResultsAsList($results, $attributes, $row);
		}
		return $this->searchResultsAsTable($results, $attributes, $row);
	}

	/**
	 * Creates the header part of the search results.
	 *
	 * @param string $dn search base
	 * @param string $filter LDAP filter
	 * @return htmlResponsiveRow content
	 */
	private function searchResultsHeader(string $dn, string $filter): htmlResponsiveRow {
		$row = new htmlResponsiveRow();
		$row->add(new htmlTitle(_('Search Results')));
		$row->addLabel(new htmlOutputText(_('Base DN')));
		$row->addField(new htmlOutputText(unescapeLdapSpecialCharacters($dn)));
		$row->addVerticalSpacer('0.5rem');
		$row->addLabel(new htmlOutputText(_('Search filter')));
		$row->addField(new htmlOutputText($filter));
		$row->addVerticalSpacer('2rem');
		return $row;
	}

	/**
	 * Returns the search results as list.
	 *
	 * @param array $results results
	 * @param array $attributes attribute list to show
	 * @param htmlResponsiveRow $row content
	 * @return string JSON
	 */
	private function searchResultsAsList(array $results, array $attributes, htmlResponsiveRow $row): string {
		foreach ($results as $result) {
			$row->add(new htmlSubTitle(getAbstractDN($result['dn']), $this->getNodeIcon($result)));
			$row->addLabel(new htmlOutputText('dn'));
			$row->addField(new htmlLink(unescapeLdapSpecialCharacters($result['dn']), 'treeView.php?dn=' . base64_encode($result['dn'])));
			$result = array_change_key_case($result, CASE_LOWER);
			foreach ($attributes as $attribute) {
				$attributeLower = strtolower($attribute);
				if (!empty($result[$attributeLower])) {
					$row->addLabel(new htmlOutputText($attribute));
					$row->addField(new htmlOutputText(implode(', ', $result[$attributeLower])));
				}
			}
			$row->addVerticalSpacer('1rem');
		}
		ob_start();
		$tabIndex = 1;
		parseHtml(null, $row, array(), false, $tabIndex, '');
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(array('content' => $content));
	}

	/**
	 * Returns the search results as table.
	 *
	 * @param array $results results
	 * @param array $attributes attribute list to show
	 * @param htmlResponsiveRow $row content
	 * @return string JSON
	 */
	private function searchResultsAsTable(array $results, array $attributes, htmlResponsiveRow $row) {
		$titles = array_merge(array('', 'dn'), $attributes);
		$data = array();
		foreach ($results as $result) {
			$dataEntry = array($this->getNodeIcon($result), new htmlLink(unescapeLdapSpecialCharacters($result['dn']), 'treeView.php?dn=' . base64_encode($result['dn'])));
			$result = array_change_key_case($result, CASE_LOWER);
			foreach ($attributes as $attribute) {
				$attributeLower = strtolower($attribute);
				if (!empty($result[$attributeLower])) {
					$dataEntry[] = new htmlOutputText(implode(', ', $result[$attributeLower]));
				}
				else {
					$dataEntry[] = new htmlOutputText('');
				}
			}
			$data[] = $dataEntry;
		}
		$table = new htmlResponsiveTable($titles, $data);
		$table->setCSSClasses(array('tree-view--table'));
		$row->add($table);
		ob_start();
		$tabIndex = 1;
		parseHtml(null, $row, array(), false, $tabIndex, '');
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(array('content' => $content));
	}

	/**
	 * Performs paste operations.
	 *
	 * @param string $dn
	 * @return string
	 */
	private function paste(string $dn): string {
		$targetDn = base64_decode($_POST['targetDn']);
		$this->validateDn($targetDn);
		$action = $_POST['action'];
		if (!in_array($action, array('COPY', 'CUT'))) {
			logNewMessage(LOG_ERR, 'Invalid tree paste action: ' . $action);
			die();
		}
		try {
			copyDnRecursive($dn, $targetDn);
			if ($action === 'CUT') {
				$errors = deleteDN($dn, true);
				if (!empty($errors)) {
					$row = new htmlResponsiveRow();
					foreach ($errors as $error) {
						$row->add(new htmlStatusMessage($error[0], $error[1], isset($error[2]) ? $error[2] : null));
					}
					ob_start();
					$tabIndex = 1;
					parseHtml(null, $row, array(), false, $tabIndex, '');
					$content = ob_get_contents();
					ob_end_clean();
					return json_encode(array('error' => $content));
				}
			}
		}
		catch (LAMException $e) {
			$message = new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
			ob_start();
			$tabIndex = 1;
			parseHtml(null, $message, array(), false, $tabIndex, '');
			$content = ob_get_contents();
			ob_end_clean();
			return json_encode(array('error' => $content));
		}
		$newDn = extractRDN($dn) . ',' . $targetDn;
		return json_encode(array());
	}

}

/**
 * Compares two nodes by interpreting their ID as DN.
 *
 * @param $a first node
 * @param $b second node
 * @return int result
 */
function compareNodeByIdAsDn($a, $b): int {
	return strnatcasecmp(extractRDN(base64_decode($a['id'])), extractRDN(base64_decode($b['id'])));
}

/**
 * Compares two arrays with LDAP attributes by global $lamOrderByAttribute.
 *
 * @param $a first node
 * @param $b second node
 * @return int result
 */
function compareByAttributes($a, $b): int {
	global $lamOrderByAttribute;
	if ($lamOrderByAttribute === 'dn') {
		return compareDN($a['dn'], $b['dn']);
	}
	$a = array_change_key_case($a, CASE_LOWER);
	$b = array_change_key_case($b, CASE_LOWER);
	if (!isset($a[$lamOrderByAttribute]) && !isset($b[$lamOrderByAttribute])) {
		return 0;
	}
	if (!empty($a[$lamOrderByAttribute]) && empty($b[$lamOrderByAttribute])) {
		return 1;
	}
	if (empty($a[$lamOrderByAttribute]) && !empty($b[$lamOrderByAttribute])) {
		return -1;
	}
	natcasesort($a[$lamOrderByAttribute]);
	natcasesort($b[$lamOrderByAttribute]);
	$maxA = array_pop($a[$lamOrderByAttribute]);
	$maxB = array_pop($b[$lamOrderByAttribute]);
	return strnatcasecmp($maxA, $maxB);
}
