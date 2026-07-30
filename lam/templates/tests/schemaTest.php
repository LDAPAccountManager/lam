<?php
namespace LAM\TOOLS\TESTS;
use htmlResponsiveRow;
use htmlTitle;
use htmlStatusMessage;
use htmlSubTitle;
use htmlOutputText;
use htmlImage;
use LAM\SCHEMA\ObjectClass;
use function LAM\SCHEMA\get_schema_objectclasses;
use function LAM\SCHEMA\get_cached_schema;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2007 - 2026  Roland Gruber

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
include_once __DIR__ . "/../../lib/security.inc";
/** access to configuration options */
include_once __DIR__ . "/../../lib/config.inc";
/** account modules */
include_once __DIR__ . "/../../lib/modules.inc";
/** LDAP schema */
include_once __DIR__ . "/../../lib/schema.inc";

// start session
startSecureSession();
enforceUserIsLoggedIn();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) {
	die();
}

checkIfToolIsActive('toolTests');

setlanguage();

include_once __DIR__ . '/../../lib/adminHeader.inc';
echo "<div class=\"smallPaddingContent\">\n";

$container = new htmlResponsiveRow();

$container->add(new htmlTitle(_("Schema test")));

get_schema_objectclasses();
$classes = get_cached_schema('objectclasses');

if (!is_array($classes)) {
	$container->add(new htmlStatusMessage('ERROR', _('Unable to retrieve schema!'), _('You do not have the required access rights or the LDAP schema is not published by your server.')));
}
else {
	// loop for active account types
	$typeManager = new \LAM\TYPES\TypeManager();
	$types = $typeManager->getConfiguredTypes();
	foreach ($types as $type) {
		$moduleNames = $_SESSION['config']->get_AccountModules($type->getId());
		$container->add(new htmlSubTitle($type->getAlias()));
		foreach ($moduleNames as $moduleName) {
			$error = checkSchemaForModule($moduleName, $type->getScope(), $type->getId());
			$message = _("No problems found.");
			$icon = '../../graphics/pass.svg';
			if ($error != null) {
				$icon = '../../graphics/del.svg';
				$message = $error;
			}
			// module name
			$aliasName = getModuleAlias($moduleName, $type->getScope()) ?? '';
			$container->add(new htmlOutputText($aliasName), 10, 3);
			// icon
			$container->add(new htmlImage($icon), 2);
			// text
			$container->add(new htmlOutputText($message), 12, 7);
			$container->addVerticalSpacer('0.5rem');
		}
	}
}

parseHtml(null, $container, [], true);

echo "</div>\n";
include_once __DIR__ . '/../../lib/adminFooter.inc';

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
	if (count($classes) === 0) {
		return null;
	}
	$schemaClasses = get_cached_schema('objectclasses');
	$schemaAttrs = [];
	// check if object classes are supported
	foreach ($classes as $objectClass) {
		if (!isset($schemaClasses[strtolower($objectClass)]) || !($schemaClasses[strtolower($objectClass)] instanceof ObjectClass)) {
			return sprintf(_("The object class %s is not supported by your LDAP server."), $objectClass);
		}
		// get attribute names
		$schemaAttrs = array_merge($schemaAttrs, getRecursiveAttributesFromObjectClass($schemaClasses[strtolower($objectClass)]));
	}
	// check if attributes are supported
	foreach ($attrs as $attributeName) {
		if (str_starts_with($attributeName, 'INFO.')) {
			continue;
		}
		if (!in_array_ignore_case($attributeName, $schemaAttrs) && !in_array_ignore_case(str_replace(';binary', '', $attributeName), $schemaAttrs)) {
			if (isset($aliases[$attributeName]) && in_array_ignore_case($aliases[$attributeName], $schemaAttrs)) {
				continue;
			}
			return sprintf(_("The attribute %s is not supported for the object class %s by your LDAP server."), $attributeName, implode("/", $classes));
		}
	}
	return null;
}

/**
 * Returns the names of all attributes which are managed by the given object class and its parents.
 *
 * @param ObjectClass $oClass object class
 * @return string[] list of attribute names
 */
function getRecursiveAttributesFromObjectClass($oClass): array {
	$attrs = [];
	$attrs = array_merge($attrs, $oClass->getMustAttrNames());
	$attrs = array_merge($attrs, $oClass->getMayAttrNames());
	$subClassNames = $oClass->getSupClasses();
	foreach ($subClassNames as $subClassName) {
		$schemaClasses = get_cached_schema('objectclasses');
		$subClass = $schemaClasses[strtolower($subClassName)];
		if ($subClass instanceof ObjectClass) {
			$attrs = array_merge($attrs, getRecursiveAttributesFromObjectClass($subClass));
		}
	}
	return $attrs;
}
