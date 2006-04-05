<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2005 - 2006  Roland Gruber

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
* Manages Samba 3 domain entries.
*
* @package modules
* @author Roland Gruber
*/

/**
* Manages Samba 3 domain entries.
*
* @package modules
*/
class sambaDomain extends baseModule {
	
	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages host accounts
		$return["account_types"] = array("smbDomain");
		// alias name
		$return["alias"] = _("Samba domain");
		// this is a base module
		$return["is_base"] = true;
		// RDN attribute
		$return["RDN"] = array("sambaDomainName" => "high");
		// LDAP filter
		$return["ldap_filter"] = array('or' => "(objectClass=sambaDomain)");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// managed object classes
		$return['objectClasses'] = array('sambaDomain');
		// help Entries
		$return['help'] = array(
			'domainName' => array(
				"Headline" => _("Domain name"),
				"Text" => _("The name of your Windows domain or workgroup.")
			),
			'domainSID' => array(
				"Headline" => _("Domain SID"),
				"Text" => _("The SID of your Samba server. Get it with \"net getlocalsid\".")
			),
			'nextRID' => array(
				"Headline" => _("Next RID"),
				"Text" => _("Next RID to use when creating accounts (only used by Winbind).")
			),
			'nextUserRID' => array(
				"Headline" => _("Next user RID"),
				"Text" => _("Next RID to use when creating user accounts (only used by Winbind).")
			),
			'nextGroupRID' => array(
				"Headline" => _("Next group RID"),
				"Text" => _("Next RID to use when creating group accounts (only used by Winbind).")
			),
			'nextRID' => array(
				"Headline" => _("RID base"),
				"Text" => _("Used for calculating RIDs from UID/GID. Do not change if unsure.")
			));
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'sambaDomain_domainName',
				'description' => _('Domain name'),
				'help' => 'domainName',
				'example' => _('Workgroup'),
				'required' => true
			),
			array(
				'name' => 'sambaDomain_domainSID',
				'description' => _('Domain SID'),
				'help' => 'domainSID',
				'example' => 'S-1-1-22-123-123-123',
				'required' => true
			),
			array(
				'name' => 'sambaDomain_RIDbase',
				'description' => _('RID base'),
				'help' => 'RIDbase',
				'example' => '1000',
				'default' => 1000
			),
			array(
				'name' => 'sambaDomain_nextRID',
				'description' => _('Next RID'),
				'help' => 'nextRID',
				'example' => '12345'
			),
			array(
				'name' => 'sambaDomain_nextUserRID',
				'description' => _('Next user RID'),
				'help' => 'nextUserRID',
				'example' => '12345'
			),
			array(
				'name' => 'sambaDomain_nextGroupRID',
				'description' => _('Next group RID'),
				'help' => 'nextGroupRID',
				'example' => '12345'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'domainName', 'domainSID', 'nextRID',
			'nextUserRID', 'nextGroupRID', 'RIDbase'
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['domainName'][0] = array('ERROR', _('Domain name is invalid!'));
		$this->messages['domainName'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_domainName', _('Domain name is invalid!'));
		$this->messages['domainSID'][0] = array('ERROR', _('Samba 3 domain SID is invalid!'));
		$this->messages['domainSID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_domainSID', _('Samba 3 domain SID is invalid!'));
		$this->messages['nextRID'][0] = array('ERROR', _('Next RID is not a number!'));
		$this->messages['nextRID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_nextRID', _('Next RID is not a number!'));
		$this->messages['nextUserRID'][0] = array('ERROR', _('Next user RID is not a number!'));
		$this->messages['nextUserRID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_nextUserRID', _('Next user RID is not a number!'));
		$this->messages['nextGroupRID'][0] = array('ERROR', _('Next group RID is not a number!'));
		$this->messages['nextGroupRID'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_nextGroupRID', _('Next group RID is not a number!'));
		$this->messages['RIDbase'][0] = array('ERROR', _('Algorithmic RID base is not a number!'));
		$this->messages['RIDbase'][1] = array('ERROR', _('Account %s:') . ' sambaDomain_RIDbase', _('Algorithmic RID base is not a number!'));
	}
	
	/**
	* This function loads all needed attributes into the object.
	*
	* @param array $attr an array as it is retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$attributes = array('sambaAlgorithmicRidBase', 'sambaDomainName', 'sambaNextGroupRid',
			'sambaNextRid', 'sambaNextUserRid', 'sambaSID');
		$this->attributes['objectClass'] = array();
		$this->attributes['macAddress'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['macAddress'] = array();
		if (isset($attr['objectClass'])) {
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		for ($i = 0; $i < sizeof($attributes); $i++) {
			if (isset($attr[$attributes[$i]])) {
				$this->attributes[$attributes[$i]] = $attr[$attributes[$i]];
				$this->orig[$attributes[$i]] = $attr[$attributes[$i]];
			}
		}
		// add object class if needed
		if (! in_array('sambaDomain', $this->orig['objectClass'])) {
			$this->attributes['objectClass'][] = 'sambaDomain';
		}
		return 0;
	}

	/**
	* This function will create the meta HTML code to show a page with all attributes.
	*
	* @param array $post HTTP-POST values
	*/
	function display_html_attributes(&$post) {
		$return = array();
		// domain name
		if ($_SESSION[$this->base]->isNewAccount) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain name').'*'),
				1 => array('kind' => 'input', 'name' => 'domainName', 'type' => 'text', 'value' => $this->attributes['sambaDomainName'][0]),
				2 => array('kind' => 'help', 'value' => 'domainName'));
		}
		else {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain name')),
				1 => array('kind' => 'text', 'text' => $this->attributes['sambaDomainName'][0]),
				2 => array('kind' => 'help', 'value' => 'domainName'));
		}
		// domain SID
		if ($_SESSION[$this->base]->isNewAccount) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain SID').'*'),
				1 => array('kind' => 'input', 'name' => 'domainSID', 'type' => 'text', 'value' => $this->attributes['sambaSID'][0]),
				2 => array('kind' => 'help', 'value' => 'domainSID'));
		}
		else {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Domain SID')),
				1 => array('kind' => 'text', 'text' => $this->attributes['sambaSID'][0]),
				2 => array('kind' => 'help', 'value' => 'domainSID'));
		}
		// next RID
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Next RID')),
			1 => array('kind' => 'input', 'name' => 'nextRID', 'type' => 'text', 'value' => $this->attributes['sambaNextRid'][0]),
			2 => array('kind' => 'help', 'value' => 'nextRID'));
		// next user RID
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Next user RID')),
			1 => array('kind' => 'input', 'name' => 'nextUserRID', 'type' => 'text', 'value' => $this->attributes['sambaNextUserRid'][0]),
			2 => array('kind' => 'help', 'value' => 'nextUserRID'));
		// next group RID
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Next group RID')),
			1 => array('kind' => 'input', 'name' => 'nextGroupRID', 'type' => 'text', 'value' => $this->attributes['sambaNextGroupRid'][0]),
			2 => array('kind' => 'help', 'value' => 'nextGroupRID'));
		// RID base
		if (!isset($this->attributes['sambaAlgorithmicRidBase'][0])) $this->attributes['sambaAlgorithmicRidBase'][0] = 1000;
		if ($_SESSION[$this->base]->isNewAccount) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('RID base').'*'),
				1 => array('kind' => 'input', 'name' => 'RIDbase', 'type' => 'text', 'value' => $this->attributes['sambaAlgorithmicRidBase'][0]),
				2 => array('kind' => 'help', 'value' => 'RIDbase'));
		}
		else {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('RID base')),
				1 => array('kind' => 'text', 'text' => $this->attributes['sambaAlgorithmicRidBase'][0]),
				2 => array('kind' => 'help', 'value' => 'RIDbase'));
		}
		return $return;
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_attributes(&$post) {
		$errors = array();
		if ($_SESSION[$this->base]->isNewAccount) {
			// domain SID
			if (!get_preg($_POST['domainSID'], 'domainSID')) {
				$errors[] = $this->messages['domainSID'][0];
			}
			else {
				$this->attributes['sambaSID'][0] = $_POST['domainSID'];
			}
			// RID base
			if (!get_preg($_POST['RIDbase'], 'digit') && !($_POST['RIDbase'] == '')) {
				$errors[] = $this->messages['RIDbase'][0];
			}
			else {
				$this->attributes['sambaAlgorithmicRidBase'][0] = $_POST['RIDbase'];
			}
			// domain name
			if (!get_preg($_POST['domainName'], 'domainname') && !($_POST['domainName'] == '')) {
				$errors[] = $this->messages['domainName'][0];
			}
			else {
				$this->attributes['sambaDomainName'][0] = $_POST['domainName'];
			}
		}
		// next RID
		if (!get_preg($_POST['nextRID'], 'digit')) {
			$errors[] = $this->messages['nextRID'][0];
		}
		else {
			$this->attributes['sambaNextRid'][0] = $_POST['nextRID'];
		}
		// next user RID
		if (!get_preg($_POST['nextUserRID'], 'digit')) {
			$errors[] = $this->messages['nextUserRID'][0];
		}
		else {
			$this->attributes['sambaNextUserRid'][0] = $_POST['nextUserRID'];
		}
		// next group RID
		if (!get_preg($_POST['nextGroupRID'], 'digit')) {
			$errors[] = $this->messages['nextGroupRID'][0];
		}
		else {
			$this->attributes['sambaNextGroupRid'][0] = $_POST['nextGroupRID'];
		}
		return array($errors);
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
			if (!in_array("sambaDomain", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "sambaDomain";
			// domain name
			if (get_preg($rawAccounts[$i][$ids['sambaDomain_domainName']], 'domainname')) {
				$partialAccounts[$i]['sambaDomainName'] = $rawAccounts[$i][$ids['sambaDomain_domainName']];
			}
			else {
				$errMsg = $this->messages['domainName'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// domain SID
			if (get_preg($rawAccounts[$i][$ids['sambaDomain_domainSID']], 'domainSID')) {
				$partialAccounts[$i]['sambaSID'] = $rawAccounts[$i][$ids['sambaDomain_domainSID']];
			}
			else {
				$errMsg = $this->messages['domainSID'][1];
				array_push($errMsg, array($i));
				$messages[] = $errMsg;
			}
			// RID base
			if ($rawAccounts[$i][$ids['sambaDomain_RIDbase']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_RIDbase']], 'digit')) {
					$partialAccounts[$i]['sambaAlgorithmicRidBase'] = $rawAccounts[$i][$ids['sambaDomain_RIDbase']];
				}
				else {
					$errMsg = $this->messages['RIDbase'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			else {
				$partialAccounts[$i]['sambaAlgorithmicRidBase'] = '1000';
			}
			// next RID
			if ($rawAccounts[$i][$ids['sambaDomain_nextRID']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_nextRID']], 'digit')) {
					$partialAccounts[$i]['sambaNextRid'] = $rawAccounts[$i][$ids['sambaDomain_nextRID']];
				}
				else {
					$errMsg = $this->messages['nextRID'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// next user RID
			if ($rawAccounts[$i][$ids['sambaDomain_nextUserRID']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_nextUserRID']], 'digit')) {
					$partialAccounts[$i]['sambaNextUserRid'] = $rawAccounts[$i][$ids['sambaDomain_nextUserRID']];
				}
				else {
					$errMsg = $this->messages['nextUserRID'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// next group RID
			if ($rawAccounts[$i][$ids['sambaDomain_nextGroupRID']]) {
				if (get_preg($rawAccounts[$i][$ids['sambaDomain_nextGroupRID']], 'digit')) {
					$partialAccounts[$i]['sambaNextGroupRid'] = $rawAccounts[$i][$ids['sambaDomain_nextGroupRID']];
				}
				else {
					$errMsg = $this->messages['nextGroupRID'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
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
		if (sizeof($this->attributes['sambaDomainName']) > 0) {
			$return['sambaDomain_domainName'][0] = '<block><key>' . _('Domain name') . '</key><value>' . implode(', ', $this->attributes['sambaDomainName']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaSID']) > 0) {
			$return['sambaDomain_domainSID'][0] = '<block><key>' . _('Domain SID') . '</key><value>' . implode(', ', $this->attributes['sambaSID']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaNextRid']) > 0) {
			$return['sambaDomain_nextRID'][0] = '<block><key>' . _('Next RID') . '</key><value>' . implode(', ', $this->attributes['sambaNextRid']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaNextUserRid']) > 0) {
			$return['sambaDomain_nextUserRID'][0] = '<block><key>' . _('Next user RID') . '</key><value>' . implode(', ', $this->attributes['sambaNextUserRid']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaNextGroupRid']) > 0) {
			$return['sambaDomain_nextGroupRID'][0] = '<block><key>' . _('Next group RID') . '</key><value>' . implode(', ', $this->attributes['sambaNextGroupRid']) . '</value></block>';
		}
		if (sizeof($this->attributes['sambaAlgorithmicRidBase']) > 0) {
			$return['sambaDomain_RIDbase'][0] = '<block><key>' . _('RID base') . '</key><value>' . implode(', ', $this->attributes['sambaAlgorithmicRidBase']) . '</value></block>';
		}
		return $return;
	}

}


?>
