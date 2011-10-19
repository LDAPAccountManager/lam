<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2011  Roland Gruber

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
		$return['attributes'] = array('c', 'alias', 'kolabHomeServer', 'kolabHomeMTA', 'kolabDelegate',
			'cyrus-userquota', 'kolabInvitationPolicy', 'kolabFreeBusyFuture', 'kolabDeleteflag');
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Mail server'), 'kolab_homeServer', null, 'homeServer'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Country'), 'kolab_country', null, 'country'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Free/Busy interval'), 'kolab_freeBusy', null, 'freeBusy'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Mail quota'), 'kolab_quota', null, 'quota'), true);
		$return['profile_options'] = $profileContainer;
		// profile checks
		$return['profile_checks']['kolab_homeServer'] = array(
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['homeServer'][0]);
		$return['profile_checks']['kolab_country'] = array(
			'type' => 'ext_preg',
			'regex' => 'country',
			'error_message' => $this->messages['country'][0]);
		$return['profile_checks']['kolab_freeBusy'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['freeBusy'][0]);
		$return['profile_checks']['kolab_quota'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['quota'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'kolab_homeServer' => 'kolabHomeServer',
			'kolab_country' => 'c',
			'kolab_freeBusy' => 'kolabFreeBusyFuture',
			'kolab_quota' => 'cyrus-userquota'
		);
		// self service field settings
		$return['selfServiceFieldSettings'] = array(
			'kolabFreeBusyFuture' => _('Free/Busy interval'),
			'kolabDelegate' => _('Delegates'),
			'kolabInvitationPolicy' => _('Invitation policy')
		);
		// help Entries
		$return['help'] = array(
			'invPol' => array(
				"Headline" => _("Invitation policy"),
				"Text" => _("For automatic invitation handling.")
			),
			'invPolList' => array(
				"Headline" => _("Invitation policy list"),
				"Text" => _("This is a comma separated list of invitation policies.")
			),
			'delegate' => array(
				"Headline" => _("Delegates"),
				"Text" => _("A user may define who is allowed to act on behalf of herself. This property is checked when using the Kolab smtp daemon (Postfix) to send emails.")
			),
			'delegateList' => array(
				"Headline" => _("Delegates"),
				"Text" => _("This is a comma separated list of delegates.")
			),
			'alias' => array(
				"Headline" => _("Email alias"),
				"Text" => _("Email alias for this account.")
			),
			'aliasList' => array(
				"Headline" => _("Email alias list"),
				"Text" => _("This is a comma separated list of eMail aliases.")
			),
			'country' => array(
				"Headline" => _("Country"),
				"Text" => _("The country name of the user.")
			),
			'homeServer' => array(
				"Headline" => _("Mailbox home server"),
				"Text" => _("The name of the server where the mailbox is located.")
			),
			'freeBusy' => array(
				"Headline" => _("Free/Busy interval"),
				"Text" => _("This is the time limit (in days) for other users who want to check future appointments. If you leave this blank the default (60 days) will be set.")
			),
			'quota' => array(
				"Headline" => _("Mail quota"),
				"Text" => _("The Cyrus mail quota for users in MBytes, leave blank for unlimited space.")
			),
			'deleteFlag' => array(
				"Headline" => _("Mark for deletion"),
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
				'name' => 'kolabUser_country',
				'description' => _('Country'),
				'help' => 'country',
				'example' => _('Germany')
			),
			array(
				'name' => 'kolabUser_homeServer',
				'description' => _('Mailbox home server'),
				'help' => 'homeServer',
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
			array(
				'name' => 'kolabUser_freeBusy',
				'description' => _('Free/Busy interval'),
				'help' => 'freeBusy',
				'example' => '60'
			),
			array(
				'name' => 'kolabUser_quota',
				'description' => _('Mail quota'),
				'help' => 'quota',
				'example' => '300'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'invPol' => _('Invitation policy'),
			'country' => _('Country'),
			'homeServer' => _('Mailbox home server'),
			'aliases' => _('Email aliases'),
			'delegate' => _('Delegates'),
			'freeBusy' => _('Free/Busy interval'),
			'quota' => _('Mail quota')
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
		$this->messages['country'][0] = array('ERROR', _('Country name is invalid!'));  // third array value is set dynamically
		$this->messages['country'][1] = array('ERROR', _('Account %s:') . ' kolabUser_country', _('Country name is invalid!'));
		$this->messages['homeServer'][0] = array('ERROR', _('Mailbox home server name is invalid!'));  // third array value is set dynamically
		$this->messages['homeServer'][1] = array('ERROR', _('Account %s:') . ' kolabUser_homeServer', _('Mailbox home server name is invalid!'));
		$this->messages['homeServer'][2] = array('ERROR', _('Mailbox home server name is empty!'), '');
		$this->messages['delegate'][0] = array('ERROR', _('Account %s:') . ' kolabUser_delegate', _('Unknown delegate address: %s'));
		$this->messages['freeBusy'][0] = array('ERROR', _('Free/Busy interval must be a number!'));  // third array value is set dynamically
		$this->messages['freeBusy'][1] = array('ERROR', _('Account %s:') . ' kolabUser_freeBusy', _('Free/Busy interval must be a number!'));
		$this->messages['quota'][0] = array('ERROR', _('Mail quota must be a number!'));  // third array value is set dynamically
		$this->messages['quota'][1] = array('ERROR', _('Account %s:') . ' kolabUser_quota', _('Mail quota must be a number!'));
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
			if (!$attrsI['mail'][0]) {
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
			// country
			$country = '';
			if (isset($this->attributes['c'][0])) {
				$country = $this->attributes['c'][0];
			}
			$basicPartContainer->addElement(new htmlTableExtendedInputField(_('Country'), 'country', $country, 'country'), true);
			// mailbox server
			if (!isset($this->orig['kolabHomeServer'][0])) {  // value currently not set
				$homeServer = '';
				if (isset($this->attributes['kolabHomeServer'][0])) {
					$homeServer = $this->attributes['kolabHomeServer'][0];
				}
				$serverInput = new htmlTableExtendedInputField(_('Mailbox home server'), 'homeServer', $homeServer, 'homeServer');
				$serverInput->setRequired(true);
				$basicPartContainer->addElement($serverInput, true);
			}
			else {  // input is unchangable when set
				$basicPartContainer->addElement(new htmlOutputText(_('Mailbox home server')));
				$basicPartContainer->addElement(new htmlOutputText($this->orig['kolabHomeServer'][0]));
				$basicPartContainer->addElement(new htmlHelpLink('homeServer'), true);
			}
			// Cyrus mail quota
			$userquota = '';
			if (isset($this->attributes['cyrus-userquota'][0])) {
				$userquota = $this->attributes['cyrus-userquota'][0];
			}
			$userquotaInput = new htmlTableExtendedInputField(_('Mail quota'), 'quota', $userquota, 'quota');
			$userquotaInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$basicPartContainer->addElement($userquotaInput, true);
			// free/busy future
			$freebusyfuture = '';
			if (isset($this->attributes['kolabFreeBusyFuture'][0])) {
				$freebusyfuture = $this->attributes['kolabFreeBusyFuture'][0];
			}
			$freebusyfutureInput = new htmlTableExtendedInputField(_('Free/Busy interval'), 'freeBusy', $freebusyfuture, 'freeBusy');
			$freebusyfutureInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$basicPartContainer->addElement($freebusyfutureInput, true);
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
			// country
			if (isset($_POST['country'])) {
				if (($_POST['country'] == "") || get_preg($_POST['country'], 'country')) {
					$this->attributes['c'][0] = $_POST['country'];
				}
				else {
					$message = $this->messages['country'][0];
					$message[] = $_POST['country'];
					$errors[] = $message;
				}
			}
			// mailbox server
			if (isset($_POST['homeServer'])) {
				if ($_POST['homeServer'] == "") {
					$errors[] = $this->messages['homeServer'][2];
				}
				elseif (get_preg($_POST['homeServer'], 'DNSname')) {
					$this->attributes['kolabHomeServer'][0] = $_POST['homeServer'];
				}
				else {
					$message = $this->messages['homeServer'][0];
					$message[] = $_POST['homeServer'];
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
			// free/busy future
			if (isset($_POST['freeBusy'])) {
				if (($_POST['freeBusy'] == "") || get_preg($_POST['freeBusy'], 'digit')) {
					$this->attributes['kolabFreeBusyFuture'][0] = $_POST['freeBusy'];
				}
				else {
					$message = $this->messages['freeBusy'][0];
					$message[] = $_POST['freeBusy'];
					$errors[] = $message;
				}
			}
			// Cyrus mail quota
			if (isset($_POST['quota'])) {
				if (($_POST['quota'] == "") || get_preg($_POST['quota'], 'digit')) {
					$this->attributes['cyrus-userquota'][0] = $_POST['quota'];
				}
				else {
					$message = $this->messages['quota'][0];
					$message[] = $_POST['quota'];
					$errors[] = $message;
				}
			}
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
			$this->attributes['kolabDeleteflag'][0] = $this->attributes['kolabHomeServer'][0];
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
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
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
			// country
			if ($rawAccounts[$i][$ids['kolabUser_country']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_country']], 'country')) {
					$partialAccounts[$i]['c'] = $rawAccounts[$i][$ids['kolabUser_country']];
				}
				else {
					$errMsg = $this->messages['country'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// mailbox server
			if ($rawAccounts[$i][$ids['kolabUser_homeServer']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_homeServer']], 'DNSname')) {
					$partialAccounts[$i]['kolabHomeServer'] = $rawAccounts[$i][$ids['kolabUser_homeServer']];
				}
				else {
					$errMsg = $this->messages['homeServer'][1];
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
			// free/busy
			if ($rawAccounts[$i][$ids['kolabUser_freeBusy']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_freeBusy']], 'digit')) {
					$partialAccounts[$i]['kolabFreeBusyFuture'] = $rawAccounts[$i][$ids['kolabUser_freeBusy']];
				}
				else {
					$errMsg = $this->messages['freeBusy'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// Cyrus mail quota
			if ($rawAccounts[$i][$ids['kolabUser_quota']] != "") {
				if (get_preg($rawAccounts[$i][$ids['kolabUser_quota']], 'digit')) {
					$partialAccounts[$i]['cyrus-userquota'] = $rawAccounts[$i][$ids['kolabUser_quota']];
				}
				else {
					$errMsg = $this->messages['quota'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
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
		// country
		if (isset($this->attributes['c'][0])) {
			$return['kolabUser_country'][0] = '<block><key>' . _('Country') . '</key><value>' . $this->attributes['c'][0] . '</value></block>';
		}
		// mail server
		if (isset($this->attributes['kolabHomeServer'][0])) {
			$return['kolabUser_homeServer'][0] = '<block><key>' . _('Mailbox home server') . '</key><value>' . $this->attributes['kolabHomeServer'][0] . '</value></block>';
		}
		// mail quota
		if (isset($this->attributes['cyrus-userquota'][0])) {
			$return['kolabUser_quota'][0] = '<block><key>' . _('Mail quota') . '</key><value>' . $this->attributes['cyrus-userquota'][0] . '</value></block>';
		}
		// free/busy
		if (isset($this->attributes['kolabFreeBusyFuture'][0])) {
			$return['kolabUser_freeBusy'][0] = '<block><key>' . _('Free/Busy interval') . '</key><value>' . $this->attributes['kolabFreeBusyFuture'][0] . '</value></block>';
		}
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
		// email aliases
		if (isset($this->attributes['alias'][0])) {
			$return['kolabUser_aliases'][0] = '<block><key>' . _('Email aliases') . '</key><value>' . implode(", ", $this->attributes['alias']) . '</value></block>';
		}
		// delegates
		if (isset($this->attributes['kolabDelegate'][0])) {
			$return['kolabUser_delegate'][0] = '<block><key>' . _('Delegates') . '</key><value>' . implode(", ", $this->attributes['kolabDelegate']) . '</value></block>';
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
	 * @return array meta HTML
	 */
	function getSelfServiceOptions($fields, $attributes) {
		if (!in_array('kolabInetOrgPerson', $attributes['objectClass'])) {
			return array();
		}
		$return = array();
		// free/busy future
		if (in_array('kolabFreeBusyFuture', $fields)) {
			$kolabFreeBusyFuture = '';
			if (isset($attributes['kolabFreeBusyFuture'][0])) {
				$kolabFreeBusyFuture = $attributes['kolabFreeBusyFuture'][0];
			}
			$return['kolabFreeBusyFuture'] = new htmlTableRow(array(
				new htmlTableExtendedInputField(_('Free/Busy interval'), 'kolabUser_kolabFreeBusyFuture', $kolabFreeBusyFuture)
			));
		}
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
				$delegateContainer->addElement(new htmlTableExtendedInputCheckbox('delDelegate_' . $i, false, $kolabDelegate[$i]));
				$delegateContainer->addElement(new htmlOutputText(_('Delete')), true);
			}
			$delegateContainer->addElement(new htmlSelect('new_delegate_value', $delegates));
			$delegateContainer->addElement(new htmlTableExtendedInputCheckbox('new_delegate', false, _("Add"), null, false), true);
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
			$invitationContainer->addElement(new htmlTableExtendedSelect('defaultInvPol', array_values($this->invitationPolicies), array($defaultInvPol), _('Anyone')), true);
			// other invitation policies
			for ($i = 0; $i < sizeof($attributes['kolabInvitationPolicy']); $i++) {
				$parts = explode(":", $attributes['kolabInvitationPolicy'][$i]);
				if (sizeof($parts) == 2) {
					$invitationContainer->addElement(new htmlInputField('invPol1' . $i, $parts[0]));
					$invitationContainer->addElement(new htmlSelect('invPol2' . $i, array_values($this->invitationPolicies), array($this->invitationPolicies[$parts[1]])));
					$invitationContainer->addElement(new htmlTableExtendedInputCheckbox('delInvPol' . $i, false, _("Remove"), null, false), true);
				}
			}
			// input box for new invitation policy
			$invitationContainer->addElement(new htmlInputField('invPol1', ''));
			$invitationContainer->addElement(new htmlSelect('invPol2', array_values($this->invitationPolicies)));
			$invitationContainer->addElement(new htmlTableExtendedInputCheckbox('addInvPol', false, _("Add"), null, false), true);
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
	 * @return array messages and attributes (array('messages' => array(), 'add' => array('mail' => array('test@test.com')), 'del' => array(), 'mod' => array(), 'info' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array(), 'info' => array());
		if (!in_array_ignore_case('kolabInetOrgPerson', $attributes['objectClass'])) {
			return $return;
		}
		$attributeNames = array(); // list of attributes which should be checked for modification
		$attributesNew = $attributes;
		// kolabFreeBusyFuture
		if (in_array('kolabFreeBusyFuture', $fields)) {
			$attributeNames[] = 'kolabFreeBusyFuture';
			if (isset($_POST['kolabUser_kolabFreeBusyFuture']) && ($_POST['kolabUser_kolabFreeBusyFuture'] != '')) {
				if (!get_preg($_POST['kolabUser_kolabFreeBusyFuture'], 'digit')) $return['messages'][] = $this->messages['freeBusy'][0];
				else $attributesNew['kolabFreeBusyFuture'][0] = $_POST['kolabUser_kolabFreeBusyFuture'];
			}
			elseif (isset($attributes['kolabFreeBusyFuture'])) {
				$attributesNew['kolabFreeBusyFuture'] = array();
			}
		}
		// delegates
		if (in_array('kolabDelegate', $fields)) {
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
		if (in_array('kolabInvitationPolicy', $fields)) {
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
