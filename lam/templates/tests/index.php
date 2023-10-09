<?php
namespace LAM\TOOLS\TESTS;
use \htmlResponsiveRow;
use \htmlOutputText;
use \htmlLink;
use \htmlTitle;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2021  Roland Gruber

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
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");

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
$container->add(new htmlTitle(_("LAM tests")), 12);

$container->add(new htmlLink(_("Lamdaemon test"), 'lamdaemonTest.php', '../../graphics/script.svg'), 12, 4);
$container->add(new htmlOutputText(_("Check if quotas and homedirectories can be managed.")), 12, 8);

$container->addVerticalSpacer('2rem');

$container->add(new htmlLink(_("Schema test"), 'schemaTest.php', '../../graphics/search-color.svg'), 12, 4);
$container->add(new htmlOutputText(_("Check if the LDAP schema fits the requirements of the selected account modules.")), 12, 8);

parseHtml(null, $container, array(), true, 'user');

echo "</div>\n";
include '../../lib/adminFooter.inc';
