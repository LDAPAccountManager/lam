<?php

namespace LAM\TOOLS\TREEVIEW;

use htmlButton;
use htmlDiv;
use htmlElement;
use htmlGroup;
use htmlHiddenInput;
use htmlImage;
use htmlInputField;
use htmlInputFileUpload;
use htmlInputTextarea;
use htmlLink;
use htmlList;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlResponsiveSelect;
use htmlResponsiveTable;
use htmlSelect;
use htmlSortableList;
use htmlSpacer;
use htmlStatusMessage;
use htmlSubTitle;
use htmlTable;
use htmlTitle;
use LAM\SCHEMA\AttributeType;
use LAM\SCHEMA\ObjectClass;
use LAMException;
use LamTemporaryFilesManager;
use function LAM\SCHEMA\get_schema_attributes;
use function LAM\SCHEMA\get_schema_objectclasses;


/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021 - 2026  Roland Gruber

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

include_once __DIR__ . '/account.inc';
include_once __DIR__ . '/tools.inc';
include_once __DIR__ . '/tools/treeview.inc';

/**
 * Tree view functions.
 *
 * @package LAM\TOOLS\TREEVIEW
 */
class TreeView {
	const AD_DELETED_DELIMITER = '\0ADEL:';

	/**
	 * @var AttributeType[] schema attributes
	 */
	private $schemaAttributes;

	/**
	 * @var ObjectClass[] schema object classes
	 */
	private $schemaObjectClasses;

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
		switch ($command) {
			case 'getRootNodes':
				return $this->getRootNodes();
			case 'getNodes':
				return $this->getSubNodes(base64_decode($_GET['dn']));
		}
		if (empty($_POST['dn'])) {
			logNewMessage(LOG_ERR, 'No dn for tree view.');
			die;
		}
		$dn = ($_POST['dn'] === '#') ? '#' : base64_decode($_POST['dn']);
		switch ($command) {
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
			case 'restoreNode':
				$this->ensureWriteAccess();
				$this->validateDn($dn);
				return $this->restoreNode($dn);
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
			case 'compare':
				$this->validateDn($dn);
				return $this->compare();
			default:
				logNewMessage(LOG_ERR, 'Invalid command for tree view: ' . $command);
				die;
		}
	}

	/**
	 * Returns the JSON for the possible new attributes select.
	 *
	 * @return string JSON
	 */
	private function getPossibleNewAttributeNameOptionsJson() {
		$objectClasses = json_decode($_POST['objectClasses'], true);
		$attributeNames = $this->getPossibleNewAttributeNameOptions($objectClasses, true);
		natcasesort($attributeNames);
		return json_encode(['data' => $attributeNames]);
	}

	/**
	 * Returns a list of root nodes for the tree view.
	 *
	 * @return string JSON data
	 */
	private function getRootNodes(): string {
		$rootDns = TreeViewTool::getRootDns();
		$result = [];
		foreach ($rootDns as $rootDn) {
			logNewMessage(LOG_DEBUG, 'Getting tree nodes for ' . $rootDn);
			$rootData = ldapGetDN($rootDn, ['objectClass']);
			if ($rootData === null) {
				continue;
			}
			$children = ldapListDN($rootDn, '(objectClass=*)', ['objectClass', 'hassubordinates']);
			$nodeData = $this->createNodeData($rootData, true, true, $children);
			$result[] = $nodeData;
		}
		return json_encode($result);
	}

	/**
	 * Returns the subnodes of the given DN.
	 *
	 * @param string $dn DN
	 * @return string JSON data
	 */
	private function getSubNodes(string $dn): string {
		$this->validateDn($dn);
		logNewMessage(LOG_DEBUG, 'Getting tree nodes for ' . $dn);
		$children = ldapListDN($dn, '(objectClass=*)', ['objectClass', 'hassubordinates']);
		$childNodes = [];
		foreach ($children as $child) {
			$childHasChildren = !isset($child['hassubordinates'][0]) || ($child['hassubordinates'][0] === 'TRUE');
			$childNodes[] = $this->createNodeData($child, false, false, [], $childHasChildren);
		}
		$this->sortNodes($childNodes);
		return json_encode($childNodes);
	}

	/**
	 * Creates the node data for the tree view.
	 *
	 * @param array<string, string[]|string> $attributes LDAP data
	 * @param bool $open open node
	 * @param bool $noShortenFirst do not shorten DN
	 * @param array<string, string[]|string>[] $children child LDAP data
	 * @param bool $hasChildren indicates that the entry has children
	 * @return array<string, string|true|array<array<string, string>|bool>> node data
	 */
	private function createNodeData(array $attributes, bool $open = false, bool $noShortenFirst = false, array $children = [], bool $hasChildren = true): array {
		$text = ($noShortenFirst) ? $attributes['dn'] : extractRDN($attributes['dn']);
		$data = [
			'key' => base64_encode($attributes['dn']),
			'title' => unescapeLdapSpecialCharacters($text),
			'icon' => $this->getNodeIcon($attributes),
		];
		if ($hasChildren) {
			$data['lazy'] = true;
		}
		if (!empty($children)) {
			$childData = [];
			foreach ($children as $child) {
				$childHasChildren = !isset($child['hassubordinates'][0]) || ($child['hassubordinates'][0] === 'TRUE');
				$childData[] = $this->createNodeData($child, false, false, [], $childHasChildren);
			}
			$this->sortNodes($childData);
			$data['children'] = $childData;
		}
		if ($open) {
			$data['state'] = [
				'expanded' => true
			];
		}
		return $data;
	}

	/**
	 * Returns the node's icon.
	 *
	 * @param array<string, string[]|string> $attributes LDAP data
	 * @return string icon
	 */
	private function getNodeIcon(array $attributes): string {
		$base = '../../graphics/';
		$icon = 'object.svg';
		$objectClasses = array_map(strtolower(...), $attributes['objectclass']);
		$rdn = extractRDNValue($attributes['dn']);
		if (in_array('sambasamaccount', $objectClasses) &&
			'$' == $rdn[strlen($rdn) - 1]) {
			$icon = 'samba.svg';
		}
		if (in_array('sambasamaccount', $objectClasses)) {
			$icon = 'samba.svg';
		}
		elseif (in_array('pwdpolicy', $objectClasses)) {
			$icon = 'locked.svg';
		}
		elseif (in_array('person', $objectClasses) ||
			in_array('organizationalperson', $objectClasses) ||
			in_array('inetorgperson', $objectClasses) ||
			in_array('account', $objectClasses) ||
			in_array('posixaccount', $objectClasses) ||
			in_array('organizationalrole', $objectClasses)) {
			$icon = 'user.svg';
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
			$icon = 'link.svg';
		}
		elseif (in_array('document', $objectClasses)) {
			$icon = 'txt.svg';
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
			$icon = 'group.svg';
		}
		elseif (in_array('iphost', $objectClasses)) {
			$icon = 'computer-small.svg';
		}
		elseif (in_array('device', $objectClasses)) {
			$icon = 'device.svg';
		}
		elseif (in_array('server', $objectClasses)) {
			$icon = 'computer-small.svg';
		}
		elseif (in_array('volume', $objectClasses)) {
			$icon = 'hard-drive.svg';
		}
		elseif (in_array('container', $objectClasses)) {
			$icon = 'folder.svg';
		}
		return $base . $icon;
	}

	/**
	 * Sorts the given node array by DN.
	 *
	 * @param array<string, string>[] $nodes nodes
	 */
	private function sortNodes(array &$nodes): void {
		usort($nodes, compareNodeByIdAsDn(...));
	}

	/**
	 * Returns the options for the drop-down to add a new attribute.
	 *
	 * @param string[] $objectClasses object classes
	 * @param bool $includeMustAttributes include required attributes
	 * @return array<string, string> list of options
	 */
	private function getPossibleNewAttributeNameOptions(array $objectClasses, bool $includeMustAttributes = false): array {
		$schemaObjectClasses = $this->getSchemaObjectClasses();
		$schemaAttributes = $this->getSchemaAttributes();
		$possibleNewAttributes = [];
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
		$row->add(new htmlTitle(unescapeLdapSpecialCharacters($dn)));
		$attributes = ldapGetDN($dn, ['*']);
		$this->addActionBar($row, $dn, $attributes);
		$row->add(new htmlDiv('ldap_actionarea_messages', new htmlOutputText('')));
		$row->add(new htmlSubTitle(_('Attributes')));
		unset($attributes['dn']);
		ksort($attributes);
		logNewMessage(LOG_DEBUG, 'LDAP attributes for ' . $dn . ': ' . print_r($attributes, true));
		$schemaAttributes = $this->getSchemaAttributes();
		$objectClasses = $attributes['objectclass'];
		$highlighted = (empty($_POST['highlight'])) ? [] : explode(',', $_POST['highlight']);
		foreach ($attributes as $attributeName => $values) {
			$schemaAttribute = null;
			if (str_contains($attributeName, ';binary')) {
				$attributeName = substr($attributeName, 0, -1 * strlen(';binary'));
			}
			if (isset($schemaAttributes[$attributeName])) {
				$schemaAttribute = $schemaAttributes[$attributeName];
				$attributeName = $schemaAttribute->getName();
			}
			$this->addAttributeContent($row, $attributeName, $values, $schemaAttribute, $objectClasses, $dn, $highlighted);
		}
		$row->addVerticalSpacer('1rem');

		// add new attributes
		$possibleNewAttributes = $this->getPossibleNewAttributeNameOptions($objectClasses);
		foreach (array_keys($attributes) as $attributeName) {
			if (isset($possibleNewAttributes[$attributeName])) {
				unset($possibleNewAttributes[$attributeName]);
			}
		}
		logNewMessage(LOG_DEBUG, 'Possible new attributes for ' . $dn . ': ' . implode('; ', $possibleNewAttributes));
		$possibleNewAttributes = ['' => ''] + $possibleNewAttributes;
		$row->add(new htmlSubTitle(_('Add new attribute')));
		$newAttributeSelect = new htmlResponsiveSelect('newAttribute', $possibleNewAttributes, [], _('Attribute'));
		$newAttributeSelect->setHasDescriptiveElements(true);
		$newAttributeSelect->setTransformSingleSelect(false);
		$newAttributeSelect->setOnchangeEvent('window.lam.treeview.addAttributeField(event, this);');
		$row->add($newAttributeSelect);
		$newAttributesContentSingleInput = new htmlResponsiveRow();
		$newAttributesContentSingleInput->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_INPUT_LABEL'));
		$newAttributesContentSingleInput->addField($this->getAttributeContentField('placeholder' . generateRandomText(), [''], false, false, false, null));
		$row->add(new htmlDiv('new-attributes-single-input', $newAttributesContentSingleInput, ['hidden']));
		$newAttributesContentMultiInput = new htmlResponsiveRow();
		$newAttributesContentMultiInput->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_INPUT_LABEL'));
		$newAttributesContentMultiInput->addField($this->getAttributeContentField('placeholder' . generateRandomText(), [''], false, true, false, null));
		$row->add(new htmlDiv('new-attributes-multi-input', $newAttributesContentMultiInput, ['hidden']));
		$newAttributesContentSingleTextarea = new htmlResponsiveRow();
		$newAttributesContentSingleTextarea->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_TEXTAREA_LABEL'));
		$newAttributesContentSingleTextarea->addField($this->getAttributeContentField('placeholder' . generateRandomText(), [''], false, false, true, null));
		$row->add(new htmlDiv('new-attributes-single-textarea', $newAttributesContentSingleTextarea, ['hidden']));
		$newAttributesContentMultiTextarea = new htmlResponsiveRow();
		$newAttributesContentMultiTextarea->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_TEXTAREA_LABEL'));
		$newAttributesContentMultiTextarea->addField($this->getAttributeContentField('placeholder' . generateRandomText(), [''], false, true, true, null));
		$row->add(new htmlDiv('new-attributes-multi-textarea', $newAttributesContentMultiTextarea, ['hidden']));
		$newAttributesContentSinglePassword = new htmlResponsiveRow();
		$newAttributesContentSinglePassword->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_PASSWORD_LABEL'));
		$newAttributesContentSinglePassword->addField($this->getAttributeContentField('userpassword' . generateRandomText(), [''], false, false, false, null));
		$row->add(new htmlDiv('new-attributes-single-password', $newAttributesContentSinglePassword, ['hidden']));
		$newAttributesContentMultiPassword = new htmlResponsiveRow();
		$newAttributesContentMultiPassword->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_PASSWORD_LABEL'));
		$newAttributesContentMultiPassword->addField($this->getAttributeContentField('userpassword' . generateRandomText(), [''], false, true, false, null));
		$row->add(new htmlDiv('new-attributes-multi-password', $newAttributesContentMultiPassword, ['hidden']));
		$newAttributesContentSingleJpeg = new htmlResponsiveRow();
		$newAttributesContentSingleJpeg->addLabel(new htmlOutputText('PLACEHOLDER_SINGLE_JPEG_LABEL'));
		$newAttributesContentSingleJpeg->addField($this->getAttributeContentField('jpegphoto' . generateRandomText(), [''], false, false, false, null));
		$row->add(new htmlDiv('new-attributes-single-jpeg', $newAttributesContentSingleJpeg, ['hidden']));
		$newAttributesContentMultiJpeg = new htmlResponsiveRow();
		$newAttributesContentMultiJpeg->addLabel(new htmlOutputText('PLACEHOLDER_MULTI_JPEG_LABEL'));
		$newAttributesContentMultiJpeg->addField($this->getAttributeContentField('jpegphoto' . generateRandomText(), [''], false, true, true, null));
		$row->add(new htmlDiv('new-attributes-multi-jpeg', $newAttributesContentMultiJpeg, ['hidden']));

		if (checkIfWriteAccessIsAllowed()) {
			$row->addVerticalSpacer('2rem');
			$saveButton = new htmlButton('savebutton', _('Save'));
			$saveButton->setOnClick('window.lam.treeview.saveAttributes(event, '
				. "'" . getSecurityTokenName() . "', "
				. "'" . getSecurityTokenValue() . "', "
				. "'" . base64_encode($dn) . "');");
			$saveButton->setCSSClasses(['lam-primary']);
			$row->add($saveButton, 12, 12, 12, 'text-center');
		}

		$internalAttributesContent = new htmlResponsiveRow();
		$internalAttributesContent->add(new htmlSubTitle(_('Internal attributes')));
		$internalAttributesButton = new htmlLink(_('Show internal attributes'), '#', null);
		$internalAttributesButton->setOnClick('window.lam.treeview.getInternalAttributesContent(event,
			"' . getSecurityTokenName() . '",
			"' . getSecurityTokenValue() . '",
			"' . base64_encode($dn) . '");');
		$internalAttributesButton->setId('internalAttributesButton');
		$internalAttributesContent->add($internalAttributesButton);
		$internalAttributesDiv = new htmlDiv('actionarea-internal-attributes', $internalAttributesContent);
		$row->add($internalAttributesDiv);
		ob_start();
		parseHtml(null, $row, [], false);
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(['content' => $content]);
	}

	/**
	 * Adds the action buttons.
	 *
	 * @param htmlResponsiveRow $row container
	 * @param string $dn entry DN
	 * @param array<string, string[]|string> $attributes LDAP attributes
	 */
	private function addActionBar(htmlResponsiveRow $row, string $dn, array $attributes): void {
		$buttonGroup = new htmlGroup();
		$buttonSize = '16px';
		$buttonClasses = ['clickable', 'lam-margin-small', 'icon'];
		$isRestorable = isset($attributes['isdeleted'][0]) && ($attributes['isdeleted'][0] === 'TRUE') && str_contains($dn, '\0ADEL');

		if (checkIfWriteAccessIsAllowed()) {
			$createButton = new htmlImage('../../graphics/add.svg', $buttonSize, $buttonSize, _('Create a child entry'), _('Create a child entry'));
			$createButton->setOnClick('window.lam.treeview.createNode(\'' . getSecurityTokenName() . '\', '
				. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
			$createButton->setCSSClasses($buttonClasses);
			$buttonGroup->addElement($createButton);

			if (!$isRestorable) {
				$deleteButton = new htmlImage('../../graphics/delete.svg', $buttonSize, $buttonSize, _('Delete'), _('Delete'));
				$deleteButton->setOnClick('window.lam.treeview.deleteNode(\'' . getSecurityTokenName() . '\', '
					. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\', \'' . base64_encode(htmlspecialchars(extractRDN($dn))) . '\', '
					. '\'' . addslashes(_('Delete')) . '\', \'' . addslashes(_('Cancel')) . '\', \'' . addslashes(_('Delete this entry')) . '\', \'' . addslashes(_('Ok')) . '\', '
					. '\'' . addslashes(_('Error')) . '\');');
				$deleteButton->setCSSClasses($buttonClasses);
				$buttonGroup->addElement($deleteButton);
			}
		}

		$refreshButton = new htmlImage('../../graphics/refresh.svg', $buttonSize, $buttonSize, _('Refresh'), _('Refresh'));
		$refreshButton->setOnClick('mar10.Wunderbaum.getTree(\'#ldap_tree\').findKey(\'' . base64_encode($dn) . '\').loadLazy(true); '
			. 'window.lam.treeview.getNodeContent(\'' . getSecurityTokenName() . '\', '
			. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
		$refreshButton->setCSSClasses($buttonClasses);
		$buttonGroup->addElement($refreshButton);

		$buttonGroup->addElement(new htmlSpacer('0.5rem'));

		if (checkIfWriteAccessIsAllowed()) {
			$copyButton = new htmlImage('../../graphics/copy.svg', $buttonSize, $buttonSize, _('Copy'), _('Copy'));
			$copyButton->setOnClick('window.lam.treeview.copyNode(\'' . base64_encode($dn) . '\');');
			$copyButton->setCSSClasses($buttonClasses);
			$buttonGroup->addElement($copyButton);

			$cutButton = new htmlImage('../../graphics/cut.svg', $buttonSize, $buttonSize, _('Cut'), _('Cut'));
			$cutButton->setOnClick('window.lam.treeview.cutNode(\'' . base64_encode($dn) . '\');');
			$cutButton->setCSSClasses($buttonClasses);
			$buttonGroup->addElement($cutButton);

			$pasteButton = new htmlImage('../../graphics/paste.svg', $buttonSize, $buttonSize, _('Paste as new child entry'), _('Paste as new child entry'));
			$pasteButton->setOnClick('window.lam.treeview.pasteNode(\'' . getSecurityTokenName() . '\', '
				. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
			$pasteButton->setCSSClasses($buttonClasses);
			$pasteButton->setId('action_button_paste');
			$buttonGroup->addElement($pasteButton);

			$buttonGroup->addElement(new htmlSpacer('0.5rem'));
		}

		$searchButton = new htmlImage('../../graphics/search.svg', $buttonSize, $buttonSize, _('Search'), _('Search'));
		$searchButton->setOnClick('window.lam.treeview.search(\'' . getSecurityTokenName() . '\', '
			. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
		$searchButton->setCSSClasses($buttonClasses);
		$buttonGroup->addElement($searchButton);

		if ($_SESSION['config']->isToolActive('ImportExport')) {
			$exportButton = new htmlImage('../../graphics/export.svg', $buttonSize, $buttonSize, _('Export'), _('Export'));
			$exportButton->setOnClick('window.location.href = \'../tools/importexport.php?tab=export&dn=' . base64_encode($dn) . '\';');
			$exportButton->setCSSClasses($buttonClasses);
			$buttonGroup->addElement($exportButton);
		}

		$buttonGroup->addElement(new htmlSpacer('0.5rem'));

		$addToComparisonButton = new htmlImage('../../graphics/list-add.svg', $buttonSize, $buttonSize, _('Add to comparison'), _('Add to comparison'));
		$addToComparisonButton->setOnClick('window.lam.treeview.addToComparison(\'' . base64_encode($dn) . '\');');
		$addToComparisonButton->setCSSClasses($buttonClasses);
		$addToComparisonButton->setId('action_button_compare_add');
		$buttonGroup->addElement($addToComparisonButton);

		$removeFromComparisonButton = new htmlImage('../../graphics/list-remove.svg', $buttonSize, $buttonSize, _('Remove from comparison'), _('Remove from comparison'));
		$removeFromComparisonButton->setOnClick('window.lam.treeview.removeFromComparison(\'' . base64_encode($dn) . '\');');
		$removeFromComparisonButton->setCSSClasses($buttonClasses);
		$removeFromComparisonButton->setId('action_button_compare_remove');
		$buttonGroup->addElement($removeFromComparisonButton);

		$startComparisonButton = new htmlImage('../../graphics/compare.svg', $buttonSize, $buttonSize, _('Open comparison'), _('Open comparison'));
		$startComparisonButton->setOnClick('window.lam.treeview.openComparison(\'' . getSecurityTokenName() . '\', '
			. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
		$startComparisonButton->setCSSClasses($buttonClasses);
		$startComparisonButton->setId('action_button_compare');
		$buttonGroup->addElement($startComparisonButton);

		$row->add($buttonGroup);

		if (checkIfWriteAccessIsAllowed() && $isRestorable) {
			$row->addVerticalSpacer('1rem');
			$row->add(new htmlSubTitle(_('Restore')));
			$targetDn = extractDNSuffix(extractDNSuffix($dn));
			if (!empty($attributes['lastknownparent'][0]) && !str_contains($attributes['lastknownparent'][0], self::AD_DELETED_DELIMITER)) {
				$targetDn = $attributes['lastknownparent'][0];
			}
			$restoreTargetDn = new htmlResponsiveInputField(_('Target DN'), 'treeview-restore-dn', $targetDn);
			$restoreTargetDn->showDnSelection();
			$row->add($restoreTargetDn);
			$row->addVerticalSpacer('0.5rem');
			$restoreButton = new htmlButton('restorebutton', _('Restore'));
			$rdn = explode(self::AD_DELETED_DELIMITER, $dn)[0];
			$restoreButton->setOnClick('window.lam.treeview.restoreNode(\'' . getSecurityTokenName() . '\', '
				. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\', \'' . base64_encode(htmlspecialchars($rdn)) . '\', '
				. '\'' . addslashes(_('Restore')) . '\', \'' . addslashes(_('Cancel')) . '\', \'' . addslashes(_('Restore this entry')) . '\', \'' . addslashes(_('Ok')) . '\', '
				. '\'' . addslashes(_('Error')) . '\');');
			$restoreButton->setCSSClasses(['lam-secondary']);
			$row->add($restoreButton, 12, 12, 12, 'text-center');
		}
	}

	/**
	 * Adds the content part for one attribute.
	 *
	 * @param htmlResponsiveRow $row container where to add content
	 * @param string $attributeName attribute name
	 * @param string[] $values values
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @param string[] $objectClasses object classes
	 * @param string|null $dn DN
	 * @param string[] $highlighted list of highlighted attribute names
	 */
	private function addAttributeContent(htmlResponsiveRow $row, string $attributeName, array $values,
										 ?AttributeType    $schemaAttribute, array $objectClasses, ?string $dn,
										 array             $highlighted): void {
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
	 * @param string[]|null $values attribute values
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @return bool is multi-value
	 */
	private function isMultiValueAttribute(?array $values, ?AttributeType $schemaAttribute): bool {
		return (count($values) > 1) || (($schemaAttribute !== null) && ($schemaAttribute->getIsSingleValue() !== true));
	}

	/**
	 * Returns the input fields for the attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param string[] $values values
	 * @param bool $required is required
	 * @param bool $isMultiValue multi-value attribute
	 * @param bool $isMultiLine textarea attribute
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @param bool $readOnly render read-only
	 * @return htmlElement content
	 */
	private function getAttributeContentField(string $attributeName, array $values, bool $required, bool $isMultiValue,
											  bool   $isMultiLine, ?AttributeType $schemaAttribute, bool $readOnly = false): htmlElement {
		if (!$isMultiValue) {
			$field = $this->getAttributeInputField($attributeName, $values[0], $required, $isMultiLine, true, $schemaAttribute, 0, $readOnly);
			if (!$readOnly) {
				$field = $this->addExtraAttributeContent($field, $attributeName, $schemaAttribute);
			}
			return $field;
		}
		$valueListContents = [];
		$autoCompleteValues = [];
		$onInput = null;
		$safeAttributeName = htmlspecialchars(strtolower($attributeName));
		if ($safeAttributeName === 'objectclass') {
			$schemaObjectClasses = $this->getSchemaObjectClasses();
			foreach ($schemaObjectClasses as $schemaObjectClass) {
				if (!in_array_ignore_case($schemaObjectClass->getName(), $values)) {
					$autoCompleteValues[] = $schemaObjectClass->getName();
				}
			}
			$onInput = "window.lam.treeview.updatePossibleNewAttributes('" . getSecurityTokenName() . "', '" . getSecurityTokenValue() . "');";
		}
		foreach ($values as $index => $value) {
			$inputField = $this->getAttributeInputField($attributeName, $value, $required, $isMultiLine, false, $schemaAttribute, $index, $readOnly);
			$cssClasses = $inputField->getCSSClasses();
			$cssClasses[] = 'lam-attr-' . $safeAttributeName;
			$inputField->setCSSClasses($cssClasses);
			if ($inputField instanceof htmlInputField) {
				if (!empty($autoCompleteValues)) {
					$inputField->enableAutocompletion($autoCompleteValues);
					$inputField->setId($attributeName . $index);
				}
				if (($onInput !== null) && !$readOnly) {
					$inputField->setOnInput($onInput);
				}
			}
			$valueLine = new htmlTable();
			$valueLine->setCSSClasses(['fullwidth']);
			$valueLine->addElement($inputField);
			if (checkIfWriteAccessIsAllowed() && !$readOnly) {
				if (!$this->isJpegAttribute($attributeName, $schemaAttribute)) {
					$addButton = new htmlLink(null, '#', '../../graphics/add.svg');
					$addButton->setCSSClasses(['margin2']);
					$addButton->setOnClick('window.lam.treeview.addValue(event, this);');
					$valueLine->addElement($addButton);
				}
				if (!$this->isJpegAttribute($attributeName, $schemaAttribute) || ($value !== '')) {
					$clearButton = new htmlLink(null, '#', '../../graphics/del.svg');
					$clearButton->setCSSClasses(['margin2']);
					$clearButton->setOnClick('window.lam.treeview.clearValue(event, this);');
					$valueLine->addElement($clearButton);
				}
			}
			$valueListContents[] = $valueLine;
		}
		$listClasses = ['nowrap unstyled-list'];
		if ($this->isOrderedAttribute($values)) {
			$listId = 'attributeList_' . $safeAttributeName;
			$valueList = new htmlSortableList($valueListContents, $listId);
			$listClasses[] = 'tree-attribute-sorted-list';
			$valueList->setOnUpdate('function() {window.lam.treeview.updateAttributePositionData(\'' . $listId . '\');}');
		}
		else {
			$valueList = new htmlList($valueListContents, 'attributeList_' . $safeAttributeName);
		}
		$valueList->setCSSClasses($listClasses);
		if (!$readOnly) {
			$valueList = $this->addExtraAttributeContent($valueList, $attributeName, $schemaAttribute);
		}
		return $valueList;
	}

	/**
	 * Returns if the attribute has ordered values.
	 *
	 * @param string[] $values values
	 * @return bool is ordered
	 */
	private function isOrderedAttribute(array $values): bool {
		if (empty($values)) {
			return false;
		}
		$regex = '/^\{\d+\}/';
		foreach ($values as $value) {
			if (!preg_match($regex, $value)) {
				return false;
			}
		}
		return true;
	}

	/**
	 * Adds any additional entry content to the element.
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
			$upload->setCSSClasses(['image-upload']);

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
	 * @param bool $readOnly render read-only
	 * @return htmlElement input field
	 * @throws LAMException error generating field
	 */
	private function getAttributeInputField(string $attributeName, string $value, bool $required, bool $isMultiLine,
											bool   $isSingleValue, ?AttributeType $schemaAttribute, int $index, bool $readOnly): htmlElement {
		if ($this->isPasswordAttribute($attributeName)) {
			return $this->getAttributePasswordInputField($attributeName, $value, $required, $isSingleValue, $readOnly);
		}
		if ($this->isJpegAttribute($attributeName, $schemaAttribute)) {
			return $this->getAttributeJpegInputField($attributeName, $value, $index);
		}
		if (($schemaAttribute !== null) && $schemaAttribute->isBinary()) {
			$value = base64_encode($value);
		}
		$value = $this->preProcessValue(strtolower($attributeName), $value);
		if ($isMultiLine || $this->isPseudoMultiLine(strtolower($attributeName))) {
			if ($readOnly) {
				$text = new htmlOutputText(chunk_split($value, 40));
				$text->setPreformatted();
				return $text;
			}
			$inputField = new htmlInputTextarea('lam_attr_' . $attributeName, $value, 50, 5);
		}
		else {
			if ($readOnly) {
				$text = new htmlOutputText(chunk_split($value, 40));
				if (strlen($value) > 40) {
					$text->setPreformatted();
				}
				return $text;
			}
			$inputField = new htmlInputField('lam_attr_' . $attributeName, $value);
		}
		$inputField->addDataAttribute('value-orig', $value);
		$inputField->addDataAttribute('attr-name', $attributeName);
		$cssClass = ($isSingleValue) ? 'single-input' : 'multi-input';
		$inputField->setCSSClasses([$cssClass, 'attribute-field']);
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
		return stripos($attributeName, 'userpassword') === 0;
	}

	/**
	 * Returns if the given attribute is a JPG image.
	 *
	 * @param string $attributeName attribute name
	 * @param AttributeType|null $schemaAttribute schema attribute
	 * @return bool is password
	 */
	private function isJpegAttribute(string $attributeName, ?AttributeType $schemaAttribute): bool {
		if (stripos($attributeName, 'jpegphoto') === 0) {
			return true;
		}
		return ($schemaAttribute !== null) && ($schemaAttribute->getType() === 'JPEG');
	}

	/**
	 * Returns an input field for a password attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param string $value value
	 * @param bool $required required
	 * @param bool $isSingleValue is single value attribute
	 * @param bool $readOnly render read-only
	 * @return htmlElement input field
	 */
	private function getAttributePasswordInputField(string $attributeName, string $value, bool $required, bool $isSingleValue, bool $readOnly): htmlElement {
		if ($readOnly) {
			return new htmlOutputText('*******');
		}
		$inputField = new htmlInputField('lam_attr_' . $attributeName, $value);
		$inputField->addDataAttribute('value-orig', $value);
		$inputField->addDataAttribute('attr-name', $attributeName);
		$cssClass = ($isSingleValue) ? 'single-input' : 'multi-input';
		$inputField->setCSSClasses([$cssClass, 'attribute-field']);
		if ($required) {
			$inputField->setRequired(true);
		}
		$inputField->setIsPassword(true);
		$group = new htmlGroup();
		$table = new htmlTable();
		$table->addElement($inputField);
		$hashes = getSupportedHashTypes();
		$selectedHash = [getHashType($value)];
		$hashSelect = new htmlSelect('lam_hash_' . $attributeName, $hashes, $selectedHash);
		$hashSelect->addDataAttribute('attr-name', $attributeName);
		$hashSelect->setCSSClasses(['hash-select']);
		$table->addElement($hashSelect);
		$checkPassword = new htmlLink(null, '#', '../../graphics/light.svg');
		$checkPassword->setTitle(_('Check password'));
		$checkPassword->setOnClick("window.lam.treeview.checkPassword(event, this," .
			" '" . getSecurityTokenName() . "', '" . getSecurityTokenValue() . "'," .
			" '" . _('Check password') . "', '" . _('Check') . "', '" . _('Cancel') . "', '" . _('Ok') . "');");
		$table->addElement($checkPassword);
		$group->addElement($table);
		return $group;
	}

	/**
	 * Returns an input field for a JPG image attribute.
	 *
	 * @param string $attributeName attribute name
	 * @param string $value value
	 * @param int $index index
	 * @return htmlElement input field
	 * @throws LAMException error writing image file
	 */
	private function getAttributeJpegInputField(string $attributeName, string $value, int $index): htmlElement {
		$tempFilesManager = new LamTemporaryFilesManager();
		$fileName = $tempFilesManager->registerTemporaryFile('.jpg');
		$handle = $tempFilesManager->openTemporaryFileForWrite($fileName);
		fwrite($handle, $value);
		fclose($handle);
		$image = new htmlImage($tempFilesManager->getResourceLink($fileName));
		$image->enableLightbox();
		$image->setCSSClasses(['thumbnail', 'image-input', 'attribute-field']);
		$image->addDataAttribute('attr-name', $attributeName);
		$image->addDataAttribute('index', (string) $index);
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
		$knownAttributes = ['postalAddress1', 'homePostalAddress', 'personalSignature', 'description', 'mailReplyText'];
		if (in_array_ignore_case($attributeName, $knownAttributes)) {
			return true;
		}
		if ($schemaAttribute === null) {
			return false;
		}
		$knownSyntaxOIDs = [
			// octet string syntax OID:
			'1.3.6.1.4.1.1466.115.121.1.40',
			// postal address syntax OID:
			'1.3.6.1.4.1.1466.115.121.1.41'];
		return in_array($schemaAttribute->getSyntaxOID(), $knownSyntaxOIDs);
	}

	/**
	 * Returns if the attribute is required for the given list of object classes.
	 *
	 * @param ObjectClass[] $schemaObjectClasses object class definitions
	 * @param AttributeType $schemaAttribute schema attribute
	 * @param string[] $objectClasses list of object classes
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
			if (!empty($schemaObjectClass->getSupClasses())
				&& $this->isAttributeRequired($schemaObjectClasses, $schemaAttribute, $schemaObjectClass->getSupClasses())) {
				return true;
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
		$row->add(new htmlSubTitle(_('Internal attributes')));
		$attributes = ldapGetDN($dn, ['+', 'creatorsName', 'createTimestamp', 'modifiersName',
			'modifyTimestamp', 'hasSubordinates', 'pwdChangedTime', 'passwordRetryCount', 'accountUnlockTime', 'nsAccountLock',
			'nsRoleDN', 'passwordExpirationTime']);
		unset($attributes['dn']);
		ksort($attributes);
		foreach ($attributes as $attributeName => $values) {
			$row->addLabel(new htmlOutputText($this->getProperAttributeName($attributeName)));
			$row->addField(new htmlOutputText(implode(', ', $values)));
			$row->addVerticalSpacer('0.5rem');
		}
		ob_start();
		parseHtml(null, $row, [], false);
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(['content' => $content]);
	}

	/**
	 * Returns the node content with the attribute listing.
	 *
	 * @return string JSON
	 */
	private function saveAttributes(string $dn): string {
		$schemaAttributes = $this->getSchemaAttributes();
		$attributes = ldapGetDN($dn, ['*']);
		unset($attributes['dn']);
		$attributes = array_change_key_case($attributes);
		$changes = json_decode($_POST['changes'], true);
		$changes = array_change_key_case($changes);
		logNewMessage(LOG_DEBUG, 'LDAP changes for ' . $dn . ': ' . print_r($changes, true));
		$ldapChanges = [];
		foreach ($changes as $attrName => $change) {
			$schemaAttribute = $schemaAttributes[$attrName] ?? null;
			if (isset($change['new'])) {
				$newValues = $change['new'];
				if (isset($change['hash'])) {
					$newValues = $this->applyPasswordHash($newValues, $change['hash']);
				}
				elseif (($schemaAttribute !== null) && $schemaAttribute->isBinary()) {
					$newValues = $this->decodeBinaryAttributeValues($newValues);
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
					if (($schemaAttribute !== null) && $schemaAttribute->isBinary()) {
						$oldValues = $this->decodeBinaryAttributeValues($oldValues);
					}
					$ldapChanges[$attrName] = array_values($oldValues);
				}
			}
			if (!empty($change['upload'])) {
				if (!isset($ldapChanges[$attrName]) && !empty($attributes[$attrName])) {
					$ldapChanges[$attrName] = $attributes[$attrName];
				}
				$oldValues = empty($attributes[$attrName]) ? [] : $attributes[$attrName];
				if ($this->isMultiValueAttribute($oldValues, $schemaAttribute)) {
					$ldapChanges[$attrName][] = base64_decode($change['upload']);
				}
				else {
					$ldapChanges[$attrName][0] = base64_decode($change['upload']);
				}
			}
		}
		$message = new htmlStatusMessage('INFO', _('You made no changes'));
		$jsonData = [];
		// rename DN if RDN attribute changed
		$rdnAttribute = strtolower(extractRDNAttribute($dn));
		$rdnValue = extractRDNValue($dn);
		if (isset($ldapChanges[$rdnAttribute][0]) && !in_array($rdnValue, $ldapChanges[$rdnAttribute])) {
			$pos = 0;
			$oldPos = array_search($rdnValue, $changes[$rdnAttribute]['old']);
			if (($oldPos !== false) && isset($ldapChanges[$rdnAttribute][$oldPos])) {
				$pos = $oldPos;
			}
			$newRdnValue = $ldapChanges[$rdnAttribute][$pos];
			$newRdn = $rdnAttribute . '=' . ldap_escape($newRdnValue, '', LDAP_ESCAPE_DN);
			$parent = extractDNSuffix($dn);
			$renameOk = ldap_rename($_SESSION['ldap']->server(), $dn, $newRdn, $parent, $_SESSION['ldap']->isActiveDirectory());
			$newDn = $newRdn . ',' . $parent;
			if ($renameOk) {
				logNewMessage(LOG_DEBUG, 'Renamed ' . $dn . ' to ' . $newDn);
				$dn = $newDn;
				$jsonData['newDn'] = base64_encode($newDn);
				$jsonData['parent'] = base64_encode($parent);
				unset($ldapChanges[$rdnAttribute]);
				$message = new htmlStatusMessage('INFO', _('All changes were successful.'));
			}
			else {
				logNewMessage(LOG_ERR, 'Renaming ' . $dn . ' to ' . $newDn . ' failed: ' . getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
				$message = new htmlStatusMessage('ERROR', sprintf(_('Was unable to rename DN: %s.'), $dn), getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
				// skip other changes
				$ldapChanges = [];
			}
		}
		if (!empty($ldapChanges)) {
			$this->handleSpecialAttributes($dn, $ldapChanges, $attributes, $schemaAttributes);
			logNewMessage(LOG_DEBUG, 'LDAP changes to DN ' . $dn . ': ' . print_r($ldapChanges, true));
			$saved = ldap_modify($_SESSION['ldap']->server(), $dn, $ldapChanges);
			if ($saved) {
				$message = new htmlStatusMessage('INFO', _('All changes were successful.'));
			}
			else {
				$message = new htmlStatusMessage('ERROR', sprintf(_('Was unable to modify attributes of DN: %s.'), $dn), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
			}
		}
		ob_start();
		parseHtml(null, $message, [], true);
		$messageContent = ob_get_contents();
		ob_end_clean();
		$jsonData['result'] = $messageContent;
		return json_encode($jsonData);
	}

	/**
	 * Handles attributes that require specific operations.
	 *
	 * @param string $dn DN
	 * @param array<string, string[]|string> $ldapChanges changes
	 * @param array<string, string[]|string> $attributes original attributes
	 * @param array<string, AttributeType> $schemaAttributes schema definition for attributes
	 */
	private function handleSpecialAttributes(string $dn, array &$ldapChanges, array $attributes, array $schemaAttributes): void {
		// olcModuleLoad is add-only
		if (isset($ldapChanges['olcmoduleload']) && isset($attributes['olcmoduleload'])) {
			$newModules = array_delete($attributes['olcmoduleload'], $ldapChanges['olcmoduleload']);
			unset($ldapChanges['olcmoduleload']);
			if (!empty($newModules)) {
				$isOk = ldap_mod_add($_SESSION['ldap']->server(), $dn, ['olcmoduleload' => $newModules]);
				if (!$isOk) {
					logNewMessage(LOG_ERR, 'Changing olcmoduleload at ' . $dn . ' failed: ' . getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
				}
			}
		}
		// olcAccess needs removal of newlines
		if (isset($ldapChanges['olcaccess'])) {
			for ($i = 0; $i < count($ldapChanges['olcaccess']); $i++) {
				$ldapChanges['olcaccess'][$i] = str_replace("\n", ' ', $ldapChanges['olcaccess'][$i]);
			}
		}
		// add ";binary" to binary attributes
		foreach ($ldapChanges as $attributeName => $value) {
			if (!isset($schemaAttributes[$attributeName])) {
				continue;
			}
			$schemaAttribute = $schemaAttributes[$attributeName];
			if ($schemaAttribute->isBinary()
				&& (!str_contains($attributeName, ';binary'))
				&& ($schemaAttribute->getSyntaxOID() === '1.3.6.1.4.1.1466.115.121.1.8')) {
				$ldapChanges[$attributeName . ';binary'] = $value;
				unset($ldapChanges[$attributeName]);
			}
		}
	}

	/**
	 * Base 64 decodes attribute values.
	 *
	 * @param string[] $encoded encoded data
	 * @return string[] binary data
	 */
	private function decodeBinaryAttributeValues(array $encoded): array {
		$binaryValues = [];
		foreach ($encoded as $value) {
			$decoded = base64_decode($value, true);
			$binaryValues[] = ($decoded !== false) ? $decoded : $value;
		}
		return $binaryValues;
	}

	/**
	 * Applies password hashing on the provided values.
	 *
	 * @param string[] $values values
	 * @param string[] $hash hash types
	 * @return string[] hashed values
	 */
	private function applyPasswordHash(array $values, array $hash): array {
		$result = [];
		for ($i = 0; $i < count($values); $i++) {
			$oldType = getHashType($values[$i]);
			$result[] = (($oldType === 'PLAIN') && ($hash[$i] !== 'PLAIN')) ? pwd_hash($values[$i], true, $hash[$i]) : $values[$i];
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
		return '';
	}

	/**
	 * Returns the content to select the object classes.
	 *
	 * @param string $dn DN
	 * @param string|null $errorMessage error if any
	 * @return string JSON data
	 */
	private function createNewNodeGetObjectClassesStep(string $dn, ?string $errorMessage = null): string {
		$row = new htmlResponsiveRow();
		$row->add(new htmlTitle(_('Create a child entry')));
		if ($errorMessage !== null) {
			$row->add(new htmlStatusMessage('ERROR', $errorMessage));
		}
		$row->addLabel(new htmlOutputText(_('Parent')));
		$row->addField(new htmlOutputText($dn));
		$row->addVerticalSpacer('1rem');
		$schemaObjectClasses = $this->getSchemaObjectClasses();
		$objectClassOptions = [];
		$selectCssClasses = [];
		foreach ($schemaObjectClasses as $schemaObjectClass) {
			$name = $schemaObjectClass->getName();
			$objectClassOptions[$name] = $name;
			if ($schemaObjectClass->getType() === 'structural') {
				$selectCssClasses[$name] = 'bold';
			}
		}
		$objectClassSelect = new htmlResponsiveSelect('objectClasses', $objectClassOptions, [], _('Object classes'), null, 10);
		$objectClassSelect->setHasDescriptiveElements(true);
		$objectClassSelect->setMultiSelect(true);
		$objectClassSelect->setOptionCssClasses($selectCssClasses);
		$row->add($objectClassSelect);
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
		$nextButton->setCSSClasses(['lam-primary']);
		$nextButton->setOnClick('window.lam.treeview.createNodeSelectObjectClassesStep(event, \'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');');
		$row->add($nextButton, 12, 12, 12, 'text-center');
		$row->addVerticalSpacer('2rem');
		$row->add(new htmlOutputText(_('Hint: You must choose exactly one structural object class (shown in bold above)')));
		$row->add(new htmlHiddenInput('parentDn', base64_encode($dn)));
		ob_start();
		parseHtml(null, $row, [], false);
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(['content' => $content]);
	}

	/**
	 * Returns the content to select the object classes.
	 *
	 * @param string $dn DN
	 * @param string|null $errorMessage error if any
	 * @param string|null $rdnAttribute RDN attribute name
	 * @param array<string, string[]|string>|null $attributes attribute values
	 * @return string JSON data
	 */
	private function createNewNodeCheckObjectClassesStep(string $dn, ?string $errorMessage = null, ?string $rdnAttribute = null, ?array $attributes = null): string {
		$objectClasses = ($attributes === null) ? explode(',', $_POST['objectClasses']) : $attributes['objectClass'];
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
		$mustAttributes = [];
		$mayAttributes = [];
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
		$rdnOptions = [];
		foreach ($allAttributes as $attribute) {
			$rdnOptions[] = $attribute->getName();
		}
		$rdnOptionsSelected = [];
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
			$values = empty($attributes[$mustAttribute->getName()]) ? [''] : $attributes[$mustAttribute->getName()];
			$this->addAttributeContent($row, $mustAttribute->getName(), $values, $mustAttribute, $objectClasses, null, []);
		}
		if (!empty($mayAttributes)) {
			$row->add(new htmlSubTitle(_('Optional attributes')));
		}
		foreach ($mayAttributes as $mayAttribute) {
			$values = empty($attributes[$mayAttribute->getName()]) ? [''] : $attributes[$mayAttribute->getName()];
			$this->addAttributeContent($row, $mayAttribute->getName(), $values, $mayAttribute, $objectClasses, null, []);
		}
		$row->addVerticalSpacer('1rem');
		$nextButton = new htmlButton('save', _('Create'));
		$nextButton->setCSSClasses(['lam-primary']);
		$nextButton->setOnClick('window.lam.treeview.createNodeEnterAttributesStep(event, \'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\');');
		$row->add($nextButton, 12, 12, 12, 'text-center');
		$row->add(new htmlHiddenInput('objectClasses', implode(',', $objectClasses)));
		$row->add(new htmlHiddenInput('parentDn', base64_encode($dn)));
		ob_start();
		parseHtml(null, $row, [], false);
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(['content' => $content]);
	}

	/**
	 * Gets a recursive list of must attribute names.
	 *
	 * @param ObjectClass[] $objectClasses schema object classes
	 * @param string $objectClass object class
	 * @return string[] attribute names
	 */
	private function getMustAttributeNamesRecursive(array $objectClasses, string $objectClass): array {
		$objectClassLower = strtolower($objectClass);
		$objectClassObject = $objectClasses[$objectClassLower];
		$attributeNames = $objectClassObject->getMustAttrNames();
		foreach ($objectClassObject->getSupClasses() as $superClass) {
			$attributeNames = array_merge($attributeNames, $this->getMustAttributeNamesRecursive($objectClasses, $superClass));
		}
		$attributeNames = array_map(strtolower(...), $attributeNames);
		return array_unique($attributeNames);
	}

	/**
	 * Gets a recursive list of may attribute names.
	 *
	 * @param ObjectClass[] $objectClasses schema object classes
	 * @param string $objectClass object class
	 * @return string[] attribute names
	 */
	private function getMayAttributeNamesRecursive(array $objectClasses, string $objectClass): array {
		$objectClassLower = strtolower($objectClass);
		$objectClassObject = $objectClasses[$objectClassLower];
		$attributeNames = $objectClassObject->getMayAttrNames();
		foreach ($objectClassObject->getSupClasses() as $superClass) {
			$attributeNames = array_merge($attributeNames, $this->getMayAttributeNamesRecursive($objectClasses, $superClass));
		}
		$attributeNames = array_map(strtolower(...), $attributeNames);
		return array_unique($attributeNames);
	}

	/**
	 * Returns the content for the check attributes step of node creation.
	 *
	 * @param string $dn DN
	 * @return string JSON data
	 */
	private function createNewNodeCheckAttributesStep(string $dn): string {
		$objectClasses = $_POST['objectClasses'];
		$rdnAttribute = $_POST['rdn'];
		$attributeChanges = json_decode($_POST['attributes'], true);
		$attributes = ['objectClass' => explode(',', $objectClasses)];
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
			return json_encode(['errors' => $errors]);
		}
		return json_encode([]);
	}

	/**
	 * Restores a node in LDAP.
	 *
	 * @param string $dn DN
	 * @return string JSON
	 */
	private function restoreNode(string $dn): string {
		$rdn = explode(self::AD_DELETED_DELIMITER, $dn)[0];
		$targetDn = $_POST['targetDn'];
		$changes = [
			[
				'attrib' => 'isDeleted',
				'modtype' => LDAP_MODIFY_BATCH_REMOVE_ALL
			],
			[
				'attrib' => 'distinguishedName',
				'modtype' => LDAP_MODIFY_BATCH_REPLACE,
				'values' => [$rdn . ',' . $targetDn]
			],
		];
		$success = ldap_modify_batch(getLDAPServerHandle(), $dn, $changes, getCommonLdapControls());
		if ($success) {
			return json_encode([]);
		}
		$error = getDefaultLDAPErrorString(getLDAPServerHandle());
		logNewMessage(LOG_ERR, 'Tree view restore node failed: ' . $error);
		return json_encode([
			'errorTitle' => htmlspecialchars(sprintf(_('Was unable to restore %s.'), $rdn)),
			'errorText' => $error
		]);
	}

	/**
	 * Stops processing if DN is invalid.
	 *
	 * @param string $dn DN
	 * @return bool DN is valid
	 */
	private function validateDn(string $dn, bool $die = true): bool {
		$dn = strtolower($dn);
		$rootDns = TreeViewTool::getRootDns();
		foreach ($rootDns as $rootDn) {
			$rootDn = strtolower($rootDn);
			if (str_ends_with($dn, $rootDn)) {
				return true;
			}
		}
		if (!$die) {
			return false;
		}
		logNewMessage(LOG_ERR, 'Invalid DN for tree view: ' . $dn);
		die();
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
	 * @return AttributeType[] attributes
	 */
	private function getSchemaAttributes(): array {
		if ($this->schemaAttributes === null) {
			$this->schemaAttributes = get_schema_attributes();
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
		$scopeOptions = [
			_('Sub (entire subtree)') => 'sub',
			_('One (one level beneath base)') => 'one',
		];
		$scopeSelect = new htmlResponsiveSelect('scope', $scopeOptions, [], _('Search scope'));
		$scopeSelect->setSortElements(false);
		$scopeSelect->setHasDescriptiveElements(true);
		$row->add($scopeSelect);
		$filterInput = new htmlResponsiveInputField(_('Search filter'), 'filter', '(objectClass=*)', null, true);
		$row->add($filterInput);
		$row->add(new htmlResponsiveInputField(_('Attributes'), 'attributes', 'cn, givenName, sn, uid', null, true));
		$row->add(new htmlResponsiveInputField(_('Order by'), 'orderBy', 'dn'));
		$resultCountInput = new htmlResponsiveInputField(_('LDAP search limit'), 'limit', '50');
		$resultCountInput->setMinimumAndMaximumNumber();
		$row->add($resultCountInput);
		$displayFormat = [
			_('List') => 'list',
			_('Table') => 'table'
		];
		$formatSelect = new htmlResponsiveSelect('format', $displayFormat, ['list'], _('Display format'));
		$formatSelect->setHasDescriptiveElements(true);
		$row->add($formatSelect);
		$row->addVerticalSpacer('2rem');
		$nextButton = new htmlButton('search', _('Search'));
		$nextButton->setCSSClasses(['lam-primary']);
		$nextButton->setOnClick('window.lam.treeview.searchResults(event, \'' . getSecurityTokenName() . '\', \'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
		$row->add($nextButton, 12, 12, 12, 'text-center');
		ob_start();
		parseHtml(null, $row, [], false);
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(['content' => $content]);
	}

	/**
	 * Renders the search results.
	 *
	 * @param string $dn DN
	 * @return string JSON
	 */
	private function searchResults(string $dn): string {
		$scope = $_POST['scope'];
		if (!in_array($scope, ['sub', 'one'])) {
			logNewMessage(LOG_ERR, 'Invalid search scope: ' . $scope);
			die();
		}
		$format = $_POST['format'];
		if (!in_array($format, ['list', 'table'])) {
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
		$results = match ($scope) {
			'sub' => searchLDAP($dn, $filter, $searchAttributes, $limit),
			default => ldapListDN($dn, $filter, $searchAttributes, null, $limit),
		};
		usort($results, compareByAttributes(...));
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
	 * @param array<string, string[]|string>[] $results results
	 * @param string[] $attributes attribute list to show
	 * @param htmlResponsiveRow $row content
	 * @return string JSON
	 */
	private function searchResultsAsList(array $results, array $attributes, htmlResponsiveRow $row): string {
		foreach ($results as $result) {
			$row->add(new htmlSubTitle(getAbstractDN($result['dn']), $this->getNodeIcon($result)));
			$row->addLabel(new htmlOutputText('dn'));
			$row->addField(new htmlLink(unescapeLdapSpecialCharacters($result['dn']), 'treeView.php?dn=' . base64_encode($result['dn'])));
			$result = array_change_key_case($result);
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
		parseHtml(null, $row, [], false);
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(['content' => $content]);
	}

	/**
	 * Returns the search results as table.
	 *
	 * @param array<string, string[]|string>[] $results results
	 * @param string[] $attributes attribute list to show
	 * @param htmlResponsiveRow $row content
	 * @return string JSON
	 */
	private function searchResultsAsTable(array $results, array $attributes, htmlResponsiveRow $row) {
		$titles = array_merge(['', 'dn'], $attributes);
		$data = [];
		foreach ($results as $result) {
			$dataEntry = [$this->getNodeIcon($result), new htmlLink(unescapeLdapSpecialCharacters($result['dn']), 'treeView.php?dn=' . base64_encode($result['dn']))];
			$result = array_change_key_case($result);
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
		$table->setCSSClasses(['colored--table']);
		$row->add($table);
		ob_start();
		parseHtml(null, $row, [], false);
		$content = ob_get_contents();
		ob_end_clean();
		return json_encode(['content' => $content]);
	}

	/**
	 * Performs paste operations.
	 *
	 * @param string $dn DN to paste
	 * @return string JSON data
	 */
	private function paste(string $dn): string {
		$targetDn = base64_decode($_POST['targetDn']);
		$this->validateDn($targetDn);
		$action = $_POST['action'];
		if (!in_array($action, ['COPY', 'CUT'])) {
			logNewMessage(LOG_ERR, 'Invalid tree paste action: ' . $action);
			die();
		}
		try {
			if (strtolower($dn) === strtolower($targetDn)) {
				throw new LAMException(_('Entry cannot be inserted into itself.'));
			}
			$entryAndChildren = ldapListDN($dn);
			$childrenCount = count($entryAndChildren);
			logNewMessage(LOG_DEBUG, 'Paste operation for entry with ' . $childrenCount . ' child entries.');
			if (($childrenCount === 0) && ($action === 'CUT')) {
				// do LDAP move for entries without children and CUT operation
				moveDn($dn, $targetDn);
				return json_encode([]);
			}
			copyDnRecursive($dn, $targetDn);
			if ($action === 'CUT') {
				$errors = deleteDN($dn, true);
				if (!empty($errors)) {
					$row = new htmlResponsiveRow();
					foreach ($errors as $error) {
						$row->add(new htmlStatusMessage($error[0], $error[1], $error[2] ?? null));
					}
					ob_start();
					parseHtml(null, $row, [], false);
					$content = ob_get_contents();
					ob_end_clean();
					return json_encode(['error' => $content]);
				}
			}
		}
		catch (LAMException $e) {
			$message = new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage());
			ob_start();
			parseHtml(null, $message, [], false);
			$content = ob_get_contents();
			ob_end_clean();
			return json_encode(['error' => $content]);
		}
		return json_encode([]);
	}

	/**
	 * Starts the DN comparison.
	 *
	 * @return string JSON data
	 */
	private function compare(): string {
		$dnListToCheck = explode(',', $_POST['dnList']);
		$dnList = [];
		foreach ($dnListToCheck as $dn) {
			$dn = base64_decode($dn);
			if ($this->validateDn($dn, false)) {
				$dnList[] = $dn;
			}
		}
		natcasesort($dnList);
		try {
			$row = new htmlResponsiveRow();
			$row->add(new htmlTitle(_('Comparison')));
			$attributeData = [];
			foreach ($dnList as $dn) {
				$attributes = ldapGetDN($dn, ['*']);
				unset($attributes['dn']);
				$attributeData[$dn] = $attributes;
			}
			$this->addCompareArea($row, $attributeData, _('Attributes'), true);
			$attributeData = [];
			foreach ($dnList as $dn) {
				$attributes = ldapGetDN($dn, ['+', 'creatorsName', 'createTimestamp', 'modifiersName',
					'modifyTimestamp', 'hasSubordinates', 'pwdChangedTime', 'passwordRetryCount', 'accountUnlockTime', 'nsAccountLock',
					'nsRoleDN', 'passwordExpirationTime']);
				unset($attributes['dn']);
				$attributeData[$dn] = $attributes;
			}
			$this->addCompareArea($row, $attributeData, _('Internal attributes'), false);
			ob_start();
			parseHtml(null, $row, [], false);
			$content = ob_get_contents();
			ob_end_clean();
			return json_encode(['content' => $content]);
		}
		catch (LAMException $e) {
			logNewMessage(LOG_ERR, $e->getMessage());
			$message = new htmlStatusMessage('ERROR', _('Comparison failed'));
			ob_start();
			parseHtml(null, $message, [], false);
			$content = ob_get_contents();
			ob_end_clean();
			return json_encode(['content' => $content]);
		}
	}

	/**
	 * Adds a comparison area.
	 *
	 * @param htmlResponsiveRow $row row
	 * @param array<string, array<string, string[]|string>> $attributeData dn => attributes
	 * @param string $title table title
	 * @param bool $showActions show action icons
	 */
	private function addCompareArea(htmlResponsiveRow $row, array $attributeData, string $title, bool $showActions): void {
		$attributeNames = [];
		$labels = [];
		foreach ($attributeData as $dn => $data) {
			$attributeNames = array_merge($attributeNames, array_keys($data));
			$labels[] = $dn;
		}
		array_unshift($labels, _('Attribute'));
		$attributeNames = array_values(array_unique($attributeNames));
		natcasesort($attributeNames);
		$data = [];
		$differentValueIndexes = [];
		$schemaAttributes = $this->getSchemaAttributes();
		for ($i = 0; $i < count($attributeNames); $i++) {
			$attributeName = $attributeNames[$i];
			$dataEntry = [new htmlOutputText($this->getProperAttributeName($attributeName))];
			$allValues = [];
			foreach ($attributeData as $singleData) {
				$schemaAttribute = $schemaAttributes[$attributeName] ?? null;
				$attributeNameSchema = ($schemaAttribute === null) ? $attributeName : $schemaAttribute->getName();
				$values = $singleData[$attributeName] ?? [''];
				$isMultiValue = $this->isMultiValueAttribute($values, $schemaAttribute);
				$isMultiLine = $this->isMultiLineAttribute($attributeName, $schemaAttribute);
				$dataEntry[] = $this->getAttributeContentField($attributeNameSchema, $values, false, $isMultiValue, $isMultiLine, $schemaAttribute, true);
				$allValues[] = implode(', ', $values);
			}
			$data[] = $dataEntry;
			$allValues = array_unique($allValues);
			if (count($allValues) > 1) {
				$differentValueIndexes[] = $i;
			}
		}
		if ($showActions) {
			$actionsRow = [new htmlOutputText('')];
			foreach (array_keys($attributeData) as $dn) {
				$buttonClasses = ['clickable', 'lam-margin-small', 'icon'];
				$removeFromComparisonButton = new htmlImage('../../graphics/list-remove.svg', '16px', '16px', _('Remove from comparison'), _('Remove from comparison'));
				if (count($attributeData) > 2) {
					$removeFromComparisonButton->setOnClick('window.lam.treeview.removeFromComparison(\'' . base64_encode($dn) . '\'); window.lam.treeview.openComparison(\'' . getSecurityTokenName() . '\', '
						. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
				}
				else {
					$removeFromComparisonButton->setOnClick('window.lam.treeview.removeFromComparison(\'' . base64_encode($dn) . '\'); window.lam.treeview.getNodeContent(\'' . getSecurityTokenName() . '\', '
						. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
				}
				$removeFromComparisonButton->setCSSClasses($buttonClasses);
				$linkButton = new htmlImage('../../graphics/link.svg', '16px', '16px', _('Open entry'), _('Open entry'));
				$linkButton->setOnClick('window.lam.treeview.getNodeContent(\'' . getSecurityTokenName() . '\', '
					. '\'' . getSecurityTokenValue() . '\', \'' . base64_encode($dn) . '\');');
				$linkButton->setCSSClasses($buttonClasses);
				$actionsGroup = new htmlGroup();
				$actionsGroup->addElement($removeFromComparisonButton);
				$actionsGroup->addElement($linkButton);
				$actionsRow[] = $actionsGroup;
			}
			$data[] = $actionsRow;
		}
		$attributeTable = new htmlResponsiveTable($labels, $data, $differentValueIndexes);
		$attributeTable->setCSSClasses(['colored--table', 'no-wrap-first-column']);
		$row->add(new htmlSubTitle($title));
		$row->add($attributeTable);
	}

	/**
	 * Returns if the attribute should be rendered multi-line even if it is single-line defined in schema.
	 *
	 * @param string $attributeName attribute name
	 * @return bool render multi-line
	 */
	private function isPseudoMultiLine(string $attributeName): bool {
		return ($attributeName === 'olcaccess');
	}

	/**
	 * Preprocesses the value before rendering.
	 *
	 * @param string $attributeName attribute name
	 * @param string $value value
	 * @return string preprocessed value
	 */
	private function preProcessValue(string $attributeName, string $value): string {
		if ($attributeName === 'olcaccess') {
			return preg_replace('/\s+by\s+/', "\r\nby ", $value);
		}
		return $value;
	}

}

/**
 * Compares two nodes by interpreting their ID as DN.
 *
 * @param array<string, string> $a first node
 * @param array<string, string> $b second node
 * @return int result
 */
function compareNodeByIdAsDn($a, $b): int {
	return strnatcasecmp(extractRDN(base64_decode($a['key'])), extractRDN(base64_decode($b['key'])));
}

/**
 * Compares two arrays with LDAP attributes by global $lamOrderByAttribute.
 *
 * @param array<string, string[]|string> $a first node
 * @param array<string, string[]|string> $b second node
 * @return int result
 */
function compareByAttributes($a, $b): int {
	global $lamOrderByAttribute;
	if ($lamOrderByAttribute === 'dn') {
		return compareDN($a['dn'], $b['dn']);
	}
	$a = array_change_key_case($a);
	$b = array_change_key_case($b);
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
