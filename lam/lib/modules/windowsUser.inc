<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2013  Roland Gruber

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
* Manages Windows AD (e.g. Samba 4) users.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Windows AD (e.g. Samba 4) users.
*
* @package modules
*/
class windowsUser extends baseModule {
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_metaData() {
		$return = array();
		// icon
		$return['icon'] = 'samba.png';
		// manages user accounts
		$return["account_types"] = array('user');
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("cn" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=user)");
		// alias name
		$return["alias"] = _("Windows");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('user');
		// managed attributes
		$return['attributes'] = array('cn', 'sAMAccountName', 'description', 'displayName', 'givenName', 'initials',
			'l', 'mail', 'otherTelephone', 'physicalDeliveryOfficeName', 'postalCode', 'postOfficeBox', 'sn', 'st',
			'streetAddress', 'telephoneNumber', 'url', 'wWWHomePage');
		// help Entries
		$return['help'] = array(
			'cn' => array(
				"Headline" => _('User name'), 'attr' => 'cn, sAMAccountName',
				"Text" => _('Please enter the user\'s name.')
			),
			'description' => array(
				"Headline" => _('Description'), 'attr' => 'description',
				"Text" => _('Please enter a descriptive text for this user.')
			),
			'displayName' => array(
				"Headline" => _('Display name'), 'attr' => 'displayName',
				"Text" => _('This is the account\'s full name on Windows systems.')
			),
			'givenName' => array(
				"Headline" => _('First name'), 'attr' => 'givenName',
				"Text" => _('First name of user. Only letters, - and spaces are allowed.')
			),
			'initials' => array(
				"Headline" => _('Initials'), 'attr' => 'initials',
				"Text" => _('The initials of the user\'s first names.')
			),
			'l' => array(
				"Headline" => _('Location'), 'attr' => 'l',
				"Text" => _('This describes the location of the user.')
			),
			'mail' => array(
				"Headline" => _('Email address'), 'attr' => 'mail',
				"Text" => _('The user\'s email address.')
			),
			'otherTelephone' => array(
				"Headline" => _('Other telephone numbers'), 'attr' => 'otherTelephone',
				"Text" => _('If the user has multiple telephone numbers then please enter it here.')
			),
			'physicalDeliveryOfficeName' => array(
				"Headline" => _('Office name'), 'attr' => 'physicalDeliveryOfficeName',
				"Text" => _('The office name of the user (e.g. YourCompany, Human Resources).')
			),
			'postalCode' => array(
				"Headline" => _('Postal code'), 'attr' => 'postalCode',
				"Text" => _('The postal code of the user\'s address.')
			),
			'postOfficeBox' => array(
				"Headline" => _('Post office box'), 'attr' => 'postOfficeBox',
				"Text" => _('The post office box of the user\'s address.')
			),
			'sn' => array(
				"Headline" => _('Last name'), 'attr' => 'sn',
				"Text" => _('Last name of user. Only letters, - and spaces are allowed.')
			),
			'st' => array(
				"Headline" => _('State'), 'attr' => 'st',
				"Text" => _('The state where the user resides or works.')
			),
			'streetAddress' => array(
				"Headline" => _('Street'), 'attr' => 'streetAddress',
				"Text" => _('The street name of the user\'s address.')
			),
			'telephoneNumber' => array(
				"Headline" => _('Telephone number'), 'attr' => 'telephoneNumber',
				"Text" => _('The user\'s telephone number.')
			),
			'url' => array(
				"Headline" => _('Other web sites'), 'attr' => 'url',
				"Text" => _('Here you can enter additional web sites for the user.')
			),
			'wWWHomePage' => array(
				"Headline" => _('Web site'), 'attr' => 'wWWHomePage',
				"Text" => _('The user\'s web site (e.g. http://www.company.com).')
			),
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'windowsHost_name',
				'description' => _('Host name'),
				'help' => 'cn',
				'example' => _('PC01'),
				'required' => true
			),
			array(
				'name' => 'windowsHost_description',
				'description' => _('Description'),
				'help' => 'description',
			),
			array(
				'name' => 'windowsHost_location',
				'description' => _('Location'),
				'help' => 'location',
				'example' => _('MyCity'),
			),
			array(
				'name' => 'windowsHost_managedBy',
				'description' => _('Managed by'),
				'help' => 'managedBy',
				'example' => 'cn=user1,o=test',
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'cn' => _('User name'),
			'description' => _('Description'),
			'displayName' => _('Display name'),
			'givenName' => _('First name'),
			'initials' => _('Initials'),
			'l' => _('Location'),
			'mail' => _('Email address'),
			'otherTelephone' => _('Other telephone numbers'),
			'physicalDeliveryOfficeName' => _('Office name'),
			'postalCode' => _('Postal code'),
			'postOfficeBox' => _('Post office box'),
			'sn' => _('Last name'),
			'st' => _('State'),
			'streetAddress' => _('Street'),
			'telephoneNumber' => _('Telephone number'),
			'url' => _('Other web sites'),
			'wWWHomePage' => _('Web site'),
		);
		return $return;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	public function load_Messages() {
		$this->messages['cn'][0] = array('ERROR', _('User name'), _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['cn'][1] = array('ERROR', _('Account %s:') . ' windowsUser_cn', _('User name contains invalid characters. Valid characters are: a-z, A-Z, 0-9 and .-_ !'));
		$this->messages['displayName'][0] = array('ERROR', _('Display name'), _('Please enter a valid display name!'));
		$this->messages['displayName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_displayName', _('Please enter a valid display name!'));
		$this->messages['givenName'][0] = array('ERROR', _('First name'), _('First name contains invalid characters!'));
		$this->messages['givenName'][1] = array('ERROR', _('Account %s:') . ' windowsUser_givenName', _('First name contains invalid characters!'));
		$this->messages['sn'][0] = array('ERROR', _('Last name'), _('Last name contains invalid characters or is empty!'));
		$this->messages['sn'][1] = array('ERROR', _('Account %s:') . ' windowsUser_sn', _('Last name contains invalid characters or is empty!'));
		$this->messages['telephoneNumber'][0] = array('ERROR', _('Telephone number'), _('Please enter a valid telephone number!'));
		$this->messages['telephoneNumber'][1] = array('ERROR', _('Account %s:') . ' windowsUser_telephone', _('Please enter a valid telephone number!'));
		$this->messages['otherTelephone'][0] = array('ERROR', _('Other telephone numbers'), _('Please enter a valid telephone number!'));
		$this->messages['otherTelephone'][1] = array('ERROR', _('Account %s:') . ' windowsUser_otherTelephone', _('Please enter a valid telephone number!'));
		$this->messages['postalCode'][0] = array('ERROR', _('Postal code'), _('Please enter a valid postal code!'));
		$this->messages['postalCode'][1] = array('ERROR', _('Account %s:') . ' windowsUser_postalCode', _('Please enter a valid postal code!'));
		$this->messages['mail'][0] = array('ERROR', _('Email address'), _('Please enter a valid email address!'));
		$this->messages['mail'][1] = array('ERROR', _('Account %s:') . ' windowsUser_mail', _('Please enter a valid email address!'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	public function display_html_attributes() {
		$container = new htmlTable();
		$this->addSimpleInputTextField($container, 'cn', _('User name'), true);
		$this->addSimpleInputTextField($container, 'givenName', _('First name'));
		$this->addSimpleInputTextField($container, 'sn', _('Last name'));
		$this->addSimpleInputTextField($container, 'displayName', _('Display name'));
		$this->addSimpleInputTextField($container, 'initials', _('Initials'));
		$this->addSimpleInputTextField($container, 'description', _('Description'));
		$container->addElement(new htmlSubTitle(_('Address')), true);
		$this->addSimpleInputTextField($container, 'streetAddress', _('Street'), false, 20, true);
		$this->addSimpleInputTextField($container, 'postOfficeBox', _('Post office box'));
		$this->addSimpleInputTextField($container, 'postalCode', _('Postal code'));
		$this->addSimpleInputTextField($container, 'l', _('Location'));
		$this->addSimpleInputTextField($container, 'st', _('State'));
		$this->addSimpleInputTextField($container, 'physicalDeliveryOfficeName', _('Office name'));
		$container->addElement(new htmlSubTitle(_('Contact data')), true);
		$this->addSimpleInputTextField($container, 'mail', _('Email address'));
		$this->addSimpleInputTextField($container, 'telephoneNumber', _('Telephone number'));
		$this->addMultiValueInputTextField($container, 'otherTelephone', _('Other telephone numbers'));
		$this->addSimpleInputTextField($container, 'wWWHomePage', _('Web site'));
		$this->addMultiValueInputTextField($container, 'url', _('Other web sites'));
		
		$container->addElement(new htmlEqualWidth(array('streetAddress', 'cn')));
		return $container;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @return array list of info/error messages
	*/
	public function process_attributes() {
		$return = array();
		// cn
		$this->attributes['cn'][0] = $_POST['cn'];
		$this->attributes['sAMAccountName'][0] = $_POST['cn'];
		if (!get_preg($_POST['cn'], 'username')) {
			$return[] = $this->messages['cn'][0];
		}
		// description
		$this->attributes['description'][0] = $_POST['description'];
		// display name
		$this->attributes['displayName'][0] = $_POST['displayName'];
		if (!empty($this->attributes['displayName'][0]) && !get_preg($_POST['displayName'], 'realname')) {
			$return[] = $this->messages['displayName'][0];
		}
		// first name
		$this->attributes['givenName'][0] = $_POST['givenName'];
		if (!empty($this->attributes['givenName'][0]) && !get_preg($_POST['givenName'], 'realname')) {
			$return[] = $this->messages['givenName'][0];
		}
		// initials
		$this->attributes['initials'][0] = $_POST['initials'];
		// location
		$this->attributes['l'][0] = $_POST['l'];
		// email
		$this->attributes['mail'][0] = $_POST['mail'];
		if (!empty($this->attributes['mail'][0]) && !get_preg($_POST['mail'], 'email')) {
			$return[] = $this->messages['mail'][0];
		}
		// other telephones
		$this->processMultiValueInputTextField('otherTelephone', $return, 'telephone');
		// office name
		$this->attributes['physicalDeliveryOfficeName'][0] = $_POST['physicalDeliveryOfficeName'];
		// postal code
		$this->attributes['postalCode'][0] = $_POST['postalCode'];
		if (!get_preg($_POST['postalCode'], 'postalCode')) {
			$return[] = $this->messages['postalCode'][0];
		}
		// post office box
		$this->attributes['postOfficeBox'][0] = $_POST['postOfficeBox'];
		// last name
		$this->attributes['sn'][0] = $_POST['sn'];
		if (!empty($this->attributes['sn'][0]) && !get_preg($_POST['sn'], 'realname')) {
			$return[] = $this->messages['sn'][0];
		}
		// state
		$this->attributes['st'][0] = $_POST['st'];
		// street
		$this->attributes['streetAddress'][0] = $_POST['streetAddress'];
		// telephone
		$this->attributes['telephoneNumber'][0] = $_POST['telephoneNumber'];
		if (!get_preg($_POST['telephoneNumber'], 'telephone')) {
			$return[] = $this->messages['telephoneNumber'][0];
		}
		// other web sites
		$this->processMultiValueInputTextField('url', $return);
		// web site
		$this->attributes['wWWHomePage'][0] = $_POST['wWWHomePage'];
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
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules) {
		$errors = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array('computer', $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = 'computer';
			// cn + sAMAccountName
			if ($rawAccounts[$i][$ids['windowsHost_name']] != "") {
				if (get_preg($rawAccounts[$i][$ids['windowsHost_name']], 'hostname')) {
					$partialAccounts[$i]['cn'] = $rawAccounts[$i][$ids['windowsHost_name']];
					$partialAccounts[$i]['sAMAccountName'] = $rawAccounts[$i][$ids['windowsHost_name']] . '$';
				}
				else {
					$errMsg = $this->messages['cn'][1];
					array_push($errMsg, array($i));
					$errors[] = $errMsg;
				}
			}
			// description
			if ($rawAccounts[$i][$ids['windowsHost_description']] != "") {
				$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['windowsHost_description']];
			}
			// location
			if ($rawAccounts[$i][$ids['windowsHost_location']] != "") {
				$partialAccounts[$i]['location'] = $rawAccounts[$i][$ids['windowsHost_location']];
			}
			// managed by
			if ($rawAccounts[$i][$ids['windowsHost_managedBy']] != "") {
				$partialAccounts[$i]['managedBy'] = $rawAccounts[$i][$ids['windowsHost_managedBy']];
			}
			// machine trust account, no password required
			$partialAccounts[$i]['userAccountControl'][0] = 4128;
		}
		return $errors;
	}
	
	/**
	* Returns a list of PDF entries
	*/
	public function get_pdfEntries() {
		$return = array();
		$this->addSimplePDFField($return, 'cn', _('User name'));
		$this->addSimplePDFField($return, 'description', _('Description'));
		$this->addSimplePDFField($return, 'displayName', _('Display name'));
		$this->addSimplePDFField($return, 'givenName', _('First name'));
		$this->addSimplePDFField($return, 'initials', _('Initials'));
		$this->addSimplePDFField($return, 'l', _('Location'));
		$this->addSimplePDFField($return, 'mail', _('Email address'));
		$this->addSimplePDFField($return, 'otherTelephone', _('Other telephone numbers'));
		$this->addSimplePDFField($return, 'physicalDeliveryOfficeName', _('Office name'));
		$this->addSimplePDFField($return, 'postalCode', _('Postal code'));
		$this->addSimplePDFField($return, 'postOfficeBox', _('Post office box'));
		$this->addSimplePDFField($return, 'sn', _('Last name'));
		$this->addSimplePDFField($return, 'st', _('State'));
		$this->addSimplePDFField($return, 'streetAddress', _('Street'));
		$this->addSimplePDFField($return, 'telephoneNumber', _('Telephone number'));
		$this->addSimplePDFField($return, 'url', _('Other web sites'));
		$this->addSimplePDFField($return, 'wWWHomePage', _('Web site'));
		return $return;
	}

}


?>
