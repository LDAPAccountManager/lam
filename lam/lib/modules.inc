<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2009  Roland Gruber

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

/** LDAP caches */
include_once("cache.inc");
/** some helper functions */
include_once("account.inc");
/** parent class of account modules */
include_once("baseModule.inc");
/** access to LDAP server */
include_once("ldap.inc");
/** lamdaemon functions */
include_once("lamdaemon.inc");
/** security functions */
include_once("security.inc");

/**
* This includes all module files.
*/
$modulesINC_dirname = substr(__FILE__, 0, strlen(__FILE__) - 12) . "/modules";
$modulesINC_dir = dir($modulesINC_dirname);
// get module names.
while ($entry = $modulesINC_dir->read())
if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($modulesINC_dirname . '/'.$entry)) {
	include_once($modulesINC_dirname . '/'.$entry);
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

/**
* Returns true if the module is a base module
*
* @param string $name the module name
* @param string $scope the account type ("user", "group", "host")
* @return boolean true if base module
*/
function is_base_module($name, $scope) {
	$module = new $name($scope);
	return $module->is_base_module();
}

/**
* Returns the LDAP filter used by the account lists
*
* @param string $scope the account type ("user", "group", "host")
* @return string LDAP filter
*/
function get_ldap_filter($scope) {
	$mods = $_SESSION['config']->get_AccountModules($scope);
	$filters = array();
	$orFilter = '';
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = new $mods[$i]($scope);
		$modinfo = $module->get_ldap_filter();
		if (isset($modinfo['or'])) $filters['or'][] = $modinfo['or'];
		if (isset($modinfo['and'])) $filters['and'][] = $modinfo['and'];
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

/**
* Returns a list of LDAP attributes which can be used to form the RDN.
*
* The list is already sorted by the priority given by the nodules.
*
* @param string $scope account type (user, group, host)
* @return array list of LDAP attributes
*/
function getRDNAttributes($scope) {
	$mods = $_SESSION['config']->get_AccountModules($scope);
	$return = array();
	$attrs_low = array();
	$attrs_normal = array();
	$attrs_high = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		// get list of attributes
		$module = new $mods[$i]($scope);
		$attrs = $module->get_RDNAttributes();
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
		if (!in_array($attrs_normal[$i], $return)) $return[] = $attrs_normal[$i];
	}
	for ($i = 0; $i < sizeof($attrs_low); $i++) {
		if (!in_array($attrs_low[$i], $return)) $return[] = $attrs_low[$i];
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
		$module = new $mods[$i]($scope);
		$return[$mods[$i]] = $module->get_dependencies();
	}
	return $return;
}


/**
* Checks if there are missing dependencies between modules.
*
* @param array $selected selected module names
* @param array $deps module dependencies
* @return mixed false if no misssing dependency was found,
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
	if (sizeof($ret) > 0) return $ret;
	else return false;
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
	$dirname = substr(__FILE__, 0, strlen(__FILE__) - 12) . "/modules";
	$dir = dir($dirname);
	$return = array();
	// get module names.
	while ($entry = $dir->read())
		if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($dirname . '/'.$entry)) {
			$entry = substr($entry, 0, strpos($entry, '.'));
			$temp = new $entry($scope);
			if ($temp->can_manage()) $return[] = $entry;
		}
	return $return;
}

/**
* Returns the elements for the profile page.
*
* @param string $scope account type (user, group, host)
* @return array profile elements
*/
function getProfileOptions($scope) {
	$mods = $_SESSION['config']->get_AccountModules($scope);
	$return = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = new $mods[$i]($scope);
		$return[$mods[$i]] = $module->get_profileOptions();
	}
	return $return;
}

/**
* Checks if the profile options are valid
*
* @param string $scope account type (user, group, host)
* @param array $options hash array containing all options (name => array(...))
* @return array list of error messages
*/
function checkProfileOptions($scope, $options) {
	$mods = $_SESSION['config']->get_AccountModules($scope);
	$return = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = new $mods[$i]($scope);
		$temp = $module->check_profileOptions($options);
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
		$m = new $modules[$i]('none');
		$return[$modules[$i]] = $m->get_configOptions($scopes[$modules[$i]], $scopes);
	}
	return $return;
}

/**
* Checks if the configuration options are valid
*
* @param array $scopes hash array (module name => array(account types))
* @param array $options hash array containing all options (name => array(...))
* @return array list of error messages
*/
function checkConfigOptions($scopes, $options) {
	$return = array();
	$modules = array_keys($scopes);
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = new $modules[$i]('none');
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
	if (!isset($module) || ($module == '') || ($module == 'main')) {
		$helpPath = "../help/help.inc";
		if (is_file("../../help/help.inc")) $helpPath = "../../help/help.inc";
		if (!isset($helpArray)) {
			include($helpPath);
		}
		return $helpArray[$helpID];
	}
	$moduleObject = new $module((($scope != '') ? $scope : 'none'));
	return $moduleObject->get_help($helpID);
}

/**
* Returns a list of available PDF entries.
*
* @param string $scope account type (user, group, host)
* @return array PDF entries
*/
function getAvailablePDFFields($scope) {
	$mods = $_SESSION['config']->get_AccountModules($scope);
	$return = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = new $mods[$i]($scope);
		$return[$mods[$i]] = $module->get_pdfFields();
	}
	$return['main'] = array('dn');
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
* @param string $scope account type
* @return array column list
*/
function getUploadColumns($scope) {
	$mods = $_SESSION['config']->get_AccountModules($scope);
	$return = array();
	for ($i = 0; $i < sizeof($mods); $i++) {
		$module = new $mods[$i]($scope);
		$return[$mods[$i]] = $module->get_uploadColumns();
	}
	return $return;
}

/**
* This function builds the LDAP accounts for the file upload.
*
* If there are problems status messages will be printed automatically.
*
* @param string $scope account type
* @param array $data array containing one account in each element
* @param array $ids array(<column_name> => <column number>)
* @return mixed array including accounts or false if there were errors
*/
function buildUploadAccounts($scope, $data, $ids) {
	// build module order
	$unOrdered = $_SESSION['config']->get_AccountModules($scope);
	$ordered = array();
	$predepends = array();
	// get dependencies
	for ($i = 0; $i < sizeof($unOrdered); $i++) {
		$mod = new $unOrdered[$i]($scope);
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
		if ($newRound) continue;
		// this point should never be reached, LAM was unable to find a correct module order
		StatusMessage("ERROR", "Internal Error: Unable to find correct module order.", "");
		return false;
	}
	// give raw data to modules
	$errors = array();
	$partialAccounts = array();
	for ($i = 0; $i < sizeof($data); $i++) $partialAccounts[$i]['objectClass'] = array();
	for ($i = 0; $i < sizeof($ordered); $i++) {
		$module = new $ordered[$i]($scope);
		$errors = $module->build_uploadAccounts($data, $ids, $partialAccounts);
		if (sizeof($errors) > 0) {
			array_unshift($errors, array("INFO", _("Displayed account numbers start at \"0\". Add 2 to get the row in your spreadsheet."), ""));
			$errors[] = array("ERROR", _("Upload was stopped after errors in %s module!"), "", array($module->get_alias()));
			break;
		}
	}
	if (sizeof($errors) > 0) {
		for ($i = 0; (($i < sizeof($errors)) || ($i > 49)); $i++) call_user_func_array("StatusMessage", $errors[$i]);
		return false;
	}
	else return $partialAccounts;
}

/**
* This function executes one post upload action.
*
* @param string $scope account type
* @param array $data array containing one account in each element
* @param array $ids array(<column_name> => <column number>)
* @param array $failed list of accounts which were not created successfully
* @return array current status
* <br> array (
* <br>  'status' => 'finished' | 'inProgress'
* <br>  'module' => <name of active module>
* <br>  'progress' => 0..100
* <br>  'errors' => array (<array of parameters for StatusMessage>)
* <br> )
*/
function doUploadPostActions($scope, $data, $ids, $failed) {
	// check if function is called the first time
	if (! isset($_SESSION['mass_postActions']['remainingModules'])) {
		// make list of remaining modules
		$moduleList = $_SESSION['config']->get_AccountModules($scope);
		$_SESSION['mass_postActions']['remainingModules'] = $moduleList;
	}
	$activeModule = $_SESSION['mass_postActions']['remainingModules'][0];
	// initialize temporary variable
	if (!isset($_SESSION['mass_postActions'][$activeModule])) {
		$_SESSION['mass_postActions'][$activeModule] = array();
	}
	// let first module do one post action
	$module = new $activeModule($scope);
	$return = $module->doUploadPostActions($data, $ids, $failed, $_SESSION['mass_postActions'][$activeModule]);
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
	$scopes = $_SESSION['config']->get_ActiveTypes();
	for ($i = 0; $i < sizeof($scopes); $i++) {
		$mods = $_SESSION['config']->get_AccountModules($scopes[$i]);
		for ($m = 0; $m < sizeof($mods); $m++) {
			$module = new $mods[$m]($scopes[$i]);
			$ext = $module->getRequiredExtensions();
			for ($e = 0; $e < sizeof($ext); $e++) {
				if (!in_array($ext[$e], $extList)) $extList[] = $ext[$e];
			}
		}
	}
	return $extList;
}

/**
* Takes a list of meta-HTML elements and prints the equivalent HTML output.
* 
* The modules are not allowed to display HTML code directly but return
* meta HTML code. This allows to have a common design for all module pages.<br>
* <br>
* Meta HTML code is always returned as a three dimensional <b>array[a][b][c]</b>
* where <b>a</b> is the row number, <b>b</b> is the column number and <b>c</b> is
* is a <i>data element</i>.<br>
* <br>
* <b>Format of data elements:</b><br>
* <br>
* A <i>data element</i> is an array which contains the data to display.<br>
* All <i>data elements</i> must contail a value <b>"kind"</b> which
* defines what kind of element should be displayed.<br>
* <br>
* These are the possibilies for <b>kind</b> and what other options have to be added to the array:<br>
* <br>
* <ul>
*   <li><b>fieldset:</b> Inserts a fieldset.
* <ul>
*   <li><b>legend:</b> The legend of the fieldset.</li>
*   <li><b>value:</b> A <i>data element</i>. Can be used recursively.</li>
* </ul>
* </li>
* <li><b>help:</b> Adds a help link.
* <ul>
*   <li><b>value:</b> The help number for the help entry.</li>
*   <li><b>scope:</b> The account type for the help entry.<br>
*   </li>
* </ul>
* </li>
* <li><b>input:</b> Adds a HTML input element.
* <ul>
*   <li><b>name:</b> The name of the element, will be used later as variable name
*                    when user input is returned.</li>
*   <li><b>type:</b> allowed values: submit, reset, checkbox, text, password, file, hidden</li>
*   <li><b>checked:</b> Boolean value, if true a checkbox will be checked. This
*                       value is only needed or checkboxes.</li>
*   <li><b>disabled:</b> Boolean value, if true the element will be disabled.</li>
*    <li><b>size:</b> The length of the input element, only used for text, password and file.</li>
*    <li><b>maxlength:</b> The maximum size of the input element, only used for
*                          text, password and file.</li>
*   <li><b>value:</b> The element will have this value as default. Button elements will have
*                     this as caption.</li>
*   <li><b>title:</b> Title value for input element (optional).</li>
*   <li><b>image:</b> This is used to display image buttons. File name of an 16x16px image in the graphics folder
*                     (e.g. add.png). You may only use this for submit buttons.</li>
* </ul>
* </li>
* <li><b>select:</b> This will add a select field.
* <ul>
*   <li><b>name:</b> The name of the element, will be used later as variable name when user input is
*                    returned.</li>
*   <li><b>multiple:</b> Boolean value, if set to true the user can select more than one entry.</li>
*   <li><b>options:</b> Array of string. This is the list of option values the user can select.</li>
*   <li><b>options_selected:</b> Array of string. This is the list of pre selected elements, must contain
*                                values that are also in <i>options</i>.</li>
*   <li><b>descriptiveOptions:</b>
* Boolean value, if set to true then all elements in options must be arrays themselves (array(<i>value</i>,
*<i>description</i>)) (default: false)<br>
*   </li>
*   <li><b>size:</b> The size of the select field, if set to 1 a dropdown box will be displayed.</li>
*   <li><b>noSorting:</b> If set to true then the entries will not be sorted. Default is false.</li>
*    <li><b>onchange:</b> onchange event<br>
*    </li>
* </ul>
* </li>
* <li><b>table:</b> Adds a table. Can be used recursively.
*  <ul>
*   <li><b>value:</b> A <i>data element</i>. Can be used recursively.</li>
*  </ul>
* </li>
*  <li><b>text:</b> Inserts a text element.
*  <ul>
*   <li><b>text:</b> The text to display.</li>
*  </ul>
* </li>
*  <li><b>textarea:</b> Adds a multiline text field.
*  <ul>
*   <li><b>name:</b> The name of the element, will be used later as variable name when user
*                    input is returned.</li>
*   <li><b>rows:</b> Number of rows (required)<br>
*   </li>
*   <li><b>cols:</b> Number of characters for each line (required)<br>
*   </li>
*   <li><b>readonly:</b> Boolean value, if true the text field will be read only.<br>
*   </li>
* </ul>
* </li>
* <li><b>image:</b> Displays an image.
* <ul>
*   <li><b>path:</b> Path to the image</li>
*   <li><b>width:</b> Width of the image</li>
*   <li><b>height:</b> Height of the image</li>
*   <li><b>alt:</b> Alt text of the image<br>
*    </li>
* </ul>
* </li>
* </ul>
* <br>
* Beneath those values a <b>"td"</b> value may be added. This has to be an array with one or more
* of these options:<br>
* <br>
* <ul>
*  <li><b>colspan:</b> Like the HTML colspan attribute for td elements</li>
*  <li><b>rowspan:</b> Like the HTML rowspan attribute for td elements</li>
*  <li><b>align:</b> left/center/right/justify Like the HTML align attribute</li>
*  <li><b>valign:</b> top/middle/bottom Like the HTML valign attribute</li>
*  <li><b>width:</b> Like the HTML height attribute for td elements</li>
* </ul>
* <br>
* Input buttons which should load a different subpage of a module must have a special <i>name</i> attribute:<br>
* <br>
* <b>name</b> => 'form_subpage_' . <i><module name></i> . '_' . <i><subpage name></i> . '_' . <i><button name></i><br>
* <ul>
* <li><b><module name>:</b> name of this account module (e.g. 'posixAccount')</li>
* <li><b><subpage name>:</b> name of next subpage (e.g. 'attributes')</li>
* <li><b><button name>:</b> a name to distinguish buttons (e.g. 'ok'/'cancel'/'back')</li>
* </ul>
* <br>
* <br>
* <b>Example:</b>
* <code>
* array(
*  array(
*    array("kind" => "text", "text" => "This is an example", "td" => array("colspan" => 3))
*  ),
*  array(
*    array("kind" => "text", "text" => "Input:"),
*    array("kind" => "input", "name" => "myinput", "type" => "text"),
*    array("kind" => "help", "value" => "42")
*  ),
*  array(
*    array("kind" => "input", "name" => 'form_subpage_myModule_attributes_back', "value" => _("Back"))
*  )
*)
* </code>
* 
*
* @param string $module Name of account module
* @param array $input List of meta-HTML elements
* @param array $values List of values which override the defaults in $input (name => value)
* @param boolean $restricted If true then no buttons will be displayed
* @param integer $tabindex Start value of tabulator index for input fields
* @param string $scope Account type
* @return array List of input field names and their type (name => type)
*/
function parseHtml($module, $input, $values, $restricted, &$tabindex, $scope) {
	$ret = array();
	if (is_array($input)) {
		echo "<table>\n";
		for ($i=0; $i<count($input); $i++) {	// $i = row number
			// Draw column
			echo "<tr>\n";
			for ($j=0; $j<count($input[$i]); $j++ ) {	// $j = column number
				// Draw cell
				echo "<td";
				if (isset($input[$i][$j]['td']['align'])) echo " align=\"" . $input[$i][$j]['td']['align'] . "\"";
				if (isset($input[$i][$j]['td']['valign'])) echo " valign=\"" . $input[$i][$j]['td']['valign'] . "\"";
				if (isset($input[$i][$j]['td']['colspan'])) echo " colspan=\"" . $input[$i][$j]['td']['colspan'] . "\"";
				if (isset($input[$i][$j]['td']['rowspan'])) echo " rowspan=\"" . $input[$i][$j]['td']['rowspan'] . "\"";
				if (isset($input[$i][$j]['td']['width'])) echo " width=\"" . $input[$i][$j]['td']['width'] . "\"";
				echo ">\n";
				switch ($input[$i][$j]['kind']) {
					// plain text
					case 'text':
						if (isset($input[$i][$j]['text'])) echo $input[$i][$j]['text'];
						break;
					// input fields
					case 'input':
						$type = $input[$i][$j]['type'];
						if ($restricted && (($type == "submit") || ($type == "reset"))) break;	// no buttons in restricted mode
						$output = "<input";
						if ($input[$i][$j]['name']!='') $output .= ' name="' . $input[$i][$j]['name'] . '"';
						if ($type != '') $output .= ' type="' . $type . '"';
						if (isset($input[$i][$j]['size']) && ($input[$i][$j]['size'] != '')) {
							$output .= ' size="' . $input[$i][$j]['size'] . '"';
						}
						if (isset($input[$i][$j]['maxlength']) && ($input[$i][$j]['maxlength'] != '')) {
							$output .= ' maxlength="' . $input[$i][$j]['maxlength'] . '"';
						}
						// checkbox
						if ($type == "checkbox") {
							if (isset($values[$input[$i][$j]['name']])) {
								if ($values[$input[$i][$j]['name']][0] == "true") $output .= ' checked';
							}
							elseif (isset($input[$i][$j]['checked']) && ($input[$i][$j]['checked'])) $output .= ' checked';
						}
						// other input element
						else {
							if (isset($values[$input[$i][$j]['name']])) {
								$output .= ' value="' . htmlspecialchars($values[$input[$i][$j]['name']][0], ENT_QUOTES, "UTF-8") . '"';
							}
							elseif (isset($input[$i][$j]['value']) && $input[$i][$j]['value']!='') {
								$output .= ' value="' . htmlspecialchars($input[$i][$j]['value'], ENT_QUOTES, "UTF-8") . '"';
							}
							if (($type == "submit") && isset($input[$i][$j]['image'])) {
								$output .= ' style="background-image: url(../../graphics/' . $input[$i][$j]['image'] . ');background-position: 2px center;background-repeat: no-repeat;width:24px;height:24px;background-color:transparent"';
							}
						}
						if (isset($input[$i][$j]['title'])) {
							$output .= ' title="' . $input[$i][$j]['title'] . '"';
						}
						if (isset($input[$i][$j]['disabled']) && ($input[$i][$j]['disabled'] == true)) $output .= ' disabled';
						// Show taborder
						else {
							$output .= " tabindex=$tabindex";
							$tabindex++;
						}
						$output .= ">";
						echo $output;
						$ret[$input[$i][$j]['name']] = $type;	// save type
						break;
					// text area
					case 'textarea':
						echo "<textarea";
						if (isset($input[$i][$j]['name'])) echo ' name="' . $input[$i][$j]['name'] . '"';
						if (isset($input[$i][$j]['readonly']) && (isset($input[$i][$j]['readonly']) === true)) echo ' readonly';
						echo ' cols="' . $input[$i][$j]['cols'] . '"';
						echo ' rows="' . $input[$i][$j]['rows'] . '"';
						echo ">";
						if (isset($values[$input[$i][$j]['name']])) {
							echo htmlspecialchars(implode("\r\n", $values[$input[$i][$j]['name']]), ENT_QUOTES, "UTF-8");
						}
						else {
							echo htmlspecialchars($input[$i][$j]['value'], ENT_QUOTES, "UTF-8");
						}
						echo "</textarea>";
						$ret[$input[$i][$j]['name']] = 'textarea';	// save type
						break;
					// inner fieldset
					case 'fieldset':
						$styleClass = '';
						if (isset($scope) && ($scope != '')) {
							$styleClass = ' class="' . $scope . 'edit"';
						}
						echo "<fieldset" . $styleClass . ">\n";
						if ($input[$i][$j]['legend']!='') echo "<legend>" . $input[$i][$j]['legend'] . "</legend>\n";
						$retTemp = parseHtml($module, $input[$i][$j]['value'], $values, $restricted, $tabindex, $scope);
						$ret = array_merge($ret, $retTemp);
						echo "</fieldset>\n";
						break;
					// selection
					case 'select':
						if (! isset($input[$i][$j]['size'])) $input[$i][$j]['size'] = 1;	// correct size if needed
						if (isset($input[$i][$j]['multiple'])) {
							echo "<select name=\"" . $input[$i][$j]['name'] . '[]"';
							echo ' multiple';
							$ret[$input[$i][$j]['name']] = 'multiselect';	// save type
						}
						else {
							echo "<select name=\"" . $input[$i][$j]['name'] . '"';
							$ret[$input[$i][$j]['name']] = 'select';	// save type
						}
						echo ' size="' . $input[$i][$j]['size'] . '"';
						if (!$restricted && isset($input[$i][$j]['onchange'])) {
							echo ' onchange="' . htmlspecialchars($input[$i][$j]['onchange']) . '"';
						}
						// Show taborder
						echo " tabindex=$tabindex";
						$tabindex++;
						echo ">\n";
						// init option fields
						if (!is_array($input[$i][$j]['options'])) $input[$i][$j]['options'] = array ( $input[$i][$j]['options'] );
						if (isset($input[$i][$j]['options_selected'])) {
							if (!is_array($input[$i][$j]['options_selected'])) {
								// one selected element
								$input[$i][$j]['options_selected'] = array ( $input[$i][$j]['options_selected'] );
							}
						}
						else {
							$input[$i][$j]['options_selected'] = array();
						}
						if (isset($values[$input[$i][$j]['name']])) $input[$i][$j]['options_selected'] = $values[$input[$i][$j]['name']];
						// merge both option arrays and sort them.
						$options = $input[$i][$j]['options'];
						if (!isset($input[$i][$j]['descriptiveOptions']) || ($input[$i][$j]['descriptiveOptions'] === false)) {
							// merge both option arrays and sort them.
							$options = array_merge($input[$i][$j]['options'], $input[$i][$j]['options_selected'] );
							$options = array_unique($options);
						}
						if (!isset($input[$i][$j]['noSorting']) || !$input[$i][$j]['noSorting']) {
							sort($options);
						}
						foreach ($options as $option) {
							if (isset($input[$i][$j]['descriptiveOptions']) && ($input[$i][$j]['descriptiveOptions'] === true)) {
								if (in_array($option[0], $input[$i][$j]['options_selected'])) {
									echo "<option value=\"" . htmlspecialchars($option[0], ENT_QUOTES, "UTF-8") . "\" selected>" . htmlspecialchars($option[1], ENT_QUOTES, "UTF-8") . "</option>\n";
								}
								else {
									echo "<option value=\"" . htmlspecialchars($option[0], ENT_QUOTES, "UTF-8") . "\">" . htmlspecialchars($option[1], ENT_QUOTES, "UTF-8") . "</option>\n";
								}
							}
							elseif ($option!='') {
								if (in_array($option, $input[$i][$j]['options_selected'])) {
									echo "<option selected>" . htmlspecialchars($option, ENT_QUOTES, "UTF-8") . "</option>\n";
								}
								else {
									echo "<option>" . htmlspecialchars($option, ENT_QUOTES, "UTF-8") . "</option>\n";
								}
							}
						}
						echo "</select>\n";
						break;
					// sub table
					case 'table':
						$retTemp = parseHtml($module, $input[$i][$j]['value'], $values, $restricted, $tabindex, $scope);
						$ret = array_merge($ret, $retTemp);
						break;
					// help link
					case 'help':
						$helpEntry = getHelp($module, $input[$i][$j]['value'], $scope);
						printHelpLink($helpEntry, $input[$i][$j]['value'], $module, $scope);
						break;
					// status message
					case 'message':
						StatusMessage($input[$i][$j]['type'], $input[$i][$j]['headline'], $input[$i][$j]['text']);
						break;
					// image
					case 'image':
						echo "<img ";
						if (isset($input[$i][$j]['path'])) echo 'src="' . $input[$i][$j]['path'] . '" ';
						if (isset($input[$i][$j]['width'])) echo 'width="' . $input[$i][$j]['width'] . '" ';
						if (isset($input[$i][$j]['height'])) echo 'height="' . $input[$i][$j]['height'] . '" ';
						if (isset($input[$i][$j]['alt'])) echo 'alt="' . $input[$i][$j]['alt'] . '" ';
						echo ">\n";
						break;
					// error, unknown type
					default:
						echo "Unrecognized type: " . $input[$i][$j]['kind'] . "\n";
						break;
				}
				echo "</td>\n";
			}
			echo "</tr>\n";
		}
	}
	echo "</table>\n";
	return $ret;
}

/**
 * Prints a LAM help link.
 *
 * @param array $entry help entry
 * @param String $number help number
 * @param String $module module name
 * @param String $scope account scope
 */
function printHelpLink($entry, $number, $module='', $scope='') {
	$helpPath = "../";
	if (is_file("./help.php")) $helpPath = "";
	$title = $entry['Headline'];
	$message = $entry['Text'];
	// replace special characters
	$message = str_replace(array("'", '"', "\n"), array("\\'", '&quot;', ''), $message);
	echo "<a href=\"" . $helpPath . "help.php?module=$module&amp;HelpNumber=". $number . "&amp;scope=" . $scope . "\" ";
		echo "target=\"help\" ";
		echo "onmouseover=\"Tip('" . $message . "', TITLE, '" . $title . "')\" onmouseout=\"UnTip()\">";
		echo "<img src=\"../$helpPath/graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
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
	* @param string $type account type
	* @param string $base key in $_SESSION where this object is saved
	*/
	function __construct($type, $base) {
		/* Set the type of account. Valid
		* types are: user, group, host
		*/
		// Check input variable

		if (!is_string($type)) trigger_error('Argument of accountContainer must be string.', E_USER_ERROR);
		if (!is_string($base)) trigger_error('Argument of accountContainer must be string.', E_USER_ERROR);
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

	/** This is an array with all module objects */
	private $module;

	/** DN suffix of the account */
	public $dn;

	/** DN suffix of account when it was loaded */
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

	/** This variable stores the name of the currently displayed page */
	private $current_page;

	/** This variable is set to the pagename of a subpage if it should be displayed */
	private $subpage;

	/** True if this is a newly created account */
	public $isNewAccount;
	
	private $lastLoadedProfile = '';

	/**
	 * Returns the account module with the given class name
	 *
	 * @param string $name class name (e.g. posixAccount)
	 * @return baseModule account module
	 */
	function getAccountModule($name) {
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
	 * @return array modules
	 */
	function getAccountModules() {
		return $this->module;
	}
	
	/**
	* Returns the accout type of this object (e.g. user, group, host).
	*
	* @return string account type
	*/
	function get_type() {
		return $this->type;
		}

	/**
	* This function is called when the user clicks on any button on the account pages.
	* It prints the HTML code of each account page.
	*/
	function continue_main() {
		$result = array();
		$stopProcessing = false; // when set to true, no module options are displayed
		$errorsOccured = false;
		$profileLoaded = $this->loadProfileIfRequested();
		if ($this->subpage=='') $this->subpage='attributes';
		if (isset($_POST['accountContainerReset'])) {
			$result = $this->load_account($this->dn_orig);
		}
		elseif (isset($_POST['setNewPasswordCancel'])) {
			// ignore
		}
		elseif (isset($_POST['setNewPasswordOk']) || isset($_POST['setNewPasswordRandom'])) {
			$pwdMessages = $this->setNewPassword();
			$pwdErrors = 0;
			for ($i = 0; $i < sizeof($pwdMessages); $i++) {
				if ($pwdMessages[$i][0] == 'ERROR') {
					$pwdErrors++;
				}
			}
			if ($pwdErrors == 0) {
				$result[] = array('INFO', _('New password set successfully.'), _('The new password will be stored in the directory after you save this account.'));
				$result = array_merge($result, $pwdMessages);
			}
			else {
				$this->printPasswordPromt($pwdMessages);
				return;
			}
		}
		elseif (!$profileLoaded) {
			// change dn suffix
			if (isset($_REQUEST['suffix']) && ($_REQUEST['suffix'] != '')) {
				$this->dn = $_REQUEST['suffix'];
			}
			if (isset($_REQUEST['accountContainerSuffix']) && ($_REQUEST['accountContainerSuffix'] != '')) {
				$this->dn = $_REQUEST['accountContainerSuffix'];
			}
			// change RDN
			if (isset($_POST['accountContainerRDN'])) {
				$this->rdn = $_POST['accountContainerRDN'];
			}
			// create another account
			if (isset($_POST['accountContainerCreateAgain'])) {
				// open fresh account page
				unset($_SESSION[$this->base]);
				metaRefresh("edit.php?type=" . $this->type . "&amp;suffix=" . $this->dn);
				exit();
			}
			// back to account list
			if (isset($_POST['accountContainerBackToList'])) {
				// Return to account list
				unset($_SESSION[$this->base]);
				metaRefresh("../lists/list.php?type=" . $this->type);
				exit;
			}
			// create PDF file
			if (isset($_POST['accountContainerCreatePDF'])) {
				// display PDf-file
				createModulePDF(array($_SESSION[$this->base]), $_POST['pdfStructure']);
				exit;
			}
			// module actions
			if ((sizeof($_POST) > 0) && checkIfWriteAccessIsAllowed()) {
				$result = call_user_func(array(&$this->module[$this->order[$this->current_page]], 'process_'.$this->subpage));
				if (is_array($result)) {  // messages were returned, check for errors
					for ($i = 0; $i < sizeof($result); $i++) {
						if ($result[$i][0] == 'ERROR') {
							$errorsOccured = true;
							break;
						}
					}
				}
			$this->sortModules();
			}
			// save account
			if (!$errorsOccured && isset($_POST['accountContainerSaveAccount'])) {
				// check if all modules are complete
				$modules = array_keys($this->module);
				$incompleteModules = array();
				foreach ($modules as $module) {
					if (!$this->module[$module]->module_complete()) {
						$incompleteModules[] = $this->module[$module]->get_alias();
					}
				}
				if (sizeof($incompleteModules) > 0) {
					StatusMessage('INFO', _('Some required information is missing'),
						sprintf(_('Please set up all required attributes on page: %s'), implode(", ", $incompleteModules)));
				}
				else {
					// save account
					$errors = $this->save_account();
					if (sizeof($errors) > 0) {
						$result = $errors;
						$stopProcessing = true;
					}
					else {
						$this->printSuccessPage();
						return;
					}
				}
			}
		}
		// change to next page
		if (is_array($result)) {  // messages were returned, check for errors
			for ($i = 0; $i < sizeof($result); $i++) {
				if ($result[$i][0] == 'ERROR') {
					$errorsOccured = true;
					break;
				}
			}
		}
		if (!$errorsOccured) {
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
		// check if password change was requested
		if (!$errorsOccured && isset($_POST['accountContainerSetPassword'])) {
			$this->printPasswordPromt(array());
			return;
		}
		// prints a module content page
		$this->printModuleContent($result, $stopProcessing);
	}
	
	/**
	 * Prints the content part provided by the current module.
	 * 
	 * @param array $result list of messages
	 * @param boolean $stopProcessing true if page should end after displaying the messages
	 */
	private function printModuleContent($result, $stopProcessing) {
		$this->printPageHeader();
		// Display error-messages
		if (is_array($result)) {
			for ($i=0; $i<sizeof($result); $i++) {
				call_user_func_array("StatusMessage", $result[$i]);
			}
			if ($stopProcessing) {
				echo "</body>\n";
				echo "</html>\n";
				return;
			}
		}
		if (checkIfWriteAccessIsAllowed()) {
			$this->printCommonControls();
		}
		echo "<br>\n";
		// create module menu
		echo "<table class=\"".$this->type."list\" border=0 width=\"100%\" style=\"border-collapse: collapse;\">\n";
		echo "<tr class=\"".$this->type."list\" valign=\"top\"><td style=\"border-bottom: 1px solid;padding:0px;\" colspan=2>";
		// tab menu
		$this->printModuleTabs();
		echo "</td></tr>\n";
		// content header
		echo "<tr class=\"" . $this->type . "list\">\n";
			echo "<td style=\"padding:10px;\">\n";
				$this->printContentHeader();
			echo "</td>\n";
		echo "</tr>\n";
		echo "<tr class=\"" . $this->type . "list\">\n";
		// content area
		echo "<td width=\"100%\" style=\"padding:10px;\">";
		// display html-code from modules
		$return = call_user_func(array($this->module[$this->order[$this->current_page]], 'display_html_'.$this->subpage));
		$y = 5000;
		parseHtml($this->order[$this->current_page], $return, array(), false, $y, $this->type);
		// Display rest of html-page
		echo "<br>*" . _('required');
		echo "</td></tr></table>\n";
		$this->printPageFooter();
	}

	/**
	 * Prints the input fields of the central password service.
	 * 
	 * @param $errors list of error messages
	 */
	private function printPasswordPromt($errors) {
		$this->printPageHeader();
		// print error messages
		for ($i = 0; $i < sizeof($errors); $i++) {
			call_user_func_array('StatusMessage', $errors[$i]);
		}
		// create module menu
		echo "<br><table class=\"".$this->type."list\" border=0 width=\"100%\" style=\"border-collapse: collapse;\">\n";
		echo "<tr class=\"" . $this->type . "list\">";
		echo "<td nowrap>&nbsp;<img alt=\"key.png\" src=\"../../graphics/key.png\">&nbsp;</td> \n";
		echo "<td nowrap>" . _("Set password") . "&nbsp;&nbsp;&nbsp;</td>";
		echo "<td>\n";
		printHelpLink(getHelp(null, 404), 404);
		echo "</td><td width=\"100%\">&nbsp;</td>\n";
		echo "<tr class=\"" . $this->type . "list\"><td colspan=4>\n";
		echo "<hr class=\"modulePage\" noshade>\n";
		echo "</td></tr>\n";
		echo "<tr class=\"" . $this->type . "list\"><td colspan=4>\n";
		echo "<table>\n";
		echo "<tr><td>\n";
			echo _('Password') . '</td><td><input type="password" name="newPassword1">';

		echo '</td>';
		echo '<td width=30 rowspan=2>&nbsp;</td>';
		echo '<td rowspan=3 valign="top">';
		echo '<table>';
		foreach ($this->module as $name => $module) {
			if (($module instanceof passwordService) && $module->managesPasswordAttributes()) {
				echo '<tr><td>';
				$buttonImage = $module->getIcon();
				if ($buttonImage != null) {
					$buttonImage = '<img src="../../graphics/' . $buttonImage . '" alt="' . getModuleAlias($name, $this->type) . '"></td><td>';
				}
				echo '<input type="checkbox" checked name="cb_' . $name . '"></td><td>' . $buttonImage . getModuleAlias($name, $this->type) . "<br>\n";
				echo '</td></tr>';
			}
		}
		echo '</table></td>';
		echo "</tr>";
		
		echo "<tr><td>";
			echo _('Repeat password') . '</td><td><input type="password" name="newPassword2">';
		echo "</td></tr>\n";
		echo "<tr><td colspan=3 valign=\"top\"><br><br>";
			echo '<input type="submit" name="setNewPasswordOk" value="'. _('Ok') . '">';
			echo '&nbsp;';
			echo '<input type="submit" name="setNewPasswordCancel" value="'. _('Cancel') . '">';
			echo '&nbsp;&nbsp;&nbsp;';
			echo '<input type="submit" name="setNewPasswordRandom" value="'. _('Set random password') . '">';
			echo "</td></tr>\n";
		echo "</table>\n";
		echo "</td></tr>\n";
		echo "</table>\n";
		$this->printPageFooter();
	}
	
	/**
	 * Sets the new password in all selected account modules.
	 * 
	 * @return array list of messages
	 */
	private function setNewPassword() {
		$return = array();
		if (isset($_POST['setNewPasswordRandom'])) {
			$_POST['newPassword1'] = generateRandomPassword();
			$return[] = array('INFO', _('The password was set to:') . ' ' . $_POST['newPassword1']);
		}
		else {
			// check if passwords match
			if ($_POST['newPassword1'] != $_POST['newPassword2']) {
				$return[] = array('ERROR', _('Passwords are different!'));
				return $return;
			}
			// check passsword stregth
			$pwdPolicyResult = checkPasswordStrength($_POST['newPassword1']);
			if ($pwdPolicyResult !== true) {
				$return[] = array('ERROR', $pwdPolicyResult);
				return $return;
			}
		}
		// set new password
		$selectedModules = array();
		foreach ($_POST as $key => $value) {
			if (substr($key, 0, 3) == 'cb_') {
				$name = substr($key, 3);
				$selectedModules[] = $name;
			}
		}
		foreach ($this->module as $name => $module) {
			if ($module instanceof passwordService) {
				$return = array_merge($return, $module->passwordChangeRequested($_POST['newPassword1'], $selectedModules));
			}
		}
		return $return;		
	}
	
	
	/**
	 * Prints common controls like the save button and the ou selection.
	 */
	private function printCommonControls() {
		echo "<table class=\"".$this->type."list\" style=\"border-width:0px;\" width=\"100%\"><tr>";
		echo "<td align=\"left\">";
		// save button
		echo "<input class=\"smallImageButton\" style=\"background-image: url(../../graphics/save.png);\" name=\"accountContainerSaveAccount\" type=\"submit\" value=\"&nbsp;&nbsp;&nbsp;&nbsp;" . _('Save') . "\"> \n";
		// reset button
		if ($this->dn_orig!='') {
			echo "<input class=\"smallImageButton\" style=\"background-image: url(../../graphics/undo.png);\" name=\"accountContainerReset\" type=\"submit\" value=\"&nbsp;&nbsp;&nbsp;&nbsp;" . _('Reset changes') . "\">\n";
		}
		if ($this->showSetPasswordButton()) {
			echo "&nbsp;&nbsp;&nbsp;&nbsp;";
			echo "<input class=\"smallImageButton\" style=\"background-image: url(../../graphics/key.png);\" name=\"accountContainerSetPassword\" type=\"submit\" value=\"&nbsp;&nbsp;&nbsp;&nbsp;" . _('Set password') . "\"> \n";
		}
		echo "</td>";
		echo "<td align=\"right\">";
		// profile selection
		$profilelist = getAccountProfiles($this->type);
		if (sizeof($profilelist) > 0) {
			sort($profilelist);
			echo "<select name=\"accountContainerSelectLoadProfile\" size=1>\n";
				for ($i = 0; $i < sizeof($profilelist); $i++) {
					$selected = '';
					if ($profilelist[$i] === $this->lastLoadedProfile) {
						$selected = 'selected';
					}
					echo "<option $selected>" . $profilelist[$i] . "</option>\n";
				}
			echo "</select> \n";
			echo "<input style=\"background-image: url(../../graphics/loadProfile.png);background-position: 2px center;background-repeat: no-repeat;height:24px;\" type=\"submit\" name=\"accountContainerLoadProfile\" value=\"&nbsp;&nbsp;&nbsp;&nbsp;" . _('Load profile') . "\"> &nbsp;\n";
			printHelpLink(getHelp('', '401'), '401');
		}
		echo "</td>";
		echo "</tr></table>";		
	}
	
	/**
	 * Returns if the page should show a button to set the password.
	 *
	 * @return boolean show or hide button
	 */
	private function showSetPasswordButton() {
		foreach ($this->module as $name => $module) {
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
		echo $_SESSION['header'];
		echo "<title>LDAP Account Manager</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $this->type . ".css\">\n";
		echo "</head><body>\n";
		echo "<script type=\"text/javascript\" src=\"../wz_tooltip.js\"></script>\n";
		echo "<form enctype=\"multipart/form-data\" action=\"edit.php\" method=\"post\">\n";		
	}
	
	/**
	 * Prints the footer of the account pages.
	 */
	private function printPageFooter() {
		echo "</form>\n";
		echo "</body>\n";
		echo "</html>\n";		
	}
	
	/**
	 * Prints the HTML code to notify the user about the successful saving.
	 *
	 */
	private function printSuccessPage() {
		$pdfStructures = getPDFStructureDefinitions($this->type);
		$this->printPageHeader();
		// Show success message
		if ($this->dn_orig == '') {
			$text = _("Account was created successfully.");
		}
		else {
			$text = _("Account was modified successfully.");
		}
		StatusMessage('INFO', _('LDAP operation successful.'), $text);
		
		echo "<br>\n";
		echo "<table class=\"".$this->type."list\" width=\"100%\" style=\"border-width:0px;\"><tr><td>\n";
		
		$type = new $this->type();
		echo "<input name=\"accountContainerCreateAgain\" type=\"submit\" value=\"" . $type->LABEL_CREATE_ANOTHER_ACCOUNT . "\"> \n";
		echo "<input name=\"accountContainerBackToList\" type=\"submit\" value=\"" . $type->LABEL_BACK_TO_ACCOUNT_LIST . "\"> \n";
		echo "&nbsp;&nbsp;&nbsp;&nbsp;";
		echo "<select name=\"pdfStructure\" size=1>\n";
		for ($i = 0; $i < sizeof($pdfStructures); $i++) {
			echo "<option>" . $pdfStructures[$i] . "</option>\n";
		}
		echo "</select>\n";
		echo "<input name=\"accountContainerCreatePDF\" type=\"submit\" value=\"" . _('Create PDF file') . "\">\n";
		printHelpLink(getHelp('', '403'), '403');
		echo "</td></tr></table>\n";
		$this->printPageFooter();
	}
	
	/**
	 * Checks if the user requested to load a profile.
	 * 
	 * @return boolean true, if profile was loaded
	 */
	private function loadProfileIfRequested() {
		if (isset($_POST['accountContainerLoadProfile']) && isset($_POST['accountContainerSelectLoadProfile'])) {
			$profile = loadAccountProfile($_POST['accountContainerSelectLoadProfile'], $this->type);
			$this->lastLoadedProfile = $_POST['accountContainerSelectLoadProfile'];
			// pass profile to each module
			$modules = array_keys($this->module);
			foreach ($modules as $module) $this->module[$module]->load_profile($profile);
			if (isset($profile['ldap_rdn'][0])) {
				if (in_array($profile['ldap_rdn'][0], getRDNAttributes($this->type))) {
					$this->rdn = $profile['ldap_rdn'][0];
				}
			}
			if (isset($profile['ldap_suffix'][0])) {
				$this->dn = $profile['ldap_suffix'][0];
			}
			return true;
		}
		return false;
	}
	
	/**
	 * Prints the HTML code of the module tabs.
	 */
	private function printModuleTabs() {
		// calculate button width
		$buttonWidth = 0;
		for ($b = 0; $b < sizeof($this->order); $b++) {
			$tempWidth = round(0.8 * strlen(utf8_decode($this->module[$this->order[$b]]->get_alias()))) + 2;
			if ($buttonWidth < $tempWidth) $buttonWidth = $tempWidth;
		}
		// $x is used to count up tabindex
		$x=0;
		echo "<table width=\"100%\" border=0 style=\"border-collapse: collapse;\">";
		echo "<tr>\n";
		// Loop for module
		for ($i=0; $i<count($this->order); $i++ ) {
			$buttonStatus = $this->module[$this->order[$i]]->getButtonStatus();
			// skip hidden buttons
			if ($buttonStatus == 'hidden') continue;
			$class = $this->type . "list-head";
			if ($this->order[$this->current_page] == $this->order[$i]) $class = $this->type . "list-edit";
			echo "<td class=\"settingsTab\" width=\"$buttonWidth\">\n";
			echo "<table width=\"100%\" style=\"border-collapse: collapse;\">\n";
			echo "<tr class=\"" . $class . "\"><td onclick=\"document.getElementsByName('form_main_".$this->order[$i]."')[0].click();\"";
				echo " align=\"center\" style=\"border: 1px solid;border-bottom: 0px solid;padding:5px;\">\n";
			// print normal button
			$buttonImage = $this->module[$this->order[$i]]->getIcon();
			$buttonImageStyle = '';
			$buttonSpace = '';
			if ($buttonImage != null) {
				$buttonImageStyle = 'background-image: url(../../graphics/' . $buttonImage . ');background-position: 2px center;background-repeat: no-repeat;height:32px;';
				$buttonSpace = '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
			}
			$buttonStyle = $buttonImageStyle . 'background-color:transparent;width:' . $buttonWidth . 'em;border:0px solid;';
			echo "<input style=\"" . $buttonStyle . "\" name=\"form_main_".$this->order[$i]."\" type=\"submit\" value=\"";
			echo $buttonSpace . $this->module[$this->order[$i]]->get_alias();
			echo "\" tabindex=$x";
			if ($buttonStatus == 'disabled') echo " disabled";
			echo ">\n";
			echo "</td></tr></table>\n";
			echo '</td>';
			$x++;
		}
		echo "<td width=\"100%\">&nbsp;</td></tr></table>\n";		
	}
	
	/**
	 * Prints the head part of the content area.
	 */
	private function printContentHeader() {
		echo "<table width=\"100%\" border=0><tr>\n";
		echo "<td align=\"left\">\n";
			// display DN
			if (isset($this->dn_orig) && ($this->dn_orig != '')) {
				echo _("DN") . ": <b>" . htmlspecialchars($this->dn_orig) . "</b>";
			}
		echo "</td>\n";
		echo "<td align=\"right\">\n";
			echo _('Suffix') . ": ";
			echo "<select name=\"accountContainerSuffix\" size=1>\n";
				// loop through all suffixes
				$rootsuffix = $_SESSION['config']->get_Suffix($this->type);
				foreach ($_SESSION['ldap']->search_units($rootsuffix) as $suffix) {
					echo "<option ";
					if ($this->dn == $suffix) {
						echo 'selected';
					}
					echo ">" . $suffix . "</option>\n";
				}
				if (!($this->dn == '') && !in_array($this->dn, $_SESSION['ldap']->search_units($rootsuffix))) {
					echo "<option selected>" . $this->dn . "</option>\n";;
				}
			echo "</select>\n";
			echo '&nbsp;&nbsp;&nbsp;&nbsp;';
			// RDN selection
			$rdnlist = getRDNAttributes($this->type);
			echo _('RDN identifier') . ": ";
			echo "<select name=\"accountContainerRDN\" size=1>\n";
				for ($i = 0; $i < sizeof($rdnlist); $i++) {
					echo "<option ";
					if ($this->rdn === $rdnlist[$i]) {
						echo 'selected';
					}
					echo ">" . $rdnlist[$i] . "</option>\n";
				}
			echo "</select>\n";
			printHelpLink(getHelp('', '301'), '301');
		echo "</td>\n";
		echo "</tr></table>\n";
		// separator line
		echo '<hr class="modulePage" noshade>';
	}

	/**
	* This function checks which LDAP attributes have changed while the account was edited.
	*
	* @param array $attributes list of current LDAP attributes
	* @param array $orig list of old attributes when account was loaded
	* @return array an array which can be passed to $this->saveAccount()
	*/
	function save_module_attributes($attributes, $orig) {
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
				foreach ($orig[$name] as $j => $value) {
					if (is_array($attributes[$name])) {
						if (!in_array($value, $attributes[$name])) {
							if (($value != '') && ($name != 'objectClass')) $torem[$name][] = $value;
						}
					}
					else if (($value != '') && ($name != 'objectClass')) $torem[$name][] = $value;
				}
			}
			// find new attributes
			if (isset($attributes[$name]) && is_array($attributes[$name])) {
				foreach ($attributes[$name] as $j => $value) {
					if (is_array($orig[$name])) {
						if (!in_array($value, $orig[$name]))
							if ($value != '') {
								$toadd[$name][] = $value;
							}
					}
					else if ($value != '') $toadd[$name][] = $value;
				}
			}
			// find unchanged attributes
			if (is_array($orig[$name]) && is_array($attributes[$name])) {
				foreach ($attributes[$name] as $j => $value) {
					if (($value != '') && in_array($value, $orig[$name])) {
						$notchanged[$name][] = $value;
					}
				}
			}
		}
		// create modify with add and remove
		$attributes2 = array_keys($toadd);
		for ($i=0; $i<count($attributes2); $i++) {
			if (isset($torem[$attributes2[$i]]))
				if ((count($toadd[$attributes2[$i]]) > 0) && (count($torem[$attributes2[$i]]) > 0)) {
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
		if (count($toadd)!=0) $return[$this->dn]['add'] = $toadd;
		if (count($torem)!=0) $return[$this->dn]['remove'] = $torem;
		if (count($tomodify)!=0) $return[$this->dn]['modify'] = $tomodify;
		if (count($notchanged)!=0) $return[$this->dn]['notchanged'] = $notchanged;
		return $return;
		}

	/**
	* Loads an LDAP account with the given DN.
	*
	* @param string $dn the DN of the account
	* @return array error messages
	*/
	function load_account($dn) {
		$this->module = array();
		$modules = $_SESSION['config']->get_AccountModules($this->type);
		$search = substr($dn, 0, strpos($dn, ','));
		$result = @ldap_read($_SESSION['ldap']->server(), escapeDN($dn), escapeDN($search), array(), 0, 0, 0, LDAP_DEREF_NEVER); // TODO use ldap_read()
		if (!$result) {
			return array(array("ERROR", _("Unable to load LDAP entry:") . " " . $dn, ldap_error($_SESSION['ldap']->server())));
		}
		$entry = @ldap_first_entry($_SESSION['ldap']->server(), $result);
		if (!$entry) {
			return array(array("ERROR", _("Unable to load LDAP entry:") . " " . $dn, ldap_error($_SESSION['ldap']->server())));
		}
		$this->dn = substr($dn, strpos($dn, ',')+1);
		$this->dn_orig = $dn;
		// extract RDN
		$this->rdn = explode("=", substr($dn, 0, strpos($dn, ',')));
		$this->rdn = $this->rdn[0];
		$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		// remove 'count' entries and numerical entries
		for ($i = 0; $i < count($attr); $i++) {
			if (isset($attr[$i])) unset($attr[$i]);
		}
		$attrNames = array_keys($attr);
		for ($i = 0; $i < sizeof($attrNames); $i++) unset($attr[$attrNames[$i]]['count']);
		unset($attr['count']);
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
		// save original attributes
		$this->attributes_orig = $attr;

		foreach ($modules as $module) {
			if (!isset($this->module[$module])) {
				$this->module[$module] = new $module($this->type);
				$this->module[$module]->init($this->base);
			}
			$this->module[$module]->load_attributes($attr);
			}

		// sort module buttons
		$this->sortModules();
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
		if (!is_array($attributes)) return $attributes;
		$keys = array_keys($attributes);
		// get correct object class names, aliases and attributes
		$objectClasses = array();
		$aliases = array();
		$ldapAttributesTemp = array();
		foreach ($modules as $module) {
			$moduleObj = new $module($this->type);
			$objectClasses = array_merge($objectClasses, $moduleObj->getManagedObjectClasses());
			$aliases = array_merge($aliases, $moduleObj->getLDAPAliases());
			$ldapAttributesTemp = array_merge($ldapAttributesTemp, $moduleObj->getManagedAttributes());
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
	function new_account() {
		$this->isNewAccount = true;
		$this->lastLoadedProfile = 'default';
		$modules = $_SESSION['config']->get_AccountModules($this->type);
		foreach ($modules as $module) {
			$this->module[$module] = new $module($this->type);
			$this->module[$module]->init($this->base);
		}
		// sort module buttons
		$this->sortModules();
		$profile = loadAccountProfile('default', $this->type);
		// pass profile to each module
		$modules = array_keys($this->module);
		foreach ($modules as $module) $this->module[$module]->load_profile($profile);
		if (isset($profile['ldap_rdn'][0])) {
			if (in_array($profile['ldap_rdn'][0], getRDNAttributes($this->type))) {
				$this->rdn = $profile['ldap_rdn'][0];
			}
		}
		if (isset($profile['ldap_suffix'][0])) {
			$this->dn = $profile['ldap_suffix'][0];
		}
		return 0;
		}

	/**
	* This function will save an account to the LDAP database.
	*
	* @return array list of status messages if any errors occured
	*/
	function save_account() {
		if (!checkIfWriteAccessIsAllowed()) {
			die();
		}
		$this->finalDN = $this->dn;
		$errors = array();
		$ldapUser = $_SESSION['ldap']->decrypt_login();
		$ldapUser = $ldapUser[0];
		$module = array_keys ($this->module);
		$attributes = array();
		// load attributes
		foreach ($module as $singlemodule) {
			// load changes
			$temp = $this->module[$singlemodule]->save_attributes();
			if (!is_array($temp)) $temp = array();
			// merge changes
			$DNs = array_keys($temp);
			if (is_array($temp)) $attributes = array_merge_recursive($temp, $attributes);
			for ($i=0; $i<count($DNs); $i++) {
				$ops = array_keys($temp[$DNs[$i]]);
				for ($j=0; $j<count($ops); $j++) {
					$attrs = array_keys($temp[$DNs[$i]][$ops[$j]]);
					for ($k=0; $k<count($attrs); $k++)
						$attributes[$DNs[$i]][$ops[$j]][$attrs[$k]] = array_unique($attributes[$DNs[$i]][$ops[$j]][$attrs[$k]]);
					}
				}
			}
		// Complete dn with RDN attribute
		$search = $this->rdn;
		$added = false;
		foreach ($attributes as $DN) {
			if (isset($DN['modify'][$search][0]) && !$added) {
				$attributes[$search.'='.$DN['modify'][$search][0].','.$this->finalDN] = $attributes[$this->finalDN];
				unset ($attributes[$this->finalDN]);
				$this->finalDN = $search.'='.$DN['modify'][$search][0].','.$this->finalDN;
				$added = true;
				}
			if (isset($DN['add'][$search][0]) && !$added) {
				$attributes[$search.'='.$DN['add'][$search][0].','.$this->finalDN] = $attributes[$this->finalDN];
				unset ($attributes[$this->finalDN]);
				$this->finalDN = $search.'='.$DN['add'][$search][0].','.$this->finalDN;
				$added = true;
				}
			if (isset($DN['notchanged'][$search][0]) && !$added) {
				$attributes[$search.'='.$DN['notchanged'][$search][0].','.$this->finalDN] = $attributes[$this->finalDN];
				unset ($attributes[$this->finalDN]);
				$this->finalDN = $search.'='.$DN['notchanged'][$search][0].','.$this->finalDN;
				$added = true;
				}
			}
		// Add old dn if dn hasn't changed
		if (!$added) {
			$attributes[$this->dn_orig] = $attributes[$this->finalDN];
			unset ($attributes[$this->finalDN]);
			$this->finalDN = $this->dn_orig;
		}
		// pre modify actions
		$currentAccountAttributes = array();
		if (isset($attributes[$this->finalDN]) && is_array($attributes[$this->finalDN])) {
			if (isset($attributes[$this->finalDN]['modify'])) {
				$currentAccountAttributes = array_merge($currentAccountAttributes, $attributes[$this->finalDN]['modify']);
			}
			if (isset($attributes[$this->finalDN]['add'])) {
				$currentAccountAttributes = array_merge($currentAccountAttributes, $attributes[$this->finalDN]['add']);
			}
			if (isset($attributes[$this->finalDN]['notchanged'])) {
				$currentAccountAttributes = array_merge($currentAccountAttributes, $attributes[$this->finalDN]['notchanged']);
			}
		}
		$currentAccountAttributes['dn'][0] = $this->finalDN;
		$preModifyOk = true;
		foreach ($module as $singlemodule) {
			$result = $this->module[$singlemodule]->preModifyActions($this->isNewAccount, $currentAccountAttributes);
			if (!$result) {
				$preModifyOk = false;
				break;
			}
		}
		if (!$preModifyOk) {
			$errors[] = array('ERROR', _('The operation was stopped because of the above errors.'));
			return $errors;
		}
		// Set to true if an real error has happened
		$stopprocessing = false;
		if (strtolower($this->finalDN) != strtolower($this->dn_orig)) {
			// move existing DN
			if ($this->dn_orig!='') {
				$success = ldap_rename($_SESSION['ldap']->server(), $this->dn_orig, $this->getRDN($this->finalDN), $this->getParentDN($this->finalDN), false);
				if ($success) {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Renamed DN ' . $this->dn_orig . " to " . $this->finalDN);
				}
				else {
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to rename DN: ' . $this->dn_orig . ' (' . ldap_error($_SESSION['ldap']->server()) . ').');
					$errors[] = array('ERROR', sprintf(_('Was unable to rename DN: %s.'), $this->dn_orig), ldap_error($_SESSION['ldap']->server()));
					$stopprocessing = true;
				}
			}
			// create complete new dn
			else {
				$attr = array();
				if (is_array($attributes[$this->finalDN]['add'])) $attr = array_merge_recursive($attr, $attributes[$this->finalDN]['add']);
				if (is_array($attributes[$this->finalDN]['notchanged'])) $attr = array_merge_recursive($attr, $attributes[$this->finalDN]['notchanged']);
				if (is_array($attributes[$this->finalDN]['modify'])) $attr = array_merge_recursive($attr, $attributes[$this->finalDN]['modify']);
				$success = ldap_add($_SESSION['ldap']->server(), $this->finalDN, $attr);
				if (!$success) {
					logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to create DN: ' . $this->finalDN . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
					$errors[] = array('ERROR', sprintf(_('Was unable to create DN: %s.'), $this->finalDN), ldap_error($_SESSION['ldap']->server()));
					$stopprocessing = true;
				}
				else {
					logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Created DN: ' . $this->finalDN);
				}
				unset($attributes[$this->finalDN]);
			}
		}
		$DNs = array_keys($attributes);
		for ($i=0; $i<count($DNs); $i++) {
			if (!$stopprocessing) {
				// modify attributes
				if (isset($attributes[$DNs[$i]]['modify']) && !$stopprocessing) {
					$success = @ldap_mod_replace($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['modify']);
					if (!$success) {
						logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to modify attribtues from DN: ' . $DNs[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
						$errors[] = array('ERROR', sprintf(_('Was unable to modify attribtues from DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
					}
					else {
						logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Modified DN: ' . $DNs[$i]);
					}
				}
				// add attributes
				if (isset($attributes[$DNs[$i]]['add']) && !$stopprocessing) {
					$success = @ldap_mod_add($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['add']);
					if (!$success) {
						logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to add attribtues to DN: ' . $DNs[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
						$errors[] = array('ERROR', sprintf(_('Was unable to add attribtues to DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
					}
					else {
						logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Modified DN: ' . $DNs[$i]);
					}
				}
				// remove attributes
				if (isset($attributes[$DNs[$i]]['remove']) && !$stopprocessing) {
					$success = @ldap_mod_del($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['remove']);
					if (!$success) {
						logNewMessage(LOG_ERR, '[' . $ldapUser .'] Unable to delete attribtues from DN: ' . $DNs[$i] . ' (' . ldap_err2str(ldap_errno($_SESSION['ldap']->server())) . ').');
						$errors[] = array('ERROR', sprintf(_('Was unable to remove attribtues from DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
					}
					else {
						logNewMessage(LOG_NOTICE, '[' . $ldapUser .'] Modified DN: ' . $DNs[$i]);
					}
				}
			}
		}

		if (!$stopprocessing) {
			// post modify actions
			foreach ($module as $singlemodule) {
				$this->module[$singlemodule]->postModifyActions($this->isNewAccount, $currentAccountAttributes);
			}
		}
		$_SESSION['cache']->refresh_cache(true);
		return $errors;
	}

	/**
	 * Returns a list of possible PDF entries for this account.
	 *
	 * @return list of PDF entries (array(<name> => <PDF lines>))
	 */
	function get_pdfEntries() {
		$return = array();
		while(($current = current($this->module)) != null) {
			$return = array_merge($return,$current->get_pdfEntries());
			next($this->module);
		}
		$return = array_merge($return,array('main_dn' => array('<block><key>' . _('DN') . '</key><value>' . $this->dn . '</value></block>')));
		return $return;
	}

	/**
	* Sorts the module buttons for the account page.
	*/
	function sortModules() {
		$order = array();
		$modules = array_keys($this->module);
		$depModules = array();
		for ($i = 0; $i < sizeof($modules); $i++) {
			// insert waiting modules
			for ($w = 0; $w < sizeof($depModules); $w++) {
				$dependencies = $this->module[$depModules[$w]]->get_dependencies($this->type);
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
			$dependencies = $this->module[$modules[$i]]->get_dependencies($this->type);
			if (is_array($dependencies['depends'])) {
				$everything_found = true;
				$dependencies = $dependencies['depends'];
				for ($d = 0; $d < sizeof($dependencies); $d++) {
					if (!in_array($dependencies[$d], $order)) {
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
			for ($i = 0; $i < sizeof($depModules); $i++) $order[] = $depModules[$i];
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
	}
	
	/**
	 * Returns the RDN part of a given DN.
	 *
	 * @param String $dn DN
	 * @return String RDN
	 */
	function getRDN($dn) {
		if (($dn == "") || ($dn == null)) return "";
		$rdn = substr($dn, 0, strpos($dn, ","));
		return $rdn;
	}
	
	/**
	 * Returns the parent DN of a given DN.
	 *
	 * @param String $dn DN
	 * @return String DN
	 */
	function getParentDN($dn) {
		if (($dn == "") || ($dn == null)) return "";
		$parent = substr($dn, strpos($dn, ",") + 1);
		return $parent;		
	}

	/**
	* Encrypts sensitive data before storing in session.
	*
	* @return array list of attributes which are serialized
	*/
	function __sleep() {
		// encrypt data
		$this->attributes = $_SESSION['ldap']->encrypt(serialize($this->attributes));
		$this->attributes_orig = $_SESSION['ldap']->encrypt(serialize($this->attributes_orig));
		$this->module = $_SESSION['ldap']->encrypt(serialize($this->module));
		// save all attributes
		return array_keys(get_object_vars($this));
	}

	/**
	* Decrypts sensitive data after accountContainer was loaded from session.
	*/
	function __wakeup() {
		// decrypt data
		$this->attributes = unserialize($_SESSION['ldap']->decrypt($this->attributes));
		$this->attributes_orig = unserialize($_SESSION['ldap']->decrypt($this->attributes_orig));
		$this->module = unserialize($_SESSION['ldap']->decrypt($this->module));
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
	 * This function is called whenever the password should be changed. Account modules
	 * must change their password attributes only if the modules list contains their module name.
	 *
	 * @param String $password new password
	 * @param $modules list of modules for which the password should be changed
	 * @return array list of error messages if any as parameter array for StatusMessage
	 *               e.g. return arrray(array('ERROR', 'Password change failed.'))
	 */
	public function passwordChangeRequested($password, $modules);
	
}

?>
