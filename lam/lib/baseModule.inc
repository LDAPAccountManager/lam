<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2008  Roland Gruber

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
* This is the parent class for all account modules.
*
* It implements the complete module interface and uses meta-data
* provided by the account modules for its functions.
*
* @package modules
* @author Roland Gruber
* @see baseModule
*/

/**
* Parent class of all account modules.
* It implements the complete module interface and uses meta-data
* provided by the account modules for its functions.<br>
* <br>
* <b>Location and naming of modules</b><br>
* All LAM modules are placed in lib/modules/ and are named "<class name>.inc".
* E.g. if you create a new module and its class name is "qmail" then the filename would be "qmail.inc".
* The class name of a module must contain only a-z, A-Z, 0-9, -, and _.<br>
* <br>
* You can avoid to override many functions by using {@link get_metaData()}.<br>
* <br>
* All module classes should extend the baseModule class.
*
* @package modules
* @author Roland Gruber
*/
abstract class baseModule {

	/** includes all meta data provided by the sub class */
	protected $meta;

	/** the account type of this module (user, group, host) */
	private $scope;

	/** configuration settings of all modules */
	protected $moduleSettings;

	/** self service settings of all modules */
	protected $selfServiceSettings;

	/** name of parent accountContainer ($_SESSION[$base]) */
	private $base;

	/** contains all ldap attributes which should be written */
	protected $attributes;

	/** contains all ldap attributes which are loaded from ldap */
	protected $orig;

	/** contains all error messages of a module */
	protected $messages;
	
	/** if true, managed object classes are added when an account is created or loaded (default: true) */
	protected $autoAddObjectClasses;

	/**
	* Creates a new base module class
	*
	* @param string $scope the account type (user, group, host)
	*/
	public function __construct($scope) {
		$this->scope = $scope;
		$this->load_Messages();
		$this->meta = $this->get_metaData();
		$this->autoAddObjectClasses = true;
		// load configuration
		if (isset($_SESSION['config'])) $this->moduleSettings = $_SESSION['config']->get_moduleSettings();
		if (isset($_SESSION['selfServiceProfile'])) $this->selfServiceSettings = $_SESSION['selfServiceProfile'];
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.
	*/
	protected function load_Messages() {
	}

	/**
	* Initializes the module after it became part of an {@link accountContainer}
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.
	*
	* @param string $base the name of the {@link accountContainer} object ($_SESSION[$base])
	*/
	public function init($base) {
		$this->base = $base;
		$this->attributes = array();
		$this->orig = array();
		// add object classes if needed
		$this->attributes['objectClass'] = array();
		$this->orig['objectClass'] = array();
		if ($this->autoAddObjectClasses === true) {
			$objectClasses = $this->getManagedObjectClasses();
			for ($i = 0; $i < sizeof($objectClasses); $i++) {
				if (!in_array($objectClasses[$i], $this->attributes['objectClass'])) {
					$this->attributes['objectClass'][] = $objectClasses[$i];
				}
			}
		}
	}


	/**
	* This function loads the LDAP attributes when an account should be loaded.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* By default this method loads the object classes and accounts which are specified in {@link getManagedObjectClasses()}
	* and {@link getManagedAttributes()}.
	*
	* @param array $attributes array like the array returned by get_ldap_attributes(dn of account) but without count indices
	*/
	public function load_attributes($attributes) {
		$this->attributes = array();
		$this->attributes = array();
		// load object classes
		if (isset($attributes['objectClass'])) {
			$this->attributes['objectClass'] = $attributes['objectClass'];
			$this->orig['objectClass'] = $attributes['objectClass'];
		}
		else {
			$this->attributes['objectClass'] = array();
			$this->orig['objectClass'] = array();
		}
		// add object classes if needed
		if ($this->autoAddObjectClasses === true) {
			$objectClasses = $this->getManagedObjectClasses();
			for ($i = 0; $i < sizeof($objectClasses); $i++) {
				if (!in_array($objectClasses[$i], $this->attributes['objectClass'])) {
					$this->attributes['objectClass'][] = $objectClasses[$i];
				}
			}
		}
		// load attributes
		$attributeNames = $this->getManagedAttributes();
		for ($i = 0; $i < sizeof($attributeNames); $i++) {
			if (isset($attributes[$attributeNames[$i]])) {
				$this->attributes[$attributeNames[$i]] = $attributes[$attributeNames[$i]];
				$this->orig[$attributeNames[$i]] = $attributes[$attributeNames[$i]];
			}
		}
	}

	/**
	* This function provides meta data which is interpreted by baseModule.
	* Only subclasses will return real data.<br>
	* <br>
	* The aim of the meta data is to reduce the number
	* of functions in the subclasses. All major data is centralized in one place.<br>
	* <br>
	* The returned array contains a list of key-value pairs for the different functions.<br>
	* <ul>
	* 
	* <li><b>{@link can_manage()}</b><br>
	* <br>
	* <b>Key:</b> account_types<br>
	* <b>Value:</b> array of account types<br>
	* <br>
	* <b>Example:</b> "account_types" => array("user", "host")
	* <br><br>
	* </li>
	* 
	* <li><b>{@link is_base_module()}</b><br>
	* <br>
	* <b>Key:</b> is_base<br>
	* <b>Value:</b> boolean<br>
	* <br>
	* <b>Example:</b> "is_base" => true
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_ldap_filter()}</b><br>
	* <br>
	* <b>Key:</b> ldap_filter<br>
	* <b>Value:</b> array of filters<br>
	* <br>
	* <b>Example:</b> "ldap_filter" => array('or' => 'objectClass=posixAccount', 'and' => '(!(uid=*$))')
	* <br><br>
	* </li>
	* 
	* <li><b>{@link getManagedObjectClasses()}</b><br>
	* <br>
	* <b>Key:</b> objectClasses<br>
	* <b>Value:</b> array of object classes<br>
	* <br>
	* <b>Example:</b> "objectClasses" => array('posixAccount')
	* <br><br>
	* </li>
	* 
	* <li><b>{@link getLDAPAliases()}</b><br>
	* <br>
	* <b>Key:</b> LDAPaliases<br>
	* <b>Value:</b> array of aliases<br>
	* <br>
	* <b>Example:</b> "LDAPaliases" => array('commonName' => 'cn')
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_RDNAttributes()}</b><br>
	* <br>
	* <b>Key:</b> RDN<br>
	* <b>Value:</b> array of RDNs<br>
	* <br>
	* <b>Example:</b> "RDN" => array('uid' => 'normal', 'cn' => 'low')
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_dependencies()}</b><br>
	* <br>
	* <b>Key:</b> dependencies<br>
	* <b>Value:</b> array of dependencies<br>
	* <br>
	* <b>Example:</b> "dependencies" => array("depends" => array("posixAccount", array("qmail", "sendmail")), "conflicts" => array("exim"))
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_profileOptions()}</b><br>
	* <br>
	* <b>Key:</b> profile_options<br>
	* <b>Value:</b> array of profile options<br>
	* <br>
	* The syntax for the value array is the same as for the return value of get_profileOptions().
	* <br><br>
	* </li>
	* 
	* <li><b>{@link check_profileOptions()}</b><br>
	* <br>
	* <b>Key:</b> profile_checks<br>
	* <b>Value:</b> array of checks (array("optionName" => array()))<br>
	* <br>
	* The "optionName" keys of the value array are the names of the option identifiers.<br>
	* Each array element is an array itself containing these values:
	* <ul>
	* <li><b>type:</b> determines how to check input<br>
	* Possible values:
	* 	<ul>
	* 	<li><b>regex:</b> check with regular expression from regex variable, case sensitive</li>
	*	<li><b>regex_i:</b> check with regular expression from regex variable, case insensitive</li>
	* 	<li><b>int_greater:</b> integer value of cmp_name1 must be greater than the integer value from the option cmp_name2</li>
	* 	<li><b>int_greaterOrEqual:</b> integer value of cmp_name1 must be greater or equal than the integer value from the option cmp_name2</li>
	* 	</ul>
	* </li>
	* <li><b>error_message:</b> message that is displayed if input value was syntactically incorrect<br>
	* 	error_message is an array to build StatusMessages (message type, message head, message text, additional variables)
	* <li><b>regex:</b> regular expression string (only if type is regex/regex_i)</li>
	* <li><b>cmp_name1:</b> name of first input variable that is used for comparison (only if type is int_greater/int_greaterOrEqual)</li>
	* <li><b>cmp_name2:</b> name of second input variable that is used for comparison (only if type is int_greater/int_greaterOrEqual)</li>
	* <li><b>required:</b> true or false, if this input field must be filled set to true (optional)
	* <li><b>required_message:</b> message that is displayed if no input value was given (only if required == true)<br>
	* 	required_message is an array to build StatusMessages (message type, message head, message text, additional variables)
	* </li>
	* </ul>
	* <br><br>
	* </li>
	* 
	* <li><b>{@link load_profile()}</b><br>
	* <br>
	* <b>Key:</b> profile_mappings<br>
	* <b>Value:</b> array('profile_identifier1' => 'LDAP_attribute1', 'profile_identifier2' => 'LDAP_attribute2')<br>
	* <br>
	* The mapped values are stored directly in $this->attributes.
	* <br>
	* <b>Example:</b> "profile_mappings" => array('inetOrgPerson_title' => 'title')
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_configOptions()}</b><br>
	* <br>
	* <b>Key:</b> config_options<br>
	* <b>Value:</b> array('user' => array, 'host' => array, 'all' => array)<br>
	* <br>
	* The values from 'all' are always returned, the other values only if they are inside the $scopes array.<br>
	* The syntax for sub arrays is the same as for the return value of {@link get_configOptions()}.
	* <br><br>
	* </li>
	* 
	* <li><b>{@link check_configOptions()}</b><br>
	* <br>
	* <b>Key:</b> config_checks<br>
	* <b>Value:</b> array('user' => array, 'host' => 'array', 'all' => array)<br>
	* <br>
	* The values from 'all' are always used for checking, the other values only if they are inside the $scopes array.
	* The syntax for sub arrays is the same as for {@link check_configOptions()}.
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_uploadColumns()}</b><br>
	* <br>
	* <b>Key:</b> upload_columns<br>
	* <b>Value:</b> array<br>
	* <br>
	* The syntax for array is the same as for the return value of {@link get_uploadColumns()}.
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_uploadPreDepends()}</b><br>
	* <br>
	* <b>Key:</b> upload_preDepends<br>
	* <b>Value:</b> array<br>
	* <br>
	* The syntax for array is the same as for the return value of {@link get_uploadPreDepends()}.
	* <br><br>
	* </li>
	* 
	* <li><b>{@link getRequiredExtensions()}</b><br>
	* <br>
	* <b>Key:</b> extensions<br>
	* <b>Value:</b> array of extension names<br>
	* <br>
	* <b>Example:</b> "extensions" => array('hash')
	* <br><br>
	* </li>
	* 
	* <li><b>{@link get_help()}</b><br>
	* <br>
	* <b>Key:</b> help<br>
	* <b>Value:</b> hashtable of help entries<br>
	* <br>
	* The hashtable is an array which maps help IDs to help entries.<br>
	* <br>
	* <b>Example:</b> 'help' => array('myEntry' => array('Headline' => 'This is the head line', 'Text' => 'Help content'))
	* <br><br>
	* </li>
	* 
	* <li><b>{@link getSelfServiceSearchAttributes()}</b><br>
	* <br>
	* <b>Key:</b> selfServiceSearchAttributes<br>
	* <b>Value:</b> array of attribute names<br>
	* <br>
	* <b>Example:</b> "selfServiceSearchAttributes" => array('uid')
	* <br><br>
	* </li>
	* 
	* <li><b>{@link getSelfServiceFields()}</b><br>
	* <br>
	* <b>Key:</b> selfServiceFieldSettings<br>
	* <b>Value:</b> array of self service fields<br>
	* <br>
	* <b>Example:</b> "selfServiceFieldSettings" => array('pwd' => 'Password')
	* <br><br>
	* </li>
	* 
	* </ul>
	* <b>Example:</b> return array("is_base" => true);
	*
	* @return array meta data
	*/
	public function get_metaData() {
		return array();
	}

	/**
	* Returns the account type of this module (user, group, host)
	*
	* @return string account type
	*/
	public function get_scope() {
		return $this->scope;
	}

	/**
	* Returns true if this module can manage accounts of the current type, otherwise false.
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.
	* 
	* @return boolean true if module fits
	* 
	* @see baseModule::get_metaData()
	*/
	public function can_manage() {
		if (is_array($this->meta["account_types"]) && in_array($this->scope, $this->meta["account_types"])) return true;
		else return false;
	}

	/**
	* Returns true if your module is a base module and otherwise false.
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* Every account type needs exactly one base module. A base module manages a structural object class.
	* E.g. the inetOrgPerson module is a base module since its object class is structural.
	*
	* @return boolean true if base module (defaults to false if no meta data is provided)
	* 
	* @see baseModule::get_metaData()
	*/
	public function is_base_module() {
		if (isset($this->meta['is_base']) && ($this->meta['is_base'] == true)) return true;
		else return false;
	}

	/**
	* Returns an LDAP filter for the account lists
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* Returns an array('or' => '...', 'and' => '...') that is used to build the LDAP filter. Usually, this is used to filter object classes.
	* All "or" filter parts of the base modules are combined with OR and then combined with the "and" parts.<br>
	* The resulting LDAP filter will look like this: (&(|(OR1)(OR2)(OR3))(AND1)(AND2)(AND3))<br>
	* <br>
	* <b>Example:</b> return array('or' => '(objectClass=posixAccount)', 'and' => '(!(uid=*$))')
	*
	* @return string LDAP filter
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_ldap_filter() {
		if (isset($this->meta['ldap_filter'])) return $this->meta['ldap_filter'];
		else return "";
	}

	/**
	* Returns an alias name for the module.
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* This function returns a more descriptive string than the class name. Alias names are used for the buttons on the account pages and the module selection in the configuration wizard.<br>
	* Please take care that your alias name is not too long. It may contain any character but should not include parts that may be interpreted by the browser (e.g. '<' or '>').
	* If you use different aliases dependent on the account type please make sure that there is a general alias for unknown types.
	* 
	* @return string alias name
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_alias() {
		if (isset($this->meta['alias'])) return $this->meta['alias'];
		else return get_class($this);
	}

	/**
	* Returns a hash array containing a list of possible LDAP attributes that can be used to form the RDN (Relative Distinguished Name).
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The returned elements have this form: <attribute> => <priority>
	* <br> <attribute> is the name of the LDAP attribute
	* <br> <priority> defines the priority of the attribute (can be "low", "normal", "high")<br>
	* <br>
	* <b>Example:</b> return array('uid' => 'normal', 'cn' => 'low')
	*
	* @return array list of attributes
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_RDNAttributes() {
		if (isset($this->meta['RDN'])) return $this->meta['RDN'];
		else return array();
	}

	/**
	* This function returns a list with all depending and conflicting modules.
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The return value is an array with two sub arrays, "depends" and "conflicts".
	* All values of the conflict array are string values with module names. All values of the depends
	* array are either string values with module names or arrays which include only string values with
	* module names.<br>
	* If an element of the depends array is itself an array, this means that your module
	* depends on one of these modules.<br>
	* <br>
	* <b>Example:</b> return array("depends" => array("posixAccount", array("qmail", "sendmail")), "conflicts" => array("exim"))
	*
	* @return array list of dependencies and conflicts
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_dependencies() {
		if (isset($this->meta['dependencies'])) return $this->meta['dependencies'];
		else return array('depends' => array(), 'conflicts' => array());
		}

	/**
	* This function defines what attributes will be used in the account profiles and their appearance in the profile editor.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The return value is an array that contains meta HTML code.<br>
	* The type "fieldset" is not allowed here. The name attributes are used as keywords to load
	* and save profiles. We recommend to use the module name as prefix for them
	* (e.g. posixAccount_homeDirectory) to avoid naming conflicts.
	* 
	* @return array meta HTML code
	* 
	* @see baseModule::get_metaData()
	* @see parseHtml()
	*/
	public function get_profileOptions() {
		if (isset($this->meta['profile_options'])) return $this->meta['profile_options'];
		else return array();
	}

	/**
	* Checks input values of account profiles.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* $options is an hash array (option name => value) that contains the user input.
	* The option values are all arrays containing one or more elements.<br>
	* If the input data is invalid the return value is an array that contains arrays
	* to build StatusMessages (message type, message head, message text). If no errors occured
	* the function returns an empty array.
	*
	* @param array $options a hash array (name => value) containing the user input
	* @return array list of error messages (array(type, title, text)) to generate StatusMessages, if any
	* 
	* @see baseModule::get_metaData()
	*/
	public function check_profileOptions($options) {
		$messages = array();
		if (is_array($this->meta['profile_checks'])) {
			$identifiers = array_keys($this->meta['profile_checks']);
			for ($i = 0; $i < sizeof($identifiers); $i++) {
				// empty input
				if (($options[$identifiers[$i]][0] == '') || !isset($options[$identifiers[$i]][0])) {
					// check if option is required
					if (isset($this->meta['profile_checks'][$identifiers[$i]]['required']) && $this->meta['profile_checks'][$identifiers[$i]]['required']) {
						$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['required_message'];
					}
					continue;
				}
				switch ($this->meta['profile_checks'][$identifiers[$i]]['type']) {
					// check by regular expression (from account.inc)
					case "ext_preg":
						// ignore empty fileds
						if ($options[$identifiers[$i]][0] == '') continue;
						if (! get_preg($options[$identifiers[$i]][0], $this->meta['profile_checks'][$identifiers[$i]]['regex'])) {
							$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
						}
					break;
					// check by regular expression (case insensitive)
					case 'regex_i':
						// ignore empty fileds
						if ($options[$identifiers[$i]][0] == '') continue;
						if (! eregi($this->meta['profile_checks'][$identifiers[$i]]['regex'], $options[$identifiers[$i]][0])) {
							$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
						}
						break;
					// check by regular expression (case sensitive)
					case 'regex':
						// ignore empty fileds
						if ($options[$identifiers[$i]][0] == '') continue;
						if (! ereg($this->meta['profile_checks'][$identifiers[$i]]['regex'], $options[$identifiers[$i]][0])) {
							$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
						}
						break;
					// check by integer comparison (greater)
					case 'int_greater':
						// ignore if both fields are empty
						if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') && ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) continue;
						// print error message if only one field is empty
						if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') || ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) {
							$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
							continue;
						}
						// compare
						if (!(intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0]) > intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0]))) {
							$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
						}
						break;
					// check by integer comparison (greater or equal)
					case 'int_greaterOrEqual':
						// ignore if both fields are empty
						if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') && ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) continue;
						// print error message if only one field is empty
						if (($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0] == '') || ($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0] == '')) {
							$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
							continue;
						}
						// compare
						if (!(intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name1']][0]) >= intval($options[$this->meta['profile_checks'][$identifiers[$i]]['cmp_name2']][0]))) {
							$messages[] = $this->meta['profile_checks'][$identifiers[$i]]['error_message'];
						}
						break;
					// print error message for invalid types
					default:
						StatusMessage("ERROR", "Unsupported type!", $this->meta['profile_checks'][$identifiers[$i]]['type']);
						break;
				}
			}
		}
		return $messages;
	}

	/**
	* This function loads the values from an account profile to the module's internal data structures.
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.
	*
	* @param array $profile hash array with profile values (identifier => value)
	* 
	* @see baseModule::get_metaData()
	*/
	public function load_profile($profile) {
		if (isset($this->meta['profile_mappings'])) {
			$identifiers = array_keys($this->meta['profile_mappings']);
			for ($i = 0; $i < sizeof($identifiers); $i++) {
				if (isset($profile[$identifiers[$i]])) {
					$this->attributes[$this->meta['profile_mappings'][$identifiers[$i]]] = $profile[$identifiers[$i]];
				}
			}
		}
	}

	/**
	* Returns a list of configuration options.
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The type "fieldset" is not allowed here. The name attributes are used as keywords to load and save settings.
	* We recommend to use the module name as prefix for them (e.g. posixAccount_homeDirectory) to avoid naming conflicts.
	*
	* @param array $scopes account types (user, group, host)
	* @param array $allScopes list of all active account modules and their scopes (module => array(scopes))
	* @return array meta HTML code
	* 
	* @see baseModule::get_metaData()
	* @see parseHtml()
	*/
	public function get_configOptions($scopes, $allScopes) {
		$return = array();
		for ($i = 0; $i < sizeof($scopes); $i++) {
			if (isset($this->meta['config_options'][$scopes[$i]])) $return = array_merge($return, $this->meta['config_options'][$scopes[$i]]);
		}
		if (isset($this->meta['config_options']['all'])) $return = array_merge($return, $this->meta['config_options']['all']);
		return $return;
	}

	/**
	* Checks input values of module settings.
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* If the input data is invalid the return value is an array that contains subarrays to build StatusMessages ('message type', 'message head', 'message text').
	* <br>If no errors occured the function returns an empty array.
	*
	* @param array $scopes list of account types which are used
	* @param array $options hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	* @return array list of error messages
	* 
	* @see baseModule::get_metaData()
	*/
	public function check_configOptions($scopes, $options) {
		$messages = array();
		$scopes[] = 'all'; // add checks that are independent of scope
		for ($s = 0; $s < sizeof($scopes); $s++) {
			if (isset($this->meta['config_checks'][$scopes[$s]]) && is_array($this->meta['config_checks'][$scopes[$s]])) {
				$identifiers = array_keys($this->meta['config_checks'][$scopes[$s]]);
				for ($i = 0; $i < sizeof($identifiers); $i++) {
					// check if option is required
					if (isset($this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['required']) && ($options[$identifiers[$i]][0] == '')) {
						$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['required_message'];
					}
					switch ($this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['type']) {
						// check by regular expression (from account.inc)
						case "ext_preg":
							// ignore empty fileds
							if ($options[$identifiers[$i]][0] == '') continue;
							if (! get_preg($options[$identifiers[$i]][0], $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['regex'])) {
								$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['error_message'];
							}
							break;
						// check by regular expression (case insensitive)
						case "regex_i":
							// ignore empty fileds
							if ($options[$identifiers[$i]][0] == '') continue;
							if (! eregi($this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['regex'], $options[$identifiers[$i]][0])) {
								$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['error_message'];
							}
							break;
						// check by regular expression (case sensitive)
						case "regex":
							// ignore empty fileds
							if ($options[$identifiers[$i]][0] == '') continue;
							if (! ereg($this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['regex'], $options[$identifiers[$i]][0])) {
								$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['error_message'];
							}
							break;
						// check by integer comparison (greater)
						case "int_greater":
							// ignore if both fields are empty
							if (($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name1']][0] == '') && ($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name2']][0] == '')) continue;
							// print error message if only one field is empty
							if (($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name1']][0] == '') || ($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name2']][0] == '')) {
								$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['error_message'];
								continue;
							}
							// compare
							if (!(intval($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name1']][0]) > intval($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name2']][0]))) {
								$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['error_message'];
							}
							break;
						// check by integer comparison (greater or equal)
						case "int_greaterOrEqual":
							// ignore if both fields are empty
							if (($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name1']][0] == '') && ($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name2']][0] == '')) continue;
							// print error message if only one field is empty
							if (($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name1']][0] == '') || ($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name2']][0] == '')) {
								$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['error_message'];
								continue;
							}
							// compare
							if (!(intval($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name1']][0]) >= intval($options[$this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['cmp_name2']][0]))) {
								$messages[] = $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['error_message'];
							}
							break;
						// print error message on undefined type
						default:
							StatusMessage("ERROR", "Unsupported type!", $this->meta['config_checks'][$scopes[$s]][$identifiers[$i]]['type']);
							break;
					}
				}
			}
		}
		return $messages;
	}

	/**
	 * Returns a hashtable with all entries that may be printed out in the PDF.
	 *
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * This method must be overwritten in case that there are non static values
	 * to be returned. The $this->meta['PDF_fields'] array may be used for static content.<br>
	 * <br>
	 * <b>Format of returned hashtable:</b><br>
	 * <br>
	 * This function uses XML formatted commands to define the PDF output. Each part in the PDF
	 * document is surrounded by "<block>" and "</block>".<br>
	 * Inside the <block> tags there are different ways to format the output:
	 * <ul>
	 * <li><b>simple line with attribute name and value:</b> <block><key><b>attribute name</b></key><value><b>attribute value</b></value></block></li>
	 * <li><b>table:</b> <block><key><b>attribute name</b></key><tr><td><b>value</b><td><td><b>value</b><td></tr></block><block><tr><td><b>value</b></td><td><b>value</b><td></tr></block></li>
	 * </ul>
	 * <b>Special commands:</b>
	 * <ul>
	 * <li><b>Alignment in <td>:</b> You can specify the alignment in <td> tags with align=(L|R|C) (e.g. <td align=\"L\">)</li>
	 * <li><b>Cell width:</b> <td> allows an attribute "width" to set the cell width (e.g. <td width=20%> or <td width=30>).</li>
	 * <li><b>Line breaks:</b> Line breaks can be specified by adding a <<br>> tag. The new line will start at the left border of the PDF document.</li>
	 * </ul>
	 * <br>
	 * <b>Examples:</b><br>
	 * <br>
	 * <b>Simple name+value lines:</b><br><br>
	 * In most cases you will just want to display a single line per attribute with its name and value.<br>
	 * <br>
	 * 'myAttribute' => '<block><key>AttrName</key><value>12345</value></block>'<br>
	 * <br>
	 * This will give the following PDF output:<br>
	 * <br>
	 * <b>Attribute name:</b> 12345<br>
	 * <br>
	 * <br>
	 * <b>Multiline values:</b><br><br>
	 * Sometimes you have multivalued attributes where it is not applicable to write all values in one line but
	 * where you want to list your values one below the other or show a table. This can be done by using the <td> tag.<br>
	 * <br>
	 * This example only uses one column but you can just use more <td> tags per <block> tag to display more columns.<br>
	 * <br>
	 * 'myAttribute' => '<block><key>AttrName</key><tr><td align=\"L\">123</td></tr></block><block><tr><td align=\"L\">456</td></tr></block><block><tr><td align=\"L\">789</td></tr></block>'
	 * 
	 * @param string $scope account type
	 * @return array PDF entries
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function get_pdfFields() {
		return ((isset($this->meta['PDF_fields'])) ? $this->meta['PDF_fields'] : array());
	}

	/**
	* Returns the PDF entries for this module.
	*
	* @return array list of possible PDF entries
	*/
	public function get_pdfEntries() {
		return array();
	}

	/**
	* Returns an array containing all input columns for the file upload.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* This funtion returns an array which contains subarrays which represent an upload column.
	* <b>Syntax of column arrays:</b>
	* <br>
	* <br> array(
	* <br>  string: name,  // fixed non-translated name which is used as column name (should be of format: <module name>_<column name>)
	* <br>  string: description,  // short descriptive name
	* <br>  string: help,  // help ID
	* <br>  string: example,  // example value
	* <br>  string: values, // possible input values (optional)
	* <br>  string: default, // default value (optional)
	* <br>  boolean: required  // true, if user must set a value for this column
	* <br>  boolean: unique // true if all values of this column must be different values (optional, default: "false")
	* <br> )
	*
	* @return array column list
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_uploadColumns() {
		if (isset($this->meta['upload_columns'])) return $this->meta['upload_columns'];
		else return array();
	}

	/**
	* Returns a list of module names which must be processed in building the account befor this module.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* The named modules may not be active, LAM will check this automatically.
	*
	* @return array list of module names
	* 
	* @see baseModule::get_metaData()
	*/
	public function get_uploadPreDepends() {
		if (isset($this->meta['upload_preDepends'])) return $this->meta['upload_preDepends'];
		else return array();
	}

	/**
	* In this function the LDAP accounts are built.
	*
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* Returns an array which contains subarrays to generate StatusMessages if any errors occured.
	*
	* @param array $rawAccounts the user input data, contains one subarray for each account.
	* @param array $partialAccounts list of hash arrays (name => value) which are later added to LDAP
	* @param array $ids list of IDs for column position (e.g. "posixAccount_uid" => 5)
	* @return array list of error messages if any
	*/
	public function build_uploadAccounts($rawAccounts, $ids, &$partialAccounts) {
		// must be implemented in sub modules
		return array();
	}

	/**
	 * This function returns the help entry array for a specific help id.
	 *
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * The result is an hashtable with the following keys:<br>
	 * <ul>
	 * <li><b>Headline (required)</b><br>
	 * The headline of this help entry. Can consist of any alpha-numeric characters. No HTML/CSS elements are allowed.</li>
	 * <li><b>Text (required)</b><br>
	 * The text of the help entry which may contain any alpha-numeric characters.</li>
	 * <li><b>SeeAlso (optional)</b><br>
	 * A reference to anonther related web site. It must be an array containing a field called "text" with the link text
	 * that should be displayed and a field called "link" which is the link target.</li>
	 * </ul>
	 * <br>
	 * <b>Example:</b><br>
	 * <br>
	 * array('Headline' => 'This is the head line', 'Text' => 'Help content', 'SeeAlso' => array('text' => 'LAM homepage', 'link' => 'http://lam.sf.net'))
	 *
	 * @param string $id The id string for the help entry needed.
	 * @return array The desired help entry.
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function get_help($id) {
		if(isset($this->meta['help'][$id])) {
			return $this->meta['help'][$id];
		}
		elseif(isset($this->meta['help'][$this->scope][$id])) {
			return $this->meta['help'][$this->scope][$id];
		}
		else {
			return false;
		}
	}

	/**
	* This function is used to check if this module page can be displayed.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* Your module might depend on input of other modules. This function determines if the user
	* can change to your module page or not. The return value is true if your module accepts
	* input, otherwise false.<br>
	* This method's return value defaults to true.
	*
	* @return boolean true, if page can be displayed
	*/
	public function module_ready() {
		return true;
	}

	/**
	* This functions is used to check if all settings for this module have been made.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* This function tells LAM if it can create/modify the LDAP account. If your module needs any
	* additional input then set this to false. The user will be notified that your module needs
	* more input.<br>
	* This method's return value defaults to true.
	*
	* @return boolean true, if settings are complete
	*/
	public function module_complete() {
		return true;
	}

	/**
	* Controls if the module button the account page is visible and activated.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* <b>Possible return values:</b>
	* <ul>
	* <li><b>enabled:</b> button is visible and active</li>
	* <li><b>disabled:</b> button is visible and deactivated (greyed)</li>
	* <li><b>hidden:</b> no button will be shown</li>
	* </ul>
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	public function getButtonStatus() {
		return "enabled";
	}

	/**
	* This function is responsible to do additional tasks after the account has been created in LDAP (e.g. modifying group memberships, adding Quota etc..).
	* 
	* Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* This function is called as long as the returned status is 'finished'. Please make sure
	* that one function call lasts no longer than 3-4 seconds. Otherwise the upload may fail
	* because the time limit is exceeded. You should not make more than one LDAP operation in
	* each call.
	*
	* @param array $data array containing one account in each element
	* @param array $ids maps the column names to keys for the sub arrays (array(<column_name> => <column number>))
	* @param array $failed list of account numbers which could not be successfully uploaded to LDAP
	* @param array $temp variable to store temporary data between two post actions
	* @return array current status
	* <br> array (
	* <br>  'status' => 'finished' | 'inProgress' // defines if all operations are complete
	* <br>  'progress' => 0..100 // the progress of the operations in percent
	* <br>  'errors' => array // list of arrays which are used to generate StatusMessages
	* <br> )
	*/
	public function doUploadPostActions($data, $ids, $failed, &$temp) {
		return array(
			'status' => 'finished',
			'progress' => 100,
			'errors' => array()
		);
	}

	/**
	* Returns a list of modifications which have to be made to the LDAP account.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	*
	* <br>This function returns an array with 3 entries:
	* <br>array( DN1 ('add' => array($attr), 'remove' => array($attr), 'modify' => array($attr)), DN2 .... )
	* <br>DN is the DN to change. It is possible to change several DNs (e.g. create a new user and add him
	* to some groups via attribute memberUid)<br>
	* <br><b>"add"</b> are attributes which have to be added to the LDAP entry
	* <br><b>"remove"</b> are attributes which have to be removed from the LDAP entry
	* <br><b>"modify"</b> are attributes which have to be modified in the LDAP entry
	* <br><b>"notchanged"</b> are attributes which stay unchanged
	* <br>
	* <br>This builds the required comands from $this-attributes and $this->orig.
	* 
	* @return array list of modifications
	*/
	public function save_attributes() {
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	 * Allows the module to run commands before the LDAP entry is changed or created.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * An error message should be printed if the function returns false.
	 * 
	 * @param boolean $newAccount new account
	 * @return true, if no problems occured
	 */
	public function preModifyActions($newAccount) {
		return true;
	}
	
	/**
	 * Allows the module to run commands after the LDAP entry is changed or created.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.
	 *
	 * @param boolean $newAccount new account
	 */
	public function postModifyActions($newAccount) {
		return;
	}
	
	/**
	 * Allows the module to run commands before the LDAP entry is deleted.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * An error message should be printed if the function returns false.
	 * 
	 * @return true, if no problems occured
	 */
	public function preDeleteActions() {
		return true;
	}
	
	/**
	 * Allows the module to run commands after the LDAP entry is deleted.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.
	 */
	public function postDeleteActions() {
		return;
	}
	
	/**
	* This function returns an array with the same syntax as save_attributes().
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* It allows additional LDAP changes when an account is deleted.
	*
	* @return List of LDAP operations, same as for save_attributes()
	*/
	public function delete_attributes() {
		return 0;
	}

	/**
	* This function creates meta HTML code which will be displayed when an account should be deleted.
	* 
	* Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	* <br>
	* This can be used to interact with the user, e.g. should the home directory be deleted? The output
	* of all modules is displayed on a single page.
	*
	* @return meta HTML code
	* @see parseHtml()
	*/
	public function display_html_delete() {
		return 0;
	}
	
	/**
	 * This function processes user input.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * It checks the user input and saves changes in the module's data structures.<br>
	 * <br>
	 * <b>Example:</b> return array(array('ERROR', 'Invalid input!', 'This is not allowed here.'));
	 * 
	 * @return array Array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	public abstract function process_attributes();
	
	/**
	 * This function creates meta HTML code to display the module page.
	 * 
	 * Calling this method requires the existence of an enclosing {@link accountContainer}.
	 * 
	 * @return meta HTML
	 *
	 * @see parseHtml()
	 */
	public abstract function display_html_attributes();

	/**
	 * Returns a list of managed object classes for this module.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * This is used to fix spelling errors in LDAP-Entries (e.g. if "posixACCOUNT" is read instead of "posixAccount" from LDAP).<br>
	 * <br>
	 * <b>Example:</b> return array('posixAccount')
	 *
	 * @return array list of object classes
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function getManagedObjectClasses() {
		if (isset($this->meta['objectClasses']) && is_array($this->meta['objectClasses'])) return $this->meta['objectClasses'];
		else return array();
	}

	/**
	 * Returns a list of aliases for LDAP attributes.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * All alias attributes will be renamed to the given attribute names.
	 *
	 * @return array list of aliases like array("alias name" => "attribute name")
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function getLDAPAliases() {
		if (isset($this->meta['LDAPaliases']) && is_array($this->meta['LDAPaliases'])) return $this->meta['LDAPaliases'];
		else return array();
	}

	/**
	 * Returns a list of LDAP attributes which are managed by this module.
	 * All attribute names will be renamed to match the given spelling.
	 *
	 * @return array list of attributes
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function getManagedAttributes() {
		if (isset($this->meta['attributes']) && is_array($this->meta['attributes'])) return $this->meta['attributes'];
		else return array();
	}

	/**
	 * This function returns a list of PHP extensions (e.g. hash) which are needed by this module.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.
	 *
	 * @return array extensions
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function getRequiredExtensions() {
		if (isset($this->meta['extensions']) && is_array($this->meta['extensions'])) return $this->meta['extensions'];
		else return array();
	}

	/**
	 * This function returns a list of possible LDAP attributes (e.g. uid, cn, ...) which can be used to search for LDAP objects.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.
	 *
	 * @return array attributes
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function getSelfServiceSearchAttributes() {
		if (isset($this->meta['selfServiceSearchAttributes']) && is_array($this->meta['selfServiceSearchAttributes'])) return $this->meta['selfServiceSearchAttributes'];
		else return array();
	}

	/**
	 * Returns a list of possible input fields and their descriptions.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * <b>Format:</b> array(<field identifier> => <field description>)
	 *
	 * @return array fields
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function getSelfServiceFields() {
		if (isset($this->meta['selfServiceFieldSettings']) && is_array($this->meta['selfServiceFieldSettings'])) return $this->meta['selfServiceFieldSettings'];
		else return array();
	}

	/**
	 * Returns the meta HTML code for each input field.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * <b>Format:</b> array(<field1> => array(<META HTML>), ...)<br>
	 * It is not possible to display help links.
	 *
	 * @param array $fields list of active fields
	 * @param array $attributes attributes of LDAP account (attribute names in lower case)
	 * @return array meta HTML
	 * 
	 * @see parseHtml()
	 */
	public function getSelfServiceOptions($fields, $attributes) {
		// this function must be overwritten by subclasses.
		return array();
	}

	/**
	 * Checks if all input values are correct and returns the LDAP attributes which should be changed.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @return array messages and attributes (array('messages' => array(), 'add' => array('mail' => array('test@test.com')), 'del' => array(), 'mod' => array()))
	 */
	public function checkSelfServiceOptions($fields, $attributes) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array());
		return $return;
	}

	/**
	 * Returns a list of self service configuration settings.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * The type "fieldset" is not allowed here. The name attributes are used as keywords to load
	 * and save settings. We recommend to use the module name as prefix for them
	 * (e.g. posixAccount_homeDirectory) to avoid naming conflicts.
	 *
	 * @return array meta HTML code
	 * 
	 * @see baseModule::get_metaData()
	 * @see parseHtml()
	 */
	public function getSelfServiceSettings() {
		if (isset($this->meta['selfServiceSettings']) && is_array($this->meta['selfServiceSettings'])) return $this->meta['selfServiceSettings'];
		else return array();
	}

	/**
	 * Checks if the self service settings are valid.
	 * 
	 * Calling this method does not require the existence of an enclosing {@link accountContainer}.<br>
	 * <br>
	 * If the input data is invalid the return value is an array that contains arrays
	 * to build StatusMessages (message type, message head, message text). If no errors
	 * occured the function returns an empty array.
	 *
	 * @param array $options hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	 * @return array error messages
	 */
	public function checkSelfServiceSettings($options) {
		// needs to be implemented by the subclasses, if needed
		return array();
	}
	
	/**
	 * Returns the {@link accountContainer} object.
	 * 
	 * @return accountContainer accountContainer object
	 *
	 * @see accountContainer
	 */
	protected function getAccountContainer() {
		if (isset($this->base) && isset($_SESSION[$this->base])) {
			return $_SESSION[$this->base];
		}
		else {
			return null;
		}
	}
	
	/**
	 * Returns the LDAP attributes which are managed in this module.
	 *
	 * @return array attributes
	 */
	public function getAttributes() {
		return $this->attributes;
	}
	
	/**
	 * Returns the LDAP attributes which are managed in this module (with unchanged values).
	 *
	 * @return array attributes
	 */
	public function getOriginalAttributes() {
		return $this->orig;
	}
	
	/**
	 * Returns the path to the module icon.
	 * The path must be releative to graphics (e.g. key.png). You can also set $this->meta['icon'].
	 * The preferred size is 32x32px.
	 *
	 * @return unknown
	 * 
	 * @see baseModule::get_metaData()
	 */
	public function getIcon() {
		if (isset($this->meta['icon'])) {
			return $this->meta['icon'];
		}
		return null;
	}

}


?>