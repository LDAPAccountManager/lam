<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2004  Roland Gruber

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
* Provides mail routing for user accounts.
*
* @package modules
* @author Roland Gruber
*/

/**
* Provides mail routing for users.
*
* @package modules
*/
class inetLocalMailRecipient extends baseModule {
	
	/** used for account pages, true if input data is correct */
	var $inputCorrect = true;

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages host accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Mail routing");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// help Entries
		$return['help'] = array(
			'routingAdr' => array(
				"Headline" => _("Routing address"),
				"Text" => _("This is the target email address for the user's mails.")
			),
			'localAdr' => array(
				"Headline" => _("Local address"),
				"Text" => _("This is one of the users public email addresses.")
			),
			'localAdrList' => array(
				"Headline" => _("Local address list"),
				"Text" => _("This is a comma separated list of the users public email addresses.")
			),
			'host' => array(
				"Headline" => _("Mail server"),
				"Text" => _("This is the mail server for the user.")
			));
		// profile options
		$return['profile_options'] = array(
			array(
				0 => array('kind' => 'text', 'text' => _('Mail server') . ":"),
				1 => array('kind' => 'input', 'name' => 'inetLocalMailRecipient_host', 'type' => 'text', 'size' => '30', 'maxlength' => '255'),
				2 => array('kind' => 'help', 'value' => 'host'))
		);
		// profile checks
		$return['profile_checks']['inetLocalMailRecipient_host'] = array(
			'type' => 'ext_preg',
			'regex' => 'DNSname',
			'error_message' => $this->messages['host'][0]);
		// profile mappings
		$return['profile_mappings'] = array(
			'inetLocalMailRecipient_host' => 'mailHost'
		);
		// upload fields
		$return['upload_columns'] = array(
			array(
				'name' => 'inetLocalMailRecipient_routingAdr',
				'description' => _('Routing address'),
				'help' => 'routingAdr',
				'example' => 'smiller@otherdomain.org'
			),
			array(
				'name' => 'inetLocalMailRecipient_localAdr',
				'description' => _('Local address list'),
				'help' => 'localAdrList',
				'example' => 'smiller@yourdomain.org'
			),
			array(
				'name' => 'inetLocalMailRecipient_server',
				'description' => _('Mail server'),
				'help' => 'host',
				'example' => 'mail.yourdomain.org'
			)
		);
		// available PDF fields
		$return['PDF_fields'] = array(
			'routingAdr', 'localAdr', 'host'
		);
		return $return;
	}

	/**
	* This function fills the error message array with messages
	*/
	function load_Messages() {
		$this->messages['routingAdr'][0] = array('ERROR', 'Routing address is invalid!');  // third array value is set dynamically
		$this->messages['routingAdr'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_routingAdr', 'Routing address is invalid!');
		$this->messages['localAdr'][0] = array('ERROR', 'Local address is invalid!');  // third array value is set dynamically
		$this->messages['localAdr'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_localAdr', 'Local address is invalid!');
		$this->messages['host'][0] = array('ERROR', 'Mail server is invalid!');  // third array value is set dynamically
		$this->messages['host'][1] = array('ERROR', _('Account %s:') . ' inetLocalMailRecipient_server', 'Mail server is invalid!');
	}
	
	/**
	* This function loads all needed attributes into the object.
	*
	* @param array $attr an array as it is retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->attributes['objectClass'] = array();
		$this->attributes['mailLocalAddress'] = array();
		$this->attributes['mailRoutingAddress'] = array();
		$this->attributes['mailHost'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['mailLocalAddress'] = array();
		$this->orig['mailRoutingAddress'] = array();
		$this->orig['mailHost'] = array();
		if (isset($attr['objectClass'])) {
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		if (isset($attr['mailLocalAddress'])) {
			$this->attributes['mailLocalAddress'] = $attr['mailLocalAddress'];
			$this->orig['mailLocalAddress'] = $attr['mailLocalAddress'];
		}
		if (isset($attr['mailRoutingAddress'])) {
			$this->attributes['mailRoutingAddress'] = $attr['mailRoutingAddress'];
			$this->orig['mailRoutingAddress'] = $attr['mailRoutingAddress'];
		}
		if (isset($attr['mailHost'])) {
			$this->attributes['mailHost'] = $attr['mailHost'];
			$this->orig['mailHost'] = $attr['mailHost'];
		}
		// add object class if needed
		if (! in_array('inetLocalMailRecipient', $this->orig['objectClass'])) {
			$this->attributes['objectClass'][] = 'inetLocalMailRecipient';
		}
		return 0;
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
		return $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	* This function returns a list of all account pages in this module.
	*/
	function pages() {
		return array('attributes');
	}

	/**
	* This function will create the meta HTML code to show a page with all attributes.
	*
	* @param array $post HTTP-POST values
	*/
	function display_html_attributes(&$post) {
		$return = array();
		// mail routing address
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Routing address')),
			1 => array('kind' => 'input', 'name' => 'routingAdr', 'type' => 'text', 'size' => '20', 'value' => $this->attributes['mailRoutingAddress'][0]),
			2 => array('kind' => 'help', 'value' => 'routingAdr'));
		// mail server
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Mail server')),
			1 => array('kind' => 'input', 'name' => 'host', 'type' => 'text', 'size' => '20', 'value' => $this->attributes['mailHost'][0]),
			2 => array('kind' => 'help', 'value' => 'host'));
		// list current local addresses
		for ($i = 0; $i < sizeof($this->attributes['mailLocalAddress']); $i++) {
			$return[] = array(
				0 => array('kind' => 'text', 'text' => _('Local address')),
				1 => array('kind' => 'input', 'name' => 'localAdr' . $i, 'type' => 'text', 'size' => '20', 'value' => $this->attributes['mailLocalAddress'][$i]),
				2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'delAdr' . $i, 'value' => _("Remove")),
				3 => array('kind' => 'help', 'value' => 'localAdr'));
		}
		// input box for new local addresses
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('New local address')),
			1 => array('kind' => 'input', 'name' => 'localAdr', 'type' => 'text', 'size' => '20', 'value' => ''),
			2 => array('kind' => 'input', 'type' => 'submit', 'name' => 'addAdr', 'value' => _("Add")),
			3 => array('kind' => 'help', 'value' => 'localAdr'),
			4 => array('kind' => 'input', 'type' => 'hidden', 'value' => sizeof($this->attributes['mailLocalAddress']), 'name' => 'adr_number'));
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
		$this->triggered_messages = array();
		$this->attributes['mailRoutingAddress'] = array();
		$this->attributes['mailLocalAddress'] = array();
		$this->attributes['mailHost'] = array();
		// check routing address
		if (isset($post['routingAdr']) && ($post['routingAdr'] != "")) {
			// check if address has correct format
			if (get_preg($post['routingAdr'], 'email')) {
				$this->attributes['mailRoutingAddress'][0] = $post['routingAdr'];
			}
			else {
					$message = $this->messages['routingAdr'][0];
					$message[] = $post['routingAdr'];
					$this->triggered_messages[] = array($message);
			}
		}
		// check mail server
		if (isset($post['host']) && ($post['host'] != "")) {
			// check if address has correct format
			if (get_preg($post['host'], 'DNSname')) {
				$this->attributes['mailHost'][0] = $post['host'];
			}
			else {
					$message = $this->messages['host'][0];
					$message[] = $post['host'];
					$this->triggered_messages[] = array($message);
			}
		}
		// check old local addresses
		if (isset($post['adr_number'])) {
			for ($i = 0; $i < $post['adr_number']; $i++) {
				if (isset($post['delAdr' . $i])) continue;
				if (isset($post['localAdr' . $i]) && ($post['localAdr' . $i] != "")) {
					// check if address has correct format
					if (!get_preg($post['localAdr' . $i], 'email')) {
						$message = $this->messages['localAdr'][0];
						$message[] = $post['localAdr' . $i];
						$this->triggered_messages[] = array($message);
					}
					$this->attributes['mailLocalAddress'][] = $post['localAdr' . $i];
				}
			}
		}
		// check new local address
		if (isset($post['localAdr']) && ($post['localAdr'] != "")) {
			// check if address has correct format
			if (get_preg($post['localAdr'], 'email')) {
				$this->attributes['mailLocalAddress'][] = $post['localAdr'];
			}
			else {
					$message = $this->messages['localAdr'][0];
					$message[] = $post['localAdr'];
					$this->triggered_messages[] = array($message);
			}
		}
		$this->attributes['mailLocalAddress'] = array_unique($this->attributes['mailLocalAddress']);
		if (sizeof($this->triggered_messages) > 0) {
			$this->inputCorrect = false;
			return $this->triggered_messages;
		}
		else {
			$this->inputCorrect = true;
		}
	}

	/**
	* This function returns true if all needed settings are done.
	*/
	function module_complete() {
		return $this->inputCorrect;
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
			if (!in_array("inetLocalMailRecipient", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "inetLocalMailRecipient";
			// add local addresses
			if ($rawAccounts[$i][$ids['inetLocalMailRecipient_localAdr']] != "") {
				$adrs = explode(',', $rawAccounts[$i][$ids['inetLocalMailRecipient_localAdr']]);
				// check format
				for ($a = 0; $a < sizeof($adrs); $a++) {
					if (get_preg($adrs[$a], 'email')) {
						$partialAccounts[$i]['mailLocalAddress'][] = $adrs[$a];
					}
					else {
						$errMsg = $this->messages['localAdr'][1];
						array_push($errMsg, array($i));
						$messages[] = $errMsg;
					}
				}
			}
			// add routing address
			if ($rawAccounts[$i][$ids['inetLocalMailRecipient_routingAdr']] != "") {
				// check format
				if (get_preg($rawAccounts[$i][$ids['inetLocalMailRecipient_routingAdr']], 'email')) {
					$partialAccounts[$i]['mailRoutingAddress'][] = $rawAccounts[$i][$ids['inetLocalMailRecipient_routingAdr']];
				}
				else {
					$errMsg = $this->messages['routingAdr'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
			// add mail server
			if ($rawAccounts[$i][$ids['inetLocalMailRecipient_server']] != "") {
				// check format
				if (get_preg($rawAccounts[$i][$ids['inetLocalMailRecipient_server']], 'DNSname')) {
					$partialAccounts[$i]['mailHost'][] = $rawAccounts[$i][$ids['inetLocalMailRecipient_server']];
				}
				else {
					$errMsg = $this->messages['host'][1];
					array_push($errMsg, array($i));
					$messages[] = $errMsg;
				}
			}
		}
		return $messages;
	}

	/**
	* Returns a list of PDF entries
	*/
	function get_pdfEntries() {
		$return = array();
		if (isset($this->attributes['mailRoutingAddress'][0])) {
			$return['inetLocalMailRecipient_routingAdr'][0] = '<block><key>' . _('Routing address') . '</key><value>' . $this->attributes['mailRoutingAddress'][0] . '</value></block>';
		}
		if (sizeof($this->attributes['mailLocalAddress']) > 0) {
			$return['inetLocalMailRecipient_localAdr'][0] = '<block><key>' . _('Local address list') . '</key><value>' . implode(', ', $this->attributes['mailLocalAddress']) . '</value></block>';
		}
		if (isset($this->attributes['mailHost'][0])) {
			$return['inetLocalMailRecipient_host'][0] = '<block><key>' . _('Mail server') . '</key><value>' . $this->attributes['mailHost'][0] . '</value></block>';
		}
		return $return;
	}

}


?>
