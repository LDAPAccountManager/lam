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

// include all needed files
include_once('../../lib/account.inc'); // File with all account-funtions
include_once('../../lib/config.inc'); // File with configure-functions
include_once('../../lib/profiles.inc'); // functions to load and save profiles
include_once('../../lib/status.inc'); // Return error-message
include_once('../../lib/pdf.inc'); // Return a pdf-file
include_once('../../lib/ldap.inc'); // LDAP-functions

/* We have to include all modules
* before start session
* *** fixme I would prefer loading them dynamic but
* i don't know how to to this
*/
$dir = opendir('../../lib/modules');
while ($entry = readdir($dir))
	if (is_file('../../lib/modules/'.$entry)) include_once ('../../lib/modules/'.$entry);

// Start session
session_save_path('../../sess');
@session_start();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	metaRefresh("../login.php");
	exit;
	}

// Set correct language, codepages, ....
setlanguage();

if (!isset($_SESSION['cache'])) {
	$_SESSION['cache'] = new cache();
	}
if ($_GET['DN']) {
	//load account
	$DN = str_replace("\'", '', $_GET['DN']);
	$_SESSION['account'] = new accountContainer('user', 'account');
	$_SESSION['account']->load_account($DN);
	}
else if (count($_POST)==0) {
	$_SESSION['account'] = new accountContainer('user', 'account');
	$_SESSION['account']->new_account();
	}
$_SESSION['account']->continue_main($_POST);

?>
