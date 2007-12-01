<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Roland Gruber

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
*/

/**
* Parent class of all account modules.
*
* @package modules
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
	function baseModule($scope) {
		$this->scope = $scope;
		$this->load_Messages();
		$this->meta = $this->get_metaData();
		$this->autoAddObjectClasses = true;
		// load configuration
		if (isset($_SESSION['config'])) $this->moduleSettings = $_SESSION['config']->get_moduleSettings();
		if (isset($_SESSION['selfServiceProfile'])) $this->selfServiceSettings = $_SESSION['selfServiceProfile']->moduleSettings;
	}

	/**
	* This function fills the $messages variable with output messages from this module.
	*/
	protected function load_Messages() {
	}

	/**
	* Initializes the module after it became part of an accountContainer
	*
	* @param string $base the name of the accountContainer object ($_SESSION[$base])
	*/
	function init($base) {
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
	* This function loads the LDAP attributes for this module.
	*
	* @param array $attributes attribute list
	*/
	function load_attributes($attributes) {
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
	* Dummy function, meta data is provided by sub classes.
	*
	* @return array empty array
	*/
	function get_metaData() {
		return array();
	}

	/**
	* Returns the account type of this module (user, group, host)
	*
	* @return string account type
	*/
	function get_scope() {
		return $this->scope;
	}

	/**
	* Returns true if this module fits for the current scope.
	*
	* @return boolean true if module fits
	*/
	function can_manage() {
		if (is_array($this->meta["account_types"]) && in_array($this->scope, $this->meta["account_types"])) return true;
		else return false;
	}

	/**
	* Returns true if this module is enough to provide a sensible account.
	*
	* There is no relation to the name of this class.
	*
	* @return boolean true if base module
	*/
	function is_base_module() {
		if (isset($this->meta['is_base']) && ($this->meta['is_base'] == true)) return true;
		else return false;
	}

	/**
	* returns an LDAP filter for the account lists
	*
	* @return string LDAP filter
	*/
	function get_ldap_filter() {
		if (isset($this->meta['ldap_filter'])) return $this->meta['ldap_filter'];
		else return "";
	}

	/**
	* Returns an alias name for the module.
	*
	* This alias is used in various places instead of the less descriptive class name.
	* The alias also has less syntax restrictions and may contain spaces or special characters.
	*
	* @return string alias name
	*/
	function get_alias() {
		if (isset($this->meta['alias'])) return $this->meta['alias'];
		else return get_class($this);
	}

	/**
	* Returns a list of possible LDAP attributes which can be used to form the RDN.
	*
	* The returned elements have this form: <attribute> => <priority>
	* <br> <attribute> is the name of the LDAP attribute
	* <br> <priority> defines the priority of the attribute (can be "low", "normal", "high")
	*
	* @return array list of attributes
	*/
	function get_RDNAttributes() {
		if (isset($this->meta['RDN'])) return $this->meta['RDN'];
		else return array();
	}

	/**
	* This function returns a list with all depending and conflicting modules.
	*
	* @return array list of dependencies and conflicts
	*/
	function get_dependencies() {
		if (isset($this->meta['dependencies'])) return $this->meta['dependencies'];
		else return array('depends' => array(), 'conflicts' => array());
		}

	/**
	* Returns a list of elements for the account profiles.
	*
	* @return profile elements
	*/
	function get_profileOptions() {
		if (isset($this->meta['profile_options'])) return $this->meta['profile_options'];
		else return array();
	}

	/**
	* Checks input values of account profiles.
	*
	* @param array $options a hash array (name => value) containing the options
	* @return array list of error messages (array(type, title, text)) to generate StatusMessages, if any
	*/
	function check_profileOptions($options) {
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
	* Loads the values of an account profile into internal variables.
	*
	* @param array $profile hash array with profile values (identifier => value)
	*/
	function load_profile($profile) {
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
	* Returns a list of elements for the configuration.
	*
	* @param array $scopes account types (user, group, host)
	* @param array $allScopes list of all modules and active scopes
	* @return array configuration elements
	*/
	function get_configOptions($scopes, $allScopes) {
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
	* @param array $scopes list of account types which are used
	* @param array $options hash array containing the settings (array('option' => array('value')))
	* @return array list of error messages
	*/
	function check_configOptions($scopes, $options) {
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
	 * Returns an array with all fields available for this account type on the PDF
	 * output. This method may be overwritten by subclasses or it may be used
	 * by using entries in the $this->meta['PDF_fields'] array of the specific sub-
	 * class.
	 *
	 * @param string $scope account type
	 * @return array list of available fields for PDF output
	 */
	function get_pdfFields() {
		return ((isset($this->meta['PDF_fields'])) ? $this->meta['PDF_fields'] : array());
	}

	/**
	 * Returns a hastable with all entries that may be printed out in the PDF. The
	 * syntax of the hashtable is specified by the module specification and the
	 * corresponding DTD. This method must be overwritten in case that there
	 * are non static things to be returned. The $this->meta['PDF_entries'] array
	 * may be used when there is only static content.
	 *
	 * @param string $scope account type
	 * @return array hastable of entries for the PDF. Each entry is an array where
	 * each entry is treated as a new line in the PDF.
	 */
	function get_pdf_entries($scope = 'user') {
		return ((isset($this->meta['PDF_entries'])) ? $this->meta['PDF_entries'] : array());
	}

	/**
	* Returns an array containing all input columns for the file upload.
	*
	* Syntax:
	* <br> array(
	* <br>  string: name,  // fixed non-translated name which is used as column name (should be of format: <module name>_<column name>)
	* <br>  string: description,  // short descriptive name
	* <br>  string: help,  // help ID
	* <br>  string: example,  // example value
	* <br>  boolean: required  // true, if user must set a value for this column
	* <br> )
	*
	* @return array column list
	*/
	function get_uploadColumns() {
		if (isset($this->meta['upload_columns'])) return $this->meta['upload_columns'];
		else return array();
	}

	/**
	* Returns a list of module names which must be processed in building the account befor this module.
	*
	* @return array list of module names
	*/
	function get_uploadPreDepends() {
		if (isset($this->meta['upload_preDepends'])) return $this->meta['upload_preDepends'];
		else return array();
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
		// must be implemented in sub modules
		return array();
	}

	/**
	 * This function return the help entry array for a specific help id. Normally this->meta can be used.
	 *
	 * @param string $id The id string for the help entry needed.
	 * @param string $scope The scope for which the help entry should be retrieved. May be empty when
	 * there is now difference of the help entry depending on the actual scope.
	 *
	 * @return array The desired help entry.
	 */
	function get_help($id) {
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
	* It returns false if a module depends on data from other modules which was not yet entered.
	*
	* @return boolean true, if page can be displayed
	*/
	function module_ready() {
		return true;
	}

	/**
	* This functions is used to check if all settings for this module have been made.
	*
	* @return boolean true, if settings are complete
	*/
	function module_complete() {
		return true;
	}

	/**
	* Controls if the module button the account page is visible and activated.
	*
	* @return string status ("enabled", "disabled", "hidden")
	*/
	function getButtonStatus() {
		return "enabled";
	}

	/**
	* This function executes one post upload action.
	*
	* @param array $data array containing one account in each element
	* @param array $ids array(<column_name> => <column number>)
	* @param array $failed list of accounts which were not created successfully
	* @param array $temp variable to store temporary data between two post actions
	* @return array current status
	* <br> array (
	* <br>  'status' => 'finished' | 'inProgress'
	* <br>  'progress' => 0..100
	* <br>  'errors' => array (<array of parameters for StatusMessage>)
	* <br> )
	*/
	function doUploadPostActions($data, $ids, $failed, &$temp) {
		return array(
			'status' => 'finished',
			'progress' => 100,
			'errors' => array()
		);
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
		return $this->getAccountContainer()->save_module_attributes($this->attributes, $this->orig);
	}

	/**
	 * Allows the module to run commands before the LDAP entry is changed or created.
	 * An error message should be printed if the function returns false.
	 * 
	 * @param boolean $newAccount new account
	 * @return true, if no problems occured
	 */
	function preModifyActions($newAccount) {
		return true;
	}
	
	/**
	 * Allows the module to run commands after the LDAP entry is changed or created.
	 *
	 * @param boolean $newAccount new account
	 */
	function postModifyActions($newAccount) {
		return;
	}
	
	/**
	 * Allows the module to run commands before the LDAP entry is deleted.
	 * An error message should be printed if the function returns false.
	 * 
	 * @return true, if no problems occured
	 */
	function preDeleteActions() {
		return true;
	}
	
	/**
	 * Allows the module to run commands after the LDAP entry is deleted.
	 */
	function postDeleteActions() {
		return;
	}
	
	/**
	* Dummy function for modules which use no special options on account deletion.
	*
	* @return List of LDAP operations, same as for save_attributes()
	*/
	function delete_attributes() {
		return 0;
	}

	/**
	* Dummy function for modules which do not print extra HTML code on account deletion.
	*
	* @return meta HTML code
	*/
	function display_html_delete() {
		return 0;
	}

	/**
	 * Returns a list of managed object classes for this module.
	 * This is used to fix incorrect spelled object class names.
	 *
	 * @return array list of object classes
	 */
	function getManagedObjectClasses() {
		if (isset($this->meta['objectClasses']) && is_array($this->meta['objectClasses'])) return $this->meta['objectClasses'];
		else return array();
	}

	/**
	 * Returns a list of aliases for LDAP attributes.
	 * All alias attributes will be renamed to the given attribute names.
	 *
	 * @return array list of aliases (alias name => attribute name)
	 */
	function getLDAPAliases() {
		if (isset($this->meta['LDAPaliases']) && is_array($this->meta['LDAPaliases'])) return $this->meta['LDAPaliases'];
		else return array();
	}

	/**
	 * Returns a list of LDAP attributes which are managed by this module.
	 * All attribute names will be renamed to match the given spelling.
	 *
	 * @return array list of attributes
	 */
	function getManagedAttributes() {
		if (isset($this->meta['attributes']) && is_array($this->meta['attributes'])) return $this->meta['attributes'];
		else return array();
	}

	/**
	 * Returns a list of required PHP extensions.
	 *
	 * @return array extensions
	 */
	function getRequiredExtensions() {
		if (isset($this->meta['extensions']) && is_array($this->meta['extensions'])) return $this->meta['extensions'];
		else return array();
	}

	/**
	 * Returns a list of possible search attributes for the self service.
	 *
	 * @return array attributes
	 */
	function getSelfServiceSearchAttributes() {
		if (isset($this->meta['selfServiceSearchAttributes']) && is_array($this->meta['selfServiceSearchAttributes'])) return $this->meta['selfServiceSearchAttributes'];
		else return array();
	}

	/**
	 * Returns a list of possible input fields and their descriptions
	 * Format: array(<field identifier> => <field description>)
	 *
	 * @return array fields
	 */
	function getSelfServiceFields() {
		if (isset($this->meta['selfServiceFieldSettings']) && is_array($this->meta['selfServiceFieldSettings'])) return $this->meta['selfServiceFieldSettings'];
		else return array();
	}

	/**
	 * Returns the meta HTML code for each input field.
	 * format: array(<field1> => array(<META HTML>), ...)
	 * It is not possible to display help links.
	 *
	 * @param array $fields list of active fields
	 * @param array $attributes attributes of LDAP account (attribute names in lower case)
	 * @return array meta HTML
	 */
	function getSelfServiceOptions($fields, $attributes) {
		// this function must be overwritten by subclasses.
		return array();
	}

	/**
	 * Checks if all input values are correct and returns the LDAP commands which should be executed.
	 *
	 * @param string $fields input fields
	 * @param array $attributes LDAP attributes
	 * @return array messages and LDAP commands (array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array()))
	 */
	function checkSelfServiceOptions($fields, $attributes) {
		$return = array('messages' => array(), 'add' => array(), 'del' => array(), 'mod' => array());
		return $return;
	}

	/**
	 * Returns a list of self service configuration settings.
	 *
	 * @return array settings
	 */
	function getSelfServiceSettings() {
		if (isset($this->meta['selfServiceSettings']) && is_array($this->meta['selfServiceSettings'])) return $this->meta['selfServiceSettings'];
		else return array();
	}

	/**
	 * Checks if the self service settings are valid.
	 *
	 * @param array $options settings
	 * @return array error messages
	 */
	function checkSelfServiceSettings($options) {
		// needs to be implemented by the subclasses, if needed
		return array();
	}
	
	/**
	 * Returns the accountContainer object.
	 * 
	 * @return accountContainer accountContainer object
	 *
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
	 */
	public function getIcon() {
		if (isset($this->meta['icon'])) {
			return $this->meta['icon'];
		}
		return null;
	}

}


?>