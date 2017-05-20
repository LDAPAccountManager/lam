<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2015  Roland Gruber

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
		$return['icon'] = 'eduPerson.png';
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
			'eduPersonNickname' => array(
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
			'eduPersonPrincipalName' => array(
				"Headline" => _("Principal name"), 'attr' => 'eduPersonPrincipalName',
				"Text" => _("The \"NetID\" of the person for the purposes of inter-institutional authentication. It should be represented in the form \"user@scope\" where scope defines a local security domain.")
			),
			'eduPersonEntitlement' => array(
				"Headline" => _("Entitlements"), 'attr' => 'eduPersonEntitlement',
				"Text" => _("URI (either URN or URL) that indicates a set of rights to specific resources.")
			),
			'entitlementUpload' => array(
				"Headline" => _("Entitlements"), 'attr' => 'eduPersonEntitlement',
				"Text" => _("URI (either URN or URL) that indicates a set of rights to specific resources.") . " " . _('Multiple values are separated by comma.')
			),
			'eduPersonOrgDN' => array(
				"Headline" => _("Organisation"), 'attr' => 'eduPersonOrgDN',
				"Text" => _("The DN of the directory entry representing the institution with which the person is associated.")
			),
			'eduPersonPrimaryOrgUnitDN' => array(
				"Headline" => _("Primary organisational unit"), 'attr' => 'eduPersonPrimaryOrgUnitDN',
				"Text" => _("The DN of the directory entry representing the person's primary organisational unit.")
			),
			'eduPersonOrgUnitDN' => array(
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
				'help' => 'eduPersonPrincipalName',
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
				'help' => 'eduPersonNickname',
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
				'help' => 'eduPersonOrgDN',
				'example' => _('ou=accounts,dc=yourdomain,dc=org')
			),
			array(
				'name' => 'eduPerson_primaryOrgUnitDN',
				'description' => _('Primary organisational unit'),
				'help' => 'eduPersonPrimaryOrgUnitDN',
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
		$this->messages['eduPersonPrincipalName'][0] = array('ERROR', _('Principal name is invalid!'));
		$this->messages['eduPersonPrincipalName'][1] = array('ERROR', _('Account %s:') . ' eduPerson_principalName', _('Principal name is invalid!'));
		$this->messages['eduPersonOrgDN'][0] = array('ERROR', _('Please enter a valid DN in the field:') . ' ' . _("Organisation"));
		$this->messages['eduPersonOrgDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_orgDN', _('This is not a valid DN!'));
		$this->messages['eduPersonPrimaryOrgUnitDN'][0] = array('ERROR', _('Please enter a valid DN in the field:') . ' ' . _("Primary organisational unit"));
		$this->messages['eduPersonPrimaryOrgUnitDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_primaryOrgUnitDN', _('This is not a valid DN!'));
		$this->messages['eduPersonOrgUnitDN'][0] = array('ERROR', _('Organisational units contains an invalid entry.'));
		$this->messages['eduPersonOrgUnitDN'][1] = array('ERROR', _('Account %s:') . ' eduPerson_orgUnitDN', _('This is not a valid list of DNs!'));
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
			$this->addSimpleInputTextField($return, 'eduPersonPrincipalName', _('Principal name'));
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
			$this->addMultiValueInputTextField($return, 'eduPersonNickname', _('Nick names'));
			// entitlements
			$this->addMultiValueInputTextField($return, 'eduPersonEntitlement', _('Entitlements'));
			// org DN
			$this->addSimpleInputTextField($return, 'eduPersonOrgDN', _('Organisation'));
			// primary OU DN
			$this->addSimpleInputTextField($return, 'eduPersonPrimaryOrgUnitDN', _('Primary organisational unit'));
			// OUs
			$this->addMultiValueInputTextField($return, 'eduPersonOrgUnitDN', _('Organisational units'));
			// assurance profiles
			$this->addMultiValueInputTextField($return, 'eduPersonAssurance', _('Assurance profiles'));
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
		$this->attributes['eduPersonPrincipalName'][0] = $_POST['eduPersonPrincipalName'];
		if ($_POST['eduPersonPrincipalName'] != '') {
			if (!preg_match('/^[0-9a-z_\\.@-]+$/i', $_POST['eduPersonPrincipalName'])) {
				$errors[] = $this->messages['eduPersonPrincipalName'][0];
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
		$this->processMultiValueInputTextField('eduPersonNickname', $errors);
		// entitlements
		$this->processMultiValueInputTextField('eduPersonEntitlement', $errors);
		// org DN
		$this->attributes['eduPersonOrgDN'][0] = $_POST['eduPersonOrgDN'];
		if ($_POST['eduPersonOrgDN'] != '') {
			if (!get_preg($_POST['eduPersonOrgDN'], 'dn')) {
				$errors[] = $this->messages['eduPersonOrgDN'][0];
			}
		}
		// primary OU
		$this->attributes['eduPersonPrimaryOrgUnitDN'][0] = $_POST['eduPersonPrimaryOrgUnitDN'];
		if ($_POST['eduPersonPrimaryOrgUnitDN'] != '') {
			if (!get_preg($_POST['eduPersonPrimaryOrgUnitDN'], 'dn')) {
				$errors[] = $this->messages['eduPersonPrimaryOrgUnitDN'][0];
			}
		}
		// OUs
		$this->processMultiValueInputTextField('eduPersonOrgUnitDN', $errors, 'dn');
		// assurance profiles
		$this->processMultiValueInputTextField('eduPersonAssurance', $errors);
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
	 * {@inheritDoc}
	 * @see baseModule::build_uploadAccounts()
	 */
	function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts, $selectedModules, &$type) {
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("eduPerson", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "eduPerson";
			// principal name
			if ($rawAccounts[$i][$ids['eduPerson_principalName']] != "") {
				if (!preg_match('/^[0-9a-z_\\.@-]+$/i', $rawAccounts[$i][$ids['eduPerson_principalName']])) {
					$error = $this->messages['eduPersonPrincipalName'][1];
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
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'eduPerson_nickname', 'eduPersonNickname', null, array(), $messages, '/,[ ]*/');
			// entitlements
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'eduPerson_entitlement', 'eduPersonEntitlement', null, array(), $messages, '/,[ ]*/');
			// organisation
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'eduPerson_orgDN', 'eduPersonOrgDN',
				'dn', $this->messages['eduPersonOrgDN'][1], $messages);
			// primary OU
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'eduPerson_primaryOrgUnitDN', 'eduPersonPrimaryOrgUnitDN',
				'dn', $this->messages['eduPersonPrimaryOrgUnitDN'][1], $messages);
			// OUs
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'eduPerson_orgUnitDN', 'eduPersonOrgUnitDN', 'dn', $this->messages['eduPersonOrgUnitDN'][1], $messages, '/;[ ]*/');
			// assurance profiles
			$this->mapSimpleUploadField($rawAccounts, $ids, $partialAccounts, $i, 'eduPerson_assurance', 'eduPersonAssurance', null, array(), $messages, '/,[ ]*/');
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 * @see baseModule::get_pdfEntries()
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		$this->addSimplePDFField($return, 'primaryAffiliation', _('Primary affiliation'), 'eduPersonPrimaryAffiliation');
		$this->addSimplePDFField($return, 'scopedAffiliation', _('Scoped affiliations'), 'eduPersonScopedAffiliation');
		$this->addSimplePDFField($return, 'principalName', _('Principal name'), 'eduPersonPrincipalName');
		$this->addSimplePDFField($return, 'nickname', _('Nick names'), 'eduPersonNickname');
		$this->addSimplePDFField($return, 'affiliation', _('Affiliations'), 'eduPersonAffiliation');
		$this->addSimplePDFField($return, 'entitlement', _('Entitlements'), 'eduPersonEntitlement');
		$this->addSimplePDFField($return, 'orgDN', _('Organisation'), 'eduPersonOrgDN');
		$this->addSimplePDFField($return, 'primaryOrgUnitDN', _('Primary organisational unit'), 'eduPersonPrimaryOrgUnitDN');
		$this->addSimplePDFField($return, 'orgUnitDN', _('Organisational units'), 'eduPersonOrgUnitDN');
		$this->addSimplePDFField($return, 'eduPersonAssurance', _('Assurance profiles'), 'eduPersonAssurance');
		return $return;
	}

}


?>
