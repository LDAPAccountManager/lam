<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2005 - 2011  Roland Gruber

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
* Manages Samba 3 domain entries.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Samba 3 domain entries.
*
* @package modules
*/
class sambaDomain extends baseModule {

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
		$return['icon'] = 'samba.png';
		// manages host accounts
		$return["account_types"] = array("smbDomain");
		// alias name
		$return["alias"] = _("Samba domain");
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("sambaDomainName" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=sambaDomain)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('sambaDomain');
		// managed attributes
		$return['attributes'] = array('sambaDomainName', 'sambaSID', 'sambaNextRid', 'sambaNextGroupRid',
			'sambaNextUserRid', 'sambaAlgorithmicRidBase', 'sambaMinPwdLength', 'sambaPwdHistoryLength',
			'sambaLogonToChgPwd', 'sambaForceLogoff', 'sambaRefuseMachinePwdChange', 'sambaLockoutThreshold',
			'sambaMinPwdAge', 'sambaMaxPwdAge', 'sambaLockoutDuration', 'sambaLockoutObservationWindow');
		// help Entries
		$return['help'] = array(
			'domainName' => array(
				"Headline" => _("Domain name"),
				"Text" => _("The name of your Windows domain or workgroup.")
			),
			'domainSID' => array(
				"Headline" => _("Domain SID"),
				"Text" => _("The SID of your Samba server. Get it with \"net getlocalsid\".")
			),
			'nextRID' => array(
				"Headline" => _("Next RID"),
				"Text" => _("Next RID to use when creating accounts (only used by Winbind).")
			),
			'nextUserRID' => array(
				"Headline" => _("Next user RID"),
				"Text" => _("Next RID to use when creating user accounts (only used by Winbind).")
			),
			'nextGroupRID' => array(
				"Headline" => _("Next group RID"),
				"Text" => _("Next RID to use when creating group accounts (only used by Winbind).")
			),
			'RIDbase' => array(
				"Headline" => _("RID base"),
				"Text" => _("Used for calculating RIDs from UID/GID. Do not change if unsure.")
			),
			'minPwdLength' => array(
				"Headline" => _("Minimal password length"),
				"Text" => _("Here you can specify the minimum number of characters for a user password.")
			),
			'pwdHistLength' => array(
				"Headline" => _("Password history length"),
				"Text" => _("This is the number of passwords which are saved to prevent that users reuse old passwords.")
			),
			'logonToChgPwd' => array(
				"Headline" => _("Logon for password change"),
				"Text" => _("If set then users need to login to change their password.")
			),
			'forceLogoff' => array(
				"Headline" => _("Disconnect users outside logon hours"),
				"Text" => _("Disconnects users if they are logged in outside logon hours.")
			),
			'refuseMachinePwdChange' => array(
				"Headline" => _("Allow machine password changes"),
				"Text" => _("Defines if workstations may change their passwords.")
			),
			'lockoutThreshold' => array(
				"Headline" => _("Lockout users after bad logon attempts"),
				"Text" => _("This is the number of bad logon attempts (0 - 999) before the account is deactivated. 0 means unlimited attempts.")
			),
			'minPwdAge' => array(
				"Headline" => _("Minimum password age"),
				"Text" => _("Number of seconds after the user is allowed to change his password again.")
			),
			'maxPwdAge' => array(
				"Headline" => _("Maximum password age"),
				"Text" => _("Number of seconds after which the user must change his password.")
			),
			'lockoutDuration' => array(
				"Headline" => _("Lockout duration"),
				"Text" => _("This is the time (in minutes) for which the user may not log in after the account was locked. -1 means forever.")
			),
			'lockoutObservationWindow' => array(
				"Headline" => _("Reset time after lockout"),
				"Text" => _("Number of minutes after which the bad logon attempts are reset.")
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'sambaDomain_domainName',
				'description' => _('Domain name'),
				'help' => 'domainName',
				'example' => _('Workgroup'),
				'required' => true
			),
			array(
				'name' => 'sambaDomain_domainSID',
				'description' => _('Domain SID'),
				'help' => 'domainSID',
				'example' => 'S-1-1-22-123-123-123',
				'required' => true
			),
			array(
				'name' => 'sambaDomain_RIDbase',
				'description' => _('RID base'),
				'help' => 'RIDbase',
				'example' => '1000',
				'default' => 1000
			),
			array(
				'name' => 'sambaDomain_nextRID',
				'description' => _('Next RID'),
				'help' => 'nextRID',
				'example' => '12345'
			),
			array(
				'name' => 'sambaDomain_nextUserRID',
				'description' => _('Next user RID'),
				'help' => 'nextUserRID',
				'example' => '12345'
			),
			array(
				'name' => 'sambaDomain_nextGroupRID',
				'description' => _('Next group RID'),
				'help' => 'nextGroupRID',
				'example' => '12345'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'domainName' => _('Domain name'),
			'domainSID' => _('Domain SID'),
			'nextRID' => _('Next RID'),
			'nextUserRID' => _('Next user RID'),
			'nextGroupRID' => _('Next group RID'),
			'RIDbase' => _('RID base'),
			'minPwdLength' => _('Minimal password length'),
			'pwdHistoryLength' => _('Password history length'),
			'logonToChgPwd' => _('Logon for password change'),
			'forceLogoff' => _('Disconnect users outside logon hours'),
			'refuseMachinePwdChange' => _('Allow machine password changes'),
			'lockoutThreshold' => _('Lockout users after bad logon attempts'),
			'minPwdAge' => _('Minimum password age'),
			'maxPwdAge' => _('Maximum password age'),
			'lockoutDuration' => _('Lockout duration'),
			'lockoutObservationWindow' => _('Reset time after lockout')
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['domainName'][0] = array('ERROR', _('Domain name is invalid!'));
		$this->messages['domainName'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_domainName', _('Domain name is invalid!'));
		$this->messages['domainSID'][0] = array('ERROR', _('Samba 3 domain SID is invalid!'));
		$this->messages['domainSID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_domainSID', _('Samba 3 domain SID is invalid!'));
		$this->messages['nextRID'][0] = array('ERROR', _('Next RID is not a number!'));
		$this->messages['nextRID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_nextRID', _('Next RID is not a number!'));
		$this->messages['nextUserRID'][0] = array('ERROR', _('Next user RID is not a number!'));
		$this->messages['nextUserRID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_nextUserRID', _('Next user RID is not a number!'));
		$this->messages['nextGroupRID'][0] = array('ERROR', _('Next group RID is not a number!'));
		$this->messages['nextGroupRID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_nextGroupRID', _('Next group RID is not a number!'));
		$this->messages['RIDbase'][0] = array('ERROR', _('Algorithmic RID base is not a number!'));
		$this->messages['RIDbase'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_RIDbase', _('Algorithmic RID base is not a number!'));
		$this->messages['pwdAge_cmp'][0] = array('ERROR',  _('Maximum password age'), _('Password maximum age must be bigger than password minimum age.'));
		$this->messages['pwdAgeMin'][0] = array('ERROR', _('Minimum password age'), _('Password minimum age must be are natural number.'));
		$this->messages['pwdAgeMax'][0] = array('ERROR', _('Maximum password age'), _('Password maximum age must be are natural number.'));
		$this->messages['lockoutDuration'][0] = array('ERROR', _('Lockout duration'), _('Lockout duration must be are natural number.'));
		$this->messages['lockoutObservationWindow'][0] = array('ERROR', _('Reset time after lockout'), _('Reset time after lockout must be are natural number.'));
		$this->messages['lockoutThreshold'][0] = array('ERROR', _('Lockout users after bad logon attempts'), _('Lockout users after bad logon attempts must be between 0 and 999.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		// domain name
		$domainName = '';
		if (isset($this->attributes['sambaDomainName'][0])) {
			$domainName = $this->attributes['sambaDomainName'][0];
		}
		if ($this->getAccountContainer()->isNewAccount) {
			$domainNameInput = new htmlTableExtendedInputField(_('Domain name'), 'domainName', $domainName, 'domainName');
			$domainNameInput->setRequired(true);
			$return->addElement($domainNameInput, true);
		}
		else {
			$return->addElement(new htmlOutputText(_('Domain name')));
			$return->addElement(new htmlOutputText($domainName));
			$return->addElement(new htmlHelpLink('domainName'), true);
		}
		// domain SID
		$domainSID = '';
		if (isset($this->attributes['sambaSID'][0])) {
			$domainSID = $this->attributes['sambaSID'][0];
		}
		if ($this->getAccountContainer()->isNewAccount) {
			$domainSIDInput = new htmlTableExtendedInputField(_('Domain SID'), 'domainSID', $domainSID, 'domainSID');
			$domainSIDInput->setRequired(true);
			$return->addElement($domainSIDInput, true);
		}
		else {
			$return->addElement(new htmlOutputText(_('Domain SID')));
			$return->addElement(new htmlOutputText($domainSID));
			$return->addElement(new htmlHelpLink('domainSID'), true);
		}

		$return->addElement(new htmlSubTitle(_("Password policy")), true);

		/* group policies */

		// minimum password length
		$sambaMinPwdLength = '-';
		if (isset($this->attributes['sambaMinPwdLength'][0])) {
			$sambaMinPwdLength = $this->attributes['sambaMinPwdLength'][0];
		}
		$return->addElement(new htmlTableExtendedSelect('minPwdLength', array('-', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
			array($sambaMinPwdLength), _('Minimal password length'), 'minPwdLength'), true);
		// password history length
		$sambaPwdHistoryLength = '-';
		if (isset($this->attributes['sambaPwdHistoryLength'][0])) {
			$sambaPwdHistoryLength = $this->attributes['sambaPwdHistoryLength'][0];
		}
		$return->addElement(new htmlTableExtendedSelect('pwdHistLength', array('-', 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
			array($sambaPwdHistoryLength), _('Password history length'), 'pwdHistLength'), true);
		// logon to change password
		$sambaLogonToChgPwd = '-';
		if (isset($this->attributes['sambaLogonToChgPwd'][0])) {
			$sambaLogonToChgPwd = $this->attributes['sambaLogonToChgPwd'][0];
		}
		$logonPwdChangeSelect = new htmlTableExtendedSelect('logonToChgPwd', array('-' => '-', _('Off') => '0', _('On') => '2'),
			array($sambaLogonToChgPwd), _('Logon for password change'), 'logonToChgPwd');
		$logonPwdChangeSelect->setHasDescriptiveElements(true);
		$return->addElement($logonPwdChangeSelect, true);
		// force logoff
		$sambaForceLogoff = '-';
		if (isset($this->attributes['sambaForceLogoff'][0])) {
			$sambaForceLogoff = $this->attributes['sambaForceLogoff'][0];
		}
		$forceLogoffSelect = new htmlTableExtendedSelect('forceLogoff', array('-' => '-', _('Off') => '-1', _('On') => '0'),
			array($sambaForceLogoff), _('Disconnect users outside logon hours'), 'forceLogoff');
		$forceLogoffSelect->setHasDescriptiveElements(true);
		$return->addElement($forceLogoffSelect, true);
		// do not allow machine password change
		$sambaRefuseMachinePwdChange = '-';
		if (isset($this->attributes['sambaRefuseMachinePwdChange'][0])) {
			$sambaRefuseMachinePwdChange = $this->attributes['sambaRefuseMachinePwdChange'][0];
		}
		$refuseMachPwdChange = new htmlTableExtendedSelect('refuseMachinePwdChange', array('-' => '-', _('Off') => '0', _('On') => '1'),
			array($sambaRefuseMachinePwdChange), _('Allow machine password changes'), 'refuseMachinePwdChange');
		$refuseMachPwdChange->setHasDescriptiveElements(true);
		$return->addElement($refuseMachPwdChange, true);
		// Lockout users after bad logon attempts
		$sambaLockoutThreshold = '';
		if (isset($this->attributes['sambaLockoutThreshold'][0])) {
			$sambaLockoutThreshold = $this->attributes['sambaLockoutThreshold'][0];
		}
		$sambaLockoutThresholdInput = new htmlTableExtendedInputField(_('Lockout users after bad logon attempts'), 'lockoutThreshold', $sambaLockoutThreshold, 'lockoutThreshold');
		$sambaLockoutThresholdInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($sambaLockoutThresholdInput, true);
		// Minimum password age
		$sambaMinPwdAge = '';
		if (isset($this->attributes['sambaMinPwdAge'][0])) {
			$sambaMinPwdAge = $this->attributes['sambaMinPwdAge'][0];
		}
		$sambaMinPwdAgeInput = new htmlTableExtendedInputField(_('Minimum password age'), 'minPwdAge', $sambaMinPwdAge, 'minPwdAge');
		$sambaMinPwdAgeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($sambaMinPwdAgeInput, true);
		// Maximum password age
		$sambaMaxPwdAge = '';
		if (isset($this->attributes['sambaMaxPwdAge'][0])) {
			$sambaMaxPwdAge = $this->attributes['sambaMaxPwdAge'][0];
		}
		$sambaMaxPwdAgeInput = new htmlTableExtendedInputField(_('Maximum password age'), 'maxPwdAge', $sambaMaxPwdAge, 'maxPwdAge');
		$sambaMaxPwdAgeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($sambaMaxPwdAgeInput, true);
		// Lockout duration
		$sambaLockoutDuration = '';
		if (isset($this->attributes['sambaLockoutDuration'][0])) {
			$sambaLockoutDuration = $this->attributes['sambaLockoutDuration'][0];
		}
		$sambaLockoutDurationInput = new htmlTableExtendedInputField(_('Lockout duration'), 'lockoutDuration', $sambaLockoutDuration, 'lockoutDuration');
		$sambaLockoutDurationInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($sambaLockoutDurationInput, true);
		// Reset time after lockout
		$sambaLockoutObservationWindow = '';
		if (isset($this->attributes['sambaLockoutObservationWindow'][0])) {
			$sambaLockoutObservationWindow = $this->attributes['sambaLockoutObservationWindow'][0];
		}
		$sambaLockoutObservationWindowInput = new htmlTableExtendedInputField(_('Reset time after lockout'), 'lockoutObservationWindow', $sambaLockoutObservationWindow, 'lockoutObservationWindow');
		$sambaLockoutObservationWindowInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($sambaLockoutObservationWindowInput, true);

		$return->addElement(new htmlSubTitle(_('RID settings')), true);

		/* RID settings */

		// next RID
		$nextRID = '';
		if (isset($this->attributes['sambaNextRid'][0])) {
			$nextRID = $this->attributes['sambaNextRid'][0];
		}
		$nextRIDInput = new htmlTableExtendedInputField(_('Next RID'), 'nextRID', $nextRID, 'nextRID');
		$nextRIDInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($nextRIDInput, true);
		// next user RID
		$nextUserRID = '';
		if (isset($this->attributes['sambaNextUserRid'][0])) {
			$nextUserRID = $this->attributes['sambaNextUserRid'][0];
		}
		$nextUserRIDInput = new htmlTableExtendedInputField(_('Next user RID'), 'nextUserRID', $nextUserRID, 'nextUserRID');
		$nextUserRIDInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($nextUserRIDInput, true);
		// next group RID
		$nextGroupRID = '';
		if (isset($this->attributes['sambaNextGroupRid'][0])) {
			$nextGroupRID = $this->attributes['sambaNextGroupRid'][0];
		}
		$nextGroupRIDInput = new htmlTableExtendedInputField(_('Next group RID'), 'nextGroupRID', $nextGroupRID, 'nextGroupRID');
		$nextGroupRIDInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
		$return->addElement($nextGroupRIDInput, true);
		// RID base
		if (!isset($this->attributes['sambaAlgorithmicRidBase'][0])) $this->attributes['sambaAlgorithmicRidBase'][0] = 1000;
		if ($this->getAccountContainer()->isNewAccount) {
			$ridBaseInput = new htmlTableExtendedInputField(_('RID base'), 'RIDbase', $this->attributes['sambaAlgorithmicRidBase'][0], 'RIDbase');
			$ridBaseInput->setRequired(true);
			$ridBaseInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$return->addElement($ridBaseInput, true);
		}
		else {
			$return->addElement(new htmlOutputText(_('RID base')));
			$return->addElement(new htmlOutputText($this->attributes['sambaAlgorithmicRidBase'][0]));
			$return->addElement(new htmlHelpLink('RIDbase'), true);
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
		if ($this->getAccountContainer()->isNewAccount) {
			// domain SID
			$this->attributes['sambaSID'][0] = $_POST['domainSID'];
			if (!get_preg($_POST['domainSID'], 'domainSID')) {
				$errors[] = $this->messages['domainSID'][0];
			}
			else {
				$this->attributes['sambaSID'][0] = $_POST['domainSID'];
			}
			// RID base
			if (!get_preg($_POST['RIDbase'], 'digit') && !($_POST['RIDbase'] == '')) {
				$errors[] = $this->messages['RIDbase'][0];
			}
			else {
				$this->attributes['sambaAlgorithmicRidBase'][0] = $_POST['RIDbase'];
			}
			// domain name
			if (!get_preg($_POST['domainName'], 'domainname') && !($_POST['domainName'] == '')) {
				$errors[] = $this->messages['domainName'][0];
			}
			else {
				$this->attributes['sambaDomainName'][0] = $_POST['domainName'];
			}
		}
		// next RID
		if (!get_preg($_POST['nextRID'], 'digit')) {
			$errors[] = $this->messages['nextRID'][0];
		}
		else {
			$this->attributes['sambaNextRid'][0] = $_POST['nextRID'];
		}
		// next user RID
		if (!get_preg($_POST['nextUserRID'], 'digit')) {
			$errors[] = $this->messages['nextUserRID'][0];
		}
		else {
			$this->attributes['sambaNextUserRid'][0] = $_POST['nextUserRID'];
		}
		// next group RID
		if (!get_preg($_POST['nextGroupRID'], 'digit')) {
			$errors[] = $this->messages['nextGroupRID'][0];
		}
		else {
			$this->attributes['sambaNextGroupRid'][0] = $_POST['nextGroupRID'];
		}
		// minimum password length
		if ($_POST['minPwdLength'] === '-') {
			if (isset($this->attributes['sambaMinPwdLength'])) unset($this->attributes['sambaMinPwdLength'][0]);
		}
		else {
			$this->attributes['sambaMinPwdLength'][0] = $_POST['minPwdLength'];
		}
		// password history length
		if ($_POST['pwdHistLength'] === '-') {
			if (isset($this->attributes['sambaPwdHistoryLength'])) unset($this->attributes['sambaPwdHistoryLength'][0]);
		}
		else {
			$this->attributes['sambaPwdHistoryLength'][0] = $_POST['pwdHistLength'];
		}
		// logon for password change
		if ($_POST['logonToChgPwd'] === '-') {
			if (isset($this->attributes['sambaLogonToChgPwd'])) unset($this->attributes['sambaLogonToChgPwd'][0]);
		}
		else {
			$this->attributes['sambaLogonToChgPwd'][0] = $_POST['logonToChgPwd'];
		}
		// force logoff
		if ($_POST['forceLogoff'] === '-') {
			if (isset($this->attributes['sambaForceLogoff'])) unset($this->attributes['sambaForceLogoff'][0]);
		}
		else {
			$this->attributes['sambaForceLogoff'][0] = $_POST['forceLogoff'];
		}
		// do not allow machine password changes
		if ($_POST['refuseMachinePwdChange'] === '-') {
			if (isset($this->attributes['sambaRefuseMachinePwdChange'])) unset($this->attributes['sambaRefuseMachinePwdChange'][0]);
		}
		else {
			$this->attributes['sambaRefuseMachinePwdChange'][0] = $_POST['refuseMachinePwdChange'];
		}
		// Lockout users after bad logon attempts
		if (! isset($_POST['lockoutThreshold']) || ($_POST['lockoutThreshold'] == '')) {
			if (isset($this->attributes['sambaLockoutThreshold'])) unset($this->attributes['sambaLockoutThreshold'][0]);
		}
		else {
			if (is_numeric($_POST['lockoutThreshold']) && ($_POST['lockoutThreshold'] >= 0) && ($_POST['lockoutThreshold'] < 1000)) {
				$this->attributes['sambaLockoutThreshold'][0] = $_POST['lockoutThreshold'];
			}
			else {
				$errors[] = $this->messages['lockoutThreshold'][0];
			}
		}
		// Minimum password age
		if (! isset($_POST['minPwdAge']) || ($_POST['minPwdAge'] == '')) {
			if (isset($this->attributes['sambaMinPwdAge'])) unset($this->attributes['sambaMinPwdAge'][0]);
		}
		else {
			if (is_numeric($_POST['minPwdAge']) && ($_POST['minPwdAge'] > -2)) {
				$this->attributes['sambaMinPwdAge'][0] = $_POST['minPwdAge'];
			}
			else {
				$errors[] = $this->messages['pwdAgeMin'][0];
			}
		}
		// Maximum password age
		if (! isset($_POST['maxPwdAge']) || ($_POST['maxPwdAge'] == '')) {
			if (isset($this->attributes['sambaMaxPwdAge'])) unset($this->attributes['sambaMaxPwdAge'][0]);
		}
		else {
			if (!is_numeric($_POST['maxPwdAge']) || ($_POST['maxPwdAge'] < -1)) {
				$errors[] = $this->messages['pwdAgeMax'][0];
			}
			elseif ($_POST['maxPwdAge'] < $_POST['minPwdAge']) {
				$errors[] = $this->messages['pwdAge_cmp'][0];
			}
			else {
				$this->attributes['sambaMaxPwdAge'][0] = $_POST['maxPwdAge'];
			}
		}
		// Lockout duration
		if (! isset($_POST['lockoutDuration']) || ($_POST['lockoutDuration'] == '')) {
			if (isset($this->attributes['sambaLockoutDuration'])) unset($this->attributes['sambaLockoutDuration'][0]);
		}
		else {
			if (is_numeric($_POST['lockoutDuration']) && ($_POST['lockoutDuration'] > -2)) {
				$this->attributes['sambaLockoutDuration'][0] = $_POST['lockoutDuration'];
			}
			else {
				$errors[] = $this->messages['lockoutDuration'][0];
			}
		}
		// Reset time after lockout
		if (! isset($_POST['lockoutObservationWindow']) || ($_POST['lockoutObservationWindow'] == '')) {
			if (isset($this->attributes['sambaLockoutObservationWindow'])) unset($this->attributes['sambaLockoutObservationWindow'][0]);
		}
		else {
			if (is_numeric($_POST['lockoutObservationWindow']) && ($_POST['lockoutObservationWindow'] > -1)) {
				$this->attributes['sambaLockoutObservationWindow'][0] = $_POST['lockoutObservationWindow'];
			}
			else {
				$errors[] = $this->messages['lockoutObservationWindow'][0];
			}
		}

		return $errors;
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
			if (!in_array("sambaDomain", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaDomain";
			// domain name
			if (get_preg($rawAccounts[$i][$ids['sambaDomain_domainName']], 'domainname')) {
				$partialAccounts[$i]['sambaDomainName'] = $rawAccounts[$i][$ids['sambaDomain_domainName']];
			}
			else {
				$errMsg = $this->messages['domainName'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// domain SID
			if (get_preg($rawAccounts[$i][$ids['sambaDomain_domainSID']], 'domainSID')) {
				$partialAccounts[$i]['sambaSID'] = $rawAccounts[$i][$ids['sambaDomain_domainSID']];
			}
			else {
				$errMsg = $this->messages['domainSID'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// RID base
			if ($rawAccounts[$i][$ids['sambaDomain_RIDbase']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_RIDbase']], 'digit')) {
					$partialAccounts[$i]['sambaAlgorithmicRidBase'] = $rawAccounts[$i][$ids['sambaDomain_RIDbase']];
				}
				else {
					$errMsg = $this->messages['RIDbase'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['sambaAlgorithmicRidBase'] = '1000';
			}
			// next RID
			if ($rawAccounts[$i][$ids['sambaDomain_nextRID']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_nextRID']], 'digit')) {
					$partialAccounts[$i]['sambaNextRid'] = $rawAccounts[$i][$ids['sambaDomain_nextRID']];
				}
				else {
					$errMsg = $this->messages['nextRID'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// next user RID
			if ($rawAccounts[$i][$ids['sambaDomain_nextUserRID']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_nextUserRID']], 'digit')) {
					$partialAccounts[$i]['sambaNextUserRid'] = $rawAccounts[$i][$ids['sambaDomain_nextUserRID']];
				}
				else {
					$errMsg = $this->messages['nextUserRID'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// next group RID
			if ($rawAccounts[$i][$ids['sambaDomain_nextGroupRID']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_nextGroupRID']], 'digit')) {
					$partialAccounts[$i]['sambaNextGroupRid'] = $rawAccounts[$i][$ids['sambaDomain_nextGroupRID']];
				}
				else {
					$errMsg = $this->messages['nextGroupRID'][1];
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
		if (sizeof($this->attributes['sambaDomainName']) > 0) {
			$return['sambaDomain_domainName'][0] = '<block><key>' . _('Domain name') . '</key><value>' . implode(', ', $this->attributes['sambaDomainName']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaSID']) > 0) {
			$return['sambaDomain_domainSID'][0] = '<block><key>' . _('Domain SID') . '</key><value>' . implode(', ', $this->attributes['sambaSID']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaNextRid']) > 0) {
			$return['sambaDomain_nextRID'][0] = '<block><key>' . _('Next RID') . '</key><value>' . implode(', ', $this->attributes['sambaNextRid']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaNextUserRid']) > 0) {
			$return['sambaDomain_nextUserRID'][0] = '<block><key>' . _('Next user RID') . '</key><value>' . implode(', ', $this->attributes['sambaNextUserRid']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaNextGroupRid']) > 0) {
			$return['sambaDomain_nextGroupRID'][0] = '<block><key>' . _('Next group RID') . '</key><value>' . implode(', ', $this->attributes['sambaNextGroupRid']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaAlgorithmicRidBase']) > 0) {
			$return['sambaDomain_RIDbase'][0] = '<block><key>' . _('RID base') . '</key><value>' . implode(', ', $this->attributes['sambaAlgorithmicRidBase']) . '</value></block>';
		}
		if (isset($this->attributes['sambaMinPwdLength'])) {
			$return['sambaDomain_minPwdLength'][0] = '<block><key>' . _('Minimal password length') . '</key><value>' . implode(', ', $this->attributes['sambaMinPwdLength']) . '</value></block>';
		}
		if (isset($this->attributes['sambaPwdHistoryLength'])) {
			$return['sambaDomain_pwdHistoryLength'][0] = '<block><key>' . _('Password history length') . '</key><value>' . implode(', ', $this->attributes['sambaPwdHistoryLength']) . '</value></block>';
		}
		if (isset($this->attributes['sambaLogonToChgPwd'])) {
			$logonToChgPwd = _('Off');
			if ($this->attributes['sambaPwdHistoryLength'][0] == 2) $logonToChgPwd = _('On');
			$return['sambaDomain_logonToChgPwd'][0] = '<block><key>' . _('Logon for password change') . '</key><value>' . $logonToChgPwd . '</value></block>';
		}
		if (isset($this->attributes['sambaForceLogoff'])) {
			$forceLogoff = _('Off');
			if ($this->attributes['sambaForceLogoff'][0] == 0) $forceLogoff = _('On');
			$return['sambaDomain_forceLogoff'][0] = '<block><key>' . _('Disconnect users outside logon hours') . '</key><value>' . $forceLogoff . '</value></block>';
		}
		if (isset($this->attributes['sambaRefuseMachinePwdChange'])) {
			$refuseMachinePwdChange = _('Off');
			if ($this->attributes['sambaRefuseMachinePwdChange'][0] == 0) $refuseMachinePwdChange = _('On');
			$return['sambaDomain_refuseMachinePwdChange'][0] = '<block><key>' . _('Allow machine password changes') . '</key><value>' . $refuseMachinePwdChange . '</value></block>';
		}
		if (isset($this->attributes['sambaLockoutThreshold'][0])) {
			$return['sambaDomain_lockoutThreshold'][0] = '<block><key>' . _('Lockout users after bad logon attempts') . '</key><value>' . $this->attributes['sambaLockoutThreshold'][0] . '</value></block>';
		}
		if (isset($this->attributes['sambaMinPwdAge'])) {
			$return['sambaDomain_minPwdAge'][0] = '<block><key>' . _('Minimum password age') . '</key><value>' . implode(', ', $this->attributes['sambaMinPwdAge']) . '</value></block>';
		}
		if (isset($this->attributes['sambaMaxPwdAge'])) {
			$return['sambaDomain_maxPwdAge'][0] = '<block><key>' . _('Maximum password age') . '</key><value>' . implode(', ', $this->attributes['sambaMaxPwdAge']) . '</value></block>';
		}
		if (isset($this->attributes['sambaLockoutDuration'])) {
			$return['sambaDomain_lockoutDuration'][0] = '<block><key>' . _('Lockout duration') . '</key><value>' . implode(', ', $this->attributes['sambaLockoutDuration']) . '</value></block>';
		}
		if (isset($this->attributes['sambaLockoutObservationWindow'])) {
			$return['sambaDomain_lockoutObservationWindow'][0] = '<block><key>' . _('Reset time after lockout') . '</key><value>' . implode(', ', $this->attributes['sambaLockoutObservationWindow']) . '</value></block>';
		}
		return $return;
	}

}


?>
