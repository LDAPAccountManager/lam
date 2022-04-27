<?php
namespace LAM\SCHEMA;
use \htmlResponsiveRow;
use \htmlSpacer;
use \htmlLink;
use \htmlStatusMessage;
use \htmlResponsiveTable;
use \htmlOutputText;
use \htmlGroup;
use \htmlSelect;
use \htmlDiv;
use \htmlSubTitle;
use htmlTitle;

/*

  Copyright (C) 2018 - 2022 Roland Gruber

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
 * Displays the LDAP schema of the server
 *
 * @package tools
 * @author Roland Gruber
 */


/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to LDAP server */
include_once(__DIR__ . "/../../lib/ldap.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");
/** schema functions */
require_once(__DIR__ . "/../../lib/schema.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();

checkIfToolIsActive('toolSchemaBrowser');

setlanguage();

include __DIR__ . '/../../lib/adminHeader.inc';
echo "<div class=\"smallPaddingContent\">\n";

$availableViews = array('objectClass', 'attribute', 'syntax', 'rule');
$selectedView = 'objectClass';
if (!empty($_GET['display']) && in_array($_GET['display'], $availableViews)) {
	$selectedView = $_GET['display'];
}

$tabindex = 1;

$row = new htmlResponsiveRow();
$row->add(new htmlTitle(_('Schema browser')));
$row->addVerticalSpacer('2rem');
$row->add(new htmlSpacer('1rem', '1px'), 0, 2);
$row->add(new htmlLink(_('Object classes'), 'schema.php'), 12, 2, 2, 'font-big text-center');
$row->add(new htmlLink(_('Attribute types'), 'schema.php?display=attribute'), 12, 2, 2, 'font-big text-center');
$row->add(new htmlLink(_('Syntaxes'), 'schema.php?display=syntax'), 12, 2, 2, 'font-big text-center');
$row->add(new htmlLink(_('Matching rules'), 'schema.php?display=rule'), 12, 2, 2, 'font-big text-center');
$row->add(new htmlSpacer('1rem', '1px'), 0, 2);
$row->addVerticalSpacer('2rem');

if ($selectedView === 'syntax') {
	displaySyntaxList($row);
}
elseif( $selectedView == 'attribute' ) {
	displayAttributeList($row);
}
elseif ($selectedView === 'rule') {
	displayRuleList($row);
}
elseif( $selectedView == 'objectClass' ) {
	displayObjectClassList($row);
}

parseHtml(null, $row, array(), false, $tabindex, 'user');

echo '</div>';
include __DIR__ . '/../../lib/adminFooter.inc';

/**
 * Displays the syntax list.
 *
 * @param htmlResponsiveRow $row row
 */
function displaySyntaxList(htmlResponsiveRow &$row) {
	$schema_syntaxes = get_schema_syntaxes(null);
	if (!$schema_syntaxes) {
		$row->add(new htmlStatusMessage("ERROR", _("Unable to retrieve schema!")), 12);
		return;
	}
	$data = array();
	$labels = array(_('Syntax OID'), _('Description'));
	$pos = 0;
	$highlighted = array();
	foreach( $schema_syntaxes as $syntax ) {
		$oid = new htmlOutputText($syntax->getOID());
		$description = new htmlOutputText($syntax->getDescription());
		$data[] = array($oid, $description);
		if (!empty($_GET['sel']) && ($syntax->getOID() === $_GET['sel'])) {
			$highlighted[] = $pos;
		}
		$pos++;
	}
	$table = new htmlResponsiveTable($labels, $data, $highlighted);
	$table->setCSSClasses(array('colored--table'));
	$row->add($table);
}

/**
 * Displays the matching rule list.
 *
 * @param htmlResponsiveRow $row row
 */
function displayRuleList(htmlResponsiveRow &$row) {
    $rules = get_schema_matching_rules(null);
	if (!$rules) {
		$row->add(new htmlStatusMessage("ERROR", _("Unable to retrieve schema!")), 12);
		return;
	}
    $row->addLabel(new htmlOutputText(_('Jump to a matching rule')));
    $availableRules = array('');
    foreach ($rules as $rule) {
		$availableRules[] = $rule->getName();
    }
    $selectedRule = array();
    if (!empty($_GET['sel']) && in_array($_GET['sel'], $availableRules)) {
    	$selectedRule[] = $_GET['sel'];
    }
    $ruleSelect = new htmlSelect('lam-schema-select', $availableRules, $selectedRule);
    $ruleSelect->addDataAttribute('display', 'rule');
	$row->addField($ruleSelect);
	$row->addVerticalSpacer('1rem');

	$labels = array(_('Matching rule OID'), _('Name'), _('Used by attributes'));
	$data = array();
	foreach ($rules as $rule) {
		if (!empty($selectedRule) && !in_array($rule->getName(), $selectedRule)) {
			continue;
		}
		$oid = new htmlOutputText($rule->getOID());
		$name = $rule->getName();
		if (!empty($rule->getDescription())) {
			$name .= ' (' . $rule->getDescription() . ')';
		}
		if ($rule->getIsObsolete()) {
			$name .= ' (' . _('Obsolete') . ')';
		}
		$nameText = new htmlOutputText($name);
		$attributes = new htmlGroup();
		foreach ($rule->getUsedByAttrs() as $attr) {
			$attributes->addElement(new htmlDiv(null, new htmlLink($attr, 'schema.php?display=attribute&sel=' . $attr)));
		}
		$data[] = array($oid, $nameText, new htmlDiv(null, $attributes, array('smallScroll')));
	}
	$table = new htmlResponsiveTable($labels, $data);
	$table->setCSSClasses(array('colored--table'));
	$row->add($table);
}

/**
 * Displays the object class list.
 *
 * @param htmlResponsiveRow $row row
 */
function displayObjectClassList(htmlResponsiveRow &$row) {
	$objectClasses = get_schema_objectclasses(null);
	if (!$objectClasses) {
		$row->add(new htmlStatusMessage("ERROR", _("Unable to retrieve schema!")), 12);
		return;
	}
    $row->addLabel(new htmlOutputText(_('Jump to an object class')));
    $availableClasses = array(_('all') => '');
    foreach ($objectClasses as $objectClass) {
		$availableClasses[$objectClass->getName()] = $objectClass->getName();
    }
    $selectedClass = array();
    if (isset($_GET['sel']) && (empty($_GET['sel']) || array_key_exists(strtolower($_GET['sel']), $objectClasses))) {
    	$selectedClass[0] = $_GET['sel'];
    }
    if (empty($selectedClass) && (sizeof($objectClasses) > 0)) {
    	// select first class by default
    	$selectedClassNames = array_keys($objectClasses);
    	$selectedClass[0] = $selectedClassNames[0];
    }
    $classSelect = new htmlSelect('lam-schema-select', $availableClasses, $selectedClass);
    $classSelect->addDataAttribute('display', 'objectClass');
    $classSelect->setHasDescriptiveElements(true);
    $classSelect->setSortElements(false);
	$row->addField($classSelect);
	$row->addVerticalSpacer('1rem');

	// fill child object classes
	foreach ($objectClasses as $name => $objectClass) {
		if (!empty($objectClass->getSupClasses())) {
			foreach ($objectClass->getSupClasses() as $subClass) {
				if (!isset($objectClasses[strtolower($subClass)])) {
					continue;
				}
				$objectClasses[strtolower($subClass)]->addChildObjectClass($name);
			}
		}
	}

	foreach ($objectClasses as $name => $objectClass) {
		if (!empty($selectedClass[0]) && ($name !== strtolower($selectedClass[0]))) {
			continue;
		}
		$row->add(new htmlSubTitle($name), 12);
		$row->addLabel(new htmlOutputText(_('OID')), 'bold-mobile-only');
		$row->addField(new htmlOutputText($objectClass->getOID()));
		if (!empty($objectClass->getDescription())) {
			$row->addLabel(new htmlOutputText(_('Description')), 'bold-mobile-only');
			$row->addField(new htmlOutputText($objectClass->getDescription()));
		}
		$row->addLabel(new htmlOutputText(_('Type')), 'bold-mobile-only');
		$row->addField(new htmlOutputText($objectClass->getType()));
		if ($objectClass->getIsObsolete()) {
			$row->addLabel(new htmlOutputText(_('Obsolete')));
			$row->addField(new htmlOutputText(_('yes')));
		}
		if (!empty($objectClass->getSupClasses())) {
			$row->addLabel(new htmlOutputText(_('Inherits from')), 'bold-mobile-only');
			$subClasses = new htmlGroup();
			foreach ($objectClass->getSupClasses() as $subClass) {
				$subClasses->addElement(new htmlDiv(null, new htmlLink($subClass, 'schema.php?display=objectClass&sel=' . rawurlencode($subClass))));
			}
			$row->addField($subClasses);
		}
		if (!empty($objectClass->getChildObjectClasses())) {
			$row->addLabel(new htmlOutputText(_('Parent to')), 'bold-mobile-only');
			$subClasses = new htmlGroup();
			foreach ($objectClass->getChildObjectClasses() as $subClass) {
				$subClasses->addElement(new htmlDiv(null, new htmlLink($subClass, 'schema.php?display=objectClass&sel=' . rawurlencode($subClass))));
			}
			$row->addField($subClasses);
		}
		if (!empty($objectClass->getMustAttrs())) {
			$row->addLabel(new htmlOutputText(_('Required attributes')), 'bold-mobile-only');
			$attributes = new htmlGroup();
			foreach ($objectClass->getMustAttrs() as $attribute) {
				$attributes->addElement(new htmlDiv(null, new htmlLink($attribute->getName(), 'schema.php?display=attribute&sel=' . rawurlencode($attribute->getName()))));
			}
			$row->addField($attributes);
		}
		if (!empty($objectClass->getMayAttrs())) {
			$row->addLabel(new htmlOutputText(_('Optional attributes')), 'bold-mobile-only');
			$attributes = new htmlGroup();
			foreach ($objectClass->getMayAttrs() as $attribute) {
				$attributes->addElement(new htmlDiv(null, new htmlLink($attribute->getName(), 'schema.php?display=attribute&sel=' . rawurlencode($attribute->getName()))));
			}
			$row->addField($attributes);
		}
	}
}

/**
 * Displays the attributes list.
 *
 * @param htmlResponsiveRow $row row
 */
function displayAttributeList(htmlResponsiveRow $row) {
	$attributes = get_schema_attributes(null);
	if (!$attributes) {
		$row->add(new htmlStatusMessage("ERROR", _("Unable to retrieve schema!")), 12);
		return;
	}
	$row->addLabel(new htmlOutputText(_('Jump to an attribute type')));
	$availableAttributes = array(_('all') => '');
	foreach ($attributes as $attribute) {
		$availableAttributes[$attribute->getName()] = $attribute->getName();
	}
	$selectedAttribute = array();
	if (isset($_GET['sel']) && (empty($_GET['sel']) || array_key_exists(strtolower($_GET['sel']), $attributes))) {
		$selectedAttribute[0] = $_GET['sel'];
	}
	if (empty($selectedAttribute) && (sizeof($availableAttributes) > 1)) {
		// select first attribute by default
		$attributeNames = array_keys($availableAttributes);
		$selectedAttribute[0] = $attributeNames[1];
	}
	$attributeSelect = new htmlSelect('lam-schema-select', $availableAttributes, $selectedAttribute);
	$attributeSelect->addDataAttribute('display', 'attribute');
	$attributeSelect->setHasDescriptiveElements(true);
	$attributeSelect->setSortElements(false);
	$row->addField($attributeSelect);
	$row->addVerticalSpacer('1rem');

	foreach ($attributes as $name => $attribute) {
		if (!empty($selectedAttribute[0]) && ($name !== strtolower($selectedAttribute[0]))) {
			continue;
		}
		$row->add(new htmlSubTitle($name), 12);
		if (!empty($attribute->getDescription())) {
			$row->addLabel(new htmlOutputText(_('Description')), 'bold-mobile-only');
			$row->addField(new htmlOutputText($attribute->getDescription()));
		}
		$row->addLabel(new htmlOutputText(_('OID')), 'bold-mobile-only');
		$row->addField(new htmlOutputText($attribute->getOID()));
		if ($attribute->getIsObsolete()) {
			$row->addLabel(new htmlOutputText(_('Obsolete')), 'bold-mobile-only');
			$row->addField(new htmlOutputText(_('yes')));
		}
		if (!empty($attribute->getSupAttribute())) {
			$row->addLabel(new htmlOutputText(_('Inherits from')), 'bold-mobile-only');
			$row->addField(new htmlLink($attribute->getSupAttribute(), 'schema.php?display=attribute&sel=' . rawurlencode($attribute->getSupAttribute())));
		}
		if (!empty($attribute->getEquality())) {
			$row->addLabel(new htmlOutputText(_('Equality')), 'bold-mobile-only');
			$row->addField(new htmlLink($attribute->getEquality(), 'schema.php?display=rule&sel=' . rawurldecode($attribute->getEquality())));
		}
		if (!empty($attribute->getOrdering())) {
			$row->addLabel(new htmlOutputText(_('Ordering')), 'bold-mobile-only');
			$row->addField(new htmlLink($attribute->getOrdering(), 'schema.php?display=rule&sel=' . rawurldecode($attribute->getOrdering())));
		}
		if (!empty($attribute->getSubstr())) {
			$row->addLabel(new htmlOutputText(_('Substring Rule')), 'bold-mobile-only');
			$row->addField(new htmlLink($attribute->getSubstr(), 'schema.php?display=rule&sel=' . rawurldecode($attribute->getSubstr())));
		}
		if (!empty($attribute->getSyntaxOID())) {
			$row->addLabel(new htmlOutputText(_('Syntax')), 'bold-mobile-only');
			$row->addField(new htmlLink($attribute->getSyntaxOID(), 'schema.php?display=syntax&sel=' . rawurldecode($attribute->getSyntaxOID())));
		}
		$row->addLabel(new htmlOutputText(_('Single valued')), 'bold-mobile-only');
		$row->addField(new htmlOutputText($attribute->getIsSingleValue() ? _('yes') : _('no')));
		$row->addLabel(new htmlOutputText(_('Collective')), 'bold-mobile-only');
		$row->addField(new htmlOutputText($attribute->getIsCollective() ? _('yes') : _('no')));
		$row->addLabel(new htmlOutputText(_('User modification')), 'bold-mobile-only');
		$row->addField(new htmlOutputText($attribute->getIsNoUserModification() ? _('no') : _('yes')));
		if (!empty($attribute->getUsage())) {
			$row->addLabel(new htmlOutputText(_('Usage')), 'bold-mobile-only');
			$row->addField(new htmlOutputText($attribute->getUsage()));
		}
		if (!empty($attribute->getMaxLength())) {
			$row->addLabel(new htmlOutputText(_('Maximum length')), 'bold-mobile-only');
			$row->addField(new htmlOutputText($attribute->getMaxLength()));
		}
		if (!empty($attribute->getAliases())) {
			$row->addLabel(new htmlOutputText(_('Aliases')), 'bold-mobile-only');
			$aliases = new htmlGroup();
			foreach ($attribute->getAliases() as $alias) {
				$aliases->addElement(new htmlDiv(null, new htmlLink($alias, 'schema.php?display=attribute&sel=' . rawurlencode($alias))));
			}
			$row->addField($aliases);
		}
		if (!empty($attribute->getUsedInObjectClasses())) {
			$row->addLabel(new htmlOutputText(_('Used by object classes')), 'bold-mobile-only');
			$objectClasses = new htmlGroup();
			foreach ($attribute->getUsedInObjectClasses() as $objectClass) {
				$objectClasses->addElement(new htmlDiv(null, new htmlLink($objectClass, 'schema.php?display=objectClass&sel=' . rawurlencode($objectClass))));
			}
			$row->addField($objectClasses);
		}

	}
}

?>
