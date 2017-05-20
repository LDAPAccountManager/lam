<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
  Copyright (C) 2007 - 2016  Roland Gruber

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
		$return['icon'] = 'keyBig.png';
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
		$day = array('-'); $mon = array('-'); $year = array('-');
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
		$profileOptionsExpire->addElement(new htmlSelect('shadowAccount_shadowExpire_day', $day, array('-')));
		$profileOptionsExpire->addElement(new htmlSelect('shadowAccount_shadowExpire_mon', $mon, array('-')));
		$profileOptionsExpire->addElement(new htmlSelect('shadowAccount_shadowExpire_yea', $year, array('-')));
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
			'shadowExpire' => _('Account expiration date'),
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
		// self service fields
		$return['selfServiceFieldSettings'] = array('shadowLastChange' => _('Last password change (read-only)'));
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
				$date = new DateTime('@' . $shAccExpirationDate*3600*24, new DateTimeZone('UTC'));
				$expirationDate = $date->format('d.m.Y');
			}
			$return->addElement(new htmlOutputText(_('Account expiration date')));
			$expireTable = new htmlTable();
			$expireTable->addElement(new htmlOutputText($expirationDate, false));
			$expireTable->addElement(new htmlAccountPageButton('shadowAccount', 'expire', 'open', 'edit.png', true, _('Change')));
			$return->addElement($expireTable);
			$return->addElement(new htmlHelpLink('shadowExpire'), true);

				$pwdChangeDate = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
				if (isset($this->attributes['shadowLastChange'][0])) {
					$shPwdChangeDate = $this->attributes['shadowLastChange'][0];
					$date = new DateTime('@' . $shPwdChangeDate*3600*24, new DateTimeZone('UTC'));
					$pwdChangeDate = $date->format('d.m.Y');
				}
				$return->addElement(new htmlOutputText(_('Last password change')));
				$pwdChangeTable = new htmlTable();
				$pwdChangeTable->addElement(new htmlOutputText($pwdChangeDate, false));
				$pwdChangeTable->addElement(new htmlAccountPageButton('shadowAccount', 'pwdChange', 'open', 'edit.png', true, _('Change')));
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
			if (isset($_POST['syncWindows']) && ($_POST['syncWindows'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('windowsUser')->setExpirationDate(
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
			if (isset($_POST['syncWindows']) && ($_POST['syncWindows'] == 'on')) {
				$this->getAccountContainer()->getAccountModule('windowsUser')->setExpirationDate(
					null, null, null);
			}
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
		$shAccExpirationDate = time()/(3600*24) + 365;
		if (isset($this->attributes['shadowExpire'][0])) {
			$shAccExpirationDate = $this->attributes['shadowExpire'][0];
		}
		$date = new DateTime('@' . $shAccExpirationDate*3600*24, new DateTimeZone('UTC'));
		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2050; $i++ ) $year[] = $i;
		$return->addElement(new htmlOutputText(_('Account expiration date')));
		$expTable = new htmlTable();
		$expTable->addElement(new htmlSelect('shadowExpire_day', $mday, array($date->format('j'))));
		$expTable->addElement(new htmlSelect('shadowExpire_mon', $mon, array($date->format('n'))));
		$expTable->addElement(new htmlSelect('shadowExpire_yea', $year, array($date->format('Y'))));
		$return->addElement($expTable);
		$return->addElement(new htmlHelpLink('shadowExpire'), true);
		if ($this->getAccountContainer()->getAccountModule('sambaSamAccount') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncSamba', false, _('Set also for Samba 3')), true);
		}
		if ($this->getAccountContainer()->getAccountModule('windowsUser') != null) {
			$return->addElement(new htmlTableExtendedInputCheckbox('syncWindows', false, _('Set also for Windows')), true);
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
	* Processes user input of the last password change page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_pwdChange() {
		$errors = array();
		// set last change date
		if (isset($_POST['form_subpage_shadowAccount_attributes_changePwdChange'])) {
			$this->setLastChangeDate($_POST['shadowLastChange_yea'], $_POST['shadowLastChange_mon'], $_POST['shadowLastChange_day']);
		}
		// remove last change date
		elseif (isset($_POST['form_subpage_shadowAccount_attributes_delPwdChange'])) {
			unset($this->attributes['shadowLastChange']);
		}
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page with the password change date.
	*
	* @return array meta HTML code
	*/
	function display_html_pwdChange() {
		$return = new htmlTable();
		$shLastChange = time()/(3600*24);
		if (isset($this->attributes['shadowLastChange'][0])) {
			$shLastChange = $this->attributes['shadowLastChange'][0];
		}
		$date = new DateTime('@' . $shLastChange*3600*24, new DateTimeZone('UTC'));
		for ( $i=1; $i<=31; $i++ ) $mday[] = $i;
		for ( $i=1; $i<=12; $i++ ) $mon[] = $i;
		for ( $i=2003; $i<=2050; $i++ ) $year[] = $i;
		$return->addElement(new htmlOutputText(_('Last password change')));
		$table = new htmlTable();
		$table->addElement(new htmlSelect('shadowLastChange_day', $mday, array($date->format('j'))));
		$table->addElement(new htmlSelect('shadowLastChange_mon', $mon, array($date->format('n'))));
		$table->addElement(new htmlSelect('shadowLastChange_yea', $year, array($date->format('Y'))));
		$return->addElement($table);
		$return->addElement(new htmlHelpLink('shadowLastChange'), true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$buttonTable = new htmlTable();
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'changePwdChange', _('Change')));
		if (isset($this->attributes['shadowLastChange'][0])) {
			$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'delPwdChange', _('Remove')));
		}
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'back', _('Cancel')));
		$buttonTable->colspan = 3;
		$return->addElement($buttonTable);
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$timeZone = getTimeZone();
		$shadowLastChange = '';
		if (!empty($this->attributes['shadowLastChange'][0])) {
			$time = new DateTime('@' . $this->attributes['shadowLastChange'][0]*24*3600, $timeZone);
			$shadowLastChange = $time->format('d.m.Y');
		}
		$shadowExpire = '';
		if (!empty($this->attributes['shadowExpire'][0])) {
			$time = new DateTime('@' . $this->attributes['shadowExpire'][0]*24*3600);
			$shadowExpire = $time->format('d.m.Y');
		}
		$return = array();
		$this->addPDFKeyValue($return, 'shadowLastChange', _('Last password change'), $shadowLastChange);
		$this->addPDFKeyValue($return, 'shadowExpire', _('Account expiration date'), $shadowExpire);
		$this->addSimplePDFField($return, 'shadowWarning', _('Password warning'));
		$this->addSimplePDFField($return, 'shadowInactive', _('Password expiration'));
		$this->addSimplePDFField($return, 'shadowMinAge', _('Minimum password age'), 'shadowMin');
		$this->addSimplePDFField($return, 'shadowMaxAge', _('Maximum password age'), 'shadowMax');
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("shadowAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "shadowAccount";
			// shadow last change
			$partialAccounts[$i]['shadowLastChange'] = array(intval(time()/3600/24));
			// password warning
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_warning', 'shadowWarning',
				'digit', $this->messages['shadowWarning'][1], $messages);
			// password expire ignoration
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_ignoreExpire', 'shadowInactive',
				'digit2', $this->messages['inactive'][1], $messages);
			// password minAge
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_minAge', 'shadowMin',
				'digit', $this->messages['shadowMin'][1], $messages);
			// password maxAge
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_maxAge', 'shadowMax',
				'digit', $this->messages['shadowMax'][1], $messages);
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
					$date = DateTime::createFromFormat('j.n.Y', $parts[0] . '.' . $parts[1] . '.' . $parts[2], new DateTimeZone('UTC'));
					$partialAccounts[$i]['shadowExpire'][] = intval($date->format('U')/3600/24);
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
		if (!empty($profile['shadowAccount_shadowExpire_day'][0])) {
			$day = $profile['shadowAccount_shadowExpire_day'][0];
			$mon = $profile['shadowAccount_shadowExpire_mon'][0];
			$year = $profile['shadowAccount_shadowExpire_yea'][0];
			if (!(($day == '-') && ($mon == '-') && ($year == '-'))) {
				$day = ($day == '-') ? 1 : $day;
				$mon = ($mon == '-') ? 1 : $mon;
				$year = ($year == '-') ? 2030 : $year;
				$this->setExpirationDate($year, $mon, $day);
			}
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
		$date = DateTime::createFromFormat('j.n.Y', $day . '.' . $month . '.' . $year, new DateTimeZone('UTC'));
		$this->attributes['shadowExpire'][0] = intval($date->format('U')/3600/24);
	}

	/**
	 * Sets the last password change date of this account.
	 * If all parameters are null the password change date will be removed.
	 *
	 * @param String $year year (e.g. 2040)
	 * @param String $month month (e.g. 8)
	 * @param String $day day (e.g. 27)
	 */
	public function setLastChangeDate($year, $month, $day) {
		if (($year == null) && ($month == null) && ($day == null)) {
			unset($this->attributes['shadowLastChange']);
			return;
		}
		$date = DateTime::createFromFormat('j.n.Y', $day . '.' . $month . '.' . $year, new DateTimeZone('UTC'));
		$this->attributes['shadowLastChange'][0] = intval($date->format('U')/3600/24);
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
		$return = array();
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		if (in_array('shadowLastChange', $fields)) {
			$shadowLastChange = '';
			if (isset($attributes['shadowLastChange'][0])) {
				$date = new DateTime('@' . $attributes['shadowLastChange'][0] * 3600 * 24, new DateTimeZone('UTC'));
				$shadowLastChange = $date->format('d.m.Y');
			}
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('shadowLastChange', _('Last password change'))));
			$row->addField(new htmlOutputText($shadowLastChange));
			$return['shadowLastChange'] = $row;
		}
		return $return;
	}

	/**
	 * Returns a list of jobs that can be run.
	 *
	 * @param LAMConfig $config configuration
	 * @return array list of jobs
	 */
	public function getSupportedJobs(&$config) {
		return array(
			new ShadowAccountPasswordNotifyJob(),
			new ShadowAccountExpirationCleanupJob()
		);
	}

}

if (interface_exists('\LAM\JOB\Job', false)) {

	include_once dirname(__FILE__) . '/../passwordExpirationJob.inc';

	/**
	 * Job to notify users about password expiration.
	 *
	 * @package jobs
	 */
	class ShadowAccountPasswordNotifyJob extends \LAM\JOB\PasswordExpirationJob {

		/**
		 * Returns the alias name of the job.
		 *
		 * @return String name
		 */
		public function getAlias() {
			return _('Shadow') . ': ' . _('Notify users about password expiration');
		}

		/**
		 * Searches for users in LDAP.
		 *
		 * @param String $jobID unique job identifier
		 * @param array $options config options (name => value)
		 * @return array list of user attributes
		 */
		protected function findUsers($jobID, $options) {
			// read users
			$sysattrs = array('mail', 'shadowLastChange', 'shadowWarning', 'shadowMax', 'userPassword');
			$attrs = $this->getAttrWildcards($jobID, $options);
			$attrs = array_values(array_unique(array_merge($attrs, $sysattrs)));
			$userResults = searchLDAPByFilter('(&(shadowLastChange=*)(shadowMax=*)(mail=*))', $attrs, array('user'));
			return $userResults;
		}

		/**
		 * Checks if a user needs to change his password.
		 *
		 * @param integer $jobID job ID
		 * @param array $options job settings
		 * @param PDO $pdo PDO
		 * @param DateTime $now current time
		 * @param array $policyOptions list of max age values (policy DN => maxAge)
		 * @param array $user user attributes
		 * @param boolean $isDryRun just do a dry run, nothing is modified
		 */
		protected function checkSingleUser($jobID, $options, &$pdo, $now, $policyOptions, $user, $isDryRun) {
			// skip if user is locked
			if (!empty($user['userpassword'][0]) && !pwd_is_enabled($user['userpassword'][0])) {
				logNewMessage(LOG_DEBUG, $user['dn'] . ' is locked.');
				return;
			}
			if ($user['shadowmax'][0] < 1) {
				logNewMessage(LOG_DEBUG, $user['dn'] . ' does not expire.');
				return;
			}


			// calculate time when password expires
			$lastPwdTimeUnix = $user['shadowlastchange'][0] * 3600 * 24;
			$lastPwdTime = new DateTime('@' . $lastPwdTimeUnix, new DateTimeZone('UTC'));
			logNewMessage(LOG_DEBUG, "Last password change on " . $lastPwdTime->format('Y-m-d'));
			$numDaysToWarn = $options[$this->getConfigPrefix() . '_mailNotificationPeriod' . $jobID][0];
			if (!empty($user['shadowwarning'][0]) && ($user['shadowwarning'][0] > 0)) {
				$numDaysToWarn += $user['shadowwarning'][0];
			}
			logNewMessage(LOG_DEBUG, "Number of days before warning " . $numDaysToWarn);
			$numDaysToExpire = $user['shadowmax'][0];
			$expireTime = $lastPwdTime->add(new DateInterval('P' . $numDaysToExpire . 'D'));
			logNewMessage(LOG_DEBUG, "Password expires on " . $expireTime->format('Y-m-d'));
			// skip already expired accounts
			if ($expireTime <= $now) {
				logNewMessage(LOG_DEBUG, $user['dn'] . ' already expired');
				return;
			}
			// calculate time of notification
			$notifyTime = clone $expireTime;
			$notifyTime->sub(new DateInterval('P' . $numDaysToWarn . 'D'));
			$notifyTime->setTimeZone(getTimeZone());
			logNewMessage(LOG_DEBUG, "Password notification on " . $notifyTime->format('Y-m-d H:i'));
			// skip if notification is in the future
			if ($notifyTime > $now) {
				logNewMessage(LOG_DEBUG, $user['dn'] . ' does not need notification yet.');
				return;
			}
			$dbLastChange = $this->getDBLastPwdChangeTime($jobID, $pdo, $user['dn']);
			// skip entries where mail was already sent
			if ($dbLastChange == $user['shadowlastchange'][0]) {
				logNewMessage(LOG_DEBUG, $user['dn'] . ' was already notified.');
				return;
			}
			if ($isDryRun) {
				// no action for dry run
				logNewMessage(LOG_NOTICE, 'Not sending email to ' . $user['dn'] . ' because of dry run.');
				return;
			}
			// send email
			$success = $this->sendMail($options, $jobID, $user, $expireTime);
			// update DB if mail was sent successfully
			if ($success) {
				$this->setDBLastPwdChangeTime($jobID, $pdo, $user['dn'], $user['shadowlastchange'][0]);
			}
		}

	}

	/**
	 * Job to delete or move users on account expiration.
	 *
	 * @package jobs
	 */
	class ShadowAccountExpirationCleanupJob extends \LAM\JOB\AccountExpirationCleanupJob {

		/**
		 * Returns the alias name of the job.
		 *
		 * @return String name
		 */
		public function getAlias() {
			return _('Shadow') .  ': ' . _('Cleanup expired user accounts');
		}

		/**
		 * Returns the description of the job.
		 *
		 * @return String description
		 */
		public function getDescription() {
			return _('This job deletes or moves user accounts when they expire.');
		}

		/**
		 * Searches for users in LDAP.
		 *
		 * @param String $jobID unique job identifier
		 * @param array $options config options (name => value)
		 * @return array list of user attributes
		 */
		protected function findUsers($jobID, $options) {
			// read users
			$attrs = array('shadowExpire');
			$userResults = searchLDAPByFilter('(shadowExpire=*)', $attrs, array('user'));
			return $userResults;
		}

		/**
		 * Checks if a user is expired.
		 *
		 * @param integer $jobID job ID
		 * @param array $options job settings
		 * @param PDO $pdo PDO
		 * @param DateTime $now current time
		 * @param array $policyOptions list of policy options by getPolicyOptions()
		 * @param array $user user attributes
		 * @param boolean $isDryRun just do a dry run, nothing is modified
		 */
		protected function checkSingleUser($jobID, $options, &$pdo, $now, $policyOptions, $user, $isDryRun) {
			$expireTimeUnix = $user['shadowexpire'][0] * 3600 * 24;
			$expireTime = new DateTime('@' . $expireTimeUnix, new DateTimeZone('UTC'));
			logNewMessage(LOG_DEBUG, "Expiration on " . $expireTime->format('Y-m-d'));
			$delay = 0;
			if (!empty($options[$this->getConfigPrefix() . '_delay' . $jobID][0])) {
				$delay = $options[$this->getConfigPrefix() . '_delay' . $jobID][0];
			}
			$actionTime = clone $expireTime;
			if ($delay != 0) {
				$actionTime->add(new DateInterval('P' . $delay . 'D'));
			}
			$actionTime->setTimeZone(getTimeZone());
			logNewMessage(LOG_DEBUG, "Action time on " . $actionTime->format('Y-m-d'));
			if ($actionTime <= $now) {
				$this->performAction($jobID, $options, $user, $isDryRun);
			}
		}

	}

}

?>
