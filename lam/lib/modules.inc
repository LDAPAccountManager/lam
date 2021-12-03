<?php

use LAM\PROFILES\AccountProfilePersistenceManager;
use LAM\TYPES\ConfiguredType;
use \LAM\TYPES\TypeManager;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2021  Roland Gruber

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
* Interface between modules and other parts of LAM.
*
* @package modules
* @author Tilo Lutz
* @author Michael Duergner
* @author Roland Gruber
*/

/** self service functions */
include_once(__DIR__ . "/selfService.inc");
if (isLAMProVersion()) {
	/** job interface */
	include_once(__DIR__ . "/jobs.inc");
}
/** some helper functions */
include_once(__DIR__ . "/account.inc");
/** parent class of account modules */
include_once(__DIR__ . "/baseModule.inc");
/** access to LDAP server */
include_once(__DIR__ . "/ldap.inc");
/** remote functions */
include_once(__DIR__ . "/remote.inc");
/** security functions */
include_once(__DIR__ . "/security.inc");
/** meta HTML classes */
include_once(__DIR__ . "/html.inc");

/**
* This includes all module files.
*/
$modulesINC_dirname = substr(__FILE__, 0, strlen(__FILE__) - 12) . "/modules";
$modulesINC_dir = dir($modulesINC_dirname);
// get module names.
while ($entry = $modulesINC_dir->read()) {
	if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($modulesINC_dirname . '/'.$entry)) {
		include_once($modulesINC_dirname . '/'.$entry);
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
	$module = moduleCache::getModule($name, $scope);
	return $module->get_alias();
}

/**
* Returns true if the module is a base module
*
* @param string $name the module name
* @param string $scope the account type ("user", "group", "host")
* @return boolean true if base module
*/
function is_base_module($name, $scope) {
	$module = moduleCache::getModule($name, $scope);
	return $module->is_base_module();
}

/**
* Returns the LDAP filter used by the account lists
*
* @param string $typeId the account type ("user", "group", "host")
* @return string LDAP filter
*/
function get_ldap_filter($typeId) {
	$typeManager = new \LAM\TYPES\TypeManager();
	$type = $typeManager->getConfiguredType($typeId);
	$mods = $_SESSION['config']->get_AccountModules($typeId);
	$filters = array('or' => array(), 'and' => array());
	$orFilter = '';
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = moduleCache::getModule($mods[$i], $type->getScope());
		$modinfo = $module->get_ldap_filter($typeId);
		if (isset($modinfo['or'])) {
			$filters['or'][] = $modinfo['or'];
		}
		if (isset($modinfo['and'])) {
			$filters['and'][] = $modinfo['and'];
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
	if (!empty($orFilter)) {
		$filters['and'][] = $orFilter;
	}
	// add type filter
	$typeSettings = $_SESSION['config']->get_typeSettings();
	if (isset($typeSettings['filter_' . $typeId]) && ($typeSettings['filter_' . $typeId] != '')) {
		if (strpos($typeSettings['filter_' . $typeId], '(') === 0) {
			$filters['and'][] = $typeSettings['filter_' . $typeId];
		}
		else {
			$filters['and'][] = '(' . $typeSettings['filter_' . $typeId] . ')';
		}
	}
	// collapse AND filters
	$finalFilter = '';
	if (sizeof($filters['and']) < 2) {
		$finalFilter = $filters['and'][0];
	}
	else {
		$finalFilter = "(&" . implode("", $filters['and']) . ")";
	}
	$loginData = $_SESSION['ldap']->getUserName();
	return str_replace('@@LOGIN_DN@@', $loginData, $finalFilter);
}

/**
* Returns a list of LDAP attributes which can be used to form the RDN.
*
* The list is already sorted by the priority given by the nodules.
*
* @param string $typeId account type (user, group, host)
* @param array $selectedModules return only RDN attributes of these modules
* @return array list of LDAP attributes
*/
function getRDNAttributes($typeId, $selectedModules=null) {
	$mods = $_SESSION['config']->get_AccountModules($typeId);
	if ($selectedModules != null) {
		$mods = $selectedModules;
	}
	$return = array();
	$attrs_low = array();
	$attrs_normal = array();
	$attrs_high = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		// get list of attributes
		$module = moduleCache::getModule($mods[$i], \LAM\TYPES\getScopeFromTypeId($typeId));
		$attrs = $module->get_RDNAttributes($typeId);
		$keys = array_keys($attrs);
		// sort attributes
		for ($k = 0; $k < sizeof($keys); $k++) {
			switch ($attrs[$keys[$k]]) {
				case "low":
					$attrs_low[] = $keys[$k];
					break;
				case "normal":
					$attrs_normal[] = $keys[$k];
					break;
				case "high":
					$attrs_high[] = $keys[$k];
					break;
				default:
					$attrs_low[] = $keys[$k];
					break;
			}
		}
	}
	// merge arrays
	$return = array_values(array_unique($attrs_high));
	for ($i = 0; $i < sizeof($attrs_normal); $i++) {
		if (!in_array($attrs_normal[$i], $return)) {
			$return[] = $attrs_normal[$i];
		}
	}
	for ($i = 0; $i < sizeof($attrs_low); $i++) {
		if (!in_array($attrs_low[$i], $return)) {
			$return[] = $attrs_low[$i];
		}
	}
	return $return;
}

/**
* Returns a hash array (module name => dependencies) of all module dependencies
*
* "dependencies" contains an array with two sub arrays: depends, conflicts
* <br>The elements of "depends" are either module names or an array of module names (OR-case).
* <br>The elements of conflicts are module names.
*
* @param string $scope the account type (user, group, host)
* @return array dependencies
*/
function getModulesDependencies($scope) {
	$mods = getAvailableModules($scope);
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = moduleCache::getModule($mods[$i], $scope);
		$return[$mods[$i]] = $module->get_dependencies();
	}
	return $return;
}


/**
* Checks if there are missing dependencies between modules.
*
* @param array $selected selected module names
* @param array $deps module dependencies
* @return mixed false if no missing dependency was found,
* otherwise an array of array(selected module, depending module) if missing dependencies were found
*/
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
	if (sizeof($ret) > 0) {
		return $ret;
	}
	return false;
}

/**
* Checks if there are conflicts between modules
*
* @param array $selected selected module names
* @param array $deps module dependencies
* @return boolean false if no conflict was found,
* otherwise an array of array(selected module, conflicting module) if conflicts were found
*/
function check_module_conflicts($selected, $deps) {
	$ret = array();
	for ($m = 0; $m < sizeof($selected); $m++) {
		for ($i = 0; $i < sizeof($deps[$selected[$m]]['conflicts']); $i++) {
			if (in_array($deps[$selected[$m]]['conflicts'][$i], $selected)) {
				$ret[] = array($selected[$m], $deps[$selected[$m]]['conflicts'][$i]);
			}
		}
	}
	if (sizeof($ret) > 0) {
		return $ret;
	}
	return false;
}

/**
* Returns an array with all available user module names
*
* @param string $scope account type (user, group, host)
* @param boolean $mustSupportAdminInterface module must support LAM admin interface (default: false)
* @return array list of possible modules
*/
function getAvailableModules($scope, $mustSupportAdminInterface = false) {
	$dirname = substr(__FILE__, 0, strlen(__FILE__) - 12) . "/modules";
	$dir = dir($dirname);
	$return = array();
	// get module names.
	while ($entry = $dir->read()) {
		if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($dirname . '/'.$entry)) {
			$entry = substr($entry, 0, strpos($entry, '.'));
			$temp = moduleCache::getModule($entry, $scope);
			if ($mustSupportAdminInterface && !$temp->supportsAdminInterface()) {
				continue;
			}
			if ($temp->can_manage()) {
				$return[] = $entry;
			}
		}
	}
	return $return;
}

/**
* Returns the elements for the profile page.
*
* @param string $typeId account type (user, group, host)
* @return array profile elements
*/
function getProfileOptions($typeId) {
	$typeManager = new TypeManager();
	$type = $typeManager->getConfiguredType($typeId);
	$mods = $type->getModules();
	$return = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = moduleCache::getModule($mods[$i], $type->getScope());
		$return[$mods[$i]] = $module->get_profileOptions($typeId);
	}
	return $return;
}

/**
* Checks if the profile options are valid
*
* @param string $typeId account type (user, group, host)
* @param array $options hash array containing all options (name => array(...))
* @return array list of error messages
*/
function checkProfileOptions($typeId, $options) {
	$typeManager = new TypeManager();
	$type = $typeManager->getConfiguredType($typeId);
	$mods = $type->getModules();
	$return = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = moduleCache::getModule($mods[$i], $type->getScope());
		$temp = $module->check_profileOptions($options, $type->getId());
		$return = array_merge($return, $temp);
	}
	return $return;
}

/**
* Returns a hash array (module name => elements) of all module options for the configuration page.
*
* @param array $scopes hash array (module name => array(account types))
* @return array configuration options
*/
function getConfigOptions($scopes) {
	$return = array();
	$modules = array_keys($scopes);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], 'none');
		$return[$modules[$i]] = $m->get_configOptions($scopes[$modules[$i]], $scopes);
	}
	return $return;
}

/**
* Checks if the configuration options are valid
*
* @param array $scopes hash array (module name => array(account type ids))
* @param array $options hash array containing all options (name => array(...))
* @return array list of error messages
*/
function checkConfigOptions($scopes, &$options) {
	$return = array();
	$modules = array_keys($scopes);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = moduleCache::getModule($modules[$i], 'none');
		$errors = $m->check_configOptions($scopes[$modules[$i]], $options);
		if (isset($errors) && is_array($errors)) {
			$return = array_merge($return, $errors);
		}
	}
	return $return;
}

/**
* Returns a help entry from an account module.
*
* @param string $module module name
* @param string $helpID help identifier
* @param string $scope account type
* @return array help entry
*/
function getHelp($module,$helpID,$scope='') {
	global $helpArray;
	if (!isset($module) || ($module == '') || ($module == 'main')) {
		$helpPath = "../help/help.inc";
		if (is_file("../../help/help.inc")) {
			$helpPath = "../../help/help.inc";
		}
		if (!isset($helpArray)) {
			include_once($helpPath);
		}
		return $helpArray[$helpID];
	}
	if (empty($scope)) {
		$scope = 'none';
	}
	$moduleObject = moduleCache::getModule($module, $scope);
	return $moduleObject->get_help($helpID);
}

/**
* Returns a list of available PDF entries.
*
* @param string $typeId account type (user, group, host)
* @return array PDF entries (field ID => field label)
*/
function getAvailablePDFFields($typeId) {
	$typeManager = new TypeManager();
	$_SESSION['pdfContainer'] = new accountContainer($typeManager->getConfiguredType($typeId), 'pdfContainer');
	$_SESSION['pdfContainer']->initModules();
	$mods = $_SESSION['pdfContainer']->getAccountModules();
	$return = array();
	foreach ($mods as $module) {
		$fields = $module->get_pdfFields($typeId);
		$moduleName = get_class($module);
		$return[$moduleName] = array();
		if (is_array($fields)) {
			foreach ($fields as $fieldID => $fieldLabel) {
				if (is_integer($fieldID)) {
					// support old PDF field list which did not contain a label
					$return[$moduleName][$fieldLabel] = $fieldLabel;
				}
				else {
					$return[$moduleName][$fieldID] = $fieldLabel;
				}
			}
		}
	}
	$return['main'] = array('dn' => _('DN'));
	unset($_SESSION['pdfContainer']);
	return $return;
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
* @param ConfiguredType $type account type
* @param array $selectedModules selected account modules
* @return array column list
*/
function getUploadColumns(&$type, $selectedModules) {
	$return = array();
	for ($i = 0; $i < sizeof($selectedModules); $i++) {
		$module = moduleCache::getModule($selectedModules[$i], $type->getScope());
		$return[$selectedModules[$i]] = $module->get_uploadColumns($selectedModules, $type);
	}
	return $return;
}

/**
* This function builds the LDAP accounts for the file upload.
*
* If there are problems status messages will be printed automatically.
*
* @param ConfiguredType $type account type
* @param array $data array containing one account in each element
* @param array $ids array(<column_name> => <column number>)
* @param array $selectedModules selected account modules
* @param htmlResponsiveRow $container HTML container
* @return mixed array including accounts or false if there were errors
*/
function buildUploadAccounts($type, $data, $ids, $selectedModules, htmlResponsiveRow $container) {
	// build module order
	$unOrdered = $selectedModules;
	$ordered = array();
	$predepends = array();
	// get dependencies
	for ($i = 0; $i < sizeof($unOrdered); $i++) {
		$mod = moduleCache::getModule($unOrdered[$i], $type->getScope());
		$predepends[$unOrdered[$i]] = $mod->get_uploadPreDepends();
	}
	// first all modules without predepends can be ordered
	for ($i = 0; $i < sizeof($unOrdered); $i++) {
		if (sizeof($predepends[$unOrdered[$i]]) == 0) {
			$ordered[] = $unOrdered[$i];
			unset($unOrdered[$i]);
			$unOrdered = array_values($unOrdered);
			$i--;
		}
	}
	$unOrdered = array_values($unOrdered);  // fix indexes
	// now add all modules with fulfilled dependencies until all are in order
	while (sizeof($unOrdered) > 0) {
		$newRound = false;
		for ($i = 0; $i < sizeof($unOrdered); $i++) {
			$deps = $predepends[$unOrdered[$i]];
			$depends = false;
			for ($d = 0; $d < sizeof($deps); $d++) {
				if (in_array($deps[$d], $unOrdered)) {
					$depends = true;
					break;
				}
			}
			if (!$depends) {  // add to order if dependencies are fulfilled
				$ordered[] = $unOrdered[$i];
				unset($unOrdered[$i]);
				$unOrdered = array_values($unOrdered);
				$newRound = true;
				break;
			}
		}
		if ($newRound) {
			continue;
		}
		// this point should never be reached, LAM was unable to find a correct module order
		$container->add(new htmlStatusMessage("ERROR", "Internal Error: Unable to find correct module order."), 12);
		return false;
	}
	// give raw data to modules
	$errors = array();
	$partialAccounts = array();
	foreach ($data as $i => $dataRow) {
		$partialAccounts[$i]['objectClass'] = array();
	}
	$stopUpload = false;
	for ($i = 0; $i < sizeof($ordered); $i++) {
		$module = new $ordered[$i]($type->getScope());
		$moduleErrors = $module->build_uploadAccounts($data, $ids, $partialAccounts, $selectedModules, $type);
		if (sizeof($moduleErrors) > 0) {
			$errors = array_merge($errors, $moduleErrors);
			foreach ($moduleErrors as $error) {
				if ($error[0] == 'ERROR') {
					array_unshift($errors, array("INFO", _("Displayed account numbers start at \"0\". Add 2 to get the row in your spreadsheet."), ""));
					$errors[] = array("ERROR", _("Upload was stopped after errors in %s module!"), "", array($module->get_alias()));
					// skip other modules if error was found
					$stopUpload = true;
					break;
				}
			}
		}
		if ($stopUpload) {
			break;
		}
	}
	if (sizeof($errors) > 0) {
		for ($i = 0; (($i < sizeof($errors)) && ($i < 50)); $i++) {
			$text = empty($errors[$i][2]) ? null : $errors[$i][2];
			$values = empty($errors[$i][3]) ? null : $errors[$i][3];
			$container->add(new htmlStatusMessage($errors[$i][0], $errors[$i][1], $text, $values), 12);
		}
	}
	if ($stopUpload) {
		return false;
	}
	return $partialAccounts;
}

/**
 * Runs any actions that need to be done before an LDAP entry is created.
 *
 * @param ConfiguredType $type account type
 * @param array $selectedModules list of selected account modules
 * @param array $attributes LDAP attributes of this entry (attributes are provided as reference, handle modifications of $attributes with care)
 * @return array array which contains status messages. Each entry is an array containing the status message parameters.
 */
function doUploadPreActions($type, $selectedModules, $attributes) {
	$messages = array();
	for ($i = 0; $i < sizeof($selectedModules); $i++) {
		$activeModule = $selectedModules[$i];
		$module = moduleCache::getModule($activeModule, $type->getScope());
		$messages = array_merge($messages, $module->doUploadPreActions($attributes, $type));
	}
	return $messages;
}

/**
* This function executes one post upload action.
*
* @param ConfiguredType $type account type
* @param array $data array containing one account in each element
* @param array $ids array(<column_name> => <column number>)
* @param array $failed list of accounts which were not created successfully
* @param array $selectedModules list of selected account modules
* @param array $accounts list of LDAP entries
* @return array current status
* <br> array (
* <br>  'status' => 'finished' | 'inProgress'
* <br>  'module' => <name of active module>
* <br>  'progress' => 0..100
* <br>  'errors' => array (<array of parameters for StatusMessage>)
* <br> )
*/
function doUploadPostActions($type, &$data, $ids, $failed, $selectedModules, &$accounts) {
	// check if function is called the first time
	if (! isset($_SESSION['mass_postActions']['remainingModules'])) {
		// make list of remaining modules
		$moduleList = $selectedModules;
		$_SESSION['mass_postActions']['remainingModules'] = $moduleList;
	}
	$activeModule = $_SESSION['mass_postActions']['remainingModules'][0];
	// initialize temporary variable
	if (!isset($_SESSION['mass_postActions'][$activeModule])) {
		$_SESSION['mass_postActions'][$activeModule] = array();
	}
	// let first module do one post action
	$module = moduleCache::getModule($activeModule, $type->getScope());
	$return = $module->doUploadPostActions($data, $ids, $failed, $_SESSION['mass_postActions'][$activeModule], $accounts, $selectedModules, $type);
	// remove active module from list if already finished
	if ($return['status'] == 'finished') {
		unset($_SESSION['mass_postActions']['remainingModules'][0]);
		$_SESSION['mass_postActions']['remainingModules'] = array_values($_SESSION['mass_postActions']['remainingModules']);
	}
	// update status and return back to upload page
	$return['module'] = $activeModule;
	if (sizeof($_SESSION['mass_postActions']['remainingModules']) > 0) {
		$return['status'] = 'inProgress';
	}
	else {
		$return['status'] = 'finished';
	}
	return $return;
}

/**
* Returns true if the module is a base module
*
* @return array required extensions
*/
function getRequiredExtensions() {
	$extList = array();
	$typeManager = new \LAM\TYPES\TypeManager();
	$types = $typeManager->getConfiguredTypes();
	foreach ($types as $type) {
		$mods = $_SESSION['config']->get_AccountModules($type->getId());
		for ($m = 0; $m < sizeof($mods); $m++) {
			$module = moduleCache::getModule($mods[$m], $type->getScope());
			$ext = $module->getRequiredExtensions();
			for ($e = 0; $e < sizeof($ext); $e++) {
				if (!in_array($ext[$e], $extList)) {
					$extList[] = $ext[$e];
				}
			}
		}
	}
	return $extList;
}

/**
* Takes a list of meta-HTML elements and prints the equivalent HTML output.
*
* The modules are not allowed to display HTML code directly but return
* meta HTML code. This allows to have a common design for all module pages.
*
* @param string $module Name of account module
* @param mixed $input htmlElement or array of htmlElement elements
* @param array $values List of values which override the defaults in $input (name => value)
* @param boolean $restricted If true then no buttons will be displayed
* @param integer $tabindex Start value of tabulator index for input fields
* @param string $scope Account type
* @return array List of input field names and their type (name => type)
*/
function parseHtml($module, $input, $values, $restricted, &$tabindex, $scope) {
	if ($input instanceof htmlElement) {
		return $input->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
	}
	if (is_array($input) && (sizeof($input) > 0)) {
		$return = array();
		for ($i = 0; $i < sizeof($input); $i++) {
			$return = array_merge($return, $input[$i]->generateHTML($module, $input, $values, $restricted, $tabindex, $scope));
		}
		return $return;
	}
	return array();
}

/**
 * Helper function to sort descriptive options in parseHTML().
 * It compares the second entries of two arrays.
 *
 * @param array $a first array
 * @param array $b second array
 * @return integer compare result
 */
function lamCompareDescriptiveOptions(&$a, &$b) {
	// check parameters
	if (!is_array($a) || !isset($a[1]) || !is_array($b) || !isset($b[1])) {
		return 0;
	}
	return strnatcasecmp($a[1], $b[1]);
}

/**
 * Prints a LAM help link.
 *
 * @param array $entry help entry
 * @param String $number help number
 * @param String $module module name
 * @param String $scope account scope
 * @param array $classes CSS classes
 */
function printHelpLink($entry, $number, $module = '', $scope = '', $classes = array()) {
	$helpPath = "../";
	if (is_file("./help.php")) {
		$helpPath = "";
	}
	$title = $entry['Headline'];
	$message = $entry['Text'];
	if (isset($entry['attr'])) {
		$message .= '<br><br><hr class="dotted">' . _('Technical name') . ': <i>' . $entry['attr'] . '</i>';
	}
	// replace special characters
	$message = htmlspecialchars($message);
	$title = htmlspecialchars($title);
	$selfServiceOption = '';
	if (isSelfService()) {
		$selfServiceOption = '&amp;selfService=1';
	}
	echo "<a class=\"margin2 " . implode(" ", $classes) . "\" href=\"" . $helpPath . "help.php?module=$module&amp;HelpNumber=". $number . "&amp;scope=" . $scope . $selfServiceOption . "\" ";
		echo "target=\"help\">";
		echo "<img helptitle=\"" . $title . "\" helpdata=\"" . $message . "\" class=\"align-middle help-icon\" src=\"../${helpPath}graphics/help.png\" alt=\"" . _('Help') . "\">";
	echo "</a>";
}


/**
* This class includes all modules and attributes of an account.
*
* @package modules
*/
class accountContainer {

	/**
	* Constructor
	*
	* @param ConfiguredType $type account type
	* @param string $base key in $_SESSION where this object is saved
	*/
	function __construct($type, $base) {
		if (!($type instanceof ConfiguredType)) {
			trigger_error('Argument of accountContainer must be ConfiguredType.', E_USER_ERROR);
		}
		if (!is_string($base)) {
			trigger_error('Argument of accountContainer must be string.', E_USER_ERROR);
		}
		$this->type = $type;
		$this->base = $base;
		// Set startpage
		$this->current_page=0;
		$this->subpage='attributes';
		$this->isNewAccount = false;
		return 0;
	}

	/**
	* Array of all used attributes
	* Syntax is attribute => array ( objectClass => MUST or MAY, ...)
	*/
	public $attributes;

	/**
	* This variale stores the account type.
	* Currently "user", "group" and "host" are supported.
	*/
	private $type;

	/** This is an array with all module objects
	 *
	 *  @var baseModule[] modules
	 */
	private $module;

	/** DN suffix of the account */
	public $dnSuffix;

	/** DN of account when it was loaded */
	public $dn_orig;

	/** RDN attribute of this account */
	public $rdn;

	/** DN of saved account */
	public $finalDN;

	/** original LDAP attributes when account was loaded from LDAP */
	public $attributes_orig;

	/** Module order */
	private $order;

	/** Name of accountContainer variable in session */
	private $base;

	/** This variable stores the page number of the currently displayed page */
	private $current_page = 0;

	/** This variable is set to the pagename of a subpage if it should be displayed */
	private $subpage;

	/** True if this is a newly created account */
	public $isNewAccount;

	/** name of last loaded account profile */
	private $lastLoadedProfile = '';

	/** cache for existing OUs */
	private $cachedOUs = null;

	/** main title in title bar */
	private $titleBarTitle = null;
	/** subtitle in title bar */
	private $titleBarSubtitle = null;
	/** send password via mail */
	private $sendPasswordViaMail = null;
	/** send password via mail to this alternate address */
	private $sendPasswordViaMailAlternateAddress = null;


	/**
	 * Returns the account module with the given class name
	 *
	 * @param string $name class name (e.g. posixAccount)
	 * @return baseModule account module
	 */
	public function getAccountModule($name) {
		if (isset($this->module[$name])) {
			return $this->module[$name];
		}
		else {
			return null;
		}
	}

	/**
	 * Returns the included account modules.
	 *
	 * @return baseModule[] modules
	 */
	public function getAccountModules() {
		return $this->module;
	}

	/**
	* Returns the account type of this object (e.g. user, group, host).
	*
	* @return ConfiguredType account type
	*/
	public function get_type() {
		return $this->type;
	}

	/**
	 * Returns the session edit key for this container.
	 *
	 * @return string key
	 */
	public function getEditKey() {
		return $this->base;
	}

	/**
	* This function is called when the user clicks on any button on the account pages.
	* It prints the HTML code of each account page.
	*/
	public function continue_main() {
		$oldPage = $this->current_page;
		$oldSubpage = $this->subpage;
		$result = array();
		$errorsOccurred = false;
		$typeObject = $this->type->getBaseType();
		$profileLoaded = $this->loadProfileIfRequested();
		if ($this->subpage == '') {
			$this->subpage='attributes';
		}
		if (isset($_POST['accountContainerReset'])) {
			$result = $this->load_account($this->dn_orig);
		}
		elseif (isset($_POST['accountContainerDelete'])) {
			metaRefresh("../lists/deletelink.php?type=" . $this->type->getId() . "&DN='" . rawurlencode($this->dn_orig) . "'");
			unset($_SESSION[$this->base]);
			exit();
		}
		elseif (!$profileLoaded) {
			// change dn suffix
			if (isset($_GET['suffix']) && ($_GET['suffix'] != '') && ($this->dnSuffix == null)) {
				$this->dnSuffix = $_GET['suffix'];
			}
			if (isset($_POST['accountContainerSuffix']) && ($_POST['accountContainerSuffix'] != '')) {
				$this->dnSuffix = $_POST['accountContainerSuffix'];
			}
			// change RDN
			if (isset($_POST['accountContainerRDN'])) {
				$this->rdn = $_POST['accountContainerRDN'];
			}
			// create another account
			if (isset($_POST['accountContainerCreateAgain'])) {
				// open fresh account page
				unset($_SESSION[$this->base]);
				metaRefresh("edit.php?type=" . $this->type->getId() . "&suffix=" . $this->dnSuffix);
				exit();
			}
			// reedit account
			if (isset($_POST['accountContainerBackToEdit'])) {
				// open fresh account page
				unset($_SESSION[$this->base]);
				metaRefresh("edit.php?type=" . $this->type->getId() . "&DN=" . urlencode($this->finalDN));
				exit();
			}
			// back to account list
			if (isset($_POST['accountContainerBackToList'])) {
				// Return to account list
				unset($_SESSION[$this->base]);
				metaRefresh("../lists/list.php?type=" . $this->type->getId() . '&accountEditBack=true');
				exit;
			}
			// create PDF file
			if (isset($_POST['accountContainerCreatePDF'])) {
				metaRefresh('../lists/list.php?printPDF=1&type=' . $this->type->getId() . "&refresh=true&PDFSessionID=" . $this->base);
				exit;
			}
			// module actions
			if ((sizeof($_POST) > 0) && checkIfWriteAccessIsAllowed($this->type->getId())) {
				$result = call_user_func(array(&$this->module[$this->order[$this->current_page]], 'process_'.$this->subpage));
				if (is_array($result)) {  // messages were returned, check for errors
					for ($i = 0; $i < sizeof($result); $i++) {
						if ($result[$i][0] == 'ERROR') {
							$errorsOccurred = true;
							break;
						}
					}
				}
				$this->sortModules();
			}
			// run type post actions
			$typeObject->runEditPagePostAction($this);
			// save account
			if (!$errorsOccurred && isset($_POST['accountContainerSaveAccount'])) {
				// check if all modules are complete
				$modules = array_keys($this->module);
				$incompleteModules = array();
				foreach ($modules as $module) {
					if (!$this->module[$module]->module_complete()) {
						$incompleteModules[] = $this->module[$module]->get_alias();
					}
				}
				if (sizeof($incompleteModules) > 0) {
					$result[] = array('INFO', _('Some required information is missing'),
						sprintf(_('Please set up all required attributes on page: %s'), implode(", ", $incompleteModules)));
				}
				else {
					// save account
					$saveMessages = $this->save_account();
					$saveOk = true;
					for ($i = 0; $i < sizeof($saveMessages); $i++) {
						if ($saveMessages[$i][0] == 'ERROR') {
							$saveOk = false;
						}
					}
					if (!$saveOk) {
						$result = $saveMessages;
					}
					else {
						$this->printSuccessPage($saveMessages);
						return;
					}
				}
			}
		}
		// change to next page
		if (is_array($result)) {  // messages were returned, check for errors
			for ($i = 0; $i < sizeof($result); $i++) {
				if ($result[$i][0] == 'ERROR') {
					$errorsOccurred = true;
					break;
				}
			}
		}
		if (!$errorsOccurred) {
			// go to subpage of current module
			$postKeys = array_keys($_POST);
			for ($p = 0; $p < sizeof($postKeys); $p++) {
				if (is_string($postKeys[$p]) && (strpos($postKeys[$p], 'form_subpage_' . $this->order[$this->current_page]) === 0)) {
					$temp = substr($postKeys[$p], strlen($this->order[$this->current_page]) + 14);
					$temp = explode('_', $temp);
					if (sizeof($temp) == 2) {
						$this->subpage = $temp[0];
					}
				}
			}
			for ($i=0; $i<count($this->order); $i++ ) {
				if (isset($_POST['form_main_'.$this->order[$i]])) {
					if ($this->module[$this->order[$i]]->module_ready()) {
						$this->current_page = $i;
						$this->subpage='attributes';
					}
					else {
						StatusMessage('ERROR', _('The module %s is not yet ready.'),
							_('Please enter the account information on the other pages first.'),
							array($this->module[$this->order[$i]]->get_alias()));
					}
				}
			}
		}
		if ($profileLoaded) {
			$profileName = $_POST['accountContainerSelectLoadProfile'];
			$result[] = array('INFO', _('Profile "%s" loaded.'), '<div class="hidden lam-dialog-msg" id="lam-make-default-profile-dlg">' . _('Setting saved') . '</div>' .
				'<a href="#" id="lam-make-default-profile" data-name="%s" data-typeid="%s" data-ok="%s">' . _('Click here to make this your default profile.') . '</a>', array($profileName, $profileName, $this->get_type()->getId(), _('Ok')));
		}
		// update titles
		$this->titleBarTitle = $typeObject->getTitleBarTitle($this);
		$this->titleBarSubtitle = $typeObject->getTitleBarSubtitle($this);
		// prints a module content page
		$this->printModuleContent($result);
		if (!$errorsOccurred && ($oldPage == $this->current_page) && ($oldSubpage == $this->subpage)
			&& isset($_POST['scrollPositionTop']) && isset($_POST['scrollPositionLeft'])) {
			// scroll to last position
			echo '<script type="text/javascript">
				jQuery(document).ready(function() {
					jQuery(window).scrollTop(' . $_POST['scrollPositionTop'] . ');
					jQuery(window).scrollLeft('. $_POST['scrollPositionLeft'] . ');
			});
			</script>';
		}
		$this->printPageFooter();
	}

	/**
	 * Prints the content part provided by the current module.
	 *
	 * @param array $result list of messages
	 */
	private function printModuleContent($result) {
		$tabindex = 1;
		$this->printPageHeader();
		$this->printPasswordPromt();
		// display error messages
		if (is_array($result)) {
			for ($i=0; $i<sizeof($result); $i++) {
				call_user_func_array("StatusMessage", $result[$i]);
			}
		}
		echo '<div id="passwordMessageArea"></div>';
		echo "<table border=0 width=\"100%\" style=\"border-collapse: collapse;\">\n";
		if (checkIfWriteAccessIsAllowed($this->type->getId())) {
			echo "<tr><td style=\"padding: 5px 0px 0px 0px;\">\n";
			$this->printCommonControls($tabindex);
			echo "</td></tr>\n";
		}
		echo "<tr valign=\"top\"><td style=\"padding: 15px;\">";
		// print title bar
		$titleBarContainer = new htmlResponsiveRow();
		$titleBarContainer->setCSSClasses(array('maxrow'));
		$titleBarTitleText = new htmlOutputText($this->titleBarTitle, false);
		$titleBarContainer->add(new htmlDiv(null, $titleBarTitleText, array('titleBarTitle', 'text-left')), 12, 12, 4);
		$titleBarSubtitleText = new htmlOutputText($this->titleBarSubtitle, false);
		$titleBarContainer->add(new htmlDiv(null, $titleBarSubtitleText, array('titleBarSubtitle', 'responsiveLabel')), 12, 12, 8);
		$titleBarSuffixRdn = new htmlResponsiveRow();
		$titleBarSuffixRdn->add(new htmlHorizontalLine(), 12);
		// suffix
		$suffixList = array();
		foreach ($this->getOUs() as $suffix) {
			$suffixList[getAbstractDN($suffix)] = $suffix;
		}
		if (!($this->dnSuffix == '') && !in_array_ignore_case($this->dnSuffix, $this->getOUs())) {
			$suffixList[getAbstractDN($this->dnSuffix)] = $this->dnSuffix;
		}
		$selectedSuffix = array($this->dnSuffix);
		$suffixSelect = new htmlResponsiveSelect('accountContainerSuffix', $suffixList, $selectedSuffix, _('Suffix'));
		$suffixSelect->setHasDescriptiveElements(true);
		$suffixSelect->setRightToLeftTextDirection(true);
		$suffixSelect->setShortLabel();
		$suffixSelect->setSortElements(false);
		$titleBarSuffixRdn->add($suffixSelect, 12, 12, 7);
		// RDN selection
		$rdnlist = getRDNAttributes($this->type->getId());
		$rdnSelect = new htmlResponsiveSelect('accountContainerRDN', $rdnlist, array($this->rdn), _('RDN identifier'), '400');
		$rdnSelect->setShortLabel();
		$titleBarSuffixRdn->add($rdnSelect, 12, 12, 5);
		$titleBarContainer->add($titleBarSuffixRdn, 12);

		$titleBarDiv = new htmlDiv(null, $titleBarContainer, array('titleBar', 'ui-corner-top'));
		parseHtml(null, $titleBarDiv, array(), false, $tabindex, $this->type->getScope());

		echo '<div id="lamVerticalTabs" class="ui-tabs ui-widget ui-widget-content ui-corner-bottom ui-helper-clearfix">';
		echo '<table class="fullwidth">';
			echo '<tr><td style="vertical-align: top;">';
			// tab menu
			$this->printModuleTabs();
			echo '</td><td style="vertical-align: top;" width="100%">';
			echo "<div class=\"ui-tabs-panel ui-widget-content ui-corner-bottom fullwidth module-content-row\">\n";
			// content area
			// display html-code from modules
			$return = call_user_func(array($this->module[$this->order[$this->current_page]], 'display_html_'.$this->subpage));
			$y = 5000;
			parseHtml($this->order[$this->current_page], $return, array(), false, $y, $this->type->getScope());
			echo "</div>\n";
			echo '</td>';
			echo '</tr>';
		echo '</table>';
		echo "</div>\n";
		echo "</td></tr>\n";
		// Display rest of html-page
		echo "</table>\n";
	}

	/**
	 * Prints the input fields of the central password service.
	 */
	private function printPasswordPromt() {
		echo "<div id=\"passwordDialog\" class=\"hidden\">\n";
		echo '<div id="passwordDialogMessageArea"></div>';
		$printContainer = false;
		$container = new htmlTable();
		// password fields
		$container->addElement(new htmlOutputText(_('Password')));
		$pwdInput1 = new htmlInputField('newPassword1');
		$pwdInput1->setIsPassword(true, true);
		$container->addElement($pwdInput1);
		$container->addElement(new htmlHelpLink('404'), true);
		$container->addElement(new htmlOutputText(_('Repeat password')));
		$pwdInput2 = new htmlInputField('newPassword2');
		$pwdInput2->setIsPassword(true);
		$pwdInput2->setSameValueFieldID('newPassword1');
		$container->addElement($pwdInput2, true);
		// print force password change option
		$forceChangeSupported = array();
		foreach ($this->module as $name => $module) {
			if (($module instanceof passwordService) && $module->supportsForcePasswordChange()) {
				$forceChangeSupported[] = $module;
			}
		}
		if (!empty($forceChangeSupported)) {
			$container->addElement(new htmlOutputText(_('Force password change')));
			$forcePwdGroup = new htmlGroup();
			$forcePwdGroup->addElement(new htmlInputCheckbox('lamForcePasswordChange', false));
			$forcePwdGroup->addElement(new htmlSpacer('1rem', null));
			foreach ($forceChangeSupported as $module) {
				$forcePwdGroup->addElement(new htmlImage('../../graphics/' . $module->getIcon(), '16px', '16px', $module->get_alias(), $module->get_alias()));
				$forcePwdGroup->addElement(new htmlSpacer('0.1rem', null));
			}
			$container->addElement($forcePwdGroup);
			$container->addElement(new htmlHelpLink('406'), true);
		}
		if (isLAMProVersion() && (isset($this->attributes_orig['mail'][0]) || $this->anyModuleManagesMail())) {
			$pwdMailCheckbox = new htmlTableExtendedInputCheckbox('lamPasswordChangeSendMail', false, _('Send via mail'));
			$pwdMailCheckbox->setTableRowsToShow(array('lamPasswordChangeSendMailAddress'));
			$container->addElement($pwdMailCheckbox);
			$container->addElement(new htmlHelpLink('407'), true);
			if ($_SESSION['config']->getLamProMailAllowAlternateAddress() != 'false') {
				$alternateMail = '';
				$pwdResetModule = $this->getAccountModule('passwordSelfReset');
				if (!empty($pwdResetModule)) {
					$backupMail = $pwdResetModule->getBackupEmail();
					if (!empty($backupMail)) {
						$alternateMail = $pwdResetModule->getBackupEmail();
					}
				}
				$container->addElement(new htmlTableExtendedInputField(_('Alternate recipient'), 'lamPasswordChangeSendMailAddress', $alternateMail, '410'));
			}
		}
		$container->addElement(new htmlSpacer(null, '10px'), true);
		// password modules
		$moduleContainer = new htmlTable();
		foreach ($this->module as $name => $module) {
			if (($module instanceof passwordService) && $module->managesPasswordAttributes()) {
				$printContainer = true;
				$buttonImage = $module->getIcon();
				if ($buttonImage != null) {
					if (!(strpos($buttonImage, 'http') === 0) && !(strpos($buttonImage, '/') === 0)) {
						$buttonImage = '../../graphics/' . $buttonImage;
					}
					$moduleContainer->addElement(new htmlImage($buttonImage, null, null, getModuleAlias($name, $this->type->getScope())));
				}
				$moduleContainer->addElement(new htmlTableExtendedInputCheckbox('password_cb_' . $name, true, getModuleAlias($name, $this->type->getScope()), null, false));
				$moduleContainer->addElement(new htmlSpacer('10px', null));
			}
		}
		$moduleContainer->colspan = 5;
		$container->addElement($moduleContainer, true);
		// generate HTML
		$tabindex = 2000;
		if ($printContainer) {
			parseHtml(null, $container, array(), false, $tabindex, $this->type->getScope());
		}
		echo "</div>\n";
	}

	/**
	 * Sets the new password in all selected account modules.
	 *
	 * @param array $input input parameters
	 */
	public function setNewPassword($input) {
		$password1 = $input['password1'];
		$password2 = $input['password2'];
		$random = $input['random'];
		$modules = $input['modules'];
		for ($m = 0; $m < sizeof($modules); $m++) {
			$modules[$m] = str_replace('password_cb_', '', $modules[$m]);
		}
		$return = array(
			'messages' => '',
			'errorsOccurred' => 'false'
		);
		$sendMail = false;
		if (isset($input['sendMail']) && ($input['sendMail'] == 'true')) {
			$sendMail = true;
		}
		if ($random == 'true') {
			$password1 = generateRandomPassword();
			if (!$sendMail) {
				$return['messages'] .= StatusMessage('INFO', _('The password was set to:') . ' ' . htmlspecialchars($password1), '', array(), true);
			}
		}
		else {
			// check if passwords match
			if ($password1 != $password2) {
				$return['messages'] .= StatusMessage('ERROR', _('Passwords are different!'), '', array(), true);
				$return['errorsOccurred'] = 'true';
			}
			// check passsword strength
			$pwdPolicyResult = checkPasswordStrength($password1, null, null);
			if ($pwdPolicyResult !== true) {
				$return['messages'] .= StatusMessage('ERROR', $pwdPolicyResult, '', array(), true);
				$return['errorsOccurred'] = 'true';
			}
		}
		$forcePasswordChange = false;
		if (isset($input['forcePasswordChange']) && ($input['forcePasswordChange'] == 'true')) {
			$forcePasswordChange = true;
		}
		$return['forcePasswordChange'] = $forcePasswordChange;
		if ($return['errorsOccurred'] == 'false') {
			// set new password
			foreach ($this->module as $module) {
				if ($module instanceof passwordService) {
					$messages = $module->passwordChangeRequested($password1, $modules, $forcePasswordChange);
					for ($m = 0; $m < sizeof($messages); $m++) {
						if ($messages[$m][0] == 'ERROR') {
							$return['errorsOccurred'] = 'true';
						}
						if (sizeof($messages[$m]) == 2) {
							$return['messages'] .= StatusMessage($messages[$m][0], $messages[$m][1], '', array(), true);
						}
						elseif (sizeof($messages[$m]) == 3) {
							$return['messages'] .= StatusMessage($messages[$m][0], $messages[$m][1], $messages[$m][2], array(), true);
						}
						elseif (sizeof($messages[$m]) == 4) {
							$return['messages'] .= StatusMessage($messages[$m][0], $messages[$m][1], $messages[$m][2], $messages[$m][3], true);
						}
					}
				}
			}
		}
		if (isLAMProVersion() && $sendMail) {
			$this->sendPasswordViaMail = $password1;
			if (($_SESSION['config']->getLamProMailAllowAlternateAddress() != 'false') && !empty($input['sendMailAlternateAddress'])) {
				if (!get_preg($input['sendMailAlternateAddress'], 'email')) {
					$return['messages'] .= StatusMessage('ERROR', _('Alternate recipient'), _('Please enter a valid email address!'), array(), true);
					$return['errorsOccurred'] = 'true';
				}
				$this->sendPasswordViaMailAlternateAddress = $input['sendMailAlternateAddress'];
			}
		}
		if ($return['errorsOccurred'] == 'false') {
			$return['messages'] .= StatusMessage('INFO', _('The new password will be stored in the directory after you save this account.'), '', array(), true);
		}
		return $return;
	}

	/**
	 * Returns if any module manages the mail attribute.
	 *
	 * @return boolean mail is managed
	 */
	private function anyModuleManagesMail() {
		foreach ($this->module as $mod) {
			if (in_array('mail', $mod->getManagedAttributes($this->get_type()->getId()))) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Prints common controls like the save button and the ou selection.
	 *
	 * @param int $tabindex tabindex for GUI elements
	 */
	private function printCommonControls(&$tabindex) {
		$row = new htmlResponsiveRow();
		$row->setCSSClasses(array('maxrow'));
		$leftButtonGroup = new htmlGroup();
		$leftButtonGroup->alignment = htmlElement::ALIGN_LEFT;
		// save button
		$saveButton = new htmlButton('accountContainerSaveAccount', _('Save'));
		$saveButton->setCSSClasses(array('fullwidth-mobile-only lam-primary'));
		$leftButtonGroup->addElement($saveButton);
		$leftButtonGroup->addElement(new htmlSpacer('1px', null));
		// set password button
		if ($this->showSetPasswordButton()) {
			$leftButtonGroup->addElement(new htmlSpacer('15px', null));
			$passwordButton = new htmlButton('accountContainerPassword', _('Set password'));
			$passwordButton->setCSSClasses(array('fullwidth-mobile-only lam-secondary'));
			$passwordButton->setOnClick('passwordShowChangeDialog(\'' . _('Set password') . '\', \'' . _('Ok') . '\', \''
				. _('Cancel') . '\', \'' . _('Set random password') . '\', \'../misc/ajax.php?function=passwordChange&editKey=' . htmlspecialchars($this->base) . '\',\''
				. getSecurityTokenName() . '\',\'' . getSecurityTokenValue() . '\');');
			$leftButtonGroup->addElement($passwordButton);
		}
		// delete button
		if (!$this->isNewAccount && checkIfDeleteEntriesIsAllowed($this->get_type()->getId())) {
			$leftButtonGroup->addElement(new htmlSpacer('15px', null));
			$deleteButton = new htmlButton('accountContainerDelete', _('Delete'));
			$deleteButton->setCSSClasses(array('fullwidth-mobile-only lam-danger'));
			$leftButtonGroup->addElement($deleteButton);
		}
		$leftButtonGroup->addElement(new htmlSpacer('15px', null));
		// reset button
		if (!$this->isNewAccount) {
			$resetButton = new htmlButton('accountContainerReset', _('Reset changes'));
			$resetButton->setCSSClasses(array('fullwidth-mobile-only'));
			$leftButtonGroup->addElement($resetButton);
		}
		$type = $this->type->getBaseType();
		$backToListButton = new htmlButton('accountContainerBackToList', $type->LABEL_BACK_TO_ACCOUNT_LIST);
		$backToListButton->setCSSClasses(array('fullwidth-mobile-only'));
		$leftButtonGroup->addElement($backToListButton);
		$row->add($leftButtonGroup, 12, 9);

		$rightGroup = new htmlGroup();
		// profile selection
		$accountProfilePersistenceManager = new AccountProfilePersistenceManager();
		$profilelist = $accountProfilePersistenceManager->getAccountProfileNames($this->type->getId(), $_SESSION['config']->getName());
		if (sizeof($profilelist) > 0) {
			$profilesSelect = new htmlSelect('accountContainerSelectLoadProfile', $profilelist, array($this->lastLoadedProfile));
			$profilesSelect->setCSSClasses(array('auto-width'));
			$rightGroup->addElement($profilesSelect);
			$profileButton = new htmlButton('accountContainerLoadProfile', _('Load profile'));
			if (!$this->isNewAccount) {
				$profileButton->setType('submit');
				$profileButton->setOnClick('confirmOrStopProcessing(\'' . _('This may overwrite existing values with profile data. Continue?') . '\', event);');
			}
			$rightGroup->addElement($profileButton);
			$rightGroup->addElement(new htmlSpacer('1px', null));
			$rightGroup->addElement(new htmlHelpLink('401'));
		}
		$row->add($rightGroup, 12, 3, 3, 'text-right');

		parseHtml(null, $row, array(), false, $tabindex, $this->type->getScope());
	}

	/**
	 * Returns if the page should show a button to set the password.
	 *
	 * @return boolean show or hide button
	 */
	private function showSetPasswordButton() {
		foreach ($this->module as $module) {
			if (($module instanceof passwordService) && $module->managesPasswordAttributes()) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Prints the header of the account pages.
	 */
	private function printPageHeader() {
		if (!empty($_POST)) {
			validateSecurityToken();
		}
		include '../../lib/adminHeader.inc';
		echo '<script type="text/javascript">
				jQuery(document).ready(function() {
					jQuery("#inputForm").validationEngine({promptPosition: "topLeft", addFailureCssClassToField: "lam-input-error", autoHidePrompt: true, autoHideDelay: 5000});
				});
			</script>';
		echo "<form id=\"inputForm\" enctype=\"multipart/form-data\" action=\"edit.php?editKey=" . htmlspecialchars($this->base) . "\" method=\"post\" onSubmit=\"saveScrollPosition('inputForm')\" autocomplete=\"off\">\n";
		echo '<input type="hidden" name="' . getSecurityTokenName() . '" value="' . getSecurityTokenValue() . '">';
	}

	/**
	 * Prints the footer of the account pages.
	 */
	private function printPageFooter() {
		echo "</form>\n";
		include '../../lib/adminFooter.inc';
	}

	/**
	 * Prints the HTML code to notify the user about the successful saving.
	 *
	 * @param array $messages array which contains status messages. Each entry is an array containing the status message parameters.
	 */
	private function printSuccessPage($messages) {
		$this->printPageHeader();
		// Show success message
		if ($this->dn_orig == '') {
			$text = _("Account was created successfully.");
		}
		else {
			$text = _("Account was modified successfully.");
		}
		echo "<div smallPaddingContent\">";

		$container = new htmlResponsiveRow();
		$container->addVerticalSpacer('2rem');
		// show messages
		for ($i = 0; $i < sizeof($messages); $i++) {
			if (sizeof($messages[$i]) == 2) {
				$message = new htmlStatusMessage($messages[$i][0], $messages[$i][1]);
				$container->add($message);
			}
			else {
				$message = new htmlStatusMessage($messages[$i][0], $messages[$i][1], $messages[$i][2]);
				$container->add($message);
			}
		}
		$message = new htmlStatusMessage('INFO', _('LDAP operation successful.'), $text);
		$container->add($message);
		$container->addVerticalSpacer('2rem');

		$type = $this->type->getBaseType();
		$buttonGroup = new htmlGroup();
		if (checkIfNewEntriesAreAllowed($this->type->getId())) {
			$createButton = new htmlButton('accountContainerCreateAgain', $type->LABEL_CREATE_ANOTHER_ACCOUNT);
			$createButton->setCSSClasses(array('lam-primary'));
			$buttonGroup->addElement($createButton);
			$buttonGroup->addElement(new htmlSpacer('10px', null));
		}
		$pdfButton = new htmlButton('accountContainerCreatePDF', _('Create PDF file'));
		$pdfButton->setCSSClasses(array('lam-secondary'));
		$buttonGroup->addElement($pdfButton);
		$buttonGroup->addElement(new htmlSpacer('10px', null));
		$backToListButton = new htmlButton('accountContainerBackToList', $type->LABEL_BACK_TO_ACCOUNT_LIST);
		$buttonGroup->addElement($backToListButton);
		$buttonGroup->addElement(new htmlSpacer('10px', null));
		$backToEditButton = new htmlButton('accountContainerBackToEdit', _('Edit again'));
		$buttonGroup->addElement($backToEditButton);
		$container->add($buttonGroup);

		$tabindex = 1;
		parseHtml(null, $container, array(), false, $tabindex, $this->type->getScope());

		echo "</div>\n";
		$this->printPageFooter();
	}

	/**
	 * Checks if the user requested to load a profile.
	 *
	 * @return boolean true, if profile was loaded
	 */
	private function loadProfileIfRequested() {
		if (isset($_POST['accountContainerLoadProfile']) && isset($_POST['accountContainerSelectLoadProfile'])) {
			$accountProfilePersistenceManager = new AccountProfilePersistenceManager();
			$profile = $accountProfilePersistenceManager->loadAccountProfile($this->type->getId(), $_POST['accountContainerSelectLoadProfile'], $_SESSION['config']->getName());
			$this->lastLoadedProfile = $_POST['accountContainerSelectLoadProfile'];
			// pass profile to each module
			$modules = array_keys($this->module);
			foreach ($modules as $module) {
				$this->module[$module]->load_profile($profile);
			}
			if (isset($profile['ldap_rdn'][0])
					&& in_array($profile['ldap_rdn'][0], getRDNAttributes($this->type->getId()))) {
				$this->rdn = $profile['ldap_rdn'][0];
			}
			if (isset($profile['ldap_suffix'][0]) && ($profile['ldap_suffix'][0] != '-')) {
				$this->dnSuffix = $profile['ldap_suffix'][0];
			}
			return true;
		}
		return false;
	}

	/**
	 * Prints the HTML code of the module tabs.
	 */
	private function printModuleTabs() {
		// $x is used to count up tabindex
		$x=1;
		echo '<ul class="ui-tabs-nav ui-helper-reset ui-helper-clearfix ui-widget-header ui-corner-all">';
		// Loop for each module
		for ($i = 0; $i < count($this->order); $i++) {
			$buttonStatus = $this->module[$this->order[$i]]->getButtonStatus();
			$alias = $this->module[$this->order[$i]]->get_alias();
			// skip hidden buttons
			if ($buttonStatus == 'hidden') {
				continue;
			}
			$buttonImage = $this->module[$this->order[$i]]->getIcon();
			$activatedClass = '';
			if ($this->order[$this->current_page] == $this->order[$i]) {
				$activatedClass = ' lam-tabs-selected';
			}
			// print button
			echo '<li class="ui-state-default ui-corner-left' . $activatedClass . '">';
			echo "<button class=\"lam-account-type\" name=\"form_main_".$this->order[$i]."\"";
			echo " tabindex=$x";
			if ($buttonStatus == 'disabled') {
				echo " disabled";
			}
			echo ' onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');">';
			if ($buttonImage != null) {
				if (!(strpos($buttonImage, 'http') === 0) && !(strpos($buttonImage, '/') === 0)) {
					$buttonImage = '../../graphics/' . $buttonImage;
				}
				echo "<img height=32 width=32 class=\"align-middle\" style=\"padding: 3px;\" alt=\"\" src=\"$buttonImage\">&nbsp;";
			}
			echo '<span class="hide-on-mobile">';
			echo $alias;
			echo '<span>';
			echo "&nbsp;</button>\n";
			echo "</li>\n";
			$x++;
		}
		echo '</ul>';
	}

	/**
	* This function checks which LDAP attributes have changed while the account was edited.
	*
	* @param array $attributes list of current LDAP attributes
	* @param array $orig list of old attributes when account was loaded
	* @return array an array which can be passed to $this->saveAccount()
	*/
	function save_module_attributes($attributes, $orig) {
		$return = array();
		$toadd = array();
		$tomodify = array();
		$torem = array();
		$notchanged = array();
		// get list of all attributes
		$attr_names = array_keys($attributes);
		$orig_names = array_keys($orig);
		// find deleted attributes (in $orig but no longer in $attributes)
		foreach ($orig_names as $i => $value) {
			if (!isset($attributes[$value])) {
				$torem[$value] = $orig[$value];
			}
		}
		// find changed attributes
		foreach ($attr_names as $i => $name) {
			// find deleted attributes
			if (isset($orig[$name]) && is_array($orig[$name])) {
				foreach ($orig[$name] as $value) {
					if (is_array($attributes[$name])) {
						if (!in_array($value, $attributes[$name], true)
								&& ($value !== null)
								&& ($value !== '')) {
							$torem[$name][] = $value;
						}
					}
					elseif (($value !== null) && ($value !== '')) {
						$torem[$name][] = $value;
					}
				}
			}
			// find new attributes
			if (isset($attributes[$name]) && is_array($attributes[$name])) {
				foreach ($attributes[$name] as $value) {
					if (isset($orig[$name]) && is_array($orig[$name])) {
						if (!in_array($value, $orig[$name], true)
								&& ($value !== null)
								&& ($value !== ''))
							$toadd[$name][] = $value;
					}
					elseif (($value !== null) && ($value !== '')) {
						$toadd[$name][] = $value;
					}
				}
			}
			// find unchanged attributes
			if (isset($orig[$name]) && is_array($orig[$name]) && is_array($attributes[$name])) {
				foreach ($attributes[$name] as $value) {
					if (($value !== null) && ($value !== '') && in_array($value, $orig[$name], true)) {
						$notchanged[$name][] = $value;
					}
				}
			}
		}
		// create modify with add and remove
		$attributes2 = array_keys($toadd);
		for ($i=0; $i<count($attributes2); $i++) {
			if (isset($torem[$attributes2[$i]]) && (count($toadd[$attributes2[$i]]) > 0) && (count($torem[$attributes2[$i]]) > 0)) {
				// found attribute which should be modified
				$tomodify[$attributes2[$i]] = $toadd[$attributes2[$i]];
				// merge unchanged values
				if (isset($notchanged[$attributes2[$i]])) {
					$tomodify[$attributes2[$i]] = array_merge($tomodify[$attributes2[$i]], $notchanged[$attributes2[$i]]);
					unset($notchanged[$attributes2[$i]]);
				}
				// remove old add and remove commands
				unset($toadd[$attributes2[$i]]);
				unset($torem[$attributes2[$i]]);
			}
		}
		if (count($toadd) > 0) {
			$return[$this->dn_orig]['add'] = $toadd;
		}
		if (count($torem) > 0) {
			$return[$this->dn_orig]['remove'] = $torem;
		}
		if (count($tomodify) > 0) {
			$return[$this->dn_orig]['modify'] = $tomodify;
		}
		if (count($notchanged) > 0) {
			$return[$this->dn_orig]['notchanged'] = $notchanged;
		}
		return $return;
	}

	/**
	* Loads an LDAP account with the given DN.
	*
	* @param string $dn the DN of the account
	* @param array $infoAttributes list of additional informational attributes that are added to the LDAP attributes
	* E.g. this is used to inject the clear text password in the file upload. Informational attribute names must start with "INFO.".
	* @return array error messages
	*/
	function load_account($dn, $infoAttributes = array()) {
		logNewMessage(LOG_DEBUG, "Edit account " . $dn);
		$this->module = array();
		$modules = $_SESSION['config']->get_AccountModules($this->type->getId());
		$filter = 'objectClass=*';
		$searchAttrs = array('*', '+');
		foreach ($modules as $module) {
			$modTmp = new $module($this->type->getScope());
			$searchAttrs = array_merge($searchAttrs, $modTmp->getManagedHiddenAttributes($this->type->getId()));
		}
		$result = @ldap_read($_SESSION['ldap']->server(), $dn, $filter, $searchAttrs, 0, 0, 0, LDAP_DEREF_NEVER);
		if (!$result) {
			return array(array("ERROR", _("Unable to load LDAP entry:") . " " . htmlspecialchars($dn), getDefaultLDAPErrorString($_SESSION['ldap']->server())));
		}
		$entry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
		if (!$entry) {
			return array(array("ERROR", _("Unable to load LDAP entry:") . " " . htmlspecialchars($dn), getDefaultLDAPErrorString($_SESSION['ldap']->server())));
		}
		$this->dnSuffix = extractDNSuffix($dn);
		$this->dn_orig = $dn;
		// extract RDN
		$this->rdn = extractRDNAttribute($dn);
		$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		$attr = array($attr);
		cleanLDAPResult($attr);
		$attr = $attr[0];
		// fix spelling errors
		$attr = $this->fixLDAPAttributes($attr, $modules);
		// get binary attributes
		$binaryAttr = array('jpegPhoto');
		for ($i = 0; $i < sizeof($binaryAttr); $i++) {
			if (isset($attr[$binaryAttr[$i]][0])) {
				$binData = ldap_get_values_len($_SESSION['ldap']->server(), $entry, $binaryAttr[$i]);
				unset($binData['count']);
				$attr[$binaryAttr[$i]] = $binData;
			}
		}
		// add informational attributes
		$attr = array_merge($attr, $infoAttributes);
		// save original attributes
		$this->attributes_orig = $attr;

		foreach ($modules as $module) {
			if (!isset($this->module[$module])) {
				$this->module[$module] = new $module($this->type->getScope());
				$this->module[$module]->init($this->base);
			}
			$this->module[$module]->load_attributes($attr);
		}

		// sort module buttons
		$this->sortModules();
		// get titles
		$typeObject = $this->type->getBaseType();
		$this->titleBarTitle = $typeObject->getTitleBarTitle($this);
		$this->titleBarSubtitle = $typeObject->getTitleBarSubtitle($this);
		return array();
	}

	/**
	 * Fixes spelling errors in the attribute names.
	 *
	 * @param array $attributes LDAP attributes
	 * @param array $modules list of active modules
	 * @return array fixed attributes
	 */
	function fixLDAPAttributes($attributes, $modules) {
		if (!is_array($attributes)) {
			return $attributes;
		}
		$keys = array_keys($attributes);
		// get correct object class names, aliases and attributes
		$objectClasses = array();
		$aliases = array();
		$ldapAttributesTemp = array();
		foreach ($modules as $module) {
			$moduleObj = moduleCache::getModule($module, $this->type->getScope());
			$objectClasses = array_merge($objectClasses, $moduleObj->getManagedObjectClasses($this->type->getId()));
			$aliases = array_merge($aliases, $moduleObj->getLDAPAliases($this->type->getId()));
			$ldapAttributesTemp = array_merge($ldapAttributesTemp,
				$moduleObj->getManagedAttributes($this->type->getId()),
				$moduleObj->getManagedHiddenAttributes($this->type->getId()));
		}
		// build lower case attribute names
		$ldapAttributes = array();
		for ($i = 0; $i < sizeof($ldapAttributesTemp); $i++) {
			$ldapAttributes[strtolower($ldapAttributesTemp[$i])] = $ldapAttributesTemp[$i];
			unset($ldapAttributes[$i]);
		}
		$ldapAttributesKeys = array_keys($ldapAttributes);
		// convert alias names to lower case (for easier comparison)
		$aliasKeys = array_keys($aliases);
		for ($i = 0; $i < sizeof($aliasKeys); $i++) {
			if ($aliasKeys[$i] != strtolower($aliasKeys[$i])) {
				$aliases[strtolower($aliasKeys[$i])] = $aliases[$aliasKeys[$i]];
				unset($aliases[$aliasKeys[$i]]);
				$aliasKeys[$i] = strtolower($aliasKeys[$i]);
			}
		}
		// fix object classes and attributes
		for ($i = 0; $i < sizeof($keys); $i++) {
			// check object classes
			if (strtolower($keys[$i]) == 'objectclass') {
				// fix object class attribute
				if ($keys[$i] != 'objectClass') {
					$temp = $attributes[$keys[$i]];
					unset($attributes[$keys[$i]]);
					$attributes['objectClass'] = $temp;
				}
				// fix object classes
				for ($attrClass = 0; $attrClass < sizeof($attributes['objectClass']); $attrClass++) {
					for ($modClass = 0; $modClass < sizeof($objectClasses); $modClass++) {
						if (strtolower($attributes['objectClass'][$attrClass]) == strtolower($objectClasses[$modClass])) {
							if ($attributes['objectClass'][$attrClass] != $objectClasses[$modClass]) {
								unset($attributes['objectClass'][$attrClass]);
								$attributes['objectClass'][] = $objectClasses[$modClass];
							}
							break;
						}
					}
				}
			}
			else {
				// fix aliases
				if (in_array(strtolower($keys[$i]), $aliasKeys)) {
					$attributes[$aliases[strtolower($keys[$i])]] = $attributes[$keys[$i]];
					unset($attributes[$keys[$i]]);
				}
				// fix attribute names
				elseif (in_array(strtolower($keys[$i]), $ldapAttributesKeys)) {
					if ($keys[$i] != $ldapAttributes[strtolower($keys[$i])]) {
						$attributes[$ldapAttributes[strtolower($keys[$i])]] = $attributes[$keys[$i]];
						unset($attributes[$keys[$i]]);
					}
				}
			}
		}
		return $attributes;
	}

	/**
	* This function will prepare the object for a new account.
	*/
	public function new_account() {
		logNewMessage(LOG_DEBUG, "New account with type " . $this->type->getId());
		$this->isNewAccount = true;
		$this->lastLoadedProfile = 'default';
		$this->initModules();
		// sort module buttons
		$this->sortModules();
		$profileName = 'default';
		$profileCookieKey = 'defaultProfile_' . $this->get_type()->getId();
		try {
			$accountProfilePersistenceManager = new AccountProfilePersistenceManager();
			if (!empty($_COOKIE[$profileCookieKey])) {
				$cookieProfileName = $_COOKIE[$profileCookieKey];
				if ($accountProfilePersistenceManager->isAccountProfileExisting($this->get_type()->getId(), $cookieProfileName, $_SESSION['config']->getName())) {
					$profileName = $cookieProfileName;
					$this->lastLoadedProfile = $cookieProfileName;
				}
			}
			$profile = $accountProfilePersistenceManager->loadAccountProfile($this->type->getId(), $profileName, $_SESSION['config']->getName());
			// pass profile to each module
			$modules = array_keys($this->module);
			foreach ($modules as $module) {
				$this->module[$module]->load_profile($profile);
			}
			if (isset($profile['ldap_rdn'][0]) && in_array($profile['ldap_rdn'][0], getRDNAttributes($this->type->getId()))) {
				$this->rdn = $profile['ldap_rdn'][0];
			}
			if (isset($profile['ldap_suffix'][0]) && ($profile['ldap_suffix'][0] != '-')) {
				$this->dnSuffix = $profile['ldap_suffix'][0];
			}
		}
		catch (LAMException $e) {
			logNewMessage(LOG_ERR, $e->getTitle());
		}
		// get titles
		$typeObject = $this->type->getBaseType();
		$this->titleBarTitle = $typeObject->getTitleBarTitle($this);
		$this->titleBarSubtitle = $typeObject->getTitleBarSubtitle($this);
		return 0;
	}

	/**
	 * Copies the data from the given account to this one.
	 *
	 * @param string $copyDn DN to copy from
	 */
	public function copyFromExistingAccount(string $copyDn) : void {
		$copyData = ldapGetDN($copyDn, array('*', '+'));
		$modules = $_SESSION['config']->get_AccountModules($this->type->getId());
		$copyData = $this->fixLDAPAttributes($copyData, $modules);
		foreach ($this->module as $accountModule) {
			$accountModule->loadAttributesFromAccountCopy($copyData);
		}
	}

	/**
	 * Creates the account modules and initializes them.
	 */
	public function initModules() {
		$modules = $_SESSION['config']->get_AccountModules($this->type->getId());
		foreach ($modules as $module) {
			$this->module[$module] = new $module($this->type->getScope());
			$this->module[$module]->init($this->base);
		}
	}

	/**
	* This function will save an account to the LDAP database.
	*
	* @return array list of status messages
	*/
	private function save_account() {
		if (!checkIfWriteAccessIsAllowed($this->type->getId())) {
			die();
		}
		$this->finalDN = $this->dn_orig;
		$errors = array();
		$module = array_keys($this->module);
		$attributes = array();
		// load attributes
		foreach ($module as $singlemodule) {
			// load changes
			$temp = $this->module[$singlemodule]->save_attributes();
			if (!is_array($temp)) {
				$temp = array();
			}
			// merge changes
			$DNs = array_keys($temp);
			if (is_array($temp)) {
				$attributes = array_merge_recursive($temp, $attributes);
			}
			for ($i = 0; $i < count($DNs); $i++) {
				$ops = array_keys($temp[$DNs[$i]]);
				for ($j=0; $j<count($ops); $j++) {
					$attrs = array_keys($temp[$DNs[$i]][$ops[$j]]);
					for ($k=0; $k<count($attrs); $k++) {
						$attributes[$DNs[$i]][$ops[$j]][$attrs[$k]] = array_values(array_unique($attributes[$DNs[$i]][$ops[$j]][$attrs[$k]]));
					}
				}
			}
		}
		// build DN for new accounts and change it for existing ones if needed
		if (isset($attributes[$this->dn_orig]['modify'][$this->rdn][0])) {
			$this->finalDN = $this->rdn . '=' . ldap_escape($attributes[$this->dn_orig]['modify'][$this->rdn][0], '', LDAP_ESCAPE_DN) . ',' . $this->dnSuffix;
			if ($this->dn_orig != $this->finalDN) {
				$attributes[$this->finalDN] = $attributes[$this->dn_orig];
				unset($attributes[$this->dn_orig]);
			}
		}
		elseif (isset($attributes[$this->dn_orig]['add'][$this->rdn][0])) {
			$this->finalDN = $this->rdn . '=' . ldap_escape($attributes[$this->dn_orig]['add'][$this->rdn][0], '', LDAP_ESCAPE_DN) . ',' . $this->dnSuffix;
			if ($this->dn_orig != $this->finalDN) {
				$attributes[$this->finalDN] = $attributes[$this->dn_orig];
				unset($attributes[$this->dn_orig]);
			}
		}
		elseif (isset($attributes[$this->dn_orig]['remove'][$this->rdn][0]) && isset($attributes[$this->dn_orig]['notchanged'][$this->rdn][0])) {
			$this->finalDN = $this->rdn . '=' . ldap_escape($attributes[$this->dn_orig]['notchanged'][$this->rdn][0], '', LDAP_ESCAPE_DN) . ',' . $this->dnSuffix;
			if ($this->dn_orig != $this->finalDN) {
				$attributes[$this->finalDN] = $attributes[$this->dn_orig];
				unset($attributes[$this->dn_orig]);
			}
		}
		elseif (!$this->isNewAccount && (($this->dnSuffix != extractDNSuffix($this->dn_orig)) || ($this->rdn != extractRDNAttribute($this->dn_orig)))) {
			$this->finalDN = $this->rdn . '=' . ldap_escape($attributes[$this->dn_orig]['notchanged'][$this->rdn][0], '', LDAP_ESCAPE_DN) . ',' . $this->dnSuffix;
			$attributes[$this->finalDN] = $attributes[$this->dn_orig];
			unset($attributes[$this->dn_orig]);
		}
		// remove pwdAccountLockedTime attribute change if also userPassword is changed (PPolicy will remove this attribute itself)
		if (isset($attributes[$this->finalDN]['modify']['userPassword']) || isset($attributes[$this->finalDN]['remove']['userPassword'])) {
			if (isset($attributes[$this->finalDN]['modify']['pwdAccountLockedTime'])) {
				unset($attributes[$this->finalDN]['modify']['pwdAccountLockedTime']);
			}
			if (isset($attributes[$this->finalDN]['remove']['pwdAccountLockedTime'])) {
				unset($attributes[$this->finalDN]['remove']['pwdAccountLockedTime']);
			}
		}
		// pre modify actions
		$prePostModifyAttributes = array();
		if (isset($attributes[$this->finalDN]) && is_array($attributes[$this->finalDN])) {
			if (isset($attributes[$this->finalDN]['notchanged'])) {
				$prePostModifyAttributes = array_merge($prePostModifyAttributes, $attributes[$this->finalDN]['notchanged']);
			}
			if (isset($attributes[$this->finalDN]['modify'])) {
				foreach ($attributes[$this->finalDN]['modify'] as $key => $value) {
					$prePostModifyAttributes[$key] = &$attributes[$this->finalDN]['modify'][$key];
				}
				foreach ($attributes[$this->finalDN]['modify'] as $key => $value) {
					$prePostModifyAttributes['MOD.' . $key] = $value;
				}
			}
			if (isset($attributes[$this->finalDN]['add'])) {
				foreach ($attributes[$this->finalDN]['add'] as $key => $value) {
					$prePostModifyAttributes[$key] = &$attributes[$this->finalDN]['add'][$key];
				}
				foreach ($attributes[$this->finalDN]['add'] as $key => $value) {
					$prePostModifyAttributes['NEW.' . $key] = $value;
				}
			}
			if (isset($attributes[$this->finalDN]['remove'])) {
				foreach ($attributes[$this->finalDN]['remove'] as $key => $value) {
					$prePostModifyAttributes['DEL.' . $key] = $value;
				}
			}
			if (isset($attributes[$this->finalDN]['info'])) {
				foreach ($attributes[$this->finalDN]['info'] as $key => $value) {
					$prePostModifyAttributes['INFO.' . $key] = $value;
				}
			}
		}
		if (!$this->isNewAccount) {
			foreach ($this->attributes_orig as $key => $value) {
				$prePostModifyAttributes['ORIG.' . $key] = $value;
			}
			$prePostModifyAttributes['ORIG.dn'][0] = $this->dn_orig;
		}
		$prePostModifyAttributes['dn'][0] = $this->finalDN;
		if (!$this->isNewAccount && ($this->finalDN != $this->dn_orig)) {
			$prePostModifyAttributes['MOD.dn'][0] = $this->finalDN;
		}
		logNewMessage(LOG_DEBUG, 'Edit page pre/postModify attributes: ' . print_r($prePostModifyAttributes, true));
		$preModifyOk = true;
		foreach ($module as $singlemodule) {
			$preModifyMessages = $this->module[$singlemodule]->preModifyActions($this->isNewAccount, $prePostModifyAttributes);
			$errors = array_merge($errors, $preModifyMessages);
			for ($i = 0; $i < sizeof($preModifyMessages); $i++) {
				if ($preModifyMessages[$i][0] == 'ERROR') {
					$preModifyOk = false;
					break;
				}
			}
		}
		if (!$preModifyOk) {
			$errors[] = array('ERROR', _('The operation was stopped because of the above errors.'));
			return $errors;
		}
		// Set to true if an real error has happened
		$stopprocessing = false;
		$finalDnLower = strtolower($this->finalDN);
		$dnOrigLower = strtolower($this->dn_orig);
		if (($finalDnLower !== $dnOrigLower) && (unescapeLdapSpecialCharacters($finalDnLower) !== unescapeLdapSpecialCharacters($dnOrigLower))) {
			// move existing DN
			if ($this->dn_orig != '') {
				$removeOldRDN = false;
				if (isset($attributes[$this->finalDN]['modify'])) {
					$attributes[$this->finalDN]['modify'] = array_change_key_case($attributes[$this->finalDN]['modify'], CASE_LOWER);
				}
				$rdnAttr = strtolower(extractRDNAttribute($this->finalDN));
				if (isset($attributes[$this->finalDN]['modify'][$rdnAttr])
						&& (sizeof($attributes[$this->finalDN]['modify'][$rdnAttr]) == 1)
						&& ($attributes[$this->finalDN]['modify'][$rdnAttr][0] == extractRDNValue($this->finalDN))) {
					// remove old RDN if attribute is single valued
					$removeOldRDN = true;
					unset($attributes[$this->finalDN]['modify'][extractRDNAttribute($this->finalDN)]);
				}
				if (isset($attributes[$this->finalDN]['notchanged'][$rdnAttr])
					&& !(isset($attributes[$this->finalDN]['add'][$rdnAttr]) || isset($attributes[$this->finalDN]['modify'][$rdnAttr]) || isset($attributes[$this->finalDN]['remove'][$rdnAttr]))) {
					// fix for AD which requires to remove RDN even if not changed
					$removeOldRDN = true;
				}
				logNewMessage(LOG_DEBUG, 'Rename ' . $this->dn_orig . ' to ' . $this->finalDN);
				$success = ldap_rename($_SESSION['ldap']->server(), $this->dn_orig, $this->getRDN($this->finalDN), $this->getParentDN($this->finalDN), $removeOldRDN);
				if ($success) {
					logNewMessage(LOG_NOTICE, 'Renamed DN ' . $this->dn_orig . " to " . $this->finalDN);
					// do not add attribute value as new one if added via rename operation
					if (!empty($attributes[$this->finalDN]['add'][$rdnAttr]) && in_array(extractRDNValue($this->finalDN), $attributes[$this->finalDN]['add'][$rdnAttr])) {
						$attributes[$this->finalDN]['add'][$rdnAttr] = array_delete(array(extractRDNValue($this->finalDN)), $attributes[$this->finalDN]['add'][$rdnAttr]);
						if (empty($attributes[$this->finalDN]['add'][$rdnAttr])) {
							unset($attributes[$this->finalDN]['add'][$rdnAttr]);
						}
					}
				}
				else {
					logNewMessage(LOG_ERR, 'Unable to rename DN: ' . $this->dn_orig . ' (' . ldap_error($_SESSION['ldap']->server()) . '). '
						. getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
					$errors[] = array('ERROR', sprintf(_('Was unable to rename DN: %s.'), $this->dn_orig), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
					$stopprocessing = true;
				}
			}
			// create complete new dn
			else {
				$attr = array();
				if (isset($attributes[$this->finalDN]['add']) && is_array($attributes[$this->finalDN]['add'])) {
					$attr = array_merge_recursive($attr, $attributes[$this->finalDN]['add']);
				}
				if (isset($attributes[$this->finalDN]['notchanged']) && is_array($attributes[$this->finalDN]['notchanged'])) {
					$attr = array_merge_recursive($attr, $attributes[$this->finalDN]['notchanged']);
				}
				if (isset($attributes[$this->finalDN]['modify']) && is_array($attributes[$this->finalDN]['modify'])) {
					$attr = array_merge_recursive($attr, $attributes[$this->finalDN]['modify']);
				}
				$success = @ldap_add($_SESSION['ldap']->server(), $this->finalDN, $attr);
				if (!$success) {
					logNewMessage(LOG_ERR, 'Unable to create DN: ' . $this->finalDN . ' (' . ldap_error($_SESSION['ldap']->server()) . '). '
						. getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
					$errors[] = array('ERROR', sprintf(_('Was unable to create DN: %s.'), $this->finalDN), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
					$stopprocessing = true;
				}
				else {
					logNewMessage(LOG_NOTICE, 'Created DN: ' . $this->finalDN);
				}
				unset($attributes[$this->finalDN]);
			}
		}
		$DNs = array_keys($attributes);
		for ($i=0; $i<count($DNs); $i++) {
			if (!$stopprocessing) {
				logNewMessage(LOG_DEBUG, 'Attribute changes for ' . $DNs[$i] . ":\n" . print_r($attributes[$DNs[$i]], true));
				// modify attributes
				if (!empty($attributes[$DNs[$i]]['modify']) && !$stopprocessing) {
					$success = @ldap_mod_replace($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['modify']);
					if (!$success) {
						logNewMessage(LOG_ERR, 'Unable to modify attributes of DN: ' . $DNs[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . '). '
							. getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
						logNewMessage(LOG_DEBUG, print_r($attributes[$DNs[$i]]['modify'], true));
						$errors[] = array('ERROR', sprintf(_('Was unable to modify attributes of DN: %s.'), $DNs[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
						$stopprocessing = true;
					}
					else {
						logNewMessage(LOG_NOTICE, 'Modified DN: ' . $DNs[$i]);
						// check if the password of the currently logged in user was changed
						$lamAdmin = $_SESSION['ldap']->getUserName();
						if ((strtolower($DNs[$i]) == strtolower($lamAdmin)) && isset($attributes[$DNs[$i]]['info']['userPasswordClearText'][0])) {
							$_SESSION['ldap']->tryAndApplyNewPassword($attributes[$DNs[$i]]['info']['userPasswordClearText'][0]);
						}
					}
				}
				// add attributes
				if (!empty($attributes[$DNs[$i]]['add']) && !$stopprocessing) {
					$success = @ldap_mod_add($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['add']);
					if (!$success) {
						logNewMessage(LOG_ERR, 'Unable to add attributes to DN: ' . $DNs[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . '). '
							. getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
						$errors[] = array('ERROR', sprintf(_('Was unable to add attributes to DN: %s.'), $DNs[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
						$stopprocessing = true;
					}
					else {
						logNewMessage(LOG_NOTICE, 'Modified DN: ' . $DNs[$i]);
					}
				}
				// remove attributes
				if (!empty($attributes[$DNs[$i]]['remove']) && !$stopprocessing) {
					$success = @ldap_mod_del($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['remove']);
					if (!$success) {
						logNewMessage(LOG_ERR, 'Unable to delete attributes from DN: ' . $DNs[$i] . ' (' . ldap_error($_SESSION['ldap']->server()) . '). '
							. getExtendedLDAPErrorMessage($_SESSION['ldap']->server()));
						$errors[] = array('ERROR', sprintf(_('Was unable to remove attributes from DN: %s.'), $DNs[$i]), getDefaultLDAPErrorString($_SESSION['ldap']->server()));
						$stopprocessing = true;
					}
					else {
						logNewMessage(LOG_NOTICE, 'Modified DN: ' . $DNs[$i]);
					}
				}
			}
		}
		// send password mail
		if (!$stopprocessing && isLAMProVersion() && ($this->sendPasswordViaMail != null)) {
			$mailMessages = sendPasswordMail($this->sendPasswordViaMail, $prePostModifyAttributes, $this->sendPasswordViaMailAlternateAddress);
			if (sizeof($mailMessages) > 0) {
				$errors = array_merge($errors, $mailMessages);
			}
			$this->sendPasswordViaMail = null;
			$this->sendPasswordViaMailAlternateAddress = null;
		}
		if (!$stopprocessing) {
			// post modify actions
			foreach ($module as $singlemodule) {
				$postMessages = $this->module[$singlemodule]->postModifyActions($this->isNewAccount, $prePostModifyAttributes);
				$errors = array_merge($errors, $postMessages);
			}
		}
		return $errors;
	}

	/**
	 * Defines if the LDAP entry has only virtual child entries. This is the case for e.g. LDAP views.
	 *
	 * @return boolean has only virtual children
	 */
	public function hasOnlyVirtualChildren() {
		foreach ($this->module as $module) {
			if ($module->hasOnlyVirtualChildren()) {
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns a list of possible PDF entries for this account.
	 *
	 * @param array $pdfKeys list of PDF keys that are included in document
	 * @param string $typeId type id (user, group, host)
	 * @return PDFEntry[] list of key => PDFEntry
	 */
	function get_pdfEntries($pdfKeys, $typeId) {
		$return = array();
		while(($current = current($this->module)) != null) {
			$return = array_merge($return,$current->get_pdfEntries($pdfKeys, $typeId));
			next($this->module);
		}
		$dn = $this->dn_orig;
		if (isset($this->finalDN)) {
			$dn = $this->finalDN;
		}
		return array_merge($return,array('main_dn' => array(new \LAM\PDF\PDFLabelValue(_('DN'), $dn))));
	}

	/**
	* Sorts the module buttons for the account page.
	*/
	function sortModules() {
		$order = array();
		$modules = array_keys($this->module);
		$depModules = array();
		if (isset($this->order)) {
			$currentPage = $this->order[$this->current_page];
		}
		for ($i = 0; $i < sizeof($modules); $i++) {
			// insert waiting modules
			for ($w = 0; $w < sizeof($depModules); $w++) {
				$dependencies = $this->module[$depModules[$w]]->get_dependencies($this->type->getScope());
				$dependencies = $dependencies['depends'];
				$everything_found = true;
				for ($d = 0; $d < sizeof($dependencies); $d++) {
					if (!in_array($dependencies[$d], $order)) {
						$everything_found = false;
						break;
					}
				}
				// inser after depending module
				if ($everything_found) {
					$order[] = $depModules[$w];
					unset($depModules[$w]);
					$depModules = array_values($depModules);
					$w--;
				}
			}
			// check next module
			$dependencies = $this->module[$modules[$i]]->get_dependencies($this->type->getScope());
			if (is_array($dependencies['depends'])) {
				$everything_found = true;
				$dependencies = $dependencies['depends'];
				for ($d = 0; $d < sizeof($dependencies); $d++) {
					if (is_array($dependencies[$d])) { // or-combined dependencies
						$noneFound = true;
						foreach ($dependencies[$d] as $or) {
							if (in_array($or, $order)) {
								$noneFound = false;
								break;
							}
						}
						if ($noneFound) {
							$everything_found = false;
							break;
						}
					}
					elseif (!in_array($dependencies[$d], $order)) { // single dependency
						$everything_found = false;
						break;
					}
				}
				// remove module if dependencies are not satisfied
				if (!$everything_found) {
					$depModules[] = $modules[$i];
					unset($modules[$i]);
					$modules = array_values($modules);
					$i--;
				}
				else {
					$order[] = $modules[$i];
				}
			}
			else {
				$order[] = $modules[$i];
			}
		}
		// add modules which could not be sorted (e.g. because of cyclic dependencies)
		if (sizeof($depModules) > 0) {
			for ($i = 0; $i < sizeof($depModules); $i++) {
				$order[] = $depModules[$i];
			}
		}
		// move disabled modules to end
		$activeModules = array();
		$passiveModules = array();
		for ($i = 0; $i < sizeof($order); $i++) {
			if ($this->module[$order[$i]]->getButtonStatus() == 'enabled') {
				$activeModules[] = $order[$i];
			}
			else {
				$passiveModules[] = $order[$i];
			}
		}
		$this->order = array_merge($activeModules, $passiveModules);
		// check if ordering changed and current page number must be updated
		if (isset($currentPage) && ($currentPage != $this->order[$this->current_page])) {
			$this->current_page = array_search($currentPage, $this->order);
		}
	}

	/**
	 * Returns the RDN part of a given DN.
	 *
	 * @param String $dn DN
	 * @return String RDN
	 */
	function getRDN($dn) {
		if (($dn == "") || ($dn == null)) {
			return "";
		}
		return substr($dn, 0, strpos($dn, ","));
	}

	/**
	 * Returns the parent DN of a given DN.
	 *
	 * @param String $dn DN
	 * @return String DN
	 */
	function getParentDN($dn) {
		if (($dn == "") || ($dn == null)) {
			return "";
		}
		return substr($dn, strpos($dn, ",") + 1);
	}

	/**
	 * Returns a list of OUs that exist for this account type.
	 *
	 * @return array OU list
	 */
	public function getOUs() {
		if ($this->cachedOUs != null) {
			return $this->cachedOUs;
		}
		$this->cachedOUs = $this->type->getSuffixList();
		return $this->cachedOUs;
	}

	/**
	 * Replaces POST data with wildcard values from modules.
	 *
	 * @param array $keyPrefixes POST keys as full name or prefix (e.g. "key" matches "key1")
	 */
	public function replaceWildcardsInPOST($keyPrefixes) {
		$replacements = array();
		foreach ($this->module as $module) {
			$replacements = array_merge($replacements, $module->getWildCardReplacements());
		}
		while (true) {
			if (!$this->replaceWildcards($replacements, $keyPrefixes, $_POST)) {
				break;
			}
		}
	}

	/**
	 * Replaces wildcards in an array.
	 *
	 * @param array $replacements replacements (key => value)
	 * @param array $keyPrefixes prefixes of $data array keys that should be replaced
	 * @param array $data data array
	 * @return boolean replacement done
	 */
	private function replaceWildcards($replacements, $keyPrefixes, &$data) {
		$found = false;
		foreach ($data as $key => $value) {
			foreach ($keyPrefixes as $keyPrefix) {
				if (strpos($key, $keyPrefix) === 0) {
					$found = $this->doReplace($replacements, $data[$key]) || $found;
				}
			}
		}
		return $found;
	}

	/**
	 * Replaces wildcards in a value.
	 *
	 * @param array $replacements replacements (key => value)
	 * @param String $value value to perform replacements
	 * @return boolean replacement done
	 */
	private function doReplace($replacements, &$value) {
		$found = false;
		foreach ($replacements as $replKey => $replValue) {
			$searchString = '$' . $replKey;
			if (strpos($value, $searchString) !== false) {
				$found = true;
				$value = str_replace($searchString, $replValue, $value);
			}
			$searchString = '$_' . $replKey;
			if (strpos($value, $searchString) !== false) {
				$found = true;
				$value = str_replace($searchString, strtolower($replValue), $value);
			}
		}
		return $found;
	}

	/**
	* Encrypts sensitive data before storing in session.
	*
	* @return array list of attributes which are serialized
	*/
	function __sleep() {
		// encrypt data
		$this->attributes = lamEncrypt(serialize($this->attributes));
		$this->attributes_orig = lamEncrypt(serialize($this->attributes_orig));
		$this->module = lamEncrypt(serialize($this->module));
		// save all attributes
		return array_keys(get_object_vars($this));
	}

	/**
	* Decrypts sensitive data after accountContainer was loaded from session.
	*/
	function __wakeup() {
		// decrypt data
		$this->attributes = unserialize(lamDecrypt($this->attributes));
		$this->attributes_orig = unserialize(lamDecrypt($this->attributes_orig));
		$this->module = unserialize(lamDecrypt($this->module));
	}

}

/**
 * This interface needs to be implemented by all account modules which manage passwords.
 * It allows LAM to provide central password changes.
 *
 * @package modules
 */
interface passwordService {

	/**
	 * This method specifies if a module manages password attributes. The module alias will
	 * then appear as option in the GUI.
	 * <br>If the module only wants to get notified about password changes then return false.
	 *
	 * @return boolean true if this module manages password attributes
	 */
	public function managesPasswordAttributes();

	/**
	 * Specifies if this module supports to force that a user must change his password on next login.
	 *
	 * @return boolean force password change supported
	 */
	public function supportsForcePasswordChange();

	/**
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param array $modules list of modules for which the password should be changed
	 * @param boolean $forcePasswordChange force the user to change his password at next login
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return array(array('ERROR', 'Password change failed.'))
	 */
	public function passwordChangeRequested($password, $modules, $forcePasswordChange);

	/**
	 * Specifies if the module support password quick change for the current account.
	 *
	 * @return bool password quick change page supported
	 */
	public function supportsPasswordQuickChangePage(): bool;

	/**
	 * Adds account details such as first/last name for the current account.
	 *
	 * @param htmlResponsiveRow $row row where to add content
	 */
	public function addPasswordQuickChangeAccountDetails(htmlResponsiveRow $row): void;

	/**
	 * Returns a list of password quick change options.
	 *
	 * @return PasswordQuickChangeOption[] options
	 */
	public function getPasswordQuickChangeOptions(): array;

	/**
	 * Returns a list of LDAP attribute changes to perform.
	 *
	 * @param string $password new password
	 * @return array LDAP attribute values (attr_name => array(attr_value))
	 * @throws LAMException error getting changes
	 */
	public function getPasswordQuickChangeChanges(string $password): array;

	/**
	 * Returns the user name if known to be validated for password strength.
	 *
	 * @return string|null user name
	 */
	public function getPasswordQuickChangePasswordStrengthUserName(): ?string;

	/**
	 * Returns additional attribute values to check when password strength is validated.
	 *
	 * @return array attribute values
	 */
	public function getPasswordQuickChangePasswordStrengthAttributes(): array;

	/**
	 * Returns if the password is not same as an old password from history.
	 *
	 * @param string $password new password
	 * @return bool is in history
	 */
	public function getPasswordQuickChangeIsPasswordInHistory(string $password): bool;

}

/**
 * Option for the password quick change page.
 */
class PasswordQuickChangeOption {

	public $label;

	public $id;

	public $preSelected;

	/**
	 * PasswordQuickChangeOption constructor.
	 * @param string $id unique id for this option
	 * @param string $label descriptive label
	 * @param bool $preSelected is preselected
	 */
	public function __construct(string $id, string $label, bool $preSelected = true) {
		$this->label = $label;
		$this->id = $id;
		$this->preSelected = $preSelected;
	}

}
