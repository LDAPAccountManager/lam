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
 * This script alters the session variable 'tree', by re-querying
 * the LDAP server to grab the contents of every expanded container.
 *
 * Variables that come in as GET vars:
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

// start session
startSecureSession();

if( ! array_key_exists( 'tree', $_SESSION ) )
	header( "Location: tree.php" );

$tree = $_SESSION['tree'];
$tree_icons = $_SESSION['tree_icons'];

// Get the icon for the base object for this server
$base_dn = $_SESSION['config']->get_Suffix('tree');
$tree_icons[ $base_dn ] = get_icon( $base_dn );

// get all the icons and container contents for all expanded entries
if( isset($tree) && is_array( $tree ) ) 
{
	foreach( $tree as $dn => $children )
	{
		$tree[$dn] = get_container_contents( $dn, 0, '(objectClass=*)' );
		if( is_array( $tree[$dn] ) ) {
			foreach( $tree[$dn] as $child_dn )
				$tree_icons[$child_dn] = get_icon( $child_dn );
			sort( $tree[ $dn ] );	
		}
	}
}
else
{
	header( "Location: tree.php" );
}

$_SESSION['tree'] = $tree;
$_SESSION['tree_icons'] = $tree_icons;
session_write_close();

header( "Location: tree.php" );


?>
