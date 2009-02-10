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
		$return['icon'] = 'uid.png';
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
			'nickName' => array(
				"Headline" => _("Nick names"),
				"Text" => _("This is a list of nick names for this user.")
			),
			'affiliation' => array(
				"Headline" => _("Affiliations"),
				"Text" => _("Specifies the person's relationship(s) to the institution in broad categories such as student, faculty, staff, alum, etc.")
			),
			'principalName' => array(
				"Headline" => _("Principal name"),
				"Text" => _("The \"NetID\" of the person for the purposes of inter-institutional authentication.") .
							" " . _("It should be represented in the form \"user@scope\" where scope defines a local security domain.")
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'ieee802Device_mac',
				'description' => _('MAC address'),
				'help' => 'macList',
				'example' => '00:01:02:DE:EF:18'
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
		$this->messages['principalName'][0] = array('ERROR', 'Principal name is invalid!');
		$this->messages['principalName'][1] = array('ERROR', _('Account %s:') . ' eduPerson_principalName', 'Principal name is invalid!');
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
			// affiliations
			$affiliations = array();
			if (isset($this->attributes['eduPersonAffiliation'][0])) {
				for ($i = 0; $i < sizeof($this->attributes['eduPersonAffiliation']); $i++) {
					$affiliations[] = array(
						array('kind' => 'select', 'name' => 'affiliation' . $i, 'options' => $this->affiliationTypes, 'options_selected' => array($this->attributes['eduPersonAffiliation'][$i])),
						array('kind' => 'input', 'type' => 'submit', 'name' => 'delAffiliation' . $i, 'image' => 'delete.png', 'value' => ' ', 'title' => _('Delete'))
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
						array('kind' => 'input', 'type' => 'submit', 'name' => 'delNickName' . $i, 'image' => 'delete.png', 'value' => ' ', 'title' => _('Delete'))
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
				array('kind' => 'text', 'text' => ' '));
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
		// principal name
		$this->attributes['eduPersonPrincipalName'][0] = $_POST['principalName'];
		if ($_POST['principalName'] != '') {
			if (eregi('^[0-9a-z_\\.@-]+$', $_POST['principalName']) === false) {
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
			// primary affiliation
			if ($rawAccounts[$i][$ids['eduPerson_primaryAffiliation']] != "") {
				
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
		return $return;
	}

}


?>
