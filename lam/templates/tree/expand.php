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
 * This script alters the session variable 'tree', expanding it
 * at the dn specified in the query string. 
 *
 * Variables that come in as GET vars:
 *  - dn (rawurlencoded)
 *
 * Note: this script is equal and opposite to collapse.php
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

setlanguage();

// This allows us to display large sub-trees without running out of time.
@set_time_limit( 0 );

$dn = $_GET['dn'];
$encoded_dn = rawurlencode( $dn );

initialize_session_tree();

$tree = $_SESSION['tree'];
$tree_icons = $_SESSION['tree_icons'];

$contents = get_container_contents($dn, 0, '(objectClass=*)');

usort( $contents, 'pla_compare_dns' );
$tree[$dn] = $contents;

foreach( $contents as $dn )
	$tree_icons[$dn] = get_icon( $dn );

$_SESSION['tree'] = $tree;
$_SESSION['tree_icons'] = $tree_icons;

// This is for Opera. By putting "random junk" in the query string, it thinks
// that it does not have a cached version of the page, and will thus
// fetch the page rather than display the cached version
$time = gettimeofday();
$random_junk = md5( strtotime( 'now' ) . $time['usec'] );

header( "Location:tree.php?foo=$random_junk#{$encoded_dn}" );
?>
