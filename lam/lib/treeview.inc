<?php
namespace LAM\TOOLS\TREEVIEW;
use htmlButton;
use htmlDiv;
use htmlElement;
use htmlGroup;
use htmlHiddenInput;
use htmlInputField;
use htmlInputTextarea;
use htmlLink;
use htmlOutputText;
use htmlResponsiveRow;
use htmlResponsiveSelect;
use htmlStatusMessage;
use htmlSubTitle;
use htmlTable;
use htmlTitle;
use LAM\SCHEMA\AttributeType;
use LAM\SCHEMA\ObjectClass;
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
			default:
				logNewMessage(LOG_ERR, 'Invalid command for tree view: ' . $command);
				die;
		}
		return json_encode(array('123'));
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
			$icon = 'o.png';
		}
		elseif (in_array('organizationalunit', $objectClasses)) {
			$icon = 'ou.png';
		}
		elseif (in_array('dcobject', $objectClasses) ||
			in_array('domainrelatedobject', $objectClasses) ||
			in_array('domain', $objectClasses) ||
			in_array('builtindomain', $objectClasses)) {
			$icon = 'o.png';
		}
		elseif (in_array('alias', $objectClasses)) {
			$icon = 'alias.png';
		}
		elseif (in_array('document', $objectClasses)) {
			$icon = 'txt.png';
		}
		elseif (in_array('country', $objectClasses)) {
			$icon = 'country.png';
		}
		elseif (in_array('locality', $objectClasses)) {
			$icon = 'location.png';
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
			$icon = 'ou.png';
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
	 * Returns the node content with the attribute listing.
	 *
	 * @return string JSON
	 */
	private function getNodeContent(string $dn): string {
		$row = new htmlResponsiveRow();
		$row->add(new htmlTitle($dn), 12);
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
		$schemaObjectClasses = $this->getSchemaObjectClasses();
		$possibleNewAttributes = array();
		foreach ($attributes['objectclass'] as $objectclass) {
			$objectclass = strtolower($objectclass);
			if (empty($schemaObjectClasses[$objectclass])) {
				continue;
			}
			$optionalAttributes = $schemaObjectClasses[$objectclass]->getMayAttrs();
			foreach ($optionalAttributes as $optionalAttribute) {
				$attributeName = $optionalAttribute->getName();
				if (!isset($attributes[strtolower($attributeName)])) {
					$schemaAttribute = $schemaAttributes[strtolower($attributeName)];
					$single = $schemaAttribute->getIsSingleValue() ? 'single' : 'multi';
					$multiLine = $this->isMultiLineAttribute($attributeName, $schemaAttribute) ? 'textarea' : 'input';
					$possibleNewAttributes[$attributeName] = $attributeName . '__#__' . $single . '__#__' . $multiLine;
				}
			}
		}
		ksort($possibleNewAttributes);
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
			$newAttributesContentSingleInput->addField($this->getAttributeContentField('PLACEHOLDER_SINGLE_INPUT', array(''), false, false, false));
			$row->add(new htmlDiv('new-attributes-single-input', $newAttributesContentSingleInput, array('hidden')), 12);
			$newAttributesContentMultiInput = new htmlResponsiveRow();
			$newAttributesContentMultiInput->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_INPUT_LABEL'));
			$newAttributesContentMultiInput->addField($this->getAttributeContentField('PLACEHOLDER_MULTI_INPUT', array(''), false, true, false));
			$row->add(new htmlDiv('new-attributes-multi-input', $newAttributesContentMultiInput, array('hidden')), 12);
			$newAttributesContentSingleTextarea = new htmlResponsiveRow();
			$newAttributesContentSingleTextarea->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_TEXTAREA_LABEL'));
			$newAttributesContentSingleTextarea->addField($this->getAttributeContentField('PLACEHOLDER_SINGLE_TEXTAREA', array(''), false, false, true));
			$row->add(new htmlDiv('new-attributes-single-textarea', $newAttributesContentSingleTextarea, array('hidden')), 12);
			$newAttributesContentMultiTextarea = new htmlResponsiveRow();
			$newAttributesContentMultiTextarea->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_TEXTAREA_LABEL'));
			$newAttributesContentMultiTextarea->addField($this->getAttributeContentField('PLACEHOLDER_MULTI_TEXTAREA', array(''), false, true, true));
			$row->add(new htmlDiv('new-attributes-multi-textarea', $newAttributesContentMultiTextarea, array('hidden')), 12);
		}

		if (checkIfWriteAccessIsAllowed()) {
			$row->addVerticalSpacer('2rem');
			$saveButton = new htmlLink(_('Save'), '#', '../../graphics/save.png', true);
			$saveButton->setId('savebutton');
			$saveButton->setOnClick('window.lam.treeview.saveAttributes(event,
			"' . getSecurityTokenName() . '",
			"' . getSecurityTokenValue() . '",
			"' . base64_encode($dn) . '");');
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
		$isMultiValue = (sizeof($values) > 1) || (($schemaAttribute !== null) && ($schemaAttribute->getIsSingleValue() !== true));
		$isMultiLine = $this->isMultiLineAttribute($attributeName, $schemaAttribute);
		$row->addField($this->getAttributeContentField($attributeName, $values, $required, $isMultiValue, $isMultiLine), $cssClasses);
		$row->addVerticalSpacer('0.5rem');
	}

	/**
	 * Returns the input fields for the attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param array $values values
	 * @param bool $required is required
	 * @param bool $isMultiValue multi-value attribute
	 * @param bool $isMultiLine textarea attribute
	 * @return htmlElement content
	 */
	private function getAttributeContentField(string $attributeName, array $values, bool $required, bool $isMultiValue, bool $isMultiLine): htmlElement {
		if (!$isMultiValue) {
			return $this->getAttributeInputField($attributeName, $values[0], $required, $isMultiLine, true);
		}
		$valueTable = new htmlTable();
		$valueTable->setCSSClasses(array('fullwidth'));
		foreach ($values as $value) {
			$valueTable->addElement($this->getAttributeInputField($attributeName, $value, $required, $isMultiLine, false));
			if (checkIfWriteAccessIsAllowed()) {
				$addButton = new htmlLink(null, '#', '../../graphics/add.png');
				$addButton->setCSSClasses(array('margin2'));
				$addButton->setOnClick('window.lam.treeview.addValue(event, this);');
				$valueTable->addElement($addButton);
				$clearButton = new htmlLink(null, '#', '../../graphics/del.png');
				$clearButton->setCSSClasses(array('margin2'));
				$clearButton->setOnClick('window.lam.treeview.clearValue(event, this);');
				$valueTable->addElement($clearButton);
			}
			$valueTable->addNewLine();
		}
		return $valueTable;
	}

	/**
	 * Returns an input field for an LDAP attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param string $value value
	 * @param bool $required required
	 * @param bool $isMultiLine is multi-line attribute
	 * @param bool $isSingleValue is single value attribute
	 * @return htmlElement input field
	 */
	private function getAttributeInputField(string $attributeName, string $value, bool $required, bool $isMultiLine, bool $isSingleValue): htmlElement {
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
		$changes = json_decode($_POST['changes'], true);
		array_change_key_case($changes, CASE_LOWER);
		logNewMessage(LOG_DEBUG, 'LDAP changes for ' . $dn . ': ' . print_r($changes, true));
		$ldapChanges = array();
		if (!empty($changes)) {
			foreach ($changes as $attrName => $change) {
				$newValues = $change['new'];
				$ldapChanges[$attrName] = $newValues;
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
			$newRdn = $rdnAttribute . '=' . $newRdnValue;
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
	 * @return string JSON data
	 */
	private function createNewNodeCheckObjectClassesStep(string $dn, string $errorMessage = null): string {
		$objectClasses = $_POST['objectClasses'];
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
		}
		$row->addLabel(new htmlOutputText(_('Parent')));
		$row->addField(new htmlOutputText($dn));
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
		if (isset($allAttributes['cn'])) {
			$rdnOptionsSelected[] = $allAttributes['cn']->getName();
		}
		$row->add(new htmlResponsiveSelect('rdn', $rdnOptions, $rdnOptionsSelected, _('RDN identifier')));
		if (!empty($mustAttributes)) {
			$row->add(new htmlSubTitle(_('Required attributes')));
		}
		foreach ($mustAttributes as $mustAttribute) {
			$this->addAttributeContent($row, $mustAttribute->getName(), array(''), $mustAttribute, $objectClasses, null, array());
		}
		if (!empty($mayAttributes)) {
			$row->add(new htmlSubTitle(_('Optional attributes')));
		}
		foreach ($mayAttributes as $mayAttribute) {
			$this->addAttributeContent($row, $mayAttribute->getName(), array(''), $mayAttribute, $objectClasses, null, array());
		}
		$row->addVerticalSpacer('1rem');
		$nextButton = new htmlButton('save', _('Create'));
		$nextButton->setOnClick('window.lam.treeview.createNodeEnterAttributesStep(event, \'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');');
		$row->add($nextButton, 12, 12, 12, 'text-center');
		$row->add(new htmlHiddenInput('objectClasses', implode(',', $objectClasses)));
		$row->add(new htmlHiddenInput('parentDn', $dn));
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
		$toolSettings = $_SESSION['config']->getToolSettings();
		$rootDn = $toolSettings[TreeViewTool::TREE_SUFFIX_CONFIG];
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
