<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013  Roland Gruber

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
* Manages Windows AD (e.g. Samba 4) hosts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Windows AD (e.g. Samba 4) hosts.
*
* @package modules
*/
class windowsHost extends baseModule {
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'samba.png';
		// manages host accounts
		$return["account_types"] = array('host');
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=computer)");
		// alias name
		$return["alias"] = _("Windows");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('computer');
		// managed attributes
		$return['attributes'] = array('cn', 'description', 'location', 'sAMAccountName', 'managedBy', 'operatingSystem', 'operatingSystemVersion', 'dNSHostName');
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _('Host name'), 'attr' => 'cn, sAMAccountName',
				"Text" => _('Please enter the host\'s name.')
			),
			'description' => array(
				"Headline" => _('Description'), 'attr' => 'description',
				"Text" => _('Please enter a descriptive text for this host.')
			),
			'location' => array(
				"Headline" => _('Location'), 'attr' => 'location',
				"Text" => _('This is the host\'s location (e.g. Munich, server room 3).')
			),
			'managedBy' => array(
				"Headline" => _('Managed by'), 'attr' => 'managedBy',
				"Text" => _('The host is managed by this contact person.')
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'windowsHost_name',
				'description' => _('Host name'),
				'help' => 'cn',
				'example' => _('PC01'),
				'required' => true
			),
			array(
				'name' => 'windowsHost_description',
				'description' => _('Description'),
				'help' => 'description',
			),
			array(
				'name' => 'windowsHost_location',
				'description' => _('Location'),
				'help' => 'location',
				'example' => _('MyCity'),
			),
			array(
				'name' => 'windowsHost_managedBy',
				'description' => _('Managed by'),
				'help' => 'managedBy',
				'example' => 'cn=user1,o=test',
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('Host name'),
			'description' => _('Description'),
			'location' => _('Location'),
			'managedBy' => _('Managed by'),
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	public function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('Host name'), _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' windowsHost_cn', _('Host name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$container = new htmlTable();
		$this->addSimpleInputTextField($container, 'cn', _('Host name'), true);
		$this->addSimpleInputTextField($container, 'description', _('Description'), false);
		$this->addSimpleInputTextField($container, 'location', _('Location'), false);
		// managed by
		$container->addElement(new htmlOutputText(_('Managed by')));
		$managedBy = '-';
		if (isset($this->attributes['managedBy'][0])) {
			$managedBy = $this->attributes['managedBy'][0];
		}
		$container->addElement(new htmlOutputText(getAbstractDN($managedBy)));
		$container->addElement(new htmlHelpLink('managedBy'), true);
		$container->addElement(new htmlOutputText(''));
		$managedByButtons = new htmlGroup();
		$managedByButtons->addElement(new htmlAccountPageButton(get_class($this), 'managedBy', 'edit', _('Change')));
		if (isset($this->attributes['managedBy'][0])) {
			$managedByButtons->addElement(new htmlSpacer('5px', null));
			$managedByButtons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'removeManagedBy', _('Remove')));
		}
		$container->addElement($managedByButtons, true);
		return $container;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$return = array();
		// cn
		$this->attributes['cn'][0] = $_POST['cn'];
		$this->attributes['sAMAccountName'][0] = $_POST['cn'] . '$';
		if (!get_preg($_POST['cn'], 'hostname')) {
			$return[] = $this->messages['cn'][0];
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// location
		$this->attributes['location'][0] = $_POST['location'];
		// managed by
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_removeManagedBy'])) {
			unset($this->attributes['managedBy']);
		}
		return $return;
	}
	
	/**
	* This function will create the meta HTML code to show a page to change the member attribute.
	* 
	* @return htmlElement HTML meta data
	*/
	function display_html_managedBy() {
		$return = new htmlTable();
		// show possible managers
		$options = array();
		$filter = get_ldap_filter('user');
		$entries = searchLDAPByFilter($filter, array('dn'), array('user'));
		for ($i = 0; $i < sizeof($entries); $i++) {
			$entries[$i] = $entries[$i]['dn'];
		}
		// sort by DN
		usort($entries, 'compareDN');
		for ($i = 0; $i < sizeof($entries); $i++) {
			$options[getAbstractDN($entries[$i])] = $entries[$i];
		}
		$selected = array();
		if (isset($this->attributes['managedBy'][0])) {
			$selected = array($this->attributes['managedBy'][0]);
			if (!in_array($selected[0], $options)) {
				$options[getAbstractDN($selected[0])] = $selected[0];
			}
		}
		$membersSelect = new htmlSelect('managedBy', $options, $selected);
		$membersSelect->setHasDescriptiveElements(true);
		$membersSelect->setRightToLeftTextDirection(true);
		$membersSelect->setSortElements(false);
		$membersSelect->setTransformSingleSelect(false);
		$return->addElement($membersSelect, true);
		$buttonTable = new htmlTable();
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'set', _('Change')));
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'cancel', _('Cancel')));
		$return->addElement($buttonTable);
		return $return;
	}
	
	/**
	* Processes user input of the members page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_managedBy() {
		$return = array();
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_set'])) {
			$this->attributes['managedBy'][] = $_POST['managedBy'];
		}
		return $return;
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$errors = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('group', $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = 'group';
			// cn + sAMAccountName
			if ($rawAccounts[$i][$ids['windowsHost_name']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsHost_name']], 'groupname')) {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['windowsHost_name']];
					$partialAccounts[$i]['sAMAccountName'] = $rawAccounts[$i][$ids['windowsHost_name']];
				}
				else {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// description
			if ($rawAccounts[$i][$ids['windowsHost_description']] != "") {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['windowsHost_description']];
			}
			// notes
			if ($rawAccounts[$i][$ids['windowsHost_notes']] != "") {
				$partialAccounts[$i]['info'] = $rawAccounts[$i][$ids['windowsHost_notes']];
			}
			// email
			if ($rawAccounts[$i][$ids['windowsHost_mail']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsHost_mail']], 'email')) {
					$partialAccounts[$i]['mail'] = $rawAccounts[$i][$ids['windowsHost_mail']];
				}
				else {
					$errMsg = $this->messages['mail'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// add members
			if ($rawAccounts[$i][$ids['windowsHost_members']] != "") {
				$partialAccounts[$i]['member'] = explode(";", $rawAccounts[$i][$ids['windowsHost_members']]);
			}
			// group scope
			if ($rawAccounts[$i][$ids['windowsHost_scope']] != "") {
				if (in_array($rawAccounts[$i][$ids['windowsHost_scope']], $this->groupScopes)) {
					switch ($rawAccounts[$i][$ids['windowsHost_scope']]) {
						case windowsHost::SCOPE_DOMAIN_LOCAL:
							$partialAccounts[$i]['groupType'] = 4;
							break;
						case windowsHost::SCOPE_GLOBAL:
							$partialAccounts[$i]['groupType'] = 2;
							break;
						case windowsHost::SCOPE_UNIVERSAL:
							$partialAccounts[$i]['groupType'] = 8;
							break;
					}
				}
				else {
					$errMsg = $this->messages['groupScope'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['groupType'] = 2;
			}
			// group type
			if ($rawAccounts[$i][$ids['windowsHost_type']] != "") {
				if (in_array($rawAccounts[$i][$ids['windowsHost_type']], $this->groupTypes)) {
					if ($rawAccounts[$i][$ids['windowsHost_type']] == windowsHost::TYPE_SECURITY) {
						$partialAccounts[$i]['groupType'] = $partialAccounts[$i]['groupType'] - 2147483648;
					}
				}
				else {
					$errMsg = $this->messages['groupType'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['groupType'] = $partialAccounts[$i]['groupType'] - 2147483648;
			}
		}
		return $errors;
	}
	
	/**
	* Returns a list of PDF entries
	*/
	public function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('Group name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'info', _('Notes'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		// group type
		$groupType = windowsHost::TYPE_SECURITY;
		$groupScope = windowsHost::SCOPE_GLOBAL;
		if (isset($this->attributes['groupType'][0])) {
			if ($this->attributes['groupType'][0] & 2) {
				$groupScope = windowsHost::SCOPE_GLOBAL;
			}
			elseif ($this->attributes['groupType'][0] & 4) {
				$groupScope = windowsHost::SCOPE_DOMAIN_LOCAL;
			}
			elseif ($this->attributes['groupType'][0] & 8) {
				$groupScope = windowsHost::SCOPE_UNIVERSAL;
			}
			if ($this->attributes['groupType'][0] & 0x80000000) {
				$groupType = windowsHost::TYPE_SECURITY;
			}
			else {
				$groupType = windowsHost::TYPE_DISTRIBUTION;
			}
		}
		$groupTypeLabels = array_flip($this->groupTypes);
		$groupType = $groupTypeLabels[$groupType];
		$groupScopeLabels = array_flip($this->groupScopes);
		$groupScope = $groupScopeLabels[$groupScope];
		$return[get_class($this) . '_groupScope'] = array('<block><key>' . _('Group scope') . '</key><value>' . $groupScope . '</value></block>');
		$return[get_class($this) . '_groupType'] = array('<block><key>' . _('Group type') . '</key><value>' . $groupType . '</value></block>');
		// members
		if (sizeof($this->attributes['member']) > 0) {
			$memberList = array();
			if (isset($this->attributes['member']) && is_array($this->attributes['member'])) {
				$memberList = $this->attributes['member'];
			}
			usort($memberList, 'compareDN');
			$return[get_class($this) . '_member'][0] = '<block><key>' . _('Members') . '</key><tr><td align=\"L\">' . $memberList[0] . '</td></tr></block>';
			for ($i = 1; $i < sizeof($memberList); $i++) {
				$return[get_class($this) . '_member'][] = '<block><tr><td align=\"L\">' . $memberList[$i] . '</td></tr></block>';
			}
		}
		return $return;
	}

}


?>
