<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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

class account extends baseModule {

	/**
	* Returns meta data that is interpreted by parent class
	*
	* @return array array with meta data
	*/
	function get_metaData() {
		$return = array();
		// manages host accounts
		$return["account_types"] = array("host", "user");
		// alias name
		$return["alias"] = _('Account');
		// this is a base module
		$return["is_base"] = true;
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// available PDF fields
		$return['PDF_fields'] = array(
			'description'
		);
		// help Entries
		$return['help'] = array (
			'host' => array(
				'description'	=>	array (
					"Headline" => _("Gecos"),
					"Text" => _("Host description. If left empty host name will be used.")
				)
			)
		);
		// upload columns
		if ($_SESSION['loggedIn']) {
			if ($this->get_scope() == 'user') {
				$return['upload_columns'][] = array(
	                'name' => 'account_hosts',
	                'description' => _('Host list'),
	                'help' => 'host'
				);
			}
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('posixAccount', $modules)) {
				$return['upload_columns'][] = array(
	                'name' => 'account_uid',
	                'description' => _('User name'),
	                'help' => 'uid',
	                'required' => true
				);
				$return['upload_columns'] = array(
					array(
	                'name' => 'account_description',
	                'description' => _('Description'),
	                'help' => 'description'
					)
				);
			}
		}
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		}

	/** this functin fills the error message array with messages
	**/
	function load_Messages() {
	}

	/* This function returns true if all required attributes from other
	* modules are set. This is required to prevent undefined states
	*/
	function module_ready() {
		return true;
		}

	/* This functions return true
	* if all needed settings are done
	*/
	function module_complete() {
		if (!$this->module_ready()) return false;
		return true;
		}	
	
	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes');
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		$this->load_ldap_attributes($attr);
		return 0;
		}

	/* This function returns an array with 4 entries:
	* array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr), 'lamdaemon' => array(cmds)), DN2 .... )
	* DN is the DN to change. It may be possible to change several DNs,
	* e.g. create a new user and add him to some groups via attribute memberUid
	* add are attributes which have to be added to ldap entry
	* remove are attributes which have to be removed from ldap entry
	* lamdaemon are lamdaemon commands to modify homedir, quotas, ...
	*/
	function save_attributes() {
		// Get easy attributes
		$return = $_SESSION[$this->base]->save_module_attributes($this->attributes, $this->orig);
		// Return attributes
		return $return;
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function process_attributes(&$post) {
		// Load attributes
		$this->attributes['description'][0] = $post['description'];
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes(&$post) {
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Description') ),
			1 => array ( 'kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['description'][0] ),
			2 => array ('kind' => 'help', 'value' => 'description'));

		return $return;
		}

	function display_html_delete(&$post) {
		return 0;
		}
	
	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "user") {
		return array('account_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
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
			if (!in_array("account", $partialAccounts[$i]['objectClass'])) $partialAccounts[$i]['objectClass'][] = "account";
			$modules = $_SESSION['config']->get_AccountModules($this->get_scope());
			if (!in_array('posixAccount', $modules)) {
				// description
				if ($rawAccounts[$i][$ids['account_description']] && ($rawAccounts[$i][$ids['account_description']] != '')) { 
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['account_description']];
				}
				else {
					$partialAccounts[$i]['description'] = $rawAccounts[$i][$ids['account_uid']];
				}
				// user name
				if (get_preg($rawAccounts[$i][$ids['account_uid']], 'username')) {
					$partialAccounts[$i]['uid'] = $rawAccounts[$i][$ids['account_uid']];
				}
				else {
					$errMsg = $this->messages['uid'][7];
					array_push($errMsg, array($i));
					$triggered_messages[] = $errMsg;
				}
			}
			if ($this->get_scope() == 'user') {
			}
		}
		return $messages;
	}

}

?>
