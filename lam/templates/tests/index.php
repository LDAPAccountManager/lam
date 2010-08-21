<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2006 - 2010  Roland Gruber

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
include_once("../../lib/security.inc");
/** access to configuration options */
include_once("../../lib/config.inc");

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

include '../main_header.php';

echo "<h1>" . _("LAM tests") . "</h1>\n";

echo "<table class=\"userlist\" rules=\"none\">\n";

echo "<tr class=\"userlist-bright\"><td style=\"padding:10px;\"><a href=\"lamdaemonTest.php\"><b>" . _("Lamdaemon test") . "</b>&nbsp;&nbsp;</a></td>";
echo "<td style=\"padding:10px;\">" . _("Check if quotas and homedirectories can be managed.") . "</td></tr>";

echo "<tr class=\"userlist-bright\"><td style=\"padding:10px;\"><a href=\"schemaTest.php\"><b>" . _("Schema test") . "</b>&nbsp;&nbsp;</a></td>";
echo "<td style=\"padding:10px;\">" . _("Check if the LDAP schema fits the requirements of the selected account modules.") . "</td></tr>";

echo "</table>\n";


include '../main_footer.php';

?>
