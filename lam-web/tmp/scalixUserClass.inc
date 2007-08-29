<?php
/*

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2006  Tom Pohl

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
* Provides attributes of object class exScalixClass.
*
* @package modules
* @author Tom Pohl
*/

/**
* This class contains all account LDAP attributes
* and funtioncs required to deal with exScalixClass.
*
* @package modules
*/
class scalixUserClass extends baseModule {

        function init($base) {
                // call parent init
                parent::init($base);
                $this->scalixScalixObject = false;
                $this->scalixAdministrator = false;
                $this->scalixMailboxAdministrator = false;
                }


	/** used for account pages, true if input data is correct */
	var $inputCorrect = true;
	var $scalixScalixObject;
	var $scalixMailnode;
	var $scalixEmailAddress;
	var $scalixAdministrator;
	var $scalixMailboxAdministrator;
	var $scalixMailboxClass;

	/**
	* Returns meta data that is interpreted by parent class
	*
	*/
	function get_metaData() {
		$return = array();
		// manages user accounts
		$return["account_types"] = array("user");
		// alias name
		$return["alias"] = _("Scalix");
		// module dependencies
		$return['dependencies'] = array('depends' => array(), 'conflicts' => array());
		// help Entries
		$return['help'] = array (
			"scalixuser" => array(
				"ext" => "FALSE", "Headline" => _("scalixScalixObject"),
				"Text" => _("boolean TRUE or FALSE for creating scalix object")),
			"mailnode" => array(
				"ext" => "FALSE", "Headline" => _("scalixMailnode"),
				"Text" => _("comma separated org units for object mail node")),
			"mboxaddr" => array(
				"ext" => "FALSE", "Headline" => _("scalixEmailAddress"),
				"Text" => _("E-Mail Adress of Scalix User")),
			"scalixadmin" => array(
				"ext" => "FALSE", "Headline" => _("scalixAdministrator"),
				"Text" => _("boolean TRUE or FALSE for admin capability")),
			"mboxadmin" => array(
				"ext" => "FALSE", "Headline" => _("scalixMailboxAdministrator"),
				"Text" => _("boolean TRUE or FALSE for mboxadmin capability")),
			"mboxclass" => array(
				"ext" => "FALSE", "Headline" => _("scalixMailboxClass"),
				"Text" => _("FULL or LIMITED for Class.  LIMITED is a Scalix Standard User, FULL is a Scalix Premium User"))
		);
		$return['attributes'] = array('scalixScalixObject', 'scalixAdministrator', 'scalixMailboxAdministrator', 'scalixMailnode','scalixMailboxClass','scalixEmailAddress');
		$return['objectClasses'] = array('scalixUserClass');
		return $return;
	}

	/**
	* This function loads all needed attributes into the object.
	*
	* @param array $attr an array as it is retured from ldap_get_attributes
	*/
	function load_attributes($attr) {
		parent::load_attributes($attr);
		//scalixScalixObject
		if (is_string($this->attributes['scalixScalixObject'][0])) {
			if ($this->attributes['scalixScalixObject'][0] == "TRUE") $this->scalixScalixObject = true;
			else $this->scalixScalixObject = false;
		}
		//scalixAdministrator
		if (is_string($this->attributes['scalixAdministrator'][0])) {
			if ($this->attributes['scalixAdministrator'][0] == "TRUE") $this->scalixAdministrator = true;
			else $this->scalixAdministrator = false;
		}
		//scalixMailboxAdministrator
		if (is_string($this->attributes['scalixMailboxAdministrator'][0])) {
			if ($this->attributes['scalixMailboxAdministrator'][0] == "TRUE") $this->scalixMailboxAdministrator = true;
			else $this->scalixMailboxAdministrator = false;
		}
		//scalixMailnode
		$this->attributes['objectClass'] = array();
		$this->attributes['scalixMailnode'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['scalixMailnode'] = array();
		if (isset($attr['objectClass'])) {
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		if (isset($attr['scalixMailnode'])) {
			$this->attributes['scalixMailnode'] = $attr['scalixMailnode'];
			$this->orig['scalixMailnode'] = $attr['scalixMailnode'];
		}

		//scalixEmailAddress
		$this->attributes['objectClass'] = array();
		$this->attributes['scalixEmailAddressx'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['scalixEmailAddress'] = array();
		if (isset($attr['objectClass'])) {
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		if (isset($attr['scalixEmailAddress'])) {
			$this->attributes['scalixEmailAddress'] = $attr['scalixEmailAddress'];
			$this->orig['scalixEmailAddress'] = $attr['scalixEmailAddress'];
		}
		//scalixMailboxClass
		$this->attributes['objectClass'] = array();
		$this->attributes['scalixMailboxClass'] = array();
		$this->orig['objectClass'] = array();
		$this->orig['scalixMailboxClass'] = array();
		if (isset($attr['objectClass'])) {
			$this->attributes['objectClass'] = $attr['objectClass'];
			$this->orig['objectClass'] = $attr['objectClass'];
		}
		if (isset($attr['scalixMailboxClass'])) {
			$this->attributes['scalixMailboxClass'] = $attr['scalixMailboxClass'];
			$this->orig['scalixMailboxClass'] = $attr['scalixMailboxClass'];
		}
		// add object class if needed
		if (! in_array('scalixUserClass', $this->orig['objectClass'])) {
			$this->attributes['objectClass'][] = 'scalixUserClass';
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
	* This function will create the meta HTML code to show a page with all attributes.
	*
	* @param array $post HTTP-POST values
	*/
	function display_html_attributes() {
		$return = array();
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Scalix User') ),
				1 => array ( 'kind' => 'input', 'name' => 'scalixScalixObject', 'type' => 'checkbox', 'checked' => $this->scalixScalixObject, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'scalixuser'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Mailnode') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'scalixMailnode', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['scalixMailnode'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'mailnode' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Mboxaddr') ),
				1 => array ( 'kind' => 'input', 'type' => 'text', 'name' => 'scalixEmailAddress', 'size' => '20', 'maxlength' => '255', 'value' => $this->attributes['scalixEmailAddress'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'mboxaddr' ));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Scalix Admin') ),
				1 => array ( 'kind' => 'input', 'name' => 'scalixAdministrator', 'type' => 'checkbox', 'checked' => $this->scalixAdministrator, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'scalixadmin'));
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Scalix Mailbox Admin') ),
				1 => array ( 'kind' => 'input', 'name' => 'scalixMailboxAdministrator', 'type' => 'checkbox', 'checked' => $this->scalixMailboxAdministrator, 'value' => 'true'),
				2 => array ('kind' => 'help', 'value' => 'mboxadmin'));
			$mboxes = array(0 => 'LIMITED', 1 => 'FULL');
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Scalix Mailbox Class') ),
				1 => array ( 'kind' => 'select', 'type' => 'text', 'name' => 'scalixMailboxClass', 'options' => $mboxes, 'size' => '1', 'maxlength' => '255', 'options_selected' => $this->attributes['scalixMailboxClass'][0]),
				2 => array ( 'kind' => 'help', 'value' => 'mboxclass' ));
		return $return;
	}

	function get_pdfEntries() {
		return array(
		'scalixUserClass_scalixScalixObject' => array('<block><key>' . _('Scalix User') . '</key><value>' . $this->attributes['scalixScalixObject'][0] . '</value></block>'),
		'scalixUserClass_scalixMailnode' => array('<block><key>' . _('Mailnode') . '</key><value>' . $this->attributes['scalixMailnode'][0] . '</value></block>'),
		'scalixUserClass_scalixMboxaddr' => array('<block><key>' . _('Mboxaddr') . '</key><value>' . $this->attributes['scalixEmailAddress'][0] . '</value></block>'),
		'scalixUserClass_scalixMailboxClass' => array('<block><key>' . _('Scalix Mailbox Class') . '</key><value>' . $this->attributes['scalixMailboxClass'][0] . '</value></block>'),
		'scalixUserClass_scalixAdministrator' => array('<block><key>' . _('Scalix Admin') . '</key><value>' . $this->attributes['scalixAdministrator'][0] . '</value></block>'),
		'scalixUserClass_scalixMailboxAdministrator' => array('<block><key>' . _('Scalix Mailbox Admin') . '</key><value>' . $this->attributes['scalixMailboxAdministrator'][0] . '</value></block>'));
	}

	/**
	* Processes user input of the primary module page.
	* It checks if all input values are correct and updates the associated LDAP attributes.
	*
	* @param array $post HTTP-POST values
	* @return array list of info/error messages
	*/
	function process_attributes() {
		$this->triggered_messages = array();
		if (isset($_POST['scalixScalixObject'])) {
			$this->scalixScalixObject = true;
			$this->attributes['scalixScalixObject'][0] = "TRUE";
		} else {
			$this->scalixScalixObject = false;
			$this->attributes['scalixScalixObject'][0] = "FALSE";
		}
		$this->attributes['scalixMailnode'][0] = $_POST['scalixMailnode'];
		$this->attributes['scalixEmailAddress'][0] = $_POST['scalixEmailAddress'];
		$this->attributes['scalixMailboxClass'][0] = $_POST['scalixMailboxClass'];
		if (isset($_POST['scalixAdministrator'])) {
			$this->scalixAdministrator = true;
			$this->attributes['scalixAdministrator'][0] = "TRUE";
		} else {
			$this->scalixAdministrator = false;
			$this->attributes['scalixAdministrator'][0] = "FALSE";
		}
		if (isset($_POST['scalixMailboxAdministrator'])) {
			$this->scalixMailboxAdministrator = true;
			$this->attributes['scalixMailboxAdministrator'][0] = "TRUE";
		} else {
			$this->scalixMailboxAdministrator = false;
			$this->attributes['scalixMailboxAdministrator'][0] = "FALSE";
		}

		if (sizeof($this->triggered_messages) > 0) {
			$this->inputCorrect = false;
			return $this->triggered_messages;
		}
		else {
			$this->inputCorrect = true;
			return 0;
		}
	}

	/**
	* This function returns true if all needed settings are done.
	*/
	function module_complete() {
		return $this->inputCorrect;
	}

	/**
	* Returns true if all settings on module page are correct.
	*/
	function module_ready() {
		return $this->inputCorrect;
	}

	function get_profileOptions() {
		$return = array();
		// use Unix password as Samba password
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Scalix User') . ': '),
			1 => array('kind' => 'input', 'name' => 'scalixUserClass_scalixScalixObject', 'type' => 'checkbox', 'checked' => true),
			2 => array('kind' => 'help', 'value' => 'scalixuser')
			);
		$mboxes = array(0 => 'LIMITED', 1 => 'FULL');
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Mailnode') . ': '),
			1 => array('kind' => 'input', 'type' => 'text', 'name' => 'scalixUserClass_scalixMailnode', 'size' => '30', 'maxlength' => '255', 'value' => ''),
			2 => array('kind' => 'help', 'value' => 'mailnode')
			);
        	$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Mboxaddr') . ': '),
			1 => array('kind' => 'select', 'type' => 'text', 'name' => 'scalixUserClass_scalixEmailAddress', 'size' => '30', 'maxlength' => '255', 'value' => ''),
			2 => array('kind' => 'help', 'value' => 'mboxaddr')
			);
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Scalix Mailbox Class') . ': '),
			1 => array('kind' => 'select', 'type' => 'text', 'name' => 'scalixUserClass_scalixMailboxClass', 'size' => '1', 'maxlength' => '255', 'options' => $mboxes, 'options_selected' => $this->attributes['scalixMailboxClass'][0]),
			2 => array('kind' => 'help', 'value' => 'mboxclass')
			);
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Scalix Admin') . ': '),
			1 => array('kind' => 'input', 'name' => 'scalixUserClass_scalixAdministrator', 'type' => 'checkbox', 'checked' => false),
			2 => array('kind' => 'help', 'value' => 'scalixadmin')
			);
		$return[] = array(
			0 => array('kind' => 'text', 'text' => _('Scalix Mailbox Admin') . ': '),
			1 => array('kind' => 'input', 'name' => 'scalixUserClass_scalixMailboxAdministrator', 'type' => 'checkbox', 'checked' => false),
			2 => array('kind' => 'help', 'value' => 'mboxadmin')
			);
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
		// special profile options
		if ($profile['scalixUserClass_scalixScalixObject'][0] == "true") {
			$this->scalixScalixObject = true;
		}
		elseif ($profile['scalixUserClass_scalixScalixObject'][0] == "false") {
			$this->scalixScalixObject = false;
		}
		if ($profile['scalixUserClass_scalixAdministrator'][0] == "true") {
			$this->scalixAdministrator = true;
		}
		elseif ($profile['scalixUserClass_scalixAdministrator'][0] == "false") {
			$this->scalixAdministrator = false;
		}
		if ($profile['scalixUserClass_scalixMailboxAdministrator'][0] == "true") {
			$this->scalixMailboxAdministrator = true;
		}
		elseif ($profile['scalixUserClass_scalixMailboxAdministrator'][0] == "false") {
			$this->scalixMailboxAdministrator = false;
		}
		if (isset($profile['scalixUserClass_scalixMailnode'][0])) {
			$this->attributes['scalixMailnode'][0] = $profile['scalixUserClass_scalixMailnode'][0];
		}
		if (isset($profile['scalixUserClass_scalixEmailAddress'][0])) {
			$this->attributes['scalixEmailAddress'][0] = $profile['scalixUserClass_scalixEmailAddress'][0];
		}
		if (isset($profile['scalixUserClass_scalixMailboxClass'][0])) {
			$this->attributes['scalixMailboxClass'][0] = $profile['scalixUserClass_scalixMailboxClass'][0];
		}
	}
}


?>

