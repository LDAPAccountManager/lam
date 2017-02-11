<?php
namespace LAM\INIT;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2017  Roland Gruber

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
* This page redirects to the correct start page after login.
*
* @package main
* @author Roland Gruber
*/

/** config object */
include_once '../lib/config.inc';
/** profiles */
include_once '../lib/profiles.inc';

// start session
startSecureSession();
enforceUserIsLoggedIn();

setlanguage();

\LAM\PROFILES\installProfileTemplates();
\LAM\PDF\installPDFTemplates();

// check if all suffixes in conf-file exist
$conf = $_SESSION['config'];
$new_suffs = array();
// get list of active types
$typeManager = new \LAM\TYPES\TypeManager();
$types = $typeManager->getConfiguredTypes();
foreach ($types as $type) {
	$info = @ldap_read($_SESSION['ldap']->server(), escapeDN($type->getSuffix()), "(objectClass=*)", array('objectClass'), 0, 0, 0, LDAP_DEREF_NEVER);
	$res = @ldap_get_entries($_SESSION['ldap']->server(), $info);
	if (!$res && !in_array($type->getSuffix(), $new_suffs)) {
		$new_suffs[] = $type->getSuffix();
	}
}

// display page to add suffixes, if needed
if ((sizeof($new_suffs) > 0) && checkIfWriteAccessIsAllowed()) {
	metaRefresh("initsuff.php?suffs='" . implode(";", $new_suffs));
}
else {
	if (sizeof($types) > 0) {
		foreach ($types as $type) {
			if ($type->isHidden()) {
				continue;
			}
			metaRefresh("lists/list.php?type=" . $type->getId());
			break;
		}
	}
	else {
		metaRefresh("tree/treeViewContainer.php");
	}
}

?>
