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
* Manages Kolab shared folders.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Kolab shared folders.
*
* @package modules
*/
class kolabSharedFolder extends baseModule {
	
	/** cache for mailHost values */
	private $mailHostCache = null;
	
	/**
	* Creates a new kolabSharedFolder object.
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
		$return["alias"] = _("Kolab shared folder");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('kolabSharedFolder');
		// managed attributes
		$return['attributes'] = array('kolabAllowSMTPRecipient', 'kolabAllowSMTPSender', 'kolabDeleteflag', 'acl',
			'alias', 'kolabDelegate', 'kolabFolderType', 'kolabTargetFolder', 'mailHost');
		// help Entries
		$return['help'] = array(
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
			'addExtension' => array(
				"Headline" => _("Add Kolab extension"),
				"Text" => _("Adds this Kolab extension.")
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'kolabSharedFolder_addExtension',
				'description' => _('Add Kolab extension'),
				'help' => 'addExtension',
				'example' => 'false',
				'default' => 'false',
				'values' => 'true, false'
			),
			array(
				'name' => 'kolabSharedFolder_kolabAllowSMTPRecipient',
				'description' => _('Allowed recepients'),
				'help' => 'kolabAllowSMTPRecipientList',
				'example' => '.com; -.net',
			),
			array(
				'name' => 'kolabSharedFolder_kolabAllowSMTPSender',
				'description' => _('Allowed senders'),
				'help' => 'kolabAllowSMTPSenderList',
				'example' => '.com; -.net',
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'kolabAllowSMTPRecipient' => _('Allowed recepients'),
			'kolabAllowSMTPSender' => _('Allowed senders'),
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	function load_Messages() {
		$this->messages['kolabAllowSMTPRecipient'][0] = array('ERROR', _('Allowed recepients'), _('Please enter a valid recepient expression.'));
		$this->messages['kolabAllowSMTPRecipient'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_kolabAllowSMTPRecipient', _('Please enter a valid recepient expression.'));
		$this->messages['kolabAllowSMTPSender'][0] = array('ERROR', _('Allowed senders'), _('Please enter a valid sender expression.'));
		$this->messages['kolabAllowSMTPSender'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_kolabAllowSMTPSender', _('Please enter a valid sender expression.'));
		$this->messages['extension'][1] = array('ERROR', _('Account %s:') . ' kolabSharedFolder_addExtension', _('You cannot add the Kolab group and shared folder extension at the same time.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$container = new htmlTable();
		if (isset($this->attributes['objectClass']) && in_array('kolabSharedFolder', $this->attributes['objectClass'])) {
			// check if account is marked for deletion
			if (isset($this->attributes['kolabDeleteflag'])) {
				$container->addElement(new htmlOutputText(_('This account is marked for deletion.')));
				return $container;
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
				$deleteContainer->addElement(new htmlAccountPageButton(get_class($this), 'deleteFlag', 'open', _('Mark account for deletion')));
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
			$this->attributes['objectClass'][] = 'kolabSharedFolder';
			return $errors;
		}
		if (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('kolabSharedFolder'), $this->attributes['objectClass']);
			$attrs = $this->meta['attributes'];
			foreach ($attrs as $name) {
				if (isset($this->attributes[$name])) {
					unset($this->attributes[$name]);
				}
			}
			return $errors;
		}
		// skip processing if object class is not set
		if (!isset($this->attributes['objectClass']) || !in_array('kolabSharedFolder', $this->attributes['objectClass'])) {
			return $errors;
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
	function display_html_deleteFlag() {
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
	function process_deleteFlag() {
		if (isset($_POST['form_subpage_kolabSharedFolder_attributes_confirm'])) {
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
		if (!in_array('kolabSharedFolder', $this->attributes['objectClass']) && !in_array('kolabSharedFolder', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
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
			// check if extension should be added
			if (isset($rawAccounts[$i][$ids['kolabSharedFolder_addExtension']]) && ($rawAccounts[$i][$ids['kolabSharedFolder_addExtension']] != 'true')) {
				continue;
			}
			// check if both group and shared folder enxtension should be added
			if (isset($rawAccounts[$i][$ids['kolabSharedFolder_addExtension']]) && ($rawAccounts[$i][$ids['kolabSharedFolder_addExtension']] == 'true')
					&& isset($rawAccounts[$i][$ids['kolabGroup_addExtension']]) && ($rawAccounts[$i][$ids['kolabGroup_addExtension']] == 'true')) {
				$errMsg = $this->messages['extension'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
				continue;
			}
			// add object classes
			if (!in_array('kolabSharedFolder', $partialAccounts[$i]['objectClass'])) {
				$partialAccounts[$i]['objectClass'][] = 'kolabSharedFolder';
			}
			// allowed recipients
			if (!empty($rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPRecipient']])) {
				$mails = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPRecipient']]);
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
			if (!empty($rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPSender']])) {
				$mails = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['kolabSharedFolder_kolabAllowSMTPSender']]);
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
		$this->addSimplePDFField($return, 'kolabAllowSMTPRecipient', _('Allowed recepients'));
		$this->addSimplePDFField($return, 'kolabAllowSMTPSender', _('Allowed senders'));
		return $return;
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
	
	/**
	* Controls if the module button the account page is visible and activated.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* <b>Possible return values:</b>
	* <ul>
	* <li><b>enabled:</b> button is visible and active</li>
	* <li><b>disabled:</b> button is visible and deactivated (greyed)</li>
	* <li><b>hidden:</b> no button will be shown</li>
	* </ul>
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	public function getButtonStatus() {
		$module = $this->getAccountContainer()->getAccountModule('kolabGroup');
		if ($module != null) {
			if ($module->extensionIsEnabled()) {
				return 'disabled';
			}
		}
		return "enabled";
	}

	/**
	 * Returns if the extension is enabled for this LDAP entry.
	 * 
	 * @return boolean enabled (= object class added to entry)
	 */
	public function extensionIsEnabled() {
		if (isset($this->attributes['objectClass']) && in_array('kolabSharedFolder', $this->attributes['objectClass'])) {
			return true;
		}
		return false;
	}
	
}


?>
