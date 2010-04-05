<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010  Roland Gruber

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
* Manages the hosts to which a user may login.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages the hosts to which a user may login.
*
* @package modules
*/
class hostObject extends baseModule {
	
	/**
	* Creates a new hostObject object.
	*
	* @param string $scope account type (user, group, host)
	*/
	function __construct($scope) {
		// call parent constructor
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
		$return['icon'] = 'computer.png';
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Hosts");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('hostObject');
		// managed attributes
		$return['attributes'] = array('host');
		// help Entries
		$return['help'] = array(
			'host' => array(
				"Headline" => _("Hosts"),
				"Text" => _("Here you can specify the list of host names where this account has login privileges. The wildcard \"*\" represents all hosts. You may also use \"!\" in front of a host name to deny access to a host.")
			),
			'autoAdd' => array(
				"Headline" => _("Automatically add this extension"),
				"Text" => _("This will enable the extension automatically if this profile is loaded.")
			)
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'hostObject_hosts',
				'description' => _('Host list'),
				'help' => 'host',
				'example' => _('pc01,pc02')
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'hosts' => _('Host list')
		);
		$return['profile_options'] = array(
			array(
				array('kind' => 'text', 'text' => _('Automatically add this extension') . ":"),
				array('kind' => 'input', 'name' => 'hostObject_addExt', 'type' => 'checkbox'),
				array('kind' => 'help', 'value' => 'autoAdd')),
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['host'][0] = array('ERROR', 'Please enter a valid host name.');  // third array value is set dynamically
		$this->messages['host'][1] = array('ERROR', _('Account %s:') . ' hostObject_host', _('Please enter a valid list of host names.'));
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
	*/
	function save_attributes() {
		if (!in_array('hostObject', $this->attributes['objectClass']) && !in_array('hostObject', $this->orig['objectClass'])) {
			// skip saving if the extension was not added/modified
			return array();
		}
		return parent::save_attributes();
	}
	
	/**
	 * Returns the HTML meta data for the main account page.
	 * 
	 * @return array HTML meta data
	 */
	function display_html_attributes() {
		if (isset($_POST['form_subpage_hostObject_attributes_addObjectClass'])) {
			$this->attributes['objectClass'][] = 'hostObject';
		}
		$return = array();
		if (in_array('hostObject', $this->attributes['objectClass'])) {
			// list current hosts
			for ($i = 0; $i < sizeof($this->attributes['host']); $i++) {
				$return[] = array(
					array('kind' => 'text', 'text' => _('Host')),
					array('kind' => 'input', 'name' => 'host' . $i, 'type' => 'text', 'size' => '20', 'value' => $this->attributes['host'][$i]),
					array('kind' => 'input', 'type' => 'submit', 'name' => 'delHost' . $i, 'title' => _("Remove"), 'value' => ' ', 'image' => 'del.png'),
					array('kind' => 'help', 'value' => 'host'));
			}
			// input box for new host
			$return[] = array(
				array('kind' => 'text', 'text' => _('New host')),
				array('kind' => 'input', 'name' => 'host', 'type' => 'text', 'size' => '20', 'value' => ''),
				array('kind' => 'input', 'type' => 'submit', 'name' => 'addHost', 'title' => _("Add"), 'value' => ' ', 'image' => 'add.png'),
				array('kind' => 'help', 'value' => 'host'),
				array('kind' => 'input', 'type' => 'hidden', 'value' => sizeof($this->attributes['host']), 'name' => 'host_number'));
			$return[] = array(
				array('kind' => 'text', 'text' => '&nbsp;')
			);
			$return[] = array(
				array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_hostObject_attributes_remObjectClass',
					'value' => _('Remove host extension'), 'td' => array('colspan' => '4'))
			);
		}
		else {
			$return[] = array(
				array('kind' => 'text', 'text' => '&nbsp;'),
				array('kind' => 'input', 'type' => 'submit', 'name' => 'form_subpage_hostObject_attributes_addObjectClass', 'value' => _('Add host extension'))
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
		if (isset($_POST['form_subpage_hostObject_attributes_remObjectClass'])) {
			$this->attributes['objectClass'] = array_delete(array('hostObject'), $this->attributes['objectClass']);
			if (isset($this->attributes['host'])) unset($this->attributes['host']);
			return array();
		}
		if (!in_array('hostObject', $this->attributes['objectClass'])) {
			return array();
		}
		$errors = array();
		$this->attributes['host'] = array();
		// check old hosts
		if (isset($_POST['host_number'])) {
			for ($i = 0; $i < $_POST['host_number']; $i++) {
				if (isset($_POST['delHost' . $i])) continue;
				if (isset($_POST['host' . $i]) && ($_POST['host' . $i] != "")) {
					// check if host has correct format
					if (!get_preg($_POST['host' . $i], 'hostObject')) {
						$message = $this->messages['host'][0];
						$message[] = $_POST['host' . $i];
						$errors[] = $message;
					}
					$this->attributes['host'][] = $_POST['host' . $i];
				}
			}
		}
		// check new host
		if (isset($_POST['host']) && ($_POST['host'] != "")) {
			// check if host has correct format
			if (get_preg($_POST['host'], 'hostObject')) {
				$this->attributes['host'][] = $_POST['host'];
			}
			else {
					$message = $this->messages['host'][0];
					$message[] = $_POST['host'];
					$errors[] = $message;
			}
		}
		$this->attributes['host'] = array_unique($this->attributes['host']);
		return $errors;
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
		$messages = array();
		for ($i = 0; $i < sizeof($rawAccounts); $i++) {
			// add object class
			if (!in_array("hostObject", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "hostObject";
			// add hosts
			if ($rawAccounts[$i][$ids['hostObject_hosts']] != "") {
				$hosts = explode(',', $rawAccounts[$i][$ids['hostObject_hosts']]);
				// check format
				for ($a = 0; $a < sizeof($hosts); $a++) {
					if (get_preg($hosts[$a], 'hostObject')) {
						$partialAccounts[$i]['host'][] = $hosts[$a];
					}
					else {
						$errMsg = $this->messages['host'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
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
		if (sizeof($this->attributes['host']) > 0) {
			$return['hostObject_hosts'][0] = '<block><key>' . _('Host list') . '</key><value>' . implode(', ', $this->attributes['host']) . '</value></block>';
		}
		return $return;
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
		if ($profile['hostObject_addExt'][0] == "true") {
			if (!in_array('hostObject', $this->attributes['objectClass'])) {
				$this->attributes['objectClass'][] = 'hostObject';
			}
		}
	}

}


?>
