<?php

use LAM\TYPES\ConfiguredType;
use LAM\TYPES\TypeManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2026  Roland Gruber

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
 * Manages PyKota group accounts.
 *
 * @package modules
 * @author Roland Gruber
 */

/**
 * Manages PyKota group accounts.
 *
 * @package modules
 */
class pykotaGroup extends baseModule {

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['pykotaGroupName'];

	/** @var string[]|null $cnCache cache for cn attribute */
	private ?array $cnCache = null;
	/** @var string[]|null $pykotaGroupNameCache cache for pykotaGroupName attribute */
	private ?array $pykotaGroupNameCache = null;
	/** @var array<string, string> $limitOptions list of limit options label => value */
	private array $limitOptions;

	/**
	 * Returns if this module also manages the structural object class pykotaObject.
	 * This is overridden by a submodule that must provide the structural object class.
	 *
	 * @return bool structural usage
	 */
	public function isStructural(): bool {
		return false;
	}

	/**
	 * Creates a new pykotaGroup object.
	 *
	 * @param string $scope account type (user, group, host)
	 */
	public function __construct($scope) {
		$this->limitOptions = [
			_('Quota') => 'quota',
			_('Balance') => 'balance',
			_('No quota') => 'noquota',
			_('Free printing') => 'nochange',
			_('Deny printing') => 'noprint',
		];
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = $this->isStructural();
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
		$return['icon'] = 'printer.svg';
		// alias name
		$return["alias"] = _("PyKota");
		// this is a base module
		$return["is_base"] = $this->isStructural();
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=pykotaGroup)"];
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['pykotaGroup'];
		// managed attributes
		$return['attributes'] = ['pykotaLimitBy', 'pykotaGroupName'];
		// help Entries
		$return['help'] = [
			'cn' => [
				"Headline" => _("Common name"), 'attr' => 'cn',
				"Text" => _("Group name of the group which should be created. Valid characters are: a-z, A-Z, 0-9 and .-_ .")
			],
			'description' => [
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Group description.")
			],
			'pykotaGroupName' => [
				"Headline" => _("PyKota group name"), 'attr' => 'pykotaGroupName',
				"Text" => _("Group name that is used for PyKota.")
			],
			'pykotaLimitBy' => [
				"Headline" => _("Limit type"), 'attr' => 'pykotaLimitBy',
				"Text" => _("Specifies the type of limit for printing if any. Please note that in contrast to \"Free printing\" the option \"No quota\" includes accounting.")
			],
			'autoAdd' => [
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			],
		];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$pykotaLimitByProfileOption = new htmlResponsiveSelect('pykotaGroup_pykotaLimitBy', $this->limitOptions, [], _('Limit type'), 'pykotaLimitBy');
		$pykotaLimitByProfileOption->setHasDescriptiveElements(true);
		$pykotaLimitByProfileOption->setSortElements(false);
		$profileContainer->add($pykotaLimitByProfileOption);
		if (!$this->isStructural()) {
			$profileContainer->add(new htmlResponsiveInputCheckbox('pykotaGroup_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		}
		$return['profile_options'] = $profileContainer;
		$return['profile_mappings']['pykotaGroup_pykotaLimitBy'] = 'pykotaLimitBy';
		// upload fields
		$return['upload_columns'] = [
			[
				'name' => 'pykotaGroup_pykotaGroupName',
				'description' => _('PyKota group name'),
				'help' => 'pykotaGroupName',
				'example' => _('adminstrators'),
				'unique' => true,
			]
		];
		$return['upload_columns'][] = [
			'name' => 'pykotaGroup_pykotaLimitBy',
			'description' => _('Limit type'),
			'help' => 'pykotaLimitBy',
			'example' => _('Quota'),
			'default' => _('Quota'),
			'values' => implode(', ', array_keys($this->limitOptions))
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'pykotaGroupName' => _('PyKota group name'),
			'pykotaLimitBy' => _('Limit type'),
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['cn'][0] = ['ERROR', _('Group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['cn'][1] = ['ERROR', _('Account %s:') . ' pykotaGroup_cn', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['cn'][2] = ['ERROR', _('Group name'), _('Group name already exists!')];
		$this->messages['cn'][3] = ['ERROR', _('Account %s:') . ' pykotaGroup_cn', _('Group name already exists!')];
		$this->messages['pykotaGroupName'][0] = ['ERROR', _('PyKota group name'), _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['pykotaGroupName'][1] = ['ERROR', _('Account %s:') . ' pykotaGroup_pykotaGroupName', _('Group name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !')];
		$this->messages['pykotaGroupName'][2] = ['ERROR', _('PyKota group name'), _('Group name already exists!')];
		$this->messages['pykotaGroupName'][3] = ['ERROR', _('Account %s:') . ' pykotaGroup_pykotaGroupName', _('Group name already exists!')];
		$this->messages['pykotaLimitBy'][0] = ['ERROR', _('Account %s:') . ' pykotaGroup_pykotaLimitBy', _('Please enter a valid limit type.')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []): void {
		parent::loadAttributesFromAccountCopy($ldapAttributes, array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY));
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$container = new htmlResponsiveRow();
		$modules = $this->getAccountContainer()->get_type()->getModules();
		if ($this->isStructural() || (isset($this->attributes['objectClass']) && in_array('pykotaGroup', $this->attributes['objectClass']))) {
			// cn
			if ($this->isUnixOrGonInactive($modules)) {
				$this->addSimpleInputTextField($container, 'cn', _('Group name'), true);
			}
			// pykotaGroupName
			$this->addSimpleInputTextField($container, 'pykotaGroupName', _('Pykota group name'));
			// limit by
			$limitOption = 'quota';
			if (!empty($this->attributes['pykotaLimitBy'][0])) {
				$limitOption = $this->attributes['pykotaLimitBy'][0];
			}
			$limitSelect = new htmlResponsiveSelect('pykotaLimitBy', $this->limitOptions, [$limitOption], _('Limit type'), 'pykotaLimitBy');
			$limitSelect->setHasDescriptiveElements(true);
			$limitSelect->setSortElements(false);
			$container->add($limitSelect);
			// description
			if ($this->isUnixOrGonInactive($modules)) {
				$this->addMultiValueInputTextField($container, 'description', _('Description'), false, null, true);
			}
			// remove button
			if (!$this->isStructural()) {
				$container->addVerticalSpacer('2rem');
				$remButton = new htmlButton('remObjectClass', _('Remove PyKota extension'));
				$remButton->setCSSClasses(['lam-danger']);
				$container->add($remButton, 12, 12, 12, 'text-center');
			}
		}
		else {
			// add button
			$container->add(new htmlButton('addObjectClass', _('Add PyKota extension')));
		}
		return $container;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$errors = [];
		if (isset($_POST['addObjectClass'])) {
			if (!isset($this->attributes['objectClass'])) {
				$this->attributes['objectClass'] = [];
			}
			if (!in_array('pykotaGroup', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'pykotaGroup';
			}
			if (!isset($this->attributes['pykotaGroupName'][0])) {
				$this->attributes['pykotaGroupName'][0] = $this->getCurrentGroupName();
			}
			return $errors;
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['pykotaGroup'], $this->attributes['objectClass']);
			$attrs = ['pykotaLimitBy', 'pykotaGroupName'];
			if ($this->isUnixOrGonInactive($modules)) {
				$attrs[] = 'description';
			}
			if ($this->isUnixOrGonInactive($modules)) {
				$attrs[] = 'cn';
			}
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		// skip processing if object class is not set
		if (!isset($this->attributes['objectClass']) || !in_array('pykotaGroup', $this->attributes['objectClass'])) {
			return $errors;
		}
		// cn
		if ($this->isUnixOrGonInactive($modules)) {
			if (isset($_POST['cn']) && ($_POST['cn'] != '')) {
				if (!get_preg($_POST['cn'], 'groupname')) {
					$errors[] = $this->messages['cn'][0];
				}
				else {
					$this->attributes['cn'][0] = $_POST['cn'];
					if ((!isset($this->orig['cn'][0]) || ($this->attributes['cn'][0] != $this->orig['cn'][0]))
						&& $this->cnExists($_POST['cn'])) {
						$errors[] = $this->messages['cn'][2];
					}
				}
			}
			elseif (isset($this->attributes['cn'][0])) {
				unset($this->attributes['cn'][0]);
			}
		}
		// PyKota group name
		if (!empty($_POST['pykotaGroupName'])) {
			if (!get_preg($_POST['pykotaGroupName'], 'groupname')) {
				$errors[] = $this->messages['pykotaGroupName'][0];
			}
			else {
				$this->attributes['pykotaGroupName'][0] = $_POST['pykotaGroupName'];
				if ((!isset($this->orig['pykotaGroupName'][0]) || ($this->attributes['pykotaGroupName'][0] != $this->orig['pykotaGroupName'][0]))
					&& $this->pykotaGroupNameExists($_POST['pykotaGroupName'])) {
					$errors[] = $this->messages['pykotaGroupName'][2];
				}
			}
		}
		else {
			$this->attributes['pykotaGroupName'][0] = $this->getCurrentGroupName();
		}
		// limit by
		$this->attributes['pykotaLimitBy'][0] = $_POST['pykotaLimitBy'];
		// description
		if ($this->isUnixOrGonInactive($modules)) {
			$this->processMultiValueInputTextField('description', $errors);
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		if (!in_array('pykotaGroup', $this->attributes['objectClass']) && !in_array('pykotaGroup', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 */
	public function getManagedAttributes(string $typeId): array {
		$attrs = parent::getManagedAttributes($typeId);
		$typeManager = new TypeManager();
		if ($this->isUnixOrGonInactive($typeManager->getConfiguredType($typeId)->getModules())) {
			$attrs[] = 'cn';
			$attrs[] = 'description';
		}
		return $attrs;
	}

	/**
	 * {@inheritDoc}
	 */
	public function module_complete(): bool {
		if (in_array('pykotaGroup', $this->attributes['objectClass'])) {
			// require cn
			$cn = $this->getCurrentGroupName();
			return !empty($cn);
		}
		return true;
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['pykotaGroup_addExt'][0])
			&& ($profile['pykotaGroup_addExt'][0] == "true")
			&& !in_array('pykotaGroup', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'pykotaGroup';
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_uploadColumns(array $selectedModules, ConfiguredType $type): array {
		$return = parent::get_uploadColumns($selectedModules, $type);
		if ($this->isUnixOrGonInactive($selectedModules)) {
			$return[] = [
				'name' => 'pykotaGroup_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('adminstrators'),
				'required' => true,
			];
			$return[] = [
				'name' => 'pykotaGroup_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Administrators group'),
			];
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// add object classes
			if (!in_array('pykotaGroup', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaGroup';
			}
			if ($this->isStructural() && !in_array('pykotaObject', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaObject';
			}
			// cn
			if ($this->isUnixOrGonInactive($selectedModules) && !empty($rawAccounts[$i][$ids['pykotaGroup_cn']])) {
				if (!get_preg($rawAccounts[$i][$ids['pykotaGroup_cn']], 'groupname')) {
					$errMsg = $this->messages['cn'][1];
					$errMsg[] = [$i];
					$messages[] = $errMsg;
				}
				elseif ($this->cnExists($rawAccounts[$i][$ids['pykotaGroup_cn']])) {
					$errMsg = $this->messages['cn'][3];
					$errMsg[] = [$i];
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['pykotaGroup_cn']];
				}
			}
			// description
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'pykotaGroup_description', 'description');
			// PyKota group name
			if (!empty($rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']])) {
				if (!get_preg($rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']], 'groupname')) {
					$errMsg = $this->messages['pykotaGroupName'][1];
					$errMsg[] = [$i];
					$messages[] = $errMsg;
				}
				elseif ($this->pykotaGroupNameExists($rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']])) {
					$errMsg = $this->messages['pykotaGroupName'][3];
					$errMsg[] = [$i];
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['pykotaGroupName'] = $rawAccounts[$i][$ids['pykotaGroup_pykotaGroupName']];
				}
			}
			// limit by
			if (!empty($rawAccounts[$i][$ids['pykotaGroup_pykotaLimitBy']])) {
				if (isset($this->limitOptions[$rawAccounts[$i][$ids['pykotaGroup_pykotaLimitBy']]])) {
					$partialAccounts[$i]['pykotaLimitBy'] = $this->limitOptions[$rawAccounts[$i][$ids['pykotaGroup_pykotaLimitBy']]];
				}
				else {
					$errMsg = $this->messages['pykotaLimitBy'][0];
					$errMsg[] = [$i];
					$messages[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['pykotaLimitBy'] = 'quota';
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfFields(string $typeId): array {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->isUnixOrGonInactive($modules)) {
			$fields['cn'] = _('Common name');
			$fields['description'] = _('Description');
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$this->addSimplePDFField($return, 'cn', _('Common name'));
		$this->addSimplePDFField($return, 'pykotaGroupName', _('PyKota group name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$limitByOptions = array_flip($this->limitOptions);
		$limitByValue = '';
		if (!empty($this->attributes['pykotaLimitBy'][0]) && isset($limitByOptions[$this->attributes['pykotaLimitBy'][0]])) {
			$limitByValue = $limitByOptions[$this->attributes['pykotaLimitBy'][0]];
		}
		$this->addPDFKeyValue($return, 'pykotaLimitBy', _('Limit type'), $limitByValue);
		return $return;
	}

	/**
	 * Checks if the active modules contain Unix or group of names.
	 *
	 * @param string[] $modules modules
	 * @return bool not active
	 */
	private function isUnixOrGonInactive($modules) {
		return !in_array('posixGroup', $modules)
			&& !in_array('groupOfNames', $modules)
			&& !in_array('groupOfUniqueNames', $modules);
	}

	/**
	 * Returns if the given cn already exists.
	 *
	 * @param String $cn cn attribute value
	 * @return boolean cn exists
	 */
	private function cnExists($cn) {
		if ($this->cnCache === null) {
			$this->loadGroupNameCache();
		}
		return in_array($cn, $this->cnCache);
	}

	/**
	 * Returns if the given pykotaGroupName already exists.
	 *
	 * @param String $pykotaGroupName pykotaGroupName attribute value
	 * @return boolean pykotaGroupName exists
	 */
	private function pykotaGroupNameExists($pykotaGroupName) {
		if ($this->pykotaGroupNameCache === null) {
			$this->loadGroupNameCache();
		}
		return in_array($pykotaGroupName, $this->pykotaGroupNameCache);
	}

	/**
	 * Loads the list of group names into the cache.
	 */
	private function loadGroupNameCache(): void {
		$results = searchLDAPByFilter('(objectClass=pykotaGroup)', ['cn', 'pykotaGroupName'], [$this->get_scope()]);
		$this->cnCache = [];
		$this->pykotaGroupNameCache = [];
		foreach ($results as $result) {
			if (isset($result['cn'][0])) {
				$this->cnCache[] = $result['cn'][0];
			}
			if (isset($result['pykotagroupname'][0])) {
				$this->pykotaGroupNameCache[] = $result['pykotagroupname'][0];
			}
		}
	}

	/**
	 * Returns the current group name (cn) of this account.
	 *
	 * @return String group name
	 */
	private function getCurrentGroupName() {
		if (!empty($this->attributes['cn'][0])) {
			return $this->attributes['cn'][0];
		}
		if ($this->getAccountContainer()->getAccountModule('posixGroup') != null) {
			$posix = $this->getAccountContainer()->getAccountModule('posixGroup');
			$attrs = $posix->getAttributes();
			if (!empty($attrs['cn'][0])) {
				return $attrs['cn'][0];
			}
		}
		if ($this->getAccountContainer()->getAccountModule('groupOfNames') != null) {
			$posix = $this->getAccountContainer()->getAccountModule('groupOfNames');
			$attrs = $posix->getAttributes();
			if (!empty($attrs['cn'][0])) {
				return $attrs['cn'][0];
			}
		}
		if ($this->getAccountContainer()->getAccountModule('groupOfUniqueNames') != null) {
			$posix = $this->getAccountContainer()->getAccountModule('groupOfUniqueNames');
			$attrs = $posix->getAttributes();
			if (!empty($attrs['cn'][0])) {
				return $attrs['cn'][0];
			}
		}
		return '';
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'cn' => _('Group name'),
			'description' => _('Description'),
			'pykotagroupname' => _('PyKota group name'),
			'pykotalimitby' => _('Limit type'),
		];
	}

}
