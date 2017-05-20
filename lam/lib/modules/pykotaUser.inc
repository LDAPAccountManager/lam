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
* Manages PyKota user accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages PyKota user accounts.
*
* @package modules
*/
class pykotaUser extends baseModule {

	/** cache for uid attribute */
	private $uidCache = null;
	/** cache for pykotaUserName attribute */
	private $pykotaUserNameCache = null;
	/** list of limit options label => value */
	private $limitOptions;

	/**
	 * Returns if this module also manages the structural object class pykotaObject.
	 * This is overridden by a submodule that must provide the structural object class.
	 *
	 * @return boolean structural usage
	 */
	public function isStructural() {
		return false;
	}

	/**
	* Creates a new pykotaUser object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		$this->limitOptions = array(
			_('Quota') => 'quota',
			_('Balance') => 'balance',
			_('No quota') => 'noquota',
			_('Free printing') => 'nochange',
			_('Deny printing') => 'noprint',
		);
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = $this->isStructural();
	}

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	*
	* @return boolean true if module fits
	*/
	public function can_manage() {
		return in_array($this->get_scope(), array('user'));
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
		$return["is_base"] = $this->isStructural();
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=pykotaAccount)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('pykotaAccount', 'pykotaAccountBalance');
		// managed attributes
		$return['attributes'] = array('uid', 'mail', 'description', 'pykotaLimitBy', 'pykotaUserName', 'pykotaBalance', 'pykotaLifeTimePaid',
			'pykotaOverCharge', 'pykotaPayments');
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _("Common name"), 'attr' => 'cn',
				"Text" => _("This is the natural name of the user.")
			),
			'uid' => array(
				"Headline" => _("User name"), 'attr' => 'uid',
				"Text" => _("User name of the user who should be created. Valid characters are: a-z,A-Z,0-9, @.-_.")
			),
			'mail' => array (
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The user's email address.")
			),
			'description' => array (
				"Headline" => _("Description"), 'attr' => 'description',
				"Text" => _("User description.")
			),
			'pykotaUserName' => array(
				"Headline" => _("PyKota user name"), 'attr' => 'pykotaUserName',
				"Text" => _("User name that is used for PyKota.")
			),
			'pykotaLimitBy' => array(
				"Headline" => _("Limit type"), 'attr' => 'pykotaLimitBy',
				"Text" => _("Specifies the type of limit for printing if any. Please note that in contrast to \"Free printing\" the option \"No quota\" includes accounting.")
			),
			'pykotaBalance' => array(
				"Headline" => _('Balance'), 'attr' => 'pykotaBalance',
				"Text" => _('Current account balance for the user.')
			),
			'pykotaBalanceComment' => array(
				"Headline" => _('Balance comment'), 'attr' => 'pykotaPayments',
				"Text" => _('Comment for initial balance.')
			),
			'pykotaLifeTimePaid' => array(
				"Headline" => _('Total paid'), 'attr' => 'pykotaLifeTimePaid',
				"Text" => _('Total money paid by the user.')
			),
			'pykotaOverCharge' => array(
				"Headline" => _('Overcharge factor'), 'attr' => 'pykotaOverCharge',
				"Text" => _('Overcharge factor that is applied when computing the cost of a print job. The number of pages is not changed.')
			),
			'pykotaPayments' => array(
				"Headline" => _('Payment and job history'), 'attr' => 'pykotaPayments',
				"Text" => _('Payment and job history for this user.')
			),
			'pykotaPaymentsAdd' => array(
				"Headline" => _('Payment'), 'attr' => 'pykotaBalance, pykotaPayments',
				"Text" => _('Adds the amount to the user\'s balance. You can also specify a comment.')
			),
			'jobSuffix' => array(
				"Headline" => _('Job suffix'),
				"Text" => _('Please enter the LDAP suffix where the PyKota job entries are stored (configuration option "jobbase").')
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
		);
		// profile options
		$profileContainer = new htmlTable();
		$pykotaLimitByProfileOption = new htmlTableExtendedSelect('pykotaUser_pykotaLimitBy', $this->limitOptions, array(), _('Limit type'), 'pykotaLimitBy');
		$pykotaLimitByProfileOption->setHasDescriptiveElements(true);
		$pykotaLimitByProfileOption->setSortElements(false);
		$profileContainer->addElement($pykotaLimitByProfileOption, true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Balance'), 'pykotaUser_pykotaBalance', '', 'pykotaBalance'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Overcharge factor'), 'pykotaUser_pykotaOverCharge', '', 'pykotaOverCharge'), true);
		if (!$this->isStructural()) {
			$profileContainer->addElement(new htmlTableExtendedInputCheckbox('pykotaUser_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
		}
		$return['profile_options'] = $profileContainer;
		$return['profile_mappings']['pykotaUser_pykotaLimitBy'] = 'pykotaLimitBy';
		$return['profile_mappings']['pykotaUser_pykotaOverCharge'] = 'pykotaOverCharge';
		$return['profile_checks']['pykotaUser_pykotaBalance'] = array(
			'type' => 'ext_preg',
			'regex' => 'float',
			'error_message' => $this->messages['pykotaBalance'][0]);
		$return['profile_checks']['pykotaUser_pykotaOverCharge'] = array(
			'type' => 'ext_preg',
			'regex' => 'float',
			'error_message' => $this->messages['pykotaOverCharge'][0]);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'pykotaUser_pykotaUserName',
				'description' => _('PyKota user name'),
				'help' => 'pykotaUserName',
				'example' => _('smiller'),
				'unique' => true,
			)
		);
		if ($this->isStructural()) {
			$return['upload_columns'][] = array(
				'name' => 'pykotaUser_cn',
				'description' => _('Common name'),
				'help' => 'cn',
				'example' => _('Steve Miller'),
				'required' => true,
			);
		}
		$return['upload_columns'][] = array(
			'name' => 'pykotaUser_pykotaLimitBy',
			'description' => _('Limit type'),
			'help' => 'pykotaLimitBy',
			'example' => _('Quota'),
			'default' => _('Quota'),
			'values' => implode(', ', array_keys($this->limitOptions))
		);
		$return['upload_columns'][] = array(
			'name' => 'pykotaUser_pykotaBalance',
			'description' => _('Balance'),
			'help' => 'pykotaBalance',
			'example' => '10.0',
		);
		$return['upload_columns'][] = array(
			'name' => 'pykotaUser_pykotaBalanceComment',
			'description' => _('Balance comment'),
			'help' => 'pykotaBalanceComment',
			'example' => _('Initial payment'),
		);
		$return['upload_columns'][] = array(
			'name' => 'pykotaUser_pykotaOverCharge',
			'description' => _('Overcharge factor'),
			'help' => 'pykotaOverCharge',
			'example' => '1.0',
			'default' => '1.0'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'pykotaUserName' => _('PyKota user name'),
			'pykotaLimitBy' => _('Limit type'),
			'pykotaBalance' => _('Balance'),
			'pykotaOverCharge' => _('Overcharge factor'),
			'pykotaLifeTimePaid' => _('Total paid'),
			'pykotaPayments' => _('Payment history'),
		);
		if ($this->isStructural()) {
			$return['PDF_fields']['cn'] = _('Common name');
		}
		// self service
		$return['selfServiceFieldSettings'] = array(
			'pykotaBalance' => _('Balance (read-only)'),
			'pykotaLifeTimePaid' => _('Total paid (read-only)'),
			'pykotaPayments' => _('Payment history'),
			'pykotaJobHistory' => _('Job history'),
		);
		// self service settings
		if (get_class($this) == 'pykotaUser') {
			$selfServiceContainer = new htmlTable();
			$selfServiceContainer->addElement(new htmlTableExtendedInputField(_('Job suffix'), 'pykotaUser_jobSuffix', null));
			$selfServiceContainer->addElement(new htmlHelpLink('jobSuffix', get_class($this)), true);
			$return['selfServiceSettings'] = $selfServiceContainer;
		}
		// config options
		$configContainer = new htmlTable();
		$configContainer->addElement(new htmlTableExtendedInputField(_('Job suffix'), 'pykotaUser_jobSuffix', '', 'jobSuffix'), true);
		$return['config_options']['all'] = $configContainer;
		// configuration checks
		$return['config_checks']['all']['pykotaUser_jobSuffix'] = array (
			'type' => 'ext_preg',
			'regex' => 'dn',
			'required' => false,
			'required_message' => $this->messages['jobSuffix'][0],
			'error_message' => $this->messages['jobSuffix'][0]);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['uid'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][1] = array('ERROR', _('Account %s:') . ' pykotaUser_uid', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['uid'][2] = array('ERROR', _('User name'), _('User name already exists!'));
		$this->messages['uid'][3] = array('ERROR', _('Account %s:') . ' pykotaUser_uid', _('User name already exists!'));
		$this->messages['pykotaUserName'][0] = array('ERROR', _('PyKota user name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['pykotaUserName'][1] = array('ERROR', _('Account %s:') . ' pykotaUser_pykotaUserName', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['pykotaUserName'][2] = array('ERROR', _('PyKota user name'), _('User name already exists!'));
		$this->messages['pykotaUserName'][3] = array('ERROR', _('Account %s:') . ' pykotaUser_pykotaUserName', _('User name already exists!'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' pykotaUser_mail', _('Please enter a valid email address!'));
		$this->messages['pykotaLimitBy'][0] = array('ERROR', _('Account %s:') . ' pykotaUser_pykotaLimitBy', _('Please enter a valid limit type.'));
		$this->messages['pykotaBalance'][0] = array('ERROR', _('Balance'), _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaBalance'][1] = array('ERROR', _('Account %s:') . ' pykotaUser_pykotaBalance', _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaOverCharge'][0] = array('ERROR', _('Overcharge factor'), _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaOverCharge'][1] = array('ERROR', _('Account %s:') . ' pykotaUser_pykotaOverCharge', _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['pykotaPayments'][0] = array('ERROR', _('Payment'), _('Please enter a valid number (e.g. "1.5").'));
		$this->messages['jobSuffix'][0] = array('ERROR', _('Job suffix'), _('Please enter a valid job suffix.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		$modules = $this->getAccountContainer()->get_type()->getModules();
		if ($this->isStructural() || (isset($this->attributes['objectClass']) && in_array('pykotaAccount', $this->attributes['objectClass']))) {
			// uid
			if ($this->manageUid($modules)) {
				$this->addSimpleInputTextField($container, 'uid', _('User name'), true);
			}
			else {
				// require uid
				$uid = $this->getCurrentUserName();
				if (empty($uid)) {
					$page = '';
					if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
						$page = $this->getAccountContainer()->getAccountModule('posixAccount')->get_alias();
					}
					elseif ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
						$page = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->get_alias();
					}
					$msg = new htmlStatusMessage('INFO', sprintf(_("Please enter an user name on this page: %s"), $page));
					$msg->colspan = 5;
					$container->addElement($msg, true);
				}
			}
			// pykotaUserName
			$this->addSimpleInputTextField($container, 'pykotaUserName', _('Pykota user name'));
			// balance
			$pykotaBalance = '';
			if (isset($this->attributes['pykotaBalance'][0])) {
				$pykotaBalance = $this->attributes['pykotaBalance'][0];
			}
			$container->addElement(new htmlOutputText(_('Balance')));
			$container->addElement(new htmlOutputText($pykotaBalance));
			$container->addElement(new htmlHelpLink('pykotaBalance'), true);
			// new payment and balance history
			$container->addElement(new htmlOutputText(_('Payment')));
			$newPaymentGroup = new htmlGroup();
			$newPaymentAmount = new htmlInputField('pykotaBalanceAdd', '', '5em');
			$newPaymentAmount->setTitle(_('Amount'));
			$newPaymentGroup->addElement($newPaymentAmount);
			$newPaymentComment = new htmlInputField('pykotaBalanceComment', '', '20em');
			$newPaymentComment->setTitle(_('Comment'));
			$newPaymentGroup->addElement($newPaymentComment);
			$newPaymentBtn = new htmlButton('addPayment', _('Add'));
			$newPaymentBtn->setIconClass('createButton');
			$newPaymentGroup->addElement($newPaymentBtn);
			$container->addElement($newPaymentGroup);
			$container->addElement(new htmlHelpLink('pykotaPaymentsAdd'), true);
			$container->addElement(new htmlOutputText(''));
			$historyGroup = new htmlGroup();
			$historyGroup->addElement(new htmlAccountPageButton(get_class($this), 'payments', 'open', _('Payment history')));
			if (!$this->getAccountContainer()->isNewAccount && !empty($this->moduleSettings['pykotaUser_jobSuffix'][0])) {
				$historyGroup->addElement(new htmlSpacer('5px', null));
				$historyGroup->addElement(new htmlAccountPageButton(get_class($this), 'jobs', 'open', _('Job history')));
			}
			$container->addElement($historyGroup);
			$container->addElement(new htmlHelpLink('pykotaPayments'), true);
			$container->addElement(new htmlSpacer(null, '10px'), true);
			// limit by
			$limitOption = 'quota';
			if (!empty($this->attributes['pykotaLimitBy'][0])) {
				$limitOption = $this->attributes['pykotaLimitBy'][0];
			}
			$limitSelect = new htmlTableExtendedSelect('pykotaLimitBy', $this->limitOptions, array($limitOption), _('Limit type'), 'pykotaLimitBy');
			$limitSelect->setHasDescriptiveElements(true);
			$limitSelect->setSortElements(false);
			$container->addElement($limitSelect, true);
			// overcharge factor
			$this->addSimpleInputTextField($container, 'pykotaOverCharge', _('Overcharge factor'));
			// cn
			if ($this->isStructural()) {
				$this->addSimpleInputTextField($container, 'cn', _('Common name'), true);
			}
			// mail
			if ($this->manageMail($modules)) {
				$this->addSimpleInputTextField($container, 'mail', _('Email address'));
			}
			// description
			if ($this->manageDescription($modules)) {
				$this->addMultiValueInputTextField($container, 'description', _('Description'), false, null, true);
			}
			// remove button
			if (!$this->isStructural()) {
				$container->addElement(new htmlSpacer(null, '20px'), true);
				$remButton = new htmlButton('remObjectClass', _('Remove PyKota extension'));
				$remButton->colspan = 5;
				$container->addElement($remButton);
			}
		}
		else {
			// add button
			$container->addElement(new htmlButton('addObjectClass', _('Add PyKota extension')));
		}
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
		if (isset($_POST['addObjectClass'])) {
			if (!isset($this->attributes['objectClass'])) {
				$this->attributes['objectClass'] = array();
			}
			if (!in_array('pykotaAccount', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'pykotaAccount';
			}
			if (!in_array('pykotaAccountBalance', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'pykotaAccountBalance';
			}
			if (!isset($this->attributes['pykotaUserName'][0])) {
				$this->attributes['pykotaUserName'][0] = $this->getCurrentUserName();
			}
			return $errors;
		}
		$modules = $this->getAccountContainer()->get_type()->getModules();
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('pykotaAccount', 'pykotaAccountBalance'), $this->attributes['objectClass']);
			$attrs = array('pykotaLimitBy', 'pykotaUserName');
			if ($this->manageDescription($modules)) {
				$attrs[] = 'description';
			}
			if ($this->manageMail($modules)) {
				$attrs[] = 'mail';
			}
			if ($this->manageUid($modules)) {
				$attrs[] = 'uid';
			}
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		// skip processing if object class is not set
		if (!isset($this->attributes['objectClass']) || !in_array('pykotaAccount', $this->attributes['objectClass'])) {
			return $errors;
		}
		// uid
		if ($this->manageUid($modules)) {
			if (isset($_POST['uid']) && ($_POST['uid'] != '')) {
				if (!get_preg($_POST['uid'], 'username')) {
					$errors[] = $this->messages['uid'][0];
				}
				else {
					$this->attributes['uid'][0] = $_POST['uid'];
					if ((!isset($this->orig['uid'][0]) || ($this->attributes['uid'][0] != $this->orig['uid'][0]))
						&& $this->uidExists($_POST['uid'])) {
						$errors[] = $this->messages['uid'][2];
					}
				}
			}
			elseif (isset($this->attributes['uid'][0])) {
				unset($this->attributes['uid'][0]);
			}
		}
		// PyKota user name
		if (!empty($_POST['pykotaUserName'])) {
			if (!get_preg($_POST['pykotaUserName'], 'username')) {
				$errors[] = $this->messages['pykotaUserName'][0];
			}
			else {
				$this->attributes['pykotaUserName'][0] = $_POST['pykotaUserName'];
				if ((!isset($this->orig['pykotaUserName'][0]) || ($this->attributes['pykotaUserName'][0] != $this->orig['pykotaUserName'][0]))
					&& $this->pykotaUserNameExists($_POST['pykotaUserName'])) {
					$errors[] = $this->messages['pykotaUserName'][2];
				}
			}
		}
		else {
			$this->attributes['pykotaUserName'][0] = $this->getCurrentUserName();
		}
		// limit by
		$this->attributes['pykotaLimitBy'][0] = $_POST['pykotaLimitBy'];
		// cn
		if ($this->isStructural()) {
			$this->attributes['cn'][0] = $_POST['cn'];
			if (empty($this->attributes['cn'][0])) {
				$this->attributes['cn'][0] = $_POST['uid'];
			}
		}
		// mail
		if ($this->manageMail($modules)) {
			$this->attributes['mail'][0] = $_POST['mail'];
			if (!empty($_POST['mail']) && !get_preg($_POST['mail'], 'email')) {
				$errors[] = $this->messages['mail'][0];
			}
		}
		// description
		if ($this->manageDescription($modules)) {
			$this->processMultiValueInputTextField('description', $errors);
		}
		// overcharge factor
		if (!empty($_POST['pykotaOverCharge'])) {
			$this->attributes['pykotaOverCharge'][0] = $_POST['pykotaOverCharge'];
		}
		else {
			$this->attributes['pykotaOverCharge'][0] = '1.0';
		}
		// add payment
		if (isset($_POST['addPayment'])) {
			$amount = $_POST['pykotaBalanceAdd'];
			if (!empty($amount) && (strpos($amount, ',') !== false)) {
				$amount = str_replace(',', '.', $amount);
			}
			if (!empty($amount) && (strpos($amount, '.') === false)) {
				$amount .= '.0';
			}
			$comment = $_POST['pykotaBalanceComment'];
			if (!empty($comment)) {
				$comment = base64_encode($comment);
			}
			if (empty($amount) || (floatval($amount) == 0.0)) {
				$errors[] = $this->messages['pykotaPayments'][0];
			}
			else {
				$this->attributes['pykotaPayments'][] = getFormattedTime('Y-m-d H:i:s,00') . ' # ' . $amount . ' # ' . $comment;
				// new balance
				$newBalance = 0.0;
				if (!empty($this->attributes['pykotaBalance'][0])) {
					$newBalance = floatval($this->attributes['pykotaBalance'][0]);
				}
				$newBalance += $amount;
				$this->attributes['pykotaBalance'][0] = strval($newBalance);
				// total paid
				$total = 0.0;
				if (!empty($this->attributes['pykotaLifeTimePaid'][0])) {
					$total = floatval($this->attributes['pykotaLifeTimePaid'][0]);
				}
				$total += $amount;
				$this->attributes['pykotaLifeTimePaid'][0] = strval($total);
			}
		}
		return $errors;
	}

	/**
	 * Returns the HTML meta data for the payments page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_payments() {
		$container = new htmlTable();
		// total paid
		$total = '';
		if (!empty($this->attributes['pykotaLifeTimePaid'][0])) {
			$total = $this->attributes['pykotaLifeTimePaid'][0];
		}
		$container->addElement(new htmlOutputText(_('Total paid')));
		$container->addElement(new htmlOutputText($total));
		$container->addElement(new htmlHelpLink('pykotaLifeTimePaid'), true);
		// payment/job history
		if (!empty($this->attributes['pykotaPayments'][0])) {
			$container->addElement(new htmlSubTitle(_('Payment history')), true);
			$spacer = new htmlSpacer('10px', null);
			$historyTable = new htmlTable();
			$historyTable->colspan = 5;
			$historyTable->addElement(new htmlOutputText(_('Date')), false, true);
			$historyTable->addElement($spacer);
			$historyTable->addElement(new htmlOutputText(_('Amount')), false, true);
			$historyTable->addElement($spacer);
			$historyTable->addElement(new htmlOutputText(_('Comment')), true, true);
			rsort($this->attributes['pykotaPayments']);
			foreach ($this->attributes['pykotaPayments'] as $payment) {
				$parts = explode(' # ', $payment);
				$historyTable->addElement(new htmlOutputText($parts[0]));
				$historyTable->addElement($spacer);
				$amount = new htmlOutputText($parts[1]);
				$amount->alignment = htmlElement::ALIGN_RIGHT;
				$historyTable->addElement($amount);
				$historyTable->addElement($spacer);
				if (!empty($parts[2])) {
					$historyTable->addElement(new htmlOutputText(base64_decode($parts[2])));
				}
				$historyTable->addNewLine();
			}
			$container->addElement($historyTable, true);
		}
		// back button
		$container->addElement(new htmlSpacer(null, '20px'), true);
		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
		$backButton->colspan = 5;
		$backButton->alignment = htmlElement::ALIGN_LEFT;
		$container->addElement($backButton, true);
		return $container;
	}

	/**
	* Processes user input of the payments module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_payments() {
		return array();
	}

	/**
	 * Returns the HTML meta data for the jobs page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_jobs() {
		$container = new htmlTable();
		// jobs
		$jobs = $this->getJobs($this->getCurrentUserName(), $this->moduleSettings['pykotaUser_jobSuffix'][0]);
		$spacer = new htmlSpacer('10px', null);
		// header
		$container->addElement(new htmlOutputText(_('Date')), false, true);
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_('Printer')), false, true);
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_('Price')), false, true);
		$container->addElement($spacer);
		$container->addElement(new htmlOutputText(_('Size')), false, true);
		$container->addElement($spacer);
		$title = new htmlOutputText(_('Title'));
		$title->alignment = htmlElement::ALIGN_LEFT;
		$container->addElement($title, true, true);
		// jobs
		foreach ($jobs as $job) {
			$container->addElement(new htmlOutputText(formatLDAPTimestamp($job['createtimestamp'][0])));
			$container->addElement($spacer);
			$container->addElement(new htmlOutputText($job['pykotaprintername'][0]));
			$container->addElement($spacer);
			$price = new htmlOutputText($job['pykotajobprice'][0]);
			$price->alignment = htmlElement::ALIGN_RIGHT;
			$container->addElement($price);
			$container->addElement($spacer);
			$size = new htmlOutputText($job['pykotajobsize'][0]);
			$size->alignment = htmlElement::ALIGN_RIGHT;
			$container->addElement($size);
			$container->addElement($spacer);
			$container->addElement(new htmlOutputText($job['pykotatitle'][0]), true);
		}
		// back button
		$container->addElement(new htmlSpacer(null, '20px'), true);
		$backButton = new htmlAccountPageButton(get_class($this), 'attributes', 'back', _('Back'));
		$backButton->colspan = 5;
		$backButton->alignment = htmlElement::ALIGN_LEFT;
		$container->addElement($backButton, true);
		return $container;
	}

	/**
	* Processes user input of the jobs module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_jobs() {
		return array();
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
		if (!in_array('pykotaAccount', $this->attributes['objectClass']) && !in_array('pykotaAccount', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	* This function is used to check if all settings for this module have been made.
	*
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* This function tells LAM if it can create/modify the LDAP account. If your module needs any
	* additional input then set this to false. The user will be notified that your module needs
	* more input.<br>
	* This method's return value defaults to true.
	*
	* @return boolean true, if settings are complete
	*/
	public function module_complete() {
		if (in_array('pykotaAccount', $this->attributes['objectClass'])) {
			// require uid
			$uid = $this->getCurrentUserName();
			return !empty($uid);
		}
		return true;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		// profile mappings in meta data
		parent::load_profile($profile);
		// add extension
		if (isset($profile['pykotaUser_addExt'][0]) && ($profile['pykotaUser_addExt'][0] == "true")) {
			if (!in_array('pykotaAccount', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'pykotaAccount';
			}
			if (!in_array('pykotaAccountBalance', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'pykotaAccountBalance';
			}
		}
		if (!empty($profile['pykotaUser_pykotaBalance']) && empty($this->attributes['pykotaBalance'][0])) {
			$amount = $profile['pykotaUser_pykotaBalance'][0];
			if (strpos($amount, '.') === false) {
				$amount .= '.0';
			}
			$this->attributes['pykotaBalance'][0] = $amount;
			$this->attributes['pykotaLifeTimePaid'][0] = $amount;
			$this->attributes['pykotaPayments'][] = getFormattedTime('Y-m-d H:i:s,00') . ' # ' . $amount . ' # ';
		}
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::getManagedAttributes()
	 */
	function get_uploadColumns($selectedModules, &$type) {
		$return = parent::get_uploadColumns($selectedModules, $type);
		if ($this->manageUid($selectedModules)) {
			$return[] = array(
				'name' => 'pykotaUser_uid',
				'description' => _('User name'),
				'help' => 'uid',
				'example' => _('smiller'),
				'required' => true,
				'unique' => true,
			);
		}
		if ($this->manageMail($selectedModules)) {
			$return[] = array(
				'name' => 'pykotaUser_mail',
				'description' => _('Email address'),
				'help' => 'mail',
				'example' => _('user@company.com'),
			);
		}
		if ($this->manageDescription($selectedModules)) {
			$return[] = array(
				'name' => 'pykotaUser_description',
				'description' => _('Description'),
				'help' => 'description',
				'example' => _('Temp, contract till December'),
			);
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object classes
			if (!in_array('pykotaAccount', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaAccount';
			}
			if (!in_array('pykotaAccountBalance', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaAccountBalance';
			}
			if ($this->isStructural() && !in_array('pykotaObject', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'pykotaObject';
			}
			// cn
			if ($this->isStructural()) {
				$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['pykotaUser_cn']];
			}
			// uid
			if ($this->manageUid($selectedModules) && !empty($rawAccounts[$i][$ids['pykotaUser_uid']])) {
				if (!get_preg($rawAccounts[$i][$ids['pykotaUser_uid']], 'username')) {
					$errMsg = $this->messages['uid'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				elseif ($this->uidExists($rawAccounts[$i][$ids['pykotaUser_uid']])) {
					$errMsg = $this->messages['uid'][3];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['pykotaUser_uid']];
				}
			}
			// mail
			if ($this->manageUid($selectedModules) && !empty($rawAccounts[$i][$ids['pykotaUser_mail']])) {
				$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'pykotaUser_mail', 'mail',
					'email', $this->messages['mail'][1], $messages);
			}
			// description
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'pykotaUser_description', 'description');
			// PyKota user name
			if (!empty($rawAccounts[$i][$ids['pykotaUser_pykotaUserName']])) {
				if (!get_preg($rawAccounts[$i][$ids['pykotaUser_pykotaUserName']], 'username')) {
					$errMsg = $this->messages['pykotaUserName'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				elseif ($this->pykotaUserNameExists($rawAccounts[$i][$ids['pykotaUser_pykotaUserName']])) {
					$errMsg = $this->messages['pykotaUserName'][3];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['pykotaUserName'] = $rawAccounts[$i][$ids['pykotaUser_pykotaUserName']];
				}
			}
			// limit by
			if (!empty($rawAccounts[$i][$ids['pykotaUser_pykotaLimitBy']])) {
				if (isset($this->limitOptions[$rawAccounts[$i][$ids['pykotaUser_pykotaLimitBy']]])) {
					$partialAccounts[$i]['pykotaLimitBy'] = $this->limitOptions[$rawAccounts[$i][$ids['pykotaUser_pykotaLimitBy']]];
				}
				else {
					$errMsg = $this->messages['pykotaLimitBy'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['pykotaLimitBy'] = 'quota';
			}
			// overcharge factor
			if (!empty($rawAccounts[$i][$ids['pykotaUser_pykotaOverCharge']])) {
				$pykotaOverCharge = $rawAccounts[$i][$ids['pykotaUser_pykotaOverCharge']];
				if (strpos($pykotaOverCharge, '.') === false) {
					$pykotaOverCharge .= '.0';
				}
				if (!get_preg($pykotaOverCharge, 'float')) {
					$errMsg = $this->messages['pykotaOverCharge'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['pykotaOverCharge'] = $pykotaOverCharge;
				}
			}
			else {
				$partialAccounts[$i]['pykotaOverCharge'] = '1.0';
			}
			// balance
			if (!empty($rawAccounts[$i][$ids['pykotaUser_pykotaBalance']])) {
				$balance = $rawAccounts[$i][$ids['pykotaUser_pykotaBalance']];
				if (strpos($balance, '.') === false) {
					$balance .= '.0';
				}
				if (!get_preg($balance, 'float')) {
					$errMsg = $this->messages['pykotaBalance'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
				else {
					$partialAccounts[$i]['pykotaBalance'] = $balance;
					$partialAccounts[$i]['pykotaLifeTimePaid'] = $balance;
					$comment = '';
					if (!empty($rawAccounts[$i][$ids['pykotaUser_pykotaBalanceComment']])) {
						$comment = base64_encode($rawAccounts[$i][$ids['pykotaUser_pykotaBalanceComment']]);
					}
					$partialAccounts[$i]['pykotaPayments'][0] = getFormattedTime('Y-m-d H:i:s,00') . ' # ' . $balance . ' # ' . $comment;
				}
			}
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfFields()
	 */
	public function get_pdfFields($typeId) {
		$fields = parent::get_pdfFields($typeId);
		$typeManager = new TypeManager();
		$modules = $typeManager->getConfiguredType($typeId)->getModules();
		if ($this->manageUid($modules)) {
			$fields['uid'] = _('User name');
		}
		if ($this->manageMail($modules)) {
			$fields['mail'] = _('Email address');
		}
		if ($this->manageDescription($modules)) {
			$fields['description'] = _('Description');
		}
		return $fields;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('Common name'));
		$this->addSimplePDFField($return, 'uid', _('User name'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'pykotaUserName', _('PyKota user name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$limitByOptions = array_flip($this->limitOptions);
		$limitByValue = '';
		if (!empty($this->attributes['pykotaLimitBy'][0]) && isset($limitByOptions[$this->attributes['pykotaLimitBy'][0]])) {
			$limitByValue = $limitByOptions[$this->attributes['pykotaLimitBy'][0]];
		}
		$this->addPDFKeyValue($return, 'pykotaLimitBy', _('Limit type'), $limitByValue);
		$this->addSimplePDFField($return, 'pykotaOverCharge', _('Overcharge factor'));
		$this->addSimplePDFField($return, 'pykotaBalance', _('Balance'));
		$this->addSimplePDFField($return, 'pykotaLifeTimePaid', _('Total paid'));
		// payment history
		if (!empty($this->attributes['pykotaPayments'][0])) {
			$pdfTable = new PDFTable();
			$pdfRow = new PDFTableRow();
			$pdfRow->cells[] = new PDFTableCell(_('Date'), '20%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Amount'), '10%', null, true);
			$pdfRow->cells[] = new PDFTableCell(_('Comment'), '70%', null, true);
			$pdfTable->rows[] = $pdfRow;
			for ($i = 0; $i < sizeof($this->attributes['pykotaPayments']); $i++) {
				$parts = explode(' # ', $this->attributes['pykotaPayments'][$i]);
				$comment = ' ';
				if (!empty($parts[2])) {
					$comment = base64_decode($parts[2]);
				}
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($parts[0], '20%');
				$pdfRow->cells[] = new PDFTableCell($parts[1], '10%', PDFTableCell::ALIGN_RIGHT);
				$pdfRow->cells[] = new PDFTableCell($comment, '70%');
				$pdfTable->rows[] = $pdfRow;
			}
			$this->addPDFTable($return, 'pykotaPayments', $pdfTable);
		}
		return $return;
	}

	/**
	 * Returns the meta HTML code for each input field.
	 * format: array(<field1> => array(<META HTML>), ...)
	 * It is not possible to display help links.
	 *
	 * @param array $fields list of active fields
	 * @param array $attributes attributes of LDAP account
	 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
	 * @param array $readOnlyFields list of read-only fields
	 * @return array list of meta HTML elements (field name => htmlResponsiveRow)
	 */
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		if ($passwordChangeOnly) {
			return array(); // no Kolab fields as long no LDAP content can be read
		}
		if (!in_array('pykotaAccount', $attributes['objectClass'])) {
			return array();
		}
		$return = array();
		if (in_array('pykotaBalance', $fields)) {
			$pykotaBalance = '';
			if (isset($attributes['pykotaBalance'][0])) $pykotaBalance = $attributes['pykotaBalance'][0];
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('pykotaBalance', _('Balance'))));
			$row->addField(new htmlOutputText($pykotaBalance));
			$return['pykotaBalance'] = $row;
		}
		if (in_array('pykotaLifeTimePaid', $fields)) {
			$pykotaLifeTimePaid = '';
			if (isset($attributes['pykotaLifeTimePaid'][0])) $pykotaLifeTimePaid = $attributes['pykotaLifeTimePaid'][0];
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('pykotaLifeTimePaid', _('Total paid'))));
			$row->addField(new htmlOutputText($pykotaLifeTimePaid));
			$return['pykotaLifeTimePaid'] = $row;
		}
		// payment history
		if (in_array('pykotaPayments', $fields)) {
			$pykotaPayments = new htmlResponsiveRow();
			if (!empty($attributes['pykotaPayments'][0])) {
				$pykotaPayments->add(new htmlOutputText(_('Date')), 3, 3, 3, 'bold');
				$pykotaPayments->add(new htmlOutputText(_('Amount')), 3, 3, 3, 'bold');
				$pykotaPayments->add(new htmlOutputText(_('Comment')), 6, 6, 6, 'bold');
				rsort($attributes['pykotaPayments']);
				foreach ($attributes['pykotaPayments'] as $payment) {
					$parts = explode(' # ', $payment);
					$pykotaPayments->add(new htmlOutputText($parts[0]), 3);
					$amount = new htmlOutputText($parts[1]);
					$amount->alignment = htmlElement::ALIGN_RIGHT;
					$pykotaPayments->add($amount, 3);
					$comment = empty($parts[2]) ? '' : $parts[2];
					$pykotaPayments->add(new htmlOutputText(base64_decode($parts[2])), 6);
				}
			}
			$row->add(new htmlSpacer(null, '10px'), 12);
			$row = new htmlResponsiveRow();
			$row->add(new htmlOutputText($this->getSelfServiceLabel('pykotaPayments', _('Payment history'))), 12, 12, 12, 'bold text-left');
			$row->add($pykotaPayments, 12);
			$return['pykotaPayments'] = $row;
		}
		// job history
		if (in_array('pykotaJobHistory', $fields) && !empty($this->selfServiceSettings->moduleSettings['pykotaUser_jobSuffix'][0]) && !empty($attributes['pykotaUserName'][0])) {
			$jobs = $this->getJobs($attributes['pykotaUserName'][0], $this->selfServiceSettings->moduleSettings['pykotaUser_jobSuffix'][0]);
			$pykotaJobs = new htmlResponsiveRow();
			$pykotaJobs->add(new htmlOutputText(_('Date')), 3, 3, 3, 'bold');
			$pykotaJobs->add(new htmlOutputText(_('Printer')), 3, 3, 3, 'bold');
			$pykotaJobs->add(new htmlOutputText(_('Price')), 2, 2, 2, 'bold');
			$pykotaJobs->add(new htmlOutputText(_('Size')), 1, 1, 1, 'bold');
			$title = new htmlOutputText(_('Title'));
			$title->alignment = htmlElement::ALIGN_LEFT;
			$pykotaJobs->add($title, 3, 3, 3, 'bold');
			foreach ($jobs as $job) {
				$pykotaJobs->add(new htmlOutputText(formatLDAPTimestamp($job['createtimestamp'][0])), 3);
				$pykotaJobs->add(new htmlOutputText($job['pykotaprintername'][0]), 3);
				$price = new htmlOutputText($job['pykotajobprice'][0]);
				$price->alignment = htmlElement::ALIGN_RIGHT;
				$pykotaJobs->add($price, 2);
				$size = new htmlOutputText($job['pykotajobsize'][0]);
				$size->alignment = htmlElement::ALIGN_RIGHT;
				$pykotaJobs->add($size, 1);
				$pykotaJobs->add(new htmlOutputText($job['pykotatitle'][0]), 3);
			}
			$row = new htmlResponsiveRow();
			$row->add(new htmlSpacer(null, '10px'), 12);
			$row->add(new htmlOutputText($this->getSelfServiceLabel('pykotaJobHistory', _('Job history'))), 12, 12, 12, 'bold text-left');
			$row->add($pykotaJobs, 12);
			$return['pykotaJobHistory'] = $row;
		}
		return $return;
	}

	/**
	 * Checks if the self service settings are valid.
	 *
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * If the input data is invalid the return value is an array that contains arrays
	 * to build StatusMessages (message type, message head, message text). If no errors
	 * occured the function returns an empty array.
	 *
	 * @param array $options hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	 * @param selfServiceProfile $profile self service profile
	 * @return array error messages
	 */
	public function checkSelfServiceSettings(&$options, &$profile) {
		$errors = array();
		if (get_class($this) == 'pykotaUser') {
			if (!empty($options['pykotaUser_jobSuffix'][0]) && !get_preg($options['pykotaUser_jobSuffix'][0], 'dn')) {
				$errors[] = $this->messages['jobSuffix'][0];
			}
		}
		return $errors;
	}

	/**
	 * Returns if the uid attribute should be managed.
	 *
	 * @param string[] $modules account modules
	 * @return boolean manage uid attribute
	 */
	private function manageUid($modules) {
		if (in_array('inetOrgPerson', $modules)
			|| in_array('posixAccount', $modules)) {
			return false;
		}
		else {
			return true;
		}
	}

	/**
	 * Returns if the mail attribute should be managed.
	 *
	 * @param string[] $modules account modules
	 * @return boolean manage mail attribute
	 */
	private function manageMail($modules) {
		if (in_array('inetOrgPerson', $modules)) {
			return false;
		}
		else {
			return true;
		}
	}

	/**
	 * Returns if the description attribute should be managed.
	 *
	 * @param string[] $modules account modules
	 * @return boolean manage description attribute
	 */
	private function manageDescription($modules) {
		if (in_array('inetOrgPerson', $modules)) {
			return false;
		}
		else {
			return true;
		}
	}

	/**
	 * Returns if the given uid already exists.
	 *
	 * @param String $uid uid attribute value
	 * @return boolean uid exists
	 */
	private function uidExists($uid) {
		if ($this->uidCache == null) {
			$this->loadUserNameCache();
		}
		return in_array($uid, $this->uidCache);
	}

	/**
	 * Returns if the given pykotaUserName already exists.
	 *
	 * @param String $pykotaUserName pykotaUserName attribute value
	 * @return boolean pykotaUserName exists
	 */
	private function pykotaUserNameExists($pykotaUserName) {
		if ($this->pykotaUserNameCache == null) {
			$this->loadUserNameCache();
		}
		return in_array($pykotaUserName, $this->pykotaUserNameCache);
	}

	/**
	 * Loads the list of user names into the cache.
	 */
	private function loadUserNameCache() {
		$results = searchLDAPByFilter('(|(uid=*)(pykotaUserName=*))', array('uid', 'pykotaUserName'), array($this->get_scope()));
		$this->uidCache = array();
		$this->pykotaUserNameCache = array();
		foreach ($results as $result) {
			if (isset($result['uid'][0])) {
				$this->uidCache[] = $result['uid'][0];
			}
			if (isset($result['pykotausername'][0])) {
				$this->pykotaUserNameCache[] = $result['pykotausername'][0];
			}
		}
	}

	/**
	 * Returns the current user name (uid) of this account.
	 *
	 * @return String user name
	 */
	private function getCurrentUserName() {
		if (!empty($this->attributes['uid'][0])) {
			return $this->attributes['uid'][0];
		}
		if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
			$posix = $this->getAccountContainer()->getAccountModule('posixAccount');
			$attrs = $posix->getAttributes();
			if (!empty($attrs['uid'][0])) {
				return $attrs['uid'][0];
			}
		}
		if ($this->getAccountContainer()->getAccountModule('inetOrgPerson') != null) {
			$personal = $this->getAccountContainer()->getAccountModule('inetOrgPerson');
			$attrs = $personal->getAttributes();
			if (!empty($attrs['uid'][0])) {
				return $attrs['uid'][0];
			}
		}
		return '';
	}

	/**
	 * Returns a list of jobs for the given user reverse-sorted by date.
	 * The number of jobs is limited to 100.
	 *
	 * @param String $user user name
	 * @param String $suffix LDAP suffix for job objects
	 */
	private function getJobs($user, $suffix) {
		$attrs = array('createTimestamp', 'pykotaTitle', 'pykotaPrinterName', 'pykotaJobPrice', 'pykotaJobSize');
		if (isset($_SESSION['ldapHandle']) && is_resource($_SESSION['ldapHandle'])) {
			$handle = $_SESSION['ldapHandle'];
		}
		else {
			$handle = $_SESSION['ldap']->server();
		}
		$sr = @ldap_search($handle, $suffix, '(&(objectClass=pykotaJob)(pykotaUserName=' . $user . '))', $attrs);
		if (!$sr) {
			return array();
		}
		$jobList = ldap_get_entries($handle, $sr);
		if (!$jobList) {
			return array();
		}
		cleanLDAPResult($jobList);
		@ldap_free_result($sr);
		$jobs = array();
		foreach ($jobList as $index => $job) {
			$jobs[$job['createtimestamp'][0] . $index] = $job;
		}
		krsort($jobs);
		if (sizeof($jobs) > 100) {
			$jobs = array_slice($jobs, 0, 100);
		}
		return array_values($jobs);
	}

}


?>
