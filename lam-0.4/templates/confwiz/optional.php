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
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


  Configuration wizard - optional pages
*/

include_once('../../lib/config.inc');
include_once('../../lib/ldap.inc');
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check master password
$cfg = new CfgMain();
if ($cfg->password != $_SESSION['confwiz_masterpwd']) {
	require("../config/conflogin.php");
	exit;
}

// if no optional pages should be displayed go to ldaptest
if (sizeof($_SESSION['confwiz_optional']) < 1) {
	metarefresh('ldaptest.php');
	exit;
}

// UID/GID ranges
if ($_SESSION['confwiz_optional']['ranges'] == 'yes') {
	metarefresh('o_ranges.php');
	exit;
}

// list attributes
if ($_SESSION['confwiz_optional']['lists'] == 'yes') {
	metarefresh('o_lists.php');
	exit;
}

// language, admins
if ($_SESSION['confwiz_optional']['lang'] == 'yes') {
	metarefresh('o_lang.php');
	exit;
}

// lamdaemon and PDF text
if ($_SESSION['confwiz_optional']['daemon'] == 'yes') {
	metarefresh('o_daemon.php');
	exit;
}

// if all pages were displayed go to ldaptest
metarefresh('ldaptest.php');

?>




