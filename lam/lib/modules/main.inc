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
		// Set aliasname
		$this->alias = _('main');
		}


	// Variables

	// This variable stores the number of the current displayed page
	var $current_page;
	// This variable os set to the pagename of a subpage if it should be displayed
	var $subpage;
	// name of accountContainer so we can read other classes in accuontArray
	var $base;
	// Alias Name. This name is shown in the menu instead of main
	var $alias;

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		return array();
		}

	function module_ready() {
		return true;
		}

	// Dummy functions to make module compatible
	function load_attributes($attr) {
		return 0;
		}

	// Dummy functions to make module compatible
	function save_attributes() {
		return array();
		}

	// Dummy functions to make module compatible
	function get_attributes() {
		return array();
		}

	function display_html_attributes($post) {
		// Get list of profiles
		$function = '$profilelist = get'.ucfirst($_SESSION[$this->base]->type).'Profiles();';
		eval($function);
		$modules = $_SESSION[$this->base]->check_attributes();
		if (count($modules)!=0) {
			$disabled = 'disabled';
			// Show reason why module is disabled
			for ($i=0; $i<count($modules); $i++) StatusMessage('ERROR', _('Check module'), sprintf(_('Please set up all required attributes on %s page'), $_SESSION[$this->base]->module[$modules[$i]]->alias));
			}
			else $disabled = '';

		echo "<table border=0 width=\"100%\">\n";
		echo "<tr>\n";
		echo "<td>" . _('Suffix') . "</td>\n";
		echo "<td><select name=\"form_main_suffix\">";
		// loop through all suffixes
		$function = '$suffix = $_SESSION[$_SESSION[$this->base]->config]->get_'.ucfirst($_SESSION[$this->base]->type).'Suffix();';
		eval($function);
		foreach ($_SESSION[$_SESSION[$this->base]->ldap]->search_units($suffix) as $suffix) {
			if ($_SESSION[$this->base]->dn) {
				if ($_SESSION[$this->base]->dn == $suffix) echo "<option selected>$suffix</option>\n";
				else echo "<option>$suffix</option>\n";
				}
			else echo "<option>$suffix</option>\n";
			}
		echo "</select></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=461\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";

		// Show fieldset with list of all user profiles
		if (count($profilelist)!=0) {
			echo "<tr>\n";
			echo "<td>" . _("Load profile") . "</td>\n";
			echo "<td><select name=\"form_main_selectLoadProfile\">";
				foreach ($profilelist as $profile) echo "<option>$profile</option>\n";
			echo "</select>\n";
			echo "<input name=\"form_main_loadProfile\" type=\"submit\" value=\"" . _('Load Profile') . "\"></td>\n";
			echo "<td><a href=\"../help.php?HelpNumber=421\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
			echo "</tr>\n";
			}

		echo "<tr>\n";
		echo "<td>" . _("Save profile") . "</td>\n";
		echo "<td><input name=\"form_main_selectSaveProfile\" type=\"text\" size=\"30\" maxlength=\"50\">\n";
			echo "<input name=\"form_main_saveProfile\" type=\"submit\" value=\"" . _('Save profile') . "\" $disabled ></td>\n";
		echo "<td><a href=\"../help.php?HelpNumber=457\" target=\"lamhelp\">" . _('Help') . "</a></td>\n";
		echo "</tr>\n";
		echo "<tr>\n";
		echo "<td><input name=\"form_main_create\" type=\"submit\" value=\"";
			if ($_SESSION[$this->base]->dn_orig!='') echo _('Modify Account');
	 			else echo _('Create Account');
			echo "\" $disabled ></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		return 0;
		}


	/* This page will be shown if an account
	* has been saved
	*/
	function display_html_finish($post) {
		// Show success message
		if ($_SESSION[$this->base]->dn_orig=='') $kind = _('created');
			else $kind = _('modified');
		$text = sprintf(_('%s has been %s.'), ucfirst($_SESSION[$this->base]->type), $kind);
		StatusMessage('INFO', _('LDAP operation successful.'), $text);

		// Show rest of page
		echo "<table border=0 width=\"100%\">\n";
		echo "<tr>\n";
		if ($_SESSION[$this->base]->dn_orig=='') {
			echo "<td><input name=\"form_main_createagain\" type=\"submit\" value=\"";
				echo sprintf(_('Create another %s'), $_SESSION[$this->base]->type);
				echo "\"></td>\n";
			}
		echo "<td><input name=\"form_main_outputpdf\" type=\"submit\" value=\"" . _('Create PDF file') . "\"></td>\n";
		echo "<td><input name=\"form_main_backmain\" type=\"submit\" value=\"";
			echo sprintf (_('Back to %s list'), $_SESSION[$this->base]->type);
			echo "\"></td>\n";
		echo "</tr>\n";
		echo "</table>\n";
		return 0;
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_finish($post) {
		if ($post['form_main_createagain']) {
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
		if ($post['form_main_backmain']) {
			// Return to *-list
			// *** fixme unset accountContainer in session
			metaRefresh("../lists/list".$_SESSION[$this->base]->type."s.php");
			exit;
			}
		if ($post['form_main_outputpdf']) {
			// Create / display PDf-file
			$function = 'create'.ucfirst($_SESSION[$this->base]->type).'PDF(array($_SESSION[$this->base]));';
			eval($function);
			exit;
			}
		}

	/* Write variables into object and do some regexp checks
	*/
	function proccess_attributes($post) {
		// change dn
		if ($post['form_main_suffix']!='') $_SESSION[$this->base]->dn = $post['form_main_suffix'];
		// load profile
		if ($post['form_main_selectLoadProfile'] && $post['form_main_loadProfile']) {
			// *** fixme load*Profile must return array in the same way ldap_get_attributes does.
			$function = '$newattributes = load'.ucfirst($scope).'Profile($post[\'form_main_selectLoadProfile\']);';
			eval($function);
			// pass newattributes to each module
			$modules = array_keys($_SESSION[$this->base]->module);
			foreach ($modules as $module) $_SESSION[$this->base]->module[$module]->load_attributes($newattributes);
			return 0;
			}
		// save account
		if ($post['form_main_create']) {
			$success = $_SESSION[$this->base]->save_account();
			if (is_array($success)) return array($success);
			// return name of subpage
			return 'finish';
			}
		// save profile
		if ($post['form_main_saveProfile']) {
			if ($post['form_main_selectSaveProfile']=='') $errors[] = array('ERROR', _('Save profile'), _('No profilename given.'));
			else {
				$function = 'save'.ucfirst($scope).'Profile();';
				eval($function);
				if ($function) $errors[] = array('INFO', _('Save profile'), _('New profile created.'));
					else $errors[] = array('ERROR', _('Save profile'), _('Wrong profilename given.'));
				}
			if (is_array($errors)) return $errors;
				else return 0;
			}
		return 0;
		}

	}

?>
