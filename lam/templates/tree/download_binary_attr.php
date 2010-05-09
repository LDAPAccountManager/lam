<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  
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
 * Used to send binary values to user.
 *
 * @package lists
 * @subpackage tree
 * @author David Smith
 * @author Roland Gruber
 */


/** security functions */
include_once('../../lib/security.inc');
/** tree functions */
include_once('../../lib/tree.inc');
/** access to configuration */
include_once('../../lib/config.inc');
/** LDAP functions */
include_once('../../lib/ldap.inc');
/** status messages */
include_once('../../lib/status.inc');

// start session
startSecureSession();

setlanguage();


$dn = rawurldecode( $_GET['dn'] );
$dn = rawurldecode( $_GET['dn'] );
$attr = $_GET['attr'];
// if there are multiple values in this attribute, which one do you want to see?
$value_num = isset( $_GET['value_num'] ) ? $_GET['value_num'] : 0;

$ds = $_SESSION['ldap']->server();

$search = @ldap_read( $ds, $dn, "(objectClass=*)", array($attr));
if( ! $search ) {
	echo $_SESSION['header'];
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
	echo "</head><body>\n";
	StatusMessage("ERROR", _('Encountered an error while performing search.'), ldap_error($ds));
	echo "</body></html>";
	exit;
}
$entry =  ldap_first_entry(     $ds, $search );
$attrs =  ldap_get_attributes(  $ds, $entry );
$attr =   ldap_first_attribute( $ds, $entry );
$values = ldap_get_values_len(  $ds, $entry, $attr );
$count = $values['count'];

// Dump the binary data to the browser
header( "Content-type: octet-stream" );
header( "Content-disposition: attachment; filename=$attr" );
header( "Expires: Mon, 26 Jul 1997 05:00:00 GMT" ); 
header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" ); 
echo $values[$value_num];

?>
