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


  LDAP Account Manager functions used by account.php
*/

/* This class contains all functions
* which are needed to manage the ldap cache
*/
class cache {
	function cache() {
		$this->config =& $_SESSION['config'];
		$this->ldap =& $_SESSION['ldap'];
		$this->time = 0;
		$this->attributes = array();
		}

	var $ldapcache; // This variable contains the cache
	var $attributes; // This variable contains a list and their scope of attributes which should be cached
	var $config; // This is a reference to the config class in session
	var $ldap; // This is a reference to the ldap class in session
	var $time; // This is the laste timestamp ldap cache has been refreshed

	/* This function adds attributes to cache
	* syntax of $attributes is array( scope1 => array ( attributes ), scope2 => array ( attributes ), ...)
	*/
	function add_cache($attributes) {
		$this->refresh_cache();
		// Check input variable
		$allowed_types = array ( 'user', 'group', 'host', '*' );
		if (!is_array($attributes)) trigger_error(_('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).'), E_USER_ERROR);
		foreach ($attributes as $attribute) {
			if (!is_array($attribute)) trigger_error(_('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).'), E_USER_ERROR);
			foreach ($attribute as $singleattribute) {
				if (!is_string($singleattribute)) trigger_error(_('Argument of add_cache must be : array ( scope => array(attribute1(string), attribute2(string), ..), scope => ... ).'), E_USER_ERROR);
				}
			}
		$scopes = array_keys($attributes);
		foreach ($scopes as $scope) {
			if (!@in_array($scope, $allowed_types)) trigger_error(sprintf(_('Invalid scope. Valid scopes are %s.'), implode(" ", $allowed_types)), E_USER_ERROR);
			}
		// Everything seems to be OK, start processing data
		foreach ($scopes as $scope) {
			for ($i=0; $i<count($attributes[$scope]); $i++ ) {
				if (!@in_array($attributes[$scope][$i] ,$this->attributes[$scope])) $this->attributes[$scope][] = $attributes[$scope][$i];
				}
			}
		}

	/* This function returns an array ( dn1 => array(uidnumber1), dn2 => array(uidnumber2), ... )
	*
	*/
	function get_cache($attribute, $objectClass, $singlescope) {
		$this->refresh_cache();
		// Check input variables
		$allowed_types = array ( 'user', 'group', 'host', '*' );
		if (!in_array($singlescope, $allowed_types)) trigger_error(sprintf(_('Invalid scope. Valid scopes are %s.'), implode(" ", $allowed_types)), E_USER_ERROR);
		$line=-1;
		for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
			if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME '$objectClass'")) $line = $i;
			}
		// Return error if objectClass isn't found
		if ($line==-1) trigger_error (sprintf(_("objectClass %s required but not defined in ldap."), $objectClass), E_USER_WARNING);
		// Everything seems to be OK, start processing data
		$this->refresh_cache();
		if ($singlescope == '*') $scopes = $allowed_types;
			else $scopes = array ( $singlescope );
		foreach ($scopes as $scope) {
			if (isset($this->ldapcache[$scope])) {
				$DNs = array_keys($this->ldapcache[$scope]);
				foreach ($DNs as $dn) {
					if (isset($this->ldapcache[$scope][$dn][$attribute]) && in_array($objectClass, $this->ldapcache[$scope][$dn]['objectClass'])) {
						// return string if only attribute exists only once
						if (count($this->ldapcache[$scope][$dn][$attribute])==1) $return[$dn][] = $this->ldapcache[$scope][$dn][$attribute][0];
						else {
							// else return array with all attributes
							$return[$dn] = $this->ldapcache[$scope][$dn][$attribute];
							}
						}
					}
				}
			}
		return $return;
		}

	/* This functions returns the dn if a dn with $attribute=$value is found
	* $values is the value $attribute is set to
	* $scope is the scope where to search
	*/
	function in_cache($value, $attribute, $singlescope) {
		$this->refresh_cache();
		// Check input variables
		$allowed_types = array ( 'user', 'group', 'host', '*' );
		if (!in_array($singlescope, $allowed_types)) trigger_error(sprintf(_('Invalid scope. Valid scopes are %s.'), implode(" ", $allowed_types)), E_USER_ERROR);
		// Create list of all allowed attributes
		for ($i=0; $i<count($this->ldap->objectClasses); $i++ ) {
			if (strpos($this->ldap->objectClasses[$i],  'MUST (')) {
				$string_withtail = substr($this->ldap->objectClasses[$i], strpos($this->ldap->objectClasses[$i],  'MUST (')+6);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$allowed_attributes = array_merge($allowed_attributes, explode(" $ ", $string));
				}
			// create array with may-attributes
			// Get startposition in string
			if (strpos($this->ldap->objectClasses[$i],  'MAY (')) {
				$string_withtail = substr($this->ldap->objectClasses[$i], strpos($this->ldap->objectClasses[$i],  'MAY (')+5);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$allowed_attributes = array_merge($allowed_attributes, explode(" $ ", $string));
				}
			}
		$allowed_attributes = array_unique($allowed_attributes);
		if (!in_array($attribute, $allowed_attributes)) trigger_error(_('Attribute not defined in LDAP.'), E_USER_WARNING);
		// Everything seems to be OK, start processing data
		$this->refresh_cache();
		if ($singlescope == '*') $scopes = $allowed_types;
			else $scopes = array ( $singlescope );
		//print_r($this->ldapcache);
		foreach ($scopes as $scope) {
			if (isset($this->ldapcache[$scope])) {
				$DNs = array_keys($this->ldapcache[$scope]);
				foreach ($DNs as $dn) {
					if (is_array($this->ldapcache[$scope][$dn][$attribute])) {
						if (in_array($value, $this->ldapcache[$scope][$dn][$attribute])) {
							// Return value if value was found
							return $dn;
							}
						}
					}
				}
			}
		// Return false if value wasn't found
		return false;
		}


	/* This functions refreshs the cache
	*/
	function refresh_cache() {
		if ($time + $this->config->get_cacheTimeoutSec() < time()) {
			// unset old cache
			unset ($this->ldapcache);
			$scopes = array_keys($this->attributes);
			foreach ($scopes as $scope) {
				// Get Scope
				$function = '$suffix = $this->config->get_'.ucfirst($scope).'Suffix();';
				If ($scope != '*') eval($function);
					else $suffix = '';
				// Get Data from ldap
				$search = $this->attributes[$scope];
				$search[] = 'objectClass';
				$result = @ldap_search($this->ldap->server(), $suffix, 'objectClass=*', $search, 0);
				// Write search result in array
				$entry = @ldap_first_entry($this->ldap->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($this->ldap->server(), $entry));
					$attr = ldap_get_attributes($this->ldap->server(), $entry);
					// unset every count entry
					unset ($attr['count']);
					$attributes = array_keys($attr);
					foreach ($attributes as $attribute) unset ($attr[$attribute]['count']);
					// unset double entries
					for ($i=0; $i<count($attr); $i++) {
						if (isset($attr[$i])) unset($attr[$i]);
						}
					// Write new cache entry
					$addcache = $attr;
					unset ($addcache['objectClass']);
					if (count($addcache)!=0) $this->ldapcache[$scope][$dn] = $attr;
					$entry = ldap_next_entry($this->ldap->server(), $entry);
					}
				}
			$this->time = time();
			}
		}

	/* This function update the cache when changes were
	* made without refrehing the complete cache
	*/
	function update_cache($dn, $attributes, $singlescope) {
		$allowed_types = array ( 'user', 'group', 'host', '*' );
		if (!in_array($singlescope, $allowed_types)) trigger_error(sprintf(_('Invalid scope. Valid scopes are %s.'), implode(" ", $allowed_types)), E_USER_ERROR);
		// Everything seems to be OK, start processing data
		$this->refresh_cache();
		if (isset($this->ldapcache[$singlescope][$dn])) unset($this->ldapcache[$singlescope][$dn]);
		$attrnames = array_keys ($this->attributes[$singlescope]);
		foreach ($attrnames as $name) {
			if (is_string($attributes[$name])) $this->ldapcache[$singlescope][$dn][$name][] = $attributes[$name];
			if (is_array($attributes[$name])) $this->ldapcache[$singlescope][$dn][$name] = $attributes[$name];
			}
		}

	}

/* Main-Module. Contains basic module functions have to be loaded first
* It also chooses which page to show.
*/
class main {
	// This variable stores the number of the current displayed page
	var $current_page;
	// This variable os set to the pagename of a subpage if it should be displayed
	var $subpage;
	// name of accountContainer so we can read other classes in accuontArray
	var $base;
	// Alias Name. This name is shown in the menu instead of main
	var $alias;

	function main($base) {
		$this->current_page = 0;
		$this->subpage = '';
		/* Create a reference to basearray so we can read all other modules
		* php will avaois recousrion itself
		*/
		$this->base = $base;
		$this->alias = _('main');
		}

	/* This function returns a list with all required modules
	*/
	function dependencies() {
		return array();
		}

	function display_html_attributes($post) {
		$function = '$profilelist = get'.ucfirst($_SESSION[$this->base]->type).'Profiles();';
		eval($function);
		$modules = $_SESSION[$this->base]->check_attributes();
		if (count($modules)!=0) {
			$disabled = 'disabled';
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


	}


class accountContainer {
	// Constructor
	function accountContainer($type, $base) {
		/* Set the type of account. Valid
		* types are: user, group, host
		*/
		// Check input variable
		if (!is_string($type)) trigger_error(_('Argument of accountContainer must be string.'), E_USER_ERROR);
		if (!is_string($base)) trigger_error(_('Argument of accountContainer must be string.'), E_USER_ERROR);
		// *** fixme use global variable to determine allowed types
		$allowed_types = array ( 'user', 'group', 'host' );
		if (!in_array($type, $allowed_types)) trigger_error(_('Account type not recognized.'), E_USER_ERROR);
		$this->type = $type;
		$this->base = $base;
		// Name of variables in session
		$this->ldap = 'ldap';
		$this->config = 'config';
		$this->cache = 'cache';
		$this->header2 = 'header';
		$this->module['main'] = new main($this->base);
		return 0;
		}

	/* Array of all used attributes
	* Syntax is attribute => array ( objectClass => MUST or MAY, ...)
	*/
	var $attributes;
	/* This variale stores the type
	* of account. Current unix, group, host are supported
	*/
	var $type;
	var $ldap; // This is a reference to the ldap class in session
	var $config; // This is a reference to the config class in session
	// Localized part of HTML-Header
	var $header2;
	var $module; // This is an array with all module objects
	// DN of the account
	var $dn;
	var $dn_orig;
	// this are stores the module order
	var $order;
	// name of accountContainer so we can read other classes in accuontArray
	var $base;

	/* Get the type of account. Valid
	* types are: user, group, host
	*/
	function get_type() {
		return $this->type;
		}

	/* This function asks $this->module['main']
	* what to do next
	*/
	function continue_main($post) {
		if ($this->module['main']->subpage=='') $this->module['main']->subpage='attributes';
		$function = '$result = $this->module[$this->order[$this->module[\'main\']->current_page]]->proccess_'.$this->module['main']->subpage.'($post);';
		eval ($function);
		if (is_string($result)) $this->module['main']->subpage = $result;
		if (is_int($result)) {
			for ($i=0; $i<count($this->order); $i++ ) {
				if ($post['form_main_'.$this->order[$i]]) $this->module['main']->current_page = $i;
				}
			$this->module['main']->subpage='attributes';
			}

		// Write HTML-Code
		echo $_SESSION[$this->header2];
		echo "<title>";
		echo _("Create new Account");
		echo "</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";
		echo "<form action=\"useredit.php\" method=\"post\">\n";
		// Display errir-messages
		if (is_array($result))
			for ($i=0; $i<sizeof($result); $i++) StatusMessage($result[$i][0], $result[$i][1], $result[$i][2]);

		// Create left module-menu
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr>";
		echo "<td><fieldset class=\"useredit-dark\"><legend class=\"useredit-bright\"><b>";
			echo _('Please select page:');
			echo "</b></legend>\n";
		// Loop for module
		for ($i=0; $i<count($this->order); $i++ ) {
			if ($this->order[$i]==$this->order[$this->module['main']->current_page]) {
				// print disabled button
				echo "<input name=\"form_main_".$this->order[$i]."\" type=\"submit\" value=\"";
				echo $this->module[$this->order[$i]]->alias;
				echo "\" disabled>\n<br>";
				}
			else {
				// print normal button
				echo "<input name=\"form_main_".$this->order[$i]."\" type=\"submit\" value=\"";
				echo $this->module[$this->order[$i]]->alias;
				echo "\">\n<br>";
				}
			}
		// *** Fixme add reset-button
		echo "</fieldset></td></tr>\n";
		echo "</table></td>\n<td>";
		// display html-code from mdule
		$function = '$result = $this->module[$this->order[$this->module[\'main\']->current_page]]->display_html_'.$this->module['main']->subpage.'($post);';
		eval ($function);
		// Display rest of html-page
		echo "</td></tr></table>\n";
		echo "</form>\n";
		echo "</body>\n";
		echo "</html>\n";
		return 0;
		}

	/* Add attributes to variable. Syntax is array( attribute = array ( objectClass1 => MUST|MAX, objectClass2 => MUST|MAY ), ... )
	*/
	function add_attributes($objectClass) {
		// loop through every existing objectlass and select current objectClass
		$line=-1;
		for ($i=0; $i<count($_SESSION[$this->ldap]->objectClasses) || $i==-1; $i++) {
			if (strpos($_SESSION[$this->ldap]->objectClasses[$i], "NAME '$objectClass'")) $line = $i;
			}
		// Return error if objectClass isn't found
		if ($line==-1) trigger_error (sprintf(_("objectClass %s required but not defined in ldap."), $objectClass), E_USER_WARNING);
		// create array with must-attributes
		// Get startposition in string
		if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')) {
			$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')+6);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			$must = explode(" $ ", $string);
			// Ad must
			foreach ($must as $attribute) {
				if (!isset($this->attributes[$attribute])) $this->attributes[$attribute][$objectClass] = 'MUST';
					else $this->attributes[$attribute][$objectClass] = 'MUST';
				}
			}
		// create array with may-attributes
		// Get startposition in string
		if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')) {
			$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')+5);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			$may = explode(" $ ", $string);
			// Ad may
			foreach ($may as $attribute) {
				if (!isset($this->attributes[$attribute])) $this->attributes[$attribute][$objectClass] = 'MAY';
					else $this->attributes[$attribute][$objectClass] = 'MAY';
				}
			}

		// Get attributes of subclasses
		while (strpos($_SESSION[$this->ldap]->objectClasses[$line], "SUP ")) {
			$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'SUP ')+4);
			$subclass = substr($string_withtail, 0, strpos($string_withtail, ' '));
			// Add account type to object
			for ($i=0; $i<count($_SESSION[$this->ldap]->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION[$this->ldap]->objectClasses[$i], "NAME '$subclass'")) $line = $i;
				}
			// Return error if objectClass isn't found
			if ($line==-1) trigger_error (sprintf(_("objectClass %s required but not defined in ldap."), $objectClass), E_USER_WARNING);


			// create array with must-attributes
			// Get startposition in string
			if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')) {
				$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')+6);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$must = explode(" $ ", $string);
				// Ad must
				foreach ($must as $attribute) {
					if (!isset($this->attributes[$attribute])) $this->attributes[$attribute][$objectClass] = 'MUST';
						else $this->attributes[$attribute][$objectClass] = 'MUST';
					}
				}
			// create array with may-attributes
			// Get startposition in string
			if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')) {
				$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')+5);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				$may = explode(" $ ", $string);
				// Ad may
				foreach ($may as $attribute) {
					if (!isset($this->attributes[$attribute])) $this->attributes[$attribute][$objectClass] = 'MAY';
						else $this->attributes[$attribute][$objectClass] = 'MAY';
					}
				}
			}


		}

	/* This function return ldap attributes
	* Syntax is get_attributes($value, $scope)
	* $scope = 'objectClass', $value = objectClass return value are all attributes of objectClass
	* $scope = 'attribute', $value = attribute returns alle objectClasses which are using the attribute
	*/
	function get_attributes($value, $scope) {
		if ($scope=='attribute' && isset($this->attributes[$value])) return $this->attributes[$value];
		if ($scope=='objectClass') {
			$keys = array_keys($this->attributes);
			foreach ($keys as $attribute) {
				if (isset($this->attributes[$attribute][$value])) $return[$attribute] = $this->attributes[$attribute][$value];
				}
			return $return;
			}
		return 0;
		}

	/* This function checks if all MUST-attribtues are set.
	* If not it will return an array with all modules
	* which have to be set first
	*/
	function check_attributes() {
		$return = array();
		// get named list of attributes
		$attributes = array_keys($this->attributes);
		for ($i=0; $i<count($attributes); $i++) {
			$singleattribute = array_keys ($this->attributes[$attributes[$i]]);
			for ($j=0; $j<count($singleattribute); $j++) {
				// found attribute which must be set
				if ($this->attributes[$attributes[$i]][$singleattribute[$j]]=='MUST') {
					// Check if attribute is set
					if ($this->module[$singleattribute[$j]]->attributes[$attributes[$i]]=='')
						if (!in_array($singleattribute[$j], $return)) $return[] = $singleattribute[$j];
					}
				}
			}
		return $return;
		}

	/* This function adds an objectClass class (module) to accountContainer
	*/
	function add_objectClass($objectClass) {
		$line=-1;
		for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
			if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME '$objectClass'")) $line = $i;
			}
		// Return error if objectClass isn't found
		if ($line==-1) trigger_error (_("objectClass $objectClass required but not defined in ldap."), E_USER_WARNING);
		else {
			// Add module if it exists
			if (class_exists($objectClass)) {
				$this->module[$objectClass] = new $objectClass($this->base);
				}
			else trigger_error (_("objectClass $objectClass required but no module found."), E_USER_WARNING);
			}
		return 0;
		}

	/* This function will load an account.
	* $dn is the dn of the account which should be loaded
	*/
	function load_account($dn) {
		$search = substr($dn, 0, strpos($dn, ','));
		$result = ldap_search($_SESSION[$this->ldap]->server(), $dn, $search);
		$entry = ldap_first_entry($_SESSION[$this->ldap]->server(), $result);
		$this->dn = substr($dn, strpos($dn, ',')+1);
		$this->dn_orig = $dn;
		$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		if (isset($attr['objectClass']['count'])) unset($attr['objectClass']['count']);
		// Add objects
		foreach ($attr['objectClass'] as $objectClass) $this->add_objectClass($objectClass);
		// load attributes
		foreach ($attr['objectClass'] as $objectClass) if (isset($this->module[$objectClass])) $this->module[$objectClass]->load_attributes($attr);
		// sortm modules and make all active because all required attributes should be set
		$module = array_keys ($this->module);
		$modulelist = array();
		// *** fixme add modules from config which should be used but not yet in loaded account

		// loop until all modules are in order.
		// We don't want to loop forever
		$remain = count($module) * count($module);
		$order = array();
		while ( (count($module) != count($modulelist)) && ($remain!=0) ) {
			$remain--;
			foreach ($module as $moduleitem) {
				$required = $this->module[$moduleitem]->dependencies();
				$everything_found = true;
				if (is_array($required)) {
					foreach ($required as $requireditem)
						if (!in_array($reuquireditem, $modulelist)) $everthing_found = false;
					}
				if ($everything_found && !in_array($moduleitem, $order) ) $order[] = $moduleitem;
				}
			}
		// Write Module-Order in variable
		$this->order = $order;
		return 0;
		}

	/* This function will prepare the object
	* for a new account
	*/
	function new_account() {
		$modulelist = array('posixAccount', 'shadowAccount');
		// *** fixme add modules from config which should be used but not yet in loaded account
		foreach ($modulelist as $objectClass) $this->add_objectClass($objectClass);

		$module = array_keys ($this->module);
		// loop until all modules are in order.
		// We don't want to loop forever
		$remain = count($module) * count($module);
		$order = array();
		while ( (count($module) != count($modulelist)) && ($remain!=0) ) {
			$remain--;
			foreach ($module as $moduleitem) {
				$required = $this->module[$moduleitem]->dependencies();
				$everything_found = true;
				if (is_array($required)) {
					foreach ($required as $requireditem)
						if (!in_array($reuquireditem, $modulelist)) $everthing_found = false;
					}
				if ($everything_found && !in_array($moduleitem, $order) ) $order[] = $moduleitem;
				}
			}
		// Write Module-Order in variable
		$this->order = $order;
		// *** fixme load*Profile must return array in the same way ldap_get_attributes does.
		$function = '$newattributes = load'.ucfirst($this->type).'Profile(\'default\');';
		eval($function);
		// pass newattributes to each module
		//foreach ($module as $module2)
		//	$this->module[$module2]->load_attributes($newattributes);
		return 0;
		}

	/* This function will load an account.
	*/
	function save_account() {
		$module = array_keys ($this->module);
		$attributes = array();
		// load attributes
		foreach ($module as $singlemodule) {
			// load changes
			$temp = $this->module[$singlemodule]->save_attributes();
			// merge changes
			$DNs = array_keys($temp);
			$attributes = array_merge_recursive($temp, $attributes);
			for ($i=0; $i<count($DNs); $i++) {
				$ops = array_keys($temp[$DNs[$i]]);
				for ($j=0; $j<count($ops); $j++) {
					$attrs = array_keys($temp[$DNs[$i]][$ops[$j]]);
					for ($k=0; $k<count($attrs); $k++)
						$attributes[$DNs[$i]][$ops[$j]][$attrs[$k]] = array_unique($attributes[$DNs[$i]][$ops[$j]][$attrs[$k]]);
					}
				}
			}
		// Complete dn with uid or cn=
		if ($this->type=='group') $search = 'cn';
			else $search = 'uid';
		$added = false;
		foreach ($attributes as $DN) {
			if (isset($DN['modify'][$search][0]) && !$added) {
				$attributes[$search.'='.$DN['modify'][$search][0].','.$this->dn] = $attributes[$this->dn];
				unset ($attributes[$this->dn]);
				$this->dn = $search.'='.$DN['modify'][$search][0].','.$this->dn;
				$added = true;
				}
			if (isset($DN['add'][$search][0]) && !$added) {
				$attributes[$search.'='.$DN['add'][$search][0].','.$this->dn] = $attributes[$this->dn];
				unset ($attributes[$this->dn]);
				$this->dn = $search.'='.$DN['add'][$search][0].','.$this->dn;
				$added = true;
				}
			if (isset($DN['notchanged'][$search][0]) && !$added) {
				$attributes[$search.'='.$DN['notchanged'][$search][0].','.$this->dn] = $attributes[$this->dn];
				unset ($attributes[$this->dn]);
				$this->dn = $search.'='.$DN['notchanged'][$search][0].','.$this->dn;
				$added = true;
				}
			}
		// Add old dn if dn hasn't changed
		if (!$added) {
			$attributes[$this->dn_orig] = $attributes[$this->dn];
			unset ($attributes[$this->dn]);
			$this->dn = $this->dn_orig;
			}
		// Add new DN
		if ($this->dn != $this->dn_orig) {
			// move existing DN
			if ($this->dn_orig!='') {
				// merge attributes together
				$attr = array_merge_recursive($attributes[$this->dn]['add'], $attributes[$this->dn]['notchanged'], $attributes[$this->dn]['modify']);
				$success = @ldap_add($_SESSION[$this->ldap]->server(), $this->dn, $attr);
				if ($success) {
					$success = @ldap_delete($_SESSION[$this->ldap]->server(), $this->dn_orig);
					if (!$success) return array('ERROR', 'LDAP', sprintf(_('Was unable to delete dn: %s.'), $this->dn_orig));
					}
				if (!$success) return array('ERROR', 'LDAP', sprintf(_('Was unable to create dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $this->dn));
				}
			// create complete new dn
			else {
				$attr = array_merge_recursive($attributes[$this->dn]['add'], $attributes[$this->dn]['notchanged'], $attributes[$this->dn]['modify']);
				$success = @ldap_add($_SESSION[$this->ldap]->server(), $this->dn, $attributes[$this->dn]['add']);
				if (!$success) return array('ERROR', 'LDAP', sprintf(_('Was unable to create dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $this->dn));
				}
			unset($attributes[$this->dn]);
			}
		$DNs = array_keys($attributes);
		for ($i=0; $i<count($DNs); $i++) {
			// modify attributes
			if (isset($attributes[$DNs[$i]]['modify'])) {
				$success = @ldap_mod_replace($_SESSION[$this->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['modify']);
				if (!$success) return array('ERROR', 'LDAP', sprintf(_('Was unable to modify attribtues from dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
				}
			// add attributes
			if (isset($attributes[$DNs[$i]]['add'])) {
				$success = @ldap_mod_add($_SESSION[$this->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['add']);
				if (!$success) return array('ERROR', 'LDAP', sprintf(_('Was unable to add attribtues to dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
				}
			// removce attributes
			if (isset($attributes[$DNs[$i]]['remove'])) {
				$success = @ldap_mod_del($_SESSION[$this->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['remove']);
				if (!$success) return array('ERROR', 'LDAP', sprintf(_('Was unable to remove attribtues from dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
				}
			}
		// *** fixme Add lamdaemon.pl

		return 0;
		}

	}



// This class keeps all needed values for any account
class account {
	// Type : user | group | host
	var $type;
	// General Settings
	var $general_objectClass;	// Array, contains old objectclasses of loaded account
	var $general_username;		// string Username, Hostname or Groupname
	var $general_uidNumber;		// string UIDNumber(user|host) GIDNumber(group) only natural numbers allowed
	var $general_surname;		// string Surname (user)
	var $general_givenname;		// string Givenname (user)
	var $general_dn;		// string DN
	var $general_group;		// string Primary group (user|host)
	var $general_groupadd;		// array(string) Addititional Groups (user) is member of
	var $general_homedir;		// atring Homedirectoy (user) For host it's hardcoded to/dev/null
	var $general_shell;		// array(string) list off all valid shells (user) hosts are hard-wired to /bin/false
	var $general_gecos;		// string, gecos-field (user|group|host)
	// Unix Password Settings
	var $unix_memberUid;		// array Stores all users which are member of group but is not primary group (group)
	var $unix_password;		// string for unix-password (user|host)
	var $unix_password_no;		// string (0|1) set unix-password to none (user|host)
	var $unix_pwdwarn;		// string number of days a user is warned before password expires (user|host) value must be a natural number (user|host)
	var $unix_pwdallowlogin;	// string number of days a user can login even his password has expired (user) muste be a natural number or 0 or -1 (user|host)
	var $unix_pwdmaxage;		// string Number of days after a user has to change his password again Value must be 0<. (user|host)
	var $unix_pwdminage;		// string Number of days a user has to wait until he\'s allowed to change his password again. Value must be 0<. (user|host)
	var $unix_pwdexpire;	// string days since 1.1.1970 the account expires (user|host)
	var $unix_deactivated;		// string (1|0) account deactivated? (user|host)
	var $unix_shadowLastChange;	// string, contains the days since 1.1.1970 the password has been changed last time (user|host)
	var $unix_host;			// list of unix hosts the user is allowed to log in
	// Samba Account
	var $smb_password;		// string for samba-password (user|host)
	var $smb_useunixpwd;		// string (1|0) use unix-password as samba-password (user|host)
	var $smb_pwdcanchange;		// string unix-timestamp user/host is able to change password (user|host)
	var $smb_pwdmustchange;		// string unix-timestamp user/host has to change password at next login (user|host)
	var $smb_homedrive;		// string Homedrive (C:, D:, ...) (user)
	var $smb_scriptPath;		// string ScriptPath (\\server\loginscript) (user)
	var $smb_profilePath;		// string profilePAth (\\server\profilepath) (user)
	var $smb_smbuserworkstations;	// string comma-separated list of workstations (user)
	var $smb_smbhome;		// string Home-Share (\\server\home) (user)
	var $smb_domain;		// string Domain of (user|host) or samba3domain-Object
	var $smb_flags;			// array of acctFlags, ( {'W'] => 0, ['X'] => 1, ......
	var $smb_mapgroup;		// decimal ID for groups
	var $smb_displayName;		// string, description, similar to gecos-field.
	// Quota Settins
	var $quota;			/* array[][] First array is an index for every chare with active quotas
					* second array Contains values for every share:
					* mountpoint, used blocks, soft block limit, hard block limit, grace block period, used inodes,
					* soft inode limit, hard inode limit, grace inode period
					*/
	// Personal Settings
	var $personal_title;		// string title of user
	var $personal_mail;		// string mailaddress of user
	var $personal_telephoneNumber;	// string telephonenumber of user
	var $personal_mobileTelephoneNumber; // string mobile umber of user
	var $personal_facsimileTelephoneNumber; // strinf fax-number of user
	var $personal_street;		// stirng streetname of user
	var $personal_postalCode;	// string postal code of user
	var $personal_postalAddress;	// string postal Address of user
	var $personal_employeeType;	// string employe type of user
	}


/* Return a list of all shells listed in ../config/shells
* Normally ../config/shells is a symbolic link to /etc/shells
*/
function getshells() {
	// Load shells from file
	$shells =  file($_SESSION['lampath'].'config/shells');
	$i=0;
	while (count($shells) > $i) {
		// remove whitespaces
		trim($shells[$i]);
		// remove lineend
		$shells[$i] = substr($shells[$i], 0, strpos($shells[$i], "\n"));
		// remove comments
		if ($shells[$i]{0}=='#') unset ($shells[$i]);
		 else $i++;
		}
	// $shells is array with all valid shells
	return $shells;
	}


/* This function will replace umlates with ascci-chars
* fixme ***
* In order to map all non-ascii characters this function should be changed
*/
function replace_umlaut($text) {
	$aTranslate = array("ä"=>"ae", "Ä"=>"Ae",
		"ö"=>"oe", "Ö"=>"Oe",
		"ü"=>"ue", "Ü"=>"Ue",
		"ß"=>"ss"
		);
	return strtr($text, $aTranslate);
	}


/* This function will return all values from $array without values of $values
* $values, $array and $return are arrays
*/
function array_delete($values, $array) {
	// Loop for every entry and check if it should be removed
	if (is_array($array)) {
		foreach ($array as $array_value)
			if (!@in_array($array_value, $values))
				$return[] = $array_value;
		return $return;
		}
	else return 0;
	}


// This function will return a password with max. 8 characters
function genpasswd() {
	// Allowed Characters to generate passwords
	// I'Ve removed characters like l and 1 because they are too similar
	$LCase = 'abcdefghjkmnpqrstuvwxyz';
	$UCase = 'ABCDEFGHJKMNPQRSTUVWXYZ';
	$Integer = '23456789';
	// DEFINE CONSTANTS FOR ALGORTTHM
	define("LEN", '1');
	$a = RndInt('letter');
	$b = RndInt('letter');
	$c = RndInt('letter');
	$d = RndInt('letter');
	$e = RndInt('number');
	$f = RndInt('number');
	$g = RndInt('letter');
	$h = RndInt('letter');
	// EXTRACT 8 CHARACTERS RANDOMLY FROM TH // E DEFINITION STRINGS
	$L1 = substr($LCase, $a, LEN);
	$L2 = substr($LCase, $b, LEN);
	$L3 = substr($LCase, $h, LEN);
	$U1 = substr($UCase, $c, LEN);
	$U2 = substr($UCase, $d, LEN);
	$U3 = substr($UCase, $g, LEN);
	$I1 = substr($Integer, $e, LEN);
	$I2 = substr($Integer, $f, LEN);
	// COMBINE THE CHARACTERS AND DISPLAY TH // E NEW PASSWORD
	$PW = $L1 . $U2 . $I1 . $L2 . $I2 . $U1 . $U3 . $L3;
	return $PW;
	}


/* THIS FUNCTION GENERATES A RANDOM NUMBER THAT WILL BE USED TO
* RANDOMLY SELECT CHARACTERS FROM THE STRINGS ABOVE
*/
function RndInt($Format){
	switch ($Format){
		case 'letter':
			$Rnd = rand(0,23);
			if ($Rnd > 23){
				$Rnd = $Rnd - 1;
				}
			break;
		case 'number':
			$Rnd = rand(2,9);
			if ($Rnd > 8){
				$Rnd = $Rnd - 1;
				}
			break;
		}
	return $Rnd;
	} // END RndInt() FUNCTION


/* Whis function will return the quotas from the specified user If empty only filesystems with enabled quotas are returned
* $users = array of account objects., return-value is an array of account objects
* if $users is account object return values is also an account object
*        An array with all quota-enabled partitions will be returned in this case all returned values are 0 exept mointpoint[x][0]
*/
function getquotas($users) {
	// define new object
	if (is_array($users)) $return = $users;
		else $return[0] = $users;
	// get username and password of the current lam-admin
	$ldap_q = $_SESSION['ldap']->decrypt();
	/* $towrite has the following syntax:
	* admin-username, admin-password, account with quotas, 'quota', operation='get', type=user|group
	* use escapeshellarg to make exec() shell-safe
	*/
	$towrite = escapeshellarg($_SESSION['config']->scriptServer)." ".escapeshellarg($_SESSION['config']->scriptPath)." ".
		escapeshellarg($ldap_q[0]).' '.escapeshellarg($ldap_q[1]);
	/* scriptServer is the IP to remote-host to which lam should connect via ssh
	* scriptPath is Path to lamdaemon.pl on remote system
	*/
	if (is_array($return)) {
		for($i=0; $i<count($return); $i++)
			// put string to trasmit together
			if ($return[$i]->general_username!='') $userstring .= $return[$i]->general_username." quota get ".$return[$i]->type."\n";
				else $userstring .= "+ quota get ".$return[$i]->type."\n";
		}
	if (function_exists(proc_open)) {
		// New Code, requires PHP 4.3
		$descriptorspec = array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stout
			2 => array("file", "/dev/null", "a") // sterr
			);
		$process = proc_open(escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite,
			$descriptorspec,
			$pipes);
		if (is_resource($process)) {
			/* perl-script is running
			* $pipes[0] is writeable handle to child stdin
			* $pipes[1] is readable handle to child stdout
			* any error is send to /dev/null
			*/
			// Write one output-line for every user
			fwrite($pipes[0], $userstring);
			fclose($pipes[0]);
			while (!feof($pipes[1])) {
				$output = fgets($pipes[1], 1024);
				if ($output!='') $output_array[] = $output;
				}
			fclose($pipes[1]);
			proc_close($process);
			}
		}
	else { // PHP 4.3>
		$command = escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite;
		$pipe = popen("echo \"$userstring\"|$command" , 'r');
		while(!feof($pipe)) {
			//$output .= fread($pipe, 1024);
			$output = fgets($pipe, 1024);
			if ($output!='') $output_array[] = $output;
			}
		pclose($pipe);
		}
	/* $vals is a string which contains a two dimensional array.
	* We have to recreate it with explode
	*
	* $return->quota[][] First array is an index for every chare with active quotas
	* second array Contains values for every share:
	* mountpoint, used blocks, soft block limit, hard block limit, grace block period, used inodes,
	* soft inode limit, hard inode limit, grace inode period
	*/
	if (is_array($output_array)) {
		for ($i=0; $i<count($return); $i++) {
			$all_quota = explode(':', $output_array[$i]);
			for ($j=0; $j<sizeof($all_quota)-1; $j++) {
				$single_quota = explode(',', $all_quota[$j]);
				// unset not existing quotas from account-object
				// collect all existing mountpoints in array
				$real_quotas[] = $single_quota[0];
				for ($k=0; $k<sizeof($single_quota); $k++)
					$return[$i]->quota[$j][$k] = $single_quota[$k];
				if ($return[$i]->quota[$j][4]<time()) $return[$i]->quota[$j][4] = '';
					else $return[$i]->quota[$j][4] = strval(intval(($return[$i]->quota[$j][4]-time())/3600)) .' '. _('hours');
				if ($return[$i]->quota[$j][8]<time()) $return[$i]->quota[$j][8] = '';
					else $return[$i]->quota[$j][8] = strval(intval(($return[$i]->quota[$j][8]-time())/3600)) .' '. _('hours');
				}
			$j=0;
			while (isset($return[$i]->quota[$j][0]))
				// remove invalid quotas
				if (!in_array($return[$i]->quota[$j][0], $real_quotas)) unset($return[$i]->quota[$j]);
					else $j++;
			// Beautify array, repair index
			if (is_array($return[$i]->quota)) $return[$i]->quota = array_values($return[$i]->quota);
			}
		if (is_array($users)) return $return;
			else return $return[0];
		}
	else {
		return $users;
		}
	}

/* Whis function will set the quotas from the specified user.
* $values2 = array of object account with quotas which should be set
*  $values2 can also be an account object
*/
function setquotas($values2) {
	// get username and password of the current lam-admin
	$ldap_q = $_SESSION['ldap']->decrypt();
	/* $towrite has the following syntax:
	* admin-username, admin-password, account with quotas, 'quota', operation='set', type=user|group
	* use escapeshellarg to make exec() shell-safe
	*/
	$towrite = escapeshellarg($_SESSION['config']->scriptServer)." ".escapeshellarg($_SESSION['config']->scriptPath)." ".
		escapeshellarg($ldap_q[0]).' '.escapeshellarg($ldap_q[1]);

	/* Check wich quotas have changed
	* Because we can not send an array to lamdaemon.pl we have to put all
	* values in a string. ':' sepraeates the first array, ',' the second
	*
	* $values->quota[][] First array is an index for every chare with active quotas
	* second array Contains values for every share:
	* mountpoint, used blocks, soft block limit, hard block limit, grace block period, used inodes,
	* soft inode limit, hard inode limit, grace inode period
	*
	* run only once if no array is given
	*
	*/
	if (is_array($values2)) {
		foreach ($values2 as $values) {
			$i=0;
			while ($values->quota[$i][0]) {
				$quotastring = $quotastring. $values->quota[$i][0] .','.$values->quota[$i][2] .','.$values->quota[$i][3]
					.','.$values->quota[$i][6] .','. $values->quota[$i][7] .':';
				$i++;
				}
			$userstring .= $values->general_username." quota set ".$values->type." ".$quotastring."\n";
			}
		}
	else {
		$i=0;
		while ($values2->quota[$i][0]) {
			$quotastring = $quotastring. $values2->quota[$i][0] .','.$values2->quota[$i][2] .','.$values2->quota[$i][3]
				.','.$values2->quota[$i][6] .','. $values2->quota[$i][7] .':';
			$i++;
			}
		$userstring = $values2->general_username." quota set ".$values2->type." ".$quotastring."\n";
		}

	if (function_exists(proc_open)) {
		// New Code, requires PHP 4.3
		$descriptorspec = array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stout
			2 => array("file", "/dev/null", "a") // sterr
			);
		$process = proc_open(escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite,
			$descriptorspec,
			$pipes);
		if (is_resource($process)) {
			/* perl-script is running
			* $pipes[0] is writeable handle to child stdin
			* $pipes[1] is readable handle to child stdout
			* any error is send to /dev/null
			*/
			// Write to stdin
			fwrite($pipes[0], $userstring);
			}
		fclose($pipes[0]);
		while (!feof($pipes[1])) {
			$output = fgets($pipes[1], 1024);
			if ($output!='') $output_array[] = $output;
			}
		fclose($pipes[1]);
		proc_close($process);
		}
	else { // PHP 4.3>
		$command = escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite;
		$pipe = popen("echo \"$userstring\"|$command" , 'r');
		while(!feof($pipe)) {
			//$output .= fread($pipe, 1024);
			$output = fgets($pipe, 1024);
			if ($output!='') $output_array[] = $output;
			}
		pclose($pipe);
		}
	if (is_array($values2)) return $output_array;
		else return $output_array[0];
	}


/* Whis function will remove the quotas from the specified user.
* $users = array of usernames of which quta should be deleted
* $users can also be a string (single user)
* $type = user or group
* Delteing quotas means settings all values to 0 which means no quotas
*/
function remquotas($users, $type) {
	// get username and password of the current lam-admin
	$ldap_q = $_SESSION['ldap']->decrypt();
	/* $towrite has the following syntax:
	* admin-username, admin-password, account with quotas, 'quota', operation='rem', type=user|group
	* use escapeshellarg to make exec() shell-safe
	*/
	$towrite = escapeshellarg($_SESSION['config']->scriptServer)." ".escapeshellarg($_SESSION['config']->scriptPath)." ".
		escapeshellarg($ldap_q[0]).' '.escapeshellarg($ldap_q[1]);

	if (is_array($users)) {
		foreach ($users as $user) {
			$userstring .= "$user quota rem $type\n";
			}
		}
	else $userstring = "$users quota rem $type\n";

	if (function_exists(proc_open)) {
		// New Code, requires PHP 4.3
		$descriptorspec = array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stout
			2 => array("file", "/dev/null", "a") // sterr
			);
		$process = proc_open(escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite,
			$descriptorspec,
			$pipes);
		if (is_resource($process)) {
			/* perl-script is running
			* $pipes[0] is writeable handle to child stdin
			* $pipes[1] is readable handle to child stdout
			* any error is send to /dev/null
			*/
			// Write to stdin
			fwrite($pipes[0], $userstring);
			}
		fclose($pipes[0]);
		while (!feof($pipes[1])) {
			$output = fgets($pipes[1], 1024);
			if ($output!='') $output_array[] = $output;
			}
		fclose($pipes[1]);
		proc_close($process);
		}
	else { // PHP 4.3>
		$command = escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite;
		$pipe = popen("echo \"$userstring\"|$command" , 'r');
		while(!feof($pipe)) {
			//$output .= fread($pipe, 1024);
			$output = fgets($pipe, 1024);
			if ($output!='') $output_array[] = $output;
			}
		pclose($pipe);
		}
	if (is_array($values2)) return $output_array;
		else return $output_array[0];
	}


/* Create Homedirectory
* lamdaemon.pl uses getpwnam on remote system to get homedir path.
* Therefore ldap have to be used on remote system for user accounts
* $users = array of usernames
* $users can also be a string (single user)
*/
function addhomedir($users) {
	// get username and password of the current lam-admin
	$ldap_q = $_SESSION['ldap']->decrypt();
	/* $towrite has the following syntax:
	* admin-username, admin-password, owner of homedir, 'home', operation='add'
	* use escapeshellarg to make exec() shell-safe
	*/
	$towrite = escapeshellarg($_SESSION['config']->scriptServer)." ".escapeshellarg($_SESSION['config']->scriptPath)." ".
		escapeshellarg($ldap_q[0]).' '.escapeshellarg($ldap_q[1]);

	if (is_array($users)) {
		foreach ($users as $user) {
			$userstring .= "$user home add\n";
			}
		}
	else $userstring = "$users home add\n";

	if (function_exists(proc_open)) {
		// New Code, requires PHP 4.3
		$descriptorspec = array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stout
			2 => array("file", "/dev/null", "a") // sterr
			);
		$process = proc_open(escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite,
			$descriptorspec,
			$pipes);
		if (is_resource($process)) {
			/* perl-script is running
			* $pipes[0] is writeable handle to child stdin
			* $pipes[1] is readable handle to child stdout
			* any error is send to /dev/null
			*/
			// Write to stdin
			fwrite($pipes[0], $userstring);
			}
		fclose($pipes[0]);
		while (!feof($pipes[1])) {
			$output = fgets($pipes[1], 1024);
			if ($output!='') $output_array[] = $output;
			}
		fclose($pipes[1]);
		proc_close($process);
		}
	else { // PHP 4.3>
		$command = escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite;
		$pipe = popen("echo \"$userstring\"|$command" , 'r');
		while(!feof($pipe)) {
			//$output .= fread($pipe, 1024);
			$output = fgets($pipe, 1024);
			if ($output!='') $output_array[] = $output;
			}
		pclose($pipe);
		}
	if (is_array($values2)) return $output_array;
		else return $output_array[0];
	}

/* Remove Homedirectory
* lamdaemon.pl uses getpwnam on remote system to get homedir path.
* Therefore ldap have to be used on remote system for user accounts
* This also means you have to remove the homedirectory before the
* account is removed from ldap
* $users = array of usernames
* $users can also be a string (single user)
*/
function remhomedir($users) {
	// get username and password of the current lam-admin
	$ldap_q = $_SESSION['ldap']->decrypt();
	/* $towrite has the following syntax:
	* admin-username, admin-password, owner of homedir, 'home', operation='add'
	* use escapeshellarg to make exec() shell-safe
	*/
	$towrite = escapeshellarg($_SESSION['config']->scriptServer)." ".escapeshellarg($_SESSION['config']->scriptPath)." ".
		escapeshellarg($ldap_q[0]).' '.escapeshellarg($ldap_q[1]);

	if (is_array($users)) {
		foreach ($users as $user) {
			$userstring .= "$user home rem\n";
			}
		}
	else $userstring = "$users home rem\n";

	if (function_exists(proc_open)) {
		// New Code, requires PHP 4.3
		$descriptorspec = array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stout
			2 => array("file", "/dev/null", "a") // sterr
			);
		$process = proc_open(escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite,
			$descriptorspec,
			$pipes);
		if (is_resource($process)) {
			/* perl-script is running
			* $pipes[0] is writeable handle to child stdin
			* $pipes[1] is readable handle to child stdout
			* any error is send to /dev/null
			*/
			// Write to stdin
			fwrite($pipes[0], $userstring);
			}
		fclose($pipes[0]);
		while (!feof($pipes[1])) {
			$output = fgets($pipes[1], 1024);
			if ($output!='') $output_array[] = $output;
			}
		fclose($pipes[1]);
		proc_close($process);
		}
	else { // PHP 4.3>
		$command = escapeshellarg($_SESSION['lampath']."lib/lamdaemon.pl")." ".$towrite;
		$pipe = popen("echo \"$userstring\"|$command" , 'r');
		while(!feof($pipe)) {
			//$output .= fread($pipe, 1024);
			$output = fgets($pipe, 1024);
			if ($output!='') $output_array[] = $output;
			}
		pclose($pipe);
		}
	if (is_array($values2)) return $output_array;
		else return $output_array[0];
	}


/* This function maintains the ldap-cache which is used to reduce ldap requests
* If the array is older than $_SESSION['config']->get_cacheTimeoutSec() it will
* be recreated
*
* $type can be user|group|host.
*
* $_SESSION['xxxxDN'] contains all attributes which are needed very often from
* more than one function
* $_SESSION['xxxx'DN'][0] contains the creation time of the array as unix timestamp.
* All other entries have the following syntax:
* $_SESSION['xxxx'DN'][$dn][$attributes]
*    $dn         = DN of cached entry
*    $attributes = All cached attributes of DN
* The cache contains the following attributes:
* user: cn, uidNumber
* group: cn, gidNumber
* host: cn, uidNumber
*/
function ldapreload($type) {
	switch ($type) {
		case 'user':
			// Do we have to recreate array?
			if ((!isset($_SESSION['userDN'])) || ($_SESSION['userDN'][0] < time()-$_SESSION['config']->get_cacheTimeoutSec())) {
				// Remove old array
				if (isset($_SESSION['userDN'])) unset($_SESSION['userDN']);
				// insert timestamp in array
				$_SESSION['userDN'][0] = time();
				// Search 4 values which should be cached
				$result = @ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(),
					'(&(objectClass=posixAccount)(!(uid=*$)))', array('cn', 'uidNumber'), 0);
				// Write search result in array
				$entry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if (isset($attr['cn'][0]))
						$_SESSION['userDN'][$dn]['cn'] = $attr['cn'][0];
					if (isset($attr['uidNumber'][0]))
						$_SESSION['userDN'][$dn]['uidNumber'] = $attr['uidNumber'][0];
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				}
			break;
		case 'group':
			// Do we have to recreate array?
			if ((!isset($_SESSION['groupDN'])) || ($_SESSION['groupDN'][0] < time()-$_SESSION['config']->get_cacheTimeoutSec())) {
				// Remove old array
				if (isset($_SESSION['groupDN'])) unset($_SESSION['groupDN']);
				// insert timestamp in array
				$_SESSION['groupDN'][0] = time();
				// Search 4 values which should be cached
				$result = @ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(),
					'objectClass=posixGroup', array('gidNumber', 'cn', 'memberUid', 'sambaSID'), 0);
				// Write search result in array
				$entry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if (isset($attr['gidNumber'][0]))
						$_SESSION['groupDN'][$dn]['uidNumber'] = $attr['gidNumber'][0];
					if (isset($attr['cn'][0]))
						$_SESSION['groupDN'][$dn]['cn'] = $attr['cn'][0];
					if (isset($attr['sambaSID'][0]))
						$_SESSION['groupDN'][$dn]['sambaSID'] = $attr['sambaSID'][0];
					$i=0;
					while (isset($attr['memberUid'][$i])) {
						$_SESSION['groupDN'][$dn]['memberUid'][$i] = $attr['memberUid'][$i];
						$i++;
						}
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				}
			break;
		case 'host':
			// Do we have to recreate array?
			if ((!isset($_SESSION['hostDN'])) || ($_SESSION['hostDN'][0] < time()-$_SESSION['config']->get_cacheTimeoutSec())) {
				// Remove old array
				if (isset($_SESSION['hostDN'])) unset($_SESSION['hostDN']);
				// insert timestamp in array
				$_SESSION['hostDN'][0] = time();
				// Search 4 values which should be cached
				$result = @ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_HostSuffix(),
					'(&(objectClass=posixAccount)(uid=*$))', array('cn', 'uidNumber'), 0);
				// Write search result in array
				$entry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
				while ($entry) {
					$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
					$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
					if (isset($attr['cn'][0]))
						$_SESSION['hostDN'][$dn]['cn'] = $attr['cn'][0];
					if (isset($attr['uidNumber'][0]))
						$_SESSION['hostDN'][$dn]['uidNumber'] = $attr['uidNumber'][0];
					$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
					}
				}
			break;
		}
	return 0;
	}


/* This function will search in cache if the DN already exists
* $values is an account-object
* $values_old is an account-object
* $values_old is needed because we don't want to raise
*             an error if the DN allredy exists but is the original DN
*/
function ldapexists($values, $values_old=false) {
	switch ($values->type) {
		case 'user':
			// Validate cache-array
			ldapreload('user');
			// Entry which we search in ldap ',' is needed to ensure the complete username is checked
			$search = 'uid='.$values->general_username.',';
			// Get copy of cache-array
			$keys = array_keys($_SESSION['userDN']);
			break;
		case 'group':
			// Validate cache-array
			ldapreload('group');
			// Entry which we search in ldap ',' is needed to ensure the complete username is checked
			$search = 'cn='.$values->general_username.',';
			// Get copy of cache-array and
			$keys = array_keys($_SESSION['groupDN']);
			break;
		case 'host':
			// Validate cache-array
			ldapreload('host');
			// Entry which we search in ldap ',' is needed to ensure the complete username is checked
			$search = 'uid='.$values->general_username.',';
			// Get copy of cache-array
			$keys = array_keys($_SESSION['hostDN']);
			break;
		}
	// Remove timestamp stored in [0]
	unset ($keys[0]);
	$keys = array_values($keys);
	if (!$values_old) {
		// Create new account
		// Check if entry allready exists
		foreach ($keys as $key)
			if (strstr($key, $search)) return sprintf (_('%s already exists!'), $values->type);
		}
	// edit existing account and don't create a new one
	else if ($values_old->general_username != $values->general_username) {
		foreach ($keys as $key)
			if (strstr($key, $search)) return sprintf (_('%s already exists!'), $values->type);
		}
	return 0;
	}


/* This function will return an array with all groupnames
* found in ldap. Groupnames are taken from cache-array.
*/
function findgroups() {
	// Validate cache-array
	ldapreload('group');
	// Get copy of cache-array
	$groups = $_SESSION['groupDN'];
	// Remove timestamp stored in [0]
	unset ($groups[0]);
	// Create and sort array
	foreach ($groups as $group) {
		$return[] = $group['cn'];
		}
	if (is_array($return)) sort ($return, SORT_STRING);
	return $return;
	}


/* This function will return the gidNumber to an existing groupname
* gidNumbers are taken from cache-array
*/
function getgid($groupname) {
	// Validate cache-array
	ldapreload('group');
	// Get copy of cache-array
	$keys = $_SESSION['groupDN'];
	// Remove timestamp stored in [0]
	unset ($keys[0]);
	// Return gidNumber as soon as it's found
	foreach ($keys as $key) {
		if ($key['cn']==$groupname)  return $key['uidNumber'];
		}
	// return -1 if groupname isn't found
	return -1;
	}

/* This function will return the groupname to an existing gidNumber
* groupnames are taken from cache-array
*/
function getgrnam($gidNumber) {
	// Validate cache-array
	ldapreload('group');
	// Get copy of cache-array
	$groupDN_local = $_SESSION['groupDN'];
	// Remove timestamp stored in [0]
	unset ($groupDN_local[0]);
	// Now we only have an array with DNs
	$groupDN_local = array_keys($groupDN_local);
	$i=0;
	// Loop until we've found the right uidNumber
	while (!isset($return) && isset($_SESSION['groupDN'][$groupDN_local[$i]]['uidNumber'])) {
		if ($_SESSION['groupDN'][$groupDN_local[$i]]['uidNumber'] == $gidNumber) {
			// We've found the correct entry. Now we need the groupname
			$return = $_SESSION['groupDN'][$groupDN_local[$i]]['cn'];
			}
		// Increase loop-variable if entry wasn't found
		else $i++;
		}
	// Set $return to -1 if no group was found
	if (!isset($return)) $return = -1;
	return $return;
	}


/* This function will return an unuesed id-number if $values->general_uidNumber is not set and $values_old is false
* If values_old is true and $values->general_uidNumber is not set the original id-number is returned
* If $values->general_uidNumber is set id-number is checked. If it's allready in use an error will be reported
* id-numbers are taken from cache-array
* $values and $values_old are account objects
* Return-Values is an integer id-number or an string-error
*/
function checkid($values, $values_old=false) {
	switch ($values->type) {
		case 'group':
			// Validate cache-array
			ldapreload('group');
			// Load all needed variables from session
			$minID = intval($_SESSION['config']->get_MinGID());
			$maxID = intval($_SESSION['config']->get_MaxGID());
			// Get copy of cache-array
			$temp = $_SESSION['groupDN'];
			break;
		case 'user':
			/* Validate cache-array
			* Because users and hosts are using the same id-numbers we have to merge
			* both cache-arrays
			*/
			ldapreload('user');
			ldapreload('host');
			// Load all needed variables from session
			$minID = intval($_SESSION['config']->get_minUID());
			$maxID = intval($_SESSION['config']->get_maxUID());
			// load and merge arrays

			$temp = $_SESSION['userDN'];
			// Remove timestamp stored in [0]
			unset ($temp[0]);
			// put only uidNumbers in array
			 foreach ($temp as $key) $uids[] = $key['uidNumber'];
			$temp = $_SESSION['hostDN'];
			break;
		case 'host':
			/* Validate cache-array
			* Because users and hosts are using the same id-numbers we have to merge
			* both cache-arrays
			*/
			ldapreload('user');
			ldapreload('host');
			// Load all needed variables from session
			$minID = intval($_SESSION['config']->get_minMachine());
			$maxID = intval($_SESSION['config']->get_maxMachine());
			// load and merge arrays

			$temp = $_SESSION['userDN'];
			// Remove timestamp stored in [0]
			unset ($temp[0]);
			// put only uidNumbers in array
			foreach ($temp as $key) $uids[] = $key['uidNumber'];
			$temp = $_SESSION['hostDN'];
			break;
		}
	// Remove timestamp stored in [0]
	unset ($temp[0]);
	// put only uidNumbers in array. Put only uids in array witch are smaller than maxID
	foreach ($temp as $key) if ($key['uidNumber'] < $maxID) $uids[] = $key['uidNumber'];
	// sort array with uids
	if(is_array($uids)) sort ($uids, SORT_NUMERIC);
	if ($values->general_uidNumber=='') {
		// No id-number given
		if (!isset($values_old->general_uidNumber)) {
			// new account -> we have to find a free id-number
			if (count($uids)!=0) {
				// There are some uids
				// Store highest id-number
				$id = $uids[count($uids)-1];
				// Return minimum allowed id-number if all found id-numbers are too low
				if ($id < $minID) return implode(':', array($minID, ''));
				// Return higesht used id-number + 1 if it's still in valid range
				if ($id < $maxID) return implode(':', array( $id+1, ''));
				/* If this function is still running we have to fid a free id-number between
				* the used id-numbers
				*/
				$i = intval($minID);
				while (in_array($i, $uids)) $i++;
				if ($i>$maxID) return implode(':', array($values->general_uidNumber , implode(';', array('ERROR', _('ID-Number'), _('No free ID-Number!')))));
					else return implode(':', array($i, implode(';', array('WARN', _('ID-Number'), _('It is possible that this ID-number is reused. This can cause several problems because files with old permissions might still exist. To avoid this warning set maxUID to a higher value.')))));
				}
			else return implode(':', array($minID, ''));
			// return minimum allowed id-number if no id-numbers are found
			}
		else return implode(':', array($values_old->general_uidNumber, ''));
		// old account -> return id-number which has been used
		}
	else {
		// Check manual ID
		// id-number is out of valid range
		if ( $values->general_uidNumber < $minID || $values->general_uidNumber > $maxID) return implode(':', array($values->general_uidNumber, implode(';', array('ERROR', _('ID-Number'), sprintf(_('Please enter a value between %s and %s!'), $minID, $maxID)))));
		// $uids is allways an array but not if no entries were found
		if (is_array($uids)) {
			// id-number is in use and account is a new account
			if ((in_array($values->general_uidNumber, $uids)) && !$values_old) return implode(':', array($values->general_uidNumber, implode(';', array('ERROR', _('ID-Number'), _('ID is already in use')))));
			// id-number is in use, account is existing account and id-number is not used by itself
			if ((in_array($values->general_uidNumber, $uids)) && $values_old && ($values_old->general_uidNumber != $values->general_uidNumber) )
				return implode(':', array($values_old->general_uidNumber, implode(';', array('ERROR', _('ID-Number'), _('ID is already in use')))));
			}
		// return id-number if everything is OK
		return implode(':', array($values->general_uidNumber, ''));
		}
	}


// This function will return the days from 1.1.1970 until now
function getdays() {
	$days = time() / 86400;
	settype($days, 'integer');
	return $days;
	}


/* This function creates all attributes stored in attrFlags. It's the same
* syntax used in smbpasswd
* $values is an array of samba flags as defined in account object
* Return value is a string
*/
function smbflag($input) {
	// Start character
	$flag = "[";
	// Add Options
	if ($input['W']) $flag .= "W"; else $flag .= "U";
	if ($input['D']) $flag .= "D";
	if ($input['X']) $flag .= "X";
	if ($input['N']) $flag .= "N";
	if ($input['S']) $flag .= "S";
	if ($input['H']) $flag .= "H";
	// Expand string to fixed length
	$flag = str_pad($flag, 12);
	// End character
	$flag = $flag. "]";
	return $flag;
	}


/* This function will load all needed values from an existing user account
* $dns is an array of  dns(string) of the users which should be loaded
* return-value is an array of account-objects
* $dns can also be an string. Then return-value is a single string too
*/
function loaduser($dns) {
	// Load userattributes from ldap
	//$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	// Get uid=$user from DN
	// Put ldapsearch together
	if (is_array($dns)) {
		foreach ($dns as $dn)
			$uids[] = substr($dn, 0, strpos($dn, ','));
		$search = "(|";
		foreach ($uids as $uid) $search .= "($uid)";
		$search .= ")";
		}
	else $search = substr($dns, 0, strpos($dns, ','));
	/* if string is langer then 1024 characters we have to search for all users and
	* output only the selcted users because searchfilter would be too big
	*/
	if (strlen($search)<1024) $result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), $search);
		else $result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), "uid=*");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);

	// store smb_domain as samba3domain-Object
	if ($_SESSION['config']->is_samba3()) $samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());

	// loop for every found user
	$i=0;
	while ($entry) {
		// Only load user if it should be loaded
		if (is_array($dns)) {
			if (in_array(ldap_get_dn($_SESSION['ldap']->server(), $entry), $dns)) $continue=true;
				else $continue=false;
			}
		else $continue=true;
		if ($continue) {
			// Create new object
			$return[$i] = new account();
			// Set type of account
			$return[$i]->type='user';
			// Set user samba flag
			$return[$i]->smb_flags['W'] = false;
			$return[$i]->general_dn = ldap_get_dn($_SESSION['ldap']->server(), $entry);
			$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			/* Write attributes into $return.
			* Some values don't have to be set. These are only loaded if they are set
			*/
			$return[$i]->general_username = $attr['uid'][0];
			$return[$i]->general_uidNumber = $attr['uidNumber'][0];
			$return[$i]->general_homedir = $attr['homeDirectory'][0];
			if (isset($attr['shadowLastChange'][0])) $return[$i]->unix_shadowLastChange = $attr['shadowLastChange'][0];
			if (isset($attr['loginShell'][0])) $return[$i]->general_shell = $attr['loginShell'][0];
			if (isset($attr['gecos'][0])) $return[$i]->general_gecos = utf8_decode($attr['gecos'][0]);

			// get groupname
			$return[$i]->general_group = getgrnam($attr['gidNumber'][0]);

			// get all additional groupmemberships
			// Load groups in cache
			ldapreload('group');
			foreach ($_SESSION['groupDN'] as $group) {
				if (is_array($group['memberUid']))
					if (in_array($return[$i]->general_username, $group['memberUid'])) $return[$i]->general_groupadd[] = $group['cn'];
				}

			/* Write attributes into $return.
			* Some values don't have to be set. These are only loaded if they are set
			*/
			if (isset($attr['shadowMin'][0])) $return[$i]->unix_pwdminage = $attr['shadowMin'][0];
			if (isset($attr['shadowMax'][0])) $return[$i]->unix_pwdmaxage = $attr['shadowMax'][0];
			if (isset($attr['shadowWarning'][0])) $return[$i]->unix_pwdwarn = $attr['shadowWarning'][0];
			if (isset($attr['shadowInactive'][0])) $return[$i]->unix_pwdallowlogin = $attr['shadowInactive'][0];
			if (isset($attr['shadowExpire'][0])) $return[$i]->unix_pwdexpire = $attr['shadowExpire'][0]*86400;

			// load hosts attributes if set
			$j=0;
			while (isset($attr['host'][$j])) {
				if ($j==0) $return[$i]->unix_host = $attr['host'][$j];
					else $return[$i]->unix_host = $return[$i]->unix_host . ', ' . $attr['host'][$j];
				$j++;
				}

			// load objectclasses
			$j=0;
			while (isset($attr['objectClass'][$j])) {
				$return[$i]->general_objectClass[$j] = $attr['objectClass'][$j];
				$j++;
				}

			// load personal settings
			if (isset($attr['givenName'][0])) $return[$i]->general_givenname = utf8_decode($attr['givenName'][0]);
			if (isset($attr['sn'][0])) $return[$i]->general_surname = utf8_decode($attr['sn'][0]);
			if (isset($attr['title'][0])) $return[$i]->personal_title = utf8_decode($attr['title'][0]);
			if (isset($attr['mail'][0])) $return[$i]->personal_mail = utf8_decode($attr['mail'][0]);
			if (isset($attr['telephoneNumber'][0])) $return[$i]->personal_telephoneNumber = utf8_decode($attr['telephoneNumber'][0]);
			if (isset($attr['mobileTelephoneNumber'][0])) $return[$i]->personal_mobileTelephoneNumber = utf8_decode($attr['mobileTelephoneNumber'][0]);
				else 	if (isset($attr['mobile'][0])) $return[$i]->personal_mobileTelephoneNumber = utf8_decode($attr['mobile'][0]);
			if (isset($attr['facsimileTelephoneNumber'][0])) $return[$i]->personal_facsimileTelephoneNumber = utf8_decode($attr['facsimileTelephoneNumber'][0]);
			if (isset($attr['street'][0])) $return[$i]->personal_street = utf8_decode($attr['street'][0]);
			if (isset($attr['postalCode'][0])) $return[$i]->personal_postalCode = utf8_decode($attr['postalCode'][0]);
			if (isset($attr['postalAddress'][0])) $return[$i]->personal_postalAddress = utf8_decode($attr['postalAddress'][0]);
			if (isset($attr['employeeType'][0])) $return[$i]->personal_employeeType = utf8_decode($attr['employeeType'][0]);

			if (isset($attr['userPassword'][0])) {
				$return[$i]->unix_password = $attr['userPassword'][0];
				$return[$i]->unix_deactivated=!pwd_is_enabled($attr['userPassword'][0]);
				}
			if (isset($attr['displayName'][0])) $return[$i]->smb_displayName = utf8_decode($attr['displayName'][0]);

			// sambaSamAccount (Samba3) is used.
			if (in_array('sambaSamAccount', $attr['objectClass'])) {
				/* Write attributes into $return.
				* Some values don't have to be set. These are only loaded if they are set
				*/
				if (isset($attr['sambaAcctFlags'][0])) {
					if (strrpos($attr['sambaAcctFlags'][0], 'D')) $return[$i]->smb_flags['D']=true;
					if (strrpos($attr['sambaAcctFlags'][0], 'X')) $return[$i]->smb_flags['X']=true;
					if (strrpos($attr['sambaAcctFlags'][0], 'N')) $return[$i]->smb_flags['N']=true;
					if (strrpos($attr['sambaAcctFlags'][0], 'S')) $return[$i]->smb_flags['S']=true;
					if (strrpos($attr['sambaAcctFlags'][0], 'H')) $return[$i]->smb_flags['H']=true;
					}
				if (isset($attr['sambaPwdCanChange'][0])) $return[$i]->smb_pwdcanchange = $attr['sambaPwdCanChange'][0];
				if (isset($attr['sambaPwdMustChange'][0])) $return[$i]->smb_pwdmustchange = $attr['sambaPwdMustChange'][0];
				if (isset($attr['sambaHomePath'][0])) $return[$i]->smb_smbhome = utf8_decode($attr['sambaHomePath'][0]);
				if (isset($attr['sambaHomeDrive'][0])) $return[$i]->smb_homedrive = $attr['sambaHomeDrive'][0];
				if (isset($attr['sambaLogonScript'][0]))  $return[$i]->smb_scriptPath = utf8_decode($attr['sambaLogonScript'][0]);
				if (isset($attr['sambaProfilePath'][0])) $return[$i]->smb_profilePath = $attr['sambaProfilePath'][0];
				if (isset($attr['sambaUserWorkstations'][0])) $return[$i]->smb_smbuserworkstations = $attr['sambaUserWorkstations'][0];
				if (isset($attr['sambaNTPassword'][0]))  $return[$i]->smb_password = $attr['sambaNTPassword'][0];
				if (isset($attr['sambaDomainName'][0])) {
					if ($_SESSION['config']->is_samba3()) {
						$j=0;
						while (!isset($return[$i]->smb_domain) && (count($samba3domains)>$j)) {
							if ($attr['sambaDomainName'][0] == $samba3domains[$j]->name)
								$return[$i]->smb_domain = $samba3domains[$j];
							else $j++;
							}
						}
					// store smb_domain as string
					if (!isset($return[$i]->smb_domain)) $return[$i]->smb_domain = $attr['sambaDomainName'];
					}
				if (isset($attr['sambaPrimaryGroupSID'][0])) {
					if ($_SESSION['config']->is_samba3())
						// store "real" SID if we want to save user as samba3 entry
						$return[$i]->smb_mapgroup = $attr['sambaPrimaryGroupSID'][0];
						// store "calculated" id if we want to save user as samba2.2 entry
						else $return[$i]->smb_mapgroup = 2*$attr['gidNumber'][0]+1001;
					}
				}

			// sambaSamAccount (Samba2.2) is used.
			// second argument should prevent samba3 settings to be overwritten from samba 2.2 settings
			if ( (in_array('sambaAccount', $attr['objectClass'])) && (!$_SESSION['config']->is_samba3() || !isset($return[$i]->smb_domain))) {
				if (isset($attr['acctFlags'][0])) {
					if (strrpos($attr['acctFlags'][0], 'D')) $return[$i]->smb_flags['D']=true;
					if (strrpos($attr['acctFlags'][0], 'X')) $return[$i]->smb_flags['X']=true;
					if (strrpos($attr['acctFlags'][0], 'N')) $return[$i]->smb_flags['N']=true;
					if (strrpos($attr['acctFlags'][0], 'S')) $return[$i]->smb_flags['S']=true;
					if (strrpos($attr['acctFlags'][0], 'H')) $return[$i]->smb_flags['H']=true;
					}
				if (isset($attr['ntPassword'][0]))  $return[$i]->smb_password = $attr['ntPassword'][0];
				if (isset($attr['smbHome'][0])) $return[$i]->smb_smbhome = utf8_decode($attr['smbHome'][0]);
				if (isset($attr['pwdCanChange'][0])) $return[$i]->smb_pwdcanchange = $attr['pwdCanChange'][0];
				if (isset($attr['pwdMustChange'][0])) $return[$i]->smb_pwdmustchange = $attr['pwdMustChange'][0];
				if (isset($attr['homeDrive'][0])) $return[$i]->smb_homedrive = $attr['homeDrive'][0];
				if (isset($attr['scriptPath'][0]))  $return[$i]->smb_scriptPath = utf8_decode($attr['scriptPath'][0]);
				if (isset($attr['profilePath'][0])) $return[$i]->smb_profilePath = $attr['profilePath'][0];
				if (isset($attr['userWorkstations'][0])) $return[$i]->smb_smbuserworkstations = $attr['userWorkstations'][0];
				if (isset($attr['domain'][0])) {
					if ($_SESSION['config']->is_samba3()) {
						$j=0;
						while (!isset($return[$i]->smb_domain) && (count($samba3domains)>$j)) {
							if ($attr['domain'][0] == $samba3domains[$j]->name)
								$return[$i]->smb_domain = $samba3domains[$j];
							else $j++;
							}
						}
					// store smb_domain as string
					if (!isset($return[$i]->smb_domain)) $return[$i]->smb_domain = $attr['domain'];
					}
				if (isset($attr['primaryGroupID'][0])) {
					if ($_SESSION['config']->is_samba3())
						// store "real" SID if we want to save user as samba3 entry
						$return[$i]->smb_mapgroup = $return[$i]->smb_domain->SID. '-' . (2*$attr['primaryGroupID'][0]+1);
						// store "calculated" id if we want to save user as samba2.2 entry
						else $return[$i]->smb_mapgroup = $attr['primaryGroupID'][0];
					}
				}
			}
		$i++;
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}

	// Return array if $dns is an array
	// else return string
	if (is_array($dns)) return $return;
		else return $return[0];
	}



/* This function will load all needed values from an existing host account
* $dn is the dn(string) of the host which should be loaded
* return-value is an account-object
*/
function loadhost($dn) {
	// Create new object
	$return = new account();
	// Set type of account
	$return->type='host';
	// Load hostattributes from ldap
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixAccount");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);

	// Set host samba flags
	$return->smb_flags['W'] = true;
	$return->smb_flags['X'] = true;
	// load objectclasses
	$i=0;
	while (isset($attr['objectClass'][$i])) {
		$return->general_objectClass[$i] = $attr['objectClass'][$i];
		$i++;
		}

	$return->general_username = $attr['uid'][0];
	$return->general_uidNumber = $attr['uidNumber'][0];
	if (isset($attr['gecos'][0])) $return->general_gecos = utf8_decode($attr['gecos'][0]);

	// Get Groupname
	$return->general_group = getgrnam($attr['gidNumber'][0]);

	// sambaSamAccount (Samba3) is used.
	if (in_array('sambaSamAccount', $attr['objectClass'])) {
		/* Write attributes into $return.
		* Some values don't have to be set. These are only loaded if they are set
		*/
		if (isset($attr['sambaAcctFlags'][0])) {
			// we load a workstation
			$return->smb_flags['W']=true;
			if (strrpos($attr['sambaAcctFlags'][0], 'X')) $return->smb_flags['X']=true;
			// Because the "D"-Flag is ignored for hosts it has been removed
			}
		if (isset($attr['sambaDomainName'][0])) {
			if ($_SESSION['config']->is_samba3()) {
				// store smb_domain as samba3domain-Object
				$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
				$i=0;
				while (!isset($return->smb_domain) && (count($samba3domains)>$i)) {
					if ($attr['sambaDomainName'][0] == $samba3domains[$i]->name)
						$return->smb_domain = $samba3domains[$i];
					else $i++;
					}
				}
			// store smb_domain as string
			if (!isset($return->smb_domain)) $return->smb_domain = $attr['sambaDomainName'];
			}
		if (isset($attr['sambaPrimaryGroupSID'][0])) {
			if ($_SESSION['config']->is_samba3())
				// store "real" SID if we want to save user as samba3 entry
				$return->smb_mapgroup = $attr['sambaPrimaryGroupSID'][0];
				// store "calculated" id if we want to save user as samba2.2 entry
				else $return->smb_mapgroup = 2*$attr['gidNumber'][0]+1001;
			}
		// return value to prevent loaded values to be overwritten from old samba 2.2 attributes
		if ($_SESSION['config']->is_samba3()) return $return;
		}

	// sambaSamAccount (Samba2.2) is used.
	if (in_array('sambaAccount', $attr['objectClass'])) {
		if (isset($attr['acctFlags'][0])) {
			// we load a workstation
			$return->smb_flags['W']=true;
			if (strrpos($attr['acctFlags'][0], 'X')) $return->smb_flags['X']=true;
			// Because the "D"-Flag is ignored for hosts it has been removed
			}
		if (isset($attr['domain'][0])) {
			if ($_SESSION['config']->is_samba3()) {
				// store smb_domain as samba3domain-Object
				$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
				$i=0;
				while (!isset($return->smb_domain) && (count($samba3domains)>$i)) {
					if ($attr['domain'][0] == $samba3domains[$i]->name)
						$return->smb_domain = $samba3domains[$i];
					else $i++;
					}
				}
			// store smb_domain as string
			if (!isset($return->smb_domain)) $return->smb_domain = $attr['domain'];
			}
		if (isset($attr['primaryGroupID'][0])) {
			if ($_SESSION['config']->is_samba3())
				// store "real" SID if we want to save user as samba3 entry
				$return->smb_mapgroup = $return->smb_domain->SID. '-' . (2*$attr['primaryGroupID'][0]+1);
				// store "calculated" id if we want to save user as samba2.2 entry
				else $return->smb_mapgroup = $attr['primaryGroupID'][0];
			}
		}
	return $return;
	}


/* This function will load all needed values from an existing group account
* $dn is the dn(string) of the group which should be loaded
* return-value is an account-object
*/
function loadgroup($dn) {
	// Create new object
	$return = new account();
	// Set type of account
	$return->type='group';
	// Load userattributes from ldap
	$result = ldap_search($_SESSION['ldap']->server(), $dn, "objectclass=PosixGroup");
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
	$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);

	/* Write attributes into $return.
	* Some values don't have to be set. These are only loaded if they are set
	*/

	// load objectclasses
	$i=0;
	while (isset($attr['objectClass'][$i])) {
		$return->general_objectClass[$i] = $attr['objectClass'][$i];
		$i++;
		}
	// Load Users which are also members of group
	$i=0;
	while (isset($attr['memberUid'][$i])) {
		$return->unix_memberUid[$i] = $attr['memberUid'][$i];
		$i++;
		}

	$return->general_uidNumber = $attr['gidNumber'][0];
	$return->general_username = $attr['cn'][0];
	if (isset($attr['description'][0])) $return->general_gecos = utf8_decode($attr['description'][0]);

	if (isset($attr['sambaSID'][0])) {
		// Samba3 Samba 2.2 don't have any objects for groups
		$return->smb_mapgroup = $attr['sambaSID'][0];
		if (isset($attr['displayName'][0])) $return->smb_displayName = utf8_decode($attr['displayName'][0]);
		// extract SID from sambaSID to find domain
		$temp = explode('-', $attr['sambaSID'][0]);
		$SID = $temp[0].'-'.$temp[1].'-'.$temp[2].'-'.$temp[3].'-'.$temp[4].'-'.$temp[5].'-'.$temp[6];
		$samba3domains = $_SESSION['ldap']->search_domains($_SESSION[config]->get_domainSuffix());
		$i=0;
		while (!isset($return->smb_domain) && (count($samba3domains)>$i)) {
			if ($SID == $samba3domains[$i]->SID)
				$return->smb_domain = $samba3domains[$i];
			else $i++;
			}
		}
	return $return;
	}


/* This function will create a new user acconut in ldap
* $values is an account-object with all attributes of the user
* if lamdaemon.pl is false no quotas are set. Usefull for massupload and deletion
* return-value is an integer
*       1 == Account has been created
*       2 == Account already exists at different location
*	4 == Error while creating Account
*/
function createuser($values, $uselamdaemon=true) {
	// These Objectclasses are needed for an user account
	$attr['objectClass'][0] = 'posixAccount';
	$attr['objectClass'][1] = 'shadowAccount';
	$attr['objectClass'][2] = 'inetOrgPerson';
	// Create DN for new user account
	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;
	// decrypt password because we don't want to store them unencrypted in session
	$iv = base64_decode($_COOKIE["IV"]);
	$key = base64_decode($_COOKIE["Key"]);
	if ($values->unix_password != '') {
		$values->unix_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->unix_password), MCRYPT_MODE_ECB, $iv);
		$values->unix_password = str_replace(chr(00), '', $values->unix_password);
		}
	if ($values->smb_password != '') {
		$values->smb_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->smb_password), MCRYPT_MODE_ECB, $iv);
		$values->smb_password = str_replace(chr(00), '', $values->smb_password);
		}

	// Attributes which are required
	$attr['cn'] = $values->general_username;
	$attr['uid'] = $values->general_username;
	$attr['uidNumber'] = $values->general_uidNumber;
	$attr['gidNumber'] = getgid($values->general_group);
	$attr['homeDirectory'] = $values->general_homedir;
	$attr['givenName'] = utf8_encode($values->general_givenname);
	$attr['sn'] = utf8_encode($values->general_surname);
	// values stored in shadowExpire, days since 1.1.1970
	if (isset($values->unix_pwdexpire)) $attr['shadowExpire'] =  intval($values->unix_pwdexpire / 86400) ;

	/* Write unix attributes into $attr array
	* Some values don't have to be set. These are only loaded if they are set
	*/
	$attr['loginShell'] = $values->general_shell; // posixAccount_may
	$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos)); // posixAccount_may
	$attr['description'] = utf8_encode($values->general_gecos); // posixAccount_may sambaAccount_may
	if ($values->unix_pwdminage!='') $attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	if ($values->unix_pwdmaxage!='') $attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	if ($values->unix_pwdwarn!='') $attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	if ($values->unix_pwdallowlogin!='') $attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	// Set unix password
	$attr['shadowLastChange'] = getdays(); // shadowAccount_may
	if ($values->unix_password_no) $values->unix_password = '';
	if ($values->unix_deactivated) $attr['userPassword'] = pwd_hash($values->unix_password, false);
	    else $attr['userPassword'] = pwd_hash($values->unix_password);

	// explode host-string and save every allowed host as separate attribute
	$values->unix_host = str_replace(' ', '', $values->unix_host);
	$hosts = explode (',', $values->unix_host);
	$i=0;
	while(isset($hosts[$i])) {
		if ($hosts[$i]!='') $attr['host'][$i] = $hosts[$i];
		$i++;
		}

	// Samba attributes
	if ($_SESSION['config']->is_samba3()) {
		// Add all attributes as samba3 objectclass
		$attr['objectClass'][3] = 'sambaSamAccount';
		if ($values->smb_flags['N']) {
			// Don't set samba-passwords
			$attr['sambaNTPassword'] = 'NO PASSWORD*****';
			$attr['sambaLMPassword'] = 'NO PASSWORD*****';
			}
		else {
			// use escapeshellarg() to make command shell-secure
			// Set samba-passwords with external perl-script
			$attr['sambaNTPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($values->smb_password));
			$attr['sambaLMPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($values->smb_password));
			}
		$attr['sambaPwdLastSet'] = time();
		// Generate SID
		$attr['sambaSID'] = $values->smb_domain->SID . "-"  . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may

		//if ($values->smb_mapgroup!='') $attr['sambaPrimaryGroupSID'] = $values->smb_mapgroup; // sambaAccount_req
		ldapreload('group');
		foreach ($_SESSION['groupDN'] as $groupname) {
			if ($groupname['cn'] == $values->general_group) $attr['sambaPrimaryGroupSID'] = $groupname['sambaSID'];
			}
		if ($values->smb_pwdcanchange!='') $attr['sambaPwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
			else $attr['sambaPwdCanChange'] = time(); // sambaAccount_may
		if ($values->smb_pwdmustchange!='') $attr['sambaPwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
			else $attr['sambaPwdMustChange'] = time() + 1000000000; // sambaAccount_may
		$attr['sambaAcctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
		$attr['displayName'] = $values->general_gecos; // sambaAccount_may
		if ($values->smb_smbhome!='') $attr['sambaHomePath'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if ($values->smb_homedrive!='') $attr['sambaHomeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if ($values->smb_scriptPath!='') $attr['sambaLogonScript']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if ($values->smb_profilePath!='') $attr['sambaProfilePath'] = $values->smb_profilePath; // sambaAccount_may
		if ($values->smb_smbuserworkstations!='') $attr['sambaUserWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if ($values->smb_domain!='') $attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		}
	else {
		// Add all attributes as samba2.2 objectclass
		$attr['objectClass'][3] = 'sambaAccount';
		if ($values->smb_flags['N']) {
			// Don't set samba-passwords
			$attr['ntPassword'] = 'NO PASSWORD*****';
			$attr['lmPassword'] = 'NO PASSWORD*****';
			}
		else {
			// use escapeshellarg() to make command shell-secure
			// Set samba-passwords with external perl-script
			$attr['ntPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($values->smb_password));
			$attr['lmPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($values->smb_password));
			}
		$attr['pwdLastSet'] = time();
		// Generate pseudo SID
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		$attr['primaryGroupID'] = $values->smb_mapgroup; // sambaAccount_req
		if ($values->smb_pwdcanchange!='') $attr['pwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
			else $attr['pwdCanChange'] = time(); // sambaAccount_may
		if ($values->smb_pwdmustchange!='') $attr['pwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
			else $attr['pwdMustChange'] = time() + 1000000000; // sambaAccount_may
		$attr['pwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
		$attr['acctFlags'] = smbflag($values->smbflags); // sambaAccount_may
		$attr['displayName'] = $values->general_gecos; // sambaAccount_may
		if ($values->smb_smbhome!='') $attr['smbHome'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if ($values->smb_homedrive!='') $attr['homeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if ($values->smb_scriptPath!='') $attr['scriptPath']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if ($values->smb_profilePath!='') $attr['profilePath'] = $values->smb_profilePath; // sambaAccount_may
		if ($values->smb_smbuserworkstations!='') $attr['userWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
		}
	$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may

	// personal attributes
	if ($values->personal_title!='') $attr['title'] = utf8_encode($values->personal_title);
	if ($values->personal_mail!='') $attr['mail'] = utf8_encode($values->personal_mail);
	if ($values->personal_telephoneNumber!='') $attr['telephoneNumber'] = utf8_encode($values->personal_telephoneNumber);
	if ($values->personal_mobileTelephoneNumber!='') $attr['mobileTelephoneNumber'] = utf8_encode($values->personal_mobileTelephoneNumber);
	if ($values->personal_facsimileTelephoneNumber!='') $attr['facsimileTelephoneNumber'] = utf8_encode($values->personal_facsimileTelephoneNumber);
	if ($values->personal_street!='') $attr['street'] = utf8_encode($values->personal_street);
	if ($values->personal_postalCode!='') $attr['postalCode'] = utf8_encode($values->personal_postalCode);
	if ($values->personal_postalAddress!='') $attr['postalAddress'] = utf8_encode($values->personal_postalAddress);
	if ($values->personal_employeeType!='') $attr['employeeType'] = utf8_encode($values->personal_employeeType);

	// Create LDAP user account
	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	// Continue if now error did ocour
	if (!$success) return 4;

	if ($_SESSION['config']->scriptServer && $uselamdaemon) {
		// lamdaemon.pl should be used
		// Set quotas if quotas are used
		if (is_array($values->quota)) setquotas(array($values));
		// Create Homedirectory
		addhomedir(array($values->general_username));
		}

	// Add User to Additional Groups
	if (isset($values->general_groupadd[0]))
		// Loop for every group
		foreach ($values->general_groupadd as $group2) {
			// Search for group in LDAP
			$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "(&(objectclass=posixGroup)(cn=$group2))", array(''));
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			// Get DN
			$dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
			// Add user to group
			$success = ldap_mod_add($_SESSION['ldap']->server(), $dn, array('memberUid' => $values->general_username));
			if (!$success) return 4;
			// Add new memberUid to cache-array
			ldapreload('group');
			$_SESSION['groupDN'][$dn]['memberUid'][] = $values->general_username;
			}
	// Add new user to cache-array
	if ((isset($_SESSION['userDN']))) {
		$_SESSION['userDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['userDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	// Everything is OK, return 1
	return 1;
	}

/* This function will modify a user acconut in ldap
* $values and $values_old are an account-object with all
* attributes of the user.
* if lamdaemon.pl is false no quotas are set. Usefull for massupload and deletion
* Only attributes which have changed will be written
* return-value is an integer
*       2 == Account already exists at different location
*       3 == Account has been modified
*	5 == Error while modifying Account
*/
function modifyuser($values,$values_old,$uselamdaemon=true) { // Will modify the LDAP-Account
	// Add missing objectclasses to user
	if (!in_array('posixAccount', $values->general_objectClass)) {
		$attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'posixAccount';
		}
	if (!in_array('shadowAccount', $values->general_objectClass)) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'shadowAccount';
		}
	// Create DN for new user account
	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;
	// decrypt password because we don't want to store them unencrypted in session
	$iv = base64_decode($_COOKIE["IV"]);
	$key = base64_decode($_COOKIE["Key"]);
	if ($values->unix_password != '') {
		$values->unix_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->unix_password), MCRYPT_MODE_ECB, $iv);
		$values->unix_password = str_replace(chr(00), '', $values->unix_password);
		}
	if ($values->smb_password != '') {
		$values->smb_password = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, base64_decode($values->smb_password), MCRYPT_MODE_ECB, $iv);
		$values->smb_password = str_replace(chr(00), '', $values->smb_password);
		}
	// Attributes which are required
	if ($values->general_username != $values_old->general_username) {
		$attr['cn'] = $values->general_username;
		$attr['uid'] = $values->general_username;
		}
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['uidNumber'] = $values->general_uidNumber;
		// Because sambaSid(rid) is related to uidNumber we have to change it if uidNumbaer has changed
		if ($_SESSION['config']->is_samba3())
			$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase);
		else $attr['rid'] = (2 * $values->general_uidNumber + 1000);
		}
	if ($values->general_group != $values_old->general_group) {
		$attr['gidNumber'] = getgid($values->general_group);
		// Because primaryGroup(S)ID is related to gidNumber we have to change it if gidNumber has changed
		if ($_SESSION['config']->is_samba3()) {
			// We use samba 3 schema
			// Change SID only if we don't use a well known SID
			ldapreload('group');
			foreach ($_SESSION['groupDN'] as $groupname) {
				if ($groupname['cn'] == $values->general_group) $attr['sambaPrimaryGroupSID'] = $groupname['sambaSID'];
				}
			}
		else {
			// We use old samba 2.2 schema
			// Change SID only if we don't use a well known SID
			if ($values->smb_mapgroup== '512') $found=true;
			if ($values->smb_mapgroup== '513') $found=true;
			if ($values->smb_mapgroup== '514') $found=true;
			if (!$found) $attr['primaryGroupID'] = (2 * getgid($values->general_group) + 1001);
			}
		}
	if ($values->general_homedir != $values_old->general_homedir)
		$attr['homeDirectory'] = $values->general_homedir;
	if ($values->general_givenname!=$values_old->general_givenname) $attr['givenName'] = utf8_encode($values->general_givenname);
	if ($values->general_surname!=$values_old->general_surname) $attr['sn'] = utf8_encode($values->general_surname);

	/* Write unix attributes into $attr array
	* Some values don't have to be set. These are only loaded if they are set
	*/
	if ($values->general_shell != $values_old->general_shell)
		$attr['loginShell'] = $values->general_shell;
	if ($values->general_gecos != $values_old->general_gecos) {
		$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos));
		$attr['description'] = utf8_encode($values->general_gecos);
		}
	if (($values->unix_pwdminage != $values_old->unix_pwdminage) && ($values->unix_pwdminage !=''))
		$attr['shadowMin'] = $values->unix_pwdminage; // shadowAccount_may
	if (($values->unix_pwdminage != $values_old->unix_pwdminage) && ($values->unix_pwdminage ==''))
		$attr_rem['shadowMin'] = $values_old->unix_pwdminage; // shadowAccount_may
	if (($values->unix_pwdmaxage != $values_old->unix_pwdmaxage) && ($values->unix_pwdmaxage !=''))
		$attr['shadowMax'] = $values->unix_pwdmaxage; // shadowAccount_may
	if (($values->unix_pwdmaxage != $values_old->unix_pwdmaxage) && ($values->unix_pwdmaxage ==''))
		$attr_rem['shadowMax'] = $values_old->unix_pwdmaxage; // shadowAccount_may
	if (($values->unix_pwdwarn != $values_old->unix_pwdwarn) && ($values->unix_pwdwarn !=''))
		$attr['shadowWarning'] = $values->unix_pwdwarn; // shadowAccount_may
	if (($values->unix_pwdwarn != $values_old->unix_pwdwarn) && ($values->unix_pwdwarn ==''))
		$attr_rem['shadowWarning'] = $values_old->unix_pwdwarn; // shadowAccount_may
	if (($values->unix_pwdallowlogin != $values_old->unix_pwdallowlogin) && ($values->unix_pwdallowlogin !=''))
		$attr['shadowInactive'] = $values->unix_pwdallowlogin; // shadowAccount_may
	if (($values->unix_pwdallowlogin != $values_old->unix_pwdallowlogin) && ($values->unix_pwdallowlogin ==''))
		$attr_rem['shadowInactive'] = $values_old->unix_pwdallowlogin; // shadowAccount_may
	// Check if shadow expire has changed
	if ($values->unix_pwdexpire != $values_old->unix_pwdexpire) $attr['shadowExpire'] = intval($values->unix_pwdexpire / 86400) ;
	// Set unix password
	if ($values->unix_password=='') {
		// $values->unix_password=='' means use old password
		if ($values->unix_deactivated != $values_old->unix_deactivated) {
			// (de)activate password
			// Split old password hash in {CRYPT} and password-hash
			$i = 0;
			while ($values_old->unix_password{$i} != '}') $i++;
			$passwd = substr($values_old->unix_password, $i+1 );
			$crypt = substr($values_old->unix_password, 0, $i+1 );
			// remove trailing ! from password hash
			if ($passwd{0} == '!') $passwd = substr($passwd, 1);
			// Write new password
			if ($values->unix_deactivated) $attr['userPassword'] = $crypt.'!'.$passwd;
				else $attr['userPassword'] = $crypt.$passwd;
			}
		if ($values->unix_password_no) {
			// use no password
			if ($values->unix_deactivated) $attr['userPassword'] = pwd_hash('', false);
				else $attr['userPassword'] = pwd_hash('');
			$attr['shadowLastChange'] = getdays(); // shadowAccount_may
			}
		}
	else {
		// Set new password
		if ($values->unix_password_no) $values->unix_password = '';
		if ($values->unix_deactivated) $attr['userPassword'] = pwd_hash($values->unix_password, false);
		else $attr['userPassword'] = pwd_hash($values->unix_password);
		$attr['shadowLastChange'] = getdays(); // shadowAccount_may
		}
	// explode host-string and save every allowed host as separate attribute
	if (($values->unix_host != $values_old->unix_host)) {
		$values->unix_host = str_replace(' ', '', $values->unix_host);
		$host = explode (',', $values->unix_host);
		$values_old->unix_host = str_replace(' ', '', $values_old->unix_host);
		$host_old = explode (',', $values_old->unix_host);
		if ($host[0]=='') $attr_rem['host'] = $host_old;
		 else if ($host[0]!='') $attr['host'] = $host;
		}

	// Samba attributes
	if ($_SESSION['config']->is_samba3()) {
		if (!in_array('sambaSamAccount', $values->general_objectClass)) {
			// We have to convert sambaAccount Objectclass to sambaSamAccount objectclass
			if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
			$attr['objectClass'][] = 'sambaSamAccount';
			// unset old sambaAccount objectClass
			for ($i=0; $i<count($attr['objectClass']); $i++)
				if ($attr['objectClass'][$i]=='sambaAccount') unset($attr['objectClass'][$i]);
			$attr['objectClass'] = array_values($attr['objectClass']);
			// Set correct values for new objectclass
			// Load old samba-values not stored in account object
			$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			// Add new attributed
			if (isset($attr_old['lmPassword'][0])) $attr['sambaLMPassword'] = $attr_old['lmPassword'][0];
			if (isset($attr_old['ntPassword'][0])) $attr['sambaNTPassword'] = $attr_old['ntPassword'][0];
			if (isset($attr_old['pwdLastSet'][0])) $attr['sambaPwdLastSet'] = $attr_old['pwdLastSet'][0];
			if (isset($attr_old['logonTime'][0])) $attr['sambaLogonTime'] = $attr_old['logonTime'][0];
			if (isset($attr_old['logoffTime'][0])) $attr['sambaLogoffTime'] = $attr_old['logoffTime'][0];
			if (isset($attr_old['kickoffTime'][0])) $attr['sambaKickoffTime'] = $attr_old['kickoffTime'][0];
			if (isset($attr_old['pwdCanChange'][0])) $attr['sambaPwdCanChange'] = $attr_old['pwdCanChange'][0];
			if (isset($attr_old['pwdMustChange'][0])) $attr['sambaPwdMustChange'] = $attr_old['pwdMustChange'][0];
			if (isset($attr_old['smbHome'][0])) $attr['sambaHomePath'] = $attr_old['smbHome'][0];
			if (isset($attr_old['homeDrive'][0])) $attr['sambaHomeDrive'] = $attr_old['homeDrive'][0];
			if (isset($attr_old['scriptPath'][0])) $attr['sambaLogonScript'] = $attr_old['scriptPath'][0];
			if (isset($attr_old['profilePath'][0])) $attr['sambaProfilePath'] = $attr_old['profilePath'][0];
			if (isset($attr_old['userWorkstations'][0])) $attr['sambaUserWorkstations'] = $attr_old['userWorkstations'][0];
			// Values used from account object
			$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
			$attr['sambaAcctFlags'] = smbflag($values->flags); // sambaAccount_may
			$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
			$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
			$attr['sambaPrimaryGroupSID'] = $values->smb_mapgroup; // sambaAccount_req
			// remove old attributes
			if (in_array('sambaAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaAccount';
			if (isset($attr_old['lmPassword'][0])) $attr_rem['lmPassword'] = $attr_old['lmPassword'][0];
			if (isset($attr_old['ntPassword'][0])) $attr_rem['ntPassword'] = $attr_old['ntPassword'][0];
			if (isset($attr_old['pwdLastSet'][0])) $attr_rem['pwdLastSet'] = $attr_old['pwdLastSet'][0];
			if (isset($attr_old['logonTime'][0])) $attr_rem['logonTime'] = $attr_old['logonTime'][0];
			if (isset($attr_old['kickoffTime'][0])) $attr_rem['kickoffTime'] = $attr_old['kickoffTime'][0];
			if (isset($attr_old['pwdCanChange'][0])) $attr_rem['pwdCanChange'] = $attr_old['pwdCanChange'][0];
			if (isset($attr_old['pwdMustChange'][0])) $attr_rem['pwdMustChange'] = $attr_old['pwdMustChange'][0];
			if (isset($attr_old['smbHome'][0])) $attr_rem['smbHome'] = $attr_old['smbHome'][0];
			if (isset($attr_old['acctFlags'][0])) $attr_rem['acctFlags'] = $attr_old['acctFlags'][0];
			if (isset($attr_old['homeDrive'][0])) $attr_rem['homeDrive'] = $attr_old['homeDrive'][0];
			if (isset($attr_old['scriptPath'][0])) $attr_rem['scriptPath'] = $attr_old['scriptPath'][0];
			if (isset($attr_old['profilePath'][0])) $attr_rem['profilePath'] = $attr_old['profilePath'][0];
			if (isset($attr_old['userWorkstations'][0])) $attr_rem['userWorkstations'] = $attr_old['userWorkstations'][0];
			if (isset($attr_old['primaryGroupID'][0])) $attr_rem['primaryGroupID'] = $attr_old['primaryGroupID'][0];
			if (isset($attr_old['domain'][0])) $attr_rem['domain'] = $attr_old['domain'][0];
			if (isset($attr_old['rid'][0])) $attr_rem['rid'] = $attr_old['rid'][0];
			}
		// Set all changed values
		if ($values->smb_flags['N']) {
			// use no samba Password
			$attr['sambaNTPassword'] = 'NO PASSWORD*****';
			$attr['sambaLMPassword'] = 'NO PASSWORD*****';
			$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
			}
		 else
			if ($values->smb_password!='') {
				// use escapeshellarg() to make command shell-secure
				// Set samba-passwords with external perl-script
				$attr['sambaNTPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($values->smb_password));
				$attr['sambaLMPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($values->smb_password));
				$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
				}
		// Check which Samba-Attributes have changed
		if ($values->smb_pwdcanchange != $values_old->smb_pwdcanchange) $attr['sambaPwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
		if ($values->smb_pwdmustchange != $values_old->smb_pwdmustchange) $attr['sambaPwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
		if (smbflag($values->smb_flags) != smbflag($values_old->smb_flags)) $attr['sambaAcctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
		if (($values->smb_smbhome!='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr['sambaHomePath'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if (($values->smb_smbhome=='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr_rem['sambaHomePath'] = utf8_encode($values_old->smb_smbhome); // sambaAccount_may
		if (($values->smb_homedrive!='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr['sambaHomeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if (($values->smb_homedrive=='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr_rem['sambaHomeDrive'] = $values_old->smb_homedrive; // sambaAccount_may
		if (($values->smb_scriptPath!='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr['sambaLogonScript']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if (($values->smb_scriptPath=='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr_rem['sambaLogonScript']  = utf8_encode($values_old->smb_scriptPath); // sambaAccount_may
		if (($values->smb_profilePath!='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr['sambaProfilePath'] = $values->smb_profilePath; // sambaAccount_may
		if (($values->smb_profilePath=='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr_rem['sambaProfilePath'] = $values_old->smb_profilePath; // sambaAccount_may
		if (($values->smb_smbuserworkstations!='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr['sambaUserWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if (($values->smb_smbuserworkstations=='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr_rem['sambaUserWorkstations'] = $values_old->smb_smbuserworkstations; // sambaAccount_may
		if ($values->smb_domain->name!=$values_old->smb_domain->name) {
			$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
			$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase);
			$attr['sambaPrimaryGroupSID'] = $values->smb_domain->SID . "-".
				(2 * getgid($values->general_group) + $values->smb_domain->RIDbase+1);
			}
		if (($values->smb_mapgroup!='') && ($values->smb_mapgroup!=$values_old->smb_mapgroup)) {
			ldapreload('group');
			foreach ($_SESSION['groupDN'] as $groupname) {
				if ($groupname['cn'] == $values->general_group) $attr['sambaPrimaryGroupSID'] = $groupname['sambaSID'];
				}
			}
		if ($values->smb_displayName != $values_old->smb_displayName) $attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		}
	else {
		// use old samba 2.2 objectclass
		if (!in_array('sambaAccount', $values->general_objectClass)) {
			// Add or convert samba attributes & object to samba 2.2
			if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
			$attr['objectClass'][] = 'sambaAccount';
			// unset old sambaAccount objectClass
			for ($i=0; $i<count($attr['objectClass']); $i++)
				if ($attr['objectClass'][$i]=='sambaSamAccount') unset($attr['objectClass'][$i]);
			$attr['objectClass'] = array_values($attr['objectClass']);
			// Set correct values for new objectclass
			// Load old samba-values not stored in account object
			$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if (isset($attr_old['sambaLMPassword'][0])) $attr['lmPassword'] = $attr_old['sambaLMPassword'][0];
			if (isset($attr_old['sambaNTPassword'][0])) $attr['ntPassword'] = $attr_old['sambaNTPassword'][0];
			if (isset($attr_old['sambaPwdLastSet'][0])) $attr['pwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
			if (isset($attr_old['sambaLogonTime'][0])) $attr['logonTime'] = $attr_old['sambaLogonTime'][0];
			if (isset($attr_old['sambaLogoffTime'][0])) $attr['logoffTime'] = $attr_old['sambaLogoffTime'][0];
			if (isset($attr_old['sambaKickoffTime'][0])) $attr['kickoffTime'] = $attr_old['sambaKickoffTime'][0];
			if (isset($attr_old['sambaPwdCanChange'][0])) $attr['pwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
			if (isset($attr_old['sambaPwdMustChange'][0])) $attr['pwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
			if (isset($attr_old['sambaHomePath'][0])) $attr['smbHome'] = $attr_old['sambaHomePath'][0];
			if (isset($attr_old['sambaHomeDrive'][0])) $attr['homeDrive'] = $attr_old['sambaHomeDrive'][0];
			if (isset($attr_old['sambaLogonScript'][0])) $attr['scriptPath'] = $attr_old['sambaLogonScript'][0];
			if (isset($attr_old['sambaProfilePath'][0])) $attr['profilePath'] = $attr_old['sambaProfilePath'][0];
			if (isset($attr_old['sambaUserWorkstations'][0])) $attr['userWorkstations'] = $attr_old['sambaUserWorkstations'][0];
			// Values used from account object
			$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
			$attr['acctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
			if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
			$attr['primaryGroupID'] = $values->smb_mapgroup; // sambaAccount_req
			$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
			// remove old attributes
			if (in_array('sambaSamAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaSamAccount';
			if (isset($attr_old['sambaLMPassword'][0])) $attr_rem['sambaLMPassword'] = $attr_old['sambaLMPassword'][0];
			if (isset($attr_old['sambaNTPassword'][0])) $attr_rem['sambaNTPassword'] = $attr_old['sambaNTPassword'][0];
			if (isset($attr_old['sambaPwdLastSet'][0])) $attr_rem['sambaPwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
			if (isset($attr_old['sambaLogonTime'][0])) $attr_rem['sambaLogonTime'] = $attr_old['sambaLogonTime'][0];
			if (isset($attr_old['sambaKickoffTime'][0])) $attr_rem['sambaKickoffTime'] = $attr_old['sambaKickoffTime'][0];
			if (isset($attr_old['sambaPwdCanChange'][0])) $attr_rem['sambaPwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
			if (isset($attr_old['sambaPwdMustChange'][0])) $attr_rem['sambaPwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
			if (isset($attr_old['sambaHomePath'][0])) $attr_rem['sambaHomePath'] = $attr_old['sambaHomePAth'][0];
			if (isset($attr_old['sambaAcctFlags'][0])) $attr_rem['sambaAcctFlags'] = $attr_old['sambaAcctFlags'][0];
			if (isset($attr_old['sambaHomeDrive'][0])) $attr_rem['sambaHomeDrive'] = $attr_old['sambaHomeDrive'][0];
			if (isset($attr_old['sambaLogonScript'][0])) $attr_rem['sambaLogonScript'] = $attr_old['sambaLogonScript'][0];
			if (isset($attr_old['sambaProfilePath'][0])) $attr_rem['sambaProfilePath'] = $attr_old['sambaProfilePath'][0];
			if (isset($attr_old['sambaUserWorkstations'][0])) $attr_rem['sambaUserWorkstations'] = $attr_old['sambaUserWorkstations'][0];
			if (isset($attr_old['sambaPrimaryGroupID'][0])) $attr_rem['sambaPrimaryGroupID'] = $attr_old['sambaPrimaryGroupID'][0];
			if (isset($attr_old['sambaDomainName'][0])) $attr_rem['sambaDomainName'] = $attr_old['sambaDomainName'][0];
			if (isset($attr_old['sambaSID'][0])) $attr_rem['sambaSID'] = $attr_old['sambaSID'][0];
			}
		// Set all changed values
		if ($values->smb_flags['N']) {
			// use no samba Password
			$attr['ntPassword'] = 'NO PASSWORD*****';
			$attr['lmPassword'] = 'NO PASSWORD*****';
			$attr['pwdLastSet'] = time();
			}
		 else
			if ($values->smb_password!='') {
				// use escapeshellarg() to make command shell-secure
				// Set samba-passwords with external perl-script
				$attr['ntPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." nt ".escapeshellarg($values->smb_password));
				$attr['lmPassword'] = exec(escapeshellarg($_SESSION['lampath'].'lib/createntlm.pl')." lm ".escapeshellarg($values->smb_password));
				$attr['pwdLastSet'] = time(); // sambaAccount_may
				}
		// Check which Samba-Attributes have changed
		if ($values->smb_pwdcanchange != $values_old->smb_pwdcanchange) $attr['pwdCanChange'] = $values->smb_pwdcanchange; // sambaAccount_may
		if ($values->smb_pwdmustchange != $values_old->smb_pwdmustchange) $attr['pwdMustChange'] = $values->smb_pwdmustchange; // sambaAccount_may
		if (smbflag($values->smb_flags) != smbflag($values_old->smb_flags)) $attr['acctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
		if (($values->smb_smbhome!='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr['smbHome'] = utf8_encode($values->smb_smbhome); // sambaAccount_may
		if (($values->smb_smbhome=='') && ($values->smb_smbhome!=$values_old->smb_smbhome)) $attr_rem['smbHome'] = utf8_encode($values_old->smb_smbhome); // sambaAccount_may
		if (($values->smb_homedrive!='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr['homeDrive'] = $values->smb_homedrive; // sambaAccount_may
		if (($values->smb_homedrive=='') && ($values->smb_homedrive!=$values_old->smb_homedrive)) $attr_rem['homeDrive'] = $values_old->smb_homedrive; // sambaAccount_may
		if (($values->smb_scriptPath!='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr['scriptPath']  = utf8_encode($values->smb_scriptPath); // sambaAccount_may
		if (($values->smb_scriptPath=='') && ($values->smb_scriptPath!=$values_old->smb_scriptPath)) $attr_rem['scriptPath']  = utf8_encode($values_old->smb_scriptPath); // sambaAccount_may
		if (($values->smb_profilePath!='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr['profilePath'] = $values->smb_profilePath; // sambaAccount_may
		if (($values->smb_profilePath=='') && ($values->smb_profilePath!=$values_old->smb_profilePath)) $attr_rem['profilePath'] = $values_old->smb_profilePath; // sambaAccount_may
		if (($values->smb_smbuserworkstations!='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr['userWorkstations'] = $values->smb_smbuserworkstations; // sambaAccount_may
		if (($values->smb_smbuserworkstations=='') && ($values->smb_smbuserworkstations!=$values_old->smb_smbuserworkstations))$attr_rem['userWorkstations'] = $values_old->smb_smbuserworkstations; // sambaAccount_may
		if (($values->smb_domain!='') && ($values->smb_domain!=$values_old->smb_domain)) $attr['domain'] = $values->smb_domain; // sambaAccount_may
		if (($values->smb_domain=='') && ($values->smb_domain!=$values_old->smb_domain)) $attr_rem['domain'] = $values_old->smb_domain; // sambaAccount_may
		if (($values->smb_mapgroup!='') && ($values->smb_mapgroup!=$values_old->smb_mapgroup)) $attr['primaryGroupID'] = $values->smb_mapgroup; // sambaAccount_may
		if (($values->smb_mapgroup=='') && ($values->smb_mapgroup!=$values_old->smb_mapgroup)) $attr_rem['primaryGroupID'] = $values_old->smb_mapgroup;
		if ($values->smb_displayName != $values_old->smb_displayName) $attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
		}

	// Check which personal attributes have changed
	if (($values->personal_title != $values_old->personal_title) && ($values->personal_title != ''))
		$attr['title'] = utf8_encode($values->personal_title);
	if (($values->personal_title != $values_old->personal_title) && ($values->personal_title == ''))
		$attr_rem['title'] = utf8_encode($values_old->personal_title);
	if (($values->personal_mail != $values_old->personal_mail) && ($values->personal_mail != ''))
		$attr['mail'] = utf8_encode($values->personal_mail);
	if (($values->personal_mail != $values_old->personal_mail) && ($values->personal_mail == ''))
		$attr_rem['mail'] = utf8_encode($values_old->personal_mail);
	if (($values->personal_telephoneNumber != $values_old->personal_telephoneNumber) && ($values->personal_telephoneNumber !=''))
		$attr['telephoneNumber'] = utf8_encode($values->personal_telephoneNumber);
	if (($values->personal_telephoneNumber != $values_old->personal_telephoneNumber) && ($values->personal_telephoneNumber ==''))
		$attr_rem['telephoneNumber'] = utf8_encode($values_old->personal_telephoneNumber);
	if (($values->personal_mobileTelephoneNumber != $values_old->personal_mobileTelephoneNumber) && ($values->personal_mobileTelephoneNumber!=''))
		$attr['mobileTelephoneNumber'] = utf8_encode($values->personal_mobileTelephoneNumber);
	if (($values->personal_mobileTelephoneNumber != $values_old->personal_mobileTelephoneNumber) && ($values->personal_mobileTelephoneNumber==''))
		$attr_rem['mobileTelephoneNumber'] = utf8_encode($values_old->personal_mobileTelephoneNumber);
	if (($values->personal_facsimileTelephoneNumber != $values_old->personal_facsimileTelephoneNumber) && ($values->personal_facsimileTelephoneNumber!=''))
		$attr['facsimileTelephoneNumber'] = utf8_encode($values->personal_facsimileTelephoneNumber);
	if (($values->personal_facsimileTelephoneNumber != $values_old->personal_facsimileTelephoneNumber) && ($values->personal_facsimileTelephoneNumber==''))
		$attr_rem['facsimileTelephoneNumber'] = utf8_encode($values_old->personal_facsimileTelephoneNumber);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street!=''))
		$attr['street'] = utf8_encode($values->personal_street);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street==''))
		$attr_rem['street'] = utf8_encode($values_old->personal_street);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street!=''))
		$attr['postalCode'] = utf8_encode($values->personal_street);
	if (($values->personal_street != $values_old->personal_street) && ($values->personal_street==''))
		$attr_rem['postalCode'] = utf8_encode($values_old->personal_street);
	if (($values->personal_postalAddress != $values_old->personal_postalAddress) && ($values->personal_postalAddress!=''))
		$attr['postalAddress'] = utf8_encode($values->personal_postalAddress);
	if (($values->personal_postalAddress != $values_old->personal_postalAddress) && ($values->personal_postalAddress==''))
		$attr_rem['postalAddress'] = utf8_encode($values_old->personal_postalAddress);
	if (($values->personal_employeeType != $values_old->personal_employeeType) && ($values->personal_employeeType!=''))
		$attr['employeeType'] = utf8_encode($values->personal_employeeType);
	if (($values->personal_employeeType != $values_old->personal_employeeType) && ($values->personal_employeeType==''))
		$attr_rem['employeeType'] = utf8_encode($values_old->personal_employeeType);

	if (($values->general_uidNumber != $values_old->general_uidNumber) && $_SESSION['config']->scriptServer) {
		// Remove old quotas
		remquotas(array($values_old->general_username), "user");
		// Remove quotas from $values_old because we have to rewrite them all
		unset ($values_old->quota);
		}

	if ($values->general_dn != $values_old->general_dn) {
		// Account should be moved to a new location
		// Load old account
		$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		// remove "count" from array
		unset($attr_old['count']);
		for ($i=0; $i < sizeof($attr_old); $i++) unset($attr_old[$i]);
		$keys = array_keys($attr_old);
		for ($i=0; $i < sizeof($keys); $i++)
			unset($attr_old[$keys[$i]]['count']);
		// Change uid to new uid. Else ldap won't create the new entry
		//$attr_old['uid'][0] = $values->general_username;
		$attr_rem_keys = @array_keys($attr_rem);
		for ($i=0; $i<count($attr_rem_keys); $i++) { // Remove attributes which should be deleted from array
			if (isset($attr_old[$attr_rem_keys[$i]][0])) unset($attr_old[$attr_rem_keys[$i]]);
			}
		$attr_keys = @array_keys($attr);
		for ($i=0; $i<count($attr_keys); $i++) { // Add attributes which have changed
			$attr_old[$attr_keys[$i]][0] = $attr[$attr_keys[$i]];
			}
		// Create account at new location
		$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
		// remove old account
		if ($success) $success = ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
		if (!$success) return 5;
		// Remove all memberUid entries. The new entries will be added again
		// Search for groups which have memberUid set to username
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), "(&(objectClass=PosixGroup)(memberUid=$values_old->general_username))", array(''));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		// loop for every found group and remove membership
		while ($entry) {
			$dn = ldap_get_dn($_SESSION['ldap']->server(), $entry);
			$success = ldap_mod_del($_SESSION['ldap']->server(), $dn , array('memberUid' => $values_old->general_username));
			// *** fixme add error-message if memberUid couldn't be deleted
			// Remove old memberUid to cache-array
			if ((isset($_SESSION['groupDN']))) {
				if (!in_array($values->general_username, $_SESSION['groupDN'][$dn]['memberUid'])) {
					$i=0;
					for ($i=0; $i<count($dn); $i++) {
						if ($_SESSION['groupDN'][$dn]['memberUid'][$i] == $values->general_username) unset ($_SESSION['groupDN'][$dn]['memberUid'][$i]);
						$_SESSION['groupDN'][$dn]['memberUid'][$i] = array_values($_SESSION['groupDN'][$dn]['memberUid'][$i]);
						}
					}
				}
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}
	else { // Just modify, not recreate
		if ($attr) {
			// Change or add new attributes
			$success = ldap_modify($_SESSION['ldap']->server(),$values->general_dn, $attr);
			if (!$success) return 5;
			}
		if ($attr_rem) {
			// Remove old attributes which are no longer in use
			$success = ldap_mod_del($_SESSION['ldap']->server(),$values->general_dn, $attr_rem);
			if (!$success) return 5;
			}
		}

	// Write additional groupmemberchips
	// Get a list with all groups
	$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_GroupSuffix(), 'objectClass=PosixGroup', array('memberUid', 'cn'));
	$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
	while ($entry) {
		$modifygroup=0;
		$attr2 = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		// Remove "count" from array
		if (is_array($attr2['memberUid'])) array_shift($attr2['memberUid']);
		array_shift($attr2['cn']);
		if ($attr2['memberUid']) {
			// Remove user from groups he's not member anymore
			if (@in_array($values->general_username, $attr2['memberUid']) && !@in_array($attr2['cn'][0],$values->general_groupadd)) {
				$dn = ldap_get_dn($_SESSION['ldap']->server(), $entry);
				$success = ldap_mod_del($_SESSION['ldap']->server(), $dn ,array('memberUid' => $values->general_username));
				if (!$success) return 5;
				// Remove old memberUid to cache-array
				ldapreload('group');
				if ((isset($_SESSION['groupDN']))) {
					for ($i=0; $i<count($_SESSION['groupDN'][$dn]['memberUid']); $i++) {
						if ($values->general_username==$_SESSION['groupDN'][$dn]['memberUid'][$i]) {
							unset($_SESSION['groupDN'][$dn]['memberUid'][$i]);
							$_SESSION['groupDN'][$dn]['memberUid'] = array_values($_SESSION['groupDN'][$dn]['memberUid']);
							}
						}
					}
				}
			// Add user to groups
			if (!@in_array($values->general_username, $attr2['memberUid']) && @in_array($attr2['cn'][0],$values->general_groupadd)) {
				$dn = ldap_get_dn($_SESSION['ldap']->server(), $entry);
				$success = ldap_mod_add($_SESSION['ldap']->server(), $dn ,array('memberUid' => $values->general_username));
				if (!$success) return 5;
				// Add new memberUid to cache-array
				ldapreload('group');
				if ((isset($_SESSION['groupDN']))) {
					if (is_array($_SESSION['groupDN'][$dn]['memberUid'])) {
						if (!in_array($values->general_username, $_SESSION['groupDN'][$dn]['memberUid'])) $_SESSION['groupDN'][$dn]['memberUid'][] = $values->general_username;
						}
					else $_SESSION['groupDN'][$dn]['memberUid'][] = $values->general_username;
					}
				}
			}
		else {
			// Add user to groups
			if (@in_array($attr2['cn'][0],$values->general_groupadd)) {
				$dn = ldap_get_dn($_SESSION['ldap']->server(), $entry);
				$success = ldap_mod_add($_SESSION['ldap']->server(), $dn ,array('memberUid' => $values->general_username));
				if (!$success) return 5;
				// Add new memberUid to cache-array
				ldapreload('group');
				if ((isset($_SESSION['groupDN']))) {
					if (is_array($_SESSION['groupDN'][$dn]['memberUid'])) {
						if (!in_array($values->general_username, $_SESSION['groupDN'][$dn]['memberUid'])) $_SESSION['groupDN'][$dn]['memberUid'][] = $values->general_username;
						}
					else $_SESSION['groupDN'][$dn]['memberUid'][] = $values->general_username;
					}
				}
			}
		$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
		}

	// Change quotas if quotas are set and lamdaemon.pl should be used
	if ($_SESSION['config']->scriptServer && is_array($values->quota) && $uselamdaemon && ($values->quota != $values_old->quota))
		setquotas(array($values));
	//make required changes in cache-array
	if ((isset($_SESSION['userDN']))) {
		if ($values->general_dn != $values_old->general_dn) {
			unset ($_SESSION['userDN'][$values_old->general_dn]);
			}
		$_SESSION['userDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['userDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	// Return 3 if everything has worked fine
	return 3;
	}


/* This function will create a new host acconut in ldap
* $values is an account-object with all attributes of the host
* return-value is an integer
*       1 == Account has been created
*       2 == Account already exists at different location
*	4 == Error while creating Account
*/
function createhost($values) {
	// These Objectclasses are needed for an host account
	$attr['objectClass'][0] = 'posixAccount';
	$attr['objectClass'][1] = 'shadowAccount';
	$attr['objectClass'][2] = 'account';
	// Create DN for new host account
	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;
	// Attributes which are required
	$attr['cn'] = $values->general_username;
	$attr['uid'] = $values->general_username;
	$attr['uidNumber'] = $values->general_uidNumber;
	$attr['gidNumber'] = getgid($values->general_group);
	$attr['homeDirectory'] = "/dev/null";

	/* Write unix attributes into $attr array
	* Some values don't have to be set. These are only loaded if they are set
	*/
	$attr['loginShell'] = "/bin/false";
	$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos));
	$attr['description'] = utf8_encode($values->general_gecos);
	// Set unix password

	// Samba attributes
	if ($_SESSION['config']->is_samba3()) {
		// Add all attributes as samba3 objectclass
		$attr['objectClass'][3] = 'sambaSamAccount';
		// "Standard" password for new hosts
		$attr['sambaNTPassword'] = '0CB6948805F797BF2A82807973B89537';
		$attr['sambaLMPassword'] = '01FC5A6BE7BC6929AAD3B435B51404EE';
		$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
		$attr['sambaSID'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
		$attr['sambaPrimaryGroupSID'] = $values->smb_domain->SID . "-515"; // sambaAccount_req
		$attr['sambaPwdCanChange'] = time(); // sambaAccount_may
		$attr['sambaPwdMustChange'] = "1893452400"; // sambaAccount_may // anywhere in year 2030
		$attr['sambaAcctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
		$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
		}
	else {
		// Add all attributes as samba2.2 objectclass
		$attr['objectClass'][3] = 'sambaAccount';
		// "Standard" password for new hosts
		$attr['ntPassword'] = '0CB6948805F797BF2A82807973B89537';
		$attr['lmPassword'] = '01FC5A6BE7BC6929AAD3B435B51404EE';
		$attr['pwdLastSet'] = time(); // sambaAccount_may
		$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
		$attr['primaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
		$attr['pwdCanChange'] = time(); // sambaAccount_may
		$attr['pwdMustChange'] = "1893452400"; // sambaAccount_may // anywhere in 2030
		$attr['acctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
		if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
		}

	// Create LDAP user account
	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	// Continue if now error did ocour
	if (!$success) return 4;
	// Add new host to cache-array
	if ((isset($_SESSION['hostDN']))) {
		$_SESSION['hostDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['hostDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	return 1;
	}

/* This function will modify a host acconut in ldap
* $values and $values_old are an account-object with all
* attributes of the host.
* Only attributes which have changed will be written
* return-value is an integer
*       2 == Account already exists at different location
*       3 == Account has been modified
*	5 == Error while modifying Account
*/
function modifyhost($values,$values_old) {
	// Add missing objectclasses to host
	if (!in_array('posixAccount', $values->general_objectClass)) {
		$attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'posixAccount';
		}
	if (!in_array('shadowAccount', $values->general_objectClass)) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'shadowAccount';
		}
	// Create DN for new host account
	$values->general_dn = 'uid=' . $values->general_username . ',' . $values->general_dn;

	// Attributes which are required
	if ($values->general_username != $values_old->general_username) {
		$attr['cn'] = $values->general_username; // posixAccount_req shadowAccount_req sambaAccount_may
		$attr['uid'] = $values->general_username; // posixAccount_req
		}
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['uidNumber'] = $values->general_uidNumber;
		// Because sambaSid(rid) is related to uidNumber we have to change it if uidNumbaer has changed
		if ($_SESSION['config']->is_samba3())
			$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase);
		else $attr['rid'] = (2 * $values->general_uidNumber + 1000);
		}
	if ($values->general_group != $values_old->general_group) {
		$attr['gidNumber'] = getgid($values->general_group);
		// Because primaryGroup(S)ID is related to gidNumber we have to change it if gidNumber has changed
		if ($_SESSION['config']->is_samba3())
			// We use samba 3 schema
			$attr['sambaPrimaryGroupSID'] = $values->smb_domain->SID . "-".
				(2 * getgid($values->general_group) + $values->smb_domain->RIDbase+1);
		else
			// We use old samba 2.2 schema
			$attr['primaryGroupID'] = (2 * getgid($values->general_group) + 1001);
		}
	/* Write unix attributes into $attr array
	* Some values don't have to be set. These are only loaded if they are set
	*/
	if ($values->general_gecos != $values_old->general_gecos) {
		$attr['gecos'] = utf8_encode(replace_umlaut($values->general_gecos)); // posixAccount_may
		$attr['description'] = utf8_encode($values->general_gecos); // posixAccount_may sambaAccount_may
		}

	// Samba attributes
	if ($_SESSION['config']->is_samba3()) {
		if (!in_array('sambaSamAccount', $values->general_objectClass)) {
			// We have to convert sambaAccount Objectclass to sambaSamAccount objectclass
			if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
			$attr['objectClass'][] = 'sambaSamAccount';
			// unset old sambaAccount objectClass
			for ($i=0; $i<count($attr['objectClass']); $i++)
				if ($attr['objectClass'][$i]=='sambaAccount') unset($attr['objectClass'][$i]);
			$attr['objectClass'] = array_values($attr['objectClass']);
			// Set correct values for new objectclass
			// Load old samba-values not stored in account object
			$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
			$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if (isset($attr_old['lmPassword'][0])) $attr['sambaLMPassword'] = $attr_old['lmPassword'][0];
			if (isset($attr_old['ntPassword'][0])) $attr['sambaNTPassword'] = $attr_old['ntPassword'][0];
			if (isset($attr_old['pwdLastSet'][0])) $attr['sambaPwdLastSet'] = $attr_old['pwdLastSet'][0];
			if (isset($attr_old['logonTime'][0])) $attr['sambaLogonTime'] = $attr_old['logonTime'][0];
			if (isset($attr_old['logoffTime'][0])) $attr['sambaLogoffTime'] = $attr_old['logoffTime'][0];
			if (isset($attr_old['kickoffTime'][0])) $attr['sambaKickoffTime'] = $attr_old['kickoffTime'][0];
			if (isset($attr_old['pwdCanChange'][0])) $attr['sambaPwdCanChange'] = $attr_old['pwdCanChange'][0];
			if (isset($attr_old['pwdMustChange'][0])) $attr['sambaPwdMustChange'] = $attr_old['pwdMustChange'][0];
			if (isset($attr_old['smbHome'][0])) $attr['sambaHomePath'] = $attr_old['smbHome'][0];
			if (isset($attr_old['homeDrive'][0])) $attr['sambaHomeDrive'] = $attr_old['homeDrive'][0];
			if (isset($attr_old['scriptPath'][0])) $attr['sambaLogonScript'] = $attr_old['scriptPath'][0];
			if (isset($attr_old['profilePath'][0])) $attr['sambaProfilePath'] = $attr_old['profilePath'][0];
			if (isset($attr_old['userWorkstations'][0])) $attr['sambaUserWorkstations'] = $attr_old['userWorkstations'][0];
			// Values used from account object
			$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
			$attr['sambaAcctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
			$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
			$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase); // sambaAccount_may
			$attr['sambaPrimaryGroupSID'] = $values->smb_domain->SID . "-" . (2 * getgid($values->general_group) + $values->smb_domain->RIDbase +1); // sambaAccount_req
			// remove old attributes
			if (in_array('sambaAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaAccount';
			if (isset($attr_old['lmPassword'][0])) $attr_rem['lmPassword'] = $attr_old['lmPassword'][0];
			if (isset($attr_old['ntPassword'][0])) $attr_rem['ntPassword'] = $attr_old['ntPassword'][0];
			if (isset($attr_old['pwdLastSet'][0])) $attr_rem['pwdLastSet'] = $attr_old['pwdLastSet'][0];
			if (isset($attr_old['logonTime'][0])) $attr_rem['logonTime'] = $attr_old['logonTime'][0];
			if (isset($attr_old['kickoffTime'][0])) $attr_rem['kickoffTime'] = $attr_old['kickoffTime'][0];
			if (isset($attr_old['pwdCanChange'][0])) $attr_rem['pwdCanChange'] = $attr_old['pwdCanChange'][0];
			if (isset($attr_old['pwdMustChange'][0])) $attr_rem['pwdMustChange'] = $attr_old['pwdMustChange'][0];
			if (isset($attr_old['smbHome'][0])) $attr_rem['smbHome'] = $attr_old['smbHome'][0];
			if (isset($attr_old['acctFlags'][0])) $attr_rem['acctFlags'] = $attr_old['acctFlags'][0];
			if (isset($attr_old['homeDrive'][0])) $attr_rem['homeDrive'] = $attr_old['homeDrive'][0];
			if (isset($attr_old['scriptPath'][0])) $attr_rem['scriptPath'] = $attr_old['scriptPath'][0];
			if (isset($attr_old['profilePath'][0])) $attr_rem['profilePath'] = $attr_old['profilePath'][0];
			if (isset($attr_old['userWorkstations'][0])) $attr_rem['userWorkstations'] = $attr_old['userWorkstations'][0];
			if (isset($attr_old['primaryGroupID'][0])) $attr_rem['primaryGroupID'] = $attr_old['primaryGroupID'][0];
			if (isset($attr_old['domain'][0])) $attr_rem['domain'] = $attr_old['domain'][0];
			if (isset($attr_old['rid'][0])) $attr_rem['rid'] = $attr_old['rid'][0];
			}
		// Reset password
		if ($values->smb_flags['N']) {
			// "Standard" password for new hosts
			$attr['sambaNTPassword'] = '0CB6948805F797BF2A82807973B89537';
			$attr['sambaLMPassword'] = '01FC5A6BE7BC6929AAD3B435B51404EE';
			$attr['sambaPwdLastSet'] = time(); // sambaAccount_may
			}
		if ($values->smb_domain->name!=$values_old->smb_domain->name) {
			$attr['sambaDomainName'] = $values->smb_domain->name; // sambaAccount_may
			$attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase);
			$attr['sambaPrimaryGroupSID'] = $values->smb_domain->SID . "-".
				(2 * getgid($values->general_group) + $values->smb_domain->RIDbase+1);
			}
		}
	else {
		// use old samba 2.2 objectclass
		if (!in_array('sambaAccount', $values->general_objectClass)) {
			// Add or convert samba attributes & object to samba 2.2
			if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
			$attr['objectClass'][] = 'sambaAccount';
			// unset old sambaAccount objectClass
			for ($i=0; $i<count($attr['objectClass']); $i++)
				if ($attr['objectClass'][$i]=='sambaSamAccount') unset($attr['objectClass'][$i]);
			$attr['objectClass'] = array_values($attr['objectClass']);
			// Set correct values for new objectclass
			// Load old samba-values not stored in account object
			$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
			$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
			$return->general_dn = (ldap_get_dn($_SESSION['ldap']->server(), $entry));
			$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
			if (isset($attr_old['sambaLMPassword'][0])) $attr['lmPassword'] = $attr_old['sambaLMPassword'][0];
			if (isset($attr_old['sambaNTPassword'][0])) $attr['ntPassword'] = $attr_old['sambaNTPassword'][0];
			if (isset($attr_old['sambaPwdLastSet'][0])) $attr['pwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
			if (isset($attr_old['sambaLogonTime'][0])) $attr['logonTime'] = $attr_old['sambaLogonTime'][0];
			if (isset($attr_old['sambaLogoffTime'][0])) $attr['logoffTime'] = $attr_old['sambaLogoffTime'][0];
			if (isset($attr_old['sambaKickoffTime'][0])) $attr['kickoffTime'] = $attr_old['sambaKickoffTime'][0];
			if (isset($attr_old['sambaPwdCanChange'][0])) $attr['pwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
			if (isset($attr_old['sambaPwdMustChange'][0])) $attr['pwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
			if (isset($attr_old['sambaHomePath'][0])) $attr['smbHome'] = $attr_old['sambaHomePath'][0];
			if (isset($attr_old['sambaHomeDrive'][0])) $attr['homeDrive'] = $attr_old['sambaHomeDrive'][0];
			if (isset($attr_old['sambaLogonScript'][0])) $attr['scriptPath'] = $attr_old['sambaLogonScript'][0];
			if (isset($attr_old['sambaProfilePath'][0])) $attr['profilePath'] = $attr_old['sambaProfilePath'][0];
			if (isset($attr_old['sambaUserWorkstations'][0])) $attr['userWorkstations'] = $attr_old['sambaUserWorkstations'][0];
			// Values used from account object
			$attr['displayName'] = utf8_encode($values->smb_displayName); // sambaAccount_may
			$attr['acctFlags'] = smbflag($values->smb_flags); // sambaAccount_may
			if ($values->smb_domain!='') $attr['domain'] = $values->smb_domain; // sambaAccount_may
			$attr['primaryGroupID'] = (2 * getgid($values->general_group) + 1001); // sambaAccount_req
			$attr['rid'] = (2 * $values->general_uidNumber + 1000); // sambaAccount_may
			// remove old attributes
			if (in_array('sambaSamAccount', $attr_old['objectClass'])) $attr_rem['objectClass'] = 'sambaSamAccount';
			if (isset($attr_old['sambaLMPassword'][0])) $attr_rem['sambaLMPassword'] = $attr_old['sambaLMPassword'][0];
			if (isset($attr_old['sambaNTPassword'][0])) $attr_rem['sambaNTPassword'] = $attr_old['sambaNTPassword'][0];
			if (isset($attr_old['sambaPwdLastSet'][0])) $attr_rem['sambaPwdLastSet'] = $attr_old['sambaPwdLastSet'][0];
			if (isset($attr_old['sambaLogonTime'][0])) $attr_rem['sambaLogonTime'] = $attr_old['sambaLogonTime'][0];
			if (isset($attr_old['sambaKickoffTime'][0])) $attr_rem['sambaKickoffTime'] = $attr_old['sambaKickoffTime'][0];
			if (isset($attr_old['sambaPwdCanChange'][0])) $attr_rem['sambaPwdCanChange'] = $attr_old['sambaPwdCanChange'][0];
			if (isset($attr_old['sambaPwdMustChange'][0])) $attr_rem['sambaPwdMustChange'] = $attr_old['sambaPwdMustChange'][0];
			if (isset($attr_old['sambaHomePath'][0])) $attr_rem['sambaHomePath'] = $attr_old['sambaHomePAth'][0];
			if (isset($attr_old['sambaAcctFlags'][0])) $attr_rem['sambaAcctFlags'] = $attr_old['sambaAcctFlags'][0];
			if (isset($attr_old['sambaHomeDrive'][0])) $attr_rem['sambaHomeDrive'] = $attr_old['sambaHomeDrive'][0];
			if (isset($attr_old['sambaLogonScript'][0])) $attr_rem['sambaLogonScript'] = $attr_old['sambaLogonScript'][0];
			if (isset($attr_old['sambaProfilePath'][0])) $attr_rem['sambaProfilePath'] = $attr_old['sambaProfilePath'][0];
			if (isset($attr_old['sambaUserWorkstations'][0])) $attr_rem['sambaUserWorkstations'] = $attr_old['sambaUserWorkstations'][0];
			if (isset($attr_old['sambaPrimaryGroupID'][0])) $attr_rem['sambaPrimaryGroupID'] = $attr_old['sambaPrimaryGroupID'][0];
			if (isset($attr_old['sambaDomainName'][0])) $attr_rem['sambaDomainName'] = $attr_old['sambaDomainName'][0];
			if (isset($attr_old['sambaSID'][0])) $attr_rem['sambaSID'] = $attr_old['sambaSID'][0];
			}
		if ($values->smb_flags['N']) {
			// "Standard" password for new hosts
			$attr['ntPassword'] = '0CB6948805F797BF2A82807973B89537';
			$attr['lmPassword'] = '01FC5A6BE7BC6929AAD3B435B51404EE';
			$attr['pwdLastSet'] = time(); // sambaAccount_may
			}
		if (isset($attr_old['sambaSID'][0])) $attr_rem['sambaSID'] = $attr_old['sambaSID'][0];
		if (($values->smb_domain!='') && ($values->smb_domain!=$values_old->smb_domain)) $attr['domain'] = $values->smb_domain; // sambaAccount_may
		if (($values->smb_domain=='') && ($values->smb_domain!=$values_old->smb_domain)) $attr_rem['domain'] = $values_old->smb_domain; // sambaAccount_may
		}

	if ($values->general_dn != $values_old->general_dn) {
		// Account should be moved to a new location
		// Load old account
		$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixAccount");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		// remove "count" from array
		unset($attr_old['count']);
		for ($i=0; $i < sizeof($attr_old); $i++) unset($attr_old[$i]);
		$keys = array_keys($attr_old);
		for ($i=0; $i < sizeof($keys); $i++)
			unset($attr_old[$keys[$i]]['count']);
		// Change uid to new uid. Else ldap won't create the new entry
		//$attr_old['uid'][0] = $values->general_username;
		$attr_rem_keys = @array_keys($attr_rem);
		for ($i=0; $i<count($attr_rem_keys); $i++) { // Remove attributes which should be deleted from array
			if (isset($attr_old[$attr_rem_keys[$i]][0])) unset($attr_old[$attr_rem_keys[$i]]);
			}
		$attr_keys = @array_keys($attr);
		for ($i=0; $i<count($attr_keys); $i++) { // Add attributes which have changed
			$attr_old[$attr_keys[$i]][0] = $attr[$attr_keys[$i]];
			}
		// Create account at new location
		$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
		// remove old account
		if ($success) $success = ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
		if (!$success) return 5;
		}
	else { // Just modify, not recreate
		if ($attr) {
			// Change or add new attributes
			$success = ldap_modify($_SESSION['ldap']->server(),$values->general_dn, $attr);
			if (!$success) return 5;
			}
		if ($attr_rem) {
			// Remove old attributes which are no longer in use
			$success = ldap_mod_del($_SESSION['ldap']->server(),$values->general_dn, $attr_rem);
			if (!$success) return 5;
			}
		}

	//make required changes in cache-array
	if ((isset($_SESSION['hostDN']))) {
		if ($values->general_dn != $values_old->general_dn) {
			unset ($_SESSION['hostDN'][$values_old->general_dn]);
			}
		$_SESSION['hostDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['hostDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		}
	// Return 3 if everything has worked fine
	return 3;
	}


/* This function will create a new group acconut in ldap
* $values is an account-object with all attributes of the group
* if lamdaemon.pl is false no quotas are set. Usefull for massupload and deletion
* return-value is an integer
*       1 == Account has been created
*       2 == Account already exists at different location
*	4 == Error while creating Account
*/
function creategroup($values, $uselamdaemon=true) {
	// These Objectclasses are needed for an user account
	$attr['objectClass'][0] = 'posixGroup';
	// Create DN for new user account
	$values->general_dn = 'cn=' . $values->general_username . ',' . $values->general_dn;
	// Attributes which are required
	$attr['cn'] = $values->general_username;
	$attr['gidNumber'] = $values->general_uidNumber;

	/* Write unix attributes into $attr array
	* Some values don't have to be set. These are only loaded if they are set
	*/
	if ($values->general_gecos) $attr['description'] = utf8_encode($values->general_gecos);
	// Samba 3 attributes
	// $values->smb_mapgroup is not set if creategroup is called from masscreate.php
	if ($_SESSION['config']->is_samba3() && isset($values->smb_mapgroup)) {
		$attr['sambaSID'] = $values->smb_mapgroup;
		$attr['objectClass'][1] = 'sambaGroupMapping';
		$attr['sambaGroupType'] = '2';
		if ($values->smb_displayName) $attr['displayName'] = utf8_encode($values->smb_displayName);
		}
	// Write additional group members
	if (is_array($values->unix_memberUid)) $attr['memberUid'] = $values->unix_memberUid;
	// Create LDAP group account
	$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr);
	// Continue if now error did ocour
	if (!$success) return 4;
	// lamdaemon.pl should be used. Set quotas if quotas are used
	if ($_SESSION['config']->scriptServer && is_array($values->quota) && $uselamdaemon) setquotas(array($values));
	// Add new group to cache-array
	if ((isset($_SESSION['groupDN']))) {
		$_SESSION['groupDN'][$values->general_dn]['memberUid'] = $values->unix_memberUid;
		$_SESSION['groupDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['groupDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		if ($_SESSION['config']->is_samba3() && isset($values->smb_mapgroup))
			$_SESSION['groupDN'][$values->general_dn]['sambaSID'] = $values->smb_mapgroup;
		}
	return 1;
	}


/* This function will modify a group acconut in ldap
* $values and $values_old are an account-object with all
* attributes of the group.
* if lamdaemon.pl is false no quotas are set. Usefull for massupload and deletion
* Only attributes which have changed will be written
* return-value is an integer
*       2 == Account already exists at different location
*       3 == Account has been modified
*	5 == Error while modifying Account
*/
function modifygroup($values,$values_old, $uselamdaemon=true) {
	// Add missing objectclasses to group
	if (!in_array('posixGroup', $values->general_objectClass)) {
		$attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'posixGroup';
		}
	if (($_SESSION['config']->is_samba3()) && (!in_array('sambaGroupMapping', $values->general_objectClass))) {
		if (!isset($attr['objectClass'])) $attr['objectClass'] = $values->general_objectClass;
		$attr['objectClass'][] = 'sambaGroupMapping';
		$attr['sambaGroupType'] = '2';
		}
	// Create DN for new group account
	$values->general_dn = 'cn=' . $values->general_username . ',' . $values->general_dn;

	// Attributes which are required
	if ($values->general_username != $values_old->general_username) $attr['cn'] = $values->general_username;
	if ($values->general_uidNumber != $values_old->general_uidNumber) {
		$attr['gidNumber'] = $values->general_uidNumber;
		// Set correct SID if UID was changed
		if ($_SESSION['config']->is_samba3()) $attr['sambaSid'] = $values->smb_domain->SID . "-" . (2 * $values->general_uidNumber + $values->smb_domain->RIDbase +1);
		}
	if ($values->general_gecos != $values_old->general_gecos) $attr['description'] = utf8_encode($values->general_gecos);
	if ($values->smb_displayName != $values_old->smb_displayName)
		$attr['displayName'] = utf8_encode($values->smb_displayName);
	// Samba 3.0 attributes
	if ($_SESSION['config']->is_samba3()) {
		if ($values->smb_mapgroup != $values_old->smb_mapgroup)
			$attr['sambaSID'] = $values->smb_mapgroup;
		}
	// Write Additional group members
	if (($values->unix_memberUid != $values_old->unix_memberUid)) {
		if (count($values->unix_memberUid)==0) $attr_rem['memberUid'] = $values_old->unix_memberUid;
		 else $attr['memberUid'] = $values->unix_memberUid;
		}
	// Rewrite quotas if uidnumbers has changed
	if ($values->general_uidNumber != $values_old->general_uidNumber && $_SESSION['config']->scriptServer) {
		// Remove old quotas
		remquotas(array($values_old->general_username), "user");
		// Remove quotas from $values_old because we have to rewrite them all
		unset ($values_old->quota);
		}

	if ($values->general_dn != $values_old->general_dn) {
		// Account should be moved to a new location
		// Load old account
		$result = ldap_search($_SESSION['ldap']->server(), $values_old->general_dn, "objectclass=PosixGroup");
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$attr_old = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		// remove "count" from array
		unset($attr_old['count']);
		for ($i=0; $i < sizeof($attr_old); $i++) unset($attr_old[$i]);
		$keys = @array_keys($attr_old);
		for ($i=0; $i < sizeof($keys); $i++)
			unset($attr_old[$keys[$i]]['count']);
		// Change cn to new cn. Else ldap won't create the new entry
		//$attr_old['cn'][0] = $values->general_username;
		$attr_rem_keys = @array_keys($attr_rem);
		for ($i=0; $i<count($attr_rem_keys); $i++) { // Remove attributes which should be deleted from array
			if (isset($attr_old[$attr_rem_keys[$i]][0])) unset($attr_old[$attr_rem_keys[$i]]);
			}
		$attr_keys = @array_keys($attr);
		for ($i=0; $i<count($attr_keys); $i++) { // Add attributes which have changed
			$attr_old[$attr_keys[$i]][0] = $attr[$attr_keys[$i]];
			}
		// Create account at new location
		$success = ldap_add($_SESSION['ldap']->server(),$values->general_dn, $attr_old);
		// remove old account
		if ($success) $success = ldap_delete($_SESSION['ldap']->server(),$values_old->general_dn);
		if (!$success) return 5;
		}
	else { // Just modify, not recreate
		if ($attr) {
			// Change or add new attributes
			$success = ldap_modify($_SESSION['ldap']->server(),$values->general_dn, $attr);
			if (!$success) return 5;
			}
		if ($attr_rem) {
			// Remove old attributes which are no longer in use
			$success = ldap_mod_del($_SESSION['ldap']->server(),$values->general_dn, $attr_rem);
			if (!$success) return 5;
			}
		}


	// Chnage GIDs of all users which are member of group
	if ( $_SESSION['final_changegids']==true ) {
		$result = ldap_search($_SESSION['ldap']->server(), $_SESSION['config']->get_UserSuffix(), 'gidNumber=' . $values_old->general_uidNumber, array(''));
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		while ($entry) {
			$user['gidNumber'][0] = $values->general_uidNumber;
			$success =ldap_modify($_SESSION['ldap']->server(), ldap_get_dn($_SESSION['ldap']->server(), $entry), $user);
			if (!$success) return 5;
			$entry = ldap_next_entry($_SESSION['ldap']->server(), $entry);
			}
		}

	// Change quotas if quotas are set and lamdaemon.pl should be used
	if ($_SESSION['config']->scriptServer && is_array($values->quota) && $uselamdaemon && ($values->quota != $values_old->quota))
		setquotas(array($values));
	//make required changes in cache-array
	if ((isset($_SESSION['groupDN']))) {
		if ($values->general_dn != $values_old->general_dn) {
			unset ($_SESSION['groupDN'][$values_old->general_dn]);
			}
		if (is_array($values->unix_memberUid)) $_SESSION['groupDN'][$values->general_dn]['memberUid'] = $values->unix_memberUid;
		$_SESSION['groupDN'][$values->general_dn]['cn'] = $values->general_username;
		$_SESSION['groupDN'][$values->general_dn]['uidNumber'] = $values->general_uidNumber;
		if ($values->smb_mapgroup != $values_old->smb_mapgroup)
			$_SESSION['groupDN'][$values->general_dn]['sambaSID'] = $values->smb_mapgroup;
		}
	// Return 3 if everything has worked fine
	return 3;
	}


?>
