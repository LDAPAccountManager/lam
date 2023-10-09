<?php
namespace LAM\TOOLS\TESTS;
use \htmlResponsiveRow;
use \htmlTitle;
use \htmlStatusMessage;
use \htmlSubTitle;
use \htmlOutputText;
use \htmlImage;
use function \LAM\SCHEMA\get_schema_objectclasses;
use function \LAM\SCHEMA\get_cached_schema;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2007 - 2022  Roland Gruber

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
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");
/** account modules */
include_once(__DIR__ . "/../../lib/modules.inc");
/** LDAP schema */
include_once(__DIR__ . "/../../lib/schema.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) {
	die();
}

checkIfToolIsActive('toolTests');

setlanguage();

include '../../lib/adminHeader.inc';
echo "<div class=\"smallPaddingContent\">\n";

$container = new htmlResponsiveRow();

$container->add(new htmlTitle(_("Schema test")), 12);

get_schema_objectclasses();
$classes = get_cached_schema('objectclasses');

if (!is_array($classes)) {
	$container->add(new htmlStatusMessage('ERROR', _('Unable to retrieve schema!'), _('You do not have the required access rights or the LDAP schema is not published by your server.')), 12);
}
else {
	// loop for active account types
	$typeManager = new \LAM\TYPES\TypeManager();
	$types = $typeManager->getConfiguredTypes();
	foreach ($types as $type) {
		$modules = $_SESSION['config']->get_AccountModules($type->getId());
		$container->add(new htmlSubTitle($type->getAlias()), 12);
		for ($m = 0; $m < sizeof($modules); $m++) {
			$error = checkSchemaForModule($modules[$m], $type->getScope(), $type->getId());
			$message = _("No problems found.");
			$icon = '../../graphics/pass.svg';
			if ($error != null) {
				$icon = '../../graphics/del.svg';
				$message = $error;
			}
			// module name
			$container->add(new htmlOutputText(getModuleAlias($modules[$m], $type->getScope())), 10, 3);
			// icon
			$container->add(new htmlImage($icon), 2);
			// text
			$container->add(new htmlOutputText($message), 12, 7);
			$container->addVerticalSpacer('0.5rem');
		}
	}
}

parseHtml(null, $container, array(), true, 'user');

echo "</div>\n";
include '../../lib/adminFooter.inc';

/**
 * Checks if the object classes and attributes for this module are available.
 *
 * @param string $name module name
 * @param string $scope type (user, group, ...)
 * @param string $typeId type id
 * @return string error message or null
 */
function checkSchemaForModule($name, $scope, $typeId): ?string {
	$module = new $name($scope);
	$classes = $module->getManagedObjectClasses($typeId);
	$attrs = $module->getManagedAttributes($typeId);
	$aliases = array_flip($module->getLDAPAliases($typeId));
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
		if (strpos($attrs[$a], 'INFO.') === 0) {
			continue;
		}
		if (!in_array_ignore_case($attrs[$a], $schemaAttrs) && !in_array_ignore_case(str_replace(';binary', '', $attrs[$a]), $schemaAttrs)) {
			if (isset($aliases[$attrs[$a]]) && in_array_ignore_case($aliases[$attrs[$a]], $schemaAttrs)) {
				continue;
			}
			return sprintf(_("The attribute %s is not supported for the object class %s by your LDAP server."), $attrs[$a], implode("/", $classes));
		}
	}
	return null;
}

/**
 * Returns the names of all attributes which are managed by the given object class and its parents.
 *
 * @param ObjectClass $oClass object class
 * @return array<mixed> list of attribute names
 */
function getRecursiveAttributesFromObjectClass($oClass): array {
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
