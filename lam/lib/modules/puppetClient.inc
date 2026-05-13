<?php

use LAM\PDF\PDFTable;
use LAM\PDF\PDFTableCell;
use LAM\PDF\PDFTableRow;
use LAM\TYPES\ConfiguredType;

/*

  This code is not yet part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2012 - 2026  Roland Gruber

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
 * Manages Puppet configuration options.
 *
 * @package modules
 * @author Roland Gruber
 */

/**
 * Manages Puppet configuration options.
 *
 * @package modules
 */
class puppetClient extends baseModule {

	/** @var string[]|null $classCache cache for classes */
	private ?array $classCache = null;
	/** @var string[]|null $variablesCache cache for variables */
	private ?array $variablesCache = null;

	/**
	 * Creates a new authorizedServiceObject object.
	 *
	 * @param string $scope account type (user, group, host)
	 */
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}


	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'host';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'computer.svg';
		// alias name
		$return["alias"] = _("Puppet");
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['puppetClient'];
		// managed attributes
		$return['attributes'] = ['environment', 'parentnode', 'puppetclass', 'puppetvar'];
		// help Entries
		$return['help'] = [
			'environment' => [
				"Headline" => _('Environment'), 'attr' => 'environment',
				"Text" => _('Please enter the environment name for this node (e.g. production).')
			],
			'environmentList' => [
				"Headline" => _('Environment'), 'attr' => 'environment',
				"Text" => _('Please enter the environment name for this node (e.g. production).') . ' ' . _('Multiple values are separated by semicolon.')
			],
			'parentnode' => [
				"Headline" => _('Parent node'), 'attr' => 'parentnode',
				"Text" => _('This is this node\'s parent. All classes and variables are inherited from this node.')
			],
			'puppetclass' => [
				"Headline" => _('Classes'), 'attr' => 'puppetclass',
				"Text" => _('The list of configured Puppet classes for this node (e.g. ntp).')
			],
			'puppetclassList' => [
				"Headline" => _('Classes'), 'attr' => 'puppetclass',
				"Text" => _('The list of configured Puppet classes for this node (e.g. ntp).') . ' ' . _('Multiple values are separated by semicolon.')
			],
			'puppetvar' => [
				"Headline" => _('Variables'), 'attr' => 'puppetvar',
				"Text" => _('Please enter your Puppet variables for this node (e.g. config_exim=true).')
			],
			'puppetvarList' => [
				"Headline" => _('Variables'), 'attr' => 'puppetvar',
				"Text" => _('Please enter your Puppet variables for this node (e.g. config_exim=true).') . ' ' . _('Multiple values are separated by semicolon.')
			],
			'autoAdd' => [
				"Headline" => _('Automatically add this extension'),
				"Text" => _('This will enable the extension automatically if this profile is loaded.')
			],
			'predefinedClasses' => [
				"Headline" => _('Predefined classes'),
				"Text" => _('These classes will be available as autocompletion hints when adding new classes.')
			],
			'predefinedEnvironments' => [
				"Headline" => _('Predefined environments'),
				"Text" => _('These environments will be available as autocompletion hints when setting the environment.')
			],
			'enforceClasses' => [
				"Headline" => _('Enforce classes'),
				"Text" => _('If set then only these classes are allowed. One class per line.')
			],
		];
		// upload fields
		$allowedClasses = '';
		if (!empty($this->moduleSettings['puppetClient_enforceClasses'][0])) {
			$allowedClasses = implode(', ', $this->moduleSettings['puppetClient_enforceClasses']);
		}
		$return['upload_columns'] = [
			[
				'name' => 'puppetClient_environment',
				'description' => _('Environment'),
				'help' => 'environmentList',
				'example' => 'production'
			],
			[
				'name' => 'puppetClient_parentnode',
				'description' => _('Parent node'),
				'help' => 'parentnode',
				'example' => 'basenode'
			],
			[
				'name' => 'puppetClient_puppetclass',
				'description' => _('Classes'),
				'help' => 'puppetclassList',
				'example' => 'ntp;exim',
				'values' => $allowedClasses
			],
			[
				'name' => 'puppetClient_puppetvar',
				'description' => _('Variables'),
				'help' => 'puppetvarList',
				'example' => 'config_exim=true;config_exim_trusted_users=root'
			],
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'environment' => _('Environment'),
			'parentnode' => _('Parent node'),
			'puppetclass' => _('Classes'),
			'puppetvar' => _('Variables'),
		];
		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputCheckbox('puppetClient_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		$profileEnvironment = new htmlResponsiveInputField(_('Environment'), 'puppetClient_environment', null, 'environment');
		$autocompleteEnvironment = [];
		if (isset($this->moduleSettings['puppetClient_environments'])) {
			$autocompleteEnvironment = $this->moduleSettings['puppetClient_environments'];
		}
		$profileEnvironment->enableAutocompletion($autocompleteEnvironment);
		$profileContainer->add($profileEnvironment);
		$profileContainer->add(new htmlResponsiveInputTextarea('puppetClient_puppetclass', '', 60, 5, _('Classes'), 'puppetclass'));
		$profileContainer->add(new htmlResponsiveInputTextarea('puppetClient_puppetvar', '', 60, 5, _('Variables'), 'puppetvar'));
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['puppetClient_environment'] = ['type' => 'ext_preg', 'regex' => 'ascii', 'error_message' => $this->messages['environment'][0]];
		// profile mappings
		$return['profile_mappings'] = [
			'puppetClient_environment' => 'environment',
		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['environment'][0] = ['ERROR', _('The environment name may only contain ASCII characters.')];
		$this->messages['environment'][1] = ['ERROR', _('Account %s:') . ' puppetClient_environment', _('The environment name may only contain ASCII characters.')];
		$this->messages['puppetclass'][0] = ['ERROR', _('The class names may only contain ASCII characters.')];
		$this->messages['puppetclass'][1] = ['ERROR', _('Account %s:') . ' puppetClient_puppetclass', _('The class names may only contain ASCII characters.')];
		$this->messages['puppetclass'][2] = ['ERROR', _('Account %s:') . ' puppetClient_puppetclass', _('The class names contain invalid values.')];
		$this->messages['puppetvar'][0] = ['ERROR', _('The variables may only contain ASCII characters.')];
		$this->messages['puppetvar'][1] = ['ERROR', _('Account %s:') . ' puppetClient_puppetvar', _('The variables may only contain ASCII characters.')];
		$this->messages['parentnode'][0] = ['ERROR', _('Account %s:') . ' puppetClient_parentnode', _('Parent node not found.')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		if (!in_array('puppetClient', $this->attributes['objectClass']) && !in_array('puppetClient', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return [];
		}
		return parent::save_attributes();
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['form_subpage_puppetClient_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'puppetClient';
		}
		$return = new htmlResponsiveRow();
		if (!in_array('puppetClient', $this->attributes['objectClass'])) {
			$return->add(new htmlAccountPageButton('puppetClient', 'attributes', 'addObjectClass', _('Add Puppet extension')));
			return $return;
		}
		$this->initCache();
		// environment
		$autocompleteEnvironment = [];
		if (isset($this->moduleSettings['puppetClient_environments'])) {
			$autocompleteEnvironment = $this->moduleSettings['puppetClient_environments'];
		}
		$environments = [];
		if (isset($this->attributes['environment'][0])) {
			$environments = $this->attributes['environment'];
		}
		if (count($environments) === 0) {
			$environments[] = '';
		}
		$environmentLabel = new htmlOutputText(_('Environment'));
		$return->addLabel($environmentLabel);
		$environmentContainer = new htmlGroup();
		for ($i = 0; $i < count($environments); $i++) {
			$environmentField = new htmlInputField('environment' . $i, $environments[$i]);
			$environmentField->enableAutocompletion($autocompleteEnvironment);
			$environmentContainer->addElement($environmentField);
			if ($i < (count($environments) - 1)) {
				$environmentContainer->addElement(new htmlLineBreak());
			}
			else {
				$environmentContainer->addElement(new htmlButton('addEnvironment', 'add.svg', true));
				$environmentContainer->addElement(new htmlHelpLink('environment'));
			}
		}
		$return->addField($environmentContainer);
		// parent node
		$possibleParentNodes = $this->getPossibleParentNodes();
		array_unshift($possibleParentNodes, '-');
		$parentnode = '-';
		if (isset($this->attributes['parentnode'][0])) {
			$parentnode = $this->attributes['parentnode'][0];
		}
		$return->add(new htmlResponsiveSelect('parentnode', $possibleParentNodes, [$parentnode], _('Parent node'), 'parentnode'));
		// classes
		if (empty($this->moduleSettings['puppetClient_enforceClasses'][0])) {
			$this->addMultiValueInputTextField($return, 'puppetclass', _('Classes'), false, null, false, $this->classCache);
		}
		else {
			$this->addMultiValueSelectField($return, 'puppetclass', _('Classes'), $this->moduleSettings['puppetClient_enforceClasses']);
		}
		// variables
		$this->addMultiValueInputTextField($return, 'puppetvar', _('Variables'), false, null, false, $this->variablesCache);

		$return->addVerticalSpacer('2rem');
		$remButton = new htmlAccountPageButton('puppetClient', 'attributes', 'remObjectClass', _('Remove Puppet extension'));
		$remButton->setCSSClasses(['lam-danger']);
		$return->add($remButton, 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		if (isset($_POST['form_subpage_puppetClient_attributes_remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(['puppetClient'], $this->attributes['objectClass']);
			for ($i = 0; $i < count($this->meta['attributes']); $i++) {
				if (isset($this->attributes[$this->meta['attributes'][$i]])) {
					unset($this->attributes[$this->meta['attributes'][$i]]);
				}
			}
			return [];
		}
		if (!in_array('puppetClient', $this->attributes['objectClass'])) {
			return [];
		}

		$errors = [];
		// environment
		$environmentCounter = 0;
		while (isset($_POST['environment' . $environmentCounter])) {
			$this->attributes['environment'][$environmentCounter] = $_POST['environment' . $environmentCounter];
			if (!get_preg($this->attributes['environment'][$environmentCounter], 'ascii')) {
				$errors[] = $this->messages['environment'][0];
			}
			if ($this->attributes['environment'][$environmentCounter] == '') {
				unset($this->attributes['environment'][$environmentCounter]);
			}
			$environmentCounter++;
		}
		if (isset($_POST['addEnvironment'])) {
			$this->attributes['environment'][] = '';
		}
		$this->attributes['environment'] = array_values($this->attributes['environment']);
		// parent node
		if (isset($this->attributes['parentnode'][0]) && ($_POST['parentnode'] == '-')) {
			unset($this->attributes['parentnode'][0]);
		}
		elseif ($_POST['parentnode'] != '-') {
			$this->attributes['parentnode'][0] = $_POST['parentnode'];
		}
		// classes
		if (empty($this->moduleSettings['puppetClient_enforceClasses'][0])) {
			$this->processMultiValueInputTextField('puppetclass', $errors, 'ascii');
		}
		else {
			$this->processMultiValueSelectField('puppetclass');
		}
		// variables
		$this->processMultiValueInputTextField('puppetvar', $errors, 'ascii');

		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// add object class
			if (!in_array("puppetClient", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "puppetClient";
			}
			// parent node
			if ($rawAccounts[$i][$ids['puppetClient_parentnode']] != "") {
				if (!in_array($rawAccounts[$i][$ids['puppetClient_parentnode']], $this->getPossibleParentNodes())) {
					$messages[] = $this->messages['parentnode'][0];
				}
				else {
					$partialAccounts[$i]['parentnode'][0] = $rawAccounts[$i][$ids['puppetClient_parentnode']];
				}
			}
			// environment
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'puppetClient_environment', 'environment', 'ascii', $this->messages['environment'][1], $messages, '/;[ ]*/');
			// classes
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'puppetClient_puppetclass', 'puppetclass', 'ascii', $this->messages['puppetclass'][1], $messages, '/;[ ]*/');
			if (!empty($this->moduleSettings['puppetClient_enforceClasses'][0]) && !empty($partialAccounts[$i]['puppetclass'])) {
				$invalidClassFound = false;
				foreach ($partialAccounts[$i]['puppetclass'] as $singleClass) {
					if (!in_array($singleClass, $this->moduleSettings['puppetClient_enforceClasses'])) {
						$invalidClassFound = true;
						break;
					}
				}
				if ($invalidClassFound) {
					$msg = $this->messages['puppetclass'][2];
					$msg[] = $i;
					$messages[] = $msg;
				}
			}
			// variables
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'puppetClient_puppetvar', 'puppetvar', 'ascii', $this->messages['puppetvar'][1], $messages, '/;[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		if (!in_array('puppetClient', $this->attributes['objectClass'])) {
			return $return;
		}
		$this->addSimplePDFField($return, 'environment', _('Environment'));
		$this->addSimplePDFField($return, 'parentnode', _('Parent node'));
		if (isset($this->attributes['puppetclass'][0])) {
			$pdfTable = new PDFTable(_('Classes'));
			for ($i = 0; $i < count($this->attributes['puppetclass']); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($this->attributes['puppetclass'][$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'puppetclass', $pdfTable);
		}
		if (isset($this->attributes['puppetvar'][0])) {
			$pdfTable = new PDFTable(_('Variables'));
			for ($i = 0; $i < count($this->attributes['puppetvar']); $i++) {
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($this->attributes['puppetvar'][$i]);
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'puppetvar', $pdfTable);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_profileOptions($typeId) {
		$return = parent::get_profileOptions($typeId);
		$possibleParentNodes = $this->getPossibleParentNodes();
		array_unshift($possibleParentNodes, '-');
		$possibleParentNodes = array_values($possibleParentNodes);
		$return->add(new htmlResponsiveSelect('puppetClient_parentnode', $possibleParentNodes, [], _('Parent node'), 'parentnode'));
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['puppetClient_addExt'][0])
			&& ($profile['puppetClient_addExt'][0] == "true")
			&& !in_array('puppetClient', $this->attributes['objectClass'])) {
			$this->attributes['objectClass'][] = 'puppetClient';
		}
		// parent node
		if (isset($profile['puppetClient_parentnode'][0]) && ($profile['puppetClient_parentnode'][0] != "-")) {
			$this->attributes['parentnode'][0] = $profile['puppetClient_parentnode'][0];
		}
		// classes
		if (isset($profile['puppetClient_puppetclass'][0]) && ($profile['puppetClient_puppetclass'][0] != '')) {
			$this->attributes['puppetclass'] = $profile['puppetClient_puppetclass'];
		}
		// variables
		if (isset($profile['puppetClient_puppetvar'][0]) && ($profile['puppetClient_puppetvar'][0] != '')) {
			$this->attributes['puppetvar'] = $profile['puppetClient_puppetvar'];
		}
	}

	/**
	 * Returns a list of valid parent nodes for this node.
	 *
	 * @return string[] parent nodes (e.g. array('node1', 'node2'))
	 */
	private function getPossibleParentNodes(): array {
		$searchResult = searchLDAPByAttribute('cn', '*', 'puppetClient', ['cn'], ['host']);
		$possibleParentNodes = [];
		for ($i = 0; $i < count($searchResult); $i++) {
			if (!get_preg($searchResult[$i]['cn'][0], 'ascii')) {
				continue;
			}
			if (($this->getAccountContainer() == null) || $this->getAccountContainer()->isNewAccount
				|| (!isset($this->getAccountContainer()->attributes_orig['cn'][0])
					|| ($this->getAccountContainer()->attributes_orig['cn'][0] != $searchResult[$i]['cn'][0]))) {
				$possibleParentNodes[] = $searchResult[$i]['cn'][0];
			}
		}
		return $possibleParentNodes;
	}

	/**
	 * Loads cached data from LDAP such as classes etc.
	 */
	private function initCache(): void {
		if ($this->classCache !== null) {
			return;
		}
		$attrs = ['puppetclass', 'puppetvar'];
		$classes = [];
		$variables = [];
		$result = searchLDAPByFilter('(objectClass=puppetClient)', $attrs, [$this->get_scope()]);
		foreach ($result as $attributes) {
			if (isset($attributes['puppetclass'])) {
				foreach ($attributes['puppetclass'] as $val) {
					$classes[] = $val;
				}
			}
			if (isset($attributes['puppetvar'])) {
				foreach ($attributes['puppetvar'] as $val) {
					$variables[] = $val;
				}
			}
		}
		$this->classCache = array_values(array_unique($classes));
		$this->variablesCache = array_values(array_unique($variables));
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_configOptions(array $scopes, array $allScopes): htmlElement|array {
		$return = new htmlResponsiveRow();
		// config options
		$return->add(new htmlResponsiveInputTextarea('puppetClient_environments', "production\r\ntesting", 30, 5, _('Predefined environments'), 'predefinedEnvironments'));
		$return->add(new htmlResponsiveInputTextarea('puppetClient_enforceClasses', '', 30, 5, _('Enforce classes'), 'enforceClasses'));
		return $return;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'environment' => _('Environment'),
			'parentnode' => _('Parent node'),
			'puppetclass' => _('Classes'),
			'puppetvar' => _('Variables'),
		];
	}

}
