<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009 - 2011  Pavel Pozdnyak
                2009 - 2011  Roland Gruber

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
* Manages the Asterisk extension of user accounts.
*
* @package modules
*
* @author Pavel Pozdnyak
* @author Roland Gruber
*/

/**
 * Manages the Asterisk extension of user accounts.
 *
 * @package modules
 */
class asteriskAccount extends baseModule implements passwordService {

	const ASTERISK_DEFAULT_REALM = "asterisk";

	/**
	* Creates a new asteriskAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	 * Returns meta data that is interpreted by parent class.
	 *
	 * @return array array with meta data
	 */
	function get_metaData() {
		$return = array();	
		// manages users accounts
		$return["account_types"] = array("user");
		$return["is_base"] = false;
		// alias name
		$return["alias"] = _("Asterisk");
		// module dependencies
		$return['dependencies'] = array('depends' => array('inetOrgPerson'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('AsteriskSIPUser');
		// managed attributes
		$return['attributes'] = array('AstAccountCallerID', 'AstAccountHost',
			'AstAccountRealmedPassword', 'AstAccountContext', 'AstAccountType',
			'AstAccountUserAgent', 'AstAccountAMAFlags', 'AstAccountCallGroup',
			'AstAccountDTMFMode', 'AstAccountFromUser', 'AstAccountFromDomain',
			'AstAccountFullContact', 'AstAccountInsecure', 'AstAccountMailbox',
			'AstAccountNAT', 'AstAccountDeny', 'AstAccountPermit',
			'AstAccountPickupGroup', 'AstAccountPort', 'AstAccountQualify',
			'AstAccountRestrictCID', 'AstAccountRTPTimeout', 'AstAccountRTPHoldTimeout',
			'AstAccountDisallowedCodec', 'AstAccountAllowedCodec', 'AstAccountMusicOnHold',
			'AstAccountExpirationTimestamp', 'AstAccountRegistrationContext',
			'AstAccountRegistrationExten', 'AstAccountCanCallForward', 'AstAccountIPAddress',
			'AstAccountDefaultUser', 'AstAccountRegistrationServer', 'AstAccountLastQualifyMilliseconds',
		);
		// icon
		$return['icon'] = 'asterisk.png';
		// self service
		$return['selfServiceFieldSettings'] = array(
			'syncAsteriskPassword' => _('Sync Asterisk password with Unix password'),
		);
		// help
		$return['help'] = array(
			'AstAccountCallerID' => array(
				"Headline" => _("Caller ID"),
				"Text" => _("This is the ID of the user in the Asterisk database. It may contain digits and letters (e.g. user1 or 200134).")
			),
			'AstAccountHost' => array(
				"Headline" => _("Host"),
				"Text" => _("This is the machine id (e.g. IP address or host name) from which the user can call/receive calls.")
			),
			'AstAccountContext' => array(
				"Headline" => _("Account context"),
				"Text" => _("The account context stores information about the dial plan.")
			),
			'AstAccountRealmedPassword' => array(
				"Headline" => _("Password"),
				"Text" => _("Please enter the password which you want to set for this account.")
			),
			'AstAccountType' => array(
				"Headline" => _("Account type"),
				"Text" => _("Please enter the account's type (e.g. \"friend\").")
			),
			'AsteriskRealm' => array(
				"Headline" => _("Asterisk realm"),
				"Text" => _("Authentication realm for Asterisk server (default: asterisk). This value set in sip.conf (option: \"realm\").")
			),
			'AstAccountUserAgent' => array(
				"Headline" => _("User agent"),
				"Text" => _("SIP user agent identification.")
			),
			'AstAccountAMAFlags' => array(
				"Headline" => _("AMA flags"),
				"Text" => _("Asterisk AMA (Automated Message Accounting) flags.")
			),
			'AstAccountCallGroup' => array(
				"Headline" => _("Call groups"),
				"Text" => _("The user's call groups.")
			),                    
			'AstAccountDTMFMode' => array(
				"Headline" => _("DTMF mode"),
				"Text" => _("DTMF mode for SIP client configuration.")
			),
			'AstAccountFromUser' => array(
				"Headline" => _("From user"),
				"Text" => _("From user setting for this account.")
			),
			'AstAccountFromDomain' => array(
				"Headline" => _("From domain"),
				"Text" => _("From domain setting for this account.")
			),
			'AstAccountFullContact' => array(
				"Headline" => _("Full contact"),
				"Text" => _("SIP URI for a realtime peer.")
			),
			'AstAccountInsecure' => array(
				"Headline" => _("Insecure"),
				"Text" => _("This is typically used to allow incoming calls (e.g. from FWD) while having a type=friend entry defined with username and password.")
			),
			'AstAccountMailbox' => array(
				"Headline" => _("Mailbox"),
				"Text" => _("Defines the mailbox to check for message waiting indication (MWI) for this peer.")
			),
			'AstAccountNAT' => array(
				"Headline" => _("NAT"),
				"Text" => _("NAT setting for this account.")
			),
			'AstAccountDeny' => array(
				"Headline" => _("Deny"),
				"Text" => _("Used to limit SIP traffic to and from this peer to a certain IP or network.")
			),
			'AstAccountPermit' => array(
				"Headline" => _("Permit"),
				"Text" => _("Used to limit SIP traffic to and from this peer to a certain IP or network.")
			),
			'AstAccountPickupGroup' => array(
				"Headline" => _("Pickup group"),
				"Text" => _("Specifies the user's pickup group.")
			),
			'AstAccountPort' => array(
				"Headline" => _("Port"),
				"Text" => _("Port number.")
			),
			'AstAccountQualify' => array(
				"Headline" => _("Qualify"),
				"Text" => _("Used to regularly check that a device is still online.")
			),
			'AstAccountRestrictCID' => array(
				"Headline" => _("Restrict caller ID"),
				"Text" => _("Use this to hide the caller ID.")
			),
			'AstAccountRTPTimeout' => array(
				"Headline" => _("RTP timeout"),
				"Text" => _("Used to automatically hangup the call if no RTP traffic is received.")
			),
			'AstAccountRTPHoldTimeout' => array(
				"Headline" => _("RTP hold timeout"),
				"Text" => _("Maximum number of seconds of inactivity before terminating a call on hold.")
			),
			'AstAccountDisallowedCodec' => array(
				"Headline" => _("Disallowed codec"),
				"Text" => _("List of disallowed codecs. Multiple values are separated by semicolon.")
			),
			'AstAccountAllowedCodec' => array(
				"Headline" => _("Allowed codec"),
				"Text" => _("List of allowed codecs. Multiple values are separated by semicolon.")
			),
			'AstAccountMusicOnHold' => array(
				"Headline" => _("Music on hold"),
				"Text" => _("Music to play on hold.")
			),
			'AstAccountExpirationTimestamp' => array(
				"Headline" => _("Expiration timestamp"),
				"Text" => _("Expiration timestamp (\"regseconds\" option).")
			),
			'AstAccountRegistrationContext' => array(
				"Headline" => _("Registration context"),
				"Text" => _("If registration context is specified, Asterisk will dynamically create and destroy a NoOp priority 1 extension for a given peer who registers or unregisters with Asterisk.")
			),
			'AstAccountRegistrationExten' => array(
				"Headline" => _("Registration extension"),
				"Text" => _("Used for registration context.")
			),
			'AstAccountCanCallForward' => array(
				"Headline" => _("Can call forward"),
				"Text" => _("Specifies if the user can call forward.")
			),
			'AstAccountIPAddress' => array(
				"Headline" => _("IP address"),
				"Text" => _("IP address of the peer. Valid only for realtime peers.")
			),
			'AstAccountDefaultUser' => array(
				"Headline" => _("Default user"),
				"Text" => _("Authentication user for outbound proxies.")
			),
			'AstAccountRegistrationServer' => array(
				"Headline" => _("Registration server"),
				"Text" => _("IP address or domain name of the registration server.")
			),
			'AstAccountLastQualifyMilliseconds' => array(
				"Headline" => _("Last qualify milliseconds"),
				"Text" => _("The number of milliseconds for the last qualify.")
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			),
		);
		// config options
		$configContainer = new htmlTable();
		$configRealmTable = new htmlTable();
		$configRealmTable->addElement(new htmlTableExtendedInputField(_('Asterisk realm'), 'asteriskAccount_AsteriskRealm', null, 'AsteriskRealm'));
		$configContainer->addElement($configRealmTable, true);
		$configContainer->addElement(new htmlSpacer(null, '10px'), true);
		$configHiddenHead = new htmlTable();
		$configHiddenHead->addElement(new htmlOutputText(_('Hidden options')));
		$configHiddenHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainerOptions = new htmlTable();
		$configContainer->addElement($configHiddenHead, true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountUserAgent', true, _('User agent'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountAMAFlags', true, _('AMA flags'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountCallGroup', true, _('Call groups'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountDTMFMode', true, _('DTFM flags'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountFromUser', true, _('From user'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountFromDomain', true, _('From domain'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountFullContact', true, _('Full contact'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountInsecure', true, _('Insecure'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountMailbox', true, _('Mailbox'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountNAT', true, _('NAT'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountDeny', true, _('Deny'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountPermit', true, _('Permit'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountPickupGroup', true, _('Pickup group'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountPort', true, _('Port'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountQualify', true, _('Qualify'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountRestrictCID', true, _('Restrict caller ID'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountRTPTimeout', true, _('RTP timeout'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountRTPHoldTimeout', true, _('RTP hold timeout'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountDisallowedCodec', true, _('Disallowed codec'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountAllowedCodec', true, _('Allowed codec'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountMusicOnHold', true, _('Music on hold'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountExpirationTimestamp', true, _('Expiration timestamp'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountRegistrationContext', true, _('Registration context'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountRegistrationExten', true, _('Registration extention'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountCanCallForward', true, _('Can call forward'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountIPAddress', true, _('IP address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountDefaultUser', true, _('Default user'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountRegistrationServer', true, _('Registration server'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('asteriskAccount_AstAccountLastQualifyMilliseconds', true, _('Last qualify milliseconds'), null, false));
		$configContainerOptions->addNewLine();
		$configContainer->addElement($configContainerOptions, true);
		$return['config_options']['user'] = $configContainer;
		// self service options	
		$selfServiceContainer = new htmlTable();
		$selfServiceContainer->addElement(new htmlTableExtendedInputField(_('Asterisk realm'), 'asteriskAccount_AsteriskRealm', null));
		$selfServiceContainer->addElement(new htmlHelpLink('AsteriskRealm', get_class($this)));
		$return['selfServiceSettings'] = $selfServiceContainer;
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Host'), 'asteriskAccount_AstAccountHost', null, 'AstAccountHost'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Account context'), 'asteriskAccount_AstAccountContext', null, 'AstAccountContext'), true);
		$profileContainer->addElement(new htmlTableExtendedInputField(_('Account type'), 'asteriskAccount_AstAccountType', null, 'AstAccountType'), true);
		$return['profile_options'] = $profileContainer;
		// profile mappings
		$return['profile_mappings'] = array(
			'asteriskAccount_AstAccountHost' => 'AstAccountHost',
			'asteriskAccount_AstAccountContext' => 'AstAccountContext',
			'asteriskAccount_AstAccountType' => 'AstAccountType',
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'AstAccountCallerID' => _('Caller ID'),
			'AstAccountContext' => _('Account context'),
			'AstAccountHost' => _('Host'),
			'AstAccountType' => _('Account type'),
			'AstAccountUserAgent' => _('User agent'),
			'AstAccountAMAFlags' => _('AMA flags'),
			'AstAccountCallGroup' => _('Call groups'),
			'AstAccountDTMFMode' => _('DTFM flags'),
			'AstAccountFromUser' => _('From user'),
			'AstAccountFromDomain' => _('From domain'),
			'AstAccountFullContact'=> _('Full contact'),
			'AstAccountInsecure' => _('Insecure'),
			'AstAccountMailbox' => _('Mailbox'),
			'AstAccountNAT' => _('NAT'),
			'AstAccountDeny' => _('Deny'),
			'AstAccountPermit' => _('Permit'),
			'AstAccountPickupGroup' => _('Pickup group'),
			'AstAccountPort' => _('Port'),
			'AstAccountQualify' => _('Qualify'),
			'AstAccountRestrictCID' => _('Restrict caller ID'),
			'AstAccountRTPTimeout' => _('RTP timeout'),
			'AstAccountRTPHoldTimeout' => _('RTP hold timeout'),
			'AstAccountDisallowedCodec' => _('Disallowed codec'),
			'AstAccountAllowedCodec' => _('Allowed codec'),
			'AstAccountMusicOnHold' => _('Music on hold'),
			'AstAccountExpirationTimestamp' => _('Expiration timestamp'),
			'AstAccountRegistrationContext' => _('Registration context'),
			'AstAccountRegistrationExten' => _('Registration extention'),
			'AstAccountCanCallForward' => _('Can call forward'),
			'AstAccountIPAddress' => _('IP address'),
			'AstAccountDefaultUser' => _('Default user'),
			'AstAccountRegistrationServer' => _('Registration server'),
			'AstAccountLastQualifyMilliseconds' => _('Last qualify milliseconds'),
		);
		// upload dependencies
		$return['upload_preDepends'] = array('posixAccount', 'inetOrgPerson');
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'asteriskAccount_AstAccountCallerID',
				'description' => _('Caller ID'),
				'help' => 'AstAccountCallerID',
				'example' => '12345',
				'required' => true
			),
			array(
				'name' => 'asteriskAccount_AstAccountContext',
				'description' => _('Account context'),
				'help' => 'AstAccountContext',
				'example' => 'default',
				'required' => true
			),
			array(
				'name' => 'asteriskAccount_AstAccountHost',
				'description' => _('Host'),
				'help' => 'AstAccountHost',
				'example' => 'dynamic',
				'default' => 'dynamic',
			),
			array(
				'name' => 'asteriskAccount_AstAccountRealmedPassword',
				'description' => _('Password'),
				'help' => 'AstAccountRealmedPassword',
				'example' => _('secret'),
			),
			array(
				'name' => 'asteriskAccount_AstAccountType',
				'description' => _('Account type'),
				'help' => 'AstAccountType',
				'example' => 'friend',
				'required' => true
			),
		);
		return $return;
	}

	/**
	 * This function fills the error message array with messages
	 */
	function load_Messages() {
		$this->messages['AstAccountCallerID'][0] = array('ERROR', _('Please enter a caller ID.'));
		$this->messages['AstAccountCallerID'][1] = array('ERROR', _('The caller ID format is invalid.'));
		$this->messages['AstAccountCallerID'][2] = array('ERROR', _('There is already another user with this caller ID.'));
		$this->messages['AstAccountCallerID'][3] = array('ERROR', _('Account %s:') . ' asteriskAccount_AstAccountCallerID', _('The caller ID format is invalid.'));
		$this->messages['AstAccountContext'][0] = array('ERROR', _('Please enter the extension context.'));
		$this->messages['AstAccountContext'][1] = array('ERROR', _('The extension context is invalid.'));
		$this->messages['AstAccountContext'][2] = array('ERROR', _('Account %s:') . ' asteriskAccount_AstAccountContext', _('The extension context is invalid.'));
		$this->messages['AstAccountHost'][0] = array('ERROR', _('Please enter the host name.'));
		$this->messages['AstAccountHost'][1] = array('ERROR', _('The host name is invalid.'));
		$this->messages['AstAccountHost'][2] = array('ERROR', _('Account %s:') . ' asteriskAccount_AstAccountHost', _('The host name is invalid.'));
		$this->messages['AstAccountType'][0] = array('ERROR', _('Please enter the account type.'));
		$this->messages['AstAccountType'][1] = array('ERROR', _('The account type is invalid.'));
		$this->messages['AstAccountType'][2] = array('ERROR', _('Account %s:') . ' asteriskAccount_AstAccountType', _('The account type is invalid.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'AsteriskSIPUser';
		}

		$return = new htmlTable();
		if (in_array('AsteriskSIPUser', $this->attributes['objectClass'])) {
			// caller ID
			$callerId = '';
			if (isset($this->attributes['AstAccountCallerID'][0])) {
				$callerId = $this->attributes['AstAccountCallerID'][0];
			}
			$callerIdInput = new htmlTableExtendedInputField(_("Caller ID"), 'AstAccountCallerID', $callerId, 'AstAccountCallerID');
			$callerIdInput->setRequired(true);
			$return->addElement($callerIdInput, true);
			
			// host
			$host = '';
			if (isset($this->attributes['AstAccountHost'][0])) {
				$host = $this->attributes['AstAccountHost'][0];
			}
			$hostInput = new htmlTableExtendedInputField(_("Host"), 'AstAccountHost', $host, 'AstAccountHost');
			$hostInput->setRequired(true);
			$return->addElement($hostInput, true);
			
			// context
			$context = '';
			if (isset($this->attributes['AstAccountContext'][0])) {
				$context = $this->attributes['AstAccountContext'][0];
			}
			$contextInput = new htmlTableExtendedInputField(_("Account context"), 'AstAccountContext', $context, 'AstAccountContext');
			$contextInput->setRequired(true);
			$return->addElement($contextInput, true);
			
			// account type
			$accountType = '';
			if (isset($this->attributes['AstAccountType'][0])) {
				$accountType = $this->attributes['AstAccountType'][0];
			}
			$accountTypeInput = new htmlTableExtendedInputField(_("Account type"), 'AstAccountType', $accountType, 'AstAccountType');
			$accountTypeInput->setRequired(true);
			$return->addElement($accountTypeInput, true);
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add Asterisk account')));
		}
		return $return;
	}

	/**
	 * Write variables into object and do some regex checks
	 */
	function process_attributes() {
		if (!in_array('AsteriskSIPUser', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		$this->attributes['AstAccountCallerID'] = array();
		$this->attributes['AstAccountHost'] = array();
		$this->attributes['AstAccountContext'] = array();
		$this->attributes['AstAccountType'] = array();
		if (isset($_POST['AstAccountCallerID'])) {
			$this->attributes['AstAccountCallerID'][0] = $_POST['AstAccountCallerID'];
			// check if caller ID is empty
			if($this->attributes['AstAccountCallerID'][0] == '') {
				$errors[] = $this->messages['AstAccountCallerID'][0];
			}
			// check format
			else if (!get_preg($this->attributes['AstAccountCallerID'][0], 'username')) {
				$errors[] = $this->messages['AstAccountCallerID'][1];
			}
			// check for duplicate caller ID
			else if (!isset($this->orig['AstAccountCallerID'][0]) || (($this->orig['AstAccountCallerID'][0] != $this->attributes['AstAccountCallerID'][0]))) {
				$entries = searchLDAPByAttribute('AstAccountCallerID', $this->attributes['AstAccountCallerID'][0], 'AsteriskSIPUser', array('dn'), array('user'));
				if (sizeof($entries) > 0) {
					$errors[] = $this->messages['AstAccountCallerID'][2];
				}
			}
		}
		if (isset($_POST['AstAccountHost'])) {
			$this->attributes['AstAccountHost'][0] = $_POST['AstAccountHost'];
			if($this->attributes['AstAccountHost'][0] == '') {
				$errors[] = $this->messages['AstAccountHost'][0];
			}
			elseif (!get_preg($this->attributes['AstAccountHost'][0], 'hostname')) {
				$errors[] = $this->messages['AstAccountHost'][1];
			}
		}
		if (isset($_POST['AstAccountContext'])) {
			$this->attributes['AstAccountContext'][0] = $_POST['AstAccountContext'];
			if($this->attributes['AstAccountContext'][0] == '') {
				$errors[] = $this->messages['AstAccountContext'][0];
			}
			elseif (!get_preg($this->attributes['AstAccountContext'][0], 'username')) {
				$errors[] = $this->messages['AstAccountContext'][1];
			}
		}
		if (isset($_POST['AstAccountType'])) {
			$this->attributes['AstAccountType'][0] = $_POST['AstAccountType'];
			if($this->attributes['AstAccountType'][0] == '') {
				$errors[] = $this->messages['AstAccountType'][0];
			}
			elseif (!get_preg($this->attributes['AstAccountType'][0], 'username')) {
				$errors[] = $this->messages['AstAccountType'][1];
			}
		}
		return $errors;
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
		if (!in_array('AsteriskSIPUser', $this->attributes['objectClass'])) {
			return array();
		}
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	* Returns a list of PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		$return[get_class($this) . '_AstAccountCallerID'] = array('<block><key>' . _('Caller ID') . '</key><value>' . $this->attributes['AstAccountCallerID'][0] . '</value></block>');
		$return[get_class($this) . '_AstAccountContext'] = array('<block><key>' . _('Account context') . '</key><value>' . $this->attributes['AstAccountContext'][0] . '</value></block>');
		$return[get_class($this) . '_AstAccountHost'] = array('<block><key>' . _('Host') . '</key><value>' . $this->attributes['AstAccountHost'][0] . '</value></block>');
		$return[get_class($this) . '_AstAccountType'] = array('<block><key>' . _('Account type') . '</key><value>' . $this->attributes['AstAccountType'][0] . '</value></block>');
		$AstAccountUserAgent = '';
		if (isset($this->attributes['AstAccountUserAgent'][0])) $AstAccountUserAgent = $this->attributes['AstAccountUserAgent'][0];
		$return[get_class($this) . '_AstAccountUserAgent'] = array('<block><key>' . _('User agent') . '</key><value>' . $AstAccountUserAgent . '</value></block>');
		$AstAccountAMAFlags = '';
		if (isset($this->attributes['AstAccountAMAFlags'][0])) $AstAccountAMAFlags = $this->attributes['AstAccountAMAFlags'][0];
		$return[get_class($this) . '_AstAccountAMAFlags'] = array('<block><key>' . _('AMA flags') . '</key><value>' . $AstAccountAMAFlags . '</value></block>');
		$AstAccountCallGroup = '';
		if (isset($this->attributes['AstAccountCallGroup'][0])) $AstAccountCallGroup = $this->attributes['AstAccountCallGroup'][0];
		$return[get_class($this) . '_AstAccountCallGroup'] = array('<block><key>' . _('Call groups') . '</key><value>' . $AstAccountCallGroup . '</value></block>');
		$AstAccountDTMFMode = '';
		if (isset($this->attributes['AstAccountDTMFMode'][0])) $AstAccountDTMFMode = $this->attributes['AstAccountDTMFMode'][0];
		$return[get_class($this) . '_AstAccountDTMFMode'] = array('<block><key>' . _('DTMF mode') . '</key><value>' . $AstAccountDTMFMode . '</value></block>');
		$AstAccountFromUser = '';
		if (isset($this->attributes['AstAccountFromUser'][0])) $AstAccountFromUser = $this->attributes['AstAccountFromUser'][0];
		$return[get_class($this) . '_AstAccountFromUser'] = array('<block><key>' . _('From user') . '</key><value>' . $AstAccountFromUser . '</value></block>');
		$AstAccountFromDomain = '';
		if (isset($this->attributes['AstAccountFromDomain'][0])) $AstAccountFromDomain = $this->attributes['AstAccountFromDomain'][0];
		$return[get_class($this) . '_AstAccountFromDomain'] = array('<block><key>' . _('From domain') . '</key><value>' . $AstAccountFromDomain . '</value></block>');
		$AstAccountFullContact = '';
		if (isset($this->attributes['AstAccountFullContact'][0])) $AstAccountFullContact = $this->attributes['AstAccountFullContact'][0];
		$return[get_class($this) . '_AstAccountFullContact'] = array('<block><key>' . _('Full contact') . '</key><value>' . $AstAccountFullContact . '</value></block>');
		$AstAccountInsecure = '';
		if (isset($this->attributes['AstAccountInsecure'][0])) $AstAccountInsecure = $this->attributes['AstAccountInsecure'][0];
		$return[get_class($this) . '_AstAccountInsecure'] = array('<block><key>' . _('Insecure') . '</key><value>' . $AstAccountInsecure . '</value></block>');
		$AstAccountMailbox = '';
		if (isset($this->attributes['AstAccountMailbox'][0])) $AstAccountMailbox = $this->attributes['AstAccountMailbox'][0];
		$return[get_class($this) . '_AstAccountMailbox'] = array('<block><key>' . _('Mailbox') . '</key><value>' . $AstAccountMailbox . '</value></block>');
		$AstAccountNAT = '';
		if (isset($this->attributes['AstAccountNAT'][0])) $AstAccountNAT = $this->attributes['AstAccountNAT'][0];
		$return[get_class($this) . '_AstAccountNAT'] = array('<block><key>' . _('NAT') . '</key><value>' . $AstAccountNAT . '</value></block>');
		$AstAccountDeny = '';
		if (isset($this->attributes['AstAccountDeny'][0])) $AstAccountDeny = $this->attributes['AstAccountDeny'][0];
		$return[get_class($this) . '_AstAccountDeny'] = array('<block><key>' . _('Deny') . '</key><value>' . $AstAccountDeny . '</value></block>');
		$AstAccountPermit = '';
		if (isset($this->attributes['AstAccountPermit'][0])) $AstAccountPermit = $this->attributes['AstAccountPermit'][0];
		$return[get_class($this) . '_AstAccountPermit'] = array('<block><key>' . _('Permit') . '</key><value>' . $AstAccountPermit . '</value></block>');
		$AstAccountPickupGroup = '';
		if (isset($this->attributes['AstAccountPickupGroup'][0])) $AstAccountPickupGroup = $this->attributes['AstAccountPickupGroup'][0];
		$return[get_class($this) . '_AstAccountPickupGroup'] = array('<block><key>' . _('Pickup group') . '</key><value>' . $AstAccountPickupGroup . '</value></block>');
		$AstAccountPort = '';
		if (isset($this->attributes['AstAccountPort'][0])) $AstAccountPort = $this->attributes['AstAccountPort'][0];
		$return[get_class($this) . '_AstAccountPort'] = array('<block><key>' . _('Port') . '</key><value>' . $AstAccountPort . '</value></block>');
		$AstAccountQualify = '';
		if (isset($this->attributes['AstAccountQualify'][0])) $AstAccountQualify = $this->attributes['AstAccountQualify'][0];
		$return[get_class($this) . '_AstAccountQualify'] = array('<block><key>' . _('Qualify') . '</key><value>' . $AstAccountQualify . '</value></block>');
		$AstAccountRestrictCID = '';
		if (isset($this->attributes['AstAccountRestrictCID'][0])) $AstAccountRestrictCID = $this->attributes['AstAccountRestrictCID'][0];
		$return[get_class($this) . '_AstAccountRestrictCID'] = array('<block><key>' . _('Restrict caller ID') . '</key><value>' . $AstAccountRestrictCID . '</value></block>');
		$AstAccountRTPTimeout = '';
		if (isset($this->attributes['AstAccountRTPTimeout'][0])) $AstAccountRTPTimeout = $this->attributes['AstAccountRTPTimeout'][0];
		$return[get_class($this) . '_AstAccountRTPTimeout'] = array('<block><key>' . _('RTP timeout') . '</key><value>' . $AstAccountRTPTimeout . '</value></block>');
		$AstAccountRTPHoldTimeout = '';
		if (isset($this->attributes['AstAccountRTPHoldTimeout'][0])) $AstAccountRTPHoldTimeout = $this->attributes['AstAccountRTPHoldTimeout'][0];
		$return[get_class($this) . '_AstAccountRTPHoldTimeout'] = array('<block><key>' . _('RTP hold timeout') . '</key><value>' . $AstAccountRTPHoldTimeout . '</value></block>');
		$AstAccountDisallowedCodec = '';
		if (isset($this->attributes['AstAccountDisallowedCodec'][0])) $AstAccountDisallowedCodec = implode('; ', $this->attributes['AstAccountDisallowedCodec']);
		$return[get_class($this) . '_AstAccountDisallowedCodec'] = array('<block><key>' . _('Disallowed codec') . '</key><value>' . $AstAccountDisallowedCodec . '</value></block>');
		$AstAccountAllowedCodec = '';
		if (isset($this->attributes['AstAccountAllowedCodec'][0])) $AstAccountAllowedCodec = implode('; ', $this->attributes['AstAccountAllowedCodec']);
		$return[get_class($this) . '_AstAccountAllowedCodec'] = array('<block><key>' . _('Allowed codec') . '</key><value>' . $AstAccountAllowedCodec . '</value></block>');
		$AstAccountMusicOnHold = '';
		if (isset($this->attributes['AstAccountMusicOnHold'][0])) $AstAccountMusicOnHold = $this->attributes['AstAccountMusicOnHold'][0];
		$return[get_class($this) . '_AstAccountMusicOnHold'] = array('<block><key>' . _('Music on hold') . '</key><value>' . $AstAccountMusicOnHold . '</value></block>');
		$AstAccountExpirationTimestamp = '';
		if (isset($this->attributes['AstAccountExpirationTimestamp'][0])) $AstAccountExpirationTimestamp = $this->attributes['AstAccountExpirationTimestamp'][0];
		$return[get_class($this) . '_AstAccountExpirationTimestamp'] = array('<block><key>' . _('Expiration timestamp') . '</key><value>' . $AstAccountExpirationTimestamp . '</value></block>');
		$AstAccountRegistrationContext = '';
		if (isset($this->attributes['AstAccountRegistrationContext'][0])) $AstAccountRegistrationContext = $this->attributes['AstAccountRegistrationContext'][0];
		$return[get_class($this) . '_AstAccountRegistrationContext'] = array('<block><key>' . _('Registration context') . '</key><value>' . $AstAccountRegistrationContext . '</value></block>');
		$AstAccountRegistrationExten = '';
		if (isset($this->attributes['AstAccountRegistrationExten'][0])) $AstAccountRegistrationExten = $this->attributes['AstAccountRegistrationExten'][0];
		$return[get_class($this) . '_AstAccountRegistrationExten'] = array('<block><key>' . _('Registration extension') . '</key><value>' . $AstAccountRegistrationExten . '</value></block>');
		$AstAccountCanCallForward = '';
		if (isset($this->attributes['AstAccountCanCallForward'][0])) $AstAccountCanCallForward = $this->attributes['AstAccountCanCallForward'][0];
		$return[get_class($this) . '_AstAccountCanCallForward'] = array('<block><key>' . _('Can call forward') . '</key><value>' . $AstAccountCanCallForward . '</value></block>');
		$AstAccountIPAddress = '';
		if (isset($this->attributes['AstAccountIPAddress'][0])) $AstAccountIPAddress = $this->attributes['AstAccountIPAddress'][0];
		$return[get_class($this) . '_AstAccountIPAddress'] = array('<block><key>' . _('IP address') . '</key><value>' . $AstAccountIPAddress . '</value></block>');
		$AstAccountDefaultUser = '';
		if (isset($this->attributes['AstAccountDefaultUser'][0])) $AstAccountDefaultUser = $this->attributes['AstAccountDefaultUser'][0];
		$return[get_class($this) . '_AstAccountDefaultUser'] = array('<block><key>' . _('Default user') . '</key><value>' . $AstAccountDefaultUser . '</value></block>');
		$AstAccountRegistrationServer = '';
		if (isset($this->attributes['AstAccountRegistrationServer'][0])) $AstAccountRegistrationServer = $this->attributes['AstAccountRegistrationServer'][0];
		$return[get_class($this) . '_AstAccountRegistrationServer'] = array('<block><key>' . _('Registration server') . '</key><value>' . $AstAccountRegistrationServer . '</value></block>');
		$AstAccountLastQualifyMilliseconds = '';
		if (isset($this->attributes['AstAccountLastQualifyMilliseconds'][0])) $AstAccountLastQualifyMilliseconds = $this->attributes['AstAccountLastQualifyMilliseconds'][0];
		$return[get_class($this) . '_AstAccountLastQualifyMilliseconds'] = array('<block><key>' . _('Last qualify milliseconds') . '</key><value>' . $AstAccountLastQualifyMilliseconds . '</value></block>');
		return $return;
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
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
		// add object class
			if (!in_array("AsteriskSIPUser", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "AsteriskSIPUser";
			// add account caller id
			if (get_preg($rawAccounts[$i][$ids['asteriskAccount_AstAccountCallerID']], 'username')) {
				$partialAccounts[$i]['AstAccountCallerID'] = $rawAccounts[$i][$ids['asteriskAccount_AstAccountCallerID']];
			}
			else {
				$errMsg = $this->messages['AstAccountCallerID'][3];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// add host
			if ($rawAccounts[$i][$ids['asteriskAccount_AstAccountHost']] == "") {
				// default value
				$partialAccounts[$i]['AstAccountHost'] = 'dynamic';
			}
			elseif (get_preg($rawAccounts[$i][$ids['asteriskAccount_AstAccountHost']], 'realname')) {
				$partialAccounts[$i]['AstAccountHost'] = $rawAccounts[$i][$ids['asteriskAccount_AstAccountHost']];
			}
			else {
				$errMsg = $this->messages['AstAccountHost'][2];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			//add context
			if (($rawAccounts[$i][$ids['asteriskAccount_AstAccountContext']] != "") && (get_preg($rawAccounts[$i][$ids['asteriskAccount_AstAccountContext']], 'realname')) ) {
				$partialAccounts[$i]['AstAccountContext'] = $rawAccounts[$i][$ids['asteriskAccount_AstAccountContext']];
			}
			else {
				$errMsg = $this->messages['AstAccountContext'][2];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			//add account type
			if (($rawAccounts[$i][$ids['asteriskAccount_AstAccountType']] != "") && (get_preg($rawAccounts[$i][$ids['asteriskAccount_AstAccountType']], 'username')) ) {
				$partialAccounts[$i]['AstAccountType'] = $rawAccounts[$i][$ids['asteriskAccount_AstAccountType']];
			}
			else {
				$errMsg = $this->messages['AstAccountType'][2];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			//add password
			if ($rawAccounts[$i][$ids['asteriskAccount_AstAccountRealmedPassword']] != "") {
				$attributes = array('AstAccountCallerID' => array($partialAccounts[$i]['AstAccountCallerID'])); // fake attribute list for password building
				$pwdString = asteriskAccount::buildPasswordString($attributes, $this->moduleSettings, $rawAccounts[$i][$ids['asteriskAccount_AstAccountRealmedPassword']]);
				$partialAccounts[$i]['AstAccountRealmedPassword'] = $pwdString;
			}
		}
		return $messages;
	}



        /**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		if (!in_array('AsteriskSIPUser', $this->attributes['objectClass'])) {
			return false;
		}
		return true;
	}

	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *			   e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules) {
		if (!in_array(get_class($this), $modules)) {
			return array();
		}
		$this->attributes['AstAccountRealmedPassword'][0] = asteriskAccount::buildPasswordString($this->attributes, $this->moduleSettings, $password);
		return array();
	}
	
	/**
	 * Builds the password string for the password attribute.
	 *
	 * @param array $attributes LDAP attributes
	 * @param array $moduleSettings module configuration settings
	 * @param String $password password
	 * @return String value for password attribute
	 */
	public static function buildPasswordString(&$attributes, &$moduleSettings, $password) {
		$astRealm = asteriskAccount::ASTERISK_DEFAULT_REALM;
		$asteriskRealmFromProfile = $moduleSettings['asteriskAccount_AsteriskRealm'][0];
		if ($asteriskRealmFromProfile != ""){
			$astRealm = $asteriskRealmFromProfile;
		}
		return asteriskAccount::hashPassword($attributes['AstAccountCallerID'][0] . ":" . $astRealm . ":" . $password);
	}
	
	/**
	 * Hashes a password value to Asterisk format.
	 *
	 * @param String $password password
	 * @return String hash
	 */
	private static function hashPassword($password) {
		return "{MD5}" . md5($password);
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
		if (!in_array_ignore_case('AsteriskSIPUser', $attributes['objectClass'])) {
			return $return;
		}
		if (isset($_POST['posixAccount_password']) && ($_POST['posixAccount_password'] != '')) {
			if ($_POST['posixAccount_password'] != $_POST['posixAccount_password2']) {
				return $return;
			}
			else {
				if (!get_preg($_POST['posixAccount_password'], 'password')) {
					return $return;
				}
				else {
					// sync password
					if (in_array('syncAsteriskPassword', $fields)) {
						$return['mod']['AstAccountRealmedPassword'][0] = asteriskAccount::buildPasswordString($attributes, $this->selfServiceSettings->moduleSettings, $_POST['posixAccount_password']);
					}
				}
			}
		}
		return $return;
	}

}

?>
