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

/*
* Variables in basearray which are no objects:
* type: Type of account. Can be user, group, host, domain
* attributes: List of all attributes, how to get them and are theiy required or optional
* dn: current DN without uid= or cn=
* dn_orig: old DN if account was loaded with uid= or cn=
*/


/* This class contains all account LDAP attributes
* and funtioncs required to deal with account
* account can only be created when it should be added
* to an array.
* $base is the name of account_container in session
*
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
		$return["account_types"] = array("host");
		// alias name
		$return["alias"] = _('Account');
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array('inetOrgPerson'));
		// available PDF fields
		$return['PDF_fields'] = array('description');
		// help Entries
		$return['help'] = array (	'description'	=>	array ("ext" => "FALSE", "Headline" => _("Gecos"), "Text" => _("Host description. If left empty host name will be used.")));
		return $return;
	}

	// Constructor
	function init($base) {
		// call parent init
		parent::init($base);
		// load attribtues which are used in account objectClass
		$this->orig = $_SESSION[$this->base]->get_module_attributes('account');
		$this->attributes = $_SESSION[$this->base]->get_module_attributes('account');
		// Add objectClass to attributes
		$this->attributes['objectClass'][0] = 'account';
		}

	// Variables
	// This variable contains all account attributes
	var $attributes;
	/* If an account was loaded all attributes are kept in this array
	* to compare it with new changed attributes
	*/
	var $orig;


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

	/*
	*/
	/*function get_help($id) {
		switch ($id) {
			case "description":
				return array ("ext" => "FALSE", "Headline" => _("Gecos"), "Text" => _("Host description. If left empty host name will be used."));
				break;
		}
		return false;
	}*/

	/* This function returns all ldap attributes
	* which are part of account and returns
	* also their values.
	*/
	function get_attributes() {
		return $this->attributes;
		}

	/* This function loads all attributes into the object
	* $attr is an array as it's retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		// Load attributes which are displayed
		// unset count entries
		unset ($attr['count']);
		$attributes = array_keys($attr);
		foreach ($attributes as $attribute) unset ($attr[$attribute]['count']);
		// unset double entries
		for ($i=0; $i<count($attr); $i++)
			if (isset($attr[$i])) unset($attr[$i]);
		foreach ($attributes as $attribute) {
			if (isset($this->attributes[$attribute])) {
				// decode as unicode
				$this->attributes[$attribute] = $attr[$attribute];
				for ($i=0; $i<count($this->attributes[$attribute]); $i++) {
					$this->attributes[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
					$this->orig[$attribute][$i] = utf8_decode ($this->attributes[$attribute][$i]);
					}
				}
			}
		// Add objectClass to orig because we don't want to add objectClass if it's already set
		$this->orig['objectClass'][0] = 'account';
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
	function proccess_attributes($post, $profile=false) {
		// Load attributes
		$this->attributes['description'][0] = $post['description'];
		return 0;
		}

	/* This function will create the html-page
	* to show a page with all attributes.
	* It will output a complete html-table
	*/
	function display_html_attributes($post, $profile=false) {
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Description') ),
			1 => array ( 'kind' => 'input', 'name' => 'description', 'type' => 'text', 'size' => '30',
				'maxlength' => '255', 'value' => $this->attributes['description'][0] ),
			2 => array ('kind' => 'help', 'value' => 'description'));

		return $return;
		}

	function display_html_delete($post, $profile=false) {
		return 0;
		}
	
	/*
	 * (non-PHPDoc)
	 * @see baseModule#get_pdfEntries
	 */
	function get_pdfEntries($account_type = "user") {
		return array('account_description' => array('<block><key>' . _('Description') . '</key><value>' . $this->attributes['description'][0] . '</value></block>'));
	}

}

?>
