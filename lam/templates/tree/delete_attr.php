<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  
  This code is based on phpLDAPadmin.
  Copyright (C) 2004  David Smith and phpLDAPadmin developers
  
  The original code was modified to fit for LDAP Account Manager by Roland Gruber.
  Copyright (C) 2005  Roland Gruber

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
 *  Deletes an attribute from an entry with NO confirmation.
 *
 *  On success, redirect to edit.php
 *  On failure, echo an error.
 *
 * @package lists
 * @subpackage tree
 * @author David Smith
 * @author Roland Gruber
 */

/** tree functions */
include_once('../../lib/tree.inc');
/** access to configuration */
include_once('../../lib/config.inc');
/** LDAP functions */
include_once('../../lib/ldap.inc');
/** status messages */
include_once('../../lib/status.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$dn = $_POST['dn'] ;
$encoded_dn = rawurlencode( $dn );
$attr = $_POST['attr'];

$update_array = array();
$update_array[$attr] = array();
$ds = $_SESSION['ldap']->server;
$res = @ldap_modify( $ds, $dn, $update_array );
if( $res ) {
	$redirect_url = "edit.php?dn=$encoded_dn";
	foreach( $update_array as $attr => $junk ) {
		$redirect_url .= "&modified_attrs[]=$attr";
	}
	header( "Location: $redirect_url" );
}
else {
	echo $_SESSION['header'];
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
	echo "</head><body>\n";
	StatusMessage("ERROR", _('Could not perform ldap_modify operation.'), ldap_error($ds));
	echo "</body></html>";
	exit;
}

?>
