<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
  Copyright (C) 2007 - 2012  Roland Gruber

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
* Manages Unix shadow accounts for users.
*
* @package modules
*
* @author Tilo Lutz
* @author Roland Gruber
* @author Michael Duergner
*/

/**
* Manages the object class "shadowAccount" for users.
*
* @package modules
*/
class shadowAccount extends baseModule implements passwordService {
	
	/**
	* Creates a new shadowAccount object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
	}

	/**
	* This function builds up the message array.
	*/
	function load_Messages() {
		// error messages for input checks
		$this->messages['shadowMin'][0] = array('ERROR', _('Minimum password age'), _('Password minimum age must be are natural number.'));
		$this->messages['shadowMin'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_minAge', _('Password minimum age must be are natural number.'));
		$this->messages['shadowMax'][0] = array('ERROR', _('Maximum password age'), _('Password maximum age must be are natural number.'));
		$this->messages['shadowMax'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_maxAge', _('Password maximum age must be are natural number.'));
		$this->messages['inactive'][0] = array('ERROR', _('Password expiration'), _('Password expiration must be are natural number or -1.'));
		$this->messages['inactive'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_ignoreExpire', _('Password expiration must be are natural number or -1.'));
		$this->messages['shadowWarning'][0] = array('ERROR', _('Password warning'), _('Password warning must be are natural number.'));
		$this->messages['shadowWarning'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_warning', _('Password warning must be are natural number.'));
		$this->messages['shadow_cmp'][0] = array('ERROR', _('Maximum password age'), _('Password maximum age must be bigger than password minimum age.'));
		$this->messages['shadow_cmp'][1] = array('ERROR',  _('Account %s:') . ' shadowAccount_min/maxAge', _('Password maximum age must be bigger as password minimum age.'));
		$this->messages['shadow_expireDate'][0] = array('ERROR',  _('Account %s:') . ' shadowAccount_expireDate', _('The expiration date is invalid.'));
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
		$return['icon'] = 'keyBig.png';
		// manages user accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _('Shadow');
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('shadowAccount');
		// managed attributes
		$return['attributes'] = array('shadowLastChange', 'shadowMin', 'shadowMax', 'shadowWarning',
			'shadowInactive', 'shadowExpire', 'shadowFlag');
		// lists for expiration date
		$day = array(); $mon = array(); $year = array();
		for ( $i=1; $i<=31; $i++ ) $day[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2030; $i++ ) $year[] = $i;
		$profileOptionsTable = new htmlTable();
		// auto add extension
		$profileOptionsTable->addElement(new htmlTableExtendedInputCheckbox('shadowAccount_addExt', false, _('Automatically add this extension'), 'autoAdd'), true);
		// password warning
		$profilePwdWarning = new htmlTableExtendedInputField(_('Password warning'), 'shadowAccount_shadowWarning', null, 'shadowWarning');
		$profilePwdWarning->setFieldSize(5);
		$profilePwdWarning->setFieldMaxLength(4);
		$profileOptionsTable->addElement($profilePwdWarning, true);
		// password expiration
		$profilePwdExpiration = new htmlTableExtendedInputField(_('Password expiration'), 'shadowAccount_shadowInactive', null, 'shadowInactive');
		$profilePwdExpiration->setFieldSize(5);
		$profilePwdExpiration->setFieldMaxLength(4);
		$profileOptionsTable->addElement($profilePwdExpiration, true);
		// minimum password age
		$profilePwdMinAge = new htmlTableExtendedInputField(_('Minimum password age'), 'shadowAccount_shadowMin', null, 'shadowMin');
		$profilePwdMinAge->setFieldSize(5);
		$profilePwdMinAge->setFieldMaxLength(5);
		$profileOptionsTable->addElement($profilePwdMinAge, true);
		// maximum password age
		$profilePwdMinAge = new htmlTableExtendedInputField(_('Maximum password age'), 'shadowAccount_shadowMax', null, 'shadowMax');
		$profilePwdMinAge->setFieldSize(5);
		$profilePwdMinAge->setFieldMaxLength(5);
		$profileOptionsTable->addElement($profilePwdMinAge, true);
		// expiration date
		$profileOptionsTable->addElement(new htmlOutputText(_('Account expiration date')));
		$profileOptionsExpire = new htmlTable();
		$profileOptionsExpire->addElement(new htmlSelect('shadowAccount_shadowExpire_day', $day, array('1')));
		$profileOptionsExpire->addElement(new htmlSelect('shadowAccount_shadowExpire_mon', $mon, array('1')));
		$profileOptionsExpire->addElement(new htmlSelect('shadowAccount_shadowExpire_yea', $year, array('2030')));
		$profileOptionsTable->addElement($profileOptionsExpire);
		$profileOptionsTable->addElement(new htmlHelpLink('shadowExpire'));
		$return['profile_options'] = $profileOptionsTable;
		// profile checks
		$return['profile_checks']['shadowAccount_shadowMin'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowMin'][0]);
		$return['profile_checks']['shadowAccount_shadowMax'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowMax'][0]);
		$return['profile_checks']['shadowAccount_cmp'] = array(
			'type' => 'int_greater',
			'cmp_name1' => 'shadowAccount_shadowMax',
			'cmp_name2' => 'shadowAccount_shadowMin',
			'error_message' => $this->messages['shadow_cmp'][0]);
		$return['profile_checks']['shadowAccount_shadowInactive'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit2',
			'error_message' => $this->messages['inactive'][0]);
		$return['profile_checks']['shadowAccount_shadowWarning'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowWarning'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'shadowAccount_shadowWarning' => 'shadowWarning',
			'shadowAccount_shadowInactive' => 'shadowInactive',
			'shadowAccount_shadowMin' => 'shadowMin',
			'shadowAccount_shadowMax' => 'shadowMax'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'shadowLastChange' => _('Last password change'),
			'shadowWarning' => _('Password warning'),
			'shadowInactive' => _('Account inactive'),
			'shadowExpire' => _('Password expiration'),
			'shadowMinAge' => _('Minimum password age'),
			'shadowMaxAge' => _('Maximum password age'),
		);
		// help Entries
		$return['help'] = array (
			'shadowWarning' => array (
				"Headline" => _("Password warning"), 'attr' => 'shadowWarning',
				"Text" => _("Days before password is to expire that user is warned of pending password expiration. If set value must be >0."). ' '. _("Can be left empty.")
			),
			'shadowInactive' => array (
				"Headline" => _("Password expiration"), 'attr' => 'shadowInactive',
				"Text" => _("Number of days a user can login even his password has expired. -1=always."). ' '. _("Can be left empty.")
			),
			'shadowMin' => array (
				"Headline" => _("Minimum password age"), 'attr' => 'shadowMin',
				"Text" => _("Number of days a user has to wait until he is allowed to change his password again. If set value must be >0."). ' '. _("Can be left empty.")
			),
			'shadowMax' => array (
				"Headline" => _("Maximum password age"), 'attr' => 'shadowMax',
				"Text" => _("Number of days after a user has to change his password again. If set value must be >0."). ' '. _("Can be left empty.")
			),
			'shadowExpire' => array (
				"Headline" => _("Account expiration date"), 'attr' => 'shadowExpire',
				"Text" => _("This is the date when the account will expire. Format: DD-MM-YYYY")
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
			'shadowLastChange' => array(
				"Headline" => _("Last password change"), 'attr' => 'shadowLastChange',
				"Text" => _("This is the date when the user changed his password. If you specify a maximum password age then you can force a password change here.")
			)
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'shadowAccount_warning',
				'description' => _('Password warning'),
				'help' => 'shadowWarning',
				'example' => '14'
			),
			array(
				'name' => 'shadowAccount_ignoreExpire',
				'description' => _('Password expiration'),
				'help' => 'shadowInactive',
				'example' => '7'
			),
			array(
				'name' => 'shadowAccount_minAge',
				'description' => _('Minimum password age'),
				'help' => 'shadowMin',
				'example' => '1'
			),
			array(
				'name' => 'shadowAccount_maxAge',
				'description' => _('Maximum password age'),
				'help' => 'shadowMax',
				'example' => '365'
			),
			array(
				'name' => 'shadowAccount_expireDate',
				'description' => _('Account expiration date'),
				'help' => 'shadowExpire',
				'example' => '17-07-2011'
			)
		);
		return $return;
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
		if (!in_array('shadowAccount', $this->attributes['objectClass']) && !in_array('shadowAccount', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_attributes() {
		if (isset($_POST['form_subpage_shadowAccount_attributes_remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('shadowAccount'), $this->attributes['objectClass']);
			if (isset($this->attributes['shadowMin'])) unset($this->attributes['shadowMin']);
			if (isset($this->attributes['shadowMax'])) unset($this->attributes['shadowMax']);
			if (isset($this->attributes['shadowWarning'])) unset($this->attributes['shadowWarning']);
			if (isset($this->attributes['shadowInactive'])) unset($this->attributes['shadowInactive']);
			if (isset($this->attributes['shadowLastChange'])) unset($this->attributes['shadowLastChange']);
			if (isset($this->attributes['shadowExpire'])) unset($this->attributes['shadowExpire']);
			if (isset($this->attributes['shadowFlag'])) unset($this->attributes['shadowFlag']);
			return array();
		}
		if (!in_array('shadowAccount', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		// Load attributes
		$this->attributes['shadowMin'][0] = $_POST['shadowMin'];
		$this->attributes['shadowMax'][0] = $_POST['shadowMax'];
		$this->attributes['shadowWarning'][0] = $_POST['shadowWarning'];
		$this->attributes['shadowInactive'][0] = $_POST['shadowInactive'];
		if ( !get_preg($this->attributes['shadowMin'][0], 'digit'))  $errors[] = $this->messages['shadowMin'][0];
		if ( !get_preg($this->attributes['shadowMax'][0], 'digit')) $errors[] = $this->messages['shadowMax'][0];
		if ( $this->attributes['shadowMin'][0] > $this->attributes['shadowMax'][0]) $errors[] = $this->messages['shadow_cmp'][0];
		if ( !get_preg($this->attributes['shadowInactive'][0], 'digit2')) $errors[] = $this->messages['inactive'][0];
		if ( !get_preg($this->attributes['shadowWarning'][0], 'digit')) $errors[] = $this->messages['shadowWarning'][0];
		if (isset($_POST['form_subpage_shadowAccount_attributes_expirePassword']) && isset($this->attributes['shadowMax'][0]) && ($this->attributes['shadowMax'][0] != 0)) {
			$this->attributes['shadowLastChange'][0] = intval(time()/3600/24) - $this->attributes['shadowMax'][0] - 1;
		}
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page with all attributes.
	*
	* @return array meta HTML code
	*/
	function display_html_attributes() {
		if (isset($_POST['form_subpage_shadowAccount_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'shadowAccount';
		}
		$return = new htmlTable();
		if (in_array('shadowAccount', $this->attributes['objectClass'])) {
			$shWarning = '';
			if (isset($this->attributes['shadowWarning'][0])) {
				$shWarning = $this->attributes['shadowWarning'][0];
			}
			$pwdWarnInput = new htmlTableExtendedInputField(_('Password warning'), 'shadowWarning', $shWarning, 'shadowWarning');
			$pwdWarnInput->setFieldMaxLength(4);
			$pwdWarnInput->setFieldSize(5);
			$pwdWarnInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$return->addElement($pwdWarnInput, true);
			
			$shPwdExpiration = '';
			if (isset($this->attributes['shadowInactive'][0])) $shPwdExpiration = $this->attributes['shadowInactive'][0];
			$pwdExpInput = new htmlTableExtendedInputField(_('Password expiration'), 'shadowInactive', $shPwdExpiration, 'shadowInactive');
			$pwdExpInput->setFieldMaxLength(4);
			$pwdExpInput->setFieldSize(5);
			$pwdExpInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$return->addElement($pwdExpInput, true);
			
			$shMinAge = '';
			if (isset($this->attributes['shadowMin'][0])) $shMinAge = $this->attributes['shadowMin'][0];
			$minAgeInput = new htmlTableExtendedInputField(_('Minimum password age'), 'shadowMin', $shMinAge, 'shadowMin');
			$minAgeInput->setFieldMaxLength(5);
			$minAgeInput->setFieldSize(5);
			$minAgeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$return->addElement($minAgeInput, true);
			
			$shMaxAge = '';
			if (isset($this->attributes['shadowMax'][0])) $shMaxAge = $this->attributes['shadowMax'][0];
			$maxAgeInput = new htmlTableExtendedInputField(_('Maximum password age'), 'shadowMax', $shMaxAge, 'shadowMax');
			$maxAgeInput->setFieldMaxLength(5);
			$maxAgeInput->setFieldSize(5);
			$maxAgeInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			$return->addElement($maxAgeInput, true);
			
			$expirationDate = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
			if (isset($this->attributes['shadowExpire'][0])) {
				$shAccExpirationDate = $this->attributes['shadowExpire'][0];
				$date = getdate($shAccExpirationDate*3600*24);
				$expirationDate = $date['mday'] . "." . $date['mon'] . "." . $date['year'];
			}
			$return->addElement(new htmlOutputText(_('Account expiration date')));
			$expireTable = new htmlTable();
			$expireTable->addElement(new htmlOutputText($expirationDate, false));
			$expireTable->addElement(new htmlAccountPageButton('shadowAccount', 'expire', 'open', _('Change')));
			$return->addElement($expireTable);
			$return->addElement(new htmlHelpLink('shadowExpire'), true);
			
				$pwdChangeDate = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
				if (isset($this->attributes['shadowLastChange'][0])) {
					$shPwdChangeDate = $this->attributes['shadowLastChange'][0];
					$date = getdate($shPwdChangeDate*3600*24);
					$pwdChangeDate = $date['mday'] . "." . $date['mon'] . "." . $date['year'];
				}
				$return->addElement(new htmlOutputText(_('Last password change')));
				$pwdChangeTable = new htmlTable();
				$pwdChangeTable->addElement(new htmlOutputText($pwdChangeDate, false));
				if (isset($this->attributes['shadowMax'][0]) && ($this->attributes['shadowMax'][0] != '')) {
					$pwdChangeTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'expirePassword', _('Force password change')));
				}
				$return->addElement($pwdChangeTable);
				$return->addElement(new htmlHelpLink('shadowLastChange'), true);
			
			$return->addElement(new htmlOutputText(''), true);
			$remButton = new htmlAccountPageButton('shadowAccount', 'attributes', 'remObjectClass', _('Remove Shadow account extension'));
			$remButton->colspan = 4;
			$return->addElement($remButton);
		}
		else {
			$return->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'addObjectClass', _('Add Shadow account extension')));
		}
		return $return;
	}

	/**
	* Processes user input of the expiration page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_expire() {
		$errors = array();
		// set expiration date
		if (isset($_POST['form_subpage_shadowAccount_attributes_change'])) {
			$this->setExpirationDate($_POST['shadowExpire_yea'], $_POST['shadowExpire_mon'], $_POST['shadowExpire_day']);
			// sync other modules
			if (isset($_POST['syncSamba']) && ($_POST['syncSamba'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('sambaSamAccount')->setExpirationDate(
					$_POST['shadowExpire_yea'], $_POST['shadowExpire_mon'], $_POST['shadowExpire_day']);
			}
			if (isset($_POST['syncHeimdal']) && ($_POST['syncHeimdal'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('heimdalKerberos')->setExpirationDate(
					$_POST['shadowExpire_yea'], $_POST['shadowExpire_mon'], $_POST['shadowExpire_day']);
			}
			if (isset($_POST['syncMIT']) && ($_POST['syncMIT'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberos')->setExpirationDate(
					$_POST['shadowExpire_yea'], $_POST['shadowExpire_mon'], $_POST['shadowExpire_day']);
			}
			if (isset($_POST['syncMITStructural']) && ($_POST['syncMITStructural'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberosStructural')->setExpirationDate(
					$_POST['shadowExpire_yea'], $_POST['shadowExpire_mon'], $_POST['shadowExpire_day']);
			}
		}
		// remove expiration date
		elseif (isset($_POST['form_subpage_shadowAccount_attributes_del'])) {
			unset($this->attributes['shadowExpire']);
			// sync other modules
			if (isset($_POST['syncSamba']) && ($_POST['syncSamba'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('sambaSamAccount')->setExpirationDate(
					null, null, null);
			}
			if (isset($_POST['syncHeimdal']) && ($_POST['syncHeimdal'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('heimdalKerberos')->setExpirationDate(
					null, null, null);
			}
			if (isset($_POST['syncMIT']) && ($_POST['syncMIT'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberos')->setExpirationDate(
					null, null, null);
			}
			if (isset($_POST['syncMITStructural']) && ($_POST['syncMITStructural'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('mitKerberosStructural')->setExpirationDate(
					null, null, null);
			}
		}
		return $errors;		
	}
	
	/**
	* This function will create the meta HTML code to show a page with the expiration date.
	*
	* @return array meta HTML code
	*/
	function display_html_expire() {
		$return = new htmlTable();
		$shAccExpirationDate = 0;
		if (isset($this->attributes['shadowExpire'][0])) {
			$shAccExpirationDate = $this->attributes['shadowExpire'][0];
		}
		$date = getdate($shAccExpirationDate*3600*24);
		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2050; $i++ ) $year[] = $i;
		$return->addElement(new htmlOutputText(_('Account expiration date')));
		$expTable = new htmlTable();
		$expTable->addElement(new htmlSelect('shadowExpire_day', $mday, array($date['mday'])));
		$expTable->addElement(new htmlSelect('shadowExpire_mon', $mon, array($date['mon'])));
		$expTable->addElement(new htmlSelect('shadowExpire_yea', $year, array($date['year'])));
		$return->addElement($expTable);
		$return->addElement(new htmlHelpLink('shadowExpire'), true);
		if ($this->getAccountContainer()->getAccountModule('sambaSamAccount') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncSamba', false, _('Set also for Samba 3')), true);
		}
		if ($this->getAccountContainer()->getAccountModule('heimdalKerberos') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncHeimdal', false, _('Set also for Kerberos')), true);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberos') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncMIT', false, _('Set also for Kerberos')), true);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberosStructural') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncMITStructural', false, _('Set also for Kerberos')), true);
		}
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$buttonTable = new htmlTable();
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'change', _('Change')));
		if (isset($this->attributes['shadowExpire'][0])) {
			$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'del', _('Remove')));
		}
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'back', _('Cancel')));
		$buttonTable->colspan=3;
		$return->addElement($buttonTable);
		return $return;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$shadowLastChange = '';
		if (isset($this->attributes['shadowLastChange'][0])) {
			$shadowLastChange = date('d. m. Y',$this->attributes['shadowLastChange'][0]*24*3600);
		}
		$shadowExpire = '';
		if (isset($this->attributes['shadowExpire'][0])) {
			$shadowExpire = date('d. m. Y',$this->attributes['shadowExpire'][0]*24*3600);
		}
		$return = array('shadowAccount_shadowLastChange' => array('<block><key>' . _('Last password change') . '</key><value>' . $shadowLastChange . '</value></block>'),
			'shadowAccount_shadowExpire' => array('<block><key>' . _('Account expiration date') . '</key><value>' . $shadowExpire . '</value></block>'),
		);
		$this->addSimplePDFField($return, 'shadowWarning', _('Password warning'));
		$this->addSimplePDFField($return, 'shadowInactive', _('Password expiration'));
		$this->addSimplePDFField($return, 'shadowMinAge', _('Minimum password age'), 'shadowMin');
		$this->addSimplePDFField($return, 'shadowMaxAge', _('Maximum password age'), 'shadowMax');
		return $return;
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
			// add object class
			if (!in_array("shadowAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "shadowAccount";
			// shadow last change
			$partialAccounts[$i]['shadowLastChange'] = array(intval(time()/3600/24));
			// password warning
			if ($rawAccounts[$i][$ids['shadowAccount_warning']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_warning']], 'digit')) {
					$partialAccounts[$i]['shadowWarning'][] = $rawAccounts[$i][$ids['shadowAccount_warning']];
				}
				else {
					$errMsg = $this->messages['shadowWarning'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// password expire ignoration
			if ($rawAccounts[$i][$ids['shadowAccount_ignoreExpire']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_ignoreExpire']], 'digit2')) {
					$partialAccounts[$i]['shadowInactive'][] = $rawAccounts[$i][$ids['shadowAccount_ignoreExpire']];
				}
				else {
					$errMsg = $this->messages['inactive'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// password minAge
			if ($rawAccounts[$i][$ids['shadowAccount_minAge']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_minAge']], 'digit')) {
					$partialAccounts[$i]['shadowMin'][] = $rawAccounts[$i][$ids['shadowAccount_minAge']];
				}
				else {
					$errMsg = $this->messages['shadowMin'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// password maxAge
			if ($rawAccounts[$i][$ids['shadowAccount_maxAge']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_maxAge']], 'digit')) {
					$partialAccounts[$i]['shadowMax'][] = $rawAccounts[$i][$ids['shadowAccount_maxAge']];
				}
				else {
					$errMsg = $this->messages['shadowMax'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// minAge <= maxAge
			if ((($rawAccounts[$i][$ids['shadowAccount_minAge']] != '') || ($rawAccounts[$i][$ids['shadowAccount_maxAge']] != '')) &&  // if at least one is set
			(($rawAccounts[$i][$ids['shadowAccount_minAge']] == '') || ($rawAccounts[$i][$ids['shadowAccount_maxAge']] == '') || (  // and one is not set
			($rawAccounts[$i][$ids['shadowAccount_minAge']] > $rawAccounts[$i][$ids['shadowAccount_maxAge']])))) {  // or minAge > maxAge
				$errMsg = $this->messages['shadow_cmp'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// expiration date
			if ($rawAccounts[$i][$ids['shadowAccount_expireDate']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_expireDate']], 'date')) {
					$parts = explode('-', $rawAccounts[$i][$ids['shadowAccount_expireDate']]);
					$partialAccounts[$i]['shadowExpire'][] = intval(mktime(0, 0, 0, intval($parts[1]), intval($parts[0]), intval($parts[2]))/3600/24);
				}
				else {
					$errMsg = $this->messages['shadow_expireDate'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
		}
		return $messages;
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
		if (isset($profile['shadowAccount_addExt'][0]) && ($profile['shadowAccount_addExt'][0] == "true")) {
			if (!in_array('shadowAccount', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'shadowAccount';
			}
		}
		// expiration date
		if (isset($profile['shadowAccount_shadowExpire_day'][0]) && ($profile['shadowAccount_shadowExpire_day'][0] != "")) {
			$date = intval(mktime(0, 0, 0, intval($profile['shadowAccount_shadowExpire_mon'][0]),
				intval($profile['shadowAccount_shadowExpire_day'][0]), intval($profile['shadowAccount_shadowExpire_yea'][0]))/3600/24);
			$this->attributes['shadowExpire'][0] = $date;
		}
	}


	/**
	 * This method specifies if a module manages password attributes.
	 * @see passwordService::managesPasswordAttributes
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes() {
		// only listen to password changes
		return false;
	}

	/**
	 * Specifies if this module supports to force that a user must change his password on next login.
	 * 
	 * @return boolean force password change supported
	 */
	public function supportsForcePasswordChange() {
		return true;
	}
	
	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @param boolean $forcePasswordChange force the user to change his password at next login
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		// update password timestamp when Unix password was updated
		if (!in_array('posixAccount', $modules)) {
			return array();
		}
		if (in_array_ignore_case('shadowAccount', $this->attributes['objectClass'])) {
			$this->attributes['shadowLastChange'][0] = intval(time()/3600/24);
			if ($forcePasswordChange && isset($this->attributes['shadowMax'][0]) && ($this->attributes['shadowMax'][0] != 0)) {
				$this->attributes['shadowLastChange'][0] = intval(time()/3600/24) - $this->attributes['shadowMax'][0] - 1;
			}
		}
		return array();
	}

	/**
	 * Sets the expiration date of this account.
	 * If all parameters are null the expiration date will be removed.
	 *
	 * @param String $year year (e.g. 2040)
	 * @param String $month month (e.g. 8)
	 * @param String $day day (e.g. 27)
	 */
	public function setExpirationDate($year, $month, $day) {
		if (($year == null) && ($month == null) && ($day == null)) {
			unset($this->attributes['shadowExpire']);
			return;
		}
		$this->attributes['shadowExpire'][0] = intval(gmmktime(0, 0, 0, intval($month), intval($day),
			intval($year))/3600/24);
	}
		
}

?>
