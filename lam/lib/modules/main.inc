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


/* Main-Module. Contains basic module functions have to be loaded first before
* any other module. This module doesn't support any ldap functions
* ldap functions in this module are only dummy functions
* It also chooses which page to show.
*/
class main {
	// Constructor
	function main($base) {
		// Set counter to first page
		$this->current_page = 0;
		// reset subpage counter
		$this->subpage = '';
		// Get local copy of name of account_container in session
		$this->base = $base;
		}


	// Variables
	// This variable stores the number of the current displayed page
	var $current_page;
	// This variable os set to the pagename of a subpage if it should be displayed
	var $subpage;
	// name of accountContainer so we can read other classes in accuontArray
	var $base;

	function get_alias() {
		return _('main');
		}

	/* This function returns a list with all required modules
	*/
	function get_dependencies($scope) {
		return array('require' => array(), 'conflict' => array() );
		}

	function module_ready() {
		return true;
		}

	/* This function returns a list of all html-pages in module
	* This is usefull for mass upload and pdf-files
	* because lam can walk trough all pages itself and do some
	* error checkings
	*/
	function pages() {
		return array('attributes', 'finish');
		}

	/*
	*/
	function get_help($id) {
		switch ($id) {
			case "description":
				return array ("ext" => "FALSE", "Headline" => _("Description"),
					"Text" => _("Host Description."));
				break;
			}
		return false;
		}

	// Dummy functions to make module compatible
	function get_attributes() {
		return array();
		}

	// Dummy functions to make module compatible
	function load_attributes($attr) {
		return 0;
		}

	// Dummy functions to make module compatible
	function save_attributes() {
		return array();
		}

	function delete_attributes($post) {
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post, $profile=false) {
		// change dn
		if ($post['suffix']!='') $_SESSION[$this->base]->dn = $post['suffix'];
		// load profile
		if ($post['selectLoadProfile'] && $post['loadProfile']) {
			// *** fixme load*Profile must return array in the same way ldap_get_attributes does.
			$function = '$newattributes = load'.ucfirst($scope).'Profile($post[\'selectLoadProfile\']);';
			eval($function);
			// pass newattributes to each module
			$modules = array_keys($_SESSION[$this->base]->module);
			foreach ($modules as $module) $_SESSION[$this->base]->module[$module]->load_attributes($newattributes);
			return 0;
			}
		// save account
		if ($post['create']) {
			$errors = $_SESSION[$this->base]->save_account();
			if (is_array($errors)) return array($errors);
			// return name of subpage
			return 'finish';
			}
		// save profile
		if ($post['saveProfile']) {
			if ($post['selectSaveProfile']=='') $errors['saveProfile'][] = array('ERROR', _('Save profile'), _('No profilename given.'));
			else {
				$function = 'save'.ucfirst($scope).'Profile();';
				eval($function);
				if ($function) $errors['saveProfile'][] = array('INFO', _('Save profile'), _('New profile created.'));
					else $errors['saveProfile'][] = array('ERROR', _('Save profile'), _('Wrong profilename given.'));
				}
			if (is_array($errors) && !$profile) return $errors;
				else return 0;
			}
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_finish($post, $profile=false) {
		if ($post['createagain']) {
			// Reset objects
			$modules = array_keys($_SESSION[$this->base]->module);
			foreach ($modules as $module)
				if ($module!='main') unset($_SESSION[$this->base]->module[$module]);
			// Reset accountContainer
			$_SESSION[$this->base]->dn = '';
			$_SESSION[$this->base]->dn_orig = '';
			$_SESSION[$this->base]->attributes = array();
			$_SESSION[$this->base]->order = array();
			$this->current_page = 0;
			$this->subpage = '';
			// Add all required objects etc.
			$_SESSION[$this->base]->new_account();
			return 0;
			}
		if ($post['backmain']) {
			// Return to *-list
			// *** fixme unset accountContainer in session
			metaRefresh("../lists/list".$_SESSION[$this->base]->type."s.php");
			exit;
			}
		if ($post['outputpdf']) {
			// Create / display PDf-file
			$function = 'create'.ucfirst($_SESSION[$this->base]->type).'PDF(array($_SESSION[$this->base]));';
			eval($function);
			exit;
			}
		}

	function display_html_attributes($post, $profile=false) {
		$modules = $_SESSION[$this->base]->check_attributes();
		if (count($modules)!=0) {
			$disabled = false;
			// Show reason why module is disabled
			for ($i=0; $i<count($modules); $i++)
				$return[] = array ( 0 => array ( 'kind' => 'message', 'type' => 'ERROR', 'headline' =>  _('Check module'),
					'text' => sprintf(_('Please set up all required attributes on %s page.'), $_SESSION[$this->base]->module[$modules[$i]]->get_alias()) ));
			}
			else $disabled = true;

		// loop through all suffixes
		$function = '$suffix = $_SESSION[$_SESSION[$this->base]->config]->get_'.ucfirst($_SESSION[$this->base]->type).'Suffix();';
		eval($function);
		foreach ($_SESSION[$_SESSION[$this->base]->ldap]->search_units($suffix) as $suffix) {
			if ($_SESSION[$this->base]->dn) {
				if ($_SESSION[$this->base]->dn == $suffix) $option_selected = $suffix;
				else $suffixes[] = $suffix;
				}
			else $suffixes[] = $suffix;
			}
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _('Suffix') ),
			1 => array ( 'kind' => 'select', 'name' => 'suffix', 'options' => $suffixes,
				'option_selected' => $selected_suffix ),
			2 => array ('kind' => 'help', 'value' => 'suffix'));

		if (!$profile) {
			// Get list of profiles
			$function = '$profilelist = get'.ucfirst($_SESSION[$this->base]->type).'Profiles();';
			eval($function);
			if (count($profilelist)!=0) {
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _("Load profile") ),
					1 => array ( 'kind' => 'select', 'name' => 'selectLoadProfile', 'options' => $profilelist ),
					2 => array ('kind' => 'help', 'value' => 'selectLoadProfile'));
				}
			$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => _("Save profile") ),
				1 => array ( 'kind' => 'table', 'value' => array ( 0 => array (
					0 => array ( 'kind' => 'input', 'name' => 'selectSaveProfile', 'type' => 'text', 'size' => '30',
						'maxlength' => '255', ),
					1 => array ('kind' => 'input', 'name' => 'saveProfile', 'type' => 'submit',
						'value' => _('Save profile'), 'disabled' => $disabled))) ),
				2 => array ('kind' => 'help', 'value' => 'saveProfile'));
			}

		if ($_SESSION[$this->base]->dn_orig!='') $text = _('Modify Account');
 			else $text = _('Create Account');
		$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => $text ),
			1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'create', 'value' => $text ),
			2 => array ('kind' => 'help', 'value' => 'create'));
		return $return;
		}

	function display_html_delete($post, $profile=false) {
		return 0;
		}

	/* This page will be shown if an account
	* has been saved
	*/
	function display_html_finish($post, $profile=false) {
		// Show success message
		if ($_SESSION[$this->base]->dn_orig=='') $kind = _('created');
			else $kind = _('modified');
		$text = sprintf(_('%s has been %s.'), ucfirst($_SESSION[$this->base]->type), $kind);

		if (!$profile) {
			$return[] = array ( 0 => array ( 'kind' => 'message', 'type' => 'INFO', 'headline' =>  _('LDAP operation successful.'),
				'text' => $text ));

			$return[] = array ( 0 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'createagain',
					'value' => sprintf(_('Create another %s'), $_SESSION[$this->base]->type) ),
				1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'outputpdf',
					'value' => _('Create PDF file') ),
				2 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'backmain',
					'value' => sprintf (_('Back to %s list'), $_SESSION[$this->base]->type) ));
			}

		return $return;
		}

	}

?>
