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
 * This script displays the LDAP tree for all the servers that you have
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

/** tree functions */
include_once('../../lib/tree.inc');
/** access to configuration */
include_once('../../lib/config.inc');
/** LDAP functions */
include_once('../../lib/ldap.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// This allows us to display large sub-trees without running out of time.
@set_time_limit( 0 );

// do we not have a tree and tree icons yet? Build a new one.
initialize_session_tree();

// get the tree and tree icons.
$tree = $_SESSION['tree'];
$tree_icons = $_SESSION['tree_icons'];


echo $_SESSION['header'];

echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
?>

<body>

<table class="tree" cellspacing="0">

<?php 

draw_server_tree();

?>

</table>
<?php 
//	echo "<pre>"; print_r( $tree ); 
?>

</body>
</html>

<?php
exit;

/**
 * Recursively descend on the given dn and draw the tree in html
 */
function draw_tree_html( $dn, $level = 0 )
{
	global $tree, $tree_icons, $search_result_size_limit;

	$encoded_dn = rawurlencode( $dn );
	$expand_href = "expand.php?dn=$encoded_dn";
	$collapse_href = "collapse.php?dn=$encoded_dn";
	$edit_href = "edit.php?dn=$encoded_dn";

	// should never happen, but just in case
	if( ! isset( $tree_icons[ $dn ] ) )
		$tree_icons[ $dn ] = get_icon( $dn );
	$img_src = '../../graphics/' . $tree_icons[ $dn ];

	$rdn = get_rdn( $dn );

	echo '<tr>';

	for( $i=0; $i<=$level; $i++ ) {
		echo '<td class="spacer"></td>' . "\n";
	}

	// is this node expanded? (deciding whether to draw "+" or "-")
	if( isset( $tree[$dn] ) ) { ?>
		<td class="expander">
			<nobr>
			<a href="<?php echo $collapse_href; ?>"><img src="../../graphics/minus.png" alt="-" /></a>
			</nobr>
		</td>
		<?php  $child_count = number_format( count( $tree[$dn] ) );
	} else { ?>	
		<td class="expander">
			<nobr>
			<a href="<?php echo $expand_href; ?>"><img src="../../graphics/plus.png" alt="+" /></a>
			</nobr>
		</td>
		<?php  	$limit = isset( $search_result_size_limit ) ? $search_result_size_limit : 50;
               $child_count = count( get_container_contents( $dn, $limit+1, 
                                     '(objectClass=*)') );
               if( $child_count > $limit )
                   $child_count = $limit . '+';
	} ?>	

	<td class="icon">
		<a href="<?php echo $edit_href; ?>"
		   target="right_frame"
		   name="<?php echo $encoded_dn; ?>"><img src="<?php echo $img_src; ?>" alt="img" /></a>
	</td>
	<td class="rdn" colspan="<?php echo (97-$level); ?>">
		<nobr>
			<a href="<?php echo $edit_href; ?>"
				target="right_frame"><?php echo ( draw_formatted_dn( $dn ) ); /*pretty_print_dn( $rdn ) );*/ ?></a>
				<?php if( $child_count ) { ?>
					<span class="count">(<?php echo $child_count; ?>)</span>
				<?php } ?>
		</nobr>
	</td>
	</tr>

	<?php 

	if( isset( $tree[$dn] ) && is_array( $tree[$dn] ) )	{
        // Draw the "create new" link at the top of the tree list if there are more than 10
        // entries in the listing for this node.
        if( count( $tree[$dn] ) > 10 )
	        draw_create_link( $rdn, $level, $encoded_dn );
		foreach( $tree[$dn] as $dn )
			draw_tree_html( $dn, $level+1 );
        // Always draw the "create new" link at the bottom of the listing
        draw_create_link( $rdn, $level, $encoded_dn );
	}
}

function draw_create_link( $rdn, $level, $encoded_dn )
{
    // print the "Create New object" link.
    $create_html = "";
    $create_href = "create_form.php?container=$encoded_dn";
    $create_html .= '<tr>';
    for( $i=0; $i<=$level; $i++ ) {
        $create_html .= '<td class="spacer"></td>';
    }
    $create_html .= '<td class="spacer"></td>';
    $create_html .= '<td class="icon"><a href="' . $create_href .
        '" target="right_frame"><img src="../../graphics/star.png" alt="' . _('Create new entry') . '" /></a></td>';
    $create_html .= '<td class="create" colspan="' . (97-$level) . '"><a href="' . $create_href . 
        '" target="right_frame" title="' . _('Create new entry') . ' ' . $rdn.'">' . 
        _('Create new entry') . '</a></td>';
    $create_html .= '</tr>';
    echo $create_html;
}

?>
