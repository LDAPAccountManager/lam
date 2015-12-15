<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2012  Roland Gruber

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
* This file shows the list views.
*
* @package lists
* @author Roland Gruber
*/

/** security functions */
include_once("../../lib/security.inc");
/** Used to get type information. */
include_once("../../lib/types.inc");
/** Access to configuration options */
include_once("../../lib/config.inc");

// start session
startSecureSession();

setlanguage();

$type = $_GET['type'];

// check if list is hidden
if (isAccountTypeHidden($type)) {
	logNewMessage(LOG_ERR, 'User tried to access hidden account list: ' . $type);
	die();
}

// create list object if needed
$listClass = getListClassName($type);
if (!isset($_SESSION['list_' . $type])) {
	$list = new $listClass($type);
	$_SESSION['list_' . $type] = $list;
}

// show page
$_SESSION['list_' . $type]->showPage();

?>