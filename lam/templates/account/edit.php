<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003 - 2006  Tilo Lutz

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

/**
* Displays the account detail page.
*
* @package modules
* @author Tilo Lutz
*/

/** security functions */
include_once("../../lib/security.inc");
/** configuration options */
include_once('../../lib/config.inc');
/** functions to load and save profiles */
include_once('../../lib/profiles.inc');
/** Return error-message */
include_once('../../lib/status.inc');
/** Return a pdf-file */
include_once('../../lib/pdf.inc');
/** module functions */
include_once('../../lib/modules.inc');

// Start session
startSecureSession();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn'])) {
	metaRefresh("../login.php");
	exit;
	}

// Set correct language, codepages, ....
setlanguage();

//load account
if (isset($_GET['DN'])) {
	$DN = str_replace("\'", '', $_GET['DN']);
	$type = str_replace("\'", '', $_GET['type']);
	if ($_GET['type'] == $type) $type = str_replace("'", '',$_GET['type']);
	if ($_GET['DN'] == $DN) $DN = str_replace("'", '',$_GET['DN']);
	$_SESSION['account'] = new accountContainer($type, 'account');
	$_SESSION['account']->load_account($DN);
}
// new account
else if (count($_POST)==0) {
	$type = str_replace("\'", '', $_GET['type']);
	if ($_GET['type'] == $type) $type = str_replace("'", '',$_GET['type']);
	$_SESSION['account'] = new accountContainer($type, 'account');
	$_SESSION['account']->new_account();
}

// remove double slashes if magic quotes are on
if (get_magic_quotes_gpc() == 1) {
	$postKeys = array_keys($_POST);
	for ($i = 0; $i < sizeof($postKeys); $i++) {
		if (is_string($_POST[$postKeys[$i]])) $_POST[$postKeys[$i]] = stripslashes($_POST[$postKeys[$i]]);
	}
}

// show account page
$_SESSION['account']->continue_main();

?>
