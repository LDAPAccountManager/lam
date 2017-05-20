<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013 - 2015  Roland Gruber

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
* Manages PyKota printers.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages PyKota printers.
*
* @package modules
*/
class pykotaPrinter extends baseModule {

	/** cache for existing printers (array(dn => array(cn => ..., description => ...))) */
	private $printerCache = null;
	/** printer group cache */
	private $groupCache = null;
	/** list of pass through options: label => value */
	private $passThroughOptions;

	/**
	* Creates a new pykotaPrinter object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		$this->passThroughOptions = array(
			_('Yes') => 't',
			_('No') => 'f',
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
		return in_array($this->get_scope(), array('pykotaPrinterType'));
	}

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*
	* @see baseModule::get_metaData()
	*/
	function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'printerBig.png';
		// alias name
		$return["alias"] = _("PyKota");
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=pykotaPrinter)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('pykotaObject', 'pykotaPrinter');
		// managed attributes
		$return['attributes'] = array('cn', 'description', 'pykotaMaxJobSize', 'pykotaPassThrough', 'pykotaPricePerJob', 'pykotaPricePerPage', 'pykotaPrinterName', 'uniqueMember');
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _("Printer name"), 'attr' => 'cn',
				"Text" => _("Printer name of the printer which should be created. Valid characters are: a-z, A-Z, 0-9 and .-_ .")
			),
			'description' => array (
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("Printer description.")
			),
			'pykotaMaxJobSize' => array(
				"Headline" => _('Maximum job size'), 'attr' => 'pykotaMaxJobSize',
				"Text" => _('The maximum number of pages per job allowed on the printer. 0 means unlimited.')
			),
			'pykotaPassThrough' => array(
				"Headline" => _('Passthrough'), 'attr' => 'pykotaPassThrough',
				"Text" => _('In passthrough mode, users are allowed to print without any impact on their quota or account balance.')
			),
			'pykotaPricePerJob' => array(
				"Headline" => _('Price per job'), 'attr' => 'pykotaPricePerJob',
				"Text" => _('The price for each print job.')
			),
			'pykotaPricePerPage' => array(
				"Headline" => _('Price per page'), 'attr' => 'pykotaPricePerPage',
				"Text" => _('The price for each page of a print job.')
			),
			'uniqueMember' => array(
				"Headline" => _('Group members'), 'attr' => 'uniqueMember',
				"Text" => _('If this entry should be a printer group then you can set the member names here.')
			),
			'uniqueMemberUpload' => array(
				"Headline" => _('Group members'), 'attr' => 'uniqueMember',
				"Text" => _('If this entry should be a printer group then you can set the member names here.')
							. ' ' . _('Multiple values are separated by comma.')
			),
			'filter' => array(
				"Headline" => _("Filter"),
				"Text" => _("Here you can enter a filter value. Only entries which contain the filter text will be shown.")
					. ' ' . _('Possible wildcards are: "*" = any character, "^" = line start, "$" = line end')
			),
		);
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Maximum job size'), 'pykotaPrinter_pykotaMaxJobSize', '', 'pykotaMaxJobSize'), true);
		$return['profile_options'] = $profileContainer;
		$return['profile_mappings']['pykotaPrinter_pykotaMaxJobSize'] = 'pykotaMaxJobSize';
		$return['profile_checks']['pykotaPrinter_pykotaMaxJobSize'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['pykotaMaxJobSize'][0]);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'pykotaPrinter_cn',
				'description' => _('Printer name'),
				'help' => 'cn',
				'example' => _('printer01'),
				'required' => true,
			),
			array(
				'name' => 'pykotaPrinter_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Color laser printer'),
			),
			array(
				'name' => 'pykotaPrinter_pykotaMaxJobSize',
				'description' => _('Maximum job size'),
				'help' => 'pykotaMaxJobSize',
				'example' => '100',
				'default' => '0',
			),
			array(
				'name' => 'pykotaPrinter_pykotaPassThrough',
				'description' => _('Passthrough'),
				'help' => 'pykotaPassThrough',
				'example' => _('No'),
				'default' => _('No'),
				'values' => _('Yes') . ', ' . _('No'),
			),
			array(
				'name' => 'pykotaPrinter_pykotaPricePerJob',
				'description' => _('Price per job'),
				'help' => 'pykotaPricePerJob',
				'example' => '0.01',
			),
			array(
				'name' => 'pykotaPrinter_pykotaPricePerPage',
				'description' => _('Price per page'),
				'help' => 'pykotaPricePerPage',
				'example' => '0.01',
			),
			array(
				'name' => 'pykotaPrinter_uniqueMember',
				'description' => _('Group members'),
				'help' => 'uniqueMemberUpload',
				'example' => _('printergroup1'),
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('Printer name'),
			'description' => _('Description'),
			'pykotaMaxJobSize' => _('Maximum job size'),
			'pykotaPassThrough' => _('Passthrough'),
			'pykotaPricePerJob' => _('Price per job'),
			'pykotaPricePerPage' => _('Price per page'),
			'uniqueMember' => _('Group members'),
			'parentUniqueMember' => _('Printer groups'),
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('Printer name'), _('Printer name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' pykotaPrinter_cn', _('Printer name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][2] = array('ERROR', _('Printer name'), _('Printer name already exists!'));
		$this->messages['cn'][3] = array('ERROR', _('Account %s:') . ' pykotaPrinter_cn', _('Printer name already exists!'));
		$this->messages['pykotaMaxJobSize'][0] = array('ERROR', _('Maximum job size'), _('Please enter a valid number.'));
		$this->messages['pykotaMaxJobSize'][1] = array('ERROR', _('Account %s:') . ' pykotaPrinter_pykotaMaxJobSize', _('Please enter a valid number.'));
		$this->messages['pykotaPricePerJob'][0] = array('ERROR', _('Price per job'), _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaPricePerJob'][1] = array('ERROR', _('Account %s:') . ' pykotaPrinter_pykotaPricePerJob', _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaPricePerPage'][0] = array('ERROR', _('Price per page'), _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaPricePerPage'][1] = array('ERROR', _('Account %s:') . ' pykotaPrinter_pykotaPricePerPage', _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaPassThrough'][0] = array('ERROR', _('Account %s:') . ' pykotaPrinter_pykotaPassThrough', _('Please enter "Yes" or "No".'));
		$this->messages['uniqueMember'][0] = array('ERROR', _('Account %s:') . ' pykotaPrinter_uniqueMember', _('Unable to find a printer with name "%s".'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		// cn
		$this->addSimpleInputTextField($container, 'cn', _('Printer name'), true);
		// job size
		$this->addSimpleInputTextField($container, 'pykotaMaxJobSize', _('Maximum job size'));
		// price per job
		$this->addSimpleInputTextField($container, 'pykotaPricePerJob', _('Price per job'));
		// price per page
		$this->addSimpleInputTextField($container, 'pykotaPricePerPage', _('Price per page'));
		// passthrough
		$pykotaPassThroughOption = 'f';
		if (!empty($this->attributes['pykotaPassThrough'][0])) {
			$pykotaPassThroughOption = $this->attributes['pykotaPassThrough'][0];
		}
		$pykotaPassThroughSelect = new htmlTableExtendedSelect('pykotaPassThrough', $this->passThroughOptions, array($pykotaPassThroughOption), _('Passthrough'), 'pykotaPassThrough');
		$pykotaPassThroughSelect->setHasDescriptiveElements(true);
		$container->addElement($pykotaPassThroughSelect);
		$container->addElement(new htmlSpacer('150px', null), true); // layout fix if many parent groups exist
		// description
		$this->addMultiValueInputTextField($container, 'description', _('Description'), false, null, true);
		// printer groups
		if (!$this->getAccountContainer()->isNewAccount) {
			$groups = $this->getPrinterGroups();
			$this->loadPrinterNameCache();
			$parentPrinters = array();
			foreach ($groups as $groupDN) {
				$parentPrinters[] = $this->printerCache[$groupDN]['cn'];
			}
			if (sizeof($parentPrinters) > 0) {
				$container->addElement(new htmlOutputText(_('Printer groups')));
				$parentPrinterText = new htmlOutputText(implode(', ', $parentPrinters));
				$parentPrinterText->colspan = 5;
				$container->addElement($parentPrinterText, true);
			}
		}
		// printer members
		$memberLabel = new htmlOutputText(_('Group members'));
		$memberLabel->alignment = htmlElement::ALIGN_TOP;
		$container->addElement($memberLabel);
		$addMemberButton = new htmlAccountPageButton(get_class($this), 'members', 'open', 'add.png', true);
		$addMemberButton->setTitle(_('Add'));
		$addMemberButton->alignment = htmlElement::ALIGN_TOP;
		if (!empty($this->attributes['uniqueMember'][0])) {
			$memberTable = new htmlTable();
			$memberTable->alignment = htmlElement::ALIGN_TOP;
			for ($i = 0; $i < sizeof($this->attributes['uniqueMember']); $i++) {
				$member = $this->attributes['uniqueMember'][$i];
				if (isset($this->printerCache[$member]['cn'])) {
					$member = $this->printerCache[$member]['cn'];
				}
				$memberTable->addElement(new htmlOutputText($member));
				$delButton = new htmlButton('uniqueMemberDel_' . $i, 'del.png', true);
				$delButton->setTitle(_('Delete'));
				$memberTable->addElement($delButton);
				if ($i == (sizeof($this->attributes['uniqueMember']) - 1)) {
					$memberTable->addElement($addMemberButton);
				}
				$memberTable->addNewLine();
			}
			$container->addElement($memberTable);
		}
		else {
			$container->addElement($addMemberButton);
		}
		$memberHelp = new htmlHelpLink('uniqueMember');
		$memberHelp->alignment = htmlElement::ALIGN_TOP;
		$container->addElement($memberHelp, true);
		return $container;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$errors = array();
		// cn
		if (isset($_POST['cn']) && ($_POST['cn'] != '')) {
			if (!get_preg($_POST['cn'], 'username')) {
				$errors[] = $this->messages['cn'][0];
			}
			else {
				$this->attributes['cn'][0] = $_POST['cn'];
				$this->attributes['pykotaPrinterName'][0] = $_POST['cn'];
				if ((!isset($this->orig['cn'][0]) || ($this->attributes['cn'][0] != $this->orig['cn'][0]))
					&& $this->cnExists($_POST['cn'])) {
					$errors[] = $this->messages['cn'][2];
				}
			}
		}
		else {
			if (isset($this->attributes['cn'][0])) {
				unset($this->attributes['cn'][0]);
			}
			if (isset($this->attributes['pykotaPrinterName'][0])) {
				unset($this->attributes['pykotaPrinterName'][0]);
			}
		}
		// description
		$this->processMultiValueInputTextField('description', $errors);
		// job size
		$pykotaMaxJobSize = '0';
		if (isset($_POST['pykotaMaxJobSize']) && ($_POST['pykotaMaxJobSize'] != '')) {
			$pykotaMaxJobSize = $_POST['pykotaMaxJobSize'];
			if (!get_preg($pykotaMaxJobSize, 'digit')) {
				$errors[] = $this->messages['pykotaMaxJobSize'][0];
			}
		}
		$this->attributes['pykotaMaxJobSize'][0] = $pykotaMaxJobSize;
		// price per job
		$pykotaPricePerJob = '0.0';
		if (isset($_POST['pykotaPricePerJob']) && ($_POST['pykotaPricePerJob'] != '')) {
			$pykotaPricePerJob = $_POST['pykotaPricePerJob'];
			$pykotaPricePerJob = str_replace(',', '.', $pykotaPricePerJob);
			if (strpos($pykotaPricePerJob, '.') === false) {
				$pykotaPricePerJob .= '.0';
			}
			if (!get_preg($pykotaPricePerJob, 'float')) {
				$errors[] = $this->messages['pykotaPricePerJob'][0];
			}
		}
		$this->attributes['pykotaPricePerJob'][0] = $pykotaPricePerJob;
		// price per page
		$pykotaPricePerPage = '0.0';
		if (isset($_POST['pykotaPricePerPage']) && ($_POST['pykotaPricePerPage'] != '')) {
			$pykotaPricePerPage = $_POST['pykotaPricePerPage'];
			$pykotaPricePerPage = str_replace(',', '.', $pykotaPricePerPage);
			if (strpos($pykotaPricePerPage, '.') === false) {
				$pykotaPricePerPage .= '.0';
			}
			if (!get_preg($pykotaPricePerPage, 'float')) {
				$errors[] = $this->messages['pykotaPricePerPage'][0];
			}
		}
		$this->attributes['pykotaPricePerPage'][0] = $pykotaPricePerPage;
		// passthrough
		$this->attributes['pykotaPassThrough'][0] = $_POST['pykotaPassThrough'];
		// delete members
		foreach ($_POST as $key => $value) {
			if (strpos($key, 'uniqueMemberDel_') === 0) {
				$index = substr($key, strlen('uniqueMemberDel_'));
				unset($this->attributes['uniqueMember'][$index]);
				$this->attributes['uniqueMember'] = array_values($this->attributes['uniqueMember']);
				break;
			}
		}
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page to add members.
	*
	* @return htmlElement HTML meta data
	*/
	function display_html_members() {
		$return = new htmlTable();
		$userFilter = '';
		$userFilterRegex = '';
		if (isset($_POST['newFilter'])) {
			$userFilter = $_POST['newFilter'];
			$userFilterRegex = '/' . str_replace(array('*', '(', ')'), array('.*', '\(', '\)'), $_POST['newFilter']) . '/ui';
		}
		$options = array();
		$this->loadPrinterNameCache();
		foreach ($this->printerCache as $dn => $attrs) {
			if (!empty($attrs['description'])) {
				$label = $attrs['cn'] . ' (' . $attrs['description'] . ')';
			}
			else {
				$label = $attrs['cn'];
			}
			// skip filtered printers
			if (!empty($userFilter) && !preg_match($userFilterRegex, $label)) {
				continue;
			}
			// skip own entry
			if (!$this->getAccountContainer()->isNewAccount && ($this->getAccountContainer()->dn_orig == $dn)) {
				continue;
			}
			// skip already set members
			if (!empty($this->attributes['uniqueMember'][0]) && in_array($dn, $this->attributes['uniqueMember'])) {
				continue;
			}
			$options[$label] = $dn;
		}
		$size = 20;
		if (sizeof($options) < 20) $size = sizeof($options);
		$membersSelect = new htmlSelect('members', $options, array(), $size);
		$membersSelect->setHasDescriptiveElements(true);
		$membersSelect->setMultiSelect(true);
		$membersSelect->setTransformSingleSelect(false);
		$return->addElement($membersSelect, true);
		$filterGroup = new htmlGroup();
		$filterGroup->addElement(new htmlInputField('newFilter', $userFilter));
		$filterGroup->addElement(new htmlButton('setFilter', _('Filter')));
		$filterGroup->addElement(new htmlHelpLink('filter'));
		$return->addElement($filterGroup, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$buttonTable = new htmlTable();
		$buttonTable->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'addMembers', _('Add')));
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
	function process_members() {
		$return = array();
		if (isset($_POST['form_subpage_' . get_class($this) . '_attributes_addMembers']) && isset($_POST['members'])) {
			for ($i = 0; $i < sizeof($_POST['members']); $i++) {
				$this->attributes['uniqueMember'][] = $_POST['members'][$i];
			}
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		$this->loadPrinterNameCache();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object classes
			if (!in_array('pykotaPrinter', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaPrinter';
			}
			if (!in_array('pykotaObject', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaObject';
			}
			// cn
			if (!empty($rawAccounts[$i][$ids['pykotaPrinter_cn']])) {
				if (!get_preg($rawAccounts[$i][$ids['pykotaPrinter_cn']], 'username')) {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				elseif ($this->cnExists($rawAccounts[$i][$ids['pykotaPrinter_cn']])) {
					$errMsg = $this->messages['cn'][3];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['pykotaPrinter_cn']];
					$partialAccounts[$i]['pykotaPrinterName'] = $rawAccounts[$i][$ids['pykotaPrinter_cn']];
				}
			}
			// description
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'pykotaPrinter_description', 'description');
			// job size
			$partialAccounts[$i]['pykotaMaxJobSize'] = '0.0';
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'pykotaPrinter_pykotaMaxJobSize', 'pykotaMaxJobSize',
				'digit', $this->messages['pykotaMaxJobSize'][1], $messages);
			// price per job
			if (!empty($rawAccounts[$i][$ids['pykotaPrinter_pykotaPricePerJob']])) {
				$pykotaPricePerJob = $rawAccounts[$i][$ids['pykotaPrinter_pykotaPricePerJob']];
				$pykotaPricePerJob = str_replace(',', '.', $pykotaPricePerJob);
				if (strpos($pykotaPricePerJob, '.') === false) {
					$pykotaPricePerJob .= '.0';
				}
				if (!get_preg($pykotaPricePerJob, 'float')) {
					$errMsg = $this->messages['pykotaPricePerJob'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['pykotaPricePerJob'] = $pykotaPricePerJob;
				}
			}
			else {
				$partialAccounts[$i]['pykotaPricePerJob'] = '0.0';
			}
			// price per page
			if (!empty($rawAccounts[$i][$ids['pykotaPrinter_pykotaPricePerPage']])) {
				$pykotaPricePerPage = $rawAccounts[$i][$ids['pykotaPrinter_pykotaPricePerPage']];
				$pykotaPricePerPage = str_replace(',', '.', $pykotaPricePerPage);
				if (strpos($pykotaPricePerPage, '.') === false) {
					$pykotaPricePerPage .= '.0';
				}
				if (!get_preg($pykotaPricePerPage, 'float')) {
					$errMsg = $this->messages['pykotaPricePerPage'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['pykotaPricePerPage'] = $pykotaPricePerPage;
				}
			}
			else {
				$partialAccounts[$i]['pykotaPricePerPage'] = '0.0';
			}
			// passthrough
			if (!empty($rawAccounts[$i][$ids['pykotaPrinter_pykotaPassThrough']])) {
				if (isset($this->passThroughOptions[$rawAccounts[$i][$ids['pykotaPrinter_pykotaPassThrough']]])) {
					$partialAccounts[$i]['pykotaPassThrough'] = $this->passThroughOptions[$rawAccounts[$i][$ids['pykotaPrinter_pykotaPassThrough']]];
				}
				else {
					$errMsg = $this->messages['pykotaPassThrough'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['pykotaPassThrough'] = 'f';
			}
			// group members
			if (!empty($rawAccounts[$i][$ids['pykotaPrinter_uniqueMember']])) {
				$members = preg_split('/,[ ]*/', $rawAccounts[$i][$ids['pykotaPrinter_uniqueMember']]);
				$memberDNs = array();
				foreach ($members as $cn) {
					if (empty($cn)) {
						continue;
					}
					// search printer cache for cn to get DN
					$found = false;
					foreach ($this->printerCache as $dn => $attrs) {
						if ($this->printerCache[$dn]['cn'] == $cn) {
							$found = true;
							$memberDNs[] = $dn;
							break;
						}
					}
					if (!$found) {
						$errMsg = $this->messages['uniqueMember'][0];
						array_push($errMsg, array($i, htmlspecialchars($cn)));
						$messages[] = $errMsg;
					}
				}
				if (sizeof($memberDNs) > 0) {
					$partialAccounts[$i]['uniqueMember'] = $memberDNs;
				}
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->loadPrinterNameCache();
		$this->addSimplePDFField($return, 'cn', _('Printer name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'pykotaMaxJobSize', _('Maximum job size'));
		$this->addSimplePDFField($return, 'pykotaPricePerJob', _('Price per job'));
		$this->addSimplePDFField($return, 'pykotaPricePerPage', _('Price per page'));
		// passthrough
		$passthroughOptions = array_flip($this->passThroughOptions);
		$passthroughValue = '';
		if (!empty($this->attributes['pykotaPassThrough'][0]) && isset($passthroughOptions[$this->attributes['pykotaPassThrough'][0]])) {
			$passthroughValue = $passthroughOptions[$this->attributes['pykotaPassThrough'][0]];
		}
		$this->addPDFKeyValue($return, 'pykotaPassThrough', _('Passthrough'), $passthroughValue);
		// members
		if (!empty($this->attributes['uniqueMember'][0])) {
			$members = array();
			foreach ($this->attributes['uniqueMember'] as $member) {
				if (!empty($this->printerCache[$member]['cn'])) {
					$members[] = $this->printerCache[$member]['cn'];
				}
				else {
					$members[] = getAbstractDN($member);
				}
			}
			$this->addPDFKeyValue($return, 'uniqueMember', _('Group members'), implode(', ', $members));
		}
		// printer groups
		$parentGroups = array();
		$groups = $this->getPrinterGroups();
		foreach ($groups as $group) {
			if (!empty($this->printerCache[$group]['cn'])) {
				$parentGroups[] = $this->printerCache[$group]['cn'];
			}
			else {
				$parentGroups[] = getAbstractDN($group);
			}
		}
		if (sizeof($parentGroups) > 0) {
			$this->addPDFKeyValue($return, 'parentUniqueMember', _('Printer groups'), implode(', ', $parentGroups));
		}
		return $return;
	}

	/**
	 * Returns if the given cn already exists.
	 *
	 * @param String $cn cn attribute value
	 * @return boolean cn exists
	 */
	private function cnExists($cn) {
		if ($this->printerCache == null) {
			$this->loadPrinterNameCache();
		}
		foreach ($this->printerCache as $dn => $attrs) {
			if (!empty($attrs['cn']) && ($attrs['cn'] == $cn)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Loads the list of printer names into the cache.
	 */
	private function loadPrinterNameCache() {
		if ($this->printerCache != null) {
			return;
		}
		$results = searchLDAPByFilter('(objectClass=pykotaPrinter)', array('cn', 'dn', 'description'), array($this->get_scope()));
		$this->printerCache = array();
		foreach ($results as $result) {
			if (isset($result['cn'][0])) {
				$this->printerCache[$result['dn']]['cn'] = $result['cn'][0];
			}
			if (isset($result['description'][0])) {
				$this->printerCache[$result['dn']]['description'] = $result['description'][0];
			}
		}
	}

	/**
	 * Returns the printer group memberships.
	 *
	 * @return array DNs of parent groups
	 */
	private function getPrinterGroups() {
		if ($this->groupCache != null) {
			return $this->groupCache;
		}
		$results = searchLDAPByFilter('(&(objectClass=pykotaPrinter)(uniqueMember=' . $this->getAccountContainer()->dn_orig . '))', array('dn'), array($this->get_scope()));
		$this->groupCache = array();
		foreach ($results as $result) {
			$this->groupCache[] = $result['dn'];
		}
		return $this->groupCache;
	}

}


?>
