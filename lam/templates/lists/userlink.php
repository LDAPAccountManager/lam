<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more detaexils.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/**
* This page will redirect to account/edit.php if the given user is valid.
*
* It is called from listgroups.php via the memberUID links.
*
* @package lists
* @author Roland Gruber
*/

/** Needed to find DNs of users */
include_once ("../../lib/ldap.inc");
/** Used to display error messages */
include_once ("../../lib/status.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// get user name
$user = $_GET['user'];
$user = str_replace("\\", '',$user);
$user = str_replace("'", '',$user);

// get DN of user
$dn = $_SESSION['ldap']->search_username($user);

if ($dn) {
	// redirect to account/edit.php
	metaRefresh("../account/edit.php?type=user&amp;DN='$dn'");

}
else {
	// print error message if user was not found
	echo $_SESSION['header'];
	echo "<title>userlink</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
	echo "</head><body>\n";
	StatusMessage("ERROR", "", _("This user was not found!") . " (" . $user . ")");
	echo "<p>&nbsp;</p>";
	echo "<p><a href=\"listgroups.php\">" . _("Back to group list") . "</a></p>";
	echo ("</body></html>\n");
}



