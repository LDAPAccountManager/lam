<?php
use \LAM\PDF\PDFTable;
use \LAM\PDF\PDFTableCell;
use \LAM\PDF\PDFTableRow;
use \LAM\TYPES\TypeManager;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2017  Roland Gruber

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
	/** group cache */
	private $groupCache;

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
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('group'));
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
		$return['attributes'] = array('cn', 'description', 'info', 'mail', 'member', 'memberOf', 'sAMAccountName',
			'groupType', 'managedBy', 'msSFU30Name', 'msSFU30NisDomain');
		// help Entries
		$return['help'] = array(
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
			'cn' => array(
				"Headline" => _('Group name'), 'attr' => 'cn, sAMAccountName',
				"Text" => _('Please enter the group name.')
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
			'member' => array(
				"Headline" => _('Members'), 'attr' => 'member',
				"Text" => _('This is a list of members of this group.')
			),
			'memberOf' => array(
				"Headline" => _('Member of'), 'attr' => 'memberOf',
				"Text" => _('This is a list of groups this group is member of.')
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
			'managedBy' => array(
				"Headline" => _('Managed by'), 'attr' => 'managedBy',
				"Text" => _('The group is managed by this contact person.')
			),
			'msSFU30Name' => array(
				"Headline" => _('NIS name'), 'attr' => 'msSFU30Name',
				"Text" => _('Group name for NIS.')
			),
			'msSFU30NisDomain' => array(
				"Headline" => _('NIS domain'), 'attr' => 'msSFU30NisDomain',
				"Text" => _('NIS domain name.')
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
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsGroup_hidemanagedBy', false, _('Managed by'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsGroup_hidemsSFU30Name', true, _('NIS name'), null, false));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('windowsGroup_hidemsSFU30NisDomain', true, _('NIS domain'), null, false));
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
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemanagedBy')) {
			$return['upload_columns'][] = array(
				'name' => 'windowsGroup_managedBy',
				'description' => _('Managed by'),
				'help' => 'managedBy',
				'example' => 'cn=user1,o=test',
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30Name', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsGroup_msSFU30Name',
				'description' => _('NIS name'),
				'help' => 'msSFU30Name',
				'example' => _('adminstrators'),
			);
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30NisDomain', true)) {
			$return['upload_columns'][] = array(
				'name' => 'windowsGroup_msSFU30NisDomain',
				'description' => _('NIS domain'),
				'help' => 'msSFU30NisDomain',
				'example' => _('domain'),
			);
		}
		// profile options
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30NisDomain', true)) {
			$profileContainer = new htmlTable();
			$profileContainer->addElement(new htmlTableExtendedInputField(_('NIS domain'), 'windowsGroup_msSFU30NisDomain', null, 'msSFU30NisDomain'), true);
			$return['profile_options'] = $profileContainer;
			$return['profile_mappings']['windowsGroup_msSFU30NisDomain'] = 'msSFU30NisDomain';
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('Group name'),
			'description' => _('Description'),
			'info' => _('Notes'),
			'member' => _('Members'),
			'memberOf' => _('Member of'),
			'groupType' => _('Group type'),
			'groupScope' => _('Group scope'),
		);
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
			$return['PDF_fields']['mail'] = _('Email address');
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemanagedBy')) {
			$return['PDF_fields']['managedBy'] = _('Managed by');
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30Name', true)) {
			$return['PDF_fields']['msSFU30Name'] = _('NIS name');
		}
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30NisDomain', true)) {
			$return['PDF_fields']['msSFU30NisDomain'] = _('NIS domain');
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
		$this->messages['groupScope'][0] = array('ERROR', _('Account %s:') . ' windowsGroup_groupScope', _('Please enter a valid group scope.'));
		$this->messages['groupType'][0] = array('ERROR', _('Account %s:') . ' windowsGroup_groupType', _('Please enter a valid group type.'));
		$this->messages['msSFU30Name'][0] = array('ERROR', _('NIS name'), _('NIS name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['msSFU30Name'][1] = array('ERROR', _('Account %s:') . ' windowsGroup_msSFU30Name', _('NIS name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
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
		// managed by
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemanagedBy')) {
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
		}
		// NIS
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30Name', true) || !$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30NisDomain', true)) {
			$container->addElement(new htmlSubTitle(_('NIS')), true);
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30Name', true)) {
				$this->addSimpleInputTextField($container, 'msSFU30Name', _('NIS name'));
			}
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30NisDomain', true)) {
				$this->addSimpleInputTextField($container, 'msSFU30NisDomain', _('NIS domain'));
			}
			$container->addVerticalSpace('20px');
		}
		// group members
		$container->addElement(new htmlSpacer(null, '10px'), true);
		$container->addElement(new htmlOutputText(_("Group members")));
		$memberButtons = new htmlGroup();
		$memberButtons->addElement(new htmlAccountPageButton(get_class($this), 'user', 'open', _('Edit')));
		if (!empty($this->attributes['member'])) {
			$memberButtons->addElement(new htmlSpacer('10px', null));
			$memberButtons->addElement(new htmlAccountPageButton(get_class($this), 'effectiveMembers', 'open', _('Show effective members')));
		}
		$container->addElement($memberButtons);
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
		// member of
		$container->addVerticalSpace('10px');
		$container->addElement(new htmlOutputText(_("Member of")));
		$container->addElement(new htmlAccountPageButton(get_class($this), 'memberof', 'open', _('Edit')));
		$container->addElement(new htmlHelpLink('memberOf'), true);
		$memberList = array();
		if (isset($this->attributes['memberOf'])) {
			for ($i = 0; $i < sizeof($this->attributes['memberOf']); $i++) {
				$memberList[] = $this->attributes['memberOf'][$i];
			}
			usort($memberList, 'compareDN');
		}
		$memberOf = new htmlTable();
		$memberOf->alignment = htmlElement::ALIGN_RIGHT;
		$memberOf->colspan = 3;
		for ($i = 0; $i < sizeof($memberList); $i++) {
			$member = new htmlOutputText(getAbstractDN($memberList[$i]));
			$member->alignment = htmlElement::ALIGN_RIGHT;
			$memberOf->addElement($member, true);
		}
		$container->addElement(new htmlOutputText(''));
		$container->addElement($memberOf, true);

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
		// managed by
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemanagedBy')) {
			if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_removeManagedBy'])) {
				unset($this->attributes['managedBy']);
			}
		}
		// NIS name
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30Name', true)) {
			if ($this->getAccountContainer()->isNewAccount && !isset($this->attributes['msSFU30Name']) && empty($_POST['msSFU30Name'])) {
				$this->attributes['msSFU30Name'][0] = $_POST['cn'];
			}
			else {
				$this->attributes['msSFU30Name'][0] = $_POST['msSFU30Name'];
			}
			if (!empty($this->attributes['msSFU30Name'][0]) && !get_preg($this->attributes['msSFU30Name'][0], 'groupname')) {
				$return[] = $this->messages['msSFU30Name'][0];
			}
		}
		// NIS domain
		if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30Name', true)) {
			$this->attributes['msSFU30NisDomain'][0] = $_POST['msSFU30NisDomain'];
		}
		return $return;
	}

	/**
	* Displays the memberof selection.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_memberof() {
		$return = new htmlTable();
		$groups = $this->findGroups();
		// sort by DN
		usort($groups, 'compareDN');

		$groupContainer = new htmlTable();
		$groupContainer->alignment = htmlElement::ALIGN_TOP;
		$groupContainer->addElement(new htmlSubTitle(_("Groups")), true);
		$groupContainer->addElement(new htmlOutputText(_("Selected groups")));
		$groupContainer->addElement(new htmlOutputText(''));
		$groupContainer->addElement(new htmlOutputText(_("Available groups")));
		$groupContainer->addNewLine();

		$selectedGroups = array();
		if (empty($this->attributes['memberOf'])) {
			$this->attributes['memberOf'] = array();
		}
		// sort by DN
		usort($this->attributes['memberOf'], 'compareDN');
		for ($i = 0; $i < sizeof($this->attributes['memberOf']); $i++) {
			if (in_array($this->attributes['memberOf'][$i], $groups)) {
				$selectedGroups[getAbstractDN($this->attributes['memberOf'][$i])] = $this->attributes['memberOf'][$i];
			}
		}
		$availableGroups = array();
		foreach ($groups as $dn) {
			if (!in_array($dn, $this->attributes['memberOf'])) {
				$availableGroups[getAbstractDN($dn)] = $dn;
			}
		}

		$remGroupSelect = new htmlSelect('removegroups', $selectedGroups, null, 15);
		$remGroupSelect->setMultiSelect(true);
		$remGroupSelect->setTransformSingleSelect(false);
		$remGroupSelect->setHasDescriptiveElements(true);
		$remGroupSelect->setRightToLeftTextDirection(true);
		$remGroupSelect->setSortElements(false);
		$groupContainer->addElement($remGroupSelect);
		$buttonGroupContainer = new htmlTable();
		$buttonGroupContainer->addElement(new htmlButton('addgroups_button', 'back.gif', true), true);
		$buttonGroupContainer->addElement(new htmlButton('removegroups_button', 'forward.gif', true), true);
		$groupContainer->addElement($buttonGroupContainer);
		$addGroupSelect = new htmlSelect('addgroups', $availableGroups, null, 15);
		$addGroupSelect->setMultiSelect(true);
		$addGroupSelect->setHasDescriptiveElements(true);
		$addGroupSelect->setTransformSingleSelect(false);
		$addGroupSelect->setRightToLeftTextDirection(true);
		$addGroupSelect->setSortElements(false);
		$groupContainer->addElement($addGroupSelect);
		$groupContainer->addNewLine();
		$return->addElement($groupContainer);
		$return->addNewLine();

		$backGroup = new htmlGroup();
		$backGroup->colspan = 10;
		$backGroup->addElement(new htmlSpacer(null, '10px'), true);
		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
		$backGroup->addElement($backButton);
		$return->addElement($backGroup);
		return $return;
	}

	/**
	* Processes user input of the memberof selection page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_memberof() {
		if (isset($_POST['addgroups']) && isset($_POST['addgroups_button'])) { // Add groups to list
			// add new group
			$this->attributes['memberOf'] = @array_merge($this->attributes['memberOf'], $_POST['addgroups']);
		}
		elseif (isset($_POST['removegroups']) && isset($_POST['removegroups_button'])) { // remove groups from list
			$this->attributes['memberOf'] = array_delete($_POST['removegroups'], $this->attributes['memberOf']);
		}
		return array();
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
	* This function will create the meta HTML code to show a page to change the member attribute.
	*
	* @return htmlElement HTML meta data
	*/
	function display_html_user() {
		$return = new htmlTable();
		$typeManager = new TypeManager();
		// show list of possible new members
		if (isset($_POST['form_subpage_' . get_class($this) . '_user_select']) && isset($_POST['type'])) {
			$type = $typeManager->getConfiguredType($_POST['type']);
			$filterGroup = new htmlGroup();
			$filterGroup->addElement(new htmlOutputText(_('Filter') . ' '));
			$filter = new htmlInputField('windows_filter');
			$filter->setFieldSize('5em');
			$filter->filterSelectBox('members');
			$filterGroup->addElement($filter);
			$return->addElement($filterGroup, true);

			$options = array();
			$filter = get_ldap_filter($type->getId());
			$entries = searchLDAP($type->getSuffix(), $filter, array('dn'));
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
			$return->addVerticalSpace('10px');
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
		$types = $typeManager->getConfiguredTypes();
		$options = array();
		$optionsSelected = array();
		foreach ($types as $type) {
			$options[$type->getAlias()] = $type->getId();
			if ($type->getScope() == 'user') {
				$optionsSelected[] = $type->getId();
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
	 * This function will create the meta HTML code to show a page to list effective members.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_effectiveMembers() {
		$return = new htmlTable();
		$effectiveMembers = $this->getEffectiveMembers();
		$entryTable = new htmlTable();
		// sort by DN, align right
		usort($effectiveMembers, 'compareDN');
		$entryTable->alignment = htmlElement::ALIGN_RIGHT;
		foreach ($effectiveMembers as $member) {
			$entry = new htmlOutputText(getAbstractDN($member));
			$entry->alignment = htmlElement::ALIGN_RIGHT;
			$entryTable->addElement($entry, true);
		}
		$return->addElement($entryTable, true);
		$return->addVerticalSpace('10px');
		$return->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'membersEffectiveBack', _('Back')));
		return $return;
	}

	/**
	 * Processes user input of the effective members page.
	 *
	 * @return array list of info/error messages
	 */
	function process_effectiveMembers() {
		// no processing, page is read-only
		return array();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
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
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsGroup_description', 'description');
			// notes
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsGroup_notes', 'info');
			// email
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemail')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsGroup_mail', 'mail',
					'email', $this->messages['mail'][1], $errors);
			}
			// managed by
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemanagedBy')) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsGroup_managedBy', 'managedBy');
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
			// NIS name
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30Name', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsGroup_msSFU30Name', 'msSFU30Name',
					'groupname', $this->messages['msSFU30Name'][1], $errors);
			}
			// NIS domain
			if (!$this->isBooleanConfigOptionSet('windowsGroup_hidemsSFU30NisDomain', true)) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'windowsGroup_msSFU30NisDomain', 'msSFU30NisDomain');
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	public function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('Group name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'info', _('Notes'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'msSFU30Name', _('NIS name'));
		$this->addSimplePDFField($return, 'msSFU30NisDomain', _('NIS domain'));
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
		$this->addPDFKeyValue($return, 'groupScope', _('Group scope'), $groupScope);
		$this->addPDFKeyValue($return, 'groupType', _('Group type'), $groupType);
		// managed by
		$managedBy = '';
		if (isset($this->attributes['managedBy'][0])) {
			$managedBy = getAbstractDN($this->attributes['managedBy'][0]);
			$this->addPDFKeyValue($return, 'managedBy', _('Managed by'), $managedBy);
		}
		// members
		if (!empty($this->attributes['member'])) {
			$memberList = array();
			if (isset($this->attributes['member']) && is_array($this->attributes['member'])) {
				$memberList = $this->attributes['member'];
			}
			usort($memberList, 'compareDN');
			$pdfTable = new PDFTable(_('Members'));
			for ($i = 0; $i < sizeof($memberList); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($memberList[$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'member', $pdfTable);
		}
		// member of
		if (!empty($this->attributes['memberOf'])) {
			$memberOfList = array();
			if (isset($this->attributes['memberOf']) && is_array($this->attributes['memberOf'])) {
				$memberOfList = $this->attributes['memberOf'];
			}
			usort($memberOfList, 'compareDN');
			$pdfTable = new PDFTable(_('Member of'));
			for ($i = 0; $i < sizeof($memberOfList); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($memberOfList[$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'memberOf', $pdfTable);
		}
		return $return;
	}

	/**
	 * Finds all existing groups.
	 *
	 * @return array group DNs
	 */
	private function findGroups() {
		if ($this->groupCache != null) {
			return $this->groupCache;
		}
		$return = array();
		$types = array('group');
		$results = searchLDAPByFilter('(objectClass=group)', array('dn'), $types);
		$count = sizeof($results);
		for ($i = 0; $i < $count; $i++) {
			if (isset($results[$i]['dn'])) {
				$return[] = $results[$i]['dn'];
			}
		}
		$this->groupCache = $return;
		return $return;
	}

	/**
	* Returns a list of modifications which have to be made to the LDAP account.
	*
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	*
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It is possible to change several DNs (e.g. create a new user and add him
	* to some groups via attribute memberUid)<br>
	* <br><b>"add"</b> are attributes which have to be added to the LDAP entry
	* <br><b>"remove"</b> are attributes which have to be removed from the LDAP entry
	* <br><b>"modify"</b> are attributes which have to be modified in the LDAP entry
	* <br><b>"notchanged"</b> are attributes which stay unchanged
	* <br><b>"info"</b> values with informational value (e.g. to be used later by pre/postModify actions)
	* <br>
	* <br>This builds the required comands from $this-attributes and $this->orig.
	*
	* @return array list of modifications
	*/
	public function save_attributes() {
		$attrs = $this->attributes;
		$orig = $this->orig;
		$attrs['memberOf'] = array();
		$orig['memberOf'] = array();
		return $this->getAccountContainer()->save_module_attributes($attrs, $orig);
	}

	/**
	 * Runs the postmodify actions.
	 *
	 * @see baseModule::postModifyActions()
	 *
	 * @param boolean $newAccount
	 * @param array $attributes LDAP attributes of this entry
	 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	public function postModifyActions($newAccount, $attributes) {
		$messages = array();
		// set groups
		$groups = $this->findGroups();
		if (!isset($this->orig['memberOf'])) {
			$this->orig['memberOf'] = array();
		}
		if (!isset($this->attributes['memberOf'])) {
			$this->attributes['memberOf'] = array();
		}
		$toAdd = array_values(array_diff($this->attributes['memberOf'], $this->orig['memberOf']));
		$toRem = array_values(array_diff($this->orig['memberOf'], $this->attributes['memberOf']));
		$toUpdate = array_values(array_intersect($this->attributes['memberOf'], $this->orig['memberOf']));
		$ldapUser = $_SESSION['ldap']->decrypt_login();
		$ldapUser = $ldapUser[0];
		// add groups
		for ($i = 0; $i < sizeof($toAdd); $i++) {
			if (in_array($toAdd[$i], $groups)) {
				$success = @ldap_mod_add($_SESSION['ldap']->server(), $toAdd[$i], array('member' => array($this->getAccountContainer()->finalDN)));
				if (!$success) {
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to add group ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toAdd[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = array('ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $toAdd[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Added group ' . $this->getAccountContainer()->finalDN . ' to group: ' . $toAdd[$i]);
				}
			}
		}
		// remove groups
		for ($i = 0; $i < sizeof($toRem); $i++) {
			if (in_array($toRem[$i], $groups)) {
				$success = @ldap_mod_del($_SESSION['ldap']->server(), $toRem[$i], array('member' => array($this->getAccountContainer()->dn_orig)));
				if (!$success) {
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete group ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$messages[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $toRem[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
				}
				else {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Removed group ' . $this->getAccountContainer()->finalDN . ' from group: ' . $toRem[$i]);
				}
			}
		}
		return $messages;
	}

	/**
	 * Recursively gets the members of this group and its subgroups.
	 *
	 * @return list of DNs
	 */
	private function getEffectiveMembers() {
		$membersToCheck = $this->attributes['member'];
		$effectiveMembers = $membersToCheck;
		while (!empty($membersToCheck)) {
			$member = array_pop($membersToCheck);
			$attrs = ldapGetDN($member, array('member'));
			if (!empty($attrs['member'])) {
				foreach ($attrs['member'] as $newMember) {
					if (!in_array($newMember, $effectiveMembers)) {
						$effectiveMembers[] = $newMember;
						$membersToCheck[] = $newMember;
					}
				}
			}
		}
		return $effectiveMembers;
	}

}


?>
