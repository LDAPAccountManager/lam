<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011 - 2012  Roland Gruber

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

	private static $monthList = array('01' => 'Jan', '02' => 'Feb', '03' => 'Mar', '04' => 'Apr', '05' => 'May',
			'06' => 'Jun', '07' => 'Jul', '08' => 'Aug', '09' => 'Sep', '10' => 'Oct', '11' => 'Nov', '12' => 'Dec'
		);
	
	
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
		// manages user accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("FreeRadius");
		// module dependencies
		$return['dependencies'] = array('depends' => array(array('posixAccount', 'inetOrgPerson')), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('radiusprofile');
		// managed attributes
		$return['attributes'] = array('radiusFramedIPAddress', 'radiusFramedIPNetmask', 'radiusRealm', 'radiusGroupName', 'radiusExpiration', 'radiusIdleTimeout');
		// help Entries
		$return['help'] = array(
			'radiusFramedIPAddress' => array(
				"Headline" => _("IP address"),
				"Text" => _("This is the IP address for the user (e.g. 123.123.123.123).")
			),
			'radiusFramedIPNetmask' => array(
				"Headline" => _("Net mask"),
				"Text" => _("The net mask for the IP address.")
			),
			'radiusRealm' => array(
				"Headline" => _("Realm"),
				"Text" => _("The Radius realm of this account.")
			),
			'radiusGroupName' => array(
				"Headline" => _("Group names"),
				"Text" => _("The group names for this account.")
			),
			'radiusGroupNameList' => array(
				"Headline" => _("Group names"),
				"Text" => _("The group names for this account.") . ' ' . _("Multiple values are separated by semicolon.")
			),
			'radiusExpiration' => array(
				"Headline" => _("Expiration date"),
				"Text" => _("The account will be locked after this date.")
			),
			'radiusIdleTimeout' => array(
				"Headline" => _("Idle timeout"),
				"Text" => _("Specifies the maximum number of seconds that a connection can be idle before the session is terminated.")
			),
			'hiddenOptions' => array(
				"Headline" => _("Hidden options"),
				"Text" => _("The selected options will not be managed inside LAM. You can use this to reduce the number of displayed input fields.")
			));
		// configuration settings
		$configContainer = new htmlTable();
		$configContainerHead = new htmlTable();
		$configContainerHead->addElement(new htmlOutputText(_('Hidden options')));
		$configContainerHead->addElement(new htmlHelpLink('hiddenOptions'));
		$configContainerOptions = new htmlTable();
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
		$configContainer->addElement($configContainerOptions, true);
		$return['config_options']['all'] = $configContainer;
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
		return $return;
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
				$radiusRealm = '';
				if (isset($this->attributes['radiusRealm'][0])) {
					$radiusRealm = $this->attributes['radiusRealm'][0];
				}
				$return->addElement(new htmlTableExtendedInputField(_('Realm'), 'radiusRealm', $radiusRealm, 'radiusRealm'), true);
			}
			// group names
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusGroupName')) {
				$groups = array();
				if (isset($this->attributes['radiusGroupName'][0])) {
					$groups = $this->attributes['radiusGroupName'];
				}
				if (sizeof($groups) == 0) {
					$groups[] = '';
				}
				$groupLabel = new htmlOutputText(_('Group names'));
				$groupLabel->alignment = htmlElement::ALIGN_TOP;
				$return->addElement($groupLabel);
				$groupContainer = new htmlGroup();
				for ($i = 0; $i < sizeof($groups); $i++) {
					$groupContainer->addElement(new htmlInputField('radiusGroupName' . $i, $groups[$i]));
					if ($i < (sizeof($groups) - 1)) {
						$groupContainer->addElement(new htmlOutputText('<br>', false));
					}
					else {
						$groupContainer->addElement(new htmlButton('addRadiusGroupName', 'add.png', true));
					}
				}
				$return->addElement($groupContainer);
				$groupHelp = new htmlHelpLink('radiusGroupName');
				$groupHelp->alignment = htmlElement::ALIGN_TOP;
				$return->addElement($groupHelp, true);
			}
			// IP address
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPAddress')) {
				$radiusFramedIPAddress = '';
				if (isset($this->attributes['radiusFramedIPAddress'][0])) {
					$radiusFramedIPAddress = $this->attributes['radiusFramedIPAddress'][0];
				}
				$return->addElement(new htmlTableExtendedInputField(_('IP address'), 'radiusFramedIPAddress', $radiusFramedIPAddress, 'radiusFramedIPAddress'), true);
			}
			// net mask
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusFramedIPNetmask')) {
				$radiusFramedIPNetmask = '';
				if (isset($this->attributes['radiusFramedIPNetmask'][0])) {
					$radiusFramedIPNetmask = $this->attributes['radiusFramedIPNetmask'][0];
				}
				$return->addElement(new htmlTableExtendedInputField(_('Net mask'), 'radiusFramedIPNetmask', $radiusFramedIPNetmask, 'radiusFramedIPNetmask'), true);
			}
			// idle timeout
			if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusIdleTimeout')) {
				$radiusIdleTimeout = '';
				if (isset($this->attributes['radiusIdleTimeout'][0])) {
					$radiusIdleTimeout = $this->attributes['radiusIdleTimeout'][0];
				}
				$radiusIdleTimeoutInput = new htmlTableExtendedInputField(_('Idle timeout'), 'radiusIdleTimeout', $radiusIdleTimeout, 'radiusIdleTimeout');
				$radiusIdleTimeoutInput->setValidationRule(htmlElement::VALIDATE_NUMERIC);
				$return->addElement($radiusIdleTimeoutInput, true);
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
			$groupCounter = 0;
			while (isset($_POST['radiusGroupName' . $groupCounter])) {
				$this->attributes['radiusGroupName'][$groupCounter] = $_POST['radiusGroupName' . $groupCounter];
				if ($this->attributes['radiusGroupName'][$groupCounter] == '') {
					unset($this->attributes['radiusGroupName'][$groupCounter]);
				}
				elseif (!get_preg($this->attributes['radiusGroupName'][$groupCounter], 'groupname')) {
					$errors[] = $this->messages['radiusGroupName'][0];
				}
				$groupCounter++;
			}
			if (isset($_POST['addRadiusGroupName'])) {
				$this->attributes['radiusGroupName'][] = '';
			}
			$this->attributes['radiusGroupName'] = array_values(array_unique($this->attributes['radiusGroupName']));
		}
		// idle timeout
		if (!$this->isBooleanConfigOptionSet('freeRadius_hideRadiusIdleTimeout')) {
			$this->attributes['radiusIdleTimeout'][0] = $_POST['radiusIdleTimeout'];
			if (($_POST['radiusIdleTimeout'] != '') && !get_preg($_POST['radiusIdleTimeout'], 'digit')) {
				$errors[] = $this->messages['radiusIdleTimeout'][0];
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
		$date = getdate(time() + 3600*24*365);
		$year = $date['year'];
		$month = str_pad($date['mon'], 2, '0', STR_PAD_LEFT);
		$month = freeRadius::$monthList[$month];
		$day = $date['mday'];
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
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @param array $selectedModules list of selected account modules
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$errors = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("radiusprofile", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "radiusprofile";
			// IP address
			if ($rawAccounts[$i][$ids['freeRadius_radiusFramedIPAddress']] != "") {
				if (get_preg($rawAccounts[$i][$ids['freeRadius_radiusFramedIPAddress']], 'ip')) {
					$partialAccounts[$i]['radiusFramedIPAddress'] = $rawAccounts[$i][$ids['freeRadius_radiusFramedIPAddress']];
				}
				else {
					$errMsg = $this->messages['radiusFramedIPAddress'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// net mask
			if ($rawAccounts[$i][$ids['freeRadius_radiusFramedIPNetmask']] != "") {
				if (get_preg($rawAccounts[$i][$ids['freeRadius_radiusFramedIPNetmask']], 'ip')) {
					$partialAccounts[$i]['radiusFramedIPNetmask'] = $rawAccounts[$i][$ids['freeRadius_radiusFramedIPNetmask']];
				}
				else {
					$errMsg = $this->messages['radiusFramedIPNetmask'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// realm
			if ($rawAccounts[$i][$ids['freeRadius_radiusRealm']] != "") {
				if (get_preg($rawAccounts[$i][$ids['freeRadius_radiusRealm']], 'DNSname')) {
					$partialAccounts[$i]['radiusRealm'] = $rawAccounts[$i][$ids['freeRadius_radiusRealm']];
				}
				else {
					$errMsg = $this->messages['radiusRealm'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// group names
			if ($rawAccounts[$i][$ids['freeRadius_radiusGroupName']] != "") {
				$groupList = preg_split('/;[ ]*/', $rawAccounts[$i][$ids['freeRadius_radiusGroupName']]);
				$partialAccounts[$i]['radiusGroupName'] = $groupList;
				for ($x = 0; $x < sizeof($groupList); $x++) {
					if (!get_preg($groupList[$x], 'groupname')) {
						$errMsg = $this->messages['radiusGroupName'][1];
						array_push($errMsg, array($i));
						$errors[] = $errMsg;
						break;
					}
				}
			}
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
			if ($rawAccounts[$i][$ids['freeRadius_radiusIdleTimeout']] != "") {
				if (get_preg($rawAccounts[$i][$ids['freeRadius_radiusIdleTimeout']], 'digit')) {
					$partialAccounts[$i]['radiusIdleTimeout'] = $rawAccounts[$i][$ids['freeRadius_radiusIdleTimeout']];
				}
				else {
					$errMsg = $this->messages['radiusIdleTimeout'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
		}
		return $errors;
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (isset($this->attributes['radiusFramedIPAddress'][0])) {
			$return[get_class($this) . '_radiusFramedIPAddress'][0] = '<block><key>' . _('IP address') . '</key><value>' . $this->attributes['radiusFramedIPAddress'][0] . '</value></block>';
		}
		if (isset($this->attributes['radiusFramedIPNetmask'][0])) {
			$return[get_class($this) . '_radiusFramedIPNetmask'][0] = '<block><key>' . _('Net mask') . '</key><value>' . $this->attributes['radiusFramedIPNetmask'][0] . '</value></block>';
		}
		if (isset($this->attributes['radiusRealm'][0])) {
			$return[get_class($this) . '_radiusRealm'][0] = '<block><key>' . _('Realm') . '</key><value>' . $this->attributes['radiusRealm'][0] . '</value></block>';
		}
		if (isset($this->attributes['radiusGroupName'][0])) {
			$return[get_class($this) . '_radiusGroupName'][0] = '<block><key>' . _('Group names') . '</key><value>' . implode(', ', $this->attributes['radiusGroupName']) . '</value></block>';
		}
		if (isset($this->attributes['radiusExpiration'][0])) {
			$return[get_class($this) . '_radiusExpiration'][0] = '<block><key>' . _('Expiration date') . '</key><value>' . $this->formatExpirationDate($this->attributes['radiusExpiration'][0]) . '</value></block>';
		}
		if (isset($this->attributes['radiusIdleTimeout'][0])) {
			$return[get_class($this) . '_radiusIdleTimeout'][0] = '<block><key>' . _('Idle timeout') . '</key><value>' . $this->attributes['radiusIdleTimeout'][0] . '</value></block>';
		}
		return $return;
	}

	/**
	* Checks input values of account profiles.
	*
	* @param array $options a hash array (name => value) containing the options
	* @return array list of error messages (array(type, title, text)) to generate StatusMessages, if any
	*/
	function check_profileOptions($options) {
		$messages = parent::check_profileOptions($options);
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
	
}


?>
