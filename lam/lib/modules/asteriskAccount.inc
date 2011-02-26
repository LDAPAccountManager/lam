<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009 - 2010  Pavel Pozdnyak
                2009 - 2010  Roland Gruber

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
			'AstAccountRealmedPassword', 'AstAccountContext', 'AstAccountType');
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
		);
		// config options
		$configContainer = new htmlTable();
		$configContainer->addElement(new htmlTableExtendedInputField(_('Asterisk realm'), 'asteriskAccount_AsteriskRealm', null, 'AsteriskRealm'));
		$return['config_options']['user'] = $configContainer;
		// self service options	
		$selfServiceContainer = new htmlTable();
		$selfServiceContainer->addElement(new htmlTableExtendedInputField(_('Asterisk realm'), 'asteriskAccount_AsteriskRealm', null, 'AsteriskRealm'));
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
