<?php

use LAM\TYPES\ConfiguredType;
use \LAM\TYPES\TypeManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2007 - 2024  Roland Gruber

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
 * Manages Samba 3 accounts for groups.
 *
 * @package modules
 *
 * @author Tilo Lutz
 * @author Roland Gruber
 * @author Michael Duergner
 */

/**
 * Manages the object class "sambaGroupMapping" for groups.
 *
 * @package modules
 */
class sambaGroupMapping extends baseModule {

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['sambaSID', 'gidNumber', 'displayName'];

	// Variables
	/** Array of well known RIDs */
	private $rids;
	/** Array of sambaGroupTypes */
	private $sambaGroupTypes;
	/** cache for domain list */
	private $cachedDomainList;
	/** cache for existing Samba entries (array(dn => cn)) */
	private $sidCache;

	/**
	 * Creates a new module for Samba 3 groups.
	 *
	 * @param string $scope account type
	 */
	function __construct($scope) {
		// load error messages
		$this->rids = [
			_('Domain admins') => 512,
			_('Domain users') => 513,
			_('Domain guests') => 514,
			_('Domain computers') => 515,
			_('Domain controllers') => 516,
			_('Domain certificate admins') => 517,
			_('Domain schema admins') => 518,
			_('Domain enterprise admins') => 519,
			_('Domain policy admins') => 520];

		$this->sambaGroupTypes = [
			_('Domain group') => 2,
			_('Local group') => 4,
			_('Builtin group') => 5,
		];
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	 * Returns true if this module can manage accounts of the current type, otherwise false.
	 *
	 * @return boolean true if module fits
	 */
	public function can_manage() {
		return in_array($this->get_scope(), ['group']);
	}

	/**
	 * Returns meta data that is interpreted by parent class
	 *
	 * @return array array with meta data
	 *
	 * @see baseModule::get_metaData()
	 */
	function get_metaData() {
		$return = [];
		// icon
		$return['icon'] = 'samba.svg';
		// alias name
		$return["alias"] = _('Samba 3');
		// module dependencies
		$return['dependencies'] = ['depends' => [['posixGroup', 'rfc2307bisPosixGroup']], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['sambaGroupMapping'];
		// managed attributes
		$return['attributes'] = ['gidNumber', 'sambaSID', 'sambaGroupType', 'displayName', 'sambaSIDList', 'description'];
		// available PDF fields
		$return['PDF_fields'] = [
			'gidNumber' => _('GID number'),
			'sambaSID' => _('Windows group'),
			'displayName' => _('Display name'),
			'sambaGroupType' => _('Samba group type'),
			'description' => _('Description'),
			'sambaSIDList' => _('Local members'),
		];
		// upload fields
		// search existing Samba 3 domains
		if (isLoggedIn()) {
			$return['upload_columns'] = [
				[
					'name' => 'sambaGroupMapping_name',
					'description' => _('Samba display name'),
					'help' => 'displayName',
					'example' => _('Domain administrators')
				],
				[
					'name' => 'sambaGroupMapping_rid',
					'description' => _('Samba RID number'),
					'help' => 'rid',
					'example' => _('Domain admins')
				],
				[
					'name' => 'sambaGroupMapping_groupType',
					'description' => _('Samba group type'),
					'help' => 'type',
					'values' => implode(", ", array_keys($this->sambaGroupTypes) + $this->sambaGroupTypes),
					'example' => '2'
				]
			];
			$return['upload_preDepends'] = ['posixGroup', 'rfc2307bisPosixGroup'];
		}
		// help Entries
		$return['help'] = [
			'displayName' => [
				"Headline" => _("Display name"), 'attr' => 'displayName',
				"Text" => _("This is the group name which will be shown in Windows.")
			],
			'sambaSID' => [
				"Headline" => _("Windows group name"), 'attr' => 'sambaSID',
				"Text" => _("If you want to use a well known RID you can select a well known group.")
			],
			'rid' => [
				"Headline" => _("Samba RID number"),
				"Text" => _("This is the relative ID (similar to UID on Unix) for Windows accounts. If you leave this empty LAM will calculate the RID from the UID. This can be either a number or the name of a special group:") . ' ' . implode(", ", array_keys($this->rids))
			],
			'sambaDomainName' => [
				"Headline" => _("Domain"),
				"Text" => _("Windows-Domain name of group.") . ' ' . _("Can be left empty.")
			],
			'type' => [
				"Headline" => _("Samba group type"), 'attr' => 'sambaGroupType',
				"Text" => _("Windows group type.")
			],
			'sambaSIDList' => [
				"Headline" => _('Local members'), 'attr' => 'sambaSIDList',
				"Text" => _("Use this to specify other groups or accounts from other domains as group members.")
			],
			'filter' => [
				"Headline" => _("Filter"),
				"Text" => _("Here you can enter a filter value. Only entries which contain the filter text will be shown.")
					. ' ' . _('Possible wildcards are: "*" = any character, "^" = line start, "$" = line end')
			],
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
	}

	/**
	 * Gets the GID number from the Unix group module.
	 *
	 * @return String GID number
	 */
	private function getGID() {
		$modules = ['posixGroup', 'rfc2307bisPosixGroup'];
		for ($i = 0; $i < sizeof($modules); $i++) {
			if ($this->getAccountContainer()->getAccountModule($modules[$i]) != null) {
				$attrs = $this->getAccountContainer()->getAccountModule($modules[$i])->getAttributes();
				if (isset($attrs['gidNumber'][0])) {
					return $attrs['gidNumber'][0];
				}
			}
		}
		return null;
	}

	/**
	 * Gets the cn from the Unix group module.
	 *
	 * @return String cn attribute
	 */
	private function getCn() {
		$modules = ['posixGroup', 'groupOfNames', 'groupOfUniqueNames'];
		for ($i = 0; $i < sizeof($modules); $i++) {
			if ($this->getAccountContainer()->getAccountModule($modules[$i]) != null) {
				$attrs = $this->getAccountContainer()->getAccountModule($modules[$i])->getAttributes();
				if (isset($attrs['cn'][0])) {
					return $attrs['cn'][0];
				}
			}
		}
		return null;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_uploadColumns()
	 */
	public function get_uploadColumns($selectedModules, &$type) {
		$return = parent::get_uploadColumns($selectedModules, $type);
		$domains = $this->getDomains();
		$domainNames = [];
		for ($i = 0; $i < sizeof($domains); $i++) $domainNames[] = $domains[$i]->name;
		$return[] = [
			'name' => 'sambaGroupMapping_domain',
			'description' => _('Samba domain name'),
			'help' => 'sambaDomainName',
			'example' => $domainNames[0],
			'values' => implode(", ", $domainNames),
			'required' => true
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		// search existing Samba 3 domains
		$domains = $this->getDomains();
		$nameToSID = [];
		// get domain SIDs
		for ($i = 0; $i < sizeof($domains); $i++) {
			$nameToSID[$domains[$i]->name] = $domains[$i]->SID;
		}
		// get domain RID bases
		$nameToRIDBase = [];
		for ($i = 0; $i < sizeof($domains); $i++) {
			$nameToRIDBase[$domains[$i]->name] = $domains[$i]->RIDbase;
		}
		$errors = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// group type
			if ($rawAccounts[$i][$ids['sambaGroupMapping_groupType']] != "") {
				if (in_array($rawAccounts[$i][$ids['sambaGroupMapping_groupType']], $this->sambaGroupTypes)) {  // number given
					$partialAccounts[$i]['sambaGroupType'] = $rawAccounts[$i][$ids['sambaGroupMapping_groupType']];
				}
				elseif (in_array($rawAccounts[$i][$ids['sambaGroupMapping_groupType']], array_keys($this->sambaGroupTypes))) {  // description given
					$partialAccounts[$i]['sambaGroupType'] = $this->sambaGroupTypes[$rawAccounts[$i][$ids['sambaGroupMapping_groupType']]];
				}
				else {  // invalid type
					$errMsg = $this->messages['groupType'][0];
					array_push($errMsg, [$i, implode(", ", array_keys($this->sambaGroupTypes) + $this->sambaGroupTypes)]);
					$errors[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['sambaGroupType'] = "2";  // 2 is the default (domain group)
			}
			if (!in_array("sambaGroupMapping", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "sambaGroupMapping";
			}
			// SID
			$domSID = $nameToSID[$rawAccounts[$i][$ids['sambaGroupMapping_domain']]];
			if (!isset($domSID)) {
				$errMsg = $this->messages['sambaSID'][1];
				array_push($errMsg, [$rawAccounts[$i][$ids['sambaGroupMapping_domain']], $i]);
				$errors[] = $errMsg;
			}
			else {
				// RID
				$rid = $rawAccounts[$i][$ids['sambaGroupMapping_rid']];
				if (isset($this->rids[$rid])) {
					$rid = $this->rids[$rid];
				}
				// check if RID has to be calculated
				if (($rid == "") || (!isset($rid))) {
					$ridBase = $nameToRIDBase[$rawAccounts[$i][$ids['sambaGroupMapping_domain']]];
					$partialAccounts[$i]['sambaSID'] = $domSID . "-" . (($partialAccounts[$i]['gidNumber'] * 2) + $ridBase + 1);
				}
				elseif (get_preg($rid, 'digit')) {
					$partialAccounts[$i]['sambaSID'] = $domSID . "-" . $rid;
				}
			}
			// display name (UTF-8, no regex check needed)
			if ($rawAccounts[$i][$ids['sambaGroupMapping_name']] == "") {
				$partialAccounts[$i]['displayName'] = $partialAccounts[$i]['cn'];
			}
			else {
				$partialAccounts[$i]['displayName'] = $rawAccounts[$i][$ids['sambaGroupMapping_name']];
			}
		}
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'sambaGroupMapping';
		}
		$return = new htmlResponsiveRow();
		if (in_array('sambaGroupMapping', $this->attributes['objectClass'])) {
			$sambaDomains = $this->getDomains();
			if (sizeof($sambaDomains) == 0) {
				StatusMessage("ERROR", _('No Samba 3 domains found in LDAP! Please create one first.'), '');
				return [];
			}
			// Get Domain-SID from group SID
			if (isset($this->attributes['sambaSID'][0])) {
				$domainSID = substr($this->attributes['sambaSID'][0], 0, strrpos($this->attributes['sambaSID'][0], "-"));
			}
			for ($i = 0; $i < count($sambaDomains); $i++) {
				// List with all valid domains
				$sambaDomainNames[] = $sambaDomains[$i]->name;
				if (isset($domainSID) && ($domainSID == $sambaDomains[$i]->SID)) {
					$SID = $sambaDomains[$i]->SID;
					$sel_domain = $sambaDomains[$i]->name;
				}
			}
			// display name
			$displayName = '';
			if (isset($this->attributes['displayName'][0])) {
				$displayName = $this->attributes['displayName'][0];
			}
			$displayNameInput = new htmlResponsiveInputField(_('Display name'), 'displayName', $displayName, 'displayName');
			$displayNameInput->setFieldMaxLength(50);
			$return->add($displayNameInput, 12);
			// Windows group
			$options = [$this->getCn()];
			$selected = [$this->getCn()];
			$names = array_keys($this->rids);
			for ($i = 0; $i < count($names); $i++) {
				if (isset($this->attributes['sambaSID'][0]) && ($this->attributes['sambaSID'][0] == $SID . "-" . $this->rids[$names[$i]])) {
					$selected = [$names[$i]];
				}
				$options[] = $names[$i];
			}
			$return->add(new htmlResponsiveSelect('sambaSID', $options, $selected, _('Windows group'), 'sambaSID'), 12);
			// group type
			$names = array_keys($this->sambaGroupTypes);
			$selected = [_('Domain group')];
			for ($i = 0; $i < count($names); $i++) {
				if (!isset($this->attributes['sambaGroupType'][0])) {
					break;
				}
				if ($this->attributes['sambaGroupType'][0] == $this->sambaGroupTypes[$names[$i]]) {
					$selected = [$names[$i]];
				}
			}
			$return->add(new htmlResponsiveSelect('sambaGroupType', $names, $selected, _('Group type'), 'type'), 12);
			// domain
			$selectedDomain = [];
			if (isset($sel_domain)) {
				$selectedDomain = [$sel_domain];
			}
			$return->add(new htmlResponsiveSelect('sambaDomainName', $sambaDomainNames, $selectedDomain, _('Domain'), 'sambaDomainName'), 12);
			// local group members
			$memberLabel = new htmlOutputText(_('Local members'));
			$return->addLabel($memberLabel);
			$addMemberButton = new htmlAccountPageButton(static::class, 'members', 'open', 'add.svg', true);
			$addMemberButton->setTitle(_('Add'));
			$addMemberButton->alignment = htmlElement::ALIGN_TOP;
			$memberHelp = new htmlHelpLink('sambaSIDList');
			if (!empty($this->attributes['sambaSIDList'][0])) {
				$this->loadSIDCache();
				$memberTable = new htmlTable();
				$memberTable->alignment = htmlElement::ALIGN_TOP;
				for ($i = 0; $i < sizeof($this->attributes['sambaSIDList']); $i++) {
					$member = $this->attributes['sambaSIDList'][$i];
					if (isset($this->sidCache[$member])) {
						$member = $this->sidCache[$member];
					}
					$memberTable->addElement(new htmlOutputText($member));
					$delButton = new htmlButton('sambaSIDListDel_' . $i, 'del.svg', true);
					$delButton->setTitle(_('Delete'));
					$memberTable->addElement($delButton);
					if ($i == (sizeof($this->attributes['sambaSIDList']) - 1)) {
						$memberTable->addElement($addMemberButton);
						$memberTable->addElement($memberHelp);
					}
					$memberTable->addNewLine();
				}
				$return->addField($memberTable);
			}
			else {
				$memberGroup = new htmlGroup();
				$memberGroup->addElement($addMemberButton);
				$memberGroup->addElement($memberHelp);
				$return->addField($memberGroup);
			}

			$return->addVerticalSpacer('2rem');

			$remButton = new htmlButton('remObjectClass', _('Remove Samba 3 extension'));
			$remButton->setCSSClasses(['lam-danger']);
			$return->add($remButton, 12, 12, 12, 'text-center');
		}
		else {
			$return->add(new htmlButton('addObjectClass', _('Add Samba 3 extension')), 12);
		}
		return $return;
	}

	/**
	 * This function will create the meta HTML code to show a page to add members.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_members() {
		$return = new htmlResponsiveRow();
		$typeManager = new TypeManager();
		// show list of possible new members
		if ((isset($_POST['form_subpage_' . static::class . '_members_select']) || isset($_POST['setFilter'])) && isset($_POST['type'])) {
			$type = $typeManager->getConfiguredType($_POST['type']);
			$this->loadSIDCache();
			$options = [];
			$filter = get_ldap_filter($type->getId());
			$entries = searchLDAP($type->getSuffix(), $filter, ['dn', 'cn', 'uid', 'sambaSID']);
			$entryCount = sizeof($entries);
			for ($i = 0; $i < $entryCount; $i++) {
				// require SID
				if (empty($entries[$i]['sambasid'][0])) {
					continue;
				}
				$sid = $entries[$i]['sambasid'][0];
				// get label
				if (!empty($this->sidCache[$sid])) {
					$label = $this->sidCache[$sid];
				}
				else {
					$label = $sid;
				}
				if (empty($this->attributes['sambaSIDList'][0]) || !in_array($sid, $this->attributes['sambaSIDList'])) {
					$options[$label] = $sid;
				}
			}
			$size = 20;
			if (sizeof($options) < 20) {
				$size = sizeof($options);
			}
			$membersSelect = new htmlSelect('members', $options, [], $size);
			$membersSelect->setHasDescriptiveElements(true);
			$membersSelect->setMultiSelect(true);
			$membersSelect->setTransformSingleSelect(false);
			$return->add($membersSelect, 12);
			$return->addVerticalSpacer('0.5rem');
			$filterGroup = new htmlGroup();
			$filterInput = new htmlInputField('newFilter');
			$filterInput->filterSelectBox('members');
			$filterGroup->addElement(new htmlOutputText(_('Filter')));
			$filterGroup->addElement($filterInput);
			$filterGroup->addElement(new htmlHiddenInput('type', $type->getId()));
			$return->add($filterGroup, 12);
			$return->addVerticalSpacer('2rem');
			$return->addLabel(new htmlAccountPageButton(static::class, 'attributes', 'addMembers', _('Add')));
			$return->addField(new htmlAccountPageButton(static::class, 'attributes', 'cancel', _('Cancel')));
			return $return;
		}
		$types = $typeManager->getConfiguredTypesForScopes(['user', 'group', 'host']);
		$options = [];
		$optionsSelected = [];
		foreach ($types as $type) {
			$options[$type->getAlias()] = $type->getId();
			if (empty($optionsSelected) && ($type->getScope() == 'group')) {
				$optionsSelected[] = $type->getId();
			}
		}
		$return->addLabel(new htmlOutputText(_('Add entries of this type:')));
		$typeSelect = new htmlSelect('type', $options, $optionsSelected);
		$typeSelect->setHasDescriptiveElements(true);
		$return->addField($typeSelect);
		$return->addLabel(new htmlOutputText('&nbsp;', false));
		$return->addField(new htmlAccountPageButton(static::class, 'members', 'select', _('Ok')));
		$return->addVerticalSpacer('2rem');
		$return->add(new htmlAccountPageButton(static::class, 'attributes', 'membersBack', _('Back')), 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	 * Processes user input of the members page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	function process_members() {
		$return = [];
		if (isset($_POST['form_subpage_' . static::class . '_attributes_addMembers']) && isset($_POST['members'])) {
			for ($i = 0; $i < sizeof($_POST['members']); $i++) {
				$this->attributes['sambaSIDList'][] = $_POST['members'][$i];
			}
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = [];
		$this->addSimplePDFField($return, 'gidNumber', _('GID number'));
		$this->addSimplePDFField($return, 'sambaSID', _('Windows group'));
		$this->addSimplePDFField($return, 'displayName', _('Display name'));
		$this->addSimplePDFField($return, 'sambaGroupType', _('Samba group type'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		// local members
		if (!empty($this->attributes['sambaSIDList'][0])) {
			$this->loadSIDCache();
			$members = [];
			foreach ($this->attributes['sambaSIDList'] as $member) {
				if (!empty($this->sidCache[$member])) {
					$members[] = $this->sidCache[$member];
				}
				else {
					$members[] = $member;
				}
			}
			$this->addPDFKeyValue($return, 'sambaSIDList', _('Local members'), implode(', ', $members));
		}
		return $return;
	}


	/**
	 * {@inheritDoc}
	 */
	function get_profileOptions($typeId) {
		$return = new htmlResponsiveRow();
		// get list of domains
		$sambaDomains = $this->getDomains();
		$sambaDomainNames = [];
		for ($i = 0; $i < count($sambaDomains); $i++) {
			// extract names
			$sambaDomainNames[] = $sambaDomains[$i]->name;
		}
		// domain
		$return->add(new htmlResponsiveSelect('sambaGroupMapping_sambaDomainName', $sambaDomainNames, null, _('Domain'), 'sambaDomainName'), 12);
		return $return;
	}


	/**
	 * Loads the values of an account profile into internal variables.
	 *
	 * @param array $profile hash array with profile values (identifier => value)
	 */
	function load_profile($profile) {
		if (isset($profile['sambaGroupMapping_sambaDomainName'][0])) {
			// get list of domains
			$sambaDomains = $this->getDomains();
			for ($i = 0; $i < sizeof($sambaDomains); $i++) {
				if ($sambaDomains[$i]->name == $profile['sambaGroupMapping_sambaDomainName'][0]) {
					$this->attributes['sambaSID'][0] = $sambaDomains[$i]->SID . "-0";
					break;
				}
			}
		}
	}

	/** this function fills the error message array with messages
	 **/
	function load_Messages() {
		$this->messages['sambaSID'][0] = ['ERROR', _('There can be only one group of this type.')];  // third parameter must be set dynamically
		$this->messages['sambaSID'][1] = ['ERROR', _("Account %s:") . " (sambaGroupMapping_domain): " . _("LAM was unable to find a Samba 3 domain with this name!")];  // third parameter must be set dynamically
		$this->messages['groupType'][0] = ['ERROR', _("Account %s:") . " (sambaGroupMapping_type): " . _("This is not a valid Samba 3 group type!"), _("Possible values") . ": %s"];
		$this->messages['primaryGroup'][0] = ['ERROR', _('There are still users who have this group as their primary group.')];
	}


	/**
	 * This function is used to check if this module page can be displayed.
	 * It returns false if a module depends on data from other modules which was not yet entered.
	 *
	 * @return boolean true, if page can be displayed
	 */
	function module_ready() {
		if (($this->getGID() == null) || ($this->getGID() == '')) {
			return false;
		}
		return true;
	}

	/**
	 * This function is used to check if all settings for this module have been made.
	 *
	 * @return boolean true, if settings are complete
	 * @see baseModule::module_complete
	 *
	 */
	public function module_complete() {
		if (!in_array('sambaGroupMapping', $this->attributes['objectClass'])) {
			return true;
		}
		if (!isset($this->attributes['sambaSID']) || ($this->attributes['sambaSID'] == '')) {
			return false;
		}
		return true;
	}

	/**
	 * Processes user input of the primary module page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array list of info/error messages
	 */
	function process_attributes() {
		// remove extension
		if (isset($_POST['remObjectClass'])) {
			// check for users that have this group as primary group
			$result = searchLDAPByAttribute('sambaPrimaryGroupSID', $this->attributes['sambaSID'][0], 'sambaSamAccount', ['dn'], ['user', 'host']);
			if (sizeof($result) > 0) {
				return [$this->messages['primaryGroup'][0]];
			}
			// remove attributes
			$this->attributes['objectClass'] = array_delete(['sambaGroupMapping'], $this->attributes['objectClass']);
			$attrKeys = array_keys($this->attributes);
			for ($k = 0; $k < sizeof($attrKeys); $k++) {
				if (strpos($attrKeys[$k], 'samba') > -1) {
					unset($this->attributes[$attrKeys[$k]]);
				}
			}
			if (isset($this->attributes['displayName'])) {
				unset($this->attributes['displayName']);
			}
			return [];
		}
		if (!in_array('sambaGroupMapping', $this->attributes['objectClass'])) {
			return [];
		}
		$errors = [];
		$sambaDomains = $this->getDomains();
		if (sizeof($sambaDomains) == 0) {
			return [];
		}
		// Save attributes
		$this->attributes['displayName'][0] = $_POST['displayName'];
		$this->attributes['sambaGroupType'][0] = $this->sambaGroupTypes[$_POST['sambaGroupType']];

		// Get Domain SID from name
		for ($i = 0; $i < count($sambaDomains); $i++) {
			if (!isset($_POST['sambaDomainName'])) {
				break;
			}
			if ($_POST['sambaDomainName'] == $sambaDomains[$i]->name) {
				$SID = $sambaDomains[$i]->SID;
				$RIDbase = $sambaDomains[$i]->RIDbase;
			}
		}
		// Load attributes
		$this->attributes['displayName'][0] = $_POST['displayName'];
		$rids = array_keys($this->rids);
		$wrid = false;
		for ($i = 0; $i < count($rids); $i++) {
			if ($_POST['sambaSID'] == $rids[$i]) {
				$wrid = true;
				// Get Domain SID
				$this->attributes['sambaSID'][0] = $SID . "-" . $this->rids[$rids[$i]];
				// Do a check if special group is unique
				if ($this->getAccountContainer()->isNewAccount) {
					$result = searchLDAPByAttribute('sambaSID', $SID . "-" . $this->rids[$rids[$i]], 'sambaGroupMapping', ['sambaSID'], ['group']);
					if (sizeof($result) > 0) {
						$message = $this->messages['sambaSID'][0];
						$message[] = $rids[$i];
						$errors[] = $message;
					}
				}
			}
		}
		if (!$wrid) {
			$this->attributes['sambaSID'][0] = $SID . "-" . ($this->getGID() * 2 + $RIDbase + 1);
		}
		// delete local members
		foreach ($_POST as $key => $value) {
			if (strpos($key, 'sambaSIDListDel_') === 0) {
				$index = substr($key, strlen('sambaSIDListDel_'));
				unset($this->attributes['sambaSIDList'][$index]);
				$this->attributes['sambaSIDList'] = array_values($this->attributes['sambaSIDList']);
				break;
			}
		}
		// Return error-messages
		return $errors;
	}


	/**
	 * Returns a list of modifications which have to be made to the LDAP account.
	 *
	 * @return array list of modifications
	 * <br>This function returns an array with 3 entries:
	 * <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	 * <br>DN is the DN to change. It may be possible to change several DNs (e.g. create a new user and add him to some groups via attribute memberUid)
	 * <br>"add" are attributes which have to be added to LDAP entry
	 * <br>"remove" are attributes which have to be removed from LDAP entry
	 * <br>"modify" are attributes which have to been modified in LDAP entry
	 * <br>"info" are values with informational value (e.g. to be used later by pre/postModify actions)
	 */
	function save_attributes() {
		if (!in_array('sambaGroupMapping', $this->attributes['objectClass']) && !in_array('sambaGroupMapping', $this->orig['objectClass'])) {
			return [];
		}
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	 * Returns a list of existing Samba 3 domains.
	 *
	 * @return array list of samba3domain objects
	 */
	private function getDomains() {
		if ($this->cachedDomainList != null) {
			return $this->cachedDomainList;
		}
		$this->cachedDomainList = search_domains();
		return $this->cachedDomainList;
	}

	/**
	 * Loads the list of Samba accounts into the cache.
	 */
	private function loadSIDCache() {
		if ($this->sidCache != null) {
			return;
		}
		$results = searchLDAPByFilter('(|(objectClass=sambaSamAccount)(objectClass=sambaGroupMapping))', ['cn', 'uid', 'sambaSID'], ['user', 'group', 'host']);
		$this->sidCache = [];
		foreach ($results as $result) {
			// require SID
			if (empty($result['sambasid'][0])) {
				continue;
			}
			// get label
			if (isset($result['cn'][0])) {
				$label = $result['cn'][0];
			}
			elseif (isset($result['uid'][0])) {
				$label = $result['uid'][0];
			}
			else {
				$label = $result['sambasid'][0];
			}
			$this->sidCache[$result['sambasid'][0]] = $label;
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'gidnumber' => _('GID number'),
			'sambasid' => _('Windows group'),
			'displayname' => _('Display name'),
			'sambagrouptype' => _('Samba group type'),
			'description' => _('Description'),
			'sambasidlist' => _('Local members'),
		];
	}

}
