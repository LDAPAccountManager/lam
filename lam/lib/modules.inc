<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2004  Roland Gruber

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


	Interface between modules and other parts of LAM.

*/

include_once("cache.inc");
include_once("account.inc");
/** parent class of account modules */
include_once("baseModule.inc");

/* We have to include all modules
* before start session
*/
// Get Lampath, needed 4 includes
$stay=0;
$relative='';
while ($stay<7) {
	if (is_dir($relative.'lib/modules')) {
		$dir = opendir($relative.'lib/modules');
		while ($entry = readdir($dir))
			if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($relative.'lib/modules/'.$entry)) include_once ($relative.'lib/modules/'.$entry);
		$stay=10;
		}
	else {
		$stay++;
		$relative .= '../';
		}
	}

/**
* Returns the alias name of a module
*
* @param string $name the module name
* @param string $scope the account type ("user", "group", "host")
* @return string alias name
*/
function getModuleAlias($name, $scope) {
	$module = new $name($scope);
	return $module->get_alias();
}

// returns if the module is a base module
// $name: the module name
// $scope: the account type ("user", "group", "host")
function is_base_module($name, $scope) {
	$module = new $name($scope);
	return $module->is_base_module();
}

// returns a LDAP filter used by the account lists
// return value is of type "(...)"
// $scope: the account type ("user", "group", "host")
function get_ldap_filter($scope) {
	$mods = getAvailableModules($scope);
	$filters = array();
	$orFilter = '';
	for ($i = 0; $i < sizeof($mods); $i++) {
		if (is_base_module($mods[$i], $scope)) {
			$module = new $mods[$i]($scope);
			$modinfo = $module->get_ldap_filter();
			if (isset($modinfo['or'])) $filters['or'][] = $modinfo['or'];
			if (isset($modinfo['and'])) $filters['and'][] = $modinfo['and'];
		}
	}
	// build OR filter
	if (sizeof($filters['or']) == 1) {
		$orFilter = $filters['or'][0];
	}
	elseif (sizeof($filters['or']) > 1) {
		$orFilter = "(|" . implode("", $filters['or']) . ")";
	}
	// add built OR filter to AND filters
	if ($orFilter != '') $filters['and'][] = $orFilter;
	// collapse AND filters
	if (sizeof($filters['and']) < 2) return $filters['and'][0];
	else return "(&" . implode("", $filters['and']) . ")";
}

// returns a hash array (module name => dependencies) of all user module dependencies
// dependencies contains an array with two sub arrays: depends, conflicts
// the elements of depends are either module names or an array of module names (OR-case)
// the elements of conflicts are module names
// $scope: user, group, host, ....
function getModulesDependencies($scope) {
	$mods = getAvailableModules($scope);
	$deps = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = new $mods[$i]($scope);
		$return[$mods[$i]] = $module->get_dependencies();
	}
	return $return;
	}


// checks if there are missing dependencies between modules
// $selected is an array of selected module names
// $deps is an array of module dependencies
// returns false if no misssing dependency was found
// returns an array of array(selected module, depending module) if missing dependencies were found
function check_module_depends($selected, $deps) {
	$ret = array();
	for ($m = 0; $m < sizeof($selected); $m++) {  // check selected modules
		for ($i = 0; $i < sizeof($deps[$selected[$m]]['depends']); $i++) {  // check dependencies of module
			// check if we have OR-combined modules
			if (is_array($deps[$selected[$m]]['depends'][$i])) {
				// one of the elements is needed
				$found = false;
				$depends = $deps[$selected[$m]]['depends'][$i];
				for ($d = 0; $d < sizeof($depends); $d++) {
					if (in_array($depends[$d], $selected)) {
						$found = true;
						break;
					}
				}
				if (! $found) {
					// missing dependency, add to return value
					$ret[] = array($selected[$m], implode(" || ", $depends));
				}
			}
			else {
				// single dependency
				if (! in_array($deps[$selected[$m]]['depends'][$i], $selected)) {
					// missing dependency, add to return value
					$ret[] = array($selected[$m], $deps[$selected[$m]]['depends'][$i]);
				}
			}
		}
	}
	if (sizeof($ret) > 0) return $ret;
	else return false;
}

// checks if there are conflicts between modules
// $selected is an array of selected module names
// $deps is an array of module dependencies
// returns false if no conflict was found
// returns an array of array(selected module, conflicting module) if conflicts were found
function check_module_conflicts($selected, $deps) {
	$ret = array();
	for ($m = 0; $m < sizeof($selected); $m++) {
		for ($i = 0; $i < sizeof($deps[$selected[$m]]['conflicts']); $i++) {
			if (in_array($deps[$selected[$m]]['conflicts'][$i], $selected)) {
				$ret[] = array($selected[$m], $deps[$selected[$m]]['conflicts'][$i]);
			}
		}
	}
	if (sizeof($ret) > 0) return $ret;
	else return false;
}

/**
* Returns an array with all available user module names
*
* @param string $scope account type (user, group, host)
* @return array list of possible modules
*/
function getAvailableModules($scope) {
	global $relative;
	$return = array();
	// get module names.
	$dir = opendir($relative . 'lib/modules');
	while ($entry = readdir($dir))
		if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($relative . 'lib/modules/'.$entry)) {
			$entry = substr($entry, 0, strpos($entry, '.'));
			$temp = new $entry($scope);
			if ($temp->can_manage()) $return[] = $entry;
		}
	return $return;
}

// $scope = user, group, host, ...
function getProfileOptions($scope) {
	// create new account container if needed
	if (! isset($_SESSION["profile_account_$scope"])) {
		$_SESSION["profile_account_$scope"] = new accountContainer($scope, "profile_account_$scope");
		$_SESSION["profile_account_$scope"]->new_account();
	}
	// get options
	return $_SESSION["profile_account_$scope"]->getProfileOptions();
}

// checks if the profile options are valid
// $scope: user, group, host, ...
// $options: an hash array containing all options (name => array(...))
function checkProfileOptions($scope, $options) {
	$return = array();
	$modules = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($modules); $i++) {
			$errors = call_user_func(array($modules[$i], "check_profileOptions"), $scope, $options);
			$return = array_merge($return, $errors);
	}
	return $return;
}

// get the single help entry array for help identifier $helpID from module $module
function getHelp($module,$helpID) {
	return call_user_func(array($module, "get_help"), $helpID);
}

// $scope = user, group, host, ...
function getAvailablePDFFields($scope) {
	// create new account container if needed
	if (! isset($_SESSION["profile_account_$scope"])) {
		$_SESSION["profile_account_$scope"] = new accountContainer($scope, "profile_account_$scope");
		$_SESSION["profile_account_$scope"]->new_account();
	}
	// get options
	return $_SESSION["profile_account_$scope"]->getAvailablePDFFields();
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
		$allowed_types = array ( 'user', 'group', 'host', 'domain' );
		if (!in_array($type, $allowed_types)) trigger_error(_('Account type not recognized.'), E_USER_ERROR);
		$this->type = $type;
		$this->base = $base;
		// Name of variables in session
		$this->ldap = 'ldap';
		$this->config = 'config';
		$this->cache = 'cache';
		$this->header2 = 'header';
		$this->module['main'] = new main($this->type);
		$this->module['main']->init($this->base);
		// create cache if needed
		if (!isset($_SESSION['cache'])) {
			$_SESSION['cache'] = new cache();
		}
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
		if ($post['form_main_reset']) {
			$this->load_account($this->dn_orig);
			}
		else {
			$function = '$result = $this->module[$this->order[$this->module[\'main\']->current_page]]->proccess_'.$this->module['main']->subpage.'($post);';
			eval ($function);
			}
		if (is_string($result)) $this->module['main']->subpage = $result;
		if (is_int($result))
			for ($i=0; $i<count($this->order); $i++ )
				if ($post['form_main_'.$this->order[$i]]) {
					$this->module['main']->current_page = $i;
					$this->module['main']->subpage='attributes';
					}
		// Write HTML-Code
		echo $_SESSION[$this->header2];
		echo "<title>";
		if ($this->dn_orig!='') echo _("Modify Account");
			else echo _("Create new Account");
		echo "</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";
		echo "<form action=\"edit.php\" method=\"post\">\n";
		// Display errir-messages
		if (is_array($result))
			foreach ($result as $result2)
				if (is_array($result2))
					for ($i=0; $i<sizeof($result2); $i++) StatusMessage($result2[$i][0], $result2[$i][1], $result2[$i][2]);
		// Create left module-menu
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr>";
		echo "<td><fieldset class=\"".$this->type."edit-dark\"><legend class=\"".$this->type."edit-bright\"><b>";
			echo _('Please select page:');
			echo "</b></legend>\n";
		// Loop for module
		for ($i=0; $i<count($this->order); $i++ ) {
			if ($this->order[$i]==$this->order[$this->module['main']->current_page] || !$this->module[$this->order[$i]]->module_ready() ) {
				// print disabled button
				echo "<input name=\"form_main_".$this->order[$i]."\" type=\"submit\" value=\"";
				echo $this->module[$this->order[$i]]->get_alias($type);
				echo "\" disabled>\n<br>";
				}
			else {
				// print normal button
				echo "<input name=\"form_main_".$this->order[$i]."\" type=\"submit\" value=\"";
				echo $this->module[$this->order[$i]]->get_alias($type);
				echo "\">\n<br>";
				}
			}
		if ($this->dn_orig!='') echo "<input name=\"form_main_reset\" type=\"submit\" value=\"" . _('Reset changes') . "\"><br>\n";
		echo "</fieldset></td></tr>\n";
		echo "</table></td>\n<td>";
		echo "<td><fieldset class=\"".$this->type."edit-dark\"><legend class=\"".$this->type."edit-bright\"><b>";
			echo $this->module[$this->order[$this->module['main']->current_page]]->get_alias($type);
			echo "</b></legend>\n";
		// display html-code from mdule
		$function = '$result = $this->module[$this->order[$this->module[\'main\']->current_page]]->display_html_'.$this->module['main']->subpage.'($post);';
		eval ($function);
		$this->parse_html($this->order[$this->module['main']->current_page], $result);
		// Display rest of html-page
		echo "</fieldset>\n";
		echo "</td></tr></table>\n";
		echo "</form>\n";
		echo "</body>\n";
		echo "</html>\n";
		return 0;
		}

	function parse_html($module, $input) {
		if (is_array($input)) {
			echo "<table>\n";
			for ($i=0; $i<count($input); $i++) {
				// Draw column
				echo "<tr>\n";
				for ($j=0; $j<count($input[$i]); $j++ ) {
					// draw raw
					switch ($input[$i][$j]['kind']) {
							case 'text':
								echo "<td";
								if ($input[$i][$j]['td']['valign']!='') echo ' valign="' . $input[$i][$j]['td']['valign'] .'"';
								echo ">\n";
								echo $input[$i][$j]['text'] . "</td>\n";
								break;
							case 'input':
								echo "<td";
								if ($input[$i][$j]['td']['valign']!='') echo ' valign="' . $input[$i][$j]['td']['valign'] .'"';
								echo ">\n";
								$output = "<input";
								if ($input[$i][$j]['name']!='') $output .= ' name="' . $input[$i][$j]['name'] . '"';
								if ($input[$i][$j]['type']!='') $output .= ' type="' . $input[$i][$j]['type'] . '"';
								if ($input[$i][$j]['size']!='') $output .= ' size="' . $input[$i][$j]['size'] . '"';
								if ($input[$i][$j]['maxlength']!='') $output .= ' maxlength="' . $input[$i][$j]['maxlength'] . '"';
								if ($input[$i][$j]['value']!='') $output .= ' value="' . $input[$i][$j]['value'] . '"';
								if ($input[$i][$j]['disabled']) $output .= ' disabled';
								if ($input[$i][$j]['checked']) $output .= ' checked';
								$output .= "></td>\n";
								echo $output;
								break;
							case 'fieldset':
								echo "<td";
								if ($input[$i][$j]['td']['valign']!='') echo ' valign="' . $input[$i][$j]['td']['valign'] .'"';
								echo ">\n";
								echo "<fieldset>\n";
								if ($input[$i][$j]['legend']!='') echo "<legend>" . $input[$i][$j]['legend'] . "</legend>\n";
								$this->parse_html($module, $input[$i][$j]['value']);
								echo "</fieldset>\n";
								break;
							case 'select':
								if (!is_array($input[$i][$j]['options'])) $input[$i][$j]['options'] = array ( $input[$i][$j]['options'] );
								if (!is_array($input[$i][$j]['options_selected'])) $input[$i][$j]['options_selected'] = array ( $input[$i][$j]['options_selected'] );
								echo "<td";
								if ($input[$i][$j]['td']['valign']!='') echo ' valign="' . $input[$i][$j]['td']['valign'] .'"';
								echo ">\n";
								echo "<select name=\"" . $input[$i][$j]['name'] . '"';
								if ($input[$i][$j]['multiple']) echo ' multiple';
								if ($input[$i][$j]['size']) echo ' size="' . $input[$i][$j]['size'] . '"';
								echo ">\n";
								// merge both option arrays and sort them.
								$options = array_merge ($input[$i][$j]['options'], $input[$i][$j]['options_selected'] );
								$options = array_unique($options);
								sort($options, SORT_NUMERIC);
								foreach ($options as $option) {
									if ($option!='') {
										if (in_array($option, $input[$i][$j]['options_selected'])) echo "<option selected>" . $option . "</option>\n";
											else echo "<option>" . $option . "</option>\n";
										}
									}
								echo "</select></td>\n";
								break;
							case 'table':
								echo "<td";
								if ($input[$i][$j]['td']['valign']!='') echo ' valign="' . $input[$i][$j]['td']['valign'] .'"';
								echo ">\n";
								$this->parse_html($module, $input[$i][$j]['value']);
								echo "</td>\n";
								break;
							case 'help':
								echo "<td";
								if ($input[$i][$j]['td']['valign']!='') echo ' valign="' . $input[$i][$j]['td']['valign'] .'"';
								echo ">\n";
								echo "<a href=../help.php?module=$module&item=". $input[$i][$j]['value'] . ">" . _('Help') . "</a></td>\n";
								break;
							case 'message':
								echo "<td";
								if ($input[$i][$j]['td']['valign']!='') echo ' valign="' . $input[$i][$j]['td']['valign'] .'"';
								echo ">\n";
								StatusMessage($input[$i][$j]['type'], $input[$i][$j]['headline'], $input[$i][$j]['text']);
								echo "</td>\n";
								break;
							default:
								echo "<td>Unrecognized type: " . $input[$i][$j]['kind'] . "</td>\n";
								break;
						}
					}
				echo "</tr>\n";
				}
			}
		echo "</table>\n";
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

		
	/* This function return ldap attributes which are uses by $objectClass
	* Syntax is get_attributes($objectClass)
	* Return is an array with all allowed attributes
	*/
	function get_module_attributes($objectClass) {
		// Add account type to object
		$line=-1;
		for ($i=0; $i<count($_SESSION[$this->ldap]->objectClasses) || $i==-1; $i++) {
			if (strpos($_SESSION[$this->ldap]->objectClasses[$i], "NAME '$objectClass'")) $line = $i;
			}
		// Return error if objectClass isn't found
		if ($line==-1) trigger_error (sprintf(_("ObjectClass %s required but not defined in ldap."), $objectClass), E_USER_WARNING);
		if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')) {
			$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')+6);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			// Ad must
			foreach (explode(" $ ", $string) as $attribute) {
				$return[$attribute] = '';
				}
			}
		// create array with may-attributes
		// Get startposition in string
		if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')) {
			$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')+5);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			// Ad may
			foreach (explode(" $ ", $string) as $attribute) {
				$return[$attribute] = '';
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
			if ($line==-1) trigger_error (sprintf(_("ObjectClass %s required but not defined in ldap."), $subclass), E_USER_WARNING);
			// create array with must-attributes
			// Get startposition in string
			if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')) {
				$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MUST (')+6);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				// Ad must
				foreach (explode(" $ ", $string) as $attribute) {
					$return[$attribute] = '';
					}
				}
			// create array with may-attributes
			// Get startposition in string
			if (strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')) {
				$string_withtail = substr($_SESSION[$this->ldap]->objectClasses[$line], strpos($_SESSION[$this->ldap]->objectClasses[$line],  'MAY (')+5);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				// Ad may
				foreach (explode(" $ ", $string) as $attribute) {
					$return[$attribute] = '';
					}
				}
			}
		$this->add_attributes($objectClass);
		return $return;
		}

	/* This function return ldap attributes which are uses by $objectClass
	* Syntax is get_attributes($attributes, $orig)
	* Return is an array as needed for $this->saveAccount()
	*/
	function save_module_attributes($attributes, $orig) {
		// Get list of all "easy" attributes
		$attr_names = array_keys($attributes);
		// Get attributes which should be added
		for ($i=0; $i<count($attr_names); $i++) {
			for ($j=0; $j<count($orig[$attr_names[$i]]); $j++) {
				if (is_array($attributes[$attr_names[$i]])) {
					if (!in_array($orig[$attr_names[$i]][$j], $attributes[$attr_names[$i]]))
						if ($orig[$attr_names[$i]][$j]!='') $torem[$attr_names[$i]][] =utf8_encode($orig[$attr_names[$i]][$j]);
					}
				else if ($orig[$attr_names[$i]][$j]!='') $torem[$attr_names[$i]][] = utf8_encode($orig[$attr_names[$i]][$j]);
				}
			for ($j=0; $j<count($attributes[$attr_names[$i]]); $j++) {
				if (is_array($orig[$attr_names[$i]])) {
					if (!in_array($attributes[$attr_names[$i]][$j], $orig[$attr_names[$i]]))
						if ($attributes[$attr_names[$i]][$j]!='') $toadd[$attr_names[$i]][] = utf8_encode($attributes[$attr_names[$i]][$j]);
					}
				else if ($attributes[$attr_names[$i]][$j]!='') $toadd[$attr_names[$i]][] = utf8_encode($attributes[$attr_names[$i]][$j]);
				}
			for ($j=0; $j<count($attributes[$attr_names[$i]]); $j++) {
				if (is_array($orig[$attr_names[$i]]) && is_array($attributes[$attr_names[$i]])) {
					if (($attributes[$attr_names[$i]][$j]==$orig[$attr_names[$i]][$j]) && $attributes[$attr_names[$i]][$j]!='')
						$notchanged[$attr_names[$i]][] = utf8_encode($attributes[$attr_names[$i]][$j]);
					}
				}
			}
		// create modify wuth add and remove
		if (is_array($toadd)) {
			$attributes2 = array_keys($toadd);
			for ($i=0; $i<count($attributes2); $i++) {
				if (isset($torem[$attributes2[$i]])) {
					// found modify entry
					// Add unchanged attributes
					if (isset($notchanged[$attributes2[$i]])) $tomodify[$attributes[$i]] = $notchanged[$attributes[$i]];
					$tomodify[$attributes2[$i]] = array_merge_recursive($tomodify[$attributes2[$i]], $toadd[$attributes2[$i]]);
						// unset attributes
					if (isset($notchanged[$attributes2[$i]])) unset($notchanged[$attributes2[$i]]);
					if (isset($toadd[$attributes2[$i]])) unset($toadd[$attributes2[$i]]);
					if (isset($torem[$attributes2[$i]])) unset($torem[$attributes2[$i]]);
					}
				}
			}
		if (count($toadd)!=0) $return[$this->dn]['add'] = $toadd;
		if (count($torem)!=0) $return[$this->dn]['remove'] = $torem;
		if (count($tomodify)!=0) $return[$this->dn]['modify'] = $tomodify;
		if (count($notchanged)!=0) $return[$this->dn]['notchanged'] = $notchanged;
		return $return;
		}


	/* This function checks if all MUST-attribtues are set.
	* If not it will return an array with all modules
	* which have to be set first
	*/
	function check_attributes() {
		$return = array();
		if (is_array($this->attributes)) {
			// get named list of attributes
			$attributes = array_keys($this->attributes);
			for ($i=0; $i<count($attributes); $i++) {
				$singleattribute = array_keys ($this->attributes[$attributes[$i]]);
				for ($j=0; $j<count($singleattribute); $j++) {
					// found attribute which must be set
					if ($this->attributes[$attributes[$i]][$singleattribute[$j]]=='MUST') {
						// Check if attribute is set
						if ($this->module[$singleattribute[$j]]->attributes[$attributes[$i]]=='') {
							if (!in_array($singleattribute[$j], $return)) $return[] = $singleattribute[$j];
							}
						}
					}
				}
			return $return;
			}
		}

	/* This function will load an account.
	* $dn is the dn of the account which should be loaded
	*/
	function load_account($dn) {
		$function = '$modules = $_SESSION[$this->config]->get_'.ucfirst($this->type).'Modules();';
		eval ($function);
		$search = substr($dn, 0, strpos($dn, ','));
		$result = ldap_search($_SESSION[$this->ldap]->server(), $dn, $search);
		$entry = ldap_first_entry($_SESSION[$this->ldap]->server(), $result);
		$this->dn = substr($dn, strpos($dn, ',')+1);
		$this->dn_orig = $dn;
		$attr = ldap_get_attributes($_SESSION[$this->ldap]->server(), $entry);
				
		foreach ($modules as $module) {
			if (!isset($this->module[$module])) {
				$this->module[$module] = new $module($this->type);
				$this->module[$module]->init($this->base);
			}
			$this->module[$module]->load_attributes($attr);
			}

		// sortm modules and make all active because all required attributes should be set
		$module = array_keys ($this->module);
		$modulelist = array();
		// loop until all modules are in order.
		// We don't want to loop forever
		$remain = count($module) * count($module);
		$order = array();
		while ( (count($module) != count($modulelist)) && ($remain!=0) ) {
			$remain--;
			foreach ($module as $moduleitem) {
				$required = $this->module[$moduleitem]->get_dependencies($this->type);
				$everything_found = true;
				if (is_array($required['require'])) {
					foreach ($required['require'] as $requireditem)
						if (!in_array($reuquireditem, $modulelist)) $everthing_found = false;
					}
				if ($everything_found && !in_array($moduleitem, $order) ) $order[] = $moduleitem;
				}
			}
		// Write Module-Order in variable
		$this->order = $order;
		return 0;
		}

	/**
	* Returns an hash array containing all profile options
	*/
	function getProfileOptions() {
		$return = array();
		$modules = array_keys($this->module);
		foreach ($modules as $singlemodule) {
			$return[$singlemodule] = $this->module[$singlemodule]->get_profileOptions();
		}
		return $return;
	}

	// TODO remove this function?
	function proccess_profile($post) {
		$return = array();
		$module = array_keys ($this->module);
		foreach ($module as $singlemodule) {
			// get list of display functions.
			$list = $this->module[$singlemodule]->pages();
			foreach ($list as $item) {
				$function = 'display_html_' . $item;
				$page = $this->module[$singlemodule]->$function($post,true);
				//eval($function);
				$return = array_merge($return, $page);
				}
			}
		return $return;
		}

	/* This function will prepare the object
	* for a new account
	*/
	function new_account() {
		$temp = ucfirst($this->type);
		$modules = call_user_func(array(&$_SESSION['config'], 'get_' . $temp . 'Modules'));
		foreach ($modules as $module) {
			$this->module[$module] = new $module($this->type);
			$this->module[$module]->init($this->base);
		}

		$module = array_keys ($this->module);
		$modulelist = array();
		// loop until all modules are in order.
		// We don't want to loop forever
		$remain = count($module) * count($module);
		$order = array();
		while ( (count($module) != count($modulelist)) && ($remain!=0) ) {
			$remain--;
			foreach ($module as $moduleitem) {
				$required = $this->module[$moduleitem]->get_dependencies($this->type);
				$everything_found = true;
				if (is_array($required['require'])) {
					foreach ($required['require'] as $requireditem)
						if (!in_array($reuquireditem, $modulelist)) $everthing_found = false;
					}
				if ($everything_found && !in_array($moduleitem, $order) ) $order[] = $moduleitem;
				}
			}
		// Write Module-Order in variable
		$this->order = $order;
		// *** fixme load*Profile must return array in the same way ldap_get_attributes does.
		$function = '$newattributes = load'.ucfirst($this->type).'Profile(\'default\');';
		//eval($function);
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
			// *** fixme don't include references
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
		print_r($attributes);
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
		// Set to true if an real error has happened
		$stopprocessing = false;
		// Add new DN
		if (isset($attributes[$DNs[$i]]['errors'])) {
			foreach ($attributes[$DNs[$i]]['errors'] as $singleerror) {
				$errors[] = $singleerror;
				if ($singleerror[0] = 'ERROR') $stopprocessing = true;
				}
			}
		// fixme *** ad update_cache after every ldap-change

		if (!$stopprocessing) {
			if ($this->dn != $this->dn_orig) {
				// move existing DN
				if ($this->dn_orig!='') {
					// merge attributes together
					$attr = array_merge_recursive($attributes[$this->dn]['add'], $attributes[$this->dn]['notchanged'], $attributes[$this->dn]['modify']);
					$success = @ldap_add($_SESSION[$this->ldap]->server(), $this->dn, $attr);
					if ($success) {
						$_SESSION[$this->cache]->update_cache($this->$dn, 'add', $attr);
						$success = @ldap_delete($_SESSION[$this->ldap]->server(), $this->dn_orig);
						if (!$success) {
							$errors[] = array('ERROR', 'LDAP', sprintf(_('Was unable to delete dn: %s.'), $this->dn_orig));
							$stopprocessing = true;
							}
						if ($success)
							$_SESSION[$this->cache]->update_cache($this->$dn, 'delete_dn');
						}
					if (!$success) {
						$errors[] = array('ERROR', 'LDAP', sprintf(_('Was unable to create dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $this->dn));
						$stopprocessing = true;
						}
					}
				// create complete new dn
				else {
					$attr = array_merge_recursive($attributes[$this->dn]['add'], $attributes[$this->dn]['notchanged'], $attributes[$this->dn]['modify']);
					$success = @ldap_add($_SESSION[$this->ldap]->server(), $this->dn, $attributes[$this->dn]['add']);
					if (!$success) {
						$errors[] = array('ERROR', 'LDAP', sprintf(_('Was unable to create dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $this->dn));
						$stopprocessing = true;
						}
					else
						$_SESSION[$this->cache]->update_cache($this->$dn, 'add', $attributes[$this->dn]['add']);
					}
				unset($attributes[$this->dn]);
				}
			}
		$DNs = array_keys($attributes);
		for ($i=0; $i<count($DNs); $i++) {
			if (!$stopprocessing) {
				// modify attributes
				if (isset($attributes[$DNs[$i]]['modify']) && !$stopprocessing) {
					$success = @ldap_mod_replace($_SESSION[$this->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['modify']);
					if (!$success) {
						$errors[] = array('ERROR', 'LDAP', sprintf(_('Was unable to modify attribtues from dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
						$stopprocessing = true;
						}
					else
						$_SESSION[$this->cache]->update_cache($this->$dn, 'modify', $attributes[$this->dn]['modify']);
					}
				// add attributes
				if (isset($attributes[$DNs[$i]]['add']) && !$stopprocessing) {
					$success = @ldap_mod_add($_SESSION[$this->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['add']);
					if (!$success) {
						$errors[] = array('ERROR', 'LDAP', sprintf(_('Was unable to add attribtues to dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
						$stopprocessing = true;
						}
					else
						$_SESSION[$this->cache]->update_cache($this->$dn, 'add', $attributes[$this->dn]['add']);
					}
				// removce attributes
				if (isset($attributes[$DNs[$i]]['remove']) && !$stopprocessing) {
					$success = @ldap_mod_del($_SESSION[$this->ldap]->server(), $DNs[$i], $attributes[$DNs[$i]]['remove']);
					if (!$success) {
						$errors[] = array('ERROR', 'LDAP', sprintf(_('Was unable to remove attribtues from dn: %s. This is possible a bug. Please check your ldap logs and send a bug report if it is a possible bug.'), $DNs[$i]));
						$stopprocessing = true;
						}
					else
						$_SESSION[$this->cache]->update_cache($this->$dn, 'remove', $attributes[$this->dn]['remove']);
					}
				}
			}

		if (!$stopprocessing) {
			foreach ($attributes as $DN) {
				if (is_array($DN['lamdaemon']['command'])) $result = $this->lamdaemon($DN['lamdaemon']['command']);
				// Error somewhere in lamdaemon
				foreach ($result as $singleresult) {
					if (is_array($singleresult)) {
						if ($singleresult[0] = 'ERROR') $stopprocessing = true;
						$temparray[0] = $singleresult[0];
						$temparray[1] = _($singleresult[1]);
						$temparray[2] = _($singleresult[2]);
						$errors[] = $temparray;
						}
					}
				}
			}
		if (count($errors)!=0) return $errors;
		return 0;
		}

	function lamdaemon($commands) {
		// get username and password of the current lam-admin
		$ldap_q = $_SESSION[$this->ldap]->decrypt_login();
		/* $towrite has the following syntax:
		* admin-username, admin-password, owner of homedir, 'home', operation='add'
		* use escapeshellarg to make exec() shell-safe
		*/
		$towrite = escapeshellarg($_SESSION[$this->config]->scriptServer)." ".escapeshellarg($_SESSION[$this->config]->scriptPath)." ".
			escapeshellarg($ldap_q[0]).' '.escapeshellarg($ldap_q[1]);

		$userstring = implode ("\n", $commands);
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
		return $output_array;
		}
		
		function get_pdfEntries($acount_type) {
			$return = array();
			while(($current = current($this->module)) != null) {
				$return = array_merge($return,$current->get_pdfEntries($account_type));
				next($this->module);
			}
			return $return;
		}
		
		// Dummy implementation
		function getAvailablePDFFields() {
			$return = array();
			foreach($this->module as $moduleName => $module) {
				echo "<pre>moduleName: $moduleName\n</pre>";
				$return[$moduleName] = $module->get_pdfFields($this->type);
			}
			return $return;
		}
	}
?>