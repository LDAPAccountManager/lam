<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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


  LDAP Account Manager displays table for creating or modifying accounts in LDAP
*/

// Get Lampath, needed 4 includes
$lampath_include = str_replace('account', '', str_replace('templates', '', realpath('.')));  // Save full path to lam in session
$lampath_include = ereg_replace ( '[/]+', '/', $lampath_include);

include_once($lampath_include.'lib/config.inc'); // File with configure-functions
include_once($lampath_include.'lib/profiles.inc'); // functions to load and save profiles
include_once($lampath_include.'lib/status.inc'); // Return error-message
include_once($lampath_include.'lib/pdf.inc'); // Return a pdf-file
include_once($lampath_include.'lib/ldap.inc'); // LDAP-functions
include_once($lampath_include.'lib/modules.inc'); // LDAP-functions
include_once($lampath_include.'lib/cache.inc');
include_once($lampath_include.'lib/account.inc');

/* We have to include all modules
* before start session
*/
$dir = opendir($lampath_include.'lib/modules');
while ($entry = readdir($dir))
	if (is_file($lampath_include.'lib/modules/'.$entry)) include_once ($lampath_include.'lib/modules/'.$entry);
	
// Start session
session_save_path($lampath_include.'sess');
@session_start();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	// *** fixme ../../
	metaRefresh("../../login.php");
	exit;
	}

// Set correct language, codepages, ....
setlanguage();

if (!isset($_SESSION['cache'])) {
	$_SESSION['cache'] = new cache();
	}

$_SESSION['scope'] = array ('user', 'group', 'host', 'domain');

?>
