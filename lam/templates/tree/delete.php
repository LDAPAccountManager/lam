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
 * Deletes a DN and presents a "job's done" message.
 *
 * Variables that come in as POST vars:
 *  - dn (rawurlencoded)
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

echo $_SESSION['header'];
	
echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
echo "<body>";

$dn = $_POST['dn'];
$encoded_dn = rawurlencode( $dn );

$ds = $_SESSION['ldap']->server;

$del_result = @ldap_delete( $ds, $dn );

if( $del_result )
{
	// kill the DN from the tree browser session variable and
	// refresh the tree viewer frame (left_frame)

	if( array_key_exists( 'tree', $_SESSION ) )
	{
		$tree = $_SESSION['tree'];
		if( isset( $tree ) && is_array( $tree ) ) {

			// does it have children? (it shouldn't, but hey, you never know)	
			if( isset( $tree[$dn] ) )
				unset( $tree[$dn] );
			
			// search and destroy
			foreach( $tree as $tree_dn => $subtree )
				foreach( $subtree as $key => $sub_tree_dn )
					if( 0 == strcasecmp( $sub_tree_dn, $dn ) ) 
						unset( $tree[$tree_dn][$key] );
			$_SESSION['tree'] = $tree;
		}
	}

	?>

	<script language="javascript">
		parent.left_frame.location.reload();
	</script>

	<br />
	<br />
	<center><?php echo sprintf( _('Entry %s deleted successfully.'), $dn ); ?></center>

	<?php 


} else {
    StatusMessage("ERROR", _("Was unable to delete DN: %s."), '', array($dn));
}

echo "</body></html>";
