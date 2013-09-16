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
* Manages Kolab group accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Kolab group accounts.
*
* @package modules
*/
class kolabGroup extends baseModule {
	
	/** cache for mail attribute */
	private $mailCache = null;
	/** cache for mailHost values */
	private $mailHostCache = null;
	
	/**
	* Creates a new kolabGroup object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
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
		$return['icon'] = 'kolab.png';
		// manages host accounts
		$return["account_types"] = array('group');
		// alias name
		$return["alias"] = _("Kolab");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('kolabGroupOfUniqueNames');
		// managed attributes
		$return['attributes'] = array('kolabAllowSMTPRecipient', 'kolabAllowSMTPSender', 'kolabDeleteflag');
		if ($this->manageMail()) {
			$return['attributes'][] = 'mail';
		}
		// help Entries
		$return['help'] = array(
			'mail' => array(
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The list's email address.")
			),
			'mailList' => array(
				"Headline" => _("Email address"), 'attr' => 'mail',
				"Text" => _("The list's email address.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'kolabAllowSMTPRecipient' => array (
				"Headline" => _('Allowed recepients'), 'attr' => 'kolabAllowSMTPRecipient',
				"Text" => _('Describes the allowed or disallowed SMTP recipient addresses for mail sent by this account (e.g. "domain.tld" or "-user@domain.tld").')
			),
			'kolabAllowSMTPRecipientList' => array (
				"Headline" => _('Allowed recepients'), 'attr' => 'kolabAllowSMTPRecipient',
				"Text" => _('Describes the allowed or disallowed SMTP recipient addresses for mail sent by this account (e.g. "domain.tld" or "-user@domain.tld").')
							. ' ' . _("Multiple values are separated by semicolon.")
			),
			'kolabAllowSMTPSender' => array (
				"Headline" => _('Allowed senders'), 'attr' => 'kolabAllowSMTPSender',
				"Text" => _('Describes the allowed or disallowed SMTP addresses sending mail to this account (e.g. "domain.tld" or "-user@domain.tld").')
			),
			'kolabAllowSMTPSenderList' => array (
				"Headline" => _('Allowed senders'), 'attr' => 'kolabAllowSMTPSender',
				"Text" => _('Describes the allowed or disallowed SMTP addresses sending mail to this account (e.g. "domain.tld" or "-user@domain.tld").')
							. ' ' . _("Multiple values are separated by semicolon.")
			),
			'deleteFlag' => array(
				"Headline" => _("Mark for deletion"), 'attr' => 'kolabDeleteflag',
				"Text" => _("This will set a special flag on the account which tells Kolabd to remove it. Use this to cleanly delete Kolab accounts (e.g. this removes mail boxes).")
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
		);
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputCheckbox('kolabGroup_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
		$return['profile_options'] = $profileContainer;
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'kolabGroup_kolabAllowSMTPRecipient',
				'description' => _('Allowed recepients'),
				'help' => 'kolabAllowSMTPRecipientList',
				'example' => '.com; -.net',
			),
			array(
				'name' => 'kolabGroup_kolabAllowSMTPSender',
				'description' => _('Allowed senders'),
				'help' => 'kolabAllowSMTPSenderList',
				'example' => '.com; -.net',
			),
		);
		if ($this->manageMail()) {
			$return['upload_columns'][] = array(
				'name' => 'kolabGroup_mail',
				'description' => _('Email address'),
				'help' => 'mailList',
				'example' => 'list@company.com',
				'required' => true,
				'unique' => true,
			);
		}
		// available PDF fields
		$return['PDF_fields'] = array(
			'kolabAllowSMTPRecipient' => _('Allowed recepients'),
			'kolabAllowSMTPSender' => _('Allowed senders'),
		);
		if ($this->manageMail()) {
			$return['PDF_fields']['mail'] = _('Email address');
		}
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' kolabGroup_mail', _('Please enter a valid email address!'));
		$this->messages['mail'][2] = array('ERROR', _('Email address'), _('Email address already exists.'));
		$this->messages['mail'][3] = array('ERROR', _('Account %s:') . ' kolabGroup_mail', _('Email address already exists.'));
		$this->messages['kolabAllowSMTPRecipient'][0] = array('ERROR', _('Allowed recepients'), _('Please enter a valid recepient expression.'));
		$this->messages['kolabAllowSMTPRecipient'][1] = array('ERROR', _('Account %s:') . ' kolabGroup_kolabAllowSMTPRecipient', _('Please enter a valid recepient expression.'));
		$this->messages['kolabAllowSMTPSender'][0] = array('ERROR', _('Allowed senders'), _('Please enter a valid sender expression.'));
		$this->messages['kolabAllowSMTPSender'][1] = array('ERROR', _('Account %s:') . ' kolabGroup_kolabAllowSMTPSender', _('Please enter a valid sender expression.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		if (isset($this->attributes['objectClass']) && in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass'])) {
			// check if account is marked for deletion
			if (isset($this->attributes['kolabDeleteflag'])) {
				$container->addElement(new htmlOutputText(_('This account is marked for deletion.')));
				return $container;
			}
			// mail
			if ($this->manageMail()) {
				$this->addSimpleInputTextField($container, 'mail', _('Email address'), true);
			}
			// allowed recepients
			$this->addMultiValueInputTextField($container, 'kolabAllowSMTPRecipient', _('Allowed recepients'));
			// allowed senders
			$this->addMultiValueInputTextField($container, 'kolabAllowSMTPSender', _('Allowed senders'));
			// delete flag
			$this->loadMailHostCache();
			if (!$this->getAccountContainer()->isNewAccount && (sizeof($this->mailHostCache) > 0)) {
				$deleteContainer = new htmlTable();
				$deleteContainer->addElement(new htmlSpacer(null, '20px'), true);
				$deleteContainer->addElement(new htmlAccountPageButton(get_class($this), 'delete', 'open', _('Mark account for deletion')));
				$deleteContainer->addElement(new htmlHelpLink('deleteFlag'));
				$container->addElement($deleteContainer);
			}
		}
		else {
			// add button
			$container->addElement(new htmlButton('addObjectClass', _('Add Kolab extension')));
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
			$this->attributes['objectClass'][] = 'kolabGroupOfUniqueNames';
			return $errors;
		}
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('kolabGroupOfUniqueNames'), $this->attributes['objectClass']);
			$attrs = $this->meta['attributes'];
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		// skip processing if object class is not set
		if (!isset($this->attributes['objectClass']) || !in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass'])) {
			return $errors;
		}
		// mail
		if ($this->manageMail()) {
			if (!empty($_POST['mail'])) {
				$this->attributes['mail'][0] = $_POST['mail'];
				// check format
				if (!get_preg($_POST['mail'], 'email')) {
					$errors[] = $this->messages['mail'][0];
				}
				// check if unique
				if ($this->getAccountContainer()->isNewAccount || (!empty($this->orig['mail'][0]) && ($this->orig['mail'][0] != $this->attributes['mail'][0]))) {
					$this->loadMailCache();
					if (in_array_ignore_case($_POST['mail'], $this->mailCache)) {
						$errors[] = $this->messages['mail'][2];
					}
				}
			}
			elseif (isset($this->attributes['mail'])) {
				unset($this->attributes['mail']);
			}
		}
		// allowed recepients
		$this->processMultiValueInputTextField('kolabAllowSMTPRecipient', $errors, 'kolabEmailPrefix');
		// allowed senders
		$this->processMultiValueInputTextField('kolabAllowSMTPSender', $errors, 'kolabEmailPrefix');
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page to mark an account for deletion.
	* 
	* @return htmlElement HTML meta data
	*/
	function display_html_delete() {
		$return = new htmlTable();
		$message = new htmlOutputText(_('Do you really want to mark this account for deletion?'));
		$return->addElement($message, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$serverTable = new htmlTable();
		$serverTable->addElement(new htmlTableExtendedSelect('deletionServer', $this->mailHostCache, array(), _('Server'), 'deleteFlag'));
		$return->addElement($serverTable, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$buttonGroup = new htmlGroup();
		$buttonGroup->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'confirm', _('Mark account for deletion')));
		$buttonGroup->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'cancel', _('Cancel')));
		$return->addElement($buttonGroup, true);
		return $return;
	}

	/**
	* Write variables into object and do some regex checks
	*/
	function process_delete() {
		if (isset($_POST['form_subpage_kolabGroup_attributes_confirm'])) {
			// set delete flag
			$this->attributes['kolabDeleteflag'][0] = $_POST['deletionServer'];
		}
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
		if (!in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass']) && !in_array('kolabGroupOfUniqueNames', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
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
		if (isset($profile['kolabGroup_addExt'][0]) && ($profile['kolabGroup_addExt'][0] == "true")) {
			if (!in_array('kolabGroupOfUniqueNames', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'kolabGroupOfUniqueNames';
			}
		}
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
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object classes
			if (!in_array('kolabGroupOfUniqueNames', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'kolabGroupOfUniqueNames';
			}
			// mail
			if ($this->manageMail() && !empty($rawAccounts[$i][$ids['kolabGroup_mail']])) {
				if (get_preg($rawAccounts[$i][$ids['kolabGroup_mail']], 'email')) {
					$this->loadMailCache();
					if (!in_array_ignore_case(trim($rawAccounts[$i][$ids['kolabGroup_mail']]), $this->mailCache)) {
						$partialAccounts[$i]['mail'] = trim($rawAccounts[$i][$ids['kolabGroup_mail']]);
					}
					else {
						$errMsg = $this->messages['mail'][3];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
				else {
					$errMsg = $this->messages['mail'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// allowed recipients
			if (!empty($rawAccounts[$i][$ids['kolabGroup_kolabAllowSMTPRecipient']])) {
				$mails = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['kolabGroup_kolabAllowSMTPRecipient']]);
				for ($m = 0; $m < sizeof($mails); $m++) {
					if (get_preg($mails[$m], 'kolabEmailPrefix')) {
						$partialAccounts[$i]['kolabAllowSMTPRecipient'][] = $mails[$m];
					}
					else {
						$errMsg = $this->messages['kolabAllowSMTPRecipient'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
						break;
					}
				}
			}
			// allowed senders
			if (!empty($rawAccounts[$i][$ids['kolabGroup_kolabAllowSMTPSender']])) {
				$mails = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['kolabGroup_kolabAllowSMTPSender']]);
				for ($m = 0; $m < sizeof($mails); $m++) {
					if (get_preg($mails[$m], 'kolabEmailPrefix')) {
						$partialAccounts[$i]['kolabAllowSMTPSender'][] = $mails[$m];
					}
					else {
						$errMsg = $this->messages['kolabAllowSMTPSender'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
						break;
					}
				}
			}
		}
		return $messages;
	}
	
	/**
	* Returns a list of PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'kolabAllowSMTPRecipient', _('Allowed recepients'));
		$this->addSimplePDFField($return, 'kolabAllowSMTPSender', _('Allowed senders'));
		return $return;
	}
	
	/**
	 * Returns if the mail attribute should be managed.
	 * 
	 * @return boolean manage mail attribute
	 */
	private function manageMail() {
		if (isset($_SESSION['config'])) {
			$conf = $_SESSION['config'];
			if (in_array('qmailGroup', $conf->get_AccountModules($this->get_scope()))) {
				return false;
			}
			else {
				return true;
			}
		}
		return false;
	}

	/**
	 * Loads the list of email addresses into the cache.
	 */
	private function loadMailCache() {
		if ($this->mailCache != null) {
			return;
		}
		$results = searchLDAPByFilter('(mail=*)', array('mail'), array($this->get_scope()));
		$this->mailCache = array();
		foreach ($results as $result) {
			if (isset($result['mail'][0])) {
				$this->mailCache[] = $result['mail'][0];
			}
		}
	}
	
	/**
	 * Loads the list of mail hosts into the cache.
	 */
	private function loadMailHostCache() {
		if ($this->mailHostCache != null) {
			return;
		}
		$results = searchLDAPByFilter('(mailHost=*)', array('mailHost'), array('user'));
		$this->mailHostCache = array();
		foreach ($results as $result) {
			if (isset($result['mailhost'][0]) && !in_array_ignore_case($result['mailhost'][0], $this->mailHostCache)) {
				$this->mailHostCache[] = $result['mailhost'][0];
			}
		}
	}
	
}


?>
