<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2009  Roland Gruber

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
* Manages the eduPerson extension for user accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages the eduPerson extension for user accounts.
*
* @package modules
*/
class eduPerson extends baseModule {
	
	private $affiliationTypes = array('faculty', 'student', 'staff', 'alum',
									'member', 'affiliate', 'employee', 'library-walk-in');

	/**
	* Creates a new eduPerson object.
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
		$return['icon'] = 'eduPerson.png';
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("EDU person");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('eduPerson');
		// managed attributes
		$return['attributes'] = array('eduPersonAffiliation', 'eduPersonNickname',
								'eduPersonOrgDN', 'eduPersonOrgUnitDN',
								'eduPersonPrimaryAffiliation', 'eduPersonPrincipalName',
								'eduPersonEntitlement', 'eduPersonPrimaryOrgUnitDN',
								'eduPersonScopedAffiliation');
		// help Entries
		$return['help'] = array(
			'primaryAffiliation' => array(
				"Headline" => _("Primary affiliation"),
				"Text" => _("Specifies the person's primary relationship to the institution in broad categories such as student, faculty, staff, alum, etc.")
			),
			'scopedAffiliation' => array(
				"Headline" => _("Scoped affiliation"),
				"Text" => _("Specifies the person's affiliation within a particular security domain in broad categories such as student, faculty, staff, alum, etc.")
			),
			'nickName' => array(
				"Headline" => _("Nick names"),
				"Text" => _("This is a list of nick names for this user.")
			),
			'affiliation' => array(
				"Headline" => _("Affiliations"),
				"Text" => _("Specifies the person's relationship(s) to the institution in broad categories such as student, faculty, staff, alum, etc.")
			),
			'affiliationUpload' => array(
				"Headline" => _("Affiliations"),
				"Text" => _("Specifies the person's relationship(s) to the institution in broad categories such as student, faculty, staff, alum, etc.") . " " .
							_('Multiple values are separated by comma.')
			),
			'principalName' => array(
				"Headline" => _("Principal name"),
				"Text" => _("The \"NetID\" of the person for the purposes of inter-institutional authentication. It should be represented in the form \"user@scope\" where scope defines a local security domain.")
			),
			'entitlement' => array(
				"Headline" => _("Entitlements"),
				"Text" => _("URI (either URN or URL) that indicates a set of rights to specific resources.")
			),
			'entitlementUpload' => array(
				"Headline" => _("Entitlements"),
				"Text" => _("URI (either URN or URL) that indicates a set of rights to specific resources.") . " " . _('Multiple values are separated by comma.')
			),
			'orgDN' => array(
				"Headline" => _("Organization"),
				"Text" => _("The DN of the directory entry representing the institution with which the person is associated.")
			),
			'primaryOrgUnitDN' => array(
				"Headline" => _("Primary organizational unit"),
				"Text" => _("The DN of the directory entry representing the person's primary organizational unit.")
			),
			'orgUnitDN' => array(
				"Headline" => _("Organizational units"),
				"Text" => _("The DNs of the directory entries representing the person's organizational units.")
			),
			'orgUnitDNUpload' => array(
				"Headline" => _("Organizational units"),
				"Text" => _("The DNs of the directory entries representing the person's organizational units.") . ' ' . _('Multiple values are separated by comma.')
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'eduPerson_principalName',
				'description' => _('Principal name'),
				'help' => 'principalName',
				'example' => _('user@company.com'),
				'unique' => 'true'
			),
			array(
				'name' => 'eduPerson_primaryAffiliation',
				'description' => _('Primary affiliation'),
				'help' => 'primaryAffiliation',
				'example' => 'student',
				'values' => implode(", ", $this->affiliationTypes)
			),
			array(
				'name' => 'eduPerson_scopedAffiliation',
				'description' => _('Scoped affiliation'),
				'help' => 'scopedAffiliation',
				'example' => 'student@domain'
			),
			array(
				'name' => 'eduPerson_affiliation',
				'description' => _('Affiliations'),
				'help' => 'affiliationUpload',
				'example' => 'student, employee',
				'values' => implode(", ", $this->affiliationTypes)
			),
			array(
				'name' => 'eduPerson_nickname',
				'description' => _('Nick names'),
				'help' => 'nickName',
				'example' => _('Steve, Stevo')
			),
			array(
				'name' => 'eduPerson_entitlement',
				'description' => _('Entitlements'),
				'help' => 'entitlementUpload',
				'example' => 'http://xstor.com/contracts/HEd123'
			),
			array(
				'name' => 'eduPerson_orgDN',
				'description' => _('Organization'),
				'help' => 'orgDN',
				'example' => _('ou=accounts,dc=yourdomain,dc=org')
			),
			array(
				'name' => 'eduPerson_primaryOrgUnitDN',
				'description' => _('Primary organizational unit'),
				'help' => 'primaryOrgUnitDN',
				'example' => _('ou=accounts,dc=yourdomain,dc=org')
			),
			array(
				'name' => 'eduPerson_orgUnitDN',
				'description' => _('Organizational units'),
				'help' => 'orgUnitDNUpload',
				'example' => _('ou=accounts,dc=yourdomain,dc=org')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'affiliation', 'nickname', 'orgDN', 'orgUnitDN',
			'primaryAffiliation', 'principalName', 'entitlement',
			'primaryOrgUnitDN', 'scopedAffiliation'
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['principalName'][0] = array('ERROR', _('Principal name is invalid!'));
		$this->messages['principalName'][1] = array('ERROR', _('Account %s:') . ' eduPerson_principalName', _('Principal name is invalid!'));
		$this->messages['orgDN'][0] = array('ERROR', _('Please enter a valid DN in the field:') . ' ' . _("Organization"));
		$this->messages['orgDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_orgDN', _('This is not a valid DN!'));
		$this->messages['primaryOrgUnitDN'][0] = array('ERROR', _('Please enter a valid DN in the field:') . ' ' . _("Primary organizational unit"));
		$this->messages['primaryOrgUnitDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_primaryOrgUnitDN', _('This is not a valid DN!'));
		$this->messages['orgUnitDN'][0] = array('ERROR', _('Organizational units contains an invalid entry.'));
		$this->messages['orgUnitDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_orgUnitDN', _('This is not a valid list of DNs!'));
		$this->messages['primaryAffiliation'][0] = array('ERROR', _('Account %s:') . ' eduPerson_primaryAffiliation', _('Please enter a valid primary affiliation.'));
		$this->messages['scopedAffiliation'][0] = array('ERROR', _('Account %s:') . ' eduPerson_scopedAffiliation', _('Please enter a valid scoped affiliation.'));
		$this->messages['affiliation'][0] = array('ERROR', _('Account %s:') . ' eduPerson_affiliation', _('Please enter a valid list of affiliations.'));
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['form_subpage_eduPerson_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'eduPerson';
		}
		$return = array();
		if (in_array('eduPerson', $this->attributes['objectClass'])) {
			// principal name
			$principal = '';
			if (isset($this->attributes['eduPersonPrincipalName'][0])) {
				$principal = $this->attributes['eduPersonPrincipalName'][0];
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Principal name')),
				array('kind' => 'input', 'type' => 'text', 'name' => 'principalName', 'value' => $principal),
				array('kind' => 'help', 'value' => 'principalName')
			);
			// primary affiliation
			$primaryAffiliation = array();
			if (isset($this->attributes['eduPersonPrimaryAffiliation'][0])) {
				$primaryAffiliation = array($this->attributes['eduPersonPrimaryAffiliation'][0]);
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Primary affiliation')),
				array('kind' => 'select', 'name' => 'primaryAffiliation', 'options' => $this->affiliationTypes, 'options_selected' => $primaryAffiliation),
				array('kind' => 'help', 'value' => 'primaryAffiliation')
			);
			// scoped affiliation
			$scopedAffiliation = '';
			$scopedAffiliationPrefix = array();
			if (isset($this->attributes['eduPersonScopedAffiliation'][0])) {
				$parts = explode('@', $this->attributes['eduPersonScopedAffiliation'][0]);
				$scopedAffiliationPrefix = array($parts[0]);
				$scopedAffiliation = substr($this->attributes['eduPersonScopedAffiliation'][0], strlen($parts[0]) + 1);
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Scoped affiliation')),
				array('kind' => 'table', 'value' => array(array(
					array('kind' => 'select', 'name' => 'scopedAffiliationPrefix', 'options' => $this->affiliationTypes, 'options_selected' => $scopedAffiliationPrefix),
					array('kind' => 'text', 'text' => '@'),
					array('kind' => 'input', 'type' => 'text', 'name' => 'scopedAffiliation', 'value' => $scopedAffiliation)
				))),
				array('kind' => 'help', 'value' => 'scopedAffiliation')
			);
			// affiliations
			$affiliations = array();
			if (isset($this->attributes['eduPersonAffiliation'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['eduPersonAffiliation']); $i++) {
					$affiliations[] = array(
						array('kind' => 'select', 'name' => 'affiliation' . $i, 'options' => $this->affiliationTypes, 'options_selected' => array($this->attributes['eduPersonAffiliation'][$i])),
						array('kind' => 'input', 'type' => 'submit', 'name' => 'delAffiliation' . $i, 'image' => 'del.png', 'value' => ' ', 'title' => _('Delete'))
					);
				}
			}
			else {
				$affiliations = array(array(array('kind' => 'text', 'text' => '-')));
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Affiliations'), 'td' => array('valign' => 'top')),
				array('kind' => 'table', 'value' => $affiliations),
				array('kind' => 'help', 'value' => 'affiliation', 'td' => array('valign' => 'top'))
			);
			$return[] = array(
				array('kind' => 'text', 'text' => ' '),
				array('kind' => 'table', 'value' => array(array(
					array('kind' => 'select', 'name' => 'affiliation', 'options' => $this->affiliationTypes),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'newAffiliation', 'image' => 'add.png', 'value' => ' ', 'title' => _('Add'))
				))),
				array('kind' => 'text', 'text' => ' '));
			// nick names
			$nicks = array();
			if (isset($this->attributes['eduPersonNickname'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['eduPersonNickname']); $i++) {
					$nicks[] = array(
						array('kind' => 'input', 'name' => 'nickName' . $i, 'type' => 'text', 'value' => $this->attributes['eduPersonNickname'][$i]),
						array('kind' => 'input', 'type' => 'submit', 'name' => 'delNickName' . $i, 'image' => 'del.png', 'value' => ' ', 'title' => _('Delete'))
					);
				}
			}
			else {
				$nicks = array(array(array('kind' => 'text', 'text' => '-')));
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Nick names'), 'td' => array('valign' => 'top')),
				array('kind' => 'table', 'value' => $nicks),
				array('kind' => 'help', 'value' => 'nickName', 'td' => array('valign' => 'top'))
			);
			$return[] = array(
				array('kind' => 'text', 'text' => ' '),
				array('kind' => 'table', 'value' => array(array(
					array('kind' => 'input', 'name' => 'nickName', 'type' => 'text', 'value' => ''),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'newNickName', 'image' => 'add.png', 'value' => ' ', 'title' => _('Add'))
				))),
				array('kind' => 'text', 'text' => ' ')
			);
			// entitlements
			$entitlements = array();
			if (isset($this->attributes['eduPersonEntitlement'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['eduPersonEntitlement']); $i++) {
					$entitlements[] = array(
						array('kind' => 'input', 'name' => 'entitlement' . $i, 'type' => 'text', 'value' => $this->attributes['eduPersonEntitlement'][$i]),
						array('kind' => 'input', 'type' => 'submit', 'name' => 'delEntitlement' . $i, 'image' => 'del.png', 'value' => ' ', 'title' => _('Delete'))
					);
				}
			}
			else {
				$entitlements = array(array(array('kind' => 'text', 'text' => '-')));
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Entitlements'), 'td' => array('valign' => 'top')),
				array('kind' => 'table', 'value' => $entitlements),
				array('kind' => 'help', 'value' => 'entitlement', 'td' => array('valign' => 'top'))
			);
			$return[] = array(
				array('kind' => 'text', 'text' => ' '),
				array('kind' => 'table', 'value' => array(array(
					array('kind' => 'input', 'name' => 'entitlement', 'type' => 'text', 'value' => ''),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'newEntitlement', 'image' => 'add.png', 'value' => ' ', 'title' => _('Add'))
				))),
				array('kind' => 'text', 'text' => ' ')
			);
			// org DN
			$orgDN = '';
			if (isset($this->attributes['eduPersonOrgDN'][0])) {
				$orgDN = $this->attributes['eduPersonOrgDN'][0];
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Organization')),
				array('kind' => 'input', 'type' => 'text', 'size' => '40', 'name' => 'orgDN', 'value' => $orgDN),
				array('kind' => 'help', 'value' => 'orgDN')
			);
			// primare OU DN
			$primaryOrgUnitDN = '';
			if (isset($this->attributes['eduPersonPrimaryOrgUnitDN'][0])) {
				$primaryOrgUnitDN = $this->attributes['eduPersonPrimaryOrgUnitDN'][0];
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Primary organizational unit')),
				array('kind' => 'input', 'type' => 'text', 'size' => '40', 'name' => 'primaryOrgUnitDN', 'value' => $primaryOrgUnitDN),
				array('kind' => 'help', 'value' => 'primaryOrgUnitDN')
			);
			// OUs
			$orgUnitDN = array();
			if (isset($this->attributes['eduPersonOrgUnitDN'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['eduPersonOrgUnitDN']); $i++) {
					$orgUnitDN[] = array(
						array('kind' => 'input', 'name' => 'orgUnitDN' . $i, 'type' => 'text', 'value' => $this->attributes['eduPersonOrgUnitDN'][$i], 'size' => '40'),
						array('kind' => 'input', 'type' => 'submit', 'name' => 'delOrgUnitDN' . $i, 'image' => 'del.png', 'value' => ' ', 'title' => _('Delete'))
					);
				}
			}
			else {
				$orgUnitDN = array(array(array('kind' => 'text', 'text' => '-')));
			}
			$return[] = array(
				array('kind' => 'text', 'text' => _('Organizational units'), 'td' => array('valign' => 'top')),
				array('kind' => 'table', 'value' => $orgUnitDN),
				array('kind' => 'help', 'value' => 'orgUnitDN', 'td' => array('valign' => 'top'))
			);
			$return[] = array(
				array('kind' => 'text', 'text' => ' '),
				array('kind' => 'table', 'value' => array(array(
					array('kind' => 'input', 'name' => 'orgUnitDN', 'type' => 'text', 'value' => '', 'size' => '40'),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'newOrgUnitDN', 'image' => 'add.png', 'value' => ' ', 'title' => _('Add'))
				))),
				array('kind' => 'text', 'text' => ' ')
			);
		}
		else {
			$return[] = array(
				array('kind' => 'text', 'text' => '&nbsp;'),
				array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_eduPerson_attributes_addObjectClass', 'value' => _('Add EDU person extension'))
			);
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
		// primary affiliation
		$this->attributes['eduPersonPrimaryAffiliation'][0] = $_POST['primaryAffiliation'];
		// scoped affiliation
		if (isset($_POST['scopedAffiliation']) && ($_POST['scopedAffiliation'] != '')) {
			$this->attributes['eduPersonScopedAffiliation'][0] = $_POST['scopedAffiliationPrefix'] . '@' . $_POST['scopedAffiliation'];
		}
		elseif (isset($this->attributes['eduPersonScopedAffiliation'][0])) {
			unset($this->attributes['eduPersonScopedAffiliation']);
		}
		// principal name
		$this->attributes['eduPersonPrincipalName'][0] = $_POST['principalName'];
		if ($_POST['principalName'] != '') {
			if (!preg_match('/^[0-9a-z_\\.@-]+$/i', $_POST['principalName'])) {
				$errors[] = $this->messages['principalName'][0];
			}
		}
		// affiliations
		$this->attributes['eduPersonAffiliation'] = array();
		$i = 0;
		while (isset($_POST['affiliation' . $i])) {
			if (!isset($_POST['delAffiliation' . $i]) && ($_POST['affiliation' . $i] != '')) {
				$this->attributes['eduPersonAffiliation'][] = $_POST['affiliation' . $i];
			}
			$i++;
		}
		if (isset($_POST['newAffiliation']) && ($_POST['affiliation'] != '')) {
			$this->attributes['eduPersonAffiliation'][] = $_POST['affiliation'];
		}
		$this->attributes['eduPersonAffiliation'] = array_unique($this->attributes['eduPersonAffiliation']);
		// nick names
		$this->attributes['eduPersonNickname'] = array();
		$i = 0;
		while (isset($_POST['nickName' . $i])) {
			if (!isset($_POST['delNickName' . $i]) && ($_POST['nickName' . $i] != '')) {
				$this->attributes['eduPersonNickname'][] = $_POST['nickName' . $i];
			}
			$i++;
		}
		if (isset($_POST['newNickName']) && ($_POST['nickName'] != '')) {
			$this->attributes['eduPersonNickname'][] = $_POST['nickName'];
		}
		$this->attributes['eduPersonNickname'] = array_unique($this->attributes['eduPersonNickname']);
		// entitlements
		$this->attributes['eduPersonEntitlement'] = array();
		$i = 0;
		while (isset($_POST['entitlement' . $i])) {
			if (!isset($_POST['delEntitlement' . $i]) && ($_POST['entitlement' . $i] != '')) {
				$this->attributes['eduPersonEntitlement'][] = $_POST['entitlement' . $i];
			}
			$i++;
		}
		if (isset($_POST['newEntitlement']) && ($_POST['entitlement'] != '')) {
			$this->attributes['eduPersonEntitlement'][] = $_POST['entitlement'];
		}
		$this->attributes['eduPersonEntitlement'] = array_unique($this->attributes['eduPersonEntitlement']);
		// org DN
		$this->attributes['eduPersonOrgDN'][0] = $_POST['orgDN'];
		if ($_POST['orgDN'] != '') {
			if (!get_preg($_POST['orgDN'], 'dn')) {
				$errors[] = $this->messages['orgDN'][0];
			}
		}
		// primary OU
		$this->attributes['eduPersonPrimaryOrgUnitDN'][0] = $_POST['primaryOrgUnitDN'];
		if ($_POST['primaryOrgUnitDN'] != '') {
			if (!get_preg($_POST['primaryOrgUnitDN'], 'dn')) {
				$errors[] = $this->messages['primaryOrgUnitDN'][0];
			}
		}
		// OUs
		$this->attributes['eduPersonOrgUnitDN'] = array();
		$i = 0;
		while (isset($_POST['orgUnitDN' . $i])) {
			if (!isset($_POST['delOrgUnitDN' . $i]) && ($_POST['orgUnitDN' . $i] != '')) {
				$this->attributes['eduPersonOrgUnitDN'][] = $_POST['orgUnitDN' . $i];
				if (!get_preg($_POST['orgUnitDN' . $i], 'dn')) {
					$error = $this->messages['orgUnitDN'][0];
					array_push($error, $_POST['orgUnitDN' . $i]);
					$errors[] = $error;
				}
			}
			$i++;
		}
		if (isset($_POST['newOrgUnitDN']) && ($_POST['orgUnitDN'] != '')) {
			$this->attributes['eduPersonOrgUnitDN'][] = $_POST['orgUnitDN'];
			if (!get_preg($_POST['orgUnitDN'], 'dn')) {
				$error = $this->messages['orgUnitDN'][0];
				array_push($error, $_POST['orgUnitDN']);
				$errors[] = $error;
			}
		}
		$this->attributes['eduPersonOrgUnitDN'] = array_unique($this->attributes['eduPersonOrgUnitDN']);
		return $errors;
	}

	/**
	* In this function the LDAP account is built up.
	*
	* @param array $rawAccounts list of hash arrays (name => value) from user input
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @return array list of error messages if any
	*/
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("eduPerson", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "eduPerson";
			// principal name
			if ($rawAccounts[$i][$ids['eduPerson_principalName']] != "") {
				if (!preg_match('/^[0-9a-z_\\.@-]+$/i', $rawAccounts[$i][$ids['eduPerson_principalName']])) {
					$error = $this->messages['principalName'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
				else {
					$partialAccounts[$i]['eduPersonPrincipalName'] = $rawAccounts[$i][$ids['eduPerson_principalName']];
				}
			}
			// primary affiliation
			if ($rawAccounts[$i][$ids['eduPerson_primaryAffiliation']] != "") {
				if (!in_array($rawAccounts[$i][$ids['eduPerson_primaryAffiliation']], $this->affiliationTypes)) {
					$error = $this->messages['primaryAffiliation'][0];
					array_push($error, $i);
					$messages[] = $error;
				}
				else {
					$partialAccounts[$i]['eduPersonPrimaryAffiliation'] = $rawAccounts[$i][$ids['eduPerson_primaryAffiliation']];
				}
			}
			// scoped affiliation
			if ($rawAccounts[$i][$ids['eduPerson_scopedAffiliation']] != "") {
				$parts = explode('@', $rawAccounts[$i][$ids['eduPerson_scopedAffiliation']]);
				if (!in_array($parts[0], $this->affiliationTypes)) {
					$error = $this->messages['scopedAffiliation'][0];
					array_push($error, $i);
					$messages[] = $error;
				}
				else {
					$partialAccounts[$i]['eduPersonScopedAffiliation'] = $rawAccounts[$i][$ids['eduPerson_scopedAffiliation']];
				}
			}
			// affiliations
			if ($rawAccounts[$i][$ids['eduPerson_affiliation']] != "") {
				$parts = explode(', ', $rawAccounts[$i][$ids['eduPerson_affiliation']]);
				$valid = true;
				for ($a = 0; $a < sizeof($parts); $a++) {
					if (!in_array($parts[$a], $this->affiliationTypes)) {
						$valid = false;
						$error = $this->messages['affiliation'][0];
						array_push($error, $i);
						$messages[] = $error;
						break;
					}
				}
				if ($valid) {
					$partialAccounts[$i]['eduPersonAffiliation'] = $parts;
				}
			}
			// nick names
			if ($rawAccounts[$i][$ids['eduPerson_nickname']] != "") {
				$parts = explode(', ', $rawAccounts[$i][$ids['eduPerson_nickname']]);
				$partialAccounts[$i]['eduPersonNickname'] = $parts;
			}
			// entitlements
			if ($rawAccounts[$i][$ids['eduPerson_entitlement']] != "") {
				$parts = explode(', ', $rawAccounts[$i][$ids['eduPerson_entitlement']]);
				$partialAccounts[$i]['eduPersonEntitlement'] = $parts;
			}
			// organization
			if ($rawAccounts[$i][$ids['eduPerson_orgDN']] != "") {
				if (!get_preg($rawAccounts[$i][$ids['eduPerson_orgDN']], 'dn')) {
					$error = $this->messages['orgDN'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
				else {
					$partialAccounts[$i]['eduPersonOrgDN'] = $rawAccounts[$i][$ids['eduPerson_orgDN']];
				}
			}
			// primary OU
			if ($rawAccounts[$i][$ids['eduPerson_primaryOrgUnitDN']] != "") {
				if (!get_preg($rawAccounts[$i][$ids['eduPerson_primaryOrgUnitDN']], 'dn')) {
					$error = $this->messages['primaryOrgUnitDN'][1];
					array_push($error, $i);
					$messages[] = $error;
				}
				else {
					$partialAccounts[$i]['eduPersonPrimaryOrgUnitDN'] = $rawAccounts[$i][$ids['eduPerson_primaryOrgUnitDN']];
				}
			}
			// OUs
			if ($rawAccounts[$i][$ids['eduPerson_orgUnitDN']] != "") {
				$parts = explode(', ', $rawAccounts[$i][$ids['eduPerson_orgUnitDN']]);
				$valid = true;
				for ($a = 0; $a < sizeof($parts); $a++) {
					if (!get_preg($parts[$a], 'dn')) {
						$valid = false;
						$error = $this->messages['orgUnitDN'][1];
						array_push($error, $i);
						$messages[] = $error;
						break;
					}
				}
				if ($valid) {
					$partialAccounts[$i]['eduPersonOrgUnitDN'] = $parts;
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
		if (isset($this->attributes['eduPersonPrimaryAffiliation'][0])) {
			$return['eduPerson_primaryAffiliation'][0] = '<block><key>' . _('Primary affiliation') . '</key><value>' . $this->attributes['eduPersonPrimaryAffiliation'][0] . '</value></block>';
		}
		if (isset($this->attributes['eduPersonScopedAffiliation'][0])) {
			$return['eduPerson_scopedAffiliation'][0] = '<block><key>' . _('Scoped affiliation') . '</key><value>' . $this->attributes['eduPersonScopedAffiliation'][0] . '</value></block>';
		}
		if (isset($this->attributes['eduPersonPrincipalName'][0])) {
			$return['eduPerson_principalName'][0] = '<block><key>' . _('Principal name') . '</key><value>' . $this->attributes['eduPersonPrincipalName'][0] . '</value></block>';
		}
		if (isset($this->attributes['eduPersonNickname'][0])) {
			sort($this->attributes['eduPersonNickname']);
			$return['eduPerson_nickname'][0] = '<block><key>' . _('Nick names') . '</key><value>' . implode(', ', $this->attributes['eduPersonNickname']) . '</value></block>';
		}
		if (isset($this->attributes['eduPersonAffiliation'][0])) {
			sort($this->attributes['eduPersonAffiliation']);
			$return['eduPerson_affiliation'][0] = '<block><key>' . _('Affiliations') . '</key><value>' . implode(', ', $this->attributes['eduPersonAffiliation']) . '</value></block>';
		}
		if (isset($this->attributes['eduPersonEntitlement'][0])) {
			sort($this->attributes['eduPersonEntitlement']);
			$return['eduPerson_entitlement'][0] = '<block><key>' . _('Entitlements') . '</key><value>' . implode(', ', $this->attributes['eduPersonEntitlement']) . '</value></block>';
		}
		if (isset($this->attributes['eduPersonOrgDN'][0])) {
			$return['eduPerson_orgDN'][0] = '<block><key>' . _('Organization') . '</key><value>' . $this->attributes['eduPersonOrgDN'][0] . '</value></block>';
		}
		if (isset($this->attributes['eduPersonPrimaryOrgUnitDN'][0])) {
			$return['eduPerson_primaryOrgUnitDN'][0] = '<block><key>' . _('Primary organizational unit') . '</key><value>' . $this->attributes['eduPersonPrimaryOrgUnitDN'][0] . '</value></block>';
		}
		if (isset($this->attributes['eduPersonOrgUnitDN'][0])) {
			$return['eduPerson_orgUnitDN'][0] = '<block><key>' . _('Organizational units') . '</key><value>' . implode(', ', $this->attributes['eduPersonOrgUnitDN']) . '</value></block>';
		}
		return $return;
	}

}


?>
