<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  
  This code is based on phpLDAPadmin.
  Copyright (C) 2004  David Smith and phpLDAPadmin developers
  
  The original code was modified to fit for LDAP Account Manager by Roland Gruber.
  Copyright (C) 2005 - 2006  Roland Gruber

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
 * This function displays the LDAP tree for all the servers that you have
 * in config.php. We read the session variable 'tree' to know which
 * dns are expanded or collapsed. No query string parameters are expected,
 * however, you can use a '#' offset to scroll to a given dn. The syntax is
 * tree.php#<rawurlencoded dn>, so if I wanted to scroll to
 * dc=example,dc=com for server 3, the URL would be: 
 *	tree.php#3_dc%3Dexample%2Cdc%3Dcom
 *
 * @package lists
 * @subpackage tree
 * @author David Smith
 * @author Roland Gruber
 */
 
/**
 * Prints the HTML of the tree view.
 */
function draw_server_tree()
{
	global $tree;
	global $tree_icons;


			$refresh_href = 'refresh.php';
			$create_href =  'create_form.php?container=' .
				rawurlencode( $_SESSION['config']->get_Suffix('tree') );
	
			// Draw the quick-links below the server name: 
			// ( schema | search | refresh | create )
			echo '<tr><td colspan="100" class="links">';
			echo '<nobr>';
			echo '( ';
			echo '<a title="' . _('Refresh') . '"'.
				' href="' . $refresh_href . '">' . _('Refresh') . '</a> | ';
			echo '<a title="' . _('Create new entry') . '"'.
					' href="' . $create_href . '" target="right_frame">' . _('Create new entry') . '</a>';
			echo ' )</nobr></td></tr>';

			// Fetch and display the base DN for this server
			$base_dn = $_SESSION['config']->get_Suffix('tree');
	
			// Did we get a base_dn for this server somehow?
			if( $base_dn ) {
				echo "\n\n<!-- base DN row -->\n<tr>\n";

				// is the root of the tree expanded already?
				if( isset( $tree[$base_dn] ) ) {
					$expand_href =  "collapse.php?" .
						"dn=" . rawurlencode( $base_dn );
					$expand_img = "../../graphics/minus.png";
					$expand_alt = "-";
					$child_count = number_format( count( $tree[$base_dn] ) );
				}
				else {
                        $expand_href =  "expand.php?" .
                            "dn=" . rawurlencode( $base_dn );
                        $expand_img = "../../graphics/plus.png";
                        $expand_alt = "+";
                            $child_count = count( get_container_contents( 
                                        $base_dn, 0, 
                                        '(objectClass=*)') );
                            if( $child_count > $limit )
                                $child_count = $limit . '+';
                }
	
				$edit_href = "edit.php?dn=" . rawurlencode( $base_dn );

				$icon = isset( $tree_icons[ $base_dn ] )
					? $tree_icons[ $base_dn ]
					: get_icon( $base_dn );

				echo "<td class=\"expander\">";
				echo "<a href=\"$expand_href\"><img src=\"$expand_img\" alt=\"$expand_alt\" /></a></td>";
				echo "<td class=\"icon\"><a href=\"$edit_href\" target=\"right_frame\">";
				echo "<img src=\"../../graphics/$icon\" alt=\"img\" /></a></td>\n";
				echo "<td class=\"rdn\" colspan=\"98\"><nobr><a href=\"$edit_href\" ";
				echo " target=\"right_frame\">" . pretty_print_dn( $base_dn ) . '</a>';
				if( $child_count )
					echo " <span class=\"count\">($child_count)</span>";
				echo "</nobr></td>\n";
				echo "</tr>\n<!-- end of base DN row -->";

            if(isset($tree[ $base_dn ])
                && count( $tree[ $base_dn ] ) > 10 )
                draw_create_link( $base_dn, -1, urlencode( $base_dn ));

			}
	
			flush();
	
			// Is the root of the tree expanded already?
			if( isset( $tree[$base_dn] ) && is_array( $tree[$base_dn] ) ) {
				foreach( $tree[ $base_dn ] as $child_dn )
					draw_tree_html( $child_dn, 0 );
					echo '<tr><td class="spacer"></td>';
					echo '<td class="icon"><a href="' . $create_href .
						'" target="right_frame"><img src="../../graphics/star.png" alt="' . 
						_('Create new entry') . '" /></a></td>';
					echo '<td class="create" colspan="100"><a href="' . $create_href
						. '" target="right_frame" title="' . _('Create new entry')
						. ' ' . $base_dn.'">' . _('Create new entry') . '</a></td></tr>';
			}

}

/**
 * Checks and fixes an initial session's tree cache if needed.
 *
 * This function is not meant as a user-callable function, but rather a convenient,
 * automated method for checking the initial data structure of the session.
 */
function initialize_session_tree()
{
	// From the PHP manual: If you use $_SESSION don't use
	// session_register(), session_is_registered() or session_unregister()!
	if( ! array_key_exists( 'tree',  $_SESSION ) )
		$_SESSION['tree'] = array();
	if( ! array_key_exists( 'tree_icons', $_SESSION ) )
		$_SESSION['tree_icons'] = build_initial_tree_icons();

    // Make sure that the tree index is indeed well formed.
    if( ! is_array( $_SESSION['tree'] ) )
		$_SESSION['tree'] = array();
    if( ! is_array( $_SESSION['tree_icons'] ) )
		$_SESSION['tree_icons'] = build_initial_tree_icons();
        
}

/**
 * Builds the initial array that stores the icon-lookup for each server's DN in the tree browser. The returned
 * array is then stored in the current session. The structure of the returned array is simple, and looks like
 * this:
 * <code>
 *   Array 
 *    ( 
 *      [0] => Array 
 *          (
 *             [dc=example,dc=com] => "dcobject.png"
 *          )
 *      [1] => Array 
            (
 *            [o=Corporation] => "o.png"
 *          )
 *     )
 * </code>
 * This function is not meant as a user-callable function, but rather a convenient, automated method for 
 * setting up the initial data structure for the tree viewer's icon cache.
 */
function build_initial_tree_icons()
{
	$tree_icons = array();

	// initialize an empty array for each server
	$tree_icons = array();
	$tree_icons[ $_SESSION['config']->get_Suffix('tree') ] = get_icon( $_SESSION['config']->get_Suffix('tree') );

	return $tree_icons;
}

/**
 * Gets whether an entry exists based on its DN. If the entry exists, 
 * returns true. Otherwise returns false.
 *
 * @param string $dn The DN of the entry of interest.
 *
 * @return bool
 */
function dn_exists( $dn )
{
	$search_result = @ldap_read( $_SESSION['ldap']->server, $dn, 'objectClass=*', array('dn') );

	if( ! $search_result )
		return false;

	$num_entries = ldap_count_entries( $_SESSION['ldap']->server, $search_result );

	if( $num_entries > 0 )
		return true;
	else
		return false;
}

/**
 * Gets a list of child entries for an entry. Given a DN, this function fetches the list of DNs of
 * child entries one level beneath the parent. For example, for the following tree:
 *
 * <code>
 * dc=example,dc=com
 *   ou=People
 *      cn=Dave
 *      cn=Fred
 *      cn=Joe
 *      ou=More People
 *         cn=Mark
 *         cn=Bob
 * </code>
 *
 * Calling <code>get_container_contents( "ou=people,dc=example,dc=com" )</code>
 * would return the following list:
 * 
 * <code>
 *  cn=Dave
 *  cn=Fred
 *  cn=Joe
 *  ou=More People
 * </code>
 * 
 * @param string $dn The DN of the entry whose children to return.
 * @param int $size_limit (optional) The maximum number of entries to return. 
 *             If unspecified, no limit is applied to the number of entries in the returned.
 * @param string $filter (optional) An LDAP filter to apply when fetching children, example: "(objectClass=inetOrgPerson)"
 * @return array An array of DN strings listing the immediate children of the specified entry.
 */
function get_container_contents( $dn, $size_limit=0, $filter='(objectClass=*)' )
{
	$search = @ldap_list( $_SESSION['ldap']->server, $dn, $filter, array( 'dn' ), 1, $size_limit, 0);
	if( ! $search )
		return array();
	$search = ldap_get_entries( $_SESSION['ldap']->server, $search );

	$return = array();
	for( $i=0; $i<$search['count']; $i++ ) {
		$entry = $search[$i];
		$dn = $entry['dn'];
		$return[] = $dn;
	}

	return $return;
}

/**
 * Given a DN and server ID, this function reads the DN's objectClasses and
 * determines which icon best represents the entry. The results of this query
 * are cached in a session variable so it is not run every time the tree
 * browser changes, just when exposing new DNs that were not displayed
 * previously. That means we can afford a little bit of inefficiency here
 * in favor of coolness. :)
 *
 * This function returns a string like "country.png". All icon files are assumed
 * to be contained in the /../../graphics/ directory of phpLDAPadmin.
 *
 * Developers are encouraged to add new icons to the images directory and modify
 * this function as needed to suit their types of LDAP entries. If the modifications
 * are general to an LDAP audience, the phpLDAPadmin team will gladly accept them
 * as a patch.
 * 
 * @param string $dn The DN of the entry whose icon you wish to fetch.
 *
 * @return string
 */
function get_icon( $dn )
{
	// fetch and lowercase all the objectClasses in an array
	$object_classes = get_object_attr( $dn, 'objectClass', true );

	if( $object_classes === null || $object_classes === false || ! is_array( $object_classes ) )
		$object_classes = array();

	foreach( $object_classes as $i => $class )
		$object_classes[$i] = strtolower( $class );

	$rdn = get_rdn( $dn );
    $rdn_parts = explode( '=', $rdn, 2 );
    $rdn_value = isset( $rdn_parts[0] ) ? $rdn_parts[0] : null;
    unset( $rdn_parts );

	// return icon filename based upon objectClass value
	if( in_array( 'sambaaccount', $object_classes ) &&
		'$' == $rdn{ strlen($rdn) - 1 } )
		return 'nt_machine.png';
	if( in_array( 'sambaaccount', $object_classes ) )
		return 'nt_user.png';
	elseif( in_array( 'sambadomain', $object_classes ) )
		return 'smbDomain.png';
	elseif( in_array( 'person', $object_classes ) ||
	    in_array( 'organizationalperson', $object_classes ) ||
	    in_array( 'inetorgperson', $object_classes ) ||
	    in_array( 'account', $object_classes ) ||
   	    in_array( 'posixaccount', $object_classes )  )
		return 'user.png';
	elseif( in_array( 'organization', $object_classes ) )
		return 'o.png';
	elseif( in_array( 'organizationalunit', $object_classes ) )
		return 'ou.png';
	elseif( in_array( 'organizationalrole', $object_classes ) )
		return 'uid.png';
	elseif( in_array( 'dcobject', $object_classes ) ||
		in_array( 'domainrelatedobject', $object_classes ) ||
		in_array( 'domain', $object_classes ) ||
        in_array( 'builtindomain', $object_classes )) 
		return 'dc.png';
    elseif( in_array( 'alias', $object_classes ) )
        return 'go.png';
    elseif( in_array( 'room', $object_classes ) )
        return 'door.png';
    elseif( in_array( 'device', $object_classes ) )
        return 'device.png';
    elseif( in_array( 'document', $object_classes ) )
        return 'document.png';
	elseif( in_array( 'jammvirtualdomain', $object_classes ) )
		return 'mail.png';
	elseif( in_array( 'locality', $object_classes ) )
		return 'locality.png';
	elseif( in_array( 'posixgroup', $object_classes ) ||
		in_array( 'groupofnames', $object_classes ) ||
		in_array( 'group', $object_classes ) )
		return 'ou.png';
	elseif( in_array( 'applicationprocess', $object_classes ) )
		return 'process.png';
	elseif( in_array( 'groupofuniquenames', $object_classes ) )
		return 'uniquegroup.png';
	elseif( in_array( 'iphost', $object_classes ) )
		return 'host.png';
	elseif( in_array( 'nlsproductcontainer', $object_classes ) )
        return 'n.png';
	elseif( in_array( 'ndspkikeymaterial', $object_classes ) )
        return 'lock.png';
	elseif( in_array( 'server', $object_classes ) )
        return 'server-small.png';
	elseif( in_array( 'volume', $object_classes ) )
        return 'hard-drive.png';
	elseif( in_array( 'ndscatcatalog', $object_classes ) )
        return 'catalog.png';
	elseif( in_array( 'resource', $object_classes ) )
        return 'n.png';
	elseif( in_array( 'ldapgroup', $object_classes ) )
        return 'ldap-server.png';
	elseif( in_array( 'ldapserver', $object_classes ) )
        return 'ldap-server.png';
	elseif( in_array( 'nisserver', $object_classes ) )
        return 'ldap-server.png';
	elseif( in_array( 'rbscollection', $object_classes ) )
        return 'ou.png';
	elseif( in_array( 'dfsconfiguration', $object_classes ) )
        return 'nt_machine.png';
	elseif( in_array( 'applicationsettings', $object_classes ) )
        return 'server-settings.png';
	elseif( in_array( 'aspenalias', $object_classes ) )
        return 'mail.png';
	elseif( in_array( 'container', $object_classes ) )
        return 'folder.png';
	elseif( in_array( 'ipnetwork', $object_classes ) )
        return 'network.png';
	elseif( in_array( 'samserver', $object_classes ) )
        return 'server-small.png';
	elseif( in_array( 'lostandfound', $object_classes ) )
        return 'find.png';
	elseif( in_array( 'infrastructureupdate', $object_classes ) )
        return 'server-small.png';
	elseif( in_array( 'filelinktracking', $object_classes ) )
        return 'files.png';
	elseif( in_array( 'automountmap', $object_classes ) ||
            in_array( 'automount', $object_classes ) )
        return 'hard-drive.png';
    elseif( 0 === strpos( $rdn_value, "ipsec" ) || 
            0 == strcasecmp( $rdn_value, "IP Security" ) ||
            0 == strcasecmp( $rdn_value, "MSRADIUSPRIVKEY Secret" ) ||
            0 === strpos( $rdn_value, "BCKUPKEY_" ) )
        return 'lock.png';
	elseif( 0 == strcasecmp( $rdn_value, "MicrosoftDNS" ) )
        return 'dc.png';
	// Oh well, I don't know what it is. Use a generic icon.
	else
		return 'object.png';
}

/**
 * Much like get_object_attrs(), but only returns the values for
 * one attribute of an object. Example calls:
 *
 * <code>
 * print_r( get_object_attr( 0, "cn=Bob,ou=people,dc=example,dc=com", "sn" ) );
 * // prints:
 * //  Array 
 * //    ( 
 * //       [0] => "Smith"
 * //    )
 *
 * print_r( get_object_attr( 0, "cn=Bob,ou=people,dc=example,dc=com", "objectClass" ) );
 * // prints:
 * //  Array 
 * //    ( 
 * //       [0] => "top"
 * //       [1] => "person"
 * //    )
 * </code>
 * 
 * @param string $dn The distinguished name (DN) of the entry whose attributes/values to fetch.
 * @param string $attr The attribute whose value(s) to return (ie, "objectClass", "cn", "userPassword")
 * @param bool $lower_case_attr_names (optional) If true, all keys of the returned associative
 *              array will be lower case. Otherwise, they will be cased as the LDAP server returns
 *              them.
 * @see get_object_attrs
 */
function get_object_attr( $dn, $attr )
{
	$search = @ldap_read( $_SESSION['ldap']->server, $dn, '(objectClass=*)', array( $attr ), 0, 0, 0 );

	if( ! $search )
		return false;

	$entry = ldap_first_entry( $_SESSION['ldap']->server, $search );

	if( ! $entry )
		return false;
	
	$attrs = ldap_get_attributes( $_SESSION['ldap']->server, $entry );

	if( ! $attrs || $attrs['count'] == 0 )
		return false;

	$vals = ldap_get_values( $_SESSION['ldap']->server, $entry, $attr );
	unset( $vals['count'] );
	return $vals;
}

/**
 * Given a DN string, this returns the 'RDN' portion of the string.
 * For example. given 'cn=Manager,dc=example,dc=com', this function returns
 * 'cn=Manager' (it is really the exact opposite of get_container()).
 *
 * @param string $dn The DN whose RDN to return.
 * @param bool $include_attrs If true, include attributes in the RDN string. 
 *               See http://php.net/ldap_explode_dn for details
 *
 * @return string The RDN
 * @see get_container
 */
function get_rdn( $dn, $include_attrs=0 )
{
	if( $dn == null )
		return null;
	$rdn = pla_explode_dn( $dn, $include_attrs );
	if( 0 == count($rdn) )
		return $dn;
	if( ! isset( $rdn[0] ) )
		return $dn;
	$rdn = $rdn[0];
	return $rdn;
}

/**
 * Explode a DN into an array of its RDN parts. This function is UTF-8 safe
 * and replaces the buggy PHP ldap_explode_dn() which does not properly
 * handle UTF-8 DNs and also causes segmentation faults with some inputs.
 *
 * @param string $dn The DN to explode.
 * @param int $with_attriutes (optional) Whether to include attribute names (see http://php.net/ldap_explode_dn for details)
 *
 * @return array An array of RDN parts of this format:
 * <code>
 *   Array
 *    (
 *       [0] => uid=ppratt
 *       [1] => ou=People
 *       [2] => dc=example
 *       [3] => dc=com
 *    )
 * </code>
 */
function pla_explode_dn( $dn, $with_attributes=0 )
{
  // replace "\," with the hexadecimal value for safe split
  $var = preg_replace("/\\\,/","\\\\\\\\2C",$dn);

  // split the dn
  $result = explode(",",$var);
  
  //translate hex code into ascii for display
  foreach( $result as $key => $value )
    $result[$key] = preg_replace("/\\\([0-9A-Fa-f]{2})/e", "''.chr(hexdec('\\1')).''", $value);
  
  return $result;
}

/** 
 * Returns an HTML-beautified version of a DN.
 * Internally, this function makes use of pla_explode_dn() to break the 
 * the DN into its components. It then glues them back together with
 * "pretty" HTML. The returned HTML is NOT to be used as a real DN, but 
 * simply displayed.
 * 
 * @param string $dn The DN to pretty-print.
 * @return string
 */
function pretty_print_dn( $dn )
{
	$dn = pla_explode_dn( $dn );
	foreach( $dn as $i => $element ) {
		$element = htmlspecialchars( $element );
		$element = explode( '=', $element, 2 );
		$element = implode( '<span style="color: blue; font-family: courier; font-weight: bold">=</span>', $element );
		$dn[$i] = $element;
	}
	$dn = implode( '<span style="color:red; font-family:courier; font-weight: bold;">,</span>', $dn );

	return $dn;
}

/**
 * Compares 2 DNs. If they are equivelant, returns 0, otherwise,
 * returns their sorting order (similar to strcmp()):
 *      Returns < 0 if dn1 is less than dn2.
 *      Returns > 0 if dn1 is greater than dn2.
 *
 * The comparison is performed starting with the top-most element 
 * of the DN. Thus, the following list:
 *    <code>
 *       ou=people,dc=example,dc=com
 *       cn=Admin,ou=People,dc=example,dc=com
 *       cn=Joe,ou=people,dc=example,dc=com
 *       dc=example,dc=com
 *       cn=Fred,ou=people,dc=example,dc=org
 *       cn=Dave,ou=people,dc=example,dc=org
 *    </code>
 * Will be sorted thus using usort( $list, "pla_compare_dns" ):
 *    <code>
 *       dc=com
 *       dc=example,dc=com
 *       ou=people,dc=example,dc=com
 *       cn=Admin,ou=People,dc=example,dc=com
 *       cn=Joe,ou=people,dc=example,dc=com
 *       cn=Dave,ou=people,dc=example,dc=org
 *       cn=Fred,ou=people,dc=example,dc=org
 *    </code>
 *
 * @param string $dn1 The first of two DNs to compare
 * @param string $dn2 The second of two DNs to compare
 * @return int
 */
function pla_compare_dns( $dn1, $dn2 )
{
	// If they are obviously the same, return immediately
	if( 0 === strcasecmp( $dn1, $dn2 ) )
		return 0;
	
	$dn1_parts = pla_explode_dn( pla_reverse_dn($dn1) );
	$dn2_parts = pla_explode_dn( pla_reverse_dn($dn2) );
	assert( is_array( $dn1_parts ) );
	assert( is_array( $dn2_parts ) );
	
	// Foreach of the "parts" of the smaller DN
	for( $i=0; $i<count( $dn1_parts ) && $i<count( $dn2_parts ); $i++ )
	{
		// dnX_part is of the form: "cn=joe" or "cn = joe" or "dc=example"
		// ie, one part of a multi-part DN.
		$dn1_part = $dn1_parts[$i];
		$dn2_part = $dn2_parts[$i];
		
		// Each "part" consists of two sub-parts:
		//   1. the attribute (ie, "cn" or "o")
		//   2. the value (ie, "joe" or "example")
		$dn1_sub_parts = explode( '=', $dn1_part, 2 );
		$dn2_sub_parts = explode( '=', $dn2_part, 2 );

		$dn1_sub_part_attr = trim( $dn1_sub_parts[0] );
		$dn2_sub_part_attr = trim( $dn2_sub_parts[0] );
		if( 0 != ( $cmp = strcasecmp( $dn1_sub_part_attr, $dn2_sub_part_attr ) ) )
			return $cmp;

		$dn1_sub_part_val = trim( $dn1_sub_parts[1] );
		$dn2_sub_part_val = trim( $dn2_sub_parts[1] );
		if( 0 != ( $cmp = strcasecmp( $dn1_sub_part_val, $dn2_sub_part_val ) ) )
			return $cmp;
	}

    // If we iterated through all entries in the smaller of the two DNs
    // (ie, the one with fewer parts), and the entries are different sized,
    // then, the smaller of the two must be "less than" than the larger.
    if( count($dn1_parts) > count($dn2_parts) ) {
        return 1;
    } elseif( count( $dn2_parts ) > count( $dn1_parts ) ) {
        return -1;
    } else {
        return 0;
    }
}

/**
 * Reverses a DN such that the top-level RDN is first and the bottom-level RDN is last
 * For example:
 * <code>
 *   cn=Brigham,ou=People,dc=example,dc=com
 * </code>
 * Becomes: 
 * <code>
 *   dc=com,dc=example,ou=People,cn=Brigham
 * </code>
 * This makes it possible to sort lists of DNs such that they are grouped by container.
 *
 * @param string $dn The DN to reverse
 *
 * @return string The reversed DN
 *
 * @see pla_compare_dns
 */
function pla_reverse_dn($dn)
{
	foreach (pla_explode_dn($dn) as $key => $branch) {

		// pla_expode_dn returns the array with an extra count attribute, we can ignore that.
		if ( $key === "count" ) continue;

		if (isset($rev)) {
			$rev = $branch.",".$rev;
		} else {
			$rev = $branch;
		}
	}
	return $rev;
}

/**
 * Gets a DN string using the user-configured tree_display_format string to format it.
 */
function draw_formatted_dn( $dn )
{
    $format = '%rdn';
    preg_match_all( "/%[a-zA-Z_0-9]+/", $format, $tokens );
    $tokens = $tokens[0];
    foreach( $tokens as $token ) {
        if( 0 == strcasecmp( $token, '%dn' ) )
            $format = str_replace( $token, pretty_print_dn( $dn ), $format );
        elseif( 0 == strcasecmp( $token, '%rdn' ) )
            $format = str_replace( $token, pretty_print_dn( get_rdn( $dn ) ), $format );
        elseif( 0 == strcasecmp( $token, '%rdnvalue' ) ) {
            $rdn = get_rdn( $dn );
            $rdn_value = explode( '=', $rdn, 2 );
            $rdn_value = $rdn_value[1];
            $format = str_replace( $token, $rdn_value, $format );
        } else {
            $attr_name = str_replace( '%', '', $token );
            $attr_values = get_object_attr( $dn, $attr_name );
            if( null == $attr_values )
                $display = 'none';
            elseif( is_array( $attr_values ) )
                $display = htmlspecialchars( implode( ', ',  $attr_values ) );
            else
                $display = htmlspecialchars( $attr_values );
            $format = str_replace( $token, $display, $format );
        }
    }
    echo $format;
}

/**
 * Gets the attributes/values of an entry. Returns an associative array whose
 * keys are attribute value names and whose values are arrays of values for
 * said attribute. Optionally, callers may specify true for the parameter 
 * $lower_case_attr_names to force all keys in the associate array (attribute 
 * names) to be lower case. 
 * 
 * Sample return value of <code>get_object_attrs( 0, "cn=Bob,ou=pepole,dc=example,dc=com" )</code>
 *
 * <code>
 * Array
 *  (
 *   [objectClass] => Array
 *       (
 *           [0] => person
 *           [1] => top
 *       )
 *   [cn] => Array
 *       (
 *           [0] => Bob
 *       )
 *   [sn] => Array
 *       (
 *           [0] => Jones
 *       )
 *   [dn] => Array
 *       (
 *            [0] => cn=Bob,ou=pepole,dc=example,dc=com
 *       )
 *  )
 * </code>
 *
 * @param string $dn The distinguished name (DN) of the entry whose attributes/values to fetch.
 * @param bool $lower_case_attr_names (optional) If true, all keys of the returned associative
 *              array will be lower case. Otherwise, they will be cased as the LDAP server returns
 *              them.
 * @param int $deref For aliases and referrals, this parameter specifies whether to 
 *            follow references to the referenced DN or to fetch the attributes for
 *            the referencing DN. See http://php.net/ldap_search for the 4 valid
 *            options.
 * @return array
 * @see get_entry_system_attrs
 * @see get_object_attr
 */
function get_object_attrs( $dn, $lower_case_attr_names=false, $deref=LDAP_DEREF_NEVER )
{

	$conn = $_SESSION['ldap']->server;
	$search = @ldap_read( $conn, $dn, '(objectClass=*)', array( ), 0, 0, 0, $deref );

	if( ! $search )
		return false;

	$entry = ldap_first_entry( $conn, $search );

	if( ! $entry )
		return false;
	
	$attrs = ldap_get_attributes( $conn, $entry );

	if( ! $attrs || $attrs['count'] == 0 )
		return false;

	$num_attrs = $attrs['count'];
	unset( $attrs['count'] );

	// strip numerical inices
	for( $i=0; $i<$num_attrs; $i++ )
		unset( $attrs[$i] );

	$return_array = array();
	foreach( $attrs as $attr => $vals ) {
		if( $lower_case_attr_names )
			$attr = strtolower( $attr );
		if( is_attr_binary( $attr ) )
			$vals = ldap_get_values_len( $conn, $entry, $attr );
		unset( $vals['count'] );
		$return_array[ $attr ] = $vals;
	}

	ksort( $return_array );

	return $return_array;
}

/**
 * Given an attribute name and server ID number, this function returns
 * whether the attrbiute may contain binary data. This is useful for 
 * developers who wish to display the contents of an arbitrary attribute
 * but don't want to dump binary data on the page.
 * 
 * @param string $attr_name The name of the attribute to test.
 * @return bool
 *
 * @see is_jpeg_photo
 */
function is_attr_binary( $attr_name )
{
    $attr_name = strtolower( $attr_name );
    /** Determining if an attribute is binary can be an expensive
       operation. We cache the results for each attr name on each
       server in the $attr_cache to speed up subsequent calls. 
       The $attr_cache looks like this:
       Array 
        0 => Array 
              'objectclass' => false
              'cn' => false
              'usercertificate' => true
        1 => Array 
              'jpegphoto' => true 
              'cn' => false
    */

    static $attr_cache;
    if( isset( $attr_cache[ $attr_name ] ) )
        return $attr_cache[ $attr_name ];

    if( $attr_name == 'userpassword' ) {
        $attr_cache[ $attr_name ] = false;
        return false;
    }

    // Quick check: If the attr name ends in ";binary", then it's binary.
	if( 0 == strcasecmp( substr( $attr_name, strlen( $attr_name ) - 7 ), ";binary" ) ) {
        $attr_cache[ $attr_name ] = true;
		return true;
    }

    // See what the server schema says about this attribute
	$schema_attr = get_schema_attribute( $attr_name );
	if( ! $schema_attr ) {
        // Strangely, some attributeTypes may not show up in the server
        // schema. This behavior has been observed in MS Active Directory.
        $type = null;
        $syntax = null;
    } else {
        $type = $schema_attr->getType();
        $syntax = $schema_attr->getSyntaxOID();
    }

	if(	0 == strcasecmp( $type, 'Certificate' ) ||
		0 == strcasecmp( $type, 'Binary' ) ||
		0 == strcasecmp( $attr_name, 'usercertificate' ) ||
		0 == strcasecmp( $attr_name, 'usersmimecertificate' ) ||
		0 == strcasecmp( $attr_name, 'networkaddress' ) ||
		0 == strcasecmp( $attr_name, 'objectGUID' ) ||
		0 == strcasecmp( $attr_name, 'objectSID' ) ||
		$syntax == '1.3.6.1.4.1.1466.115.121.1.10' ||
		$syntax == '1.3.6.1.4.1.1466.115.121.1.28' ||
		$syntax == '1.3.6.1.4.1.1466.115.121.1.5' ||
		$syntax == '1.3.6.1.4.1.1466.115.121.1.8' ||
		$syntax == '1.3.6.1.4.1.1466.115.121.1.9' ) {
            $attr_cache[ $attr_name ] = true;
			return true;
    } else {
            $attr_cache[ $attr_name ] = false;
			return false;
    }
}

/** 
 * Prunes off anything after the ";" in an attr name. This is useful for
 * attributes that may have ";binary" appended to their names. With 
 * real_attr_name(), you can more easily fetch these attributes' schema
 * with their "real" attribute name.
 *
 * @param string $attr_name The name of the attribute to examine.
 * @return string
 */
function real_attr_name( $attr_name )
{
	$attr_name = preg_replace( "/;.*$/U", "", $attr_name );
	return $attr_name;
}

/** 
 * Gets the operational attributes for an entry. Given a DN, this function fetches that entry's
 * operational (ie, system or internal) attributes. These attributes include "createTimeStamp", 
 * "creatorsName", and any other attribute that the LDAP server sets automatically. The returned
 * associative array is of this form:
 * <code>
 *  Array 
 *  (
 *    [creatorsName] => Array 
 *        (
 *           [0] => "cn=Admin,dc=example,dc=com"
 *        )
 *    [createTimeStamp]=> Array 
 *        (
 *           [0] => "10401040130"
 *        )
 *    [hasSubordinates] => Array 
 *        (
 *           [0] => "FALSE"
 *        )
 *  )
 * </code>
 *
 * @param string $dn The DN of the entry whose interal attributes are desired.
 * @param int $deref For aliases and referrals, this parameter specifies whether to 
 *            follow references to the referenced DN or to fetch the attributes for
 *            the referencing DN. See http://php.net/ldap_search for the 4 valid
 *            options.
 * @return array An associative array whose keys are attribute names and whose values
 *              are arrays of values for the aforementioned attribute.
 */
function get_entry_system_attrs( $dn, $deref=LDAP_DEREF_NEVER )
{
	$conn = $_SESSION['ldap']->server;
	$attrs = array( 'creatorsname', 'createtimestamp', 'modifiersname', 
			'structuralObjectClass', 'entryUUID',  'modifytimestamp', 
			'subschemaSubentry', 'hasSubordinates', '+' );
	$search = @ldap_read( $conn, $dn, '(objectClass=*)', $attrs, 0, 0, 0, $deref );
	if( ! $search )
		return false;
	$entry = ldap_first_entry( $conn, $search );
	if( ! $entry)
	    return false;
	$attrs = ldap_get_attributes( $conn, $entry );
	if( ! $attrs )
		return false;
	if( ! isset( $attrs['count'] ) )
		return false;
	$count = $attrs['count'];
	unset( $attrs['count'] );
	$return_attrs = array();
	for( $i=0; $i<$count; $i++ ) {
		$attr_name = $attrs[$i];
		unset( $attrs[$attr_name]['count'] );
		$return_attrs[$attr_name] = $attrs[$attr_name];
	}
	return $return_attrs;
}

function arrayLower($array) {
	foreach ($array as $key => $value) {
		$newarray[$key] = strtolower($value);
	}
	return $newarray;
}

/**
 * Used to determine if the specified attribute is indeed a jpegPhoto. If the
 * specified attribute is one that houses jpeg data, true is returned. Otherwise
 * this function returns false.
 *
 * @param string $attr_name The name of the attribute to test.
 * @return bool
 * @see draw_jpeg_photos
 */
function is_jpeg_photo( $attr_name )
{
	// easy quick check
	if( 0 == strcasecmp( $attr_name, 'jpegPhoto' ) ||
	    0 == strcasecmp( $attr_name, 'photo' ) )
	    return true;

	// go to the schema and get the Syntax OID
	$schema_attr = get_schema_attribute( $attr_name );
	if( ! $schema_attr )
		return false;

	$oid = $schema_attr->getSyntaxOID();
	$type = $schema_attr->getType();

	if( 0 == strcasecmp( $type, 'JPEG' ) )
		return true;
	if( $oid == '1.3.6.1.4.1.1466.115.121.1.28' )
		return true;

	return false;
}

/**
 * Given an attribute name and server ID number, this function returns
 * whether the attrbiute contains boolean data. This is useful for 
 * developers who wish to display the contents of a boolean attribute
 * with a drop-down.
 * 
 * @param string $attr_name The name of the attribute to test.
 * @return bool
 */
function is_attr_boolean( $attr_name )
{
    $type = ( $schema_attr = get_schema_attribute( $attr_name ) ) ? 
        $schema_attr->getType() : 
        null;
    if( 0 == strcasecmp( 'boolean', $type ) ||
        0 == strcasecmp( 'isCriticalSystemObject', $attr_name ) ||
        0 == strcasecmp( 'showInAdvancedViewOnly', $attr_name ) )
        return true;
    else
        return false;
}

/** 
 * Get whether a string looks like an email address (user@example.com).
 * 
 * @param string $str The string to analyze.
 * @return bool Returns true if the specified string looks like 
 *   an email address or false otherwise.
 */
function is_mail_string( $str )
{
    $mail_regex = "/^[_A-Za-z0-9-]+(\\.[_A-Za-z0-9-]+)*@[A-Za-z0-9-]+(\\.[A-Za-z0-9-]+)*$/";
    if( preg_match( $mail_regex, $str ) )
        return true;
    else
        return false;
}

/** 
 * Get whether a string looks like a web URL (http://www.example.com/)
 * 
 * @param string $str The string to analyze.
 * @return bool Returns true if the specified string looks like 
 *   a web URL or false otherwise.
 */
function is_url_string( $str )
{
    $url_regex = '/(ftp|https?):\/\/+[\w\.\-\/\?\=\&]*\w+/';
    if( preg_match( $url_regex, $str ) )
        return true;
    else
        return false;

}

function sortAttrs($a,$b) {
	return strcmp ($a, $b);
}

/**
 * Determines if an attribute's value can contain multiple lines. Attributes that fall
 * in this multi-line category may be configured in config.php. Hence, this function
 * accesses the global variable $multi_line_attributes;
 *
 * Usage example:
 * <code>
 *  if( is_muli_line_attr( "postalAddress" ) )
 *      echo "<textarea name=\"postalAddress\"></textarea>";
 *  else 
 *      echo "<input name=\"postalAddress\" type=\"text\">";
 * </code>
 *
 * @param string $attr_name The name of the attribute of interestd (case insensivite)
 * @param string $val (optional) The current value of the attribute (speeds up the 
 *               process by searching for carriage returns already in the attribute value)
 * @return bool
 */
function is_multi_line_attr( $attr_name, $val=null )
{
    // First, check the optional val param for a \n or a \r
    if( null != $val && 
        ( false !== strpos( $val, "\n" ) || 
          false !== strpos( $val, "\r" ) ) )
        return true;

    // Next, compare strictly by name first
    global $multi_line_attributes;
    if( isset( $multi_line_attributes ) && is_array( $multi_line_attributes ) )
        foreach( $multi_line_attributes as $multi_line_attr_name )
            if( 0 == strcasecmp( $multi_line_attr_name, $attr_name ) ) {
                return true;
            }

        global $multi_line_syntax_oids;
        if( isset( $multi_line_syntax_oids ) && is_array( $multi_line_syntax_oids ) ) {
            $schema_attr = get_schema_attribute( $attr_name );
            if( ! $schema_attr )
                return false;
            $syntax_oid = $schema_attr->getSyntaxOID();
            if( ! $syntax_oid )
                return false;
            foreach( $multi_line_syntax_oids as $multi_line_syntax_oid )
                if( $multi_line_syntax_oid == $syntax_oid )
                    return true;
        }

    return false;

}

/**
 * Returns true if the attribute specified is required to take as input a DN.
 * Some examples include 'distinguishedName', 'member' and 'uniqueMember'.
 * @param string $attr_name The name of the attribute of interest (case insensitive)
 * @return bool
 */
function is_dn_attr( $attr_name )
{
    // Simple test first
    $dn_attrs = array( "aliasedObjectName" );
    foreach( $dn_attrs as $dn_attr )
        if( 0 == strcasecmp( $attr_name, $dn_attr ) )
            return true;

    // Now look at the schema OID
	$attr_schema = get_schema_attribute( $attr_name );
	if( ! $attr_schema )
		return false;
	$syntax_oid = $attr_schema->getSyntaxOID();
	if( '1.3.6.1.4.1.1466.115.121.1.12' == $syntax_oid )
		return true;
	if( '1.3.6.1.4.1.1466.115.121.1.34' == $syntax_oid )
		return true;
	$syntaxes = get_schema_syntaxes();
	if( ! isset( $syntaxes[ $syntax_oid ] ) )
		return false;
	$syntax_desc = $syntaxes[ $syntax_oid ]->getDescription();
	if( false !== strpos( strtolower($syntax_desc), 'distinguished name' ) )
		return true;
	return false;
}

function get_enc_type( $user_password )
{
    /* Capture the stuff in the { } to determine if this is crypt, md5, etc. */
    $enc_type = null;
    if( preg_match( "/{([^}]+)}/", $user_password, $enc_type) ) 
        return strtoupper($enc_type[1]); 
    else
        return null;
}

/**
 * Draw the jpegPhoto image(s) for an entry wrapped in HTML. Many options are available to 
 * specify how the images are to be displayed.
 *
 * Usage Examples:
 *  <code>
 *    draw_jpeg_photos( 0, "cn=Bob,ou=People,dc=example,dc=com", "jpegPhoto" true, false, "border: 1px; width: 150px" );
 *    draw_jpeg_photos( 1, "cn=Fred,ou=People,dc=example,dc=com" );
 *  </code>
 *
 * @param string $dn The DN of the entry that contains the jpeg attribute you want to draw.
 * @param string $attr_name The name of the attribute containing the jpeg data (usually 'jpegPhoto').
 * @param bool $draw_delete_buttons If true, draws a button beneath the image titled 'Delete' allowing the user
 *                  to delete the jpeg attribute by calling JavaScript function deleteAttribute() provided
 *                  in the default modification template.
 * @param bool $draw_bytes_and_size If true, draw text below the image indicating the byte size and dimensions.
 * @param string $table_html_attrs Specifies optional CSS style attributes for the table tag.
 *
 * @return void
 */
function draw_jpeg_photos( $dn, $attr_name='jpegPhoto', $draw_delete_buttons=false,
				$draw_bytes_and_size=true, $table_html_attrs='align="left"', $img_html_attrs='' )
{
	$jpeg_temp_dir = $_SESSION['lampath'] . 'tmp';

	$conn = $_SESSION['ldap']->server;
	$search_result = ldap_read( $conn, $dn, 'objectClass=*', array( $attr_name ) );
	$entry = ldap_first_entry( $conn, $search_result );

	echo "<table $table_html_attrs><td><center>\n\n";
	// for each jpegPhoto in the entry, draw it (there may be only one, and that's okay)
	$jpeg_data = @ldap_get_values_len( $conn, $entry, $attr_name );
    if( ! is_array( $jpeg_data ) ) {
        echo "Could not fetch jpeg data from LDAP server for attribute " . htmlspecialchars( $attr_name );
        return;
    }
	for( $i=0; $i<$jpeg_data['count']; $i++ )
	{
		// ensures that the photo is written to the specified jpeg_temp_dir
		$jpeg_temp_dir = realpath($jpeg_temp_dir.'/');
		$jpeg_filename = $jpeg_temp_dir . '/' . 'jpg' . $_SESSION['ldap']->new_rand() . '.jpg';
		$outjpeg = @fopen($jpeg_filename, "wb");
		fwrite($outjpeg, $jpeg_data[$i]);
		fclose ($outjpeg);
		$jpeg_data_size = filesize( $jpeg_filename );
		if( $jpeg_data_size < 6 && $draw_delete_buttons ) {
			echo _('jpegPhoto contains errors');
			echo '<br><a href="javascript:deleteAttribute( \'' . $attr_name . '\' );" style="color:red; font-size: 75%">'. _('Delete') .'</a>';
			continue;
		}

        if( function_exists( 'getimagesize' ) ) {
            $jpeg_dimensions = @getimagesize( $jpeg_filename );
            $width = $jpeg_dimensions[0];
            $height = $jpeg_dimensions[1];
        } else {
            $width = 0; 
            $height = 0;
        }
		if( $width > 300 ) {
			$scale_factor = 300 / $width;
			$img_width = 300;
			$img_height = $height * $scale_factor;
		} else {
			$img_width = $width;
			$img_height = $height;
		}
		echo "<img width=\"$img_width\" height=\"$img_height\" $img_html_attrs
			src=\"../../tmp/" . basename($jpeg_filename) . "\" /><br />\n";
		if( $draw_bytes_and_size ) {
			echo "<small>" . number_format($jpeg_data_size) . " bytes. ";
			echo "$width x $height pixels.<br /></small>\n\n";
		}

		if( $draw_delete_buttons )
		{ ?>
			<!-- JavaScript function deleteJpegPhoto() to be defined later by calling script -->
			<a href="javascript:deleteAttribute( '<?php echo $attr_name; ?>' );" style="color:red; font-size: 75%">Delete Photo</a>
		<?php }
	}
	echo "</center></td></table>\n\n";
}

/**
 * A handy ldap searching function very similar to PHP's ldap_search() with the 
 * following exceptions: Callers may specify a search scope and the return value 
 * is an array containing the search results rather than an LDAP result resource.
 *
 * Example usage:
 * <code>
 * $samba_users = ldap_search( 0, "(&(objectClass=sambaAccount)(objectClass=posixAccount))", 
 *                              "ou=People,dc=example,dc=com", array( "uid", "homeDirectory" ) );
 * print_r( $samba_users );
 * // prints (for example): 
 * //  Array 
 * //    ( 
 * //       [uid=jsmith,ou=People,dc=example,dc=com] => Array
 * //           (
 * //               [dn] => "uid=jsmith,ou=People,dc=example,dc=com"
 * //               [uid] => "jsmith"
 * //               [homeDirectory] => "\\server\jsmith"
 * //           )
 * //       [uid=byoung,ou=People,dc=example,dc=com] => Array
 * //           (
 * //               [dn] => "uid=byoung,ou=Samba,ou=People,dc=example,dc=com"
 * //               [uid] => "byoung"
 * //               [homeDirectory] => "\\server\byoung"
 * //           )
 * //    )
 * </code>
 * 
 * WARNING: This function will use a lot of memory on large searches since the entire result set is
 * stored in a single array. For large searches, you should consider sing the less memory intensive 
 * PHP LDAP API directly (ldap_search(), ldap_next_entry(), ldap_next_attribute(), etc).
 *
 * @param string $filter The LDAP filter to use when searching (example: "(objectClass=*)") (see RFC 2254)
 * @param string $base_dn The DN of the base of search. 
 * @param array $attrs An array of attributes to include in the search result (example: array( "objectClass", "uid", "sn" )).
 * @param string $scope The LDAP search scope. Must be one of "base", "one", or "sub". Standard LDAP search scope.
 * @param bool $sort_results Specify false to not sort results by DN or true to have the 
 *                  returned array sorted by DN (uses ksort)
 * @param int $deref When handling aliases or referrals, this specifies whether to follow referrals. Must be one of 
 *                  LDAP_DEREF_ALWAYS, LDAP_DEREF_NEVER, LDAP_DEREF_SEARCHING, or LDAP_DEREF_FINDING. See the PHP LDAP API for details.
 */
function pla_ldap_search( $filter, $base_dn=null, $attrs=array(), $scope='sub', $sort_results=true, $deref=LDAP_DEREF_ALWAYS )
{
	$ds = $_SESSION['ldap']->server;
	switch( $scope ) {
		case 'base':
			$search = @ldap_read( $ds, $base_dn, $filter, $attrs, 0, 0, 0, $deref );
			break;
		case 'one':
			$search = @ldap_list( $ds, $base_dn, $filter, $attrs, 0, 0, 0, $deref );
			break;
		case 'sub':
		default:
			$search = @ldap_search( $ds, $base_dn, $filter, $attrs, 0, 0, 0, $deref );
			break;
	}

	if( ! $search )
		return array();

	$return = array();
	//get the first entry identifier
	if( $entry_id = ldap_first_entry($ds,$search) )

		//iterate over the entries
		while($entry_id) {

			//get the distinguished name of the entry
			$dn = ldap_get_dn($ds,$entry_id);

			//get the attributes of the entry
			$attrs = ldap_get_attributes($ds,$entry_id);
			$return[$dn]['dn'] = $dn;

			//get the first attribute of the entry
			if($attr = ldap_first_attribute($ds,$entry_id,$attrs))

				//iterate over the attributes
				while($attr){
				  if( is_attr_binary($attr))
						$values = ldap_get_values_len($ds,$entry_id,$attr);
					else
						$values = ldap_get_values($ds,$entry_id,$attr);

					//get the number of values for this attribute
					$count = $values['count'];
					unset($values['count']);
					if($count==1)
						$return[$dn][$attr] = $values[0];
					else
						$return[$dn][$attr] = $values;

					$attr = ldap_next_attribute($ds,$entry_id,$attrs);
				}// end while attr

			$entry_id = ldap_next_entry($ds,$entry_id);

		} // end while entry_id

	if( $sort_results && is_array( $return ) )
		ksort( $return );

	return $return;
}

/**
 * Given a DN string, this returns the parent container portion of the string.
 * For example. given 'cn=Manager,dc=example,dc=com', this function returns
 * 'dc=example,dc=com'.
 * 
 * @param string $dn The DN whose container string to return.
 *
 * @return string The container
 * @see get_rdn
 */
function get_container( $dn )
{
	$parts = pla_explode_dn( $dn );
    if( count( $parts ) <= 1 )
        return null;
	$container = $parts[1];
	for( $i=2; $i<count($parts); $i++ )
		$container .= ',' . $parts[$i];
	return $container;
}

?>
