<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2013  Roland Gruber

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
* Manages Kolab user accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Kolab user accounts.
*
* @package modules
*/
class kolabUser extends baseModule {

	/** list of invitation policies */
	private $invitationPolicies;

	/**
	* Creates a new kolabUser object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		// list of invitation policies
		$this->invitationPolicies = array(
			'ACT_ALWAYS_ACCEPT' => _('Always accept'),
			'ACT_ALWAYS_REJECT' => _('Always reject'),
			'ACT_MANUAL' => _('Manual'),
			'ACT_REJECT_IF_CONFLICTS' => _('Reject if conflicts'),
			'ACT_MANUAL_IF_CONFLICTS' => _('Manual if conflicts')
		);
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
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Kolab");
		// module dependencies
		$return['dependencies'] = array('depends' => array('inetOrgPerson'), 'conflicts' => array());
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=kolabInetOrgPerson)");
		// managed object classes
		$return['objectClasses'] = array('kolabInetOrgPerson');
		// managed attributes
		$return['attributes'] = array('alias', 'mailHost', 'kolabDelegate', 'kolabInvitationPolicy', 'kolabDeleteflag');
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Mail server'), 'kolab_mailHost', null, 'mailHost'), true);
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['kolab_mailHost'] = array(
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['mailHost'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'kolab_mailHost' => 'mailHost',
		);
		// self service field settings
		$return['selfServiceFieldSettings'] = array(
			'kolabDelegate' => _('Delegates'),
			'kolabInvitationPolicy' => _('Invitation policy')
		);
		// possible self service read-only fields
		$return['selfServiceReadOnlyFields'] = array('kolabDelegate', 'kolabInvitationPolicy');
		// help Entries
		$return['help'] = array(
			'invPol' => array(
				"Headline" => _("Invitation policy"), 'attr' => 'kolabInvitationPolicy',
				"Text" => _("For automatic invitation handling.")
			),
			'invPolList' => array(
				"Headline" => _("Invitation policy list"), 'attr' => 'kolabInvitationPolicy',
				"Text" => _("This is a comma separated list of invitation policies.")
			),
			'delegate' => array(
				"Headline" => _("Delegates"), 'attr' => 'kolabDelegate',
				"Text" => _("A user may define who is allowed to act on behalf of herself. This property is checked when using the Kolab smtp daemon (Postfix) to send emails.")
			),
			'delegateList' => array(
				"Headline" => _("Delegates"), 'attr' => 'kolabDelegate',
				"Text" => _("This is a comma separated list of delegates.")
			),
			'alias' => array(
				"Headline" => _("Email alias"), 'attr' => 'alias',
				"Text" => _("Email alias for this account.")
			),
			'aliasList' => array(
				"Headline" => _("Email alias list"), 'attr' => 'alias',
				"Text" => _("This is a comma separated list of eMail aliases.")
			),
			'mailHost' => array(
				"Headline" => _("Mailbox home server"), 'attr' => 'mailHost',
				"Text" => _("The name of the server where the mailbox is located.")
			),
			'deleteFlag' => array(
				"Headline" => _("Mark for deletion"), 'attr' => 'kolabDeleteflag',
				"Text" => _("This will set a special flag on the account which tells Kolabd to remove it. Use this to cleanly delete Kolab accounts (e.g. this removes mail boxes).")
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'kolabUser_invPol',
				'description' => _('Invitation policy'),
				'help' => 'invPolList',
				'example' => 'user@domain:ACT_ALWAYS_ACCEPT,user2@domain:ACT_MANUAL'
			),
			array(
				'name' => 'kolabUser_mailHost',
				'description' => _('Mailbox home server'),
				'help' => 'mailHost',
				'example' => 'localhost',
				'required' => true
			),
			array(
				'name' => 'kolabUser_aliases',
				'description' => _('Email aliases'),
				'help' => 'aliasList',
				'example' => 'user@domain,user2@domain'
			),
			array(
				'name' => 'kolabUser_delegates',
				'description' => _('Delegates'),
				'help' => 'delegateList',
				'example' => 'user@domain,user2@domain'
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'invPol' => _('Invitation policy'),
			'mailHost' => _('Mailbox home server'),
			'aliases' => _('Email aliases'),
			'delegate' => _('Delegates'),
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['invPol'][0] = array('ERROR', _('Target of invitation policy is invalid!'));  // third array value is set dynamically
		$this->messages['invPol'][1] = array('ERROR', _('Account %s:') . ' kolabUser_invPol', _('Policy list has invalid format!'));
		$this->messages['alias'][0] = array('ERROR', _('Email alias is invalid!'));  // third array value is set dynamically
		$this->messages['alias'][1] = array('ERROR', _('Account %s:') . ' kolabUser_aliases', _('Email alias list has invalid format!'));
		$this->messages['mailHost'][0] = array('ERROR', _('Mailbox home server name is invalid!'));  // third array value is set dynamically
		$this->messages['mailHost'][1] = array('ERROR', _('Account %s:') . ' kolabUser_mailHost', _('Mailbox home server name is invalid!'));
		$this->messages['mailHost'][2] = array('ERROR', _('Mailbox home server name is empty!'));
		$this->messages['delegate'][0] = array('ERROR', _('Account %s:') . ' kolabUser_delegate', _('Unknown delegate address: %s'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		if (isset($this->attributes['objectClass']) && in_array('kolabInetOrgPerson', $this->attributes['objectClass'])) {
			$attrsI = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			if ($this->getAccountContainer()->isNewAccount) {
				if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
					$attrsP = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
					if (!isset($attrsP['userPassword'][0]) || ($attrsP['userPassword'][0] == '')) {
						$message = new htmlStatusMessage('ERROR', _("Please enter a user password."));
						$message->colspan = 5;
						$return->addElement($message, true);
					}
				}
				elseif (!isset($attrsI['userPassword'][0]) || ($attrsI['userPassword'][0] == '')) {
					$message = new htmlStatusMessage('ERROR', _("Please enter a user password."));
					$message->colspan = 5;
					$return->addElement($message, true);
				}
			}
			if (!isset($attrsI['mail'][0])) {
				$message = new htmlStatusMessage('ERROR', _("Please enter an email address on this page: %s"), '', array($this->getAccountContainer()->getAccountModule('inetOrgPerson')->get_alias()));
				$message->colspan = 5;
				$return->addElement($message, true);
			}
			// check if account is marked for deletion
			if (isset($this->attributes['kolabDeleteflag'])) {
				$return->addElement(new htmlOutputText(_('This account is marked for deletion.')));
				return $return;
			}
			$basicPartContainer = new htmlTable();
			// mailbox server
			if (!isset($this->orig['mailHost'][0])) {  // value currently not set
				$mailHost = '';
				if (isset($this->attributes['mailHost'][0])) {
					$mailHost = $this->attributes['mailHost'][0];
				}
				$serverInput = new htmlTableExtendedInputField(_('Mailbox home server'), 'mailHost', $mailHost, 'mailHost');
				$serverInput->setRequired(true);
				$basicPartContainer->addElement($serverInput, true);
			}
			else {  // input is unchangable when set
				$basicPartContainer->addElement(new htmlOutputText(_('Mailbox home server')));
				$basicPartContainer->addElement(new htmlOutputText($this->attributes['mailHost'][0]));
				$basicPartContainer->addElement(new htmlHelpLink('mailHost'), true);
			}
			$return->addElement($basicPartContainer, true);
			// invitation policies
			$return->addElement(new htmlSubTitle(_('Invitation policy')), true);
			$invitationContainer = new htmlTable();
			// default invitation policy
			$defaultInvPol = $this->invitationPolicies['ACT_MANUAL'];
			if (isset($this->attributes['kolabInvitationPolicy'])) {
				for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
					$parts = explode(":", $this->attributes['kolabInvitationPolicy'][$i]);
					if (sizeof($parts) == 1) {
						$defaultInvPol = $this->invitationPolicies[$this->attributes['kolabInvitationPolicy'][$i]];
						unset($this->attributes['kolabInvitationPolicy'][$i]);
						$this->attributes['kolabInvitationPolicy'] = array_values($this->attributes['kolabInvitationPolicy']);
						break;
					}
				}
			}
			$invitationContainer->addElement(new htmlTableExtendedSelect('defaultInvPol', array_values($this->invitationPolicies), array($defaultInvPol), _('Anyone'), 'invPol'), true);
			// other invitation policies
			if (isset($this->attributes['kolabInvitationPolicy'])) {
				for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
					$parts = explode(":", $this->attributes['kolabInvitationPolicy'][$i]);
					if (sizeof($parts) == 2) {
						$invitationContainer->addElement(new htmlInputField('invPol1' . $i, $parts[0]));
						$invitationContainer->addElement(new htmlSelect('invPol2' . $i, array_values($this->invitationPolicies), array($this->invitationPolicies[$parts[1]])));
						$invitationContainer->addElement(new htmlButton('delInvPol' . $i, 'del.png', true));
						$invitationContainer->addElement(new htmlHelpLink('invPol'), true);
					}
				}
			}
			// input box for new invitation policy
			$invitationContainer->addElement(new htmlInputField('invPol1', ''));
			$invitationContainer->addElement(new htmlSelect('invPol2', array_values($this->invitationPolicies)));
			$invitationContainer->addElement(new htmlButton('addInvPol', 'add.png', true));
			$invitationContainer->addElement(new htmlHelpLink('invPol'), true);
			$return->addElement($invitationContainer, true);
			// mail aliases
			$return->addElement(new htmlSubTitle(_('Email aliases')), true);
			$mailAliasContainer = new htmlTable();
			if (isset($this->attributes['alias'])) {
				for ($i = 0; $i < sizeof($this->attributes['alias']); $i++) {
					$mailAliasContainer->addElement(new htmlInputField('alias' . $i, $this->attributes['alias'][$i]));
					$mailAliasContainer->addElement(new htmlButton('delAlias' . $i, 'del.png', true));
					$mailAliasContainer->addElement(new htmlHelpLink('alias'), true);
				}
			}
			// input box for new mail alias
			$mailAliasContainer->addElement(new htmlInputField('alias'));
			$mailAliasContainer->addElement(new htmlButton('addAlias', 'add.png', true));
			$mailAliasContainer->addElement(new htmlHelpLink('alias'), true);
			$return->addElement($mailAliasContainer, true);
			// delegates
			$delegates = searchLDAPByAttribute('mail', '*', 'inetOrgPerson', array('mail'), array('user'));
			for ($i = 0; $i < sizeof($delegates); $i++) {
				$delegates[$i] = $delegates[$i]['mail'][0];
			}
			sort($delegates);
			$return->addElement(new htmlSubTitle(_('Delegates')), true);
			$delegatesContainer = new htmlTable();
			if (isset($this->attributes['kolabDelegate'])) {
				for ($i = 0; $i < sizeof($this->attributes['kolabDelegate']); $i++) {
					$delegatesContainer->addElement(new htmlSelect('delegate' . $i, $delegates, array($this->attributes['kolabDelegate'][$i])));
					$delegatesContainer->addElement(new htmlButton('delDelegate' . $i, 'del.png', true));
					$delegatesContainer->addElement(new htmlHelpLink('delegate'), true);
				}
			}
			// input box for new delegate
			$delegatesContainer->addElement(new htmlSelect('delegate', $delegates));
			$delegatesContainer->addElement(new htmlButton('addDelegate', 'add.png', true));
			$delegatesContainer->addElement(new htmlHelpLink('delegate'), true);
			$return->addElement($delegatesContainer, true);
			// delete flag
			if (!$this->getAccountContainer()->isNewAccount) {
				$deleteContainer = new htmlTable();
				$deleteContainer->addElement(new htmlSpacer(null, '20px'), true);
				$deleteContainer->addElement(new htmlAccountPageButton(get_class($this), 'deleteUser', 'open', _('Mark account for deletion')));
				$deleteContainer->addElement(new htmlHelpLink('deleteFlag'));
				$return->addElement($deleteContainer);
			}
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add Kolab extension')));
		}
		return $return;
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
			$this->attributes['objectClass'][] = 'kolabInetOrgPerson';
		}
		else {
			if (isset($_POST['form_subpage_kolabUser_deleteUser_open']))  return array();
			$this->attributes['kolabInvitationPolicy'] = array();
			// mailbox server
			if (isset($_POST['mailHost'])) {
				if ($_POST['mailHost'] == "") {
					$errors[] = $this->messages['mailHost'][2];
				}
				elseif (get_preg($_POST['mailHost'], 'DNSname')) {
					$this->attributes['mailHost'][0] = $_POST['mailHost'];
				}
				else {
					$message = $this->messages['mailHost'][0];
					$message[] = $_POST['mailHost'];
					$errors[] = $message;
				}
			}
			// check old invitation policies
			$policies = array_flip($this->invitationPolicies);
			$targets = array();
			$i = 0;
			while (isset($_POST['invPol1' . $i])) {
				if (isset($_POST['delInvPol' . $i])) {
					$i++;
					continue;
				}
				if (isset($_POST['invPol2' . $i]) && ($_POST['invPol1' . $i] != "") && !in_array($_POST['invPol1' . $i], $targets)) {
					$targets[] = $_POST['invPol1' . $i];
					// check invitation policy
					if (!get_preg($_POST['invPol1' . $i], 'email')) {
						$message = $this->messages['invPol'][0];
						$message[] = $_POST['invPol1' . $i];
						$errors[] = $message;
					}
					else {
						$this->attributes['kolabInvitationPolicy'][] = $_POST['invPol1' . $i] . ':' . $policies[$_POST['invPol2' . $i]];
					}
				}
				$i++;
			}
			// check new invitation policy
			if (isset($_POST['invPol1']) && ($_POST['invPol1'] != "") && !in_array($_POST['invPol1'], $targets)) {
				// check new invitation policy
				if (!get_preg($_POST['invPol1'], 'email')) {
					$message = $this->messages['invPol'][0];
					$message[] = $_POST['invPol1'];
					$errors[] = $message;
				}
				else {
					$this->attributes['kolabInvitationPolicy'][] = $_POST['invPol1'] . ':' . $policies[$_POST['invPol2']];
				}
			}
			// default invitation policy
			if (isset($_POST['defaultInvPol']) && ($_POST['defaultInvPol'] != "")) {
				$this->attributes['kolabInvitationPolicy'][] = $policies[$_POST['defaultInvPol']];
			}
			// check old mail aliases
			$this->attributes['alias'] = array();
			$i = 0;
			while (isset($_POST['alias' . $i])) {
				if (isset($_POST['delAlias' . $i])) {
					$i++;
					continue;
				}
				if (isset($_POST['alias' . $i]) && ($_POST['alias' . $i] != "") && !in_array($_POST['alias' . $i], $this->attributes['alias'])) {
					// check mail alias
					if (!get_preg($_POST['alias' . $i], 'email')) {
						$message = $this->messages['alias'][0];
						$message[] = $_POST['alias' . $i];
						$errors[] = $message;
					}
					$this->attributes['alias'][] = $_POST['alias' . $i];
				}
				$i++;
			}
			// check new mail alias
			if (isset($_POST['alias']) && ($_POST['alias'] != "")) {
				// check new mail alias
				if (!get_preg($_POST['alias'], 'email')) {
					$message = $this->messages['alias'][0];
					$message[] = $_POST['alias'];
					$errors[] = $message;
				}
				else {
					$this->attributes['alias'][] = $_POST['alias'];
				}
			}
			$this->attributes['alias'] = array_unique($this->attributes['alias']);
			// check old delegates
			$this->attributes['kolabDelegate'] = array();
			$i = 0;
			while (isset($_POST['delegate' . $i])) {
				if (isset($_POST['delDelegate' . $i])) {
					$i++;
					continue;
				}
				$this->attributes['kolabDelegate'][] = $_POST['delegate' . $i];
				$i++;
			}
			// check new delegate
			if (isset($_POST['addDelegate']) && ($_POST['delegate'] != "")) {
				$this->attributes['kolabDelegate'][] = $_POST['delegate'];
			}
			$this->attributes['kolabDelegate'] = array_unique($this->attributes['kolabDelegate']);
		}
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page to mark an account for deletion.
	* 
	* @return htmlElement HTML meta data
	*/
	function display_html_deleteUser() {
		$return = new htmlTable();
		$message = new htmlOutputText(_('Do you really want to mark this account for deletion?'));
		$message->colspan = 2;
		$return->addElement($message, true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$return->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'confirm', _('Mark account for deletion')));
		$return->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'cancel', _('Cancel')));
		return $return;
	}

	/**
	* Write variables into object and do some regex checks
	*/
	function process_deleteUser() {
		if (isset($_POST['form_subpage_kolabUser_attributes_confirm'])) {
			// set delete flag
			$this->attributes['kolabDeleteflag'][0] = $this->attributes['mailHost'][0];
		}
	}

	/**
	* This function returns true if all needed settings are done.
	*
	* @return true, if account can be saved
	*/
	function module_complete() {
		if (isset($this->attributes['objectClass']) && in_array('kolabInetOrgPerson', $this->attributes['objectClass'])) {
			$attrsI = $this->getAccountContainer()->getAccountModule('inetOrgPerson')->getAttributes();
			if (!$attrsI['mail'][0]) return false;
			if ($this->getAccountContainer()->isNewAccount) {
				if ($this->getAccountContainer()->getAccountModule('posixAccount') != null) {
					$attrsP = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
					if (!$attrsP['userPassword'][0]) return false;
				}
				elseif (!$attrsI['userPassword'][0]) {
					return false;
				}
			}
		}
		return true;
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
		if (!in_array('kolabInetOrgPerson', $this->attributes['objectClass']) && !in_array('kolabInetOrgPerson', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
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
		$delegates = searchLDAPByAttribute(null, null, 'inetOrgPerson', array('mail'), array('user'));
		for ($d = 0; $d < sizeof($delegates); $d++) {
			if (isset($delegates[$d]['mail'][0])) {
				$delegates[$d] = $delegates[$d]['mail'][0];
			}
		}
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("kolabInetOrgPerson", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "kolabInetOrgPerson";
			// mailbox server
			if ($rawAccounts[$i][$ids['kolabUser_mailHost']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_mailHost']], 'DNSname')) {
					$partialAccounts[$i]['mailHost'] = $rawAccounts[$i][$ids['kolabUser_mailHost']];
				}
				else {
					$errMsg = $this->messages['mailHost'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// add invitation policies
			if ($rawAccounts[$i][$ids['kolabUser_invPol']] != "") {
				$pols = explode(',', $rawAccounts[$i][$ids['kolabUser_invPol']]);
				// check format
				$policies = array_keys($this->invitationPolicies);
				$defaultFound = false;
				for ($p = 0; $p < sizeof($pols); $p++) {
					$parts = explode(":", $pols[$p]);
					// default policy
					if (sizeof($parts) == 1) {
						if (!$defaultFound && get_preg($parts[0], 'email')) {
							$partialAccounts[$i]['kolabInvitationPolicy'][] = $parts[0];
						}
						else {
							$errMsg = $this->messages['invPol'][1];
							array_push($errMsg, array($i));
							$messages[] = $errMsg;
						}
						$defaultFound = true;
					}
					// additional policies
					elseif (sizeof($parts) == 2) {
						if (get_preg($parts[0], 'email') && in_array($parts[1], $policies)) {
							$partialAccounts[$i]['kolabInvitationPolicy'][] = $pols[$p];
						}
						else {
							$errMsg = $this->messages['invPol'][1];
							array_push($errMsg, array($i));
							$messages[] = $errMsg;
						}
					}
					// invalid format
					else {
						$errMsg = $this->messages['invPol'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// add mail aliases
			if ($rawAccounts[$i][$ids['kolabUser_aliases']] != "") {
				$aliases = explode(',', $rawAccounts[$i][$ids['kolabUser_aliases']]);
				// check format
				for ($a = 0; $a < sizeof($aliases); $a++) {
					if (get_preg($aliases[$a], 'email')) {
						$partialAccounts[$i]['alias'][] = $aliases[$a];
					}
					// invalid format
					else {
						$errMsg = $this->messages['alias'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// add delegates
			if ($rawAccounts[$i][$ids['kolabUser_delegates']] != "") {
				$newDelegates = explode(',', $rawAccounts[$i][$ids['kolabUser_delegates']]);
				// check format
				for ($d = 0; $d < sizeof($newDelegates); $d++) {
					if (in_array($newDelegates[$d], $delegates)) {
						$partialAccounts[$i]['kolabDelegate'][] = $newDelegates[$d];
					}
					// invalid format
					else {
						$errMsg = $this->messages['delegate'][0];
						array_push($errMsg, array($i, $newDelegates[$d]));
						$messages[] = $errMsg;
					}
				}
			}
		}
		return $messages;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'mailHost', _('Mailbox home server'));
		// invitation policies
		if (isset($this->attributes['kolabInvitationPolicy'][0])) {
			// find default policy
			$default = "";
			for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
				if (!strpos($this->attributes['kolabInvitationPolicy'][$i], ":")) {
					$default = $this->attributes['kolabInvitationPolicy'][$i];
					break;
				}
			}
			$return['kolabUser_invPol'][0] = '<block><key>' . _('Invitation policy') . '</key><tr><td align=\"L\">' . _('Anyone') . ": " . $this->invitationPolicies[$default] . '</td></tr></block>';
			for ($i = 0; $i < sizeof($this->attributes['kolabInvitationPolicy']); $i++) {
				$parts = explode(':', $this->attributes['kolabInvitationPolicy'][$i]);
				if (sizeof($parts) == 2) {
					$return['kolabUser_invPol'][] = '<block><tr><td align=\"L\">' . $parts[0] . ": " . $this->invitationPolicies[$parts[1]] . '</td></tr></block>';
				}
			}
		}
		$this->addSimplePDFField($return, 'aliases', _('Email aliases'), 'alias');
		$this->addSimplePDFField($return, 'delegate', _('Delegates'), 'kolabDelegate');
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
	 * @return array list of meta HTML elements (field name => htmlTableRow)
	 */
	function getSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		if ($passwordChangeOnly) {
			return array(); // no Kolab fields as long no LDAP content can be read
		}
		if (!in_array('kolabInetOrgPerson', $attributes['objectClass'])) {
			return array();
		}
		$return = array();
		// delegates
		if (in_array('kolabDelegate', $fields)) {
			$delegates = array();
			$sr = @ldap_search($_SESSION['ldapHandle'], escapeDN($this->selfServiceSettings->LDAPSuffix), '(&(objectClass=inetOrgPerson)(mail=*))', array('mail'), 0, 0, 0, LDAP_DEREF_NEVER);
			if ($sr) {
				$result = ldap_get_entries($_SESSION['ldapHandle'], $sr);
				cleanLDAPResult($result);
				for ($i = 0; $i < sizeof($result); $i++) {
					$delegates[] = $result[$i]['mail'][0];
				}
			}
			$delegates = array_unique($delegates);
			sort($delegates);
			$kolabDelegate = array();
			if (isset($attributes['kolabDelegate'])) {
				$kolabDelegate = $attributes['kolabDelegate'];
				// do not show existing delegation candidates in selection list
				for ($i = 0; $i < sizeof($kolabDelegate); $i++) {
					$key = array_search($kolabDelegate[$i], $delegates);
					if ($key !== false) {
						unset($delegates[$key]);
					}
				}
			}
			$delegates = array_values($delegates);
			sort($kolabDelegate);
			$_SESSION['kolabUser_kolabDelegate'] = $kolabDelegate;
			$delegateFields = array();
			$delegateContainer = new htmlTable();
			for ($i = 0; $i < sizeof($kolabDelegate); $i++) {
				$delegateContainer->addElement(new htmlOutputText($kolabDelegate[$i]));
				if (!in_array('kolabDelegate', $readOnlyFields)) {
					$delegateContainer->addElement(new htmlTableExtendedInputCheckbox('delDelegate_' . $i, false, _('Delete'), null, false), true);
				}
				else {
					$delegateContainer->addNewLine();
				}
			}
			if (!in_array('kolabDelegate', $readOnlyFields)) {
				$delegateContainer->addElement(new htmlSelect('new_delegate_value', $delegates));
				$delegateContainer->addElement(new htmlTableExtendedInputCheckbox('new_delegate', false, _("Add"), null, false), true);
			}
			$delegateLabel = new htmlOutputText(_('Delegates'));
			$delegateLabel->alignment = htmlElement::ALIGN_TOP;
			$return['kolabDelegate'] = new htmlTableRow(array(
				$delegateLabel, $delegateContainer
			));
		}
		// invitation policies
		if (in_array('kolabInvitationPolicy', $fields)) {
			$invitationContainer = new htmlTable();
			// default invitation policy
			$defaultInvPol = $this->invitationPolicies['ACT_MANUAL'];
			for ($i = 0; $i < sizeof($attributes['kolabInvitationPolicy']); $i++) {
				$parts = explode(":", $attributes['kolabInvitationPolicy'][$i]);
				if (sizeof($parts) == 1) {
					$defaultInvPol = $this->invitationPolicies[$attributes['kolabInvitationPolicy'][$i]];
					unset($attributes['kolabInvitationPolicy'][$i]);
					$attributes['kolabInvitationPolicy'] = array_values($attributes['kolabInvitationPolicy']);
					break;
				}
			}
			if (!in_array('kolabDelegate', $readOnlyFields)) {
				$invitationContainer->addElement(new htmlTableExtendedSelect('defaultInvPol', array_values($this->invitationPolicies), array($defaultInvPol), _('Anyone')), true);
			}
			else {
				$invitationContainer->addElement(new htmlOutputText(_('Anyone')));
				$invitationContainer->addElement(new htmlOutputText($defaultInvPol), true);
			}
			// other invitation policies
			for ($i = 0; $i < sizeof($attributes['kolabInvitationPolicy']); $i++) {
				$parts = explode(":", $attributes['kolabInvitationPolicy'][$i]);
				if (sizeof($parts) == 2) {
					if (!in_array('kolabDelegate', $readOnlyFields)) {
						$invitationContainer->addElement(new htmlInputField('invPol1' . $i, $parts[0]));
						$invitationContainer->addElement(new htmlSelect('invPol2' . $i, array_values($this->invitationPolicies), array($this->invitationPolicies[$parts[1]])));
						$invitationContainer->addElement(new htmlTableExtendedInputCheckbox('delInvPol' . $i, false, _("Remove"), null, false), true);
					}
					else {
						$invitationContainer->addElement(new htmlOutputText($parts[0]));
						$invitationContainer->addElement(new htmlOutputText($this->invitationPolicies[$parts[1]]), true);
					}
				}
			}
			// input box for new invitation policy
			if (!in_array('kolabDelegate', $readOnlyFields)) {
				$invitationContainer->addElement(new htmlInputField('invPol1', ''));
				$invitationContainer->addElement(new htmlSelect('invPol2', array_values($this->invitationPolicies)));
				$invitationContainer->addElement(new htmlTableExtendedInputCheckbox('addInvPol', false, _("Add"), null, false), true);
			}
			$invitationLabel = new htmlOutputText(_('Invitation policy'));
			$invitationLabel->alignment = htmlElement::ALIGN_TOP;
			$return['kolabInvitationPolicy'] = new htmlTableRow(array(
				$invitationLabel, $invitationContainer
			));
		}
		return $return;
	}

	/**
	 * Checks if all input values are correct and returns the LDAP attributes which should be changed.
	 * <br>Return values:
	 * <br>messages: array of parameters to create status messages
	 * <br>add: array of attributes to add
	 * <br>del: array of attributes to remove
	 * <br>mod: array of attributes to modify
	 * <br>info: array of values with informational value (e.g. to be used later by pre/postModify actions)
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @param boolean $passwordChangeOnly indicates that the user is only allowed to change his password and no LDAP content is readable
	 * @param array $readOnlyFields list of read-only fields
	 * @return array messages and attributes (array('messages' => array(), 'add' => array('mail' => array('test@test.com')), 'del' => array(), 'mod' => array(), 'info' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes, $passwordChangeOnly, $readOnlyFields) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array(), 'info' => array());
		if ($passwordChangeOnly) {
			return $return; // skip processing if only a password change is done
		}
		if (!in_array_ignore_case('kolabInetOrgPerson', $attributes['objectClass'])) {
			return $return;
		}
		$attributeNames = array(); // list of attributes which should be checked for modification
		$attributesNew = $attributes;
		// delegates
		if (in_array('kolabDelegate', $fields) && !in_array('kolabDelegate', $readOnlyFields)) {
			$attributeNames[] = 'kolabDelegate';
			// new delegation
			if (isset($_POST['new_delegate']) && ($_POST['new_delegate'] == 'on')) {
				$attributesNew['kolabDelegate'][] = $_POST['new_delegate_value'];
			}
			// check for deleted delegations
			$postKeys = array_keys($_POST);
			for ($i = 0; $i < sizeof($postKeys); $i++) {
				if (strpos($postKeys[$i], 'delDelegate_') !== false) {
					$sKey = substr($postKeys[$i], strlen('delDelegate_'));
					$key = array_search($_SESSION['kolabUser_kolabDelegate'][$sKey], $attributesNew['kolabDelegate']);
					if ($key !== false) {
						unset($attributesNew['kolabDelegate'][$key]);
						$attributesNew['kolabDelegate'] = array_values($attributesNew['kolabDelegate']);
					}
				}
			}
		}
		// invitation policies
		if (in_array('kolabInvitationPolicy', $fields) && !in_array('kolabInvitationPolicy', $readOnlyFields)) {
			$attributeNames[] = 'kolabInvitationPolicy';
			$policies = array_flip($this->invitationPolicies);
			$attributesNew['kolabInvitationPolicy'] = array();
			// check old invitation policies
			$targets = array();
			$i = 0;
			while (isset($_POST['invPol1' . $i])) {
				if (isset($_POST['delInvPol' . $i])) {
					$i++;
					continue;
				}
				if (isset($_POST['invPol2' . $i]) && ($_POST['invPol1' . $i] != "") && !in_array($_POST['invPol1' . $i], $targets)) {
					$targets[] = $_POST['invPol1' . $i];
					// check invitation policy
					if (!get_preg($_POST['invPol1' . $i], 'email')) {
						$message = $this->messages['invPol'][0];
						$message[] = $_POST['invPol1' . $i];
						$errors[] = $message;
					}
					else {
						$attributesNew['kolabInvitationPolicy'][] = $_POST['invPol1' . $i] . ':' . $policies[$_POST['invPol2' . $i]];
					}
				}
				$i++;
			}
			// check new invitation policy
			if (isset($_POST['invPol1']) && ($_POST['invPol1'] != "") && !in_array($_POST['invPol1'], $targets)) {
				// check new invitation policy
				if (!get_preg($_POST['invPol1'], 'email')) {
					$message = $this->messages['invPol'][0];
					$message[] = $_POST['invPol1'];
					$errors[] = $message;
				}
				else {
					$attributesNew['kolabInvitationPolicy'][] = $_POST['invPol1'] . ':' . $policies[$_POST['invPol2']];
				}
			}
			// default invitation policy
			if (isset($_POST['defaultInvPol']) && ($_POST['defaultInvPol'] != "")) {
				$attributesNew['kolabInvitationPolicy'][] = $policies[$_POST['defaultInvPol']];
			}
		}
		// find differences
		for ($i = 0; $i < sizeof($attributeNames); $i++) {
			$attrName = $attributeNames[$i];
			if (isset($attributes[$attrName]) && !isset($attributesNew[$attrName])) {
				$return['del'][$attrName] = $attributes[$attrName];
			}
			elseif (!isset($attributes[$attrName]) && isset($attributesNew[$attrName])) {
				$return['add'][$attrName] = $attributesNew[$attrName];
			}
			else {
				if (isset($attributes[$attrName])) {
					for ($a = 0; $a < sizeof($attributes[$attrName]); $a++) {
						if (!in_array($attributes[$attrName][$a], $attributesNew[$attrName])) {
							$return['mod'][$attrName] = $attributesNew[$attrName];
							break;
						}
					}
				}
				if (isset($attributesNew[$attrName])) {
					for ($a = 0; $a < sizeof($attributesNew[$attrName]); $a++) {
						if (!in_array($attributesNew[$attrName][$a], $attributes[$attrName])) {
							$return['mod'][$attrName] = $attributesNew[$attrName];
							break;
						}
					}
				}
			}
		}
		return $return;
	}

}


?>
