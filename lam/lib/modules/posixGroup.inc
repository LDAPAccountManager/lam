<?php

use LAM\TYPES\TypeManager;
use function LAM\TYPES\getScopeFromTypeId;
use LAM\TYPES\ConfiguredType;

/*

This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
Copyright (C) 2003 - 2006  Tilo Lutz
              2007 - 2026  Roland Gruber

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
 * Manages Unix accounts for groups.
 *
 * @package modules
 *
 * @author Tilo Lutz
 * @author Roland Gruber
 * @author Michael Duergner
 */

/**
 * Manages the object class "posixGroup" for groups.
 *
 * @package modules
 */
class posixGroup extends baseModule implements passwordService {

	/** change GIDs of users and hosts? */
	protected bool $changegids = false;
	/** password attribute */
	protected string $passwordAttrName = 'userPassword';
	/** @var string[]|null $cachedGIDList cache for existing GID numbers */
	private ?array $cachedGIDList = null;
	/** @var array<string, array<string, string>>|null $cachedUserToGIDList cache for existing users and their GIDs */
	private ?array $cachedUserToGIDList = null;
	/** @var string[]|null $cachedGroupNameList cache for existing groups */
	private ?array $cachedGroupNameList = null;

	/**
	 * {@inheritDoc}
	 */
	public function get_uploadColumns(array $selectedModules, ConfiguredType $type): array {
		$return = parent::get_uploadColumns($selectedModules, $type);
		$typeId = $type->getId();
		if ($this->manageCnAndDescription($selectedModules)) {
			array_unshift($return,
				[
					'name' => 'posixGroup_cn',
					'description' => _('Group name'),
					'help' => 'cn',
					'example' => _('adminstrators'),
					'required' => true,
					'unique' => true
				]
			);
			array_unshift($return,
				[
					'name' => 'posixGroup_description',
					'description' => _('Group description'),
					'help' => 'descriptionList',
					'example' => _('Administrators group')
				]
			);
		}
		if (!$this->isBooleanConfigOptionSet('posixGroup_' . $typeId . '_hidememberUid')) {
			$return[] = [
				'name' => 'posixGroup_members',
				'description' => _('Group members'),
				'help' => 'upload_members',
				'example' => _('user01,user02,user03')
			];
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$error_messages = [];
		$needAutoGID = [];
		$typeId = $type->getId();
		for ($i = 0; $i < count($rawAccounts); $i++) {
			if (!in_array("posixGroup", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "posixGroup";
			}
			if ($this->manageCnAndDescription($selectedModules)) {
				// group name
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'posixGroup_cn', 'cn', 'groupname', $this->messages['cn'][3], $error_messages);
				// description
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'posixGroup_description',
					'description', null, [], $errors, '/;[ ]*/');
			}
			// GID
			if ($rawAccounts[$i][$ids['posixGroup_gid']] == "") {
				// autoGID
				$needAutoGID[] = $i;
			}
			elseif (get_preg($rawAccounts[$i][$ids['posixGroup_gid']], 'digit')) {
				$partialAccounts[$i]['gidNumber'] = $rawAccounts[$i][$ids['posixGroup_gid']];
			}
			else {
				$errMsg = $this->messages['gidNumber'][8];
				$errMsg[] = [$i];
				$error_messages[] = $errMsg;
			}
			// group members
			if (!$this->isBooleanConfigOptionSet('posixGroup_' . $typeId . '_hidememberUid') && ($rawAccounts[$i][$ids['posixGroup_members']] != "")) {
				if (get_preg($rawAccounts[$i][$ids['posixGroup_members']], 'usernameList')) {
					$partialAccounts[$i]['memberUid'] = explode(",", $rawAccounts[$i][$ids['posixGroup_members']]);
				}
				else {
					$errMsg = $this->messages['memberUID'][0];
					$errMsg[] = $i;
					$error_messages[] = $errMsg;
				}
			}
			// password
			if ($rawAccounts[$i][$ids['posixGroup_password']] != "") {
				if (get_preg($rawAccounts[$i][$ids['posixGroup_password']], 'password')) {
					$partialAccounts[$i][$this->passwordAttrName] = pwd_hash($rawAccounts[$i][$ids['posixGroup_password']], true, $this->moduleSettings['posixAccount_pwdHash'][0]);
				}
				else {
					$error_messages[] = $this->messages['userPassword'][1];
				}
			}
		}
		// fill in autoGIDs
		if ($needAutoGID !== []) {
			$errorsTemp = [];
			$gids = $this->getNextGIDs(count($needAutoGID), $errorsTemp, $type);
			if (is_array($gids)) {
				for ($i = 0; $i < count($needAutoGID); $i++) {
					$partialAccounts[$i]['gidNumber'] = $gids[$i];
				}
			}
			else {
				$error_messages[] = $this->messages['gidNumber'][2];
			}
		}
		return $error_messages;
	}

	/**
	 * {@inheritDoc}
	 */
	public function delete_attributes(): array {
		$return = [];
		$result = searchLDAPByFilter('(&(objectClass=posixAccount)(gidNumber=' . $this->attributes['gidNumber'][0] . '))', ['dn'], ['user', 'host']);
		if (count($result) > 0) {
			$max = 5;
			if (count($result) < 5) {
				$max = count($result);
			}
			$users = [];
			for ($i = 0; $i < $max; $i++) {
				$users[] = getAbstractDN($result[$i]['dn']);
			}
			$message = $this->messages['primaryGroup'][0];
			$message[] = implode(', ', $users);
			$return[$this->getAccountContainer()->dn_orig]['errors'][] = $message;
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$typeId = $this->getAccountContainer()->get_type()->getId();
		if ($this->autoAddObjectClasses || (isset($this->attributes['objectClass']) && in_array('posixGroup', $this->attributes['objectClass']))) {
			// auto sync group members
			if ($this->isBooleanConfigOptionSet('posixGroup_autoSyncGon')) {
				$this->syncGon(true);
			}
			// group name
			if ($this->manageCnAndDescription($modules)) {
				$this->addSimpleInputTextField($return, 'cn', _("Group name"), true);
			}
			// GID number
			$gidNumberInput = $this->addSimpleInputTextField($return, 'gidNumber', _('GID number'));
			$gidNumberInput->setMinimumAndMaximumNumber();
			// description
			if ($this->manageCnAndDescription($modules)) {
				$this->addMultiValueInputTextField($return, 'description', _('Description'));
			}
			// password buttons
			if (checkIfWriteAccessIsAllowed($this->get_scope()) && isset($this->attributes[$this->passwordAttrName][0])) {
				$return->addLabel(new htmlOutputText(_('Password')));
				$pwdContainer = new htmlGroup();
				if (pwd_is_enabled($this->attributes[$this->passwordAttrName][0])) {
					$pwdContainer->addElement(new htmlButton('lockPassword', _('Lock password')));
				}
				else {
					$pwdContainer->addElement(new htmlButton('unlockPassword', _('Unlock password')));
				}
				$pwdContainer->addElement(new htmlSpacer('0.5rem', null));
				$pwdContainer->addElement(new htmlButton('removePassword', _('Remove password')));
				$return->addField($pwdContainer);
			}
			if (isset($this->orig['gidNumber'][0]) && $this->attributes['gidNumber'][0] != $this->orig['gidNumber'][0]) {
				$return->add(new htmlResponsiveInputCheckbox('changegids', $this->changegids, _('Change GID number of users and hosts'), 'changegids'));
			}
			// group members
			if (!$this->isBooleanConfigOptionSet('posixGroup_' . $typeId . '_hidememberUid')) {
				$return->addVerticalSpacer('0.5rem');
				$return->addLabel(new htmlOutputText(_("Group members")));
				$membersGroup = new htmlGroup();
				if (!$this->isBooleanConfigOptionSet('posixGroup_autoSyncGon')) {
					$membersGroup->addElement(new htmlAccountPageButton(static::class, 'user', 'open', _('Edit members')));
				}
				$membersGroup->addElement(new htmlHelpLink('members'));
				$return->addField($membersGroup);
				$return->addLabel(new htmlOutputText('&nbsp;', false));
				$users = $this->getUsers();
				$members = [];
				if (isset($this->attributes['memberUid'][0])) {
					foreach ($this->attributes['memberUid'] as $uid) {
						$members[] = (isset($users[$uid]) && isset($users[$uid]['cn'])) ? $uid . ' (' . $users[$uid]['cn'] . ')' : $uid;
					}
				}
				$members = array_unique($members);
				natcasesort($members);
				$members = array_map(htmlspecialchars(...), $members);
				$return->addField(new htmlOutputText(implode('<br>', $members), false));
			}
			// remove button
			if (!$this->autoAddObjectClasses) {
				$return->addVerticalSpacer('2rem');
				$remButton = new htmlButton('remObjectClass', _('Remove Unix extension'));
				$remButton->setCSSClasses(['lam-danger']);
				$return->add($remButton, 12, 12, 12, 'text-center');
			}
		}
		else {
			// add button
			$return->add(new htmlButton('addObjectClass', _('Add Unix extension')));
		}
		return $return;
	}


	/**
	 * Displays selections to add or remove users from current group.
	 *
	 * @return htmlElement meta HTML output
	 */
	function display_html_user() {
		$return = new htmlResponsiveRow();
		if (!isset($this->attributes['memberUid'])) {
			$this->attributes['memberUid'] = [];
		}
		// load list with all users
		$userAndGIDs = $this->getUsers();
		$users = [];
		foreach ($userAndGIDs as $user => $userAttrs) {
			if (!in_array($user, $this->attributes['memberUid'])) {
				$display = $user . ' (' . $userAttrs['cn'] . ')';
				if ($this->attributes['gidNumber'][0] == $userAttrs['gid']) {
					if (isset($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0])
						&& ($this->moduleSettings['posixAccount_primaryGroupAsSecondary'][0] == 'true')) {
						$users[$display] = $user;
					}
				}
				else {
					$users[$display] = $user;
				}
			}
		}
		natcasesort($users);
		$return->add(new htmlSubTitle(_("Group members")));

		$remUsers = $this->attributes['memberUid'];
		natcasesort($remUsers);
		$remUsersDescriptive = [];
		foreach ($remUsers as $user) {
			if (isset($userAndGIDs[$user])) {
				$remUsersDescriptive[$user . ' (' . $userAndGIDs[$user]['cn'] . ')'] = $user;
			}
			else {
				$remUsersDescriptive[$user] = $user;
			}
		}

		$this->addDoubleSelectionArea($return, _("Selected users"), _("Available users"), $remUsersDescriptive, [],
			$users, [], 'members', false, true);

		// sync from group of names
		$gon = $this->getAccountContainer()->getAccountModule('groupOfNames');
		if ($gon == null) {
			$gon = $this->getAccountContainer()->getAccountModule('groupOfUniqueNames');
		}
		if ($gon == null) {
			$gon = $this->getAccountContainer()->getAccountModule('groupOfMembers');
		}
		if ($gon != null) {
			$return->addVerticalSpacer('2rem');
			$syncButton = new htmlButton('syncGON', sprintf(_('Sync from %s'), $gon->get_alias()));
			$return->add($syncButton, 12, 12, 12, 'text-center');
			$return->addVerticalSpacer('1rem');
			$return->add(new htmlResponsiveInputCheckbox('syncGON_delete', true, _('Delete non-matching entries'), null, false));
		}
		$windows = $this->getAccountContainer()->getAccountModule('windowsGroup');
		if ($windows != null) {
			$return->addVerticalSpacer('2rem');
			$syncButton = new htmlButton('syncWindows', sprintf(_('Sync from %s'), $windows->get_alias()));
			$return->add($syncButton, 12, 12, 12, 'text-center');
			$return->addVerticalSpacer('1rem');
			$return->add(new htmlResponsiveInputCheckbox('syncWindows_delete', true, _('Delete non-matching entries'), null, false));
		}

		// back button
		$return->addVerticalSpacer('2rem');
		$return->add(new htmlAccountPageButton(static::class, 'attributes', 'back', _('Back')));

		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'group';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'tux.svg';
		if ($this->get_scope() == "group") {
			// this is a base module
			$return["is_base"] = true;
			// LDAP filter
			$return["ldap_filter"] = ['or' => "(objectClass=posixGroup)"];
		}
		// alias name
		$return["alias"] = _('Unix');
		// RDN attribute
		$return["RDN"] = ["cn" => "normal"];
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['posixGroup'];
		// LDAP aliases
		$return['LDAPaliases'] = ['commonName' => 'cn'];
		// managed attributes
		$return['attributes'] = ['gidNumber', $this->passwordAttrName, 'memberUid'];
		// profile options
		if (!$this->autoAddObjectClasses) {
			$profileContainer = new htmlResponsiveRow();
			$profileContainer->add(new htmlResponsiveInputCheckbox('posixGroup_addExt', false, _('Automatically add this extension'), 'autoAdd'));
			$return['profile_options'] = $profileContainer;
		}
		// available PDF fields
		$return['PDF_fields'] = [
			'gidNumber' => _('GID number'),
		];
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'posixGroup_gid',
				'description' => _('GID number'),
				'help' => 'gidNumber',
				'example' => '2034'
			],
			[
				'name' => 'posixGroup_password',
				'description' => _('Group password'),
				'help' => 'password',
				'example' => _('secret')
			]
		];
		// help Entries
		$return['help'] = [
			'gidNumber' => [
				"Headline" => _("GID number"), 'attr' => 'gidNumber',
				"Text" => _("If empty GID number will be generated automatically depending on your configuration settings.")
			],
			'description' => [
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Here you can enter a description for this group.")
			],
			'descriptionList' => [
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Here you can enter a description for this group.") . ' ' . _("Multiple values are separated by semicolon.")
			],
			'members' => [
				"Headline" => _("Group members"), 'attr' => 'memberUid',
				"Text" => _("Users who are member of the current group. Users who have set their primary group to this group will not be shown.")
			],
			'upload_members' => [
				"Headline" => _("Group members"), 'attr' => 'memberUid',
				"Text" => _("Users who will become member of the current group. User names are separated by semicolons.")
			],
			'password' => [
				"Headline" => _("Group password"), 'attr' => $this->passwordAttrName,
				"Text" => _("Sets the group password.")
			],
			'minMaxGID' => [
				"Headline" => _("GID number"),
				"Text" => _("These are the minimum and maximum numbers to use for group IDs when creating new group accounts. New group accounts will always get the highest number in use plus one.")
			],
			'pwdHash' => [
				"Headline" => _("Password hash type"),
				"Text" => _("LAM supports a large number of possibilities to generate the hash value of passwords. CRYPT-SHA512 and SSHA are the most common. We do not recommend to use plain text passwords unless passwords are hashed server-side.")
			],
			'cn' => [
				"Headline" => _("Group name"), 'attr' => 'cn',
				"Text" => _("Group name of the group which should be created. Valid characters are: a-z, A-Z, 0-9 and .-_ . If group name is already used group name will be expanded with a number. The next free number will be used.")
			],
			'changegids' => [
				"Headline" => _("Change GID number of users and hosts"),
				"Text" => _("The ID of this group was changed. You can update all user and host entries to the new group ID.")
			],
			'gidCheckSuffix' => [
				"Headline" => _("Suffix for GID/group name check"),
				"Text" => _("LAM checks if the entered group name and GID are unique. Here you can enter the LDAP suffix that is used to search for duplicates. By default, the account type suffix is used. You only need to change this if you use multiple server profiles with different OUs but need unique group names or GIDs.")
			],
			'gidGenerator' => [
				"Headline" => _("GID generator"),
				"Text" => _("LAM will automatically suggest UID/GID numbers. You can either use a fixed range of numbers or an LDAP entry with object class \"sambaUnixIdPool\" or \"msSFU30DomainInfo\".")
					. ' ' . _('Magic number will set a fixed value that must match your server configuration.')
			],
			'sambaIDPoolDN' => [
				"Headline" => _("Samba ID pool DN"),
				"Text" => _("Please enter the DN of the LDAP entry with object class \"sambaUnixIdPool\".")
			],
			'windowsIDPoolDN' => [
				"Headline" => _("Windows domain info DN"),
				"Text" => _("Please enter the DN of the LDAP entry with object class \"msSFU30DomainInfo\".")
			],
			'filter' => [
				"Headline" => _("Filter"),
				"Text" => _("Here you can enter a filter value. Only entries which contain the filter text will be shown.")
					. ' ' . _('The filter can be any regular expression, e.g. ".*" = any characters, "^" = line start, "$" = line end.')
			],
			'hidememberUid' => [
				"Headline" => _('Disable membership management'), 'attr' => 'memberUid',
				"Text" => _('Disables the group membership management.')
			],
			'autoAdd' => [
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			],
			'autoSyncGon' => [
				"Headline" => _("Force sync with group of names"),
				"Text" => _("This will force syncing with group of names members of the same group.")
			],
			'magicNumber' => [
				"Headline" => _("Magic number"),
				"Text" => _("Please enter the magic number you configured on server side.")
			],
		];

		return $return;
	}


	/**
	 * {@inheritDoc}
	 */
	public function get_configOptions(array $scopes, array $allScopes): htmlElement|array {
		$typeManager = new TypeManager($_SESSION['conf_config']);
		// configuration options
		$configContainer = new htmlResponsiveRow();
		$configContainer->add(new htmlSubTitle(_("Groups")));
		foreach ($allScopes[static::class] as $typeId) {
			if (count($allScopes[static::class]) > 1) {
				$title = new htmlDiv(null, new htmlOutputText($typeManager->getConfiguredType($typeId)->getAlias()));
				$title->setCSSClasses(['bold', 'responsiveLabel']);
				$configContainer->add($title, 12, 6);
				$configContainer->add(new htmlOutputText('&nbsp;', false), 0, 6);
			}
			$this->addAccountSpecificConfigOptions($configContainer, $typeId);
			$configContainer->addVerticalSpacer('2rem');
		}
		$gonModules = ['groupOfNames', 'groupOfUniqueNames', 'groupOfMembers'];
		$gonFound = false;
		foreach ($gonModules as $gonModule) {
			if (!empty($allScopes[$gonModule])) {
				foreach ($allScopes[$gonModule] as $gonTypeId) {
					if (getScopeFromTypeId($gonTypeId) === 'group') {
						$gonFound = true;
					}
				}
			}
		}
		if ($gonFound || !isset($allScopes['posixAccount'])) {
			$configContainer->add(new htmlSubTitle(_("Options")));
		}
		if ($gonFound) {
			$configContainer->add(new htmlResponsiveInputCheckbox('posixGroup_autoSyncGon', false, _('Force sync with group of names'), 'autoSyncGon'));
		}
		// display password hash option only if posixAccount module is not used
		if (!isset($allScopes['posixAccount'])) {
			$configContainer->add(new htmlResponsiveSelect('posixAccount_pwdHash', getSupportedHashTypes(), ['CRYPT-SHA512'], _("Password hash type"), 'pwdHash'));
		}
		return $configContainer;
	}

	/**
	 * Adds the account specific options to the config container.
	 *
	 * @param htmlResponsiveRow $configContainer container
	 * @param string $typeId type ID
	 */
	protected function addAccountSpecificConfigOptions($configContainer, $typeId): void {
		$genOptions = [
			_('Fixed range') => 'range',
			_('Samba ID pool') => 'sambaPool',
			_('Windows domain info') => 'windowsDomain',
			_('Magic number') => 'magicNumber'
		];
		$gidGeneratorSelect = new htmlResponsiveSelect('posixGroup_' . $typeId . '_gidGenerator', $genOptions, ['range'], _('GID generator'), 'gidGenerator');
		$gidGeneratorSelect->setHasDescriptiveElements(true);
		$gidGeneratorSelect->setTableRowsToHide([
			'range' => ['posixGroup_' . $typeId . '_sambaIDPoolDN', 'posixGroup_' . $typeId . '_windowsIDPoolDN', 'posixGroup_' . $typeId . '_magicNumber'],
			'sambaPool' => ['posixGroup_' . $typeId . '_minGID', 'posixGroup_' . $typeId . '_maxGID', 'posixGroup_' . $typeId . '_windowsIDPoolDN', 'posixGroup_' . $typeId . '_magicNumber'],
			'windowsDomain' => ['posixGroup_' . $typeId . '_minGID', 'posixGroup_' . $typeId . '_maxGID', 'posixGroup_' . $typeId . '_sambaIDPoolDN', 'posixGroup_' . $typeId . '_magicNumber'],
			'magicNumber' => ['posixGroup_' . $typeId . '_minGID', 'posixGroup_' . $typeId . '_maxGID', 'posixGroup_' . $typeId . '_windowsIDPoolDN', 'posixGroup_' . $typeId . '_sambaIDPoolDN']
		]);
		$gidGeneratorSelect->setTableRowsToShow([
			'range' => ['posixGroup_' . $typeId . '_minGID', 'posixGroup_' . $typeId . '_maxGID'],
			'sambaPool' => ['posixGroup_' . $typeId . '_sambaIDPoolDN'],
			'windowsDomain' => ['posixGroup_' . $typeId . '_windowsIDPoolDN'],
			'magicNumber' => ['posixGroup_' . $typeId . '_magicNumber']
		]);
		$configContainer->add($gidGeneratorSelect);
		$minGidInput = new htmlResponsiveInputField(_('Minimum GID number'), 'posixGroup_' . $typeId . '_minGID', null, 'minMaxGID');
		$minGidInput->setRequired(true);
		$configContainer->add($minGidInput);
		$maxGidInput = new htmlResponsiveInputField(_('Maximum GID number'), 'posixGroup_' . $typeId . '_maxGID', null, 'minMaxGID');
		$maxGidInput->setRequired(true);
		$configContainer->add($maxGidInput);
		$gidGeneratorDN = new htmlResponsiveInputField(_('Samba ID pool DN'), 'posixGroup_' . $typeId . '_sambaIDPoolDN', null, 'sambaIDPoolDN');
		$gidGeneratorDN->setRequired(true);
		$configContainer->add($gidGeneratorDN);
		$winGeneratorDN = new htmlResponsiveInputField(_('Windows domain info DN'), 'posixGroup_' . $typeId . '_windowsIDPoolDN', null, 'windowsIDPoolDN');
		$winGeneratorDN->setRequired(true);
		$configContainer->add($winGeneratorDN);
		$magicNumber = new htmlResponsiveInputField(_('Magic number'), 'posixGroup_' . $typeId . '_magicNumber', null, 'magicNumber');
		$magicNumber->setRequired(true);
		$configContainer->add($magicNumber);
		$configContainer->add(new htmlResponsiveInputField(_('Suffix for GID/group name check'), 'posixGroup_' . $typeId . '_gidCheckSuffix', '', 'gidCheckSuffix'));
		$configContainer->add(new htmlResponsiveInputCheckbox('posixGroup_' . $typeId . '_hidememberUid', false, _('Disable membership management'), 'hidememberUid'));
	}

	/**
	 * {@inheritDoc}
	 */
	public function check_configOptions(array $typeIds, array &$options): array {
		foreach ($typeIds as $typeId) {
			if ($options['posixGroup_' . $typeId . '_gidGenerator'][0] == 'range') {
				$this->meta['config_checks']['group']['posixGroup_' . $typeId . '_minGID'] = [
					'type' => 'ext_preg',
					'regex' => 'digit',
					'required' => true,
					'required_message' => $this->messages['gidNumber'][5],
					'error_message' => $this->messages['gidNumber'][5]];
				$this->meta['config_checks']['group']['posixGroup_' . $typeId . '_maxGID'] = [
					'type' => 'ext_preg',
					'regex' => 'digit',
					'required' => true,
					'required_message' => $this->messages['gidNumber'][6],
					'error_message' => $this->messages['gidNumber'][6]];
				$this->meta['config_checks']['group']['cmpGID'] = [
					'type' => 'int_greater',
					'cmp_name1' => 'posixGroup_' . $typeId . '_maxGID',
					'cmp_name2' => 'posixGroup_' . $typeId . '_minGID',
					'error_message' => $this->messages['gidNumber'][7]];
			}
			elseif ($options['posixGroup_' . $typeId . '_gidGenerator'][0] == 'sambaPool') {
				$this->meta['config_checks']['group']['posixGroup_' . $typeId . '_sambaIDPoolDN'] = [
					'type' => 'ext_preg',
					'regex' => 'dn',
					'required' => true,
					'required_message' => $this->messages['sambaIDPoolDN'][0],
					'error_message' => $this->messages['sambaIDPoolDN'][0]];
			}
			elseif ($options['posixGroup_' . $typeId . '_gidGenerator'][0] == 'windowsDomain') {
				$this->meta['config_checks']['group']['posixGroup_' . $typeId . '_windowsIDPoolDN'] = [
					'type' => 'ext_preg',
					'regex' => 'dn',
					'required' => true,
					'required_message' => $this->messages['windowsIDPoolDN'][0],
					'error_message' => $this->messages['windowsIDPoolDN'][0]];
			}
			elseif ($options['posixGroup_' . $typeId . '_gidGenerator'][0] == 'magicNumber') {
				$this->meta['config_checks']['group']['posixGroup_' . $typeId . '_magicNumber'] = [
					'type' => 'ext_preg',
					'regex' => 'digit',
					'required' => true,
					'required_message' => $this->messages['magicNumber'][0],
					'error_message' => $this->messages['magicNumber'][0]];
			}
		}
		return parent::check_configOptions($typeIds, $options);
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_profileOptions(string $typeId): ?htmlResponsiveRow {
		$parentOptions = parent::get_profileOptions($typeId);
		$typeManager = new TypeManager();
		if (!$this->manageCnAndDescription($typeManager->getConfiguredType($typeId)->getModules())) {
			return $parentOptions;
		}
		$parentOptions ??= new htmlResponsiveRow();
		$parentOptions->add(new htmlResponsiveInputTextarea('posixGroup_description', '', 30, 4, _('Description'), 'description'));
		return $parentOptions;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfFields(string $typeId): array {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->manageCnAndDescription($modules)) {
			$fields['cn'] = _('Group name');
			$fields['description'] = _('Description');
		}
		if (!$this->isBooleanConfigOptionSet('posixGroup_' . $typeId . '_hidememberUid')) {
			$fields['memberUid'] = _('Group members');
			$fields['memberUidPrimary'] = _('Group members (incl. primary members)');
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'memberUid', _('Group members'));
		$this->addSimplePDFField($return, 'cn', _('Group name'));
		$this->addSimplePDFField($return, 'gidNumber', _('GID number'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		if (in_array(static::class . '_memberUidPrimary', $pdfKeys)) {
			$members = empty($this->attributes['memberUid']) ? [] : $this->attributes['memberUid'];
			if (!empty($this->attributes['gidNumber'])) {
				$filter = "(&(&" . get_ldap_filter('user') . ")(gidNumber=" . $this->attributes['gidNumber'][0] . "))";
				$entries = searchLDAPByFilter($filter, ['uid'], ['user']);
				foreach ($entries as $entry) {
					$members[] = $entry['uid'][0];
				}
			}
			$this->addPDFKeyValue($return, 'memberUidPrimary', _('Group members'), $members);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['userPassword'][1] = ['ERROR', _('Password'), _('Password contains invalid characters. Valid characters are:') . ' a-z, A-Z, 0-9 and #*,.;:_-+!%&/|?{[()]}=@$ §°!'];
		$this->messages['gidNumber'][0] = ['INFO', _('GID number'), _('GID number has changed. Please select checkbox to change GID number of users and hosts.')];
		$this->messages['gidNumber'][2] = ['WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.')];
		$this->messages['gidNumber'][3] = ['ERROR', _('ID-Number'), _('No free ID-Number!')];
		$this->messages['gidNumber'][4] = ['ERROR', _('ID-Number'), _('ID is already in use')];
		$this->messages['gidNumber'][5] = ['ERROR', _('Minimum GID number'), _('Minimum GID number is invalid or empty!')];
		$this->messages['gidNumber'][6] = ['ERROR', _('Maximum GID number'), _('Maximum GID number is invalid or empty!')];
		$this->messages['gidNumber'][7] = ['ERROR', _('Maximum GID number'), _('Maximum GID number must be greater than minimum GID number!')];
		$this->messages['gidNumber'][8] = ['ERROR', _('Account %s:') . ' posixGroup_gid', _('GID number has to be a numeric value!')];
		$this->messages['cn'][0] = ['WARN', _('Group name'), _('You are using capital letters. This can cause problems because Windows is not case-sensitive.')];
		$this->messages['cn'][1] = ['WARN', _('Group name'), _('Group name in use. Selected next free group name.')];
		$this->messages['cn'][2] = ['ERROR', _('Group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['cn'][3] = ['ERROR', _('Account %s:') . ' posixGroup_cn', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['memberUID'][0] = ['ERROR', _('Account %s:') . ' posixGroup_members', _("This value must be a list of user names separated by semicolons.")];
		$this->messages['primaryGroup'][0] = ['ERROR', _('There are still users who have this group as their primary group.')];
		$this->messages['sambaIDPoolDN'][0] = ['ERROR', _('Samba ID pool DN'), _('This is not a valid DN!')];
		$this->messages['windowsIDPoolDN'][0] = ['ERROR', _('Windows domain info DN'), _('This is not a valid DN!')];
		$this->messages['magicNumber'][0] = ['ERROR', _('Magic number'), _('Please enter a valid number.')];
	}


	/**
	 * {@inheritDoc}
	 */
	public function getManagedAttributes(string $typeId): array {
		$attrs = parent::getManagedAttributes($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->manageCnAndDescription($modules)) {
			$attrs[] = 'cn';
			$attrs[] = 'description';
		}
		return $attrs;
	}

	/**
	 * {@inheritDoc}
	 */
	public function module_complete(): bool {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('posixGroup', $objectClasses)) {
				return true;
			}
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		if ($this->manageCnAndDescription($modules) && ($this->attributes['cn'][0] == '')) {
			return false;
		}
		return isset($this->attributes['gidNumber'][0]) && $this->attributes['gidNumber'][0] !== '';
	}


	/**
	 * {@inheritDoc}
	 */
	public function getButtonStatus(): string {
		if (!$this->getAccountContainer()->isNewAccount) {
			// check if account is based on our object class
			$objectClasses = $this->getAccountContainer()->attributes_orig['objectClass'];
			if (is_array($objectClasses) && !in_array('posixGroup', $objectClasses)) {
				return "disabled";
			}
		}
		return "enabled";
	}

	/**
	 * {@inheritdoc}
	 */
	public function getWildCardReplacements(): array {
		$replacements = [];
		// group name
		if (!empty($_POST['cn'])) {
			$replacements['group'] = $_POST['cn'];
		}
		elseif (!empty($this->attributes['cn'][0])) {
			$replacements['group'] = $this->attributes['cn'][0];
		}
		return $replacements;
	}

	/**
	 * {@inheritdoc}
	 */
	public function getWildcardTargetAttributeNames(): array {
		return ['description'];
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$this->getAccountContainer()->replaceWildcardsInPOST($this->getWildcardTargetAttributeNames());
		$errors = [];
		if (isset($_POST['addObjectClass'])) {
			if (!isset($this->attributes['objectClass'])) {
				$this->attributes['objectClass'] = [];
			}
			if (!in_array('posixGroup', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'posixGroup';
			}
			return $errors;
		}
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['posixGroup'], $this->attributes['objectClass']);
			$attrs = $this->getManagedAttributes($this->getAccountContainer()->get_type()->getId());
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		$typeId = $this->getAccountContainer()->get_type()->getId();
		// skip processing if object class is not set
		if (!$this->autoAddObjectClasses && (!isset($this->attributes['objectClass']) || !in_array('posixGroup', $this->attributes['objectClass']))) {
			return $errors;
		}
		if ($this->manageCnAndDescription($modules)) {
			$this->processMultiValueInputTextField('description', $errors);
		}
		if (isset($_POST['lockPassword'])) {
			$this->attributes[$this->passwordAttrName][0] = pwd_disable($this->attributes[$this->passwordAttrName][0]);
		}
		if (isset($_POST['unlockPassword'])) {
			$this->attributes[$this->passwordAttrName][0] = pwd_enable($this->attributes[$this->passwordAttrName][0]);
		}
		if (isset($_POST['removePassword'])) {
			unset($this->attributes[$this->passwordAttrName]);
		}
		$this->changegids = isset($_POST['changegids']);
		if (!isset($this->attributes['gidNumber'][0]) || ($this->attributes['gidNumber'][0] != $_POST['gidNumber'])) {
			// Check if GID is valid. If none value was entered, the next usable value will be inserted
			$this->attributes['gidNumber'][0] = $_POST['gidNumber'];
			if ($this->attributes['gidNumber'][0] == '') {
				// No id-number given, find free GID
				if (!isset($this->orig['gidNumber'][0])) {
					$newGID = $this->getNextGIDs(1, $errors, $this->getAccountContainer()->get_type());
					if (is_array($newGID)) {
						$this->attributes['gidNumber'][0] = $newGID[0];
					}
					else {
						$errors[] = $this->messages['gidNumber'][3];
					}
				}
				else {
					// old account -> return id-number which has been used
					$this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
				}
			}
			else {
				$gids = $this->getGIDs($this->getAccountContainer()->get_type());
				// Check manual ID
				if ($this->getAccountContainer()->isNewAccount || !isset($this->orig['gidNumber'][0]) || ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0])) {
					// check range
					if ($this->moduleSettings['posixGroup_' . $typeId . '_gidGenerator'][0] == 'range') {
						$minID = intval($this->moduleSettings['posixGroup_' . $typeId . '_minGID'][0]);
						$maxID = intval($this->moduleSettings['posixGroup_' . $typeId . '_maxGID'][0]);
						if (($this->attributes['gidNumber'][0] < $minID) || ($this->attributes['gidNumber'][0] > $maxID) || !is_numeric($this->attributes['gidNumber'][0])) {
							$errors[] = ['ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID)];
							if (isset($this->orig['gidNumber'][0])) {
								$this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
							}
							else {
								unset($this->attributes['gidNumber'][0]);
							}
						}
					}
					// id-number is in use and account is a new account
					if ((in_array($this->attributes['gidNumber'][0], $gids)) && ($this->orig['gidNumber'][0] == '')) {
						$errors[] = $this->messages['gidNumber'][4];
						unset($this->attributes['gidNumber'][0]);
					}
					// id-number is in use, account is existing account and id-number is changed
					if ((in_array($this->attributes['gidNumber'][0], $gids)) && ($this->orig['gidNumber'][0] != '') && ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0])) {
						$errors[] = $this->messages['gidNumber'][4];
						$this->attributes['gidNumber'][0] = $this->orig['gidNumber'][0];
					}
				}
			}
		}
		if ($this->manageCnAndDescription($modules)) {
			$this->attributes['cn'][0] = $_POST['cn'];
			if (preg_match('/^[A-Z]+$/', $_POST['cn'])) {
				$errors[] = $this->messages['cn'][0];
			}
			// Check if Groupname contains only valid characters
			if (!get_preg($this->attributes['cn'][0], 'groupname')) {
				$errors[] = $this->messages['cn'][2];
			}
			// Create automatic useraccount with number if original user already exists
			// Reset name to original name if new name is in use
			// Set username back to original name if new group name is in use
			if ($this->groupNameExists($this->attributes['cn'][0]) && ($this->orig['cn'][0] != '')) {
				$this->attributes['cn'][0] = $this->orig['cn'][0];
			}
			// Change gid to a new gid until a free gid is found
			else {
				while ($this->groupNameExists($this->attributes['cn'][0])) {
					// get last character of group name
					$lastchar = substr($this->attributes['cn'][0], strlen($this->attributes['cn'][0]) - 1, 1);
					// Last character is no number
					if (!preg_match('/^(\d)+$/', $lastchar)) {
						/* Last character is no number. Therefore we only have to
						* add "2" to it.
						*/
						$this->attributes['cn'][0] .= '2';
					}
					else {
						/* Last character is a number -> we have to increase the number until we've
						* found a groupname with trailing number which is not in use.
						*
						* $i will show us were we have to split groupname so we get a part
						* with the groupname and a part with the trailing number
						*/
						$i = strlen($this->attributes['cn'][0]) - 1;
						// Set $i to the last character which is a number in $account_new->general_username
						while (true) {
							if (preg_match('/^(\d)+$/', substr($this->attributes['cn'][0], $i, strlen($this->attributes['cn'][0]) - $i))) {
								$i--;
							}
							else {
								break;
							}
						}
						// increase last number with one
						$firstchars = substr($this->attributes['cn'][0], 0, $i + 1);
						$lastchars = substr($this->attributes['cn'][0], $i + 1, strlen($this->attributes['cn'][0]) - $i);
						// Put username together
						$this->attributes['cn'][0] = $firstchars . (intval($lastchars) + 1);
					}
				}
			}
			// Show warning if lam has changed group name
			if ($this->attributes['cn'][0] != $_POST['cn']) {
				$errors[] = $this->messages['cn'][1];
			}
		}
		// show info when gidnumber has changed
		if (isset($this->orig['gidNumber'][0]) && ($this->orig['gidNumber'][0] != $this->attributes['gidNumber'][0])
			&& ($this->orig['gidNumber'][0] != '') && !$this->changegids) {
			$errors[] = $this->messages['gidNumber'][0];
		}
		// Return error-messages
		return $errors;
	}


	/**
	 * Processes user input of the user selection page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return string[][] list of info/error messages
	 */
	public function process_user(): array {
		$return = [];
		if (!isset($this->attributes['memberUid'])) {
			$this->attributes['memberUid'] = [];
		}
		if (isset($_POST['members_2']) && isset($_POST['members_left'])) { // Add groups to list
			// add new group
			$this->attributes['memberUid'] = @array_merge($this->attributes['memberUid'], $_POST['members_2']);
		}
		elseif (isset($_POST['members_1']) && isset($_POST['members_right'])) { // remove groups from list
			$this->attributes['memberUid'] = array_delete($_POST['members_1'], $this->attributes['memberUid']);
		}
		// sync users
		elseif (isset($_POST['syncGON'])) {
			$return = array_merge($return, $this->syncGon());
		}
		// sync Windows
		elseif (isset($_POST['syncWindows'])) {
			$return = array_merge($return, $this->syncWindows());
		}
		return $return;
	}

	/**
	 * Syncs with group of names members.
	 *
	 * @param bool $forceDelete force deletion of members
	 * @return string[][] list of status messages
	 */
	protected function syncGon($forceDelete = false): array {
		$delete = $forceDelete || (isset($_POST['syncGON_delete']) && ($_POST['syncGON_delete'] == 'on'));
		$return = [];
		$gon = $this->getAccountContainer()->getAccountModule('groupOfNames');
		if ($gon == null) {
			$gon = $this->getAccountContainer()->getAccountModule('groupOfUniqueNames');
		}
		if ($gon == null) {
			$gon = $this->getAccountContainer()->getAccountModule('groupOfMembers');
		}
		if ($gon == null) {
			return $return;
		}
		if (!isset($this->attributes['memberUid'])) {
			$this->attributes['memberUid'] = [];
		}
		$memberDNs = $gon->getMembers();
		$users = $this->getUsers();
		$oldValues = $this->attributes['memberUid'];
		if ($delete) {
			$this->attributes['memberUid'] = [];
		}
		foreach ($memberDNs as $dn) {
			foreach ($users as $userName => $userAttrs) {
				if ($userAttrs['dn'] != $dn) {
					continue;
				}
				$this->attributes['memberUid'][] = $userName;
			}
		}
		$added = array_delete($oldValues, $this->attributes['memberUid']);
		if (!empty($added)) {
			$return[] = ['INFO', _('Added users'), htmlspecialchars(implode(', ', $added))];
		}
		if ($delete) {
			$deleted = array_delete($this->attributes['memberUid'], $oldValues);
			if (!empty($deleted)) {
				$return[] = ['INFO', _('Removed users'), htmlspecialchars(implode(', ', $deleted))];
			}
		}
		return $return;
	}

	/**
	 * Syncs with Windows.
	 *
	 * @return string[][] list of status messages
	 */
	protected function syncWindows(): array {
		$delete = isset($_POST['syncWindows_delete']) && ($_POST['syncWindows_delete'] == 'on');
		$return = [];
		$windows = $this->getAccountContainer()->getAccountModule('windowsGroup');
		if (!isset($this->attributes['memberUid'])) {
			$this->attributes['memberUid'] = [];
		}
		$windowsAttributes = $windows->getAttributes();
		$memberDNs = [];
		if (!empty($windowsAttributes['member'])) {
			$memberDNs = $windowsAttributes['member'];
		}
		$users = $this->getUsers();
		$oldValues = $this->attributes['memberUid'];
		if ($delete) {
			$this->attributes['memberUid'] = [];
		}
		foreach ($memberDNs as $dn) {
			foreach ($users as $userName => $userAttrs) {
				if ($userAttrs['dn'] != $dn) {
					continue;
				}
				$this->attributes['memberUid'][] = $userName;
			}
		}
		$added = array_delete($oldValues, $this->attributes['memberUid']);
		if (!empty($added)) {
			$return[] = ['INFO', _('Added users'), htmlspecialchars(implode(', ', $added))];
		}
		if ($delete) {
			$deleted = array_delete($this->attributes['memberUid'], $oldValues);
			if (!empty($deleted)) {
				$return[] = ['INFO', _('Removed users'), htmlspecialchars(implode(', ', $deleted))];
			}
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		// skip saving if account is based on another structural object class
		if ($this->is_base_module() && !$this->getAccountContainer()->isNewAccount && !in_array('posixGroup', $this->getAccountContainer()->attributes_orig['objectClass'])) {
			return [];
		}
		if (!in_array('posixGroup', $this->attributes['objectClass']) && !in_array('posixGroup', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		// auto sync group members
		if ($this->isBooleanConfigOptionSet('posixGroup_autoSyncGon')) {
			$this->syncGon(true);
		}
		$return = $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
		// Change gids of users and hosts?
		if ($this->changegids) {
			// find all accounts to change
			$result = searchLDAPByFilter('(&(objectClass=posixAccount)(gidNumber=' . $this->orig['gidNumber'][0] . '))', ['dn'], ['user', 'host']);
			if (count($result) > 0) {
				for ($i = 0; $i < count($result); $i++) {
					$return[$result[$i]['dn']]['modify']['gidNumber'][0] = $this->attributes['gidNumber'][0];
				}
			}
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['posixGroup_addExt'][0]) && ($profile['posixGroup_addExt'][0] == "true")
			&& !in_array('posixGroup', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'posixGroup';
		}
		if ($this->manageCnAndDescription($this->getAccountContainer()->get_type()->getModules()) && !empty($profile['posixGroup_description'][0])) {
			$this->attributes['description'] = $profile['posixGroup_description'];
		}
	}

	/**
	 * Returns one or more free GID numbers.
	 *
	 * @param integer $count Number of needed free GIDs.
	 * @param array<string|string[]>[] $errors list of error messages where errors can be added
	 * @param ConfiguredType $type account type
	 * @return int[]|null Null if no GIDs are free else an array of free GIDs.
	 */
	public function getNextGIDs($count, &$errors, $type): ?array {
		$typeId = $type->getId();
		// check if UIDs should be taken from Samba pool entry
		if (isset($this->moduleSettings['posixGroup_' . $typeId . '_gidGenerator']) && ($this->moduleSettings['posixGroup_' . $typeId . '_gidGenerator'][0] == 'sambaPool')) {
			return $this->getNextSambaPoolGIDs($count, $typeId);
		}
		// check if UIDs should be taken from domain info entry
		if (isset($this->moduleSettings['posixGroup_' . $typeId . '_gidGenerator']) && ($this->moduleSettings['posixGroup_' . $typeId . '_gidGenerator'][0] == 'windowsDomain')) {
			return $this->getNextDomainInfoGIDs($count, $typeId);
		}
		// use magic number
		if (isset($this->moduleSettings['posixGroup_' . $typeId . '_gidGenerator']) && ($this->moduleSettings['posixGroup_' . $typeId . '_gidGenerator'][0] == 'magicNumber')) {
			$return = [];
			for ($i = 0; $i < $count; $i++) {
				$return[] = intval($this->moduleSettings['posixGroup_' . $typeId . '_magicNumber'][0]);
			}
			return $return;
		}
		$ret = [];
		$minID = intval($this->moduleSettings['posixGroup_' . $typeId . '_minGID'][0]);
		$maxID = intval($this->moduleSettings['posixGroup_' . $typeId . '_maxGID'][0]);
		$gidList = $this->getGIDs($type);
		$gids = [];
		foreach ($gidList as $gid) {
			if (($gid <= $maxID) && ($gid >= $minID)) {
				$gids[] = intval($gid);  // ignore GIDs > maxID and GIDs < minID
			}
		}
		for ($i = 0; $i < $count; $i++) {
			if (count($gids) !== 0) {
				// there already are some GIDs
				// store highest id-number
				$id = $gids[count($gids) - 1];
				// Return minimum allowed id-number if all found id-numbers are too low
				if ($id < $minID) {
					$ret[] = $minID;
					$gids[] = $minID;
				}
				// return highest used id-number + 1 if it's still in valid range
				elseif ($id < $maxID) {
					$ret[] = $id + 1;
					$gids[] = $id + 1;
				}
				// find free numbers between existing ones
				else {
					$k = intval($minID);
					while (in_array($k, $gids)) {
						$k++;
					}
					if ($k > $maxID) {
						return null;
					}
					else {
						$ret[] = $k;
						$gids[] = $k;
						sort($gids, SORT_NUMERIC);
					}
					// show warning message
					$errors[] = $this->messages['gidNumber'][2];
				}
			}
			else {
				// return minimum allowed id-number if no id-numbers are found
				$ret[] = $minID;
				$gids[] = $minID;
			}
		}
		return $ret;
	}

	/**
	 * Gets the free GID numbers from an Samba pool entry in LDAP.
	 *
	 * @param integer $count number of needed free GIDs.
	 * @param string $typeId account type id
	 * @return int[]|null null if no GIDs are free else an array of free GIDs
	 */
	private function getNextSambaPoolGIDs($count, $typeId) {
		$dn = $this->moduleSettings['posixGroup_' . $typeId . '_sambaIDPoolDN'][0];
		$attrs = ldapGetDN($dn, ['gidNumber']);
		if (isset($attrs['gidnumber'][0]) && ($attrs['gidnumber'][0] != '')) {
			$newValue = intval($attrs['gidnumber'][0]) + $count;
			$ldapHandle = $_SESSION['ldap']->server();
			ldap_modify($ldapHandle, $dn, ['gidnumber' => [$newValue]]);
			logNewMessage(LOG_DEBUG, 'Updated Samba ID pool ' . $dn . ' with GID number ' . $newValue . ' and LDAP code ' . ldap_errno($ldapHandle));
			if (ldap_errno($ldapHandle) !== 0) {
				logNewMessage(LOG_NOTICE, 'Updating Samba ID pool ' . $dn . ' with GID number ' . $newValue . ' failed. ' . ldap_error($ldapHandle));
				return null;
			}
			$result = [];
			for ($i = 0; $i < $count; $i++) {
				$result[] = intval($attrs['gidnumber'][0]) + $i;
			}
			return $result;
		}
		return null;
	}

	/**
	 * Gets the free GID numbers from an Windows domain info entry in LDAP.
	 *
	 * @param integer $count number of needed free GIDs.
	 * @param string $typeId account type id
	 * @return int[]|null null if no GIDs are free else an array of free GIDs
	 */
	private function getNextDomainInfoGIDs($count, $typeId) {
		$dn = $this->moduleSettings['posixGroup_' . $typeId . '_windowsIDPoolDN'][0];
		$attrs = ldapGetDN($dn, ['msSFU30MaxGidNumber']);
		if (isset($attrs['mssfu30maxgidnumber'][0]) && ($attrs['mssfu30maxgidnumber'][0] != '')) {
			$newValue = intval($attrs['mssfu30maxgidnumber'][0]) + $count;
			$ldapHandle = $_SESSION['ldap']->server();
			ldap_modify($ldapHandle, $dn, ['mssfu30maxgidnumber' => [$newValue]]);
			logNewMessage(LOG_DEBUG, 'Updated domain info ' . $dn . ' with GID number ' . $newValue . ' and LDAP code ' . ldap_errno($ldapHandle));
			if (ldap_errno($ldapHandle) !== 0) {
				logNewMessage(LOG_NOTICE, 'Updating domain info ' . $dn . ' with GID number ' . $newValue . ' failed. ' . ldap_error($ldapHandle));
				return null;
			}
			$result = [];
			for ($i = 0; $i < $count; $i++) {
				$result[] = intval($attrs['mssfu30maxgidnumber'][0]) + $i;
			}
			return $result;
		}
		return null;
	}

	/**
	 * {@inheritDoc}
	 */
	public function managesPasswordAttributes(): bool {
		return true;
	}

	/**
	 * {@inheritDoc}
	 */
	public function supportsForcePasswordChange(): bool {
		return false;
	}

	/**
	 * {@inheritDoc}
	 */
	public function passwordChangeRequested(string $password, array $modules, bool $forcePasswordChange): array {
		if (!in_array(static::class, $modules)) {
			return [];
		}
		$this->attributes[$this->passwordAttrName][0] = pwd_hash($password, true, $this->moduleSettings['posixAccount_pwdHash'][0]);
		return [];
	}

	/**
	 * Returns a list of existing GID numbers.
	 *
	 * @param ConfiguredType $type account type
	 * @return string[] list of GID numbers
	 */
	private function getGIDs(ConfiguredType $type): array {
		if ($this->cachedGIDList !== null) {
			return $this->cachedGIDList;
		}
		$this->cachedGIDList = [];
		$attrs = ['gidNumber'];
		$filter = '(&(objectClass=posixGroup)(gidNumber=*))';
		$suffix = $type->getSuffix();
		$typeId = $type->getId();
		if (!empty($this->moduleSettings['posixGroup_' . $typeId . '_gidCheckSuffix'][0])) {
			$suffix = $this->moduleSettings['posixGroup_' . $typeId . '_gidCheckSuffix'][0];
		}
		$result = searchLDAP($suffix, $filter, $attrs);
		for ($i = 0; $i < count($result); $i++) {
			$this->cachedGIDList[] = $result[$i]['gidnumber'][0];
		}
		sort($this->cachedGIDList, SORT_NUMERIC);
		return $this->cachedGIDList;
	}

	/**
	 * Returns a list of existing users and their GID numbers and cn.
	 *
	 * @return array<string, array<string, string>> list in format array(uid => array('gid' => 123, 'cn' => 'Some user'))
	 */
	protected function getUsers(): array {
		if ($this->cachedUserToGIDList !== null) {
			return $this->cachedUserToGIDList;
		}
		$this->cachedUserToGIDList = [];
		$filter = '(&(objectClass=posixAccount)(gidNumber=*)(uid=*))';
		if ($this->isWindows()) {
			$filter = '(&(objectClass=user)(gidNumber=*)(uid=*))';
		}
		$result = searchLDAPByFilter($filter, ['uid', 'gidNumber', 'cn'], ['user']);
		$resultCount = count($result);
		for ($i = 0; $i < $resultCount; $i++) {
			$this->cachedUserToGIDList[$result[$i]['uid'][0]] = [
				'gid' => $result[$i]['gidnumber'][0],
				'cn' => $result[$i]['cn'][0],
				'dn' => $result[$i]['dn']];
		}
		logNewMessage(LOG_DEBUG, 'Found ' . $resultCount . ' Unix users.');
		return $this->cachedUserToGIDList;
	}

	/**
	 * Checks if the given group name already exists in LDAP.
	 *
	 * @param String $groupName group name
	 * @return boolean true if already exists
	 */
	private function groupNameExists($groupName) {
		return in_array($groupName, $this->getGroupNames());
	}

	/**
	 * Returns a list of all group names in LDAP.
	 *
	 * @return string[] group names
	 */
	private function getGroupNames(): array {
		if ($this->cachedGroupNameList !== null) {
			return $this->cachedGroupNameList;
		}
		$this->cachedGroupNameList = [];
		$attrs = ['cn'];
		$filter = '(&(objectClass=posixGroup)(cn=*))';
		$suffix = $this->getAccountContainer()->get_type()->getSuffix();
		$typeId = $this->getAccountContainer()->get_type()->getId();
		if (!empty($this->moduleSettings['posixGroup_' . $typeId . '_gidCheckSuffix'][0])) {
			$suffix = $this->moduleSettings['posixGroup_' . $typeId . '_gidCheckSuffix'][0];
		}
		$result = searchLDAP($suffix, $filter, $attrs);
		for ($i = 0; $i < count($result); $i++) {
			$this->cachedGroupNameList[] = $result[$i]['cn'][0];
		}
		return $this->cachedGroupNameList;
	}

	/**
	 * Returns if the Windows module is active.
	 *
	 * @return boolean is Windows
	 */
	private function isWindows() {
		return (in_array('windowsGroup', $this->getAccountContainer()->get_type()->getModules()));
	}

	/**
	 * Returns if cn and description attributes should be managed.
	 *
	 * @param string[] $modules modules
	 * @return boolean manage cn+description
	 */
	protected function manageCnAndDescription($modules) {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function supportsPasswordQuickChangePage(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function addPasswordQuickChangeAccountDetails(htmlResponsiveRow $row): void {
		// no details
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeOptions(bool $forcePasswordChangeByDefault): array {
		return [];
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeChanges(string $password): array {
		return [];
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthUserName(): ?string {
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthAttributes(): array {
		return [];
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeIsPasswordInHistory(string $password): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			"cn" => _("Name"),
			"description" => _("Group description"),
			"gidnumber" => _("GID number"),
			"memberuid" => _("Group members"),
		];
	}

}
