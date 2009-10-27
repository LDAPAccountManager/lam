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
 * Creates a new object.
 *
 * Variables that come in as POST vars:
 *  - new_dn
 *  - attrs (an array of attributes)
 *  - vals (an array of values for the above attrs)
 *  - required_attrs (an array with indices being the attributes,
 *		      and the values being their respective values)
 *  - object_classes (rawurlencoded, and serialized array of objectClasses)
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

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

$new_dn = isset( $_POST['new_dn'] ) ? $_POST['new_dn'] : null;
$encoded_dn = rawurlencode( $new_dn );
$vals = isset( $_POST['vals'] ) ? $_POST['vals'] : array();
$attrs = isset( $_POST['attrs'] ) ? $_POST['attrs'] : array();
$required_attrs = isset( $_POST['required_attrs'] ) ? $_POST['required_attrs'] : false;
$object_classes = unserialize( rawurldecode( $_POST['object_classes'] ) );
$container = get_container( $new_dn );

// build the new entry
$new_entry = array();
if( isset( $required_attrs ) && is_array( $required_attrs ) ) {
	foreach( $required_attrs as $attr => $val ) {
		if( $val == '' ) {
			echo $_SESSION['header'];
			echo "<title>LDAP Account Manager</title>\n";
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
			echo "</head><body>\n";
	
			StatusMessage("ERROR", _('You left the value blank for required attribute: %s.'), '', array(htmlspecialchars($attr)));
			
			echo "</body></html>";
			exit;
		}
		$new_entry[ $attr ][] = $val;
	}
}

if( isset( $attrs ) && is_array( $attrs ) ) {
	foreach( $attrs as $i => $attr ) {
		if( is_attr_binary( $attr ) ) {
			if( isset( $_FILES['vals']['name'][$i] ) && $_FILES['vals']['name'][$i] != '' ) {
				// read in the data from the file
				$file = $_FILES['vals']['tmp_name'][$i];
				$f = fopen( $file, 'r' );
				$binary_data = fread( $f, filesize( $file ) );
				fclose( $f );
				$val = $binary_data;
				$new_entry[ $attr ][] = $val;
			}
		} else {
            $val = isset( $vals[$i] ) ? $vals[$i] : '';
			if( '' !== trim($val) )
			  $new_entry[ $attr ][] = $val;
		}
	}
}

$new_entry['objectClass'] = $object_classes;
if( ! in_array( 'top', $new_entry['objectClass'] ) )
	$new_entry['objectClass'][] = 'top';

foreach( $new_entry as $attr => $vals ) {
	if( ! is_attr_binary( $attr ) )
		if( is_array( $vals ) ) {
			foreach( $vals as $i => $v ) {
                           $new_entry[ $attr ][ $i ] = $v;
			}
		}
		else {
			$new_entry[ $attr ] = $vals;
		}
}

//echo "<pre>"; var_dump( $new_dn );print_r( $new_entry ); echo "</pre>";

$ds = $_SESSION['ldap']->server();

// Check the user-defined custom call back first
$add_result = @ldap_add( $ds, $new_dn, $new_entry );
if( $add_result )
{
    $redirect_url = "edit.php?dn=" . rawurlencode( $new_dn );

	if( array_key_exists( 'tree', $_SESSION ) )
	{
		$tree = $_SESSION['tree'];
		$tree_icons = $_SESSION['tree_icons'];

		if( isset( $tree[$container] ) ) {
			$tree[$container][] = $new_dn;
			sort( $tree[$container] );
			$tree_icons[$new_dn] = get_icon( $new_dn );
		}

		$_SESSION['tree'] = $tree;
		$_SESSION['tree_icons'] = $tree_icons;
		session_write_close();
	}

	?>
	<?php
	if( isset( $tree[$container])) {
		echo $_SESSION['header'];
		echo "<meta http-equiv=\"refresh\" content=\"0; URL=" . $redirect_url . "\">\n";
		echo "<title></title>\n";
		echo "</head>\n";
		echo "<body>\n";
		echo "<script language=\"javascript\">";
			echo "parent.left_frame.location.reload();";
		echo "</script>";
		// print link if refresh does not work
		echo "<p>\n";
		echo "<a href=\"" . $redirect_url . "\">" . _("Click here if you are not directed to the next page.") . "</a>\n";
		echo "</p>\n";
		echo "</body>\n";
		echo "</html>\n";
	}
	?>

<?PHP
	}
	else {
		echo $_SESSION['header'];
		echo "<title>LDAP Account Manager</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";

		StatusMessage("ERROR", _("LAM was unable to create account %s! An LDAP error occured."), ldap_error($ds), array($new_dn));

		echo "</body></html>\n";
	}
	
?>
