<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2010  Roland Gruber

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
* Provides a list of LAM tests.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration options */
include_once("../lib/config.inc");

// start session
startSecureSession();

setlanguage();

$namingContexts = '';
$configcontext = '';
$supportedldapversion = '';
$supportedsaslmechanisms = '';
$subschemasubentry = '';
$vendorname = '';
$vendorversion = '';
$dynamicSubtrees = '';

$result = @ldap_read($_SESSION['ldap']->server(), '', 'objectclass=*', array('+', '*', 'subschemasubentry'), 0, 0, 0, LDAP_DEREF_NEVER);
if ($result) {
	$info = @ldap_get_entries($_SESSION['ldap']->server(), $result);
	if ($info) {
		$info = $info[0];
		foreach ($info as $key => $value) {
			if (is_array($info[$key]) && isset($info[$key]['count'])) {
				unset($info[$key]['count']);
			}
		}
		if (isset($info['namingcontexts'])) {
			$namingContexts = implode(', ', $info['namingcontexts']);
		}
		if (isset($info['configcontext'])) {
			$configcontext = $info['configcontext'][0];
		}
		if (isset($info['supportedldapversion'])) {
			$supportedldapversion = implode(', ', $info['supportedldapversion']);
		}
		if (isset($info['supportedsaslmechanisms'])) {
			$supportedsaslmechanisms = implode(', ', $info['supportedsaslmechanisms']);
		}
		if (isset($info['subschemasubentry'])) {
			$subschemasubentry = $info['subschemasubentry'][0];
		}
		if (isset($info['vendorname'])) {
			$vendorname = $info['vendorname'][0];
		}
		if (isset($info['vendorversion'])) {
			$vendorversion = $info['vendorversion'][0];
		}
		if (isset($info['dynamicsubtrees'])) {
			$dynamicSubtrees = implode(', ', $info['dynamicsubtrees']);
		}
	}
}

include 'main_header.php';
echo '<div class="userlist-bright smallPaddingContent">';
$tabindex = 1;
$container = new htmlTable();

$container->addElement(new htmlTitle(_("Server information")), true);

$container->addElement(new htmlOutputText('<b>' . _("Managed suffixes") . '</b>', false));
$container->addElement(new htmlSpacer('20px', null));
$container->addElement(new htmlOutputText($namingContexts), true);

$container->addElement(new htmlOutputText('<b>' . _("LDAP version") . '</b>', false));
$container->addElement(new htmlSpacer('20px', null));
$container->addElement(new htmlOutputText($supportedldapversion), true);

if ($configcontext != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Config suffix") . '</b>', false));
	$container->addElement(new htmlSpacer('20px', null));
	$container->addElement(new htmlOutputText($configcontext), true);
}

$container->addElement(new htmlOutputText('<b>' . _("Schema suffix") . '</b>', false));
$container->addElement(new htmlSpacer('20px', null));
$container->addElement(new htmlOutputText($subschemasubentry), true);

if ($dynamicSubtrees != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Dynamic subtrees") . '</b>', false));
	$container->addElement(new htmlSpacer('20px', null));
	$container->addElement(new htmlOutputText($dynamicSubtrees), true);
}

$container->addElement(new htmlOutputText('<b>' . _("SASL mechanisms") . '</b>', false));
$container->addElement(new htmlSpacer('20px', null));
$container->addElement(new htmlOutputText($supportedsaslmechanisms), true);

if ($vendorname != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Vendor name") . '</b>', false));
	$container->addElement(new htmlSpacer('20px', null));
	$container->addElement(new htmlOutputText($vendorname), true);
}

if ($vendorversion != '') {
	$container->addElement(new htmlOutputText('<b>' . _("Vendor version") . '</b>', false));
	$container->addElement(new htmlSpacer('20px', null));
	$container->addElement(new htmlOutputText($vendorversion), true);
}

parseHtml(null, $container, array(), true, $tabindex, 'user');

echo '</div>';
include 'main_footer.php';

?>
