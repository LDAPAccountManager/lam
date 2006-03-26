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
 * Adds an objectClass to the specified dn.
 * Variables that come in as POST vars:
 *
 * Note, this does not do any schema violation checking. That is
 * performed in add_oclass_form.php.
 *
 * Vars that come in as POST:
 *  - dn (rawurlencoded)
 *  - new_oclass
 *  - new_attrs (array, if any)
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

$dn = rawurldecode( $_POST['dn'] );
$encoded_dn = rawurlencode( $dn );
$new_oclass = unserialize( rawurldecode( $_POST['new_oclass'] ) );
$new_attrs = $_POST['new_attrs'];

$new_entry = array();
$new_entry['objectClass'] = $new_oclass;

$new_attrs_entry = array();
$new_oclass_entry = array( 'objectClass' => $new_oclass );

if( is_array( $new_attrs ) && count( $new_attrs ) > 0 )
	foreach( $new_attrs as $attr => $val ) {
		$new_entry[ $attr ] = $val;
	}

$ds = $_SESSION['ldap']->server;
$add_res = @ldap_mod_add( $ds, $dn, $new_entry );

if( ! $add_res )
{
	echo $_SESSION['header'];
	
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
	echo "</head>\n";
	StatusMessage('ERROR', _("Was unable to modify attribtues from DN: %s."), ldap_error( $ds ), array($dn));
	echo "</body></html>";
}
else
{
	header( "Location: edit.php?dn=$encoded_dn&amp;modified_attrs[]=objectclass" );
}

?>
