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

include_once('../../lib/include.inc');

if ($_GET['DN']) {
	//load account
	if ($_GET['DN'] == $DN) $DN = str_replace("'", '',$_GET['DN']);
	$DN = str_replace("\'", '', $_GET['DN']);
	$type = str_replace("\'", '', $_GET['type']);
	if ($_GET['type'] == $type) $type = str_replace("'", '',$_GET['type']);
	$_SESSION['account'] = new accountContainer($type, 'account');
	$_SESSION['account']->load_account($DN);
	}
else if (count($_POST)==0) {
	$type = str_replace("\'", '', $_GET['type']);
	if ($_GET['type'] == $type) $type = str_replace("'", '',$_GET['type']);
	$_SESSION['account'] = new accountContainer($type, 'account');
	$_SESSION['account']->new_account();
	}
$_SESSION['account']->continue_main($_POST);

?>
