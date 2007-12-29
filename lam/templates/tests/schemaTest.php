<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2007  Roland Gruber

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
* Tests the lamdaemon script.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../../lib/security.inc");
/** access to configuration options */
include_once("../../lib/config.inc");
/** account modules */
include_once("../../lib/modules.inc");
/** LDAP schema */
include_once("../../lib/schema.inc");

// start session
startSecureSession();

setlanguage();

echo $_SESSION['header'];


echo "<title></title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
$types = $_SESSION['config']->get_ActiveTypes();
for ($t = 0; $t < sizeof($types); $t++) {
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/type_" . $types[$t] . ".css\">\n";
}
echo "</head>";

echo "<body>\n";

echo "<h1 align=\"center\">" . _("Schema test") . "</h1>\n";

get_schema_objectclasses();
$classes = get_cached_schema('objectclasses');

if (!is_array($classes)) {
	StatusMessage('ERROR', _('Unable to retrieve schema!'), _('You do not have the required access rights or the LDAP schema is not published by your server.'));
	echo "</body></html>\n";
	die();
}

// loop for active account types
for ($t = 0; $t < sizeof($types); $t++) {
	$modules = $_SESSION['config']->get_AccountModules($types[$t]);
	echo "<h2>" . getTypeAlias($types[$t]) . "</h2>\n";
	echo "<table width=\"100%\" class=\"" . $types[$t] . "list\">\n";

	for ($m = 0; $m < sizeof($modules); $m++) {
		$error = checkSchemaForModule($modules[$m], $types[$t]);
		$message = _("No problems found.");
		$icon = '../../graphics/pass.png';
		if ($error != null) {
			$icon = '../../graphics/fail.png';
			$message = $error;
		}
		// module name
		echo "<tr class=\"" . $types[$t] . "list\">\n";
		echo "<td style=\"padding-left:10px;\" nowrap>" . getModuleAlias($modules[$m], $types[$t]) . "</td>\n";
		// icon
		echo "<td style=\"padding-left:10px;padding-right:10px;\"><img alt=\"\" src=\"" . $icon . "\"></td>\n";
		// text
		echo "<td width=\"100%\">" . $message . "</td>\n";
		echo "</tr>\n";
	}
	
	echo "</table>\n<br>";
}

echo "</body>\n";
echo "</html>\n";

/**
 * Checks if the object classes and attributes for this module are available.
 *
 * @param String $name module name
 * @param String $type type (user, group, ...)
 * @return String error message or null
 */
function checkSchemaForModule($name, $type) {
	$module = new $name($type);
	$classes = $module->getManagedObjectClasses();
	$attrs = $module->getManagedAttributes();
	$aliases = array_flip($module->getLDAPAliases());
	if (sizeof($classes) == 0) {
		return null;
	}
	$schemaClasses = get_cached_schema('objectclasses');
	$schemaAttrs = array();
	// check if object classes are supported
	for ($o = 0; $o < sizeof($classes); $o++) {
		if (!isset($schemaClasses[strtolower($classes[$o])])) {
			return sprintf(_("The object class %s is not supported by your LDAP server."), $classes[$o]);
		}
		// get attribute names
		$schemaAttrs = array_merge($schemaAttrs, getRecursiveAttributesFromObjectClass($schemaClasses[strtolower($classes[$o])]));
	}
	// check if attributes are supported
	for ($a = 0; $a < sizeof($attrs); $a++) {
		if (!in_array_ignore_case($attrs[$a], $schemaAttrs)) {
			if (isset($aliases[$attrs[$a]]) && in_array_ignore_case($aliases[$attrs[$a]], $schemaAttrs)) {
				continue;
			}
			return sprintf(_("The attribute %s is not supported for the object class(es) %s by your LDAP server."), $attrs[$a], implode(", ", $classes));
		}
	}
	return null;
}

/**
 * Returns the names of all attributes which are managed by the given object class and its parents.
 *
 * @param ObjectClass $oClass object class
 * @return array list of attribute names
 */
function getRecursiveAttributesFromObjectClass($oClass) {
	$attrs = array();
	$attrs = array_merge($attrs, $oClass->getMustAttrNames());
	$attrs = array_merge($attrs, $oClass->getMayAttrNames());
	$subClassNames = $oClass->getSupClasses();
	for ($i = 0; $i < sizeof($subClassNames); $i++) {
		$schemaClasses = get_cached_schema('objectclasses');
		$subClass = $schemaClasses[strtolower($subClassNames[$i])];
		$attrs = array_merge($attrs, getRecursiveAttributesFromObjectClass($subClass));
	}
	return $attrs;
}

?>
