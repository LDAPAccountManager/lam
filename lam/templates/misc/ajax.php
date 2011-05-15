<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2011  Roland Gruber

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
* Manages all AJAX requests.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../../lib/security.inc");

// start session
startSecureSession();

setlanguage();

lamAjax::handleRequest();

/**
 * Manages all AJAX requests.
 */
class lamAjax {
	
	/**
	 * Manages an AJAX request.
	 */
	public static function handleRequest() {
		if (!isset($_GET['function'])) {
			die();
		}
		$function = $_GET['function'];
		if (!isset($_POST['jsonInput'])) {
			die();
		}
		$jsonInput = $_POST['jsonInput'];
		
		if ($function == 'passwordChange') {
			lamAjax::managePasswordChange($jsonInput);
		}
	}

	/**
	 * Manages a password change request on the edit account page.
	 *
	 * @param array $input input parameters
	 */
	public static function managePasswordChange($input) {
		$return = $_SESSION['account']->setNewPassword($input);
		echo json_encode($return);
	}
	
}


?>
