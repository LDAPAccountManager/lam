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
		$return[$modules[$i]] = $m->get_configOptions($scopes[$modules[$i]]);
	}
	return $return;
}

/**
* Returns a hash array (module name => descriptions) containing descriptions shown on configuration pages.
*
* The returned array has the format array('legend' => array('posixAccount' => '...', ...), descriptions => array('option1' => '...', ...)).
* <br> The "legend" value is used as text for the fieldset, the descriptions are used when the configuration is printed.
*
* @return array configuration descriptions
*/
function getConfigDescriptions() {
	$return = array('legend' => array(), 'descriptions' => array());
	$modules = array_merge(getAvailableModules('user'), getAvailableModules('group'), getAvailableModules('host'));
	$modules = array_values(array_unique($modules));
	for ($i = 0; $i < sizeof($modules); $i++) {
		$m = new $modules[$i]('none');
		$desc = $m->get_configDescriptions();
		$return['legend'][$modules[$i]] = $desc['legend'];
		$return['descriptions'] = array_merge($return['descriptions'], $desc['descriptions']);
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
		$return = array_merge($return, $errors);
	}
	return $return;
}

/**
* Returns a help entry from an account module.
*
* @param string $helpID help identifier
* @param string $module module name
* @return array help entry
*/
function getHelp($module,$helpID,$scope='') {
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
 * Return a list of current available scopes
 *
 * @return array Available scopes
 */
function getAvailableScopes() {
	return array('user','group','host', 'domain');
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
* Takes a list of meta-HTML elements and prints the equivalent HTML output.
*
* @param string $module Name of account module
* @param array $input List of meta-HTML elements
* @param array $values List of values which override the defaults in $input (name => value)
* @param boolean $restricted If true then no buttons will be displayed
* @param integer $tabindex Start value of tabulator index for input fields
* @param integer $tabindexLink Start value of tabulator index for links
* @param string $scope Account type
* @return array List of input field names and their type (name => type)
*/
function parseHtml($module, $input, $values, $restricted, &$tabindex, &$tabindexLink, $scope) {
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
						echo $input[$i][$j]['text'];
						break;
					// input fields
					case 'input':
						$type = $input[$i][$j]['type'];
						if ($restricted && (($type == "submit") || ($type == "reset") || ($type == "file"))) break;	// no buttons in restricted mode
						$output = "<input";
						if ($input[$i][$j]['name']!='') $output .= ' name="' . $input[$i][$j]['name'] . '"';
						if ($type != '') $output .= ' type="' . $type . '"';
						if ($input[$i][$j]['size']!='') $output .= ' size="' . $input[$i][$j]['size'] . '"';
						if ($input[$i][$j]['maxlength']!='') $output .= ' maxlength="' . $input[$i][$j]['maxlength'] . '"';
						// checkbox
						if ($type == "checkbox") {
							if (isset($values[$input[$i][$j]['name']])) {
								if ($values[$input[$i][$j]['name']][0] == "true") $output .= ' checked';
							}
							elseif ($input[$i][$j]['checked']) $output .= ' checked';
						}
						// other input element
						else {
							if (isset($values[$input[$i][$j]['name']])) {
								$output .= ' value="' . $values[$input[$i][$j]['name']][0] . '"';
							}
							elseif ($input[$i][$j]['value']!='') $output .= ' value="' . $input[$i][$j]['value'] . '"';
						}
						if ($input[$i][$j]['disabled']) $output .= ' disabled';
						// Show taborder
						else {
							$output .= " tabindex=$tabindex";
							$tabindex++;
						}
						$output .= ">";
						echo $output;
						$ret[$input[$i][$j]['name']] = $type;	// save type
						break;
					// inner fieldset
					case 'fieldset':
						echo "<fieldset>\n";
						if ($input[$i][$j]['legend']!='') echo "<legend>" . $input[$i][$j]['legend'] . "</legend>\n";
						parseHtml($module, $input[$i][$j]['value'], $values, $restricted, $tabindex, $tabindexLink, $scope);
						echo "</fieldset>\n";
						break;
					// selection
					case 'select':
						if (! is_numeric($input[$i][$j]['size'])) $input[$i][$j]['size'] = 1;	// correct size if needed
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
						$options = array_merge ($input[$i][$j]['options'], $input[$i][$j]['options_selected'] );
						$options = array_unique($options);
						if (!$input[$i][$j]['noSorting']) {
							if (get_preg($options[0], 'digit')) sort($options, SORT_NUMERIC);
							else sort($options, SORT_STRING);
						}
						foreach ($options as $option) {
							if ($option!='') {
								if (in_array($option, $input[$i][$j]['options_selected'])) echo "<option selected>" . $option . "</option>\n";
								else echo "<option>" . $option . "</option>\n";
							}
						}
						echo "</select>\n";
						break;
					// sub table
					case 'table':
						$retTemp = parseHtml($module, $input[$i][$j]['value'], $values, $restricted, $tabindex, $tabindexLink, $scope);
						$ret = array_merge($ret, $retTemp);
						break;
					// help link
					case 'help':
						$helpPath = "../";
						if (is_file("./help.php")) $helpPath = "";
						echo "<a href=" . $helpPath . "help.php?module=$module&amp;HelpNumber=". $input[$i][$j]['value'] . "&amp;scope=" . $scope . " target=\"help\" tabindex=$tabindexLink>";
						echo "<img src=\"../$helpPath/graphics/help.png\" alt=\"" . _('Help') . "\" title=\"" . _('Help') . "\">";
						echo "</a>\n";
						$tabindexLink++;
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
			}
			echo "</td>\n";
			echo "</tr>\n";
		}
	}
	echo "</table>\n";
	return $ret;
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
	function accountContainer($type, $base) {
		/* Set the type of account. Valid
		* types are: user, group, host
		*/
		// Check input variable
		
		if (!is_string($type)) trigger_error('Argument of accountContainer must be string.', E_USER_ERROR);
		if (!is_string($base)) trigger_error('Argument of accountContainer must be string.', E_USER_ERROR);
		// *** fixme use global variable to determine allowed types
		if (!in_array($type, getAvailableScopes())) trigger_error('Account type not recognized.', E_USER_ERROR);
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
	var $attributes;
	
	/**
	* This variale stores the account type.
	* Currently "user", "group" and "host" are supported.
	*/
	var $type;
	
	/** This is an array with all module objects */
	var $module;
	
	/** DN suffix of the account */
	var $dn;
	
	/** DN suffix of account when it was loaded */
	var $dn_orig;
	
	/** RDN attribute of this account */
	var $rdn;
	
	/** original LDAP attributes when account was loaded from LDAP */
	var $attributes_orig;
	
	/** Module order */
	var $order;
	
	/** Name of accountContainer variable in session */
	var $base;
	
	/** This variable stores the name of the currently displayed page */
	var $current_page;
	
	/** This variable is set to the pagename of a subpage if it should be displayed */
	var $subpage;
	
	/** True if this is a newly created account */
	var $isNewAccount;

	
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
	*
	* @param array $post HTTP POST variables
	*/
	function continue_main($post) {
		if ($this->subpage=='') $this->subpage='attributes';
		if ($post['form_main_reset']) {
			$this->load_account($this->dn_orig);
			}
		else {
			if ($this->current_page==0) {
				if ($this->subpage=='attributes') {
					$result = 0;
					// change dn
					if ($post['suffix']!='') $this->dn = $post['suffix'];
					// change RDN
					if (isset($post['rdn'])) $this->rdn = $post['rdn'];
						
					// load profile
					if ($post['selectLoadProfile'] && $post['loadProfile']) {
						$profile = loadAccountProfile($post['selectLoadProfile'], $this->type);
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
						$result = 0;
						}
					// save account
					if ($post['create']) {
						$errors = $this->save_account();
						if (is_array($errors)) {
							$result = array($errors);
							$stopProcessing = true;
						}
						else $result = 'finish';
						}
					}
				if ($this->subpage=='finish') {
					if ($post['createagain']) {
						// Reset objects
						$modules = array_keys($this->module);
						foreach ($modules as $module) unset($this->module[$module]);
						// Reset accountContainer
						$this->dn = '';
						$this->dn_orig = '';
						$this->attributes = array();
						$this->order = array();
						$this->current_page = 0;
						$this->subpage = '';
						// Add all required objects etc.
						$this->new_account();
						$result = 0;
						}
					if ($post['backmain']) {
						// Return to *-list
						// *** fixme unset accountContainer in session
						metaRefresh("../lists/list".$this->type."s.php");
						exit;
						}
					if ($post['outputpdf']) {
						// Create / display PDf-file
						createModulePDF(array($_SESSION[$this->base]), $post['pdfStructure']);
						exit;
						}
					}
				}
				else $result = call_user_func(array(&$this->module[$this->order[$this->current_page]], 'process_'.$this->subpage), $post);
			}
		// change to next page
		if (is_string($result)) $this->subpage = $result;  // go to subpage of current module
		$errorsOccured = false;
		if (is_array($result)) {  // messages were returned, check for errors
			$errorKeys = array_keys($result);
			for ($i = 0; $i < sizeof($errorKeys); $i++) {
				for ($m = 0; $m < sizeof($result[$errorKeys[$i]]); $m++) {
					if (($result[$errorKeys[$i]][$m][0] == 'ERROR') || ($result[$errorKeys[$i]][$m][0] == 'WARN')) {
						$errorsOccured = true;
						break;
					}
				}
			}
		}
		if (is_int($result) || !$errorsOccured) {  // numeric return value means to change to another module
			if ($post['form_main_main']) {
				$this->current_page = 0;
				$this->subpage='attributes';
			}
			else for ($i=1; $i<count($this->order); $i++ )
				if (isset($post['form_main_'.$this->order[$i]])) {
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
		// Write HTML-Code
		echo $_SESSION['header'];
		echo "<title>";
		if ($this->dn_orig!='') echo _("Modify Account");
			else echo _("Create new Account");
		echo "</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";
		echo "<form action=\"edit.php\" method=\"post\">\n";
		// Display error-messages
		if (is_array($result)) {
			foreach ($result as $result2) {
				if (is_array($result2)) {
					for ($i=0; $i<sizeof($result2); $i++) StatusMessage($result2[$i][0], $result2[$i][1], $result2[$i][2]);
				}
			}
			if (isset($stopProcessing) && ($stopProcessing == true)) {
				echo "</body>\n";
				echo "</html>\n";
				return 0;
			}
		}
		if ($this->current_page==0) {
			if ($this->subpage=='attributes')  {
				$modules = array_keys($this->module);
				$disabled = false;
				$incompleteModules = array();
				foreach ($modules as $module) {
					if (!$this->module[$module]->module_complete()) {
						$disabled = true;
						$incompleteModules[] = $this->module[$module]->get_alias();
					}		
				}
				if (sizeof($incompleteModules) > 0) {
					StatusMessage('INFO', _('Some required information is missing'),
						sprintf(_('Please set up all required attributes on page: %s'), implode(", ", $incompleteModules)));
				}
			}
		}
		// Create left module-menu
		echo "<table border=0 width=\"100%\">\n<tr><td valign=\"top\" width=\"15%\" >";
		echo "<table><tr>";
		
		echo "<td><fieldset class=\"".$this->type."edit\"><legend><b>";
			echo _('Please select page:');
			echo "</b></legend>\n";
		$x=0;
		// print normal button
		echo "<input name=\"form_main_main\" type=\"submit\" value=\"";
		echo _('Main');
		echo "\" tabindex=$x";
		if ($this->subpage == 'finish') echo " disabled";
		echo ">\n<br>";
		$x++;
		// Loop for module
		// $x is used to count up tabindex
		for ($i=1; $i<count($this->order); $i++ ) {
			// print normal button
			echo "<input name=\"form_main_".$this->order[$i]."\" type=\"submit\" value=\"";
			echo $this->module[$this->order[$i]]->get_alias();
			echo "\" tabindex=$x";
			if ($this->subpage == 'finish') echo " disabled";
			echo ">\n<br>";
			$x++;
			}
		if ($this->dn_orig!='') {
			echo "<input name=\"form_main_reset\" type=\"submit\" value=\"" . _('Reset changes') . "\"";
			if ($this->subpage == 'finish') echo " disabled";
			echo "><br>\n";
		}
		echo "</fieldset></td></tr>\n";
		echo "</table></td>\n<td>";
		if ($this->current_page==0) {
			echo "<td><fieldset class=\"".$this->type."edit\"><legend><b>";
			echo _('Main');
			echo "</b></legend>\n";
			}
		else {
			echo "<td><fieldset class=\"".$this->type."edit\"><legend><b>";
				echo $this->module[$this->order[$this->current_page]]->get_alias();
				echo "</b></legend>\n";
			}
		// display html-code from mdule
		$return = array();
		if ($this->current_page == 0) {			
			if ($this->subpage == 'finish') {
				// Show success message
				if ($this->dn_orig == '') {
					$text = _("Account was created successfully.");
				}
				else {
					$text = _("Account was modified successfully.");
				}
				$return[] = array(
					0 => array('kind' => 'message', 'type' => 'INFO', 'headline' =>  _('LDAP operation successful.'), 'text' => $text,
						'td' => array('colspan' => 3)));
				$return[] = array(
					0 => array('kind' => 'text', 'text' => '', 'td' => array('colspan' => 3)));
				$return[] = array(
					0 => array('kind' => 'input', 'type' => 'submit', 'name' => 'createagain', 'value' => _('Create another account')),
					1 => array('kind' => 'input', 'type' => 'submit', 'name' => 'backmain', 'value' => _('Back to account list')));
				$return[] = array(
					0 => array('kind' => 'text', 'text' => '&nbsp;')
				);
				$pdfStructures = getPDFStructureDefinitions($this->type);
				$return[] = array(
					0 => array('kind' => 'fieldset', 'legend' => '<b>PDF</b>', 'td' => array('colspan' => 2), 'value' => array(array(
						0 => array('kind' => 'select', 'name' => 'pdfStructure', 'options' => $pdfStructures),
						1 => array('kind' => 'input', 'type' => 'submit', 'name' => 'outputpdf', 'value' => _('Create PDF file')))
					)));
			}
			else {
				// loop through all suffixes
				$rootsuffix = $_SESSION['config']->get_Suffix($this->type);
				foreach ($_SESSION['ldap']->search_units($rootsuffix) as $suffix) {
					if ($this->dn == $suffix) $options_selected = $suffix;
					$suffixes[] = $suffix;
				}
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('Suffix')),
					1 => array('kind' => 'select', 'name' => 'suffix', 'options' => $suffixes, 'options_selected' => array($options_selected), 'noSorting' => true),
					2 => array ('kind' => 'help', 'value' => 'suffix'));
				// RDN selection
				$rdnlist = getRDNAttributes($this->type);
				$rdnSelected = array();
				if ($this->rdn != '') {
					$rdnSelected[] = $this->rdn;
				}
				else {
					$rdnSelected[] = $rdnlist[0];
				}
				$return[] = array(
					0 => array('kind' => 'text', 'text' => _('RDN identifier')),
					1 => array('kind' => 'select', 'name' => 'rdn', 'options' => $rdnlist, 'options_selected' => $rdnSelected, 'noSorting' => true),
					2 => array ('kind' => 'help', 'value' => '301'));
		
				// Get list of profiles
				$profilelist = getAccountProfiles($this->type);
				if (count($profilelist)!=0) {
					$return[] = array(
						0 => array('kind' => 'text', 'text' => _("Load profile")),
						1 => array('kind' => 'table', 'value' => array(0 => array(
							0 => array('kind' => 'select', 'name' => 'selectLoadProfile', 'options' => $profilelist),
							1 => array('kind' => 'input', 'type' => 'submit', 'name' => 'loadProfile', 'value' => _('Load profile'))
							)),
						2 => array('kind' => 'help', 'value' => 'selectLoadProfile'))
					);
				}
				if ($this->dn_orig!='') $text = _('Modify Account');
					else $text = _('Create Account');
				$return[] = array ( 0 => array ( 'kind' => 'text', 'text' => $text ),
					1 => array ( 'kind' => 'input', 'type' => 'submit', 'name' => 'create', 'value' => $text, 'disabled' => $disabled),
					2 => array ('kind' => 'help', 'value' => 'create'));
			}
		}
		else $return = call_user_func(array($this->module[$this->order[$this->current_page]], 'display_html_'.$this->subpage), $post);
		$y = 5000;
		$z = 10000;
		parseHtml($this->order[$this->current_page], $return, array(), false, $y, $z, $this->type);
		// Display rest of html-page
		echo "</fieldset>\n";
		echo "</td></tr></table>\n";
		echo "</form>\n";
		echo "</body>\n";
		echo "</html>\n";
		return 0;
		}

	/**
	* Add attributes to variable. Syntax is array( attribute = array ( objectClass1 => MUST|MAX, objectClass2 => MUST|MAY ), ... )
	* @todo document this function
	*/
	function add_attributes($objectClass) {
		// loop through every existing objectlass and select current objectClass
		$line=-1;
		for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
			if (strpos(strtolower($_SESSION['ldap']->objectClasses[$i]), strtolower("NAME '$objectClass'"))) $line = $i;
			}
		// Return error if objectClass isn't found
		if ($line==-1) trigger_error (sprintf(_("ObjectClass %s required but not defined in LDAP."), $objectClass), E_USER_WARNING);
		// create array with must-attributes
		// Get startposition in string
		if (strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')+6);
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
		if (strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')+5);
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
		while (strpos($_SESSION['ldap']->objectClasses[$line], "SUP ")) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'SUP ')+4);
			$subclass = substr($string_withtail, 0, strpos($string_withtail, ' '));
			// Add account type to object
			for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME '$subclass'")) $line = $i;
				}
			// Return error if objectClass isn't found
			if ($line==-1) trigger_error (sprintf(_("ObjectClass %s required but not defined in LDAP."), $objectClass), E_USER_WARNING);
			// create array with must-attributes
			// Get startposition in string
			if (strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')+6);
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
			if (strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')+5);
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

	/**
	* This function return ldap attributes
	* Syntax is get_attributes($value, $scope)
	* $scope = 'objectClass', $value = objectClass return value are all attributes of objectClass
	* $scope = 'attribute', $value = attribute returns alle objectClasses which are using the attribute
	*
	* @todo Obsolete function???
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

		
	/**
	* This function returns all LDAP attributes which are used by the given object class.
	* LDAP attributs which are already in use by another object class are passed as reference.
	* <br>Therefore this function must be called as reference: $result =& ..get_module_attributes
	*
	* @param string $objectClass object class name
	* @param boolean $original If original is true references will be set to original attributes. This are the original attributes
	* when an LDAP entry is loaded.
	* @return array list of attributes
	*/
	function get_module_attributes($objectClass, $original=false) {
		// Add account type to object
		$line=-1;
		for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
			if (strpos(strtolower($_SESSION['ldap']->objectClasses[$i]), strtolower("NAME '$objectClass'"))) $line = $i;
			}
		// Return empty array if no objectClass wasn't found
		if ($line==-1) return array();
		if (strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')+6);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			// Add must
			foreach (explode(" $ ", $string) as $attribute) {
				$return[$attribute] = array('');
			}
		}
		elseif (strpos($_SESSION['ldap']->objectClasses[$line],  'MUST ')) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MUST ')+5);
			// Now we have a string with the must-attribute
			$string = substr($string_withtail, 0, strpos($string_withtail, ' '));
			$string = trim($string);
			// Add must
			$return[$string] = array('');
		}
		// create array with may-attributes
		// Get startposition in string
		if (strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')+5);
			// Now we have a string with all must-attributes
			$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
			$string = trim($string);
			// Add may
			foreach (explode(" $ ", $string) as $attribute) {
				$return[$attribute] = array('');
			}
		}
		elseif (strpos($_SESSION['ldap']->objectClasses[$line],  'MAY ')) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MAY ')+4);
			// Now we have a string with the may-attribute
			$string = substr($string_withtail, 0, strpos($string_withtail, ' '));
			$string = trim($string);
			// Add must
			$return[$string] = array('');
		}
		// Get attributes of subclasses
		while (strpos($_SESSION['ldap']->objectClasses[$line], "SUP ")) {
			$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'SUP ')+4);
			$subclass = substr($string_withtail, 0, strpos($string_withtail, ' '));
			// Add account type to object
			for ($i=0; $i<count($_SESSION['ldap']->objectClasses) || $i==-1; $i++) {
				if (strpos($_SESSION['ldap']->objectClasses[$i], "NAME '$subclass'")) $line = $i;
				}
			// Return error if objectClass isn't found
			if ($line==-1) trigger_error (sprintf(_("ObjectClass %s required but not defined in LDAP."), $subclass), E_USER_WARNING);
			// create array with must-attributes
			// Get startposition in string
			if (strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MUST (')+6);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				// Add must
				foreach (explode(" $ ", $string) as $attribute) {
					$return[$attribute] = array('');
				}
			}
			elseif (strpos($_SESSION['ldap']->objectClasses[$line],  'MUST ')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MUST ')+5);
				// Now we have a string with the must-attribute
				$string = substr($string_withtail, 0, strpos($string_withtail, ' '));
				$string = trim($string);
				// Add must
				$return[$string] = array('');
			}
			// create array with may-attributes
			// Get startposition in string
			if (strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MAY (')+5);
				// Now we have a string with all must-attributes
				$string = substr($string_withtail, 0, strpos($string_withtail, ')'));
				$string = trim($string);
				// Add may
				foreach (explode(" $ ", $string) as $attribute) {
					$return[$attribute] = array('');
				}
			}
			elseif (strpos($_SESSION['ldap']->objectClasses[$line],  'MAY ')) {
				$string_withtail = substr($_SESSION['ldap']->objectClasses[$line], strpos($_SESSION['ldap']->objectClasses[$line],  'MAY ')+4);
				// Now we have a string with the may-attribute
				$string = substr($string_withtail, 0, strpos($string_withtail, ' '));
				$string = trim($string);
				// Add must
				$return[$string] = array('');
			}
		}

		// make references with attibutes which are used by more than one module
		$newattributes = array_keys($return);
		$module = array_keys($this->module);
		if (!$original) {
			// Only add attributes when original is false. We don't want to add them twice
			$this->add_attributes($objectClass);
			for ($i=0; $i<count($module); $i++) {
				if ($module[$i]!=$objectClass)
					foreach ($newattributes as $attribute)
						if (isset($this->module[$module[$i]]->attributes[$attribute]) && ($attribute!='objectClass'))
							$return[$attribute] =& $this->module[$module[$i]]->attributes[$attribute];
				}
			}
		else {		
			for ($i=0; $i<count($module); $i++) {
				if ($module[$i]!=$objectClass)
					foreach ($newattributes as $attribute)
						if (isset($this->module[$module[$i]]->orig[$attribute]) && ($attribute!='objectClass'))
							$return[$attribute] =& $this->module[$module[$i]]->orig[$attribute];
				}
			}
		$return['objectClass'] = array();
		return $return;
		}

	/**
	* This function checks which LDAP attributes have changed while the account was edited.
	* 
	* @param array $attributes list of current LDAP attributes
	* @param array $orig list of old attributes when account was loaded
	* @return array an array which can be passed to $this->saveAccount()
	*/
	function save_module_attributes($attributes, $orig) {
		// Get list of all "easy" attributes
		$attr_names = array_keys($attributes);
		// Get attributes which should be added
		for ($i=0; $i<count($attr_names); $i++) {
			for ($j=0; $j<count($orig[$attr_names[$i]]); $j++) {
				if (is_array($attributes[$attr_names[$i]])) {
					if (!in_array($orig[$attr_names[$i]][$j], $attributes[$attr_names[$i]]))
						if (($orig[$attr_names[$i]][$j]!='') && ($attr_names[$i]!='objectClass')) $torem[$attr_names[$i]][] = $orig[$attr_names[$i]][$j];
					}
				else if (($orig[$attr_names[$i]][$j]!='') && ($attr_names[$i]!='objectClass')) $torem[$attr_names[$i]][] = $orig[$attr_names[$i]][$j];
				}
			for ($j=0; $j<count($attributes[$attr_names[$i]]); $j++) {
				if (is_array($orig[$attr_names[$i]])) {
					if (!in_array($attributes[$attr_names[$i]][$j], $orig[$attr_names[$i]]))
						if ($attributes[$attr_names[$i]][$j]!='') $toadd[$attr_names[$i]][] = $attributes[$attr_names[$i]][$j];
					}
				else if ($attributes[$attr_names[$i]][$j]!='') $toadd[$attr_names[$i]][] = $attributes[$attr_names[$i]][$j];
				}
			for ($j=0; $j<count($attributes[$attr_names[$i]]); $j++) {
				if (is_array($orig[$attr_names[$i]]) && is_array($attributes[$attr_names[$i]])) {
					if (($attributes[$attr_names[$i]][$j]==$orig[$attr_names[$i]][$j]) && $attributes[$attr_names[$i]][$j]!='')
						$notchanged[$attr_names[$i]][] = $attributes[$attr_names[$i]][$j];
					}
				}
			}
		// create modify with add and remove
		if (is_array($toadd)) {
			$attributes2 = array_keys($toadd);
			for ($i=0; $i<count($attributes2); $i++) {
				if (isset($torem[$attributes2[$i]]))
					/* found modify entry
					* Some ldap attributes must be set exactly one time.
					* Adding or removing such an attribute wont't work
					* because it would conflict with an ldap schema.
					* Therefore when an attribute has only one entry
					* and is set in $toadd and $torem this will be merged
					* to $tomodify
					*/
					if ((count($toadd[$attributes2[$i]]==0)) && (count($torem[$attributes2[$i]]==0))) {
						// found attribute which should only modified
						$tomodify[$attributes2[$i]] = $toadd[$attributes2[$i]];
						unset($toadd[$attributes2[$i]]);
						unset($torem[$attributes2[$i]]);
					}
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
	*/
	function load_account($dn) {
		$modules = $_SESSION['config']->get_AccountModules($this->type);
		$search = substr($dn, 0, strpos($dn, ','));
		$result = ldap_search($_SESSION['ldap']->server(), $dn, $search);
		$entry = ldap_first_entry($_SESSION['ldap']->server(), $result);
		$this->dn = substr($dn, strpos($dn, ',')+1);
		$this->dn_orig = $dn;
		// extract RDN
		$this->rdn = split("=", substr($dn, 0, strpos($dn, ',')));
		$this->rdn = $this->rdn[0];
		$attr = ldap_get_attributes($_SESSION['ldap']->server(), $entry);
		$attrNames = array_keys($attr);
		// remove 'count' entries and numerical entries
		for ($i = 0; $i < count($attr); $i++) {
			if (isset($attr[$i])) unset($attr[$i]);
		}
		for ($i = 0; $i < sizeof($attrNames); $i++) unset($attr[$attrNames[$i]]['count']);
		unset($attr['count']);
		// save original attributes
		$this->attributes_orig = $attr;
				
		foreach ($modules as $module) {
			if (!isset($this->module[$module])) {
				$this->module[$module] = new $module($this->type);
				$this->module[$module]->init($this->base);
			}
			$this->module[$module]->load_attributes($attr);
			}

		// sortm modules and make all active because all required attributes should be set
		$module = array_keys ($this->module);
		// loop until all modules are in order.
		// We don't want to loop forever
		$remain = count($module) * count($module);
		$order = array();
		while ( (count($module) != count($order)) && ($remain!=0) ) {
			$remain--;
			foreach ($module as $moduleitem) {
				$required = $this->module[$moduleitem]->get_dependencies($this->type);
				$everything_found = true;
				if (is_array($required['depends'])) {
					foreach ($required['depends'] as $requireditem) {
						if (!in_array($requireditem, $order)) {
							$everything_found = false;
							break;
						}
					}
				}
				if ($everything_found && !in_array($moduleitem, $order) ) $order[] = $moduleitem;
			}
		}
		// Write Module-Order in variable
		array_unshift($order, 'main');
		$this->order = $order;
		return 0;
		}

	/**
	* This function will prepare the object for a new account.
	*/
	function new_account() {
		$this->isNewAccount = true;
		$modules = $_SESSION['config']->get_AccountModules($this->type);
		foreach ($modules as $module) {
			$this->module[$module] = new $module($this->type);
			$this->module[$module]->init($this->base);
		}

		$module = array_keys ($this->module);
		// loop until all modules are in order.
		// We don't want to loop forever
		$remain = count($module) * count($module);
		$order = array();
		while ( (count($module) != count($order)) && ($remain!=0) ) {
			$remain--;
			foreach ($module as $moduleitem) {
				$required = $this->module[$moduleitem]->get_dependencies($this->type);
				$everything_found = true;
				if (is_array($required['depends'])) {
					foreach ($required['depends'] as $requireditem) {
						if (!in_array($requireditem, $order)) {
							$everything_found = false;
							break;
						}
					}
				}
				if ($everything_found && !in_array($moduleitem, $order) ) $order[] = $moduleitem;
			}
		}
		// Write Module-Order in variable
		array_unshift($order, 'main');
		$this->order = $order;
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
		$module = array_keys ($this->module);
		$attributes = array();
		// load attributes
		foreach ($module as $singlemodule) {
			// load changes
			$temp = $this->module[$singlemodule]->save_attributes();
			// merge changes
			$DNs = array_keys($temp);
			// *** fixme don't include references
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
		/* We have to some string checks now. Not every ldap attributes allow utf8
		 * strings. Therefore we do a syntax check here and change utf8 strings to ascci
		 * strings. Only "7bit" ascci is allowed
		*/
		// TODO how do we interact with the user and show him what has been changed
		// Complete dn with uid or cn=
		$search = $this->rdn;
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
					$attr = array();
					if (is_array($attributes[$this->dn]['add'])) $attr = array_merge_recursive($attr, $attributes[$this->dn]['add']);
					if (is_array($attributes[$this->dn]['notchanged'])) $attr = array_merge_recursive($attr, $attributes[$this->dn]['notchanged']);
					if (is_array($attributes[$this->dn]['modify'])) $attr = array_merge_recursive($attr, $attributes[$this->dn]['modify']);
					// add attributes which are not controlled by modules from original account
					$attrNames = array_keys($this->attributes_orig);
					for ($i = 0; $i < sizeof($attrNames); $i++) {
						if (!isset($attr[$attrNames[$i]])) $attr[$attrNames[$i]] = $this->attributes_orig[$attrNames[$i]];
					}
					$success = ldap_add($_SESSION['ldap']->server(), $this->dn, $attr);
					if ($success) {
						//$_SESSION['cache']->update_cache($this->dn, 'add', $attr); TODO: reactivate
						$success = ldap_delete($_SESSION['ldap']->server(), $this->dn_orig);
						if (!$success) {
							$errors[] = array('ERROR', sprintf(_('Was unable to delete DN: %s.'), $this->dn_orig), ldap_error($_SESSION['ldap']->server()));
							$stopprocessing = true;
							}
						//if ($success) TODO: reactivate
							//$_SESSION['cache']->update_cache($this->dn, 'delete_dn'); TODO: reactivate
						}
					if (!$success) {
						$errors[] = array('ERROR', sprintf(_('Was unable to create DN: %s.'), $this->dn), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
						}
					}
				// create complete new dn
				else {
					$attr = array();
					if (is_array($attributes[$this->dn]['add'])) $attr = array_merge_recursive($attr, $attributes[$this->dn]['add']);
					if (is_array($attributes[$this->dn]['notchanged'])) $attr = array_merge_recursive($attr, $attributes[$this->dn]['notchanged']);
					if (is_array($attributes[$this->dn]['modify'])) $attr = array_merge_recursive($attr, $attributes[$this->dn]['modify']);
					$success = ldap_add($_SESSION['ldap']->server(), $this->dn, $attr);
					if (!$success) {
						$errors[] = array('ERROR', sprintf(_('Was unable to create DN: %s.'), $this->dn), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
						}
					//else TODO: reactivate
						//$_SESSION['cache']->update_cache($this->dn, 'add', $attr); TODO: reactivate
					}
				unset($attributes[$this->dn]);
				}
			}
		$DNs = array_keys($attributes);
		for ($i=0; $i<count($DNs); $i++) {
			if (!$stopprocessing) {
				// modify attributes
				if (isset($attributes[$DNs[$i]]['modify']) && !$stopprocessing) {
					$success = @ldap_mod_replace($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['modify']);
					if (!$success) {
						$errors[] = array('ERROR', sprintf(_('Was unable to modify attribtues from DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
						}
					//else TODO: reactivate
						//$_SESSION['cache']->update_cache($this->dn, 'modify', $attributes[$this->dn]['modify']); TODO: reactivate
					}
				// add attributes
				if (isset($attributes[$DNs[$i]]['add']) && !$stopprocessing) {
					$success = @ldap_mod_add($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['add']);
					if (!$success) {
						$errors[] = array('ERROR', sprintf(_('Was unable to add attribtues to DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
						}
					//else TODO: reactivate
						//$_SESSION['cache']->update_cache($this->dn, 'add', $attributes[$this->dn]['add']); TODO: reactivate
					}
				// removce attributes
				if (isset($attributes[$DNs[$i]]['remove']) && !$stopprocessing) {
					$success = @ldap_mod_del($_SESSION['ldap']->server(), $DNs[$i], $attributes[$DNs[$i]]['remove']);
					if (!$success) {
						$errors[] = array('ERROR', sprintf(_('Was unable to remove attribtues from DN: %s.'), $DNs[$i]), ldap_error($_SESSION['ldap']->server()));
						$stopprocessing = true;
						}
					//else TODO: reactivate
						//$_SESSION['cache']->update_cache($this->dn, 'remove', $attributes[$this->dn]['remove']); TODO: reactivate
					}
				}
			}

		if (!$stopprocessing) {
			foreach ($attributes as $DN) {
				if (is_array($DN['lamdaemon']['command'])) $result = lamdaemon($DN['lamdaemon']['command']);
				// Error somewhere in lamdaemon
				if (is_array($result))
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
		$_SESSION['cache']->refresh_cache(true); // TODO: remove when update_cache is fixed
		if (count($errors)!=0) return $errors;
		return 0;
	}

	/**
	 * Returns a list of possible PDF entries for this account.
	 * 
	 * @return list of PDF entries (array(<name> => <PDF lines>))
	 */
	function get_pdfEntries() {
		$return = array();
		while(($current = current($this->module)) != null) {
			$return = array_merge($return,$current->get_pdfEntries($this->type));
			next($this->module);
		}
		$return = array_merge($return,array('main_dn' => array('<block><key>' . _('DN') . '</key><value>' . $this->dn . '</value></block>')));
		return $return;
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
		return array_keys(get_object_vars(&$this));
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
?>
