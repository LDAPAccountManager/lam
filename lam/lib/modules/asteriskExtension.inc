<?php

use LAM\PDF\PDFTable;
use LAM\PDF\PDFTableCell;
use LAM\PDF\PDFTableRow;
use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009 - 2012  Pavel Pozdniak
                2009 - 2026  Roland Gruber

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
 * Manages Asterisk extensions.
 *
 * @package modules
 *
 * @author Pavel Pozdniak
 * @author Roland Gruber
 */

/**
 * Manages Asterisk extensions.
 *
 * @package modules
 */
class asteriskExtension extends baseModule {

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	private const ATTRIBUTES_TO_IGNORE_ON_COPY = ['AstExtension'];

	/** if true then only Asterisk accounts are shown as possible owners */
	private bool $filterOwnerForAsteriskAccounts = true;

	/** @var array<int, array<string, array<int, string>|string>> $extensionRows Contains all attributes information for set of accounts */
	private array $extensionRows = [];
	/** @var array<int, array<string, array<int, string>|string>> $extensionRowsOrig Contains all original attributes information for set of accounts */
	private array $extensionRowsOrig = [];
	/** @var array<int, string> $extensionOwners extension owners */
	private array $extensionOwners = [];
	/** flag if new rule should be added */
	private bool $addRuleFlag = false;

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return $this->get_scope() === 'asteriskExt';
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = ["cn" => "normal"];
		// LDAP filter
		$return["ldap_filter"] = ['or' => "(objectClass=AsteriskExtension)"];
		// alias name
		$return["alias"] = _("Asterisk extension");
		// module dependencies
		$return['dependencies'] = ['depends' => [], 'conflicts' => []];
		// managed object classes
		$return['objectClasses'] = ['AsteriskExtension', 'groupOfNames'];
		// managed attributes
		$return['attributes'] = ['cn', 'member', 'AstExtension', 'AstPriority',
			'AstApplication', 'AstApplicationData', 'AstContext'];
		//ldap aliases
		$return['LDAPaliases'] = ['commonName' => 'cn'];
		// icon
		$return['icon'] = 'asterisk.png';
		// help
		$return['help'] = [
			'AstApplicationData' => [
				"Headline" => _("Application data"), 'attr' => 'AstApplicationData',
				"Text" => _("This defines what application Asterisk has to start for this user (e.g. SIP/user1).")
			],
			'AstContext' => [
				"Headline" => _("Account context"), 'attr' => 'AstContext',
				"Text" => _("The account context stores information about the dial plan.")
			],
			'AstExtension' => [
				"Headline" => _("Extension name"), 'attr' => 'AstExtension',
				"Text" => _("The name of the extension (e.g. voicemail or sip).")
			],
			'AstPriority' => [
				"Headline" => _("Priority"), 'attr' => 'AstPriority',
				"Text" => _("The priority is a sequence number used to order the execution of commands. Commands are executed beginning with the lowest sequence number.")
			],
			'AstApplication' => [
				"Headline" => _("Application"), 'attr' => 'AstApplication',
				"Text" => _("The name of a command to execute. The available commands may be listed at the Asterisk command line by executing \"core show applications\".")
			],
			'member' => [
				"Headline" => _("Extension owners"), 'attr' => 'member',
				"Text" => _("DNs of the extension owners")
			],
			'cn' => [
				"Headline" => _("Common name"), 'attr' => 'cn',
				"Text" => _("Common name of the Asterisk extension.")
			],
			'ownerOptions' => [
				"Headline" => _("Options"),
				"Text" => _("These options change the list of potential extension owners below. You can select to show Asterisk accounts or all users.")
			],
		];

		// profile options
		$profileContainer = new htmlResponsiveRow();
		$profileContainer->add(new htmlResponsiveInputField(_('Account context'), 'AsteriskExtension_AstContext', null, 'AstContext'));
		$return['profile_options'] = $profileContainer;
		$return['profile_mappings'] = [
			'AsteriskExtension_AstContext' => 'AstContext',
		];
		// available PDF fields
		$return['PDF_fields'] = [
			'AstContext' => _('Account context'),
			'AstExtension' => _('Extension name'),
			'owners' => _('Extension owners'),
			'rules' => _('Rules'),
		];

		$return['upload_columns'] = [
			[
				'name' => 'asteriskExtension_AstExtension',
				'description' => _('Extension name'),
				'help' => 'AstExtension',
				'example' => '500',
				'required' => true
			],
			[
				'name' => 'asteriskExtension_AstContext',
				'description' => _('Account context'),
				'help' => 'AstContext',
				'example' => _('test'),
				'required' => true,
			],
			[
				'name' => 'asteriskExtension_owner',
				'description' => _('Extension owners'),
				'help' => 'member',
				'example' => 'uid=user1,o=test;uid=user2,o=test',
			],
			[
				'name' => 'asteriskExtension_AstApplication',
				'description' => _('Application'),
				'help' => 'AstApplication',
				'example' => 'Playback',
				'required' => true
			],
			[
				'name' => 'asteriskExtension_AstApplicationData',
				'description' => _('Application data'),
				'help' => 'AstApplicationData',
				'example' => _('test-start'),
			],

		];
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		$this->messages['cn'][0] = ['ERROR', _('Please enter a common name.')];
		$this->messages['AstApplication'][0] = ['ERROR', _('Please enter the application.')];
		$this->messages['AstApplicationData'][0] = ['ERROR', _('Please enter the application data.')];
		$this->messages['AstContext'][0] = ['ERROR', _('Please enter the account context.')];
		$this->messages['AstExtension'][0] = ['ERROR', _('Please enter the extension name.')];
		$this->messages['AstExtension'][1] = ['ERROR', _('Extension with this name already exists.')];
		$this->messages['AstPriority'][0] = ['ERROR', _('Please enter the priority.')];
		$this->messages['AstExtensionAstPriority'][0] = ['ERROR', _('This pair of extension name and priority already exists.')];
		$this->messages['member'][0] = ['ERROR', _('Please add at least one extension owner.')];

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
		$return = new htmlResponsiveRow();

		$extName = '';
		if (isset($this->attributes['AstExtension'][0])) {
			$extName = $this->attributes['AstExtension'][0];
		}
		if (isset($this->extensionRows[0]['astextension'][0])) {
			$extName = $this->extensionRows[0]['astextension'][0];
		}
		//This procedure LOAD all data from LDAP and then print to a page
		$this->render_extensions_by_priority($extName, $return);

		// owners
		//Set default owner as logged in user if no other owner is set
		if (!$this->isExtensionOwnerSet()) {
			$this->setDefaultExtensionOwner();
		}

		$this->render_exten_owners_set_controls($return);

		return $return;
	}

	/**
	 * This function prints management elements to manipulate owners of an extension.
	 *
	 * @param htmlResponsiveRow $renderContainer
	 */
	private function render_exten_owners_set_controls($renderContainer): void {
		$renderContainer->addVerticalSpacer('1rem');
		$title = new htmlSubTitle(_("Extension owners"));
		$title->setHelpId('member');
		$renderContainer->add($title);
		$ownerList = new htmlTable();
		if ((count($this->extensionOwners) === 0) && isset($this->extensionRows[0]['member'])) {
			$this->extensionOwners = $this->extensionRows[0]['member'];
		}
		for ($i = 0; $i < count($this->extensionOwners); $i++) {
			$ownerList->addElement(new htmlOutputText(getAbstractDN($this->extensionOwners[$i])), true);
		}
		$renderContainer->add($ownerList);
		$renderContainer->addVerticalSpacer('1rem');
		$renderContainer->add(new htmlAccountPageButton(static::class, 'user', 'open', _('Change')));
	}

	/**
	 * Loads all related extension entries.
	 *
	 * @param string $extension extension name
	 * @return array<int, array<string, string[]|string>> extensions
	 */
	private function load_extension_parts($extension): array {
		if (!isset($this->extensionRows[0]['astcontext'][0])) {
			$entries = searchLDAP($this->getAccountContainer()->dnSuffix, '(&(objectClass=AsteriskExtension)(AstExtension=' . $extension . '))', ['cn', 'AstContext', 'AstPriority',
				'AstApplication', 'AstApplicationData', 'AstExtension', 'member', 'ObjectClass']);

			$entries = $this->array_sort($entries, 'astpriority');
			$this->extensionRowsOrig = $entries;
			$this->extensionRows = $entries;
			if (isset($this->extensionRows[0]['member'])) {
				$this->extensionOwners = $this->extensionRows[0]['member'];
			}
		}
		else {
			$entries = $this->extensionRows;
		}
		return $entries;
	}

	/**
	 * Generates the meta HTML for the rules.
	 *
	 * @param string $extension extension name
	 * @param htmlResponsiveRow $renderContainer container
	 */
	private function render_extensions_by_priority(string $extension, $renderContainer): void {
		$entries = $this->load_extension_parts($extension);

		if ($this->getAccountContainer()->isNewAccount) {
			$suggestedExtName = $this->generateNextExtensionName();
			$extNameInput = new htmlResponsiveInputField(_("Extension name"), 'AstExtension', $suggestedExtName, 'AstExtension');
		}
		else {
			$extNameInput = new htmlResponsiveInputField(_("Extension name"), 'AstExtension', $extension, 'AstExtension');
		}
		$extNameInput->setRequired(true);
		$renderContainer->add($extNameInput);

		$accountContext = '';
		if (isset($entries[0]['astcontext'][0])) {
			//LOAD context from already entities that  already exists
			$accountContext = $entries[0]['astcontext'][0];
		}
		elseif (isset($this->attributes['AstContext'][0])) {
			//LOAD context from attributes, this is to deal with profile
			$accountContext = $this->attributes['AstContext'][0];
		}
		$accountContextInput = new htmlResponsiveInputField(_("Account context"), 'AstContext', $accountContext, 'AstContext');
		$accountContextInput->setRequired(true);
		$renderContainer->add($accountContextInput);


		$renderContainer->add(new htmlSubTitle(_('Rules')));
		$renderContainer->addVerticalSpacer('1rem');

		for ($i = 0; $i < count($entries); $i++) {
			$this->render_extension($entries[$i], $i, $renderContainer);
			$renderContainer->addLabel(new htmlButton("delete_rule_" . $i, _('Delete rule')));

			$upDownButtons = new htmlTable();
			if ($i > 0) {
				$upDownButtons->addElement(new htmlButton('rule_up_button_' . $i, 'up.svg', true));
			}
			if (($i < count($entries) - 1) &&
				((count($entries) > 1) || ($this->addRuleFlag))) {
				$upDownButtons->addElement(new htmlButton('rule_down_button_' . $i, 'down.svg', true));
			}
			$renderContainer->addField($upDownButtons);

			$renderContainer->addVerticalSpacer('2rem');
		}

		$displayEntrNum = count($entries);

		if ($this->addRuleFlag || count($entries) === 0) {

			$this->render_extension([], count($entries), $renderContainer);

			if ($this->addRuleFlag) {
				$upDownButtons = new htmlTable();
				$renderContainer->addLabel(new htmlButton("delete_rule_" . $i, _('Delete rule')));
				$upDownButtons->addElement(new htmlButton('rule_up_button_' . $i, 'up.svg', true));
				$renderContainer->addField($upDownButtons);
			}

			$displayEntrNum++;
			$this->addRuleFlag = false;
		}
		// the size of found rows plus 1 for the new one
		$renderContainer->add(new htmlHiddenInput("extension_rows", (string) $displayEntrNum));

		$renderContainer->add(new htmlButton("add_rule", _('Add another rule')));
	}

	/**
	 * Generates the meta HTML for a single rule.
	 *
	 * @param array<string, string[]|string> $extensLine attributes of rule
	 * @param int $placeInList rule position
	 * @param htmlResponsiveRow $renderContainer container
	 */
	private function render_extension($extensLine, $placeInList, $renderContainer): void {
		// application
		$application = '';
		if (isset($extensLine['astapplication'][0])) {
			$application = $extensLine['astapplication'][0];
		}
		$applicationInput = new htmlResponsiveInputField(_("Application"), "AstApplication_" . $placeInList, $application, 'AstApplication');
		$applicationInput->setRequired(true);
		$renderContainer->add($applicationInput);
		// application data
		$applicationData = '';
		if (isset($extensLine['astapplicationdata'][0])) {
			$applicationData = $extensLine['astapplicationdata'][0];
		}
		$renderContainer->add(new htmlResponsiveInputField(_("Application data"), "AstApplicationData_" . $placeInList, $applicationData, 'AstApplicationData'));
		//delimiter
		$renderContainer->addVerticalSpacer('0.5rem');
	}

	/**
	 * Sorts an array of arrays by the given key.
	 *
	 * @param array<int, array<string, string[]|string>|string> $array array
	 * @param string $on key
	 * @param string $order order (SORT_ASC or SORT_DESC)
	 * @return array<int, array<string, string[]|string>|string> result
	 */
	private function array_sort($array, $on, $order = 'SORT_ASC'): array {
		$new_array = [];
		$sortable_array = [];

		if (count($array) > 0) {
			foreach ($array as $k => $v) {
				if (is_array($v)) {
					foreach ($v as $k2 => $v2) {
						if ($k2 == $on) {
							$sortable_array[$k] = $v2;
						}
					}
				}
				else {
					$sortable_array[$k] = $v;
				}
			}
			if ($order === 'SORT_ASC') {
				asort($sortable_array);
			}
			else {
				arsort($sortable_array);
			}
			foreach (array_keys($sortable_array) as $k) {
				$new_array[] = $array[$k];
			}
		}
		return $new_array;
	}

	/**
	 * Displays a list of possible owners of this extension.
	 *
	 * @return htmlResponsiveRow HTML meta data
	 */
	public function display_html_user(): htmlResponsiveRow {
		$return = new htmlResponsiveRow();
		// load list with all potential owners
		$searchScope = 'user';
		$searchClass = 'inetOrgPerson';
		if ($this->filterOwnerForAsteriskAccounts) {
			$searchClass = 'AsteriskSIPUser';
		}
		$entries = searchLDAPByAttribute(null, null, $searchClass, ['dn'], [$searchScope]);
		$users_dn = [];
		for ($i = 0; $i < count($entries); $i++) {
			$dn = $entries[$i]['dn'];
			if (!in_array($dn, $this->extensionOwners)) {
				$users_dn[getAbstractDN($dn)] = $dn;
			}
		}
		uasort($users_dn, compareDN(...));
		$memberList = $this->extensionOwners;
		usort($memberList, compareDN(...));
		$members = [];
		for ($i = 0; $i < count($memberList); $i++) {
			$members[getAbstractDN($memberList[$i])] = $memberList[$i];
		}
		// options
		$return->add(new htmlSubTitle(_("Filters")));
		$return->add(new htmlResponsiveInputCheckbox('filterAsteriskUsers', $this->filterOwnerForAsteriskAccounts, _('Show only Asterisk accounts'), 'ownerOptions', false));
		$refreshButton = new htmlButton('changeFilter', _('Refresh'));
		$return->add($refreshButton);
		$return->addVerticalSpacer('1rem');
		// owners
		$return->add(new htmlSubTitle(_("Extension owners")));
		$this->addDoubleSelectionArea($return, _("Selected users"), _("Available users"), $members, [], $users_dn, [], 'owners', true);
		// back button
		$backButton = new htmlAccountPageButton(static::class, 'attributes', 'back', _('Ok'));
		$backButton->colspan = 3;
		$return->add($backButton);
		return $return;
	}

	/**
	 * Processes user input of the user selection page.
	 * It checks if all input values are correct and updates the associated LDAP attributes.
	 *
	 * @return array<string|string[]>[] list of info/error messages
	 */
	public function process_user(): array {
		if (isset($_POST['changeFilter'])) {
			// update filter value
			if (isset($_POST['filterAsteriskUsers'])) {
				$this->filterOwnerForAsteriskAccounts = ($_POST['filterAsteriskUsers'] == 'on');
			}
			else {
				$this->filterOwnerForAsteriskAccounts = false;
			}
		}
		if (isset($_POST['owners_2']) && isset($_POST['owners_left'])) {
			// Add new user
			$this->extensionOwners = @array_merge($this->extensionOwners, $_POST['owners_2']);
		}
		elseif (isset($_POST['owners_1']) && isset($_POST['owners_right'])) {
			$this->extensionOwners = array_delete($_POST['owners_1'], $this->extensionOwners);
		}
		$this->attributes['member'] = $this->extensionOwners;

		return [];
	}

	/**
	 * Returns if the extension was moved to another OU.
	 *
	 * @return bool true if moved
	 */
	private function isMoveToNewSuffix(): bool {
		$orig_suffix = extractDNSuffix($this->getAccountContainer()->dn_orig);
		return strcmp($orig_suffix, $this->getAccountContainer()->dnSuffix) !== 0
			&& !$this->getAccountContainer()->isNewAccount;
	}


	/**
	 * Returns true if at least one owner is set and false otherwise
	 *
	 * @return bool true if one or more owners
	 */
	private function isExtensionOwnerSet(): bool {
		return (count($this->extensionOwners) > 0);
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		$errors = [];
		$extensionName = [];

		if (!isset($_POST['generate_extension_name'])) {

			//perform normal set of operations
			if (isset($_POST['AstExtension']) && $_POST['AstExtension'] != '') {
				$this->attributes['AstExtension'][0] = $_POST['AstExtension'];
				$extensionName[0] = $_POST['AstExtension'];
			}
			else {
				$errors[] = $this->messages['AstExtension'][0];
			}
			$extensionContext = [];
			if (isset($_POST['AstContext'])) {
				$this->attributes['AstContext'][0] = $_POST['AstContext'];
				$extensionContext[0] = $_POST['AstContext'];
				if ($extensionContext[0] == '') {
					$errors[] = $this->messages['AstContext'][0];
				}
			}

			if ($this->getAccountContainer()->isNewAccount && isset($extensionName[0])) {
				$isPresented = $this->isThisExtensionPresented($extensionName[0]);
				if ($isPresented) {
					$errors[] = $this->messages['AstExtension'][1];
				}
			}

			if (isset($_POST['add_rule'])) {
				$this->addRuleFlag = true;
			}
			//process fields that are individual for each row
			$extrow_errors = $this->processExtensionRows($extensionName, $extensionContext);

			if (!empty($extrow_errors)) {
				$errors = array_merge($errors, $extrow_errors);
			}
		}
		else {
			//find new free extension name
			$this->extensionRows[0]['astextension'][0] = $this->generateNextExtensionName();
		}

		return $errors;
	}

	/**
	 * Processes the rule data.
	 *
	 * @param string[] $extensionName extension name
	 * @param string[] $extensionContext extension context
	 * @return array<string|string[]>[] error messages
	 */
	private function processExtensionRows($extensionName, $extensionContext): array {
		$errors = [];
		if (isset($_POST['extension_rows']) && get_preg($_POST['extension_rows'], 'digit')) {

			$extensionPriorityCntr = 1;
			$this->extensionRows = [];
			for ($entryCounter = 0; $entryCounter < $_POST['extension_rows']; $entryCounter++) {
				$extRow = [];
				if (!isset($_POST['delete_rule_' . $entryCounter])) {
					$singleExtAddErrors = $this->processSingleExtension($extRow, $extensionName, $extensionContext, $entryCounter, $extensionPriorityCntr);
					$errors += $singleExtAddErrors;

					if ((isset($extRow['astapplication'][0]) && $extRow['astapplication'][0] != "") ||
						(isset($extRow['astapplicationdata'][0]) && $extRow['astapplicationdata'][0] != "")
					) {
						$this->extensionRows[$extensionPriorityCntr - 1] = $extRow;
						$extensionPriorityCntr++;
					}
				}
			}

			//trow error banner if last row unporpertly filled (for now the only reason for that is unfilled AstApplication filed)
			if (isset($_POST['AstApplicationData_' . ($_POST['extension_rows'] - 1)]) && $_POST['AstApplicationData_' . ($_POST['extension_rows'] - 1)] != "" &&
				($_POST['AstApplication_' . ($_POST['extension_rows'] - 1)] == "" || !isset($_POST['AstApplication_' . ($_POST['extension_rows'] - 1)]))) {
				$errors[] = $this->messages['AstApplication'][0];
			}

			//process priority change on rule
			$this->processPriorityChange();

			//finally sort extensions by priority
			$this->extensionRows = $this->array_sort($this->extensionRows, 'astpriority');
		}
		return $errors;
	}

	/**
	 * Set extension owner as current logged in user.
	 */
	private function setDefaultExtensionOwner(): void {
		$login = $this->getDefaultExtensionOwner();
		$this->extensionOwners[0] = $login;
		$this->attributes['member'] = [$login];
	}

	/**
	 * Returns the default extension owner.
	 *
	 * @return String owner
	 */
	private function getDefaultExtensionOwner() {
		return $_SESSION['ldap']->getUserName();
	}

	/**
	 * Fills the fields of a single extension row.
	 * In Asterisk it would only be an extension name,a priority,an application, but LDAP specific
	 * add to processing context field.
	 *
	 * @param array<string, array<int, string>|string> $extRow - hash array to store single extension properties;
	 * @param string[] $extensionName extension name
	 * @param string[] $extensionContext extension context
	 * @param int $entryCounter - counter to distinguish single extensuion properties from $_POST
	 * @param int $extensionPriorityCntr - it is the variable where actual number of extension rules wuld be aggregated
	 * @return array<string|string[]>[] errors
	 */
	private function processSingleExtension(&$extRow, $extensionName, $extensionContext, $entryCounter, $extensionPriorityCntr): array {
		$errors = [];
		if (isset($extensionContext[0])) {
			$extRow['astcontext'][0] = $extensionContext[0];
		}
		if (isset($extensionName[0])) {
			$extRow['astextension'][0] = $extensionName[0];
		}

		$extRow["objectclass"][0] = "top";
		$extRow["objectclass"][1] = "groupOfNames";
		$extRow["objectclass"][2] = "AsteriskExtension";

		$extRow['astapplication'][0] = (string) $_POST['AstApplication_' . $entryCounter];
		if ((!isset($_POST['AstApplication_' . $entryCounter]) || $_POST['AstApplication_' . $entryCounter] == '')
			&& ($entryCounter < ($_POST['extension_rows'] - 1))) {
			$errors[] = $this->messages['AstApplication'][0];
		}

		$extRow['astpriority'][0] = strval($extensionPriorityCntr);

		$extRow['cn'][0] = $extRow['astextension'][0] . "-" . $extRow['astpriority'][0];

		if (isset($_POST['AstApplicationData_' . $entryCounter]) && $_POST['AstApplicationData_' . $entryCounter] != "") {
			$extRow['astapplicationdata'][0] = (string) $_POST['AstApplicationData_' . $entryCounter];
		}

		//Fill the member filed
		$extRow['member'] = $this->extensionOwners;
		if ((count($extRow['member']) === 0)
			&& !isset($_POST['form_subpage_' . static::class . '_user_open'])) {
			$errors[] = $this->messages['member'][0];
		}
		return $errors;
	}

	/**
	 * Reorders the rules if the user clicked on a move button.
	 */
	private function processPriorityChange(): void {
		for ($entryCounter = 0; $entryCounter < count($this->extensionRows); $entryCounter++) {
			if (isset($_POST["rule_up_button_" . $entryCounter])) {
				$this->extensionRows[$entryCounter]['astpriority'][0] = (string) (intval($this->extensionRows[$entryCounter]['astpriority'][0]) - 1);
				$this->extensionRows[$entryCounter - 1]['astpriority'][0] = (string) (intval($this->extensionRows[$entryCounter - 1]['astpriority'][0]) + 1);
			}
			elseif (isset($_POST["rule_down_button_" . $entryCounter])) {
				$this->extensionRows[$entryCounter]['astpriority'][0] = (string) (intval($this->extensionRows[$entryCounter]['astpriority'][0]) + 1);
				$this->extensionRows[$entryCounter + 1]['astpriority'][0] = (string) (intval($this->extensionRows[$entryCounter + 1]['astpriority'][0]) - 1);
			}
		}
	}

	/**
	 * Search by extension name and return true if fields with this extension name exists
	 * and false otherwise.
	 * Equal extension names are allowed in different OUs.
	 *
	 * @param string $extension extension name
	 * @return bool true if there are entries with this extension name.
	 */
	private function isThisExtensionPresented($extension): bool {
		$searchClass = "AsteriskExtension";
		$searchScope = 'asteriskExt';

		$entries = searchLDAPByAttribute("AstExtension", $extension, $searchClass, ['dn'], [$searchScope]);

		$exists = false;
		for ($i = 0; $i < count($entries); $i++) {
			$dn = extractDNSuffix($entries[$i]['dn']);
			if ($dn == $this->getAccountContainer()->dnSuffix) {
				$exists = true;
				break;
			}
		}
		return $exists;
	}

	/**
	 * This function searches in the base subtree and finds all extensions names within.
	 * The generation algorithm is the naive one, so only work when extension is numbers.
	 * All extension name is sorted and new extension name will be greates on + 1.
	 *
	 * @return string suggested extension name
	 */
	private function generateNextExtensionName() {
		$searchClass = "AsteriskExtension";
		$searchScope = 'asteriskExt';
		if (isset($this->extensionRows[0]['astextension'][0])) {
			$suggested_extension = $this->extensionRows[0]['astextension'][0];
		}
		else {
			$entries = searchLDAPByAttribute(null, null, $searchClass, ['astextension'], [$searchScope]);
			$entries = $this->array_sort($entries, 'astextension');

			$greater_ext_name = $entries[count($entries) - 1]['astextension'][0] ?? 0;
			if (is_numeric($greater_ext_name)) {
				$suggested_extension = $greater_ext_name + 1;
			}
			else {
				$ptn = "/(?P<name>[A-Za-z0-9]+)_(?P<digit>\\d+)/";
				if (preg_match_all($ptn, $greater_ext_name, $matches)) {
					$next_digit = ((int) $matches['digit'][0]) + 1;
					$suggested_extension = $matches['name'][0] . '_' . $next_digit;
				}
				else {
					$suggested_extension = $greater_ext_name . '_1';
				}
			}
		}
		return $suggested_extension;
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		$return = [];
		$extName = '';
		if (isset($this->attributes['AstExtension'][0])) {
			$extName = $this->attributes['AstExtension'][0];
		}
		if (isset($this->extensionRows[0]['astextension'][0])) {
			$extName = $this->extensionRows[0]['astextension'][0];
		}
		// global values
		$this->addSimplePDFField($return, 'AstContext', _('Account context'));
		$this->addSimplePDFField($return, 'AstExtension', _('Extension name'));
		$this->addSimplePDFField($return, 'owners', _('Extension owners'), 'member', '; ');
		// rules
		$entries = $this->load_extension_parts($extName);
		$pdfTable = new PDFTable();
		$pdfRow = new PDFTableRow();
		$pdfRow->cells[] = new PDFTableCell(_('Name'), '20%', null, true);
		$pdfRow->cells[] = new PDFTableCell(_('Application'), '30%', null, true);
		$pdfRow->cells[] = new PDFTableCell(_('Application data'), '30%', null, true);
		$pdfRow->cells[] = new PDFTableCell(_('Priority'), '20%', null, true);
		$pdfTable->rows[] = $pdfRow;
		for ($i = 0; $i < count($entries); $i++) {
			$appdata = ' ';
			if (isset($entries[$i]['astapplicationdata'][0])) {
				$appdata = $entries[$i]['astapplicationdata'][0];
			}
			$pdfRow = new PDFTableRow();
			$pdfRow->cells[] = new PDFTableCell($entries[$i]['cn'][0], '20%');
			$pdfRow->cells[] = new PDFTableCell($entries[$i]['astapplication'][0], '30%');
			$pdfRow->cells[] = new PDFTableCell($appdata, '30%');
			$pdfRow->cells[] = new PDFTableCell($entries[$i]['astpriority'][0], '20%');
			$pdfTable->rows[] = $pdfRow;
		}
		$this->addPDFTable($return, 'rules', $pdfTable);
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function build_uploadAccounts(array $rawAccounts, array $ids, array &$partialAccounts, array $selectedModules, ConfiguredType &$type): array {
		$messages = [];
		//hash to store extension_name => priority. For new extension priority will be 1
		$extensionNamePriorityMap = [];
		for ($i = 0; $i < count($rawAccounts); $i++) {
			// add object class
			if (!in_array("AsteriskExtension", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "AsteriskExtension";
			}
			if (!in_array("groupOfNames", $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = "groupOfNames";
			}
			// attributes
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskExtension_AstExtension', 'AstExtension');

			$extensionName = $partialAccounts[$i]['AstExtension'];
			if (isset($extensionNamePriorityMap[$extensionName])) {
				$astPriorityTmp = $extensionNamePriorityMap[$extensionName] + 1;
				$extensionNamePriorityMap[$extensionName] = $astPriorityTmp;
			}
			else {
				$astPriorityTmp = 1;
				$extensionNamePriorityMap[$extensionName] = $astPriorityTmp;
			}
			$partialAccounts[$i]['AstPriority'] = $astPriorityTmp;

			$partialAccounts[$i]['cn'] = $extensionName . '-' . $astPriorityTmp;

			if (isset($rawAccounts[$i][$ids['asteriskExtension_owner']]) && $rawAccounts[$i][$ids['asteriskExtension_owner']] != '') {
				$partialAccounts[$i]['member'] = explode(';', $rawAccounts[$i][$ids['asteriskExtension_owner']]);
			}
			else {
				$partialAccounts[$i]['member'] = $this->getDefaultExtensionOwner();
			}

			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskExtension_AstApplication', 'AstApplication');
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskExtension_AstApplicationData', 'AstApplicationData');
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'asteriskExtension_AstContext', 'AstContext');
		}

		return $messages;
	}

	/**
	 * Get list of all applications for given extension and move it into new suffix.
	 *
	 * @param array<string, string[]|string> $row attributes of original extension
	 * @return array<string|string[]>[] list of error messages
	 */
	private function moveExtentionToNewSuffix($row): array {
		$errors = [];
		if ($this->isMoveToNewSuffix()) {
			$oldDN = "cn=" . $row["cn"][0] . "," . extractDNSuffix($this->getAccountContainer()->dn_orig);
			$newRDN = "cn=" . $row["cn"][0];
			$is_rename_success = @ldap_rename($_SESSION['ldap']->server(), $oldDN, $newRDN, $this->getAccountContainer()->dnSuffix, true);
			if (!$is_rename_success) {
				$errors[] = ['ERROR', sprintf(_('Was unable to rename DN: %s.'), $this->getAccountContainer()->dn_orig), getDefaultLDAPErrorString($_SESSION['ldap']->server())];
				logNewMessage(LOG_ERR, 'Unable to rename ' . $oldDN . ' to ' . $newRDN . ',' . $this->getAccountContainer()->dnSuffix);
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		$this->extensionRows = $this->array_sort($this->extensionRows, 'astpriority');

		// modify existent config rows if necessary
		for ($rowCounter = 0; $rowCounter < count($this->extensionRowsOrig); $rowCounter++) {
			$rowOrig = $this->extensionRowsOrig[$rowCounter];
			unset($rowOrig['dn']);
			if ($rowCounter < count($this->extensionRows)) {
				$row = $this->extensionRows[$rowCounter];
				$attributeNames = array_values(array_unique(array_merge(array_keys($row), array_keys($rowOrig))));
				$ldapChanges = [];
				foreach ($attributeNames as $attributeName) {
					if (isset($rowOrig[$attributeName]) && !isset($row[$attributeName])) {
						$ldapChanges[$attributeName] = [];
					}
					elseif (!isset($rowOrig[$attributeName]) && isset($row[$attributeName])) {
						$ldapChanges[$attributeName] = $row[$attributeName];
					}
					elseif ($rowOrig[$attributeName][0] != $row[$attributeName][0]) {
						$ldapChanges[$attributeName] = $row[$attributeName][0];
					}
				}
				if (!empty($ldapChanges)) {
					if (!isset($ldapChanges['cn'])) {
						ldap_mod_replace($_SESSION['ldap']->server(), "cn=" . $row["cn"][0] . "," . $this->getAccountContainer()->dnSuffix, $ldapChanges);
					}
					else {
						$origDN = "cn=" . $rowOrig["cn"][0] . "," . extractDNSuffix($this->getAccountContainer()->dn_orig);
						$newRDN = "cn=" . $row["cn"][0];
						ldap_rename($_SESSION['ldap']->server(), $origDN, $newRDN, extractDNSuffix($this->getAccountContainer()->dn_orig), true);
						unset($ldapChanges['cn']);
						if (!empty($ldapChanges)) {
							ldap_mod_replace($_SESSION['ldap']->server(), $newRDN . "," . extractDNSuffix($this->getAccountContainer()->dn_orig), $ldapChanges);
						}
					}
				}
				// if new suffix just move old rows to the new suffix and go on
				$this->moveExtentionToNewSuffix($row);
			}
			else {
				ldapDeleteEntry($_SESSION['ldap']->server(), "cn=" . $rowOrig["cn"][0] . "," . extractDNSuffix($this->getAccountContainer()->dn_orig));
			}
		}
		// add new config rows
		for ($rowCounter = count($this->extensionRowsOrig); $rowCounter < count($this->extensionRows); $rowCounter++) {
			$row = $this->extensionRows[$rowCounter];
			ldapAddNewEntry($_SESSION['ldap']->server(), "cn=" . $row["cn"][0] . "," . $this->getAccountContainer()->dnSuffix, $row);
		}

		//a trick for Edit again to work
		$this->getAccountContainer()->dn_orig = "cn=" . $this->extensionRows[0]['cn'][0] . "," . $this->getAccountContainer()->dnSuffix;
		$this->getAccountContainer()->finalDN = "cn=" . $this->extensionRows[0]['cn'][0] . "," . $this->getAccountContainer()->dnSuffix;

		return $this->getAccountContainer()->save_module_attributes($this->orig, $this->orig);
	}

	/**
	 * {@inheritDoc}
	 */
	public function postDeleteActions(): array {
		$extension = $this->attributes['AstExtension'][0];
		$searchClass = "AsteriskExtension";
		$searchScope = 'asteriskExt';
		$entries = searchLDAPByAttribute("AstExtension", $extension, $searchClass, ['cn', 'AstContext', 'AstPriority',
			'AstApplication', 'AstApplicationData', 'AstExtension', 'member', 'ObjectClass', 'dn'], [$searchScope]);
		$entries = $this->array_sort($entries, 'astpriority');

		for ($rowCounter = 0; $rowCounter < count($entries); $rowCounter++) {
			$rowOrig = $entries[$rowCounter];
			if ($rowOrig["astpriority"][0] > 1) {
				ldapDeleteEntry($_SESSION['ldap']->server(), $rowOrig['dn']);
			}
		}
		return [];
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return [
			'astcontext' => _('Account context'),
			'astextension' => _('Extension name'),
			"member" => _("Owner"),
		];
	}

}
