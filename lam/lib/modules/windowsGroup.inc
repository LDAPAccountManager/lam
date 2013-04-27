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
* Manages Windows AD (e.g. Samba 4) groups.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Windows AD (e.g. Samba 4) groups.
*
* @package modules
*/
class windowsGroup extends baseModule {
	
	/** possible group types (e.g. distribution) */
	private $groupTypes;
	/** possible group scopes (e.g. universal) */
	private $groupScopes;
	
	/** security group */
	const TYPE_SECURITY = 'security';
	/** email list */
	const TYPE_DISTRIBUTION = 'distribution';
	/** domain local group */
	const SCOPE_DOMAIN_LOCAL = 'domain';
	/** global group */
	const SCOPE_GLOBAL = 'global';
	/** universal group */
	const SCOPE_UNIVERSAL = 'universal';
	
	/**
	* Creates a new module for Samba 3 groups.
	*
	* @param string $scope account type
	*/
	function __construct($scope) {
		$this->groupTypes = array(
			_('Security') => windowsGroup::TYPE_SECURITY,
			_('Distribution') => windowsGroup::TYPE_DISTRIBUTION,
		);
		$this->groupScopes = array(
			_('Domain local') => windowsGroup::SCOPE_DOMAIN_LOCAL,
			_('Global') => windowsGroup::SCOPE_GLOBAL,
			_('Universal') => windowsGroup::SCOPE_UNIVERSAL,
		);
		// call parent constructor
		parent::__construct($scope);
	}
	
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
		$return["account_types"] = array('group');
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('and' => "", 'or' => '(objectClass=group)');
		// alias name
		$return["alias"] = _("Windows");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('group', 'securityPrincipal', 'mailRecipient');
		// managed attributes
		$return['attributes'] = array('cn', 'description', 'info', 'mail', 'member', 'sAMAccountName', 'groupType', 'otherMailbox');
		// help Entries
		$return['help'] = array(
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
			'cn' => array(
				"Headline" => _('Group name'), 'attr' => 'cn, sAMAccountName',
				"Text" => _('Please enter the group\'s name.')
			),
			'description' => array(
				"Headline" => _('Description'), 'attr' => 'description',
				"Text" => _('Please enter a descriptive text for this group.')
			),
			'info' => array(
				"Headline" => _('Notes'), 'attr' => 'info',
				"Text" => _('Additional notes to describe this entry.')
			),
			'mail' => array(
				"Headline" => _('Email address'), 'attr' => 'mail',
				"Text" => _('The list\'s email address.')
			),
			'otherMailbox' => array(
				"Headline" => _("Email alias"), 'attr' => 'otherMailbox',
				"Text" => _("Email alias for this account.")
			),
			'otherMailboxList' => array(
				"Headline" => _("Email alias"), 'attr' => 'otherMailbox',
				"Text" => _("Email alias for this account.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'member' => array(
				"Headline" => _('Members'), 'attr' => 'member',
				"Text" => _('This is a list of members of this group.')
			),
			'memberList' => array(
				"Headline" => _('Members'), 'attr' => 'member',
				"Text" => _('This is a list of members of this group. Multiple members are separated by semicolons.')
			),
			'groupType' => array(
				"Headline" => _('Group type'), 'attr' => 'groupType',
				"Text" => _('Security groups are used for permission management and distribution groups as email lists.')
			),
			'groupScope' => array(
				"Headline" => _('Group scope'), 'attr' => 'groupType',
				"Text" => _('Please specify the group scope.')
			),
		);
		// configuration settings
		$configContainer = new htmlTable();
		$configContainerHead = new htmlTable();
		$configContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$configContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainerOptions = new htmlTable();
		$configContainer->addElement($configContainerHead, true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsGroup_hidemail', false, _('Email address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsGroup_hideotherMailbox', false, _('Email alias'), null, false));
		$configContainer->addElement($configContainerOptions, true);
		$return['config_options']['all'] = $configContainer;
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'windowsGroup_name',
				'description' => _('Group name'),
				'help' => 'cn',
				'example' => _('Domain administrators'),
				'required' => true
			),
			array(
				'name' => 'windowsGroup_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Domain administrators'),
			),
			array(
				'name' => 'windowsGroup_notes',
				'description' => _('Notes'),
				'help' => 'info',
				'example' => _('Domain administrators'),
			),
			array(
				'name' => 'windowsGroup_scope',
				'description' => _('Group scope'),
				'help' => 'groupScope',
				'values' => implode(', ', array_values($this->groupScopes)),
				'example' => windowsGroup::SCOPE_GLOBAL,
				'default' => windowsGroup::SCOPE_GLOBAL,
			),
			array(
				'name' => 'windowsGroup_type',
				'description' => _('Group type'),
				'help' => 'groupType',
				'values' => implode(', ', array_values($this->groupTypes)),
				'example' => windowsGroup::TYPE_SECURITY,
				'default' => windowsGroup::TYPE_SECURITY,
			),
			array(
				'name' => 'windowsGroup_members',
				'description' => _('Members'),
				'help' => 'memberList',
				'example' => 'uid=user1,o=test;uid=user2,o=test',
			),
		);
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$return['upload_columns'][] = array(
				'name' => 'windowsGroup_mail',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('group@company.com'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hideotherMailbox')) {
			$return['upload_columns'][] = array(
				'name' => 'windowsGroup_otherMailbox',
				'description' => _('Email alias'),
				'help' => 'otherMailboxList',
				'example' => _('group@company.com'),
			);
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('Group name'),
			'description' => _('Description'),
			'info' => _('Notes'),
			'member' => _('Members'),
			'groupType' => _('Group type'),
			'groupScope' => _('Group scope'),
		);
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$return['PDF_fields']['mail'] = _('Email address');
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hideotherMailbox')) {
			$return['PDF_fields']['otherMailbox'] = _('Email alias');
		}
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	public function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('Group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' windowsGroup_cn', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' windowsGroup_mail', _('Please enter a valid email address!'));
		$this->messages['otherMailbox'][0] = array('ERROR', _('Email alias'), _('Email alias is invalid!'));
		$this->messages['otherMailbox'][1] = array('ERROR', _('Account %s:') . ' windowsGroup_otherMailbox', _('Email alias is invalid!'));
		$this->messages['groupScope'][0] = array('ERROR', _('Account %s:') . ' windowsGroup_groupScope', _('Please enter a valid group scope.'));
		$this->messages['groupType'][0] = array('ERROR', _('Account %s:') . ' windowsGroup_groupType', _('Please enter a valid group type.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$container = new htmlTable();
		$this->addSimpleInputTextField($container, 'cn', _('Group name'), true);
		$this->addSimpleInputTextField($container, 'description', _('Description'), false);
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$this->addSimpleInputTextField($container, 'mail', _('Email address'), false);
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hideotherMailbox')) {
			$this->addMultiValueInputTextField($container, 'otherMailbox', _('Email alias'));
		}
		// group type
		$groupType = windowsGroup::TYPE_SECURITY;
		$groupScope = windowsGroup::SCOPE_GLOBAL;
		if (isset($this->attributes['groupType'][0])) {
			if ($this->attributes['groupType'][0] & 2) {
				$groupScope = windowsGroup::SCOPE_GLOBAL;
			}
			elseif ($this->attributes['groupType'][0] & 4) {
				$groupScope = windowsGroup::SCOPE_DOMAIN_LOCAL;
			}
			elseif ($this->attributes['groupType'][0] & 8) {
				$groupScope = windowsGroup::SCOPE_UNIVERSAL;
			}
			if ($this->attributes['groupType'][0] & 0x80000000) {
				$groupType = windowsGroup::TYPE_SECURITY;
			}
			else {
				$groupType = windowsGroup::TYPE_DISTRIBUTION;
			}
		}
		$scopeList = $this->groupScopes;
		// do not allow invalid conversions
		if (isset($this->orig['groupType'][0])) {
			$flippedScopes = array_flip($this->groupScopes);
			if ($this->orig['groupType'][0] & 2) {
				// no change from global to domain local
				unset($scopeList[$flippedScopes[windowsGroup::SCOPE_DOMAIN_LOCAL]]);
			}
			elseif ($this->orig['groupType'][0] & 4) {
				// no change from domain local to global
				unset($scopeList[$flippedScopes[windowsGroup::SCOPE_GLOBAL]]);
			}
		}
		$groupScopeSelect = new htmlTableExtendedSelect('groupScope', $scopeList, array($groupScope), _('Group scope'), 'groupScope');
		$groupScopeSelect->setHasDescriptiveElements(true);
		$container->addElement($groupScopeSelect, true);
		$groupTypeSelect = new htmlTableExtendedSelect('groupType', $this->groupTypes, array($groupType), _('Group type'), 'groupType');
		$groupTypeSelect->setHasDescriptiveElements(true);
		$container->addElement($groupTypeSelect, true);
		// notes
		$info = '';
		if (isset($this->attributes['info'][0])) {
			$info = $this->attributes['info'][0];
		}
		$container->addElement(new htmlTableExtendedInputTextarea('info', $info, 30, 5, _('Notes'), 'info'), true);
		// group members
		$container->addElement(new htmlSpacer(null, '10px'), true);
		$container->addElement(new htmlOutputText(_("Group members")));
		$container->addElement(new htmlAccountPageButton(get_class($this), 'user', 'open', _('Edit members')));
		$container->addElement(new htmlHelpLink('member'), true);
		$memberList = array();
		if (isset($this->attributes['member'])) {
			for ($i = 0; $i < sizeof($this->attributes['member']); $i++) {
				$memberList[] = $this->attributes['member'][$i];
			}
			usort($memberList, 'compareDN');
		}
		$members = new htmlTable();
		$members->alignment = htmlElement::ALIGN_RIGHT;
		$members->colspan = 3;
		for ($i = 0; $i < sizeof($memberList); $i++) {
			$member = new htmlOutputText(getAbstractDN($memberList[$i]));
			$member->alignment = htmlElement::ALIGN_RIGHT;
			$members->addElement($member, true);
		}
		$container->addElement(new htmlOutputText(''));
		$container->addElement($members, true);
		$container->addElement(new htmlEqualWidth(array('groupType', 'groupScope')));
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
		$this->attributes['sAMAccountName'][0] = $_POST['cn'];
		if (!get_preg($_POST['cn'], 'groupname')) {
			$return[] = $this->messages['cn'][0];
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// email
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$this->attributes['mail'][0] = $_POST['mail'];
			if (!empty($_POST['mail']) && !get_preg($_POST['mail'], 'email')) {
				$return[] = $this->messages['mail'][0];
			}
		}
		// email aliases
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hideotherMailbox')) {
			$this->processMultiValueInputTextField('otherMailbox', $return, 'email');
		}
		// group scope
		switch ($_POST['groupScope']) {
			case windowsGroup::SCOPE_DOMAIN_LOCAL:
				$this->attributes['groupType'][0] = 4;
				break;
			case windowsGroup::SCOPE_GLOBAL:
				$this->attributes['groupType'][0] = 2;
				break;
			case windowsGroup::SCOPE_UNIVERSAL:
				$this->attributes['groupType'][0] = 8;
				break;
		}
		// group type
		if ($_POST['groupType'] == windowsGroup::TYPE_SECURITY) {
			$this->attributes['groupType'][0] = $this->attributes['groupType'][0] - 2147483648;
		}
		// notes
		$this->attributes['info'][0] = $_POST['info'];
		return $return;
	}
	
	/**
	* This function will create the meta HTML code to show a page to change the member attribute.
	* 
	* @return htmlElement HTML meta data
	*/
	function display_html_user() {
		$return = new htmlTable();
		// show list of possible new members
		if (isset($_POST['form_subpage_' . get_class($this) . '_user_select']) && isset($_POST['type'])) {
			$options = array();
			$filter = get_ldap_filter($_POST['type']);
			$entries = searchLDAPByFilter($filter, array('dn'), array($_POST['type']));
			for ($i = 0; $i < sizeof($entries); $i++) {
				$entries[$i] = $entries[$i]['dn'];
			}
			// sort by DN
			usort($entries, 'compareDN');
			for ($i = 0; $i < sizeof($entries); $i++) {
				if (!isset($this->attributes['member']) || !in_array($entries[$i], $this->attributes['member'])) {
					$options[getAbstractDN($entries[$i])] = $entries[$i];
				}
			}
			$size = 20;
			if (sizeof($options) < 20) $size = sizeof($options);
			$membersSelect = new htmlSelect('members', $options, array(), $size);
			$membersSelect->setHasDescriptiveElements(true);
			$membersSelect->setMultiSelect(true);
			$membersSelect->setRightToLeftTextDirection(true);
			$membersSelect->setSortElements(false);
			$membersSelect->setTransformSingleSelect(false);
			$return->addElement($membersSelect, true);
			$buttonTable = new htmlTable();
			$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'user', 'addMembers', _('Add')));
			$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'user', 'cancel', _('Cancel')));
			$return->addElement($buttonTable);
			return $return;
		}
		// show existing members
		$membersTemp = array();
		if (isset($this->attributes['member'])) {
			$membersTemp = $this->attributes['member'];
		}
		// sort by DN
		usort($membersTemp, 'compareDN');
		$members = array();
		for ($i = 0; $i < sizeof($membersTemp); $i++) {
			$members[getAbstractDN($membersTemp[$i])] = $membersTemp[$i];
		}
		$size = 20;
		if (isset($this->attributes['member']) && (sizeof($this->attributes['member']) < 20)) {
			$size = sizeof($this->attributes['member']);
		}
		if (sizeof($members) > 0) {
			$membersSelect = new htmlSelect('members', $members, array(), $size);
			$membersSelect->setHasDescriptiveElements(true);
			$membersSelect->setMultiSelect(true);
			$membersSelect->setRightToLeftTextDirection(true);
			$membersSelect->setSortElements(false);
			$membersSelect->setTransformSingleSelect(false);
			$return->addElement($membersSelect, true);
			$removeButton = new htmlAccountPageButton(get_class($this), 'user', 'remove', _('Remove selected entries'));
			$removeButton->colspan = 3;
			$return->addElement($removeButton, true);
			$return->addElement(new htmlOutputText('&nbsp;', false), true);
		}
		$types = $_SESSION['config']->get_ActiveTypes();
		$options = array();
		$optionsSelected = array();
		for ($i = 0; $i < sizeof($types); $i++) {
			$options[getTypeAlias($types[$i])] = $types[$i];
			if ($types[$i] == 'user') {
				$optionsSelected[] = $types[$i];
			}
		}
		$typeTable = new htmlTable();
		$typeTable->addElement(new htmlOutputText(_('Add entries of this type:') . ' '));
		$typeSelect = new htmlSelect('type', $options, $optionsSelected);
		$typeSelect->setHasDescriptiveElements(true);
		$typeTable->addElement($typeSelect);
		$typeTable->addElement(new htmlAccountPageButton(get_class($this), 'user', 'select', _('Ok')));
		$return->addElement($typeTable, true);
		$return->addElement(new htmlOutputText('&nbsp;', false), true);
		$return->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'membersBack', _('Back')));
		return $return;
	}
	
	/**
	* Processes user input of the members page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_user() {
		$return = array();
		if (isset($_POST['form_subpage_' . get_class($this) . '_user_remove']) && isset($_POST['members'])) {
			$members = array_flip($this->attributes['member']);
			for ($i = 0; $i < sizeof($_POST['members']); $i++) {
				if (isset($members[$_POST['members'][$i]])) {
					unset($members[$_POST['members'][$i]]);
				}
			}
			$this->attributes['member'] = array_values(array_flip($members));
		}
		elseif (isset($_POST['form_subpage_' . get_class($this) . '_user_addMembers']) && isset($_POST['members'])) {
			for ($i = 0; $i < sizeof($_POST['members']); $i++) {
				$this->attributes['member'][] = $_POST['members'][$i];
				$this->attributes['member'] = array_unique($this->attributes['member']);
			}
		}
		// check input
		if (!isset($_POST['form_subpage_' . get_class($this) . '_user_select'])) {
			if (!$this->isBooleanConfigOptionSet('groupOfNames_membersOptional')) {
				if (!isset($this->attributes['member']) || (sizeof($this->attributes['member']) < 1)) {
					$return[] = $this->messages['member'][0];
				}
			}
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
			if ($rawAccounts[$i][$ids['windowsGroup_name']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsGroup_name']], 'groupname')) {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['windowsGroup_name']];
					$partialAccounts[$i]['sAMAccountName'] = $rawAccounts[$i][$ids['windowsGroup_name']];
				}
				else {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// description
			if ($rawAccounts[$i][$ids['windowsGroup_description']] != "") {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['windowsGroup_description']];
			}
			// notes
			if ($rawAccounts[$i][$ids['windowsGroup_notes']] != "") {
				$partialAccounts[$i]['info'] = $rawAccounts[$i][$ids['windowsGroup_notes']];
			}
			// email
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail') && ($rawAccounts[$i][$ids['windowsGroup_mail']] != "")) {
				if (get_preg($rawAccounts[$i][$ids['windowsGroup_mail']], 'email')) {
					$partialAccounts[$i]['mail'] = $rawAccounts[$i][$ids['windowsGroup_mail']];
				}
				else {
					$errMsg = $this->messages['mail'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// email aliases
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hideotherMailbox') && isset($ids['windowsGroup_otherMailbox']) && ($rawAccounts[$i][$ids['windowsGroup_otherMailbox']] != "")) {
				$valueList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['windowsGroup_otherMailbox']]);
				$partialAccounts[$i]['otherMailbox'] = $valueList;
				for ($x = 0; $x < sizeof($valueList); $x++) {
					if (!get_preg($valueList[$x], 'email')) {
						$errMsg = $this->messages['otherMailbox'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
			// add members
			if ($rawAccounts[$i][$ids['windowsGroup_members']] != "") {
				$partialAccounts[$i]['member'] = explode(";", $rawAccounts[$i][$ids['windowsGroup_members']]);
			}
			// group scope
			if ($rawAccounts[$i][$ids['windowsGroup_scope']] != "") {
				if (in_array($rawAccounts[$i][$ids['windowsGroup_scope']], $this->groupScopes)) {
					switch ($rawAccounts[$i][$ids['windowsGroup_scope']]) {
						case windowsGroup::SCOPE_DOMAIN_LOCAL:
							$partialAccounts[$i]['groupType'] = 4;
							break;
						case windowsGroup::SCOPE_GLOBAL:
							$partialAccounts[$i]['groupType'] = 2;
							break;
						case windowsGroup::SCOPE_UNIVERSAL:
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
			if ($rawAccounts[$i][$ids['windowsGroup_type']] != "") {
				if (in_array($rawAccounts[$i][$ids['windowsGroup_type']], $this->groupTypes)) {
					if ($rawAccounts[$i][$ids['windowsGroup_type']] == windowsGroup::TYPE_SECURITY) {
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
		$this->addSimplePDFField($return, 'otherMailbox', _('Email alias'));
		// group type
		$groupType = windowsGroup::TYPE_SECURITY;
		$groupScope = windowsGroup::SCOPE_GLOBAL;
		if (isset($this->attributes['groupType'][0])) {
			if ($this->attributes['groupType'][0] & 2) {
				$groupScope = windowsGroup::SCOPE_GLOBAL;
			}
			elseif ($this->attributes['groupType'][0] & 4) {
				$groupScope = windowsGroup::SCOPE_DOMAIN_LOCAL;
			}
			elseif ($this->attributes['groupType'][0] & 8) {
				$groupScope = windowsGroup::SCOPE_UNIVERSAL;
			}
			if ($this->attributes['groupType'][0] & 0x80000000) {
				$groupType = windowsGroup::TYPE_SECURITY;
			}
			else {
				$groupType = windowsGroup::TYPE_DISTRIBUTION;
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
