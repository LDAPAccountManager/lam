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
 * Recursively deletes the specified DN and all of its children
 * Variables that come in as POST vars:
 *  - dn (rawurlencoded)
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

echo $_SESSION['header'];
	
echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";

$dn = $_POST['dn'];
$encoded_dn = rawurlencode( $dn );
$rdn = get_rdn( $dn );

$ds = $_SESSION['ldap']->server;

echo "<body>\n";
echo "<h3 class=\"tree_title\">" . sprintf( _('Deleting %s'), htmlspecialchars($rdn) ) . "</h3>\n";
echo "<h3 class=\"tree_subtitle\">" . _('Recursive delete progress') . "</h3>";
echo "<br /><br />";
echo "<small>\n";
flush();

// prevent script from bailing early on a long delete
@set_time_limit( 0 );

$del_result = pla_rdelete( $dn );
echo "</small><br />\n";
if( $del_result )
{
	// kill the DN from the tree browser session variable and
	// refresh the tree viewer frame (left_frame)

	if( array_key_exists( 'tree', $_SESSION ) )
	{
		$tree = $_SESSION['tree'];

		// does it have children? (it shouldn't, but hey, you never know)	
		if( isset( $tree[$dn] ) )
			unset( $tree[$dn] );
		
        // Get a tree in the session if not already gotten
        initialize_session_tree();

		// search and destroy from the tree sesssion
		foreach( $tree as $tree_dn => $subtree )
			foreach( $subtree as $key => $sub_tree_dn )
				if( 0 == strcasecmp( $sub_tree_dn, $dn ) ) 
					unset( $tree[$tree_dn][$key] );
	}

	$_SESSION['tree'] = $tree;

	?>

	<script language="javascript">
		parent.left_frame.location.reload();
	</script>

	<?php 

	echo sprintf( _('Entry %s and sub-tree deleted successfully.'), '<b>' . htmlspecialchars( $dn ) . '</b>' );

} else {
	StatusMessage("ERROR", _('Failed to delete entry %s'), '', array(htmlspecialchars($dn)));
}

echo "</body></html>";

exit;


function pla_rdelete( $dn )
{
	$children = get_container_contents( $dn );
	global $ds;
	$ds = $_SESSION['ldap']->server;

	if( ! is_array( $children ) || count( $children ) == 0 ) {
		echo "<nobr>" . sprintf( _('Deleting %s'), htmlspecialchars( $dn ) ) . "...";
		flush();
		if( @ldap_delete( $ds, $dn ) ) {
			echo " <span style=\"color:green\">" . _('Success') . "</span></nobr><br />\n";
			return true;
		} else {
			StatusMessage("ERROR", _('Failed to delete entry %s'), '', array(htmlspecialchars($dn)));
		}
	} else {
		foreach( $children as $child_dn ) {
			pla_rdelete( $child_dn );
		}
		echo "<nobr>" . sprintf( _('Deleting %s'), htmlspecialchars( $dn ) ) . "...";
		flush();
		if( @ldap_delete( $ds, $dn ) ) {
			echo " <span style=\"color:green\">" . _('Success') . "</span></nobr><br />\n";
			return true;
		} else {
			StatusMessage("ERROR", _('Failed to delete entry %s'), '', array(htmlspecialchars($dn)));
		}
	}
	return false;
}
