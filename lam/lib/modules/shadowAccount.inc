<?php

use function LAM\ACCOUNTLIST\isPasswordChangeByDefault;
use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
  Copyright (C) 2007 - 2023  Roland Gruber

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
class shadowAccount extends baseModule implements passwordService,AccountStatusProvider {

	/**
	 * These attributes will be ignored by default if a new account is copied from an existing one.
	 */
	const ATTRIBUTES_TO_IGNORE_ON_COPY = array('shadowlastchange', 'shadowexpire');

	/**
	 * ID for expired account status.
	 */
	const STATUS_ACCOUNT_EXPIRED = "SHADOW_ACCOUNT_EXPIRED";

	/**
	 * ID for expired password status.
	 */
	const STATUS_PASSWORD_EXPIRED = "SHADOW_PASSWORD_EXPIRED";

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
		$this->messages['shadowmin'][0] = array('ERROR', _('Minimum password age'), _('Password minimum age must be are natural number.'));
		$this->messages['shadowmin'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_minAge', _('Password minimum age must be are natural number.'));
		$this->messages['shadowmax'][0] = array('ERROR', _('Maximum password age'), _('Password maximum age must be are natural number.'));
		$this->messages['shadowmax'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_maxAge', _('Password maximum age must be are natural number.'));
		$this->messages['inactive'][0] = array('ERROR', _('Password expiration'), _('Password expiration must be are natural number or -1.'));
		$this->messages['inactive'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_ignoreExpire', _('Password expiration must be are natural number or -1.'));
		$this->messages['shadowwarning'][0] = array('ERROR', _('Password warning'), _('Password warning must be are natural number.'));
		$this->messages['shadowwarning'][1] = array('ERROR', _('Account %s:') . ' shadowAccount_warning', _('Password warning must be are natural number.'));
		$this->messages['shadow_cmp'][0] = array('ERROR', _('Maximum password age'), _('Password maximum age must be bigger than password minimum age.'));
		$this->messages['shadow_cmp'][1] = array('ERROR',  _('Account %s:') . ' shadowAccount_min/maxAge', _('Password maximum age must be bigger as password minimum age.'));
		$this->messages['shadow_expireDate'][0] = array('ERROR',  _('Account %s:') . ' shadowAccount_expireDate', _('The expiration date is invalid.'));
		$this->messages['shadow_lastChange'][0] = array('ERROR',  _('Account %s:') . ' shadowAccount_lastChange', _('The last password change date is invalid.'));
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
		$return = [];
		// icon
		$return['icon'] = 'key.svg';
		// alias name
		$return["alias"] = _('Shadow');
		// module dependencies
		$return['dependencies'] = array('depends' => array('posixAccount'), 'conflicts' => []);
		// managed object classes
		$return['objectClasses'] = array('shadowAccount');
		// managed attributes
		$return['attributes'] = array('shadowlastchange', 'shadowmin', 'shadowmax', 'shadowwarning',
			'shadowinactive', 'shadowexpire', 'shadowflag');
		// lists for expiration date
		$day = array('-'); $mon = array('-'); $year = array('-');
		for ( $i=1; $i<=31; $i++ ) {
			$day[] = $i;
		}
		for ( $i=1; $i<=12; $i++ ) {
			$mon[] = $i;
		}
		for ( $i=2003; $i<=2030; $i++ ) {
			$year[] = $i;
		}
		$profileOptionsTable = new htmlResponsiveRow();
		// auto add extension
		$profileOptionsTable->add(new htmlResponsiveInputCheckbox('shadowAccount_addExt', false, _('Automatically add this extension'), 'autoAdd'), 12);
		// password warning
		$profilePwdWarning = new htmlResponsiveInputField(_('Password warning'), 'shadowAccount_shadowWarning', null, 'shadowwarning');
		$profilePwdWarning->setFieldMaxLength(4);
		$profileOptionsTable->add($profilePwdWarning, 12);
		// password expiration
		$profilePwdExpiration = new htmlResponsiveInputField(_('Password expiration'), 'shadowAccount_shadowInactive', null, 'shadowinactive');
		$profilePwdExpiration->setFieldMaxLength(4);
		$profileOptionsTable->add($profilePwdExpiration, 12);
		// minimum password age
		$profilePwdMinAge = new htmlResponsiveInputField(_('Minimum password age'), 'shadowAccount_shadowMin', null, 'shadowmin');
		$profilePwdMinAge->setFieldMaxLength(5);
		$profileOptionsTable->add($profilePwdMinAge, 12);
		// maximum password age
		$profilePwdMinAge = new htmlResponsiveInputField(_('Maximum password age'), 'shadowAccount_shadowMax', null, 'shadowmax');
		$profilePwdMinAge->setFieldMaxLength(5);
		$profileOptionsTable->add($profilePwdMinAge, 12);
		// expiration date
		$profileOptionsTable->addLabel(new htmlOutputText(_('Account expiration date')));
		$profileOptionsExpire = new htmlResponsiveRow();
		$profileOptionsExpire->add(new htmlSelect('shadowAccount_shadowExpire_day', $day, array('-')), 2, 2, 2, 'padding-right05');
		$profileOptionsExpire->add(new htmlSelect('shadowAccount_shadowExpire_mon', $mon, array('-')), 2, 2, 2, 'padding-left-right05');
		$profileOptionsExpire->add(new htmlSelect('shadowAccount_shadowExpire_yea', $year, array('-')), 6, 6, 6, 'padding-left-right05');
		$profileOptionsExpire->add(new htmlHelpLink('shadowexpire'), 2, 2, 2, 'padding-left-right05');
		$profileOptionsTable->addField($profileOptionsExpire);
		$return['profile_options'] = $profileOptionsTable;
		// profile checks
		$return['profile_checks']['shadowAccount_shadowMin'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowmin'][0]);
		$return['profile_checks']['shadowAccount_shadowMax'] = array(
			'type' => 'ext_preg',
			'regex' => 'digit',
			'error_message' => $this->messages['shadowmax'][0]);
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
			'error_message' => $this->messages['shadowwarning'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'shadowAccount_shadowWarning' => 'shadowwarning',
			'shadowAccount_shadowInactive' => 'shadowinactive',
			'shadowAccount_shadowMin' => 'shadowmin',
			'shadowAccount_shadowMax' => 'shadowmax'
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'shadowLastChange' => _('Last password change'),
			'shadowWarning' => _('Password warning'),
			'shadowInactive' => _('Password expiration'),
			'shadowExpire' => _('Account expiration date'),
			'shadowMinAge' => _('Minimum password age'),
			'shadowMaxAge' => _('Maximum password age'),
		);
		// help Entries
		$return['help'] = array (
			'shadowwarning' => array (
				"Headline" => _("Password warning"), 'attr' => 'shadowWarning',
				"Text" => _("Days before password is to expire that user is warned of pending password expiration. If set value must be >0."). ' '. _("Can be left empty.")
			),
			'shadowinactive' => array (
				"Headline" => _("Password expiration"), 'attr' => 'shadowInactive',
				"Text" => _("Number of days a user can login even his password has expired. -1=always."). ' '. _("Can be left empty.")
			),
			'shadowmin' => array (
				"Headline" => _("Minimum password age"), 'attr' => 'shadowMin',
				"Text" => _("Number of days a user has to wait until he is allowed to change his password again. If set value must be >0."). ' '. _("Can be left empty.")
			),
			'shadowmax' => array (
				"Headline" => _("Maximum password age"), 'attr' => 'shadowMax',
				"Text" => _("Number of days after a user has to change his password again. If set value must be >0."). ' '. _("Can be left empty.")
			),
			'shadowexpire' => array (
				"Headline" => _("Account expiration date"), 'attr' => 'shadowExpire',
				"Text" => _("This is the date when the account will expire. Format: DD-MM-YYYY")
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			),
			'shadowlastchange' => array(
				"Headline" => _("Last password change"), 'attr' => 'shadowLastChange',
				"Text" => _("This is the date when the user changed his password. If you specify a maximum password age then you can force a password change here.")
			),
			'cronAddWarningPeriod' => array(
				"Headline" => _('Add expiration warning time'), 'attr' => 'shadowWarning',
				"Text" => _('This will add the warning period for password expiration on top.')
			)
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'shadowAccount_warning',
				'description' => _('Password warning'),
				'help' => 'shadowwarning',
				'example' => '14'
			),
			array(
				'name' => 'shadowAccount_ignoreExpire',
				'description' => _('Password expiration'),
				'help' => 'shadowinactive',
				'example' => '7'
			),
			array(
				'name' => 'shadowAccount_minAge',
				'description' => _('Minimum password age'),
				'help' => 'shadowmin',
				'example' => '1'
			),
			array(
				'name' => 'shadowAccount_maxAge',
				'description' => _('Maximum password age'),
				'help' => 'shadowmax',
				'example' => '365'
			),
			array(
				'name' => 'shadowAccount_expireDate',
				'description' => _('Account expiration date'),
				'help' => 'shadowexpire',
				'example' => '17-07-2011'
			),
			array(
				'name' => 'shadowAccount_lastChange',
				'description' => _('Last password change'),
				'help' => 'shadowlastchange',
				'example' => '17-07-2011'
			)
		);
		// self service fields
		$return['selfServiceFieldSettings'] = array(
			'shadowLastChange' => _('Last password change (read-only)'),
			'shadowExpire' => _('Account expiration date (read-only)')
		);
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function loadAttributesFromAccountCopy(array $ldapAttributes, array $attributesToIgnore = []) : void {
		$attributesToIgnore = array_merge(baseModule::ATTRIBUTES_TO_IGNORE_ON_COPY_DEFAULT, self::ATTRIBUTES_TO_IGNORE_ON_COPY);
		parent::loadAttributesFromAccountCopy($ldapAttributes, $attributesToIgnore);
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
			return [];
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
			if (isset($this->attributes['shadowmin'])) {
				unset($this->attributes['shadowmin']);
			}
			if (isset($this->attributes['shadowmax'])) {
				unset($this->attributes['shadowmax']);
			}
			if (isset($this->attributes['shadowwarning'])) {
				unset($this->attributes['shadowwarning']);
			}
			if (isset($this->attributes['shadowinactive'])) {
				unset($this->attributes['shadowinactive']);
			}
			if (isset($this->attributes['shadowlastchange'])) {
				unset($this->attributes['shadowlastchange']);
			}
			if (isset($this->attributes['shadowexpire'])) {
				unset($this->attributes['shadowexpire']);
			}
			if (isset($this->attributes['shadowflag'])) {
				unset($this->attributes['shadowflag']);
			}
			return [];
		}
		if (!in_array('shadowAccount', $this->attributes['objectClass'])) {
			return [];
		}
		$errors = [];
		// Load attributes
		$this->attributes['shadowmin'][0] = $_POST['shadowmin'];
		$this->attributes['shadowmax'][0] = $_POST['shadowmax'];
		$this->attributes['shadowwarning'][0] = $_POST['shadowwarning'];
		$this->attributes['shadowinactive'][0] = $_POST['shadowinactive'];
		if ( !get_preg($this->attributes['shadowmin'][0], 'digit'))  {
			$errors[] = $this->messages['shadowmin'][0];
		}
		if ( !get_preg($this->attributes['shadowmax'][0], 'digit')) {
			$errors[] = $this->messages['shadowmax'][0];
		}
		if ( $this->attributes['shadowmin'][0] > $this->attributes['shadowmax'][0]) {
			$errors[] = $this->messages['shadow_cmp'][0];
		}
		if ( !get_preg($this->attributes['shadowinactive'][0], 'digit2')) {
			$errors[] = $this->messages['inactive'][0];
		}
		if ( !get_preg($this->attributes['shadowwarning'][0], 'digit')) {
			$errors[] = $this->messages['shadowwarning'][0];
		}
		if (isset($_POST['form_subpage_shadowAccount_attributes_expirePassword']) && isset($this->attributes['shadowmax'][0]) && ($this->attributes['shadowmax'][0] != 0)) {
			$this->attributes['shadowlastchange'][0] = intval(time()/3600/24) - $this->attributes['shadowmax'][0] - 1;
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
		$return = new htmlResponsiveRow();
		if (in_array('shadowAccount', $this->attributes['objectClass'])) {
			$pwdWarnInput = $this->addSimpleInputTextField($return, 'shadowwarning', _('Password warning'));
			$pwdWarnInput->setType('number');

			$pwdExpInput = $this->addSimpleInputTextField($return, 'shadowinactive', _('Password expiration'));
			$pwdExpInput->setType('number');

			$minAgeInput = $this->addSimpleInputTextField($return, 'shadowmin', _('Minimum password age'));
			$minAgeInput->setType('number');

			$maxAgeInput = $this->addSimpleInputTextField($return, 'shadowmax', _('Maximum password age'));
			$maxAgeInput->setType('number');

			$expirationDate = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
			if (isset($this->attributes['shadowexpire'][0])) {
				$shAccExpirationDate = $this->attributes['shadowexpire'][0];
				$date = new DateTime('@' . $shAccExpirationDate*3600*24, new DateTimeZone('UTC'));
				$expirationDate = $date->format('d.m.Y');
			}
			$return->addLabel(new htmlOutputText(_('Account expiration date')));
			$expireTable = new htmlGroup();
			$expireTable->addElement(new htmlOutputText($expirationDate, false));
			$expireTable->addElement(new htmlAccountPageButton('shadowAccount', 'expire', 'open', 'edit-document.svg', true, _('Change')));
			$expireTable->addElement(new htmlHelpLink('shadowexpire'), true);
			$return->addField($expireTable);

			$pwdChangeDate = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
			if (isset($this->attributes['shadowlastchange'][0])) {
				$shPwdChangeDate = $this->attributes['shadowlastchange'][0];
				$date = new DateTime('@' . $shPwdChangeDate*3600*24, new DateTimeZone('UTC'));
				$pwdChangeDate = $date->format('d.m.Y');
			}
			$return->addLabel(new htmlOutputText(_('Last password change')));
			$pwdChangeTable = new htmlGroup();
			$pwdChangeTable->addElement(new htmlOutputText($pwdChangeDate, false));
			$pwdChangeTable->addElement(new htmlAccountPageButton('shadowAccount', 'pwdChange', 'open', 'edit-document.svg', true, _('Change')));
			if (isset($this->attributes['shadowmax'][0]) && ($this->attributes['shadowmax'][0] != '')) {
				$pwdChangeTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'expirePassword', _('Force password change')));
			}
			$pwdChangeTable->addElement(new htmlHelpLink('shadowlastchange'), true);
			$return->addField($pwdChangeTable);

			$return->addVerticalSpacer('2rem');
			$remButton = new htmlAccountPageButton('shadowAccount', 'attributes', 'remObjectClass', _('Remove Shadow account extension'));
			$remButton->setCSSClasses(array('lam-danger'));
			$return->add($remButton, 12, 12, 12, 'text-center');
		}
		else {
			$return->add(new htmlAccountPageButton('shadowAccount', 'attributes', 'addObjectClass', _('Add Shadow account extension')), 12);
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
		$errors = [];
		// set expiration date
		if (isset($_POST['form_subpage_shadowAccount_attributes_change']) && !empty($_POST['shadowexpire'])) {
			$date = DateTime::createFromFormat('d.m.Y', $_POST['shadowexpire'], new DateTimeZone('UTC'));
			$this->attributes['shadowexpire'][0] = intval($date->format('U')/3600/24);
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
		elseif (isset($_POST['form_subpage_shadowAccount_attributes_del'])
			|| (empty($_POST['shadowexpire']) && isset($this->attributes['shadowexpire']))) {
			unset($this->attributes['shadowexpire']);
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
		$return = new htmlResponsiveRow();
		$shAccExpirationDate = time() / (3600 * 24) + 365;
		if (isset($this->attributes['shadowexpire'][0])) {
			$shAccExpirationDate = $this->attributes['shadowexpire'][0];
		}
		$date = new DateTime('@' . $shAccExpirationDate*3600*24, new DateTimeZone('UTC'));
		$dateField = new htmlResponsiveInputField(_('Account expiration date'), 'shadowexpire', $date->format('d.m.Y'), 'shadowexpire');
		$dateField->showCalendar('d.m.Y');
		$return->add($dateField);
		if ($this->getAccountContainer()->getAccountModule('sambaSamAccount') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncSamba', false, _('Set also for Samba 3')), 12);
		}
		if ($this->getAccountContainer()->getAccountModule('windowsUser') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncWindows', false, _('Set also for Windows')), 12);
		}
		if ($this->getAccountContainer()->getAccountModule('heimdalKerberos') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncHeimdal', false, _('Set also for Kerberos')), 12);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberos') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncMIT', false, _('Set also for Kerberos')), 12);
		}
		if ($this->getAccountContainer()->getAccountModule('mitKerberosStructural') != null) {
			$return->add(new htmlResponsiveInputCheckbox('syncMITStructural', false, _('Set also for Kerberos')), 12);
		}
		$return->addVerticalSpacer('2rem');
		$buttonTable = new htmlGroup();
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'change', _('Change')));
		if (isset($this->attributes['shadowexpire'][0])) {
			$buttonTable->addElement(new htmlSpacer('0.5rem', null));
			$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'del', _('Remove')));
		}
		$buttonTable->addElement(new htmlSpacer('0.5rem', null));
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'back', _('Cancel')));
		$return->add($buttonTable, 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	* Processes user input of the last password change page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	function process_pwdChange() {
		$errors = [];
		// set last change date
		if (isset($_POST['form_subpage_shadowAccount_attributes_changePwdChange']) && !empty($_POST['shadowlastchange'])) {
			$date = DateTime::createFromFormat('d.m.Y', $_POST['shadowlastchange'], new DateTimeZone('UTC'));
			$this->attributes['shadowlastchange'][0] = intval($date->format('U')/3600/24);
		}
		// remove last change date
		elseif (isset($_POST['form_subpage_shadowAccount_attributes_delPwdChange'])
			|| (empty($_POST['shadowlastchange']) && isset($this->attributes['shadowlastchange']))) {
			unset($this->attributes['shadowlastchange']);
		}
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page with the password change date.
	*
	* @return array meta HTML code
	*/
	function display_html_pwdChange() {
		$return = new htmlResponsiveRow();
		$shLastChange = time()/(3600*24);
		if (isset($this->attributes['shadowlastchange'][0])) {
			$shLastChange = $this->attributes['shadowlastchange'][0];
		}
		$date = new DateTime('@' . $shLastChange*3600*24, new DateTimeZone('UTC'));
		$dateField = new htmlResponsiveInputField(_('Last password change'), 'shadowlastchange', $date->format('d.m.Y'), 'shadowlastchange');
		$dateField->showCalendar('d.m.Y');
		$return->add($dateField);
		$return->addVerticalSpacer('2rem');
		$buttonTable = new htmlGroup();
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'changePwdChange', _('Change')));
		if (isset($this->attributes['shadowlastchange'][0])) {
			$buttonTable->addElement(new htmlSpacer('0.5rem', null));
			$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'delPwdChange', _('Remove')));
		}
		$buttonTable->addElement(new htmlSpacer('0.5rem', null));
		$buttonTable->addElement(new htmlAccountPageButton('shadowAccount', 'attributes', 'back', _('Cancel')));
		$return->add($buttonTable, 12, 12, 12, 'text-center');
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$timeZone = getTimeZone();
		$shadowLastChange = '';
		if (!empty($this->attributes['shadowlastchange'][0])) {
			$time = new DateTime('@' . $this->attributes['shadowlastchange'][0]*24*3600, $timeZone);
			$shadowLastChange = $time->format('d.m.Y');
		}
		$shadowExpire = '';
		if (!empty($this->attributes['shadowexpire'][0])) {
			$time = new DateTime('@' . $this->attributes['shadowexpire'][0]*24*3600);
			$shadowExpire = $time->format('d.m.Y');
		}
		$return = [];
		$this->addPDFKeyValue($return, 'shadowLastChange', _('Last password change'), $shadowLastChange);
		$this->addPDFKeyValue($return, 'shadowExpire', _('Account expiration date'), $shadowExpire);
		$this->addSimplePDFField($return, 'shadowWarning', _('Password warning'));
		$this->addSimplePDFField($return, 'shadowInactive', _('Password expiration'));
		$this->addSimplePDFField($return, 'shadowMinAge', _('Minimum password age'), 'shadowmin');
		$this->addSimplePDFField($return, 'shadowMaxAge', _('Maximum password age'), 'shadowmax');
		return $return;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = [];
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("shadowAccount", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "shadowAccount";
			// password warning
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_warning', 'shadowwarning',
				'digit', $this->messages['shadowwarning'][1], $messages);
			// password expire ignoration
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_ignoreExpire', 'shadowinactive',
				'digit2', $this->messages['inactive'][1], $messages);
			// password minAge
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_minAge', 'shadowmin',
				'digit', $this->messages['shadowmin'][1], $messages);
			// password maxAge
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'shadowAccount_maxAge', 'shadowmax',
				'digit', $this->messages['shadowmax'][1], $messages);
			// minAge <= maxAge
			if ((($rawAccounts[$i][$ids['shadowAccount_minAge']] != '') || ($rawAccounts[$i][$ids['shadowAccount_maxAge']] != '')) &&  // if at least one is set
			(($rawAccounts[$i][$ids['shadowAccount_minAge']] == '') || ($rawAccounts[$i][$ids['shadowAccount_maxAge']] == '') ||   // and one is not set
			 ($rawAccounts[$i][$ids['shadowAccount_minAge']] > $rawAccounts[$i][$ids['shadowAccount_maxAge']]))) {  // or minAge > maxAge
				$errMsg = $this->messages['shadow_cmp'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// expiration date
			if ($rawAccounts[$i][$ids['shadowAccount_expireDate']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_expireDate']], 'date')) {
					$parts = explode('-', $rawAccounts[$i][$ids['shadowAccount_expireDate']]);
					$date = DateTime::createFromFormat('j.n.Y', $parts[0] . '.' . $parts[1] . '.' . $parts[2], new DateTimeZone('UTC'));
					$partialAccounts[$i]['shadowexpire'][] = intval($date->format('U')/3600/24);
				}
				else {
					$errMsg = $this->messages['shadow_expireDate'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// shadow last change
			if ($rawAccounts[$i][$ids['shadowAccount_lastChange']] != '') {
				if (get_preg($rawAccounts[$i][$ids['shadowAccount_lastChange']], 'date')) {
					$parts = explode('-', $rawAccounts[$i][$ids['shadowAccount_lastChange']]);
					$date = DateTime::createFromFormat('j.n.Y', $parts[0] . '.' . $parts[1] . '.' . $parts[2], new DateTimeZone('UTC'));
					$partialAccounts[$i]['shadowlastchange'][] = intval($date->format('U')/3600/24);
				}
				else {
					$errMsg = $this->messages['shadow_lastChange'][0];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			else {
				$date = new DateTime('now', new DateTimeZone('UTC'));
				$partialAccounts[$i]['shadowlastchange'][] = intval($date->format('U')/3600/24);
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
	 *               e.g. return array(array('ERROR', 'Password change failed.'))
	 * @see passwordService::passwordChangeRequested
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange) {
		// update password timestamp when Unix password was updated
		if (!in_array('posixAccount', $modules)) {
			return [];
		}
		if (in_array_ignore_case('shadowAccount', $this->attributes['objectClass'])) {
			$this->attributes['shadowlastchange'][0] = intval(time()/3600/24);
			if ($forcePasswordChange && isset($this->attributes['shadowmax'][0]) && ($this->attributes['shadowmax'][0] != 0)) {
				$this->attributes['shadowlastchange'][0] = intval(time()/3600/24) - $this->attributes['shadowmax'][0] - 1;
			}
		}
		return [];
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
			unset($this->attributes['shadowexpire']);
			return;
		}
		$date = DateTime::createFromFormat('j.n.Y', $day . '.' . $month . '.' . $year, new DateTimeZone('UTC'));
		$this->attributes['shadowexpire'][0] = intval($date->format('U')/3600/24);
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
			unset($this->attributes['shadowlastchange']);
			return;
		}
		$date = DateTime::createFromFormat('j.n.Y', $day . '.' . $month . '.' . $year, new DateTimeZone('UTC'));
		$this->attributes['shadowlastchange'][0] = intval($date->format('U')/3600/24);
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
		$return = [];
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		if (in_array('shadowLastChange', $fields)) {
			$shadowLastChange = '';
			if (isset($attributes['shadowlastchange'][0])) {
				$date = new DateTime('@' . $attributes['shadowlastchange'][0] * 3600 * 24, new DateTimeZone('UTC'));
				$shadowLastChange = $date->format('d.m.Y');
			}
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('shadowLastChange', _('Last password change'))));
			$row->addField(new htmlOutputText($shadowLastChange));
			$return['shadowLastChange'] = $row;
		}
		if (in_array('shadowExpire', $fields)) {
			$shadowExpire = '';
			if (isset($attributes['shadowexpire'][0])) {
				$date = new DateTime('@' . $attributes['shadowexpire'][0] * 3600 * 24, new DateTimeZone('UTC'));
				$shadowExpire = $date->format('d.m.Y');
			}
			$row = new htmlResponsiveRow();
			$row->addLabel(new htmlOutputText($this->getSelfServiceLabel('shadowExpire', _('Account expiration date'))));
			$row->addField(new htmlOutputText($shadowExpire));
			$return['shadowExpire'] = $row;
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
			new ShadowAccountExpirationCleanupJob(),
			new ShadowAccountExpirationNotifyJob()
		);
	}

	/**
	 * Returns if the given account is expired.
	 *
	 * @param array $attrs LDAP attributes
	 * @return bool expired
	 */
	public static function isAccountExpired($attrs) {
		if (empty($attrs['shadowexpire'][0])) {
			return false;
		}
		$time = new DateTime('@' . $attrs['shadowexpire'][0] * 24 * 3600, new DateTimeZone('UTC'));
		$now = new DateTime('now', getTimeZone());
		return ($time < $now);
	}

	/**
	 * Returns if the given password is expired.
	 *
	 * @param array $attrs LDAP attributes
	 * @return bool expired
	 */
	public static function isPasswordExpired($attrs) {
		if (empty($attrs['shadowlastchange'][0]) || empty($attrs['shadowmax'][0])) {
			return false;
		}
		if (($attrs['shadowlastchange'][0] < 1) || ($attrs['shadowmax'][0] < 1)) {
			return;
		}
		$time = new DateTime('@' . $attrs['shadowlastchange'][0] * 24 * 3600, new DateTimeZone('UTC'));
		$time = $time->add(new DateInterval('P' . $attrs['shadowmax'][0] . 'D'));
		if (!empty($attrs['shadowinactive'][0]) && ($attrs['shadowinactive'][0] > 0)) {
			$time = $time->add(new DateInterval('P' . $attrs['shadowinactive'][0] . 'D'));
		}
		$now = new DateTime('now', getTimeZone());
		return ($time < $now);
	}

	/**
	 * @inheritDoc
	 */
	public function supportsPasswordQuickChangePage(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function addPasswordQuickChangeAccountDetails(htmlResponsiveRow $row): void {
		// no details
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeOptions(): array {
		$options = [];
		if (in_array_ignore_case('shadowAccount', $this->attributes['objectClass'])) {
			$options[] = new PasswordQuickChangeOption('forcePasswordChangeUnix', _('Force Unix password change'), isPasswordChangeByDefault());
		}
		return $options;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeChanges(string $password): array {
		if (!in_array_ignore_case('shadowAccount', $this->attributes['objectClass'])) {
			return [];
		}
		$attrs = [];
		if (isset($this->attributes['shadowlastchange'][0]) && isset($_POST['updateUnixPwd'])) {
			$attrs['shadowlastchange'][0] = intval(time()/3600/24);
		}
		if (isset($_POST['forcePasswordChangeUnix'])
			&& isset($this->attributes['shadowmax'][0])
			&& ($this->attributes['shadowmax'][0] != 0)) {
			$attrs['shadowlastchange'][0] = intval(time()/3600/24) - $this->attributes['shadowmax'][0] - 1;
		}
		return $attrs;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthUserName(): ?string {
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangePasswordStrengthAttributes(): array {
		return [];
	}

	/**
	 * @inheritDoc
	 */
	public function getPasswordQuickChangeIsPasswordInHistory(string $password): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusDetails(ConfiguredType $type, ?array &$attributes): array {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$details = [];
		if (self::isAccountExpired($attributes)) {
			$details[] = AccountStatusDetails::newPartiallyExpired(_('Shadow') . ': ' . _('Account expiration'), self::STATUS_ACCOUNT_EXPIRED);
		}
		if (self::isPasswordExpired($attributes)) {
			$details[] = AccountStatusDetails::newPartiallyExpired(_('Shadow') . ': ' . _('Password expiration'), self::STATUS_PASSWORD_EXPIRED);
		}
		return $details;
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusRequiredAttributes(ConfiguredType $type): array {
		return array('shadowexpire', 'shadowlastchange', 'shadowmax', 'shadowinactive');
	}

	/**
	 * @inheritDoc
	 */
	public function getAccountStatusPossibleLockOptions(ConfiguredType $type, ?array &$attributes): array {
		if ($attributes === null) {
			$attributes = $this->attributes;
		}
		$options = [];
		if (!self::isAccountExpired($attributes)) {
			$options[] = AccountStatusDetails::newPartiallyExpired(_('Shadow') . ': ' . _('Account expiration'), self::STATUS_ACCOUNT_EXPIRED);
		}
		if (!self::isPasswordExpired($attributes)) {
			$options[] = AccountStatusDetails::newPartiallyExpired(_('Shadow') . ': ' . _('Password expiration'), self::STATUS_PASSWORD_EXPIRED);
		}
		return $options;
	}

	/**
	 * @inheritDoc
	 */
	public function accountStatusPerformLock(ConfiguredType $type, ?array &$attributes, array $lockIds): void {
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (in_array(self::STATUS_ACCOUNT_EXPIRED, $lockIds)) {
			$attributes['shadowexpire'][0] = '1';
		}
		if (in_array(self::STATUS_PASSWORD_EXPIRED, $lockIds)) {
			$attributes['shadowlastchange'][0] = intval(time()/3600/24) - $attributes['shadowmax'][0] - $attributes['shadowinactive'][0] - 1;
		}
	}

	/**
	 * @inheritDoc
	 */
	public function accountStatusPerformUnlock(ConfiguredType $type, ?array &$attributes, array $lockIds): void {
		if ($attributes === null) {
			$attributes = &$this->attributes;
		}
		if (in_array(self::STATUS_ACCOUNT_EXPIRED, $lockIds)) {
			unset($attributes['shadowexpire']);
		}
		if (in_array(self::STATUS_PASSWORD_EXPIRED, $lockIds)) {
			$attributes['shadowlastchange'][0] = intval(time()/3600/24);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function getListFilterFunction(string $attributeName): ?callable {
		if (($attributeName == 'shadowexpire') || ($attributeName == 'shadowlastchange')) {
			return function(?array $values, ?string $filterValue): bool {
				$regex = str_replace(array('*'), array('.*'), $filterValue);
				$regex = '/' . $regex . '/i';
				if (!empty($values[0])) {
					$time = new DateTime('@' . $values[0] * 24 * 3600, getTimeZone());
					$formattedTime = $time->format('d.m.Y');
					return preg_match($regex, $formattedTime);
				}
				return false;
			};
		}
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getListRenderFunction(string $attributeName): ?callable {
		if (($attributeName === 'shadowexpire') || ($attributeName === 'shadowlastchange')) {
			return function(array $entry, string $attribute): ?htmlElement {
				if (!empty($entry[$attribute][0])) {
					$time = new DateTime('@' . $entry[$attribute][0] * 24 * 3600, getTimeZone());
					return new htmlOutputText($time->format('d.m.Y'));
				}
				return null;
			};
		}
		return null;
	}

	/**
	 * @inheritDoc
	 */
	public function getListAttributeDescriptions(ConfiguredType $type): array {
		return array(
			'shadowlastchange' => _('Last password change'),
			'shadowwarning' => _('Password warning'),
			'shadowinactive' => _('Password expiration'),
			'shadowexpire' => _('Account expiration date'),
			'shadowminage' => _('Minimum password age'),
			'shadowmaxage' => _('Maximum password age'),
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
		 * {@inheritDoc}
		 * @see \LAM\JOB\PasswordExpirationJob::getConfigOptions()
		 */
		public function getConfigOptions($jobID) {
			$container = parent::getConfigOptions($jobID);
			$container->add(new htmlResponsiveInputCheckbox($this->getConfigPrefix() . '_addWarningPeriod' . $jobID, true, _('Add expiration warning time'), ['cronAddWarningPeriod', 'shadowAccount']));
			return $container;
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
			$sysattrs = array('mail', 'shadowlastchange', 'shadowwarning', 'shadowmax', 'userPassword');
			$attrs = $this->getAttrWildcards($jobID, $options);
			$attrs = array_values(array_unique(array_merge($attrs, $sysattrs)));
			return searchLDAPByFilter('(&(shadowlastchange=*)(shadowmax=*)(mail=*))', $attrs, array('user'));
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
				$this->jobResultLog->logDebug($user['dn'] . ' is locked.');
				return;
			}
			if ($user['shadowmax'][0] < 1) {
				$this->jobResultLog->logDebug($user['dn'] . ' does not expire.');
				return;
			}
			// calculate time when password expires
			$lastPwdTimeUnix = $user['shadowlastchange'][0] * 3600 * 24;
			$lastPwdTime = new DateTime('@' . $lastPwdTimeUnix, new DateTimeZone('UTC'));
			$this->jobResultLog->logDebug("Last password change on " . $lastPwdTime->format('Y-m-d'));
			$numDaysToWarn = $options[$this->getConfigPrefix() . '_mailNotificationPeriod' . $jobID][0];
			$addPasswordWarningTime = !isset($options[$this->getConfigPrefix() . '_addWarningPeriod' . $jobID][0])
				|| ($options[$this->getConfigPrefix() . '_addWarningPeriod' . $jobID][0] === 'true');
			if ($addPasswordWarningTime && !empty($user['shadowwarning'][0]) && ($user['shadowwarning'][0] > 0)) {
				$numDaysToWarn += $user['shadowwarning'][0];
			}
			$this->jobResultLog->logDebug("Number of days before warning " . $numDaysToWarn);
			$numDaysToExpire = $user['shadowmax'][0];
			$expireTime = $lastPwdTime->add(new DateInterval('P' . $numDaysToExpire . 'D'));
			$this->jobResultLog->logDebug("Password expires on " . $expireTime->format('Y-m-d'));
			// skip already expired accounts
			if ($expireTime <= $now) {
				$this->jobResultLog->logDebug($user['dn'] . ' already expired');
				return;
			}
			// calculate time of notification
			$notifyTime = clone $expireTime;
			$notifyTime->sub(new DateInterval('P' . $numDaysToWarn . 'D'));
			$notifyTime->setTimeZone(getTimeZone());
			$this->jobResultLog->logDebug("Password notification on " . $notifyTime->format('Y-m-d H:i'));
			// skip if notification is in the future
			if ($notifyTime > $now) {
				$this->jobResultLog->logDebug($user['dn'] . ' does not need notification yet.');
				return;
			}
			$dbLastChange = $this->getDBLastPwdChangeTime($jobID, $pdo, $user['dn']);
			// skip entries where mail was already sent
			if ($dbLastChange == $user['shadowlastchange'][0]) {
				$this->jobResultLog->logDebug($user['dn'] . ' was already notified.');
				return;
			}
			if ($isDryRun) {
				// no action for dry run
				$this->jobResultLog->logInfo('Not sending email to ' . $user['dn'] . ' because of dry run.');
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
	 * Job to notify users about account expiration.
	 *
	 * @package jobs
	 */
	class ShadowAccountExpirationNotifyJob extends \LAM\JOB\PasswordExpirationJob {

		/**
		 * Returns the alias name of the job.
		 *
		 * @return String name
		 */
		public function getAlias() {
			return _('Shadow') . ': ' . _('Notify users about account expiration');
		}

		/**
		 * {@inheritDoc}
		 * @see \LAM\JOB\PasswordExpirationJob::getDescription()
		 */
		public function getDescription() {
			return _('This job sends out emails to inform your users that their account will expire soon.');
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
			$sysattrs = array('mail', 'shadowexpire');
			$attrs = $this->getAttrWildcards($jobID, $options);
			$attrs = array_values(array_unique(array_merge($attrs, $sysattrs)));
			return searchLDAPByFilter('(&(shadowexpire=*)(mail=*))', $attrs, array('user'));
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
			$dn = $user['dn'];
			$expireTimeUnix = $user['shadowexpire'][0] * 3600 * 24;
			$expireTime = new DateTime('@' . $expireTimeUnix, new DateTimeZone('UTC'));
			$this->jobResultLog->logDebug("Expiration on " . $expireTime->format('Y-m-d'));
			if ($expireTime <= $now) {
				$this->jobResultLog->logDebug($dn . ' already expired');
				return;
			}
			$numDaysToWarn = 0;
			if (!empty($options[$this->getConfigPrefix() . '_mailNotificationPeriod' . $jobID][0])) {
				$numDaysToWarn = $options[$this->getConfigPrefix() . '_mailNotificationPeriod' . $jobID][0];
			}
			$actionTime = clone $expireTime;
			if ($numDaysToWarn != 0) {
				$actionTime->sub(new DateInterval('P' . $numDaysToWarn . 'D'));
			}
			$actionTime->setTimeZone(getTimeZone());
			$this->jobResultLog->logDebug("Action time on " . $actionTime->format('Y-m-d'));
			if ($actionTime > $now) {
				$this->jobResultLog->logDebug($dn . ' does not need notification yet.');
				return;
			}
			$dbLastChange = $this->getDBLastPwdChangeTime($jobID, $pdo, $user['dn']);
			// skip entries where mail was already sent
			if ($dbLastChange == $user['shadowexpire'][0]) {
				$this->jobResultLog->logDebug($dn . ' was already notified.');
				return;
			}
			if ($isDryRun) {
				// no action for dry run
				$this->jobResultLog->logInfo('Not sending email to ' . $dn . ' because of dry run.');
				return;
			}
			// send email
			$success = $this->sendMail($options, $jobID, $user, $expireTime);
			// update DB if mail was sent successfully
			if ($success) {
				$this->setDBLastPwdChangeTime($jobID, $pdo, $dn, $user['shadowexpire'][0]);
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
			$attrs = array('shadowexpire');
			return searchLDAPByFilter('(shadowexpire=*)', $attrs, array('user'));
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
			$this->jobResultLog->logDebug("Expiration on " . $expireTime->format('Y-m-d'));
			$delay = 0;
			if (!empty($options[$this->getConfigPrefix() . '_delay' . $jobID][0])) {
				$delay = $options[$this->getConfigPrefix() . '_delay' . $jobID][0];
			}
			$actionTime = clone $expireTime;
			if ($delay != 0) {
				$actionTime->add(new DateInterval('P' . $delay . 'D'));
			}
			$actionTime->setTimeZone(getTimeZone());
			$this->jobResultLog->logDebug("Action time on " . $actionTime->format('Y-m-d'));
			if ($actionTime <= $now) {
				$this->performAction($jobID, $options, $user, $isDryRun);
			}
		}

	}

}
