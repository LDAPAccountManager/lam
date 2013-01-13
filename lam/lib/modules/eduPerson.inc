<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2013  Roland Gruber

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
	
	/** possible affiliation types */
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
								'eduPersonScopedAffiliation', 'eduPersonAssurance');
		// RDN attribute
		$return["RDN"] = array('eduPersonPrincipalName' => 'low');
		// help Entries
		$return['help'] = array(
			'primaryAffiliation' => array(
				"Headline" => _("Primary affiliation"), 'attr' => 'eduPersonPrimaryAffiliation',
				"Text" => _("Specifies the person's primary relationship to the institution in broad categories such as student, faculty, staff, alum, etc.")
			),
			'scopedAffiliation' => array(
				"Headline" => _("Scoped affiliations"), 'attr' => 'eduPersonScopedAffiliation',
				"Text" => _("Specifies the person's affiliation within a particular security domain in broad categories such as student, faculty, staff, alum, etc.")
			),
			'scopedAffiliationUpload' => array(
				"Headline" => _("Scoped affiliations"), 'attr' => 'eduPersonScopedAffiliation',
				"Text" => _("Specifies the person's affiliation within a particular security domain in broad categories such as student, faculty, staff, alum, etc.") . " " .
							_('Multiple values are separated by comma.')
			),
			'nickName' => array(
				"Headline" => _("Nick names"), 'attr' => 'eduPersonNickname',
				"Text" => _("This is a list of nick names for this user.")
			),
			'affiliation' => array(
				"Headline" => _("Affiliations"), 'attr' => 'eduPersonAffiliation',
				"Text" => _("Specifies the person's relationships to the institution in broad categories such as student, faculty, staff, alum, etc.")
			),
			'affiliationUpload' => array(
				"Headline" => _("Affiliations"), 'attr' => 'eduPersonAffiliation',
				"Text" => _("Specifies the person's relationships to the institution in broad categories such as student, faculty, staff, alum, etc.") . " " .
							_('Multiple values are separated by comma.')
			),
			'principalName' => array(
				"Headline" => _("Principal name"), 'attr' => 'eduPersonPrincipalName',
				"Text" => _("The \"NetID\" of the person for the purposes of inter-institutional authentication. It should be represented in the form \"user@scope\" where scope defines a local security domain.")
			),
			'entitlement' => array(
				"Headline" => _("Entitlements"), 'attr' => 'eduPersonEntitlement',
				"Text" => _("URI (either URN or URL) that indicates a set of rights to specific resources.")
			),
			'entitlementUpload' => array(
				"Headline" => _("Entitlements"), 'attr' => 'eduPersonEntitlement',
				"Text" => _("URI (either URN or URL) that indicates a set of rights to specific resources.") . " " . _('Multiple values are separated by comma.')
			),
			'orgDN' => array(
				"Headline" => _("Organisation"), 'attr' => 'eduPersonOrgDN',
				"Text" => _("The DN of the directory entry representing the institution with which the person is associated.")
			),
			'primaryOrgUnitDN' => array(
				"Headline" => _("Primary organisational unit"), 'attr' => 'eduPersonPrimaryOrgUnitDN',
				"Text" => _("The DN of the directory entry representing the person's primary organisational unit.")
			),
			'orgUnitDN' => array(
				"Headline" => _("Organisational units"), 'attr' => 'eduPersonOrgUnitDN',
				"Text" => _("The DNs of the directory entries representing the person's organisational units.")
			),
			'orgUnitDNUpload' => array(
				"Headline" => _("Organisational units"), 'attr' => 'eduPersonOrgUnitDN',
				"Text" => _("The DNs of the directory entries representing the person's organisational units.") . ' ' . _('Multiple values are separated by comma.')
			),
			'eduPersonAssurance' => array(
				"Headline" => _('Assurance profiles'), 'attr' => 'eduPersonAssurance',
				"Text" => _('Assurance profiles are the set of standards that are met by an identity assertion.')
			),
			'eduPersonAssuranceUpload' => array(
				"Headline" => _('Assurance profiles'), 'attr' => 'eduPersonAssurance',
				"Text" => _('Assurance profiles are the set of standards that are met by an identity assertion.') . " " . _('Multiple values are separated by comma.')
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			));
		// profile options
		$profileContainer = new htmlTable();
		$profileContainer->addElement(new htmlTableExtendedInputCheckbox('eduPerson_addExt', false, _('Automatically add this extension'), 'autoAdd'));
		$return['profile_options'] = $profileContainer;
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
				'description' => _('Scoped affiliations'),
				'help' => 'scopedAffiliationUpload',
				'example' => 'student@domain, student@domain2'
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
				'description' => _('Organisation'),
				'help' => 'orgDN',
				'example' => _('ou=accounts,dc=yourdomain,dc=org')
			),
			array(
				'name' => 'eduPerson_primaryOrgUnitDN',
				'description' => _('Primary organisational unit'),
				'help' => 'primaryOrgUnitDN',
				'example' => _('ou=accounts,dc=yourdomain,dc=org')
			),
			array(
				'name' => 'eduPerson_orgUnitDN',
				'description' => _('Organisational units'),
				'help' => 'orgUnitDNUpload',
				'example' => _('ou=accounts,dc=yourdomain,dc=org')
			),
			array(
				'name' => 'eduPerson_assurance',
				'description' => _('Assurance profiles'),
				'help' => 'eduPersonAssuranceUpload',
				'example' => 'urn:mace:incommon:IAQ:sample, http://idm.example.org/LOA#sample'
			),
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'affiliation' => _('Affiliations'),
			'nickname' => _('Nick names'),
			'orgDN' => _('Organisation'),
			'orgUnitDN' => _('Organisational units'),
			'primaryAffiliation' => _('Primary affiliation'),
			'principalName' => _('Principal name'),
			'entitlement' => _('Entitlements'),
			'primaryOrgUnitDN' => _('Primary organisational unit'),
			'scopedAffiliation' => _('Scoped affiliations'),
			'eduPersonAssurance' => _('Assurance profiles'),
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['principalName'][0] = array('ERROR', _('Principal name is invalid!'));
		$this->messages['principalName'][1] = array('ERROR', _('Account %s:') . ' eduPerson_principalName', _('Principal name is invalid!'));
		$this->messages['orgDN'][0] = array('ERROR', _('Please enter a valid DN in the field:') . ' ' . _("Organisation"));
		$this->messages['orgDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_orgDN', _('This is not a valid DN!'));
		$this->messages['primaryOrgUnitDN'][0] = array('ERROR', _('Please enter a valid DN in the field:') . ' ' . _("Primary organisational unit"));
		$this->messages['primaryOrgUnitDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_primaryOrgUnitDN', _('This is not a valid DN!'));
		$this->messages['orgUnitDN'][0] = array('ERROR', _('Organisational units contains an invalid entry.'));
		$this->messages['orgUnitDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_orgUnitDN', _('This is not a valid list of DNs!'));
		$this->messages['primaryAffiliation'][0] = array('ERROR', _('Account %s:') . ' eduPerson_primaryAffiliation', _('Please enter a valid primary affiliation.'));
		$this->messages['scopedAffiliation'][0] = array('ERROR', _('Account %s:') . ' eduPerson_scopedAffiliation', _('Please enter a valid scoped affiliation.'));
		$this->messages['affiliation'][0] = array('ERROR', _('Account %s:') . ' eduPerson_affiliation', _('Please enter a valid list of affiliations.'));
	}

	/**
	* This functions return true if all needed settings are done.
	*
	* @return boolean true, if all is ok
	*/
	function module_complete() {
		if (($this->getAccountContainer()->rdn == 'eduPersonPrincipalName') && !isset($this->attributes['eduPersonPrincipalName'][0])) {
			return false;
		}
		return true;
	}

	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return htmlElement HTML meta data
	 */
	function display_html_attributes() {
		$return = new htmlTable();
		if (in_array('eduPerson', $this->attributes['objectClass'])) {
			// principal name
			$principal = '';
			if (isset($this->attributes['eduPersonPrincipalName'][0])) {
				$principal = $this->attributes['eduPersonPrincipalName'][0];
			}
			$return->addElement(new htmlTableExtendedInputField(_('Principal name'), 'principalName', $principal, 'principalName'), true);
			// primary affiliation
			$primaryAffiliation = array();
			if (isset($this->attributes['eduPersonPrimaryAffiliation'][0])) {
				$primaryAffiliation = array($this->attributes['eduPersonPrimaryAffiliation'][0]);
			}
			$return->addElement(new htmlTableExtendedSelect('primaryAffiliation', $this->affiliationTypes, $primaryAffiliation, _('Primary affiliation'), 'primaryAffiliation'), true);
			// scoped affiliations
			$scopedAffiliationLabel = new htmlOutputText(_('Scoped affiliations'));
			$scopedAffiliationLabel->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($scopedAffiliationLabel);
			$scopedAffiliations = new htmlTable();
			if (isset($this->attributes['eduPersonScopedAffiliation'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['eduPersonScopedAffiliation']); $i++) {
					$parts = explode('@', $this->attributes['eduPersonScopedAffiliation'][$i]);
					$scopedAffiliationPrefix = array($parts[0]);
					$scopedAffiliation = substr($this->attributes['eduPersonScopedAffiliation'][$i], strlen($parts[0]) + 1);
					$scopedAffiliationContainer = new htmlGroup();
					$scopedAffiliationContainer->addElement(new htmlSelect('scopedAffiliationPrefix' . $i, $this->affiliationTypes, $scopedAffiliationPrefix));
					$scopedAffiliationContainer->addElement(new htmlOutputText('@'));
					$scopedAffiliationContainer->addElement(new htmlInputField('scopedAffiliation' . $i, $scopedAffiliation));
					$scopedAffiliationContainer->addElement(new htmlButton('deleduPersonScopedAffiliation' . $i, 'del.png', true));
					if ($i == (sizeof($this->attributes['eduPersonScopedAffiliation']) - 1)) {
						$scopedAffiliationContainer->addElement(new htmlButton('addeduPersonScopedAffiliation', 'add.png', true));
					}
					$scopedAffiliations->addElement($scopedAffiliationContainer, true);
				}
			}
			else {
				$scopedAffiliationContainer = new htmlGroup();
				$scopedAffiliationContainer->addElement(new htmlOutputText('-'));
				$scopedAffiliationContainer->addElement(new htmlSpacer('10px', null));
				$scopedAffiliationContainer->addElement(new htmlButton('addeduPersonScopedAffiliation', 'add.png', true));
				$scopedAffiliations->addElement($scopedAffiliationContainer);
			}
			$return->addElement($scopedAffiliations);
			$scopedAffiliationHelp = new htmlHelpLink('scopedAffiliation');
			$scopedAffiliationHelp->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($scopedAffiliationHelp, true);
			// affiliations
			$affiliations = new htmlTable();
			if (isset($this->attributes['eduPersonAffiliation'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['eduPersonAffiliation']); $i++) {
					$affiliations->addElement(new htmlSelect('affiliation' . $i, $this->affiliationTypes, array($this->attributes['eduPersonAffiliation'][$i])));
					$affiliationButton = new htmlButton('delAffiliation' . $i, 'del.png', true);
					$affiliations->addElement($affiliationButton, true);
				}
			}
			else {
				$affiliations->addElement(new htmlOutputText('-'), true);
			}
			$affiliationLabel = new htmlOutputText(_('Affiliations'));
			$affiliationLabel->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($affiliationLabel);
			$return->addElement($affiliations);
			$affiliationHelp = new htmlHelpLink('affiliation');
			$affiliationHelp->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($affiliationHelp, true);
			$return->addElement(new htmlOutputText(''));
			$newAffiliationContainer = new htmlTable();
			$newAffiliationContainer->addElement(new htmlSelect('affiliation', $this->affiliationTypes));
			$newAffiliationContainer->addElement(new htmlButton('newAffiliation', 'add.png', true));
			$return->addElement($newAffiliationContainer);
			$return->addElement(new htmlOutputText(''), true);
			// nick names
			$nickNames = array();
			if (isset($this->attributes['eduPersonNickname'][0])) {
				$nickNames = $this->attributes['eduPersonNickname'];
			}
			if (sizeof($nickNames) == 0) {
				$nickNames[] = '';
			}
			$nickNameLabel = new htmlOutputText(_('Nick names'));
			$nickNameLabel->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($nickNameLabel);
			$nickNameContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($nickNames); $i++) {
				$nickNameField = new htmlInputField('eduPersonNickname' . $i, $nickNames[$i]);
				$nickNameField->setFieldSize(40);
				$nickNameContainer->addElement($nickNameField);
				if ($nickNames[$i] != '') {
					$nickNameContainer->addElement(new htmlButton('deleduPersonNickname' . $i, 'del.png', true));
				}
				if ($i < (sizeof($nickNames) - 1)) {
					$nickNameContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$nickNameContainer->addElement(new htmlButton('addeduPersonNickname', 'add.png', true));
				}
			}
			$return->addElement($nickNameContainer);
			$nickNameHelp = new htmlHelpLink('nickName');
			$nickNameHelp->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($nickNameHelp, true);
			// entitlements
			$entitlements = array();
			if (isset($this->attributes['eduPersonEntitlement'][0])) {
				$entitlements = $this->attributes['eduPersonEntitlement'];
			}
			if (sizeof($entitlements) == 0) {
				$entitlements[] = '';
			}
			$entitlementLabel = new htmlOutputText(_('Entitlements'));
			$entitlementLabel->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($entitlementLabel);
			$entitlementContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($entitlements); $i++) {
				$entitlementField = new htmlInputField('eduPersonEntitlement' . $i, $entitlements[$i]);
				$entitlementField->setFieldSize(40);
				$entitlementContainer->addElement($entitlementField);
				if ($entitlements[$i] != '') {
					$entitlementContainer->addElement(new htmlButton('deleduPersonEntitlement' . $i, 'del.png', true));
				}
				if ($i < (sizeof($entitlements) - 1)) {
					$entitlementContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$entitlementContainer->addElement(new htmlButton('addeduPersonEntitlement', 'add.png', true));
				}
			}
			$return->addElement($entitlementContainer);
			$entitlementHelp = new htmlHelpLink('entitlement');
			$entitlementHelp->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($entitlementHelp, true);
			// org DN
			$orgDN = '';
			if (isset($this->attributes['eduPersonOrgDN'][0])) {
				$orgDN = $this->attributes['eduPersonOrgDN'][0];
			}
			$orgDNInput = new htmlTableExtendedInputField(_('Organisation'), 'orgDN', $orgDN, 'orgDN');
			$orgDNInput->setFieldSize(40);
			$return->addElement($orgDNInput, true);
			// primary OU DN
			$primaryOrgUnitDN = '';
			if (isset($this->attributes['eduPersonPrimaryOrgUnitDN'][0])) {
				$primaryOrgUnitDN = $this->attributes['eduPersonPrimaryOrgUnitDN'][0];
			}
			$primaryOUInput = new htmlTableExtendedInputField(_('Primary organisational unit'), 'primaryOrgUnitDN', $primaryOrgUnitDN, 'primaryOrgUnitDN');
			$primaryOUInput->setFieldSize(40);
			$return->addElement($primaryOUInput, true);
			// OUs
			$orgUnitDNs = array();
			if (isset($this->attributes['eduPersonOrgUnitDN'][0])) {
				$orgUnitDNs = $this->attributes['eduPersonOrgUnitDN'];
			}
			if (sizeof($orgUnitDNs) == 0) {
				$orgUnitDNs[] = '';
			}
			$orgUnitDNLabel = new htmlOutputText(_('Organisational units'));
			$orgUnitDNLabel->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($orgUnitDNLabel);
			$orgUnitDNContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($orgUnitDNs); $i++) {
				$orgUnitDNField = new htmlInputField('eduPersonOrgUnitDN' . $i, $orgUnitDNs[$i]);
				$orgUnitDNField->setFieldSize(40);
				$orgUnitDNContainer->addElement($orgUnitDNField);
				if ($orgUnitDNs[$i] != '') {
					$orgUnitDNContainer->addElement(new htmlButton('deleduPersonOrgUnitDN' . $i, 'del.png', true));
				}
				if ($i < (sizeof($orgUnitDNs) - 1)) {
					$orgUnitDNContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$orgUnitDNContainer->addElement(new htmlButton('addeduPersonOrgUnitDN', 'add.png', true));
				}
			}
			$return->addElement($orgUnitDNContainer);
			$orgUnitDNHelp = new htmlHelpLink('orgUnitDN');
			$orgUnitDNHelp->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($orgUnitDNHelp, true);
			// assurance profiles
			$assurances = array();
			if (isset($this->attributes['eduPersonAssurance'][0])) {
				$assurances = $this->attributes['eduPersonAssurance'];
			}
			if (sizeof($assurances) == 0) {
				$assurances[] = '';
			}
			$assuranceLabel = new htmlOutputText(_('Assurance profiles'));
			$assuranceLabel->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($assuranceLabel);
			$assuranceContainer = new htmlGroup();
			for ($i = 0; $i < sizeof($assurances); $i++) {
				$assuranceField = new htmlInputField('eduPersonAssurance' . $i, $assurances[$i]);
				$assuranceField->setFieldSize(40);
				$assuranceContainer->addElement($assuranceField);
				if ($assurances[$i] != '') {
					$assuranceContainer->addElement(new htmlButton('deleduPersonAssurance' . $i, 'del.png', true));
				}
				if ($i < (sizeof($assurances) - 1)) {
					$assuranceContainer->addElement(new htmlOutputText('<br>', false));
				}
				else {
					$assuranceContainer->addElement(new htmlButton('addeduPersonAssurance', 'add.png', true));
				}
			}
			$return->addElement($assuranceContainer);
			$assuranceHelp = new htmlHelpLink('eduPersonAssurance');
			$assuranceHelp->alignment = htmlElement::ALIGN_TOP;
			$return->addElement($assuranceHelp, true);
			// remove button
			$return->addElement(new htmlSpacer(null, '10px'), true);
			$addButton = new htmlButton('remObjectClass', _('Remove EDU person extension'));
			$addButton->colspan = 3;
			$return->addElement($addButton);
		}
		else {
			$return->addElement(new htmlButton('addObjectClass', _('Add EDU person extension')));
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
			$this->attributes['objectClass'][] = 'eduPerson';
			return array();
		}
		elseif (isset($_POST['remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('eduPerson'), $this->attributes['objectClass']);
			for ($i = 0; $i < sizeof($this->meta['attributes']); $i++) {
				if (isset($this->attributes[$this->meta['attributes'][$i]])) {
					unset($this->attributes[$this->meta['attributes'][$i]]);
				}
			}
			return array();
		}
		if (!in_array('eduPerson', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		// primary affiliation
		$this->attributes['eduPersonPrimaryAffiliation'][0] = $_POST['primaryAffiliation'];
		// scoped affiliations
		$scopedAffiliationCounter = 0;
		while (isset($_POST['scopedAffiliation' . $scopedAffiliationCounter])) {
			$this->attributes['eduPersonScopedAffiliation'][$scopedAffiliationCounter] = $_POST['scopedAffiliationPrefix' . $scopedAffiliationCounter] . '@' . $_POST['scopedAffiliation' . $scopedAffiliationCounter];
			if (($_POST['scopedAffiliation' . $scopedAffiliationCounter] == '') || isset($_POST['deleduPersonScopedAffiliation' . $scopedAffiliationCounter])) {
				unset($this->attributes['eduPersonScopedAffiliation'][$scopedAffiliationCounter]);
			}
			$scopedAffiliationCounter++;
		}
		if (isset($_POST['addeduPersonScopedAffiliation'])) {
			$this->attributes['eduPersonScopedAffiliation'][] = '';
		}
		$this->attributes['eduPersonScopedAffiliation'] = array_values(array_unique($this->attributes['eduPersonScopedAffiliation']));
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
		$nickNameCounter = 0;
		while (isset($_POST['eduPersonNickname' . $nickNameCounter])) {
			$this->attributes['eduPersonNickname'][$nickNameCounter] = $_POST['eduPersonNickname' . $nickNameCounter];
			if (($this->attributes['eduPersonNickname'][$nickNameCounter] == '') || isset($_POST['deleduPersonNickname' . $nickNameCounter])) {
				unset($this->attributes['eduPersonNickname'][$nickNameCounter]);
			}
			$nickNameCounter++;
		}
		if (isset($_POST['addeduPersonNickname'])) {
			$this->attributes['eduPersonNickname'][] = '';
		}
		$this->attributes['eduPersonNickname'] = array_values(array_unique($this->attributes['eduPersonNickname']));
		// entitlements
		$entitlementCounter = 0;
		while (isset($_POST['eduPersonEntitlement' . $entitlementCounter])) {
			$this->attributes['eduPersonEntitlement'][$entitlementCounter] = $_POST['eduPersonEntitlement' . $entitlementCounter];
			if (($this->attributes['eduPersonEntitlement'][$entitlementCounter] == '') || isset($_POST['deleduPersonEntitlement' . $entitlementCounter])) {
				unset($this->attributes['eduPersonEntitlement'][$entitlementCounter]);
			}
			$entitlementCounter++;
		}
		if (isset($_POST['addeduPersonEntitlement'])) {
			$this->attributes['eduPersonEntitlement'][] = '';
		}
		$this->attributes['eduPersonEntitlement'] = array_values(array_unique($this->attributes['eduPersonEntitlement']));
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
		$orgUnitDNCounter = 0;
		while (isset($_POST['eduPersonOrgUnitDN' . $orgUnitDNCounter])) {
			$this->attributes['eduPersonOrgUnitDN'][$orgUnitDNCounter] = $_POST['eduPersonOrgUnitDN' . $orgUnitDNCounter];
			if (($this->attributes['eduPersonOrgUnitDN'][$orgUnitDNCounter] == '') || isset($_POST['deleduPersonOrgUnitDN' . $orgUnitDNCounter])) {
				unset($this->attributes['eduPersonOrgUnitDN'][$orgUnitDNCounter]);
			}
			elseif (!get_preg($this->attributes['eduPersonOrgUnitDN'][$orgUnitDNCounter], 'dn')) {
				$errors[] = $this->messages['eduPersonOrgUnitDN'][0];
			}
			$orgUnitDNCounter++;
		}
		if (isset($_POST['addeduPersonOrgUnitDN'])) {
			$this->attributes['eduPersonOrgUnitDN'][] = '';
		}
		$this->attributes['eduPersonOrgUnitDN'] = array_values(array_unique($this->attributes['eduPersonOrgUnitDN']));
		// assurance profiles
		$this->attributes['eduPersonAssurance'] = array();
		$assuranceCounter = 0;
		while (isset($_POST['eduPersonAssurance' . $assuranceCounter])) {
			$this->attributes['eduPersonAssurance'][$assuranceCounter] = $_POST['eduPersonAssurance' . $assuranceCounter];
			if (($this->attributes['eduPersonAssurance'][$assuranceCounter] == '') || isset($_POST['deleduPersonAssurance' . $assuranceCounter])) {
				unset($this->attributes['eduPersonAssurance'][$assuranceCounter]);
			}
			$assuranceCounter++;
		}
		if (isset($_POST['addeduPersonAssurance'])) {
			$this->attributes['eduPersonAssurance'][] = '';
		}
		$this->attributes['eduPersonAssurance'] = array_values(array_unique($this->attributes['eduPersonAssurance']));
		return $errors;
	}

	/**
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
		parent::load_profile($profile);
		// add extension
		if (isset($profile['eduPerson_addExt'][0]) && ($profile['eduPerson_addExt'][0] == "true")) {
			if (!in_array('eduPerson', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'eduPerson';
			}
		}
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
			// scoped affiliations
			if ($rawAccounts[$i][$ids['eduPerson_scopedAffiliation']] != "") {
				$parts = explode(', ', $rawAccounts[$i][$ids['eduPerson_scopedAffiliation']]);
				for ($a = 0; $a < sizeof($parts); $a++) {
					$subparts = explode('@', $parts[$a]);
					if (!in_array($subparts[0], $this->affiliationTypes)) {
						$error = $this->messages['scopedAffiliation'][0];
						array_push($error, $i);
						$messages[] = $error;
					}
					else {
						$partialAccounts[$i]['eduPersonScopedAffiliation'] = $parts;
					}
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
			// organisation
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
			// assurance profiles
			if ($rawAccounts[$i][$ids['eduPerson_assurance']] != "") {
				$parts = explode(', ', $rawAccounts[$i][$ids['eduPerson_assurance']]);
				$partialAccounts[$i]['eduPersonAssurance'] = $parts;
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
			$return['eduPerson_scopedAffiliation'][0] = '<block><key>' . _('Scoped affiliations') . '</key><value>' . implode(', ', $this->attributes['eduPersonScopedAffiliation']) . '</value></block>';
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
			$return['eduPerson_orgDN'][0] = '<block><key>' . _('Organisation') . '</key><value>' . $this->attributes['eduPersonOrgDN'][0] . '</value></block>';
		}
		if (isset($this->attributes['eduPersonPrimaryOrgUnitDN'][0])) {
			$return['eduPerson_primaryOrgUnitDN'][0] = '<block><key>' . _('Primary organisational unit') . '</key><value>' . $this->attributes['eduPersonPrimaryOrgUnitDN'][0] . '</value></block>';
		}
		if (isset($this->attributes['eduPersonOrgUnitDN'][0])) {
			$return['eduPerson_orgUnitDN'][0] = '<block><key>' . _('Organisational units') . '</key><value>' . implode(', ', $this->attributes['eduPersonOrgUnitDN']) . '</value></block>';
		}
		if (isset($this->attributes['eduPersonAssurance'][0])) {
			sort($this->attributes['eduPersonAssurance']);
			$return['eduPerson_eduPersonAssurance'][0] = '<block><key>' . _('Assurance profiles') . '</key><value>' . implode(', ', $this->attributes['eduPersonAssurance']) . '</value></block>';
		}
		return $return;
	}

}


?>
