<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2017  Roland Gruber

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
* Manages FreeRadius accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages FreeRadius accounts.
*
* @package modules
*/
class freeRadius extends baseModule {

	/** list of possible months */
	private static $monthList = array('01' => 'Jan', '02' => 'Feb', '03' => 'Mar', '04' => 'Apr', '05' => 'May',
			'06' => 'Jun', '07' => 'Jul', '08' => 'Aug', '09' => 'Sep', '10' => 'Oct', '11' => 'Nov', '12' => 'Dec'
		);

	/** cache for profile DNs */
	private $profileCache = null;


	/**
	* Creates a new freeRadius object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		parent::__construct($scope);
		$this->autoAddObjectClasses = false;
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
		$return['icon'] = 'freeRadius.png';
		// alias name
		$return["alias"] = _("FreeRadius");
		// module dependencies
		$return['dependencies'] = array('depends' => array(array('posixAccount', 'inetOrgPerson')), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('radiusprofile');
		// managed attributes
		$return['attributes'] = array('radiusFramedIPAddress', 'radiusFramedIPNetmask', 'radiusRealm', 'radiusGroupName',
			'radiusExpiration', 'radiusIdleTimeout', 'dialupAccess', 'radiusProfileDn');
		// help Entries
		$return['help'] = array(
			'radiusFramedIPAddress' => array(
				"Headline" => _("IP address"), 'attr' => 'radiusFramedIPAddress',
				"Text" => _("This is the IP address for the user (e.g. 123.123.123.123).")
			),
			'radiusFramedIPNetmask' => array(
				"Headline" => _("Net mask"), 'attr' => 'radiusFramedIPNetmask',
				"Text" => _("The net mask for the IP address.")
			),
			'radiusRealm' => array(
				"Headline" => _("Realm"), 'attr' => 'radiusRealm',
				"Text" => _("The Radius realm of this account.")
			),
			'radiusGroupName' => array(
				"Headline" => _("Group names"), 'attr' => 'radiusGroupName',
				"Text" => _("The group names for this account.")
			),
			'radiusGroupNameList' => array(
				"Headline" => _("Group names"), 'attr' => 'radiusGroupName',
				"Text" => _("The group names for this account.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'radiusExpiration' => array(
				"Headline" => _("Expiration date"), 'attr' => 'radiusExpiration',
				"Text" => _("The account will be locked after this date.")
			),
			'radiusIdleTimeout' => array(
				"Headline" => _("Idle timeout"), 'attr' => 'radiusIdleTimeout',
				"Text" => _("Specifies the maximum number of seconds that a connection can be idle before the session is terminated.")
			),
			'dialupAccess' => array(
				"Headline" => _("Enabled"), 'attr' => 'dialupAccess',
				"Text" => _("Specifies if the user may authenticate with FreeRadius.")
			),
			'profileDN' => array(
				"Headline" => _("Profile DN"), 'attr' => 'radiusProfileDn',
				"Text" => _('DN where Radius profile templates are stored.')
			),
			'radiusProfileDn' => array(
				"Headline" => _("Profile"), 'attr' => 'radiusProfileDn',
				"Text" => _('Radius profile for this user.')
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			));
		// profile settings
		$profileElements = array();
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPNetmask')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Net mask'), 'freeRadius_radiusFramedIPNetmask', null, 'radiusFramedIPNetmask');
			$return['profile_checks']['freeRadius_radiusFramedIPNetmask'] = array(
				'type' => 'ext_preg',
				'regex' => 'ip',
				'error_message' => $this->messages['radiusFramedIPNetmask'][0]);
			$return['profile_mappings']['freeRadius_radiusFramedIPNetmask'] = 'radiusFramedIPNetmask';
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusRealm')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Realm'), 'freeRadius_radiusRealm', null, 'radiusRealm');
			$return['profile_checks']['freeRadius_radiusRealm'] = array(
				'type' => 'ext_preg',
				'regex' => 'DNSname',
				'error_message' => $this->messages['radiusRealm'][0]);
			$return['profile_mappings']['freeRadius_radiusRealm'] = 'radiusRealm';
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Group names'), 'freeRadius_radiusGroupName', null, 'radiusGroupNameList');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusIdleTimeout')) {
			$profileElements[] = new htmlTableExtendedInputField(_('Idle timeout'), 'freeRadius_radiusIdleTimeout', null, 'radiusIdleTimeout');
			$return['profile_checks']['freeRadius_radiusIdleTimeout'] = array(
				'type' => 'ext_preg',
				'regex' => 'digit',
				'error_message' => $this->messages['radiusIdleTimeout'][0]);
			$return['profile_mappings']['freeRadius_radiusIdleTimeout'] = 'radiusIdleTimeout';
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideDialupAccess')) {
			$enabledOptions = array('-' => '', _('Yes') => 'true', _('No') => 'false');
			$dialupAccessSelect = new htmlTableExtendedSelect('freeRadius_dialupAccess', $enabledOptions, array('true'), _('Enabled'), 'dialupAccess');
			$dialupAccessSelect->setHasDescriptiveElements(true);
			$profileElements[] = $dialupAccessSelect;
			$return['profile_mappings']['freeRadius_dialupAccess'] = 'dialupAccess';
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusProfileDn') && isLoggedIn()) {
			$profileOptions = array('-' => '');
			foreach ($this->getProfiles() as $dn) {
				$profileOptions[getAbstractDN($dn)] = $dn;
			}
			$profileSelect = new htmlTableExtendedSelect('freeRadius_radiusProfileDn', $profileOptions, array(''), _('Profile'), 'radiusProfileDn');
			$profileSelect->setHasDescriptiveElements(true);
			$profileElements[] = $profileSelect;
			$return['profile_mappings']['freeRadius_radiusProfileDn'] = 'radiusProfileDn';
		}
		if (sizeof($profileElements) > 0) {
			$profileContainer = new htmlTable();
			for ($i = 0; $i < sizeof($profileElements); $i++) {
				$profileContainer->addElement($profileElements[$i]);
				$profileContainer->addNewLine();
			}
			$return['profile_options'] = $profileContainer;
		}
		// upload fields
		$return['upload_columns'] = array();
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusRealm')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_radiusRealm',
				'description' => _('Realm'),
				'help' => 'radiusRealm',
				'example' => _('company.com')
			);
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_radiusGroupName',
				'description' => _('Group names'),
				'help' => 'radiusGroupNameList',
				'example' => _('group01;group02')
			);
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPAddress')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_radiusFramedIPAddress',
				'description' => _('IP address'),
				'help' => 'radiusFramedIPAddress',
				'example' => '123.123.123.123',
			);
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPNetmask')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_radiusFramedIPNetmask',
				'description' => _('Net mask'),
				'help' => 'radiusFramedIPNetmask',
				'example' => '255.255.255.0'
			);
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusExpiration')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_radiusExpiration',
				'description' => _('Expiration date'),
				'help' => 'radiusExpiration',
				'example' => '17.07.2017 00:00'
			);
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusIdleTimeout')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_radiusIdleTimeout',
				'description' => _('Idle timeout'),
				'help' => 'radiusIdleTimeout',
				'example' => '3600'
			);
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideDialupAccess')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_dialupAccess',
				'description' => _('Enabled'),
				'help' => 'dialupAccess',
				'example' => 'true',
				'values' => 'true, false'
			);
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusProfileDn')) {
			$return['upload_columns'][] = array(
				'name' => 'freeRadius_radiusProfileDn',
				'description' => _('Profile'),
				'help' => 'radiusProfileDn',
				'example' => 'cn=profile,ou=radiusProfile,dc=example,dc=com'
			);
		}
		// available PDF fields
		$return['PDF_fields'] = array();
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPAddress')) {
			$return['PDF_fields']['radiusFramedIPAddress'] = _('IP address');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPNetmask')) {
			$return['PDF_fields']['radiusFramedIPNetmask'] = _('Net mask');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusRealm')) {
			$return['PDF_fields']['radiusRealm'] = _('Realm');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
			$return['PDF_fields']['radiusGroupName'] = _('Group names');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusExpiration')) {
			$return['PDF_fields']['radiusExpiration'] = _('Expiration date');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusIdleTimeout')) {
			$return['PDF_fields']['radiusIdleTimeout'] = _('Idle timeout');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideDialupAccess')) {
			$return['PDF_fields']['dialupAccess'] = _('Enabled');
		}
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusProfileDn')) {
			$return['PDF_fields']['radiusProfileDn'] = _('Profile');
		}
		return $return;
	}

	/**
	* Returns a list of configuration options.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The field names are used as keywords to load and save settings.
	* We recommend to use the module name as prefix for them (e.g. posixAccount_homeDirectory) to avoid naming conflicts.
	*
	* @param array $scopes account types (user, group, host)
	* @param array $allScopes list of all active account modules and their scopes (module => array(scopes))
	* @return mixed htmlElement or array of htmlElement
	*
	* @see htmlElement
	*/
	public function get_configOptions($scopes, $allScopes) {
		$configContainer = new htmlTable();
		$configContainer->addElement(new htmlTableExtendedInputField(_('Profile DN'), 'freeRadius_profileDN', '', 'profileDN'), true);
		$configContainer->addVerticalSpace('10px');
		$configContainerHead = new htmlTable();
		$configContainerHead->colspan = 5;
		$configContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$configContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainerOptions = new htmlTable();
		$configContainerOptions->colspan = 5;
		$configContainer->addElement($configContainerHead, true);
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideRadiusFramedIPAddress', false, _('IP address'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideRadiusFramedIPNetmask', false, _('Net mask'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideRadiusRealm', false, _('Realm'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideRadiusGroupName', false, _('Group names'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideRadiusExpiration', false, _('Expiration date'), null, false));
		$configContainerOptions->addNewLine();
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideRadiusIdleTimeout', false, _('Idle timeout'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideRadiusProfileDn', false, _('Profile'), null, false));
		$configContainerOptions->addElement(new htmlOutputText(' '));
		$configContainerOptions->addElement(new htmlTableExtendedInputCheckbox('freeRadius_hideDialupAccess', false, _('Enabled'), null, false));
		$configContainer->addElement($configContainerOptions, true);
		return $configContainer;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['radiusFramedIPAddress'][0] = array('ERROR', _('The IP address is invalid.'));
		$this->messages['radiusFramedIPAddress'][1] = array('ERROR', _('Account %s:') . ' freeRadius_radiusFramedIPAddress', _('The IP address is invalid.'));
		$this->messages['radiusFramedIPNetmask'][0] = array('ERROR', _('The net mask is invalid.'));
		$this->messages['radiusFramedIPNetmask'][1] = array('ERROR', _('Account %s:') . ' freeRadius_radiusFramedIPNetmask', _('The net mask is invalid.'));
		$this->messages['radiusRealm'][0] = array('ERROR', _('Please enter a valid realm.'));
		$this->messages['radiusRealm'][1] = array('ERROR', _('Account %s:') . ' freeRadius_radiusRealm', _('Please enter a valid realm.'));
		$this->messages['radiusGroupName'][0] = array('ERROR', _('Please enter a valid list of group names.'));
		$this->messages['radiusGroupName'][1] = array('ERROR', _('Account %s:') . ' freeRadius_radiusGroupName', _('Please enter a valid list of group names.'));
		$this->messages['radiusExpiration'][0] = array('ERROR', _('The expiration date must be in format DD.MM.YYYY HH:MM.'));
		$this->messages['radiusExpiration'][1] = array('ERROR', _('Account %s:') . ' freeRadius_radiusExpiration', _('The expiration date must be in format DD.MM.YYYY HH:MM.'));
		$this->messages['radiusIdleTimeout'][0] = array('ERROR', _('Please enter a numeric value for the idle timeout.'));
		$this->messages['radiusIdleTimeout'][1] = array('ERROR', _('Account %s:') . ' freeRadius_radiusIdleTimeout', _('Please enter a numeric value for the idle timeout.'));
		$this->messages['dialupAccess'][0] = array('ERROR', _('Account %s:') . ' freeRadius_dialupAccess', _('This value can only be "true" or "false".'));
		$this->messages['radiusProfileDn'][0] = array('ERROR', _('Account %s:') . ' freeRadius_radiusProfileDn', _('This is not a valid DN!'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 *
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		if (in_array('radiusprofile', $this->attributes['objectClass'])) {
			// realm
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusRealm')) {
				$this->addSimpleInputTextField($return, 'radiusRealm', _('Realm'));
			}
			// group names
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
				$this->addMultiValueInputTextField($return, 'radiusGroupName', _('Group names'));
			}
			// IP address
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPAddress')) {
				$this->addSimpleInputTextField($return, 'radiusFramedIPAddress', _('IP address'));
			}
			// net mask
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPNetmask')) {
				$this->addSimpleInputTextField($return, 'radiusFramedIPNetmask', _('Net mask'));
			}
			// idle timeout
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusIdleTimeout')) {
				$radiusIdleTimeoutInput = $this->addSimpleInputTextField($return, 'radiusIdleTimeout', _('Idle timeout'));
				$radiusIdleTimeoutInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
			}
			// expiration date
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusExpiration')) {
				$radiusExpiration = '&nbsp;&nbsp;-';
				if (isset($this->attributes['radiusExpiration'][0])) {
					$radiusExpiration = $this->formatExpirationDate($this->attributes['radiusExpiration'][0]);
				}
				$return->addElement(new htmlOutputText('Expiration date'));
				$radiusExpirationList = new htmlGroup();
				$radiusExpirationList->addElement(new htmlOutputText($radiusExpiration . ' &nbsp;&nbsp;&nbsp;&nbsp;', false));
				$radiusExpirationList->addElement(new htmlAccountPageButton(get_class($this), 'expiration', 'change', _('Change')));
				$return->addElement($radiusExpirationList);
				$return->addElement(new htmlHelpLink('radiusExpiration'), true);
			}
			// profile DN
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusProfileDn')) {
				$profiles = array('-' => '-');
				foreach ($this->getProfiles() as $dn) {
					$profiles[getAbstractDN($dn)] = $dn;
				}
				$profile = array();
				if (!empty($this->attributes['radiusProfileDn'][0])) {
					$profile = $this->attributes['radiusProfileDn'];
					if (!in_array($this->attributes['radiusProfileDn'][0], $profiles)) {
						$profiles[getAbstractDN($this->attributes['radiusProfileDn'][0])] = $this->attributes['radiusProfileDn'][0];
					}
				}
				$profileSelect = new htmlTableExtendedSelect('radiusProfileDn', $profiles, $profile, _('Profile'), 'radiusProfileDn');
				$profileSelect->setHasDescriptiveElements(true);
				$return->addElement($profileSelect, true);
			}
			// enabled
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideDialupAccess')) {
				$enabled = array('');
				if (!empty($this->attributes['dialupAccess'][0])) {
					$enabled = array($this->attributes['dialupAccess'][0]);
					// value in LDAP may be anything other than "false" to count as "true"
					if (!in_array($this->attributes['dialupAccess'][0], array('true', 'false', 'TRUE', 'FALSE'))) {
						$enabled = array('true');
					}
				}
				$enabledOptions = array('-' => '', _('Yes') => 'true', _('No') => 'false');
				$enabledSelect = new htmlTableExtendedSelect('dialupAccess', $enabledOptions, $enabled, _('Enabled'), 'dialupAccess');
				$enabledSelect->setHasDescriptiveElements(true);
				$return->addElement($enabledSelect, true);
			}
			// button to remove extension
			$return->addElement(new htmlSpacer(null, '10px'), true);
			$remButton = new htmlButton('remObjectClass', _('Remove FreeRadius extension'));
			$remButton->colspan = 3;
			$return->addElement($remButton);
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add FreeRadius extension')));
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
		if (isset($_POST['addObjectClass'])) {
			$this->attributes['objectClass'][] = 'radiusprofile';
			return array();
		}
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('radiusprofile'), $this->attributes['objectClass']);
			for ($i = 0; $i < sizeof($this->meta['attributes']); $i++) {
				if (isset($this->attributes[$this->meta['attributes'][$i]])) {
					unset($this->attributes[$this->meta['attributes'][$i]]);
				}
			}
			return array();
		}
		// skip processing if extension is not active
		if (!in_array('radiusprofile', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		// IP address
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPAddress')) {
			$this->attributes['radiusFramedIPAddress'][0] = $_POST['radiusFramedIPAddress'];
			if (($_POST['radiusFramedIPAddress'] != '') && !get_preg($_POST['radiusFramedIPAddress'], 'ip')) {
				$errors[] = $this->messages['radiusFramedIPAddress'][0];
			}
		}
		// net mask
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPNetmask')) {
			$this->attributes['radiusFramedIPNetmask'][0] = $_POST['radiusFramedIPNetmask'];
			if (($_POST['radiusFramedIPNetmask'] != '') && !get_preg($_POST['radiusFramedIPNetmask'], 'ip')) {
				$errors[] = $this->messages['radiusFramedIPNetmask'][0];
			}
		}
		// realm
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusRealm')) {
			$this->attributes['radiusRealm'][0] = $_POST['radiusRealm'];
			if (($_POST['radiusRealm'] != '') && !get_preg($_POST['radiusRealm'], 'DNSname')) {
				$errors[] = $this->messages['radiusRealm'][0];
			}
		}
		// group names
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
			$this->processMultiValueInputTextField('radiusGroupName', $errors, 'groupname');
		}
		// idle timeout
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusIdleTimeout')) {
			$this->attributes['radiusIdleTimeout'][0] = $_POST['radiusIdleTimeout'];
			if (($_POST['radiusIdleTimeout'] != '') && !get_preg($_POST['radiusIdleTimeout'], 'digit')) {
				$errors[] = $this->messages['radiusIdleTimeout'][0];
			}
		}
		// enabled
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideDialupAccess')) {
			if (!empty($this->attributes['dialupAccess']) && ($_POST['dialupAccess'] === '')) {
				unset($this->attributes['dialupAccess']);
			}
			elseif ($_POST['dialupAccess'] === 'false') {
				$this->attributes['dialupAccess'][0] = 'false';
			}
			elseif ($_POST['dialupAccess'] === 'true') {
				$this->attributes['dialupAccess'][0] = 'true';
			}
		}
		// profile DN
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusProfileDn')) {
			if (($_POST['radiusProfileDn'] == '-') && !empty($this->attributes['radiusProfileDn'])) {
				unset($this->attributes['radiusProfileDn']);
			}
			elseif ($_POST['radiusProfileDn'] != '-') {
				$this->attributes['radiusProfileDn'][0] = $_POST['radiusProfileDn'];
			}
		}
		return $errors;
	}

	/**
	* This function will create the meta HTML code to show a page to change the expiration date.
	*
	* @return htmlElement meta HTML code
	*/
	function display_html_expiration() {
		$return = new htmlTable();
		$attr = 'radiusExpiration';
		$text = _('Expiration date');
		$help = "radiusExpiration";
		$date = new DateTime('@' . (time() + 3600*24*365), new DateTimeZone('UTC'));
		$year = $date->format('Y');
		$month = $date->format('m');
		$month = freeRadius::$monthList[$month];
		$day = $date->format('d');
		$hour = '00';
		$minute = '00';
		if (isset($this->attributes[$attr][0]) && ($this->attributes[$attr][0] != '')) {
			$parts = explode(' ', $this->attributes[$attr][0]);
			$year = $parts[2];
			$month = $parts[1];
			$day = $parts[0];
			if (isset($parts[3])) {
				$timeParts = explode(':', $parts[3]);
				$hour = $timeParts[0];
				$minute = $timeParts[1];
			}
		}
		for ( $i=0; $i<=59; $i++ ) {
			$minuteList[] = str_pad($i, 2, '0', STR_PAD_LEFT);
		}
		for ( $i=0; $i<=23; $i++ ) {
			$hourList[] = str_pad($i, 2, '0', STR_PAD_LEFT);
		}
		for ( $i=1; $i<=31; $i++ ) {
			$dayList[] = str_pad($i, 2, '0', STR_PAD_LEFT);
		}
		for ( $i=2003; $i<=2050; $i++ ) $yearList[] = $i;
		$return->addElement(new htmlOutputText($text));
		$return->addElement(new htmlSelect('expire_day', $dayList, array($day)));
		$monthSelect = new htmlSelect('expire_mon', freeRadius::$monthList, array($month));
		$monthSelect->setHasDescriptiveElements(true);
		$return->addElement($monthSelect);
		$return->addElement(new htmlSelect('expire_yea', $yearList, array($year)));
		$return->addElement(new htmlSpacer('10px', null));
		$return->addElement(new htmlSelect('expire_hour', $hourList, array($hour)));
		$return->addElement(new htmlSelect('expire_minute', $minuteList, array($minute)));
		$return->addElement(new htmlHelpLink($help), true);
		$return->addElement(new htmlSpacer(null, '10px'), true);
		$buttons = new htmlTable();
		$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'change' . $attr, _('Change')));
		if (isset($this->attributes[$attr][0])) {
			$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'del' . $attr, _('Remove')));
		}
		$buttons->addElement(new htmlAccountPageButton(get_class($this), 'attributes', 'back' . $attr, _('Cancel')));
		$buttons->colspan = 6;
		$return->addElement($buttons);
		return $return;
	}

	/**
	* Processes user input of the time selection page.
	*
	* @return array list of info/error messages
	*/
	function process_expiration() {
		$return = array();
		// find button name
		$buttonName = '';
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (strpos($postKeys[$i], 'form_subpage_freeRadius_attributes_') !== false) {
				$buttonName = $postKeys[$i];
			}
		}
		if (($buttonName == '') || (strpos($buttonName, '_back') !== false)) return array();
		// get attribute name
		$attr = '';
		if (strpos($buttonName, 'radiusExpiration') !== false) {
			$attr = 'radiusExpiration';
		}
		if ($attr == '') return array();
		// determine action
		if (strpos($buttonName, '_change') !== false) {
			// set new expiration date
			$this->attributes[$attr][0] = $_POST['expire_day'] . ' ' . $_POST['expire_mon'] . ' ' . $_POST['expire_yea'] . ' ' . $_POST['expire_hour'] . ':' . $_POST['expire_minute'];
		}
		elseif (strpos($buttonName, '_del') !== false) {
			// remove attribute value
			unset($this->attributes[$attr]);
		}
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
		if (!in_array('radiusprofile', $this->attributes['objectClass']) && !in_array('radiusprofile', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$errors = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("radiusprofile", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "radiusprofile";
			// IP address
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'freeRadius_radiusFramedIPAddress', 'radiusFramedIPAddress',
				'ip', $this->messages['radiusFramedIPAddress'][1], $errors);
			// net mask
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'freeRadius_radiusFramedIPNetmask', 'radiusFramedIPNetmask',
				'ip', $this->messages['radiusFramedIPNetmask'][1], $errors);
			// realm
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'freeRadius_radiusRealm', 'radiusRealm',
				'DNSname', $this->messages['radiusRealm'][1], $errors);
			// group names
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'freeRadius_radiusGroupName', 'radiusGroupName', 'groupname', $this->messages['radiusGroupName'][1], $errors, '/;[ ]*/');
			// expiration date
			if ($rawAccounts[$i][$ids['freeRadius_radiusExpiration']] != "") {
				if (preg_match('/^[0-9]{1,2}.[0-9]{1,2}.[0-9]{4} [0-9]{1,2}:[0-9]{1,2}$/', $rawAccounts[$i][$ids['freeRadius_radiusExpiration']])) {
					$dateParts = explode(' ', $rawAccounts[$i][$ids['freeRadius_radiusExpiration']]);
					$dateParts1 = explode('.', $dateParts[0]);
					$radiusExpiration = str_pad($dateParts1[0], 2, '0', STR_PAD_LEFT) . ' ' . freeRadius::$monthList[str_pad($dateParts1[1], 2, '0', STR_PAD_LEFT)] . ' ' . $dateParts1[2];
					$dateParts2 = explode(':', $dateParts[1]);
					$radiusExpiration .= ' ' . str_pad($dateParts2[0], 2, '0', STR_PAD_LEFT) . ':' . str_pad($dateParts2[1], 2, '0', STR_PAD_LEFT);
					$partialAccounts[$i]['radiusExpiration'] = $radiusExpiration;
				}
				else {
					$errMsg = $this->messages['radiusExpiration'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// idle timeout
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'freeRadius_radiusIdleTimeout', 'radiusIdleTimeout',
				'digit', $this->messages['radiusIdleTimeout'][1], $errors);
			// enabled
			if (!empty($rawAccounts[$i][$ids['freeRadius_dialupAccess']])) {
				if (in_array($rawAccounts[$i][$ids['freeRadius_dialupAccess']], array('true', 'false'))) {
					$partialAccounts[$i]['dialupAccess'] = $rawAccounts[$i][$ids['freeRadius_dialupAccess']];
				}
				else {
					$errMsg = $this->messages['dialupAccess'][0];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// profile DN
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'freeRadius_radiusProfileDn', 'radiusProfileDn', 'dn', $this->messages['radiusProfileDn'][0], $errors);
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'radiusFramedIPAddress', _('IP address'));
		$this->addSimplePDFField($return, 'radiusFramedIPNetmask', _('Net mask'));
		$this->addSimplePDFField($return, 'radiusRealm', _('Realm'));
		$this->addSimplePDFField($return, 'radiusGroupName', _('Group names'));
		$this->addSimplePDFField($return, 'radiusIdleTimeout', _('Idle timeout'));
		$this->addSimplePDFField($return, 'radiusProfileDn', _('Profile'));
		if (isset($this->attributes['radiusExpiration'][0])) {
			$this->addPDFKeyValue($return, 'radiusExpiration', _('Expiration date'), $this->formatExpirationDate($this->attributes['radiusExpiration'][0]));
		}
		if (isset($this->attributes['dialupAccess'][0])) {
			$enabled = _('Yes');
			if (in_array($this->attributes['dialupAccess'][0], array('false', 'FALSE'))) {
				$enabled = _('No');
			}
			$this->addPDFKeyValue($return, 'dialupAccess', _('Enabled'), $enabled);
		}
		return $return;
	}

	/**
	* {@inheritDoc}
	*/
	function check_profileOptions($options, $typeId) {
		$messages = parent::check_profileOptions($options, $typeId);
		// group names
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
			if (isset($options['freeRadius_radiusGroupName'][0]) && ($options['freeRadius_radiusGroupName'][0] != '')) {
				$list = preg_split('/;[ ]*/', $options['freeRadius_radiusGroupName'][0]);
				for ($i = 0; $i < sizeof($list); $i++) {
					if (!get_preg($list[$i], 'groupname')) {
						$messages[] = $this->messages['radiusGroupName'][0];
						break;
					}
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
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
			// group names
			if (isset($profile['freeRadius_radiusGroupName'][0]) && $profile['freeRadius_radiusGroupName'][0] != '') {
				$this->attributes['radiusGroupName'] = preg_split('/;[ ]*/', $profile['freeRadius_radiusGroupName'][0]);
			}
		}
	}

	/**
	 * Formats the expiration date attribute.
	 *
	 * @param String $date date value
	 */
	private function formatExpirationDate($date) {
		if (is_null($date) || ($date == '')) {
			return $date;
		}
		foreach (freeRadius::$monthList as $replace => $search) {
			$date = str_replace($search, $replace, $date);
		}
		$dateParts = explode(' ', $date);
		$date = $dateParts[0] . '.' . $dateParts[1] . '.' . $dateParts[2];
		if (isset($dateParts[3])) {
			$date .= ' ' . $dateParts[3];
		}
		return $date;
	}

	/**
	 * Returns a list of possible profile DNs.
	 *
	 * @return array list of profile DNs
	 */
	private function getProfiles() {
		if ($this->profileCache != null) {
			return $this->profileCache;
		}
		if (empty($this->moduleSettings['freeRadius_profileDN'][0])) {
			return array();
		}
		$list = searchLDAP($this->moduleSettings['freeRadius_profileDN'][0], '(objectClass=radiusProfile)', array('dn'));
		foreach ($list as $attr) {
			$this->profileCache[] = $attr['dn'];
		}
		usort($this->profileCache, 'compareDN');
		return $this->profileCache;
	}

	/**
	 * Returns a list of jobs that can be run.
	 *
	 * @param LAMConfig $config configuration
	 * @return array list of jobs
	 */
	public function getSupportedJobs(&$config) {
		return array(
			new FreeRadiusAccountExpirationCleanupJob()
		);
	}

}

if (interface_exists('\LAM\JOB\Job', false)) {

	include_once dirname(__FILE__) . '/../passwordExpirationJob.inc';

	/**
	 * Job to delete or move users on account expiration.
	 *
	 * @package jobs
	 */
	class FreeRadiusAccountExpirationCleanupJob extends \LAM\JOB\AccountExpirationCleanupJob {

		/**
		 * Returns the alias name of the job.
		 *
		 * @return String name
		 */
		public function getAlias() {
			return _('FreeRadius') .  ': ' . _('Cleanup expired user accounts');
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
			$attrs = array('radiusExpiration');
			$userResults = searchLDAPByFilter('(radiusExpiration=*)', $attrs, array('user'));
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
			$expireTime = DateTime::createFromFormat('d M Y H:i', $user['radiusexpiration'][0], new DateTimeZone('UTC'));
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
