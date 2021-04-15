<?php
namespace LAM\TOOLS\TREEVIEW;
use htmlDiv;
use htmlLink;
use htmlOutputText;
use htmlResponsiveRow;
use htmlSubTitle;
use htmlTitle;
use function LAM\SCHEMA\get_schema_attributes;


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
		$dn = base64_decode($_POST['dn']);
		switch ($command) {
			case 'getNodes':
				return $this->getNodes($dn);
				break;
			case 'getNodeContent':
				return $this->getNodeContent($dn);
				break;
			case 'getInternalAttributesContent':
				return $this->getInternalAttributesContent($dn);
				break;
			case 'deleteNode':
				return $this->deleteNode($dn);
				break;
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
			'id' => $attributes['dn'],
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
		$this->validateDn($dn);
		$row = new htmlResponsiveRow();
		$row->add(new htmlTitle($dn), 12);
		$internalAttributesContent = new htmlResponsiveRow();
		$internalAttributesContent->add(new htmlSubTitle(_('Internal attributes')), 12);
		$internalAttributesButton = new htmlLink(_('Show internal attributes'), '#', null, true);
		$internalAttributesButton->setOnClick('window.lam.treeview.getInternalAttributesContent(event,
			"' . getSecurityTokenName() . '",
			"' . getSecurityTokenValue() . '",
			"' . base64_encode($dn) . '");');
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
	 * Returns the internal attributes.
	 *
	 * @param string $dn DN
	 */
	private function getInternalAttributesContent(string $dn) {
		$this->validateDn($dn);
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
	 * Deletes a node in LDAP.
	 *
	 * @param string $dn DN
	 * @return string JSON
	 */
	private function deleteNode(string $dn): string {
		$this->validateDn($dn);
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

	private function getSchemaAttributes(): array {
		if ($this->schemaAttributes === null) {
			$this->schemaAttributes = get_schema_attributes(null);
		}
		return $this->schemaAttributes;
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
	return strnatcasecmp(extractRDN($a['id']), extractRDN($b['id']));
}
