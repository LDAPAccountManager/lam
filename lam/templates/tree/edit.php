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
 * Displays the specified dn from the specified server for editing
 *
 * Variables that come in as GET vars:
 *  - dn (rawurlencoded)
 *  - modified_attrs (optional) an array of attributes to highlight as 
 *                              they were changed by the last operation
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

/** If an entry has more children than this, stop searching and display this amount with a '+' */
$max_children = 100;

$dn = $_GET['dn'];
$encoded_dn = rawurlencode( $dn );
$modified_attrs = isset( $_GET['modified_attrs'] ) ? $_GET['modified_attrs'] : false;
$show_internal_attrs = isset( $_GET['show_internal_attrs'] ) ? true : false;
if( null != $dn ) {
	$rdn = pla_explode_dn( $dn );
	if( isset( $rdn[0] ) )
		$rdn = $rdn[0];
	else
		$rdn = null;
} else {
	$rdn = null;
}

$attrs = get_object_attrs( $dn, false );

$system_attrs = get_entry_system_attrs( $dn );
dn_exists( $dn ) or StatusMessage('ERROR', _('No such entry!'), $dn);

echo $_SESSION['header'];

echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
?>
<body>

<h3 class="tree_title"><?php echo htmlspecialchars( ( $rdn ) ); ?></h3>
<h3 class="tree_subtitle"></b><?php echo _('DN');?>: <b><?php echo htmlspecialchars( ( $dn ) ); ?></b></h3>

<table class="tree_edit_dn_menu">

<tr>
	<?php  $time = gettimeofday(); $random_junk = md5( strtotime( 'now' ) . $time['usec'] ); ?>
	<td class="icon"><img src="../../graphics/refresh.png" /></td>
	<td><a href="edit.php?dn=<?php echo $encoded_dn; ?>&amp;random=<?php
			echo $random_junk; ?>"
	       title="<?php echo _('Refresh'); ?>"><?php echo _('Refresh'); ?></a></td>
<?php if( $show_internal_attrs ) { ?>
    <td class="icon"><img src="../../graphics/tools-no.png" /></td>
    <td><a href="edit.php?dn=<?php echo $encoded_dn; ?>"><?php echo _('Hide internal attributes'); ?></a></td>
<?php } else { ?>
    <td class="icon"><img src="../../graphics/tools.png" /></td>
    <td><a href="edit.php?dn=<?php echo $encoded_dn; ?>&amp;show_internal_attrs=true"><?php echo _('Show internal attributes'); ?></a></td>
<?php } ?>
</tr>

<tr>
	<td class="icon"><img src="../../graphics/delete.gif" /></td>
	<td><a style="color: red" href="delete_form.php?dn=<?php echo $encoded_dn; ?>">
	<?php echo _('Delete'); ?></a></td>
</tr>
<tr>
    	<td class="icon"><img src="../../graphics/light.png" /></td>
    	<td colspan="3"><span class="tree_hint"><?php echo _('Hint: To delete an attribute, empty the text field and click save.'); ?></span></td>
    </tr>
<tr>
	<td class="icon"><img src="../../graphics/star.png" /></td>
	<td><a href="<?php echo "create_form.php?container=$encoded_dn"; ?>"><?php echo _('Create new entry'); ?></a></td>
	<td class="icon"><img src="../../graphics/add.png" /></td>
	<td><a href="<?php echo "add_attr_form.php?dn=$encoded_dn"; ?>"><?php echo _('Add new attribute'); ?></a></td>
</tr>


<?php flush(); ?>

<?php if( $modified_attrs ) { ?>
<tr>
	<td class="icon"><img src="../../graphics/light.png" /></td>
	<?php if( count( $modified_attrs ) > 1 ) { ?>
		<td colspan="3"><?php echo sprintf( _('Some attributes (%s) were modified and are highlighted below.'), implode( ', ', $modified_attrs ) ); ?></td>
	<?php } else { ?>
		<td colspan="3"><?php echo sprintf( _('An attribute (%s) was modified and is highlighted below.'), implode( '', $modified_attrs ) ); ?></td>
	<?php } ?>
</tr>
<?php 
    // lower-case all the modified attrs to remove ambiguity when searching the array later
    foreach( $modified_attrs as $i => $attr ) {
        $modified_attrs[$i] = strtolower( $attr );
    }
}
?>

</table>

<?php flush(); ?>

<br />
<table class="tree_edit_dn">

<?php
if( $show_internal_attrs ) {
	$counter = 0;
	foreach( get_entry_system_attrs( $dn ) as $attr => $vals ) {
		$counter++;
		$schema_href = "../schema/schema.php?view=attributes&amp;viewvalue=" . real_attr_name($attr);
		?>

        <tr>
		<td class="val">
        <?php 
           if( is_attr_binary( $attr ) ) {
               $href = "download_binary_attr.php?dn=$encoded_dn&amp;attr=$attr";
               ?>
        		<small>
        		<?php echo _('Binary value'); ?><br />
        		<?php if( count( $vals ) > 1 ) { for( $i=1; $i<=count($vals); $i++ ) { ?>
        			<a href="<?php echo $href . "&amp;value_num=$i"; ?>"><img 
        				src="../../graphics/save.png" /> <?php echo _('download value'); ?>(<?php echo $i; ?>)</a><br />
        		<?php } } else { ?>
        			<a href="<?php echo $href; ?>"><img src="../../graphics/save.png" /> <?php echo _('download value'); ?></a><br />
        		<?php }
           }  else {
               foreach( $vals as $v ) {
                   echo htmlspecialchars( $v );
                   echo "<br />\n";
               }
           } ?>
		</td>
		</tr>
	<?php } 
	if( $counter == 0 )
		echo "<tr><td colspan=\"2\">(" . _('No internal attributes') . ")</td></tr>\n";
}

?>

<?php flush(); ?>

<!-- Table of attributes/values to edit -->

	<form action="update_confirm.php" method="post" name="edit_form">
	<input type="hidden" name="dn" value="<?php echo $dn; ?>" />

<?php $counter=0; ?>

<?php

if( ! $attrs || ! is_array( $attrs ) ) {
    echo "<tr><td colspan=\"2\">(" . _('This entry has no attributes') . ")</td></tr>\n";
    echo "</table>";
    echo "</html>";
    die();
}

uksort( $attrs, 'sortAttrs' );
foreach( $attrs as $attr => $vals ) { 

	flush();

    $schema_attr = get_schema_attribute( $attr, $dn );
    if( $schema_attr )
        $attr_syntax = $schema_attr->getSyntaxOID();
    else
        $attr_syntax = null;

	if( 0 == strcasecmp( $attr, 'dn' ) )
		continue;

    // Setup the $attr_note, which will be displayed to the right of the attr name (if any)
    $attr_note = '';

		$attr_note = "";
		$attr_display = $attr;

	// is this attribute required by an objectClass?
	$required_by = '';
	if( $schema_attr )
		foreach( $schema_attr->getRequiredByObjectClasses() as $required )
			if( in_array( strtolower( $required ), arrayLower( $attrs['objectClass'] ) ) ) {
				$required_by .= $required . ' ';
			}
	if( $required_by ) {
		if( trim( $attr_note ) )
			$attr_note .= ', ';
			$attr_note .= "<acronym title=\"" . sprintf( _('Required attribute for objectClass(es) %s'), $required_by ) . "\">" . _('required') . "</acronym>&nbsp;";
	}
	?>

	<?php  
    if( is_array( $modified_attrs ) && in_array( strtolower($attr), $modified_attrs ) )
        $is_modified_attr = true;
    else
        $is_modified_attr = false;
    ?>

    <?php if( $is_modified_attr ) { ?>
		<tr class="updated_attr">
	<?php  } else { ?>
        <tr>
	<?php  } ?>

	<td class="attr">
		<?php $schema_href="../schema/schema.php?view=attributes&amp;viewvalue=" . real_attr_name($attr); ?>
		<b>
            <a href="<?php echo $schema_href; ?>"><?php echo $attr_display; ?></a></b>
	</td>
    <td class="attr_note">
		<sup><small><?php echo $attr_note; ?></small></sup>
    </td>
    </tr>

    <?php if( $is_modified_attr ) { ?>
		<tr class="updated_attr">
	<?php  } else { ?>
        <tr>
	<?php  } ?>
	<td class="val" colspan="2">

	<?php 
	
	/*
	 * Is this attribute a jpegPhoto? 
	 */
	if( is_jpeg_photo( $attr ) ) {
		
		draw_jpeg_photos( $dn, $attr, true );
			
		// proceed to the next attribute
        echo "</td></tr>\n";
        if( $is_modified_attr ) 
            echo '<tr class="updated_attr"><td class="bottom" colspan="2"></td></tr>';
		continue;
	} 


	/*
	 * Is this attribute binary?
	 */
	if( is_attr_binary( $attr ) ) {
		$href = "download_binary_attr.php?dn=$encoded_dn&amp;attr=$attr";
		?>

		<small>
		<?php echo _('Binary value'); ?><br />
		<?php if( count( $vals ) > 1 ) { for( $i=1; $i<=count($vals); $i++ ) { ?>
			<a href="<?php echo $href . "&amp;value_num=$i"; ?>"><img 
				src="../../graphics/save.png" /> <?php echo _('download value'); ?>(<?php echo $i; ?>)</a><br />
		<?php } } else { ?>
			<a href="<?php echo $href; ?>"><img src="../../graphics/save.png" /> <?php echo _('download value'); ?></a><br />
		<?php } ?>

		<a href="javascript:deleteAttribute( '<?php echo $attr; ?>' );"
			style="color:red;"><img src="../../graphics/delete.gif" /> <?php echo _('delete attribute'); ?></a>

		</small>
		</td>
        </tr>

		<?php 
        if( $is_modified_attr ) 
            echo '<tr class="updated_attr"><td class="bottom" colspan="2"></td></tr>';
        continue; 
	}


	/*
	 * Note: at this point, the attribute must be text-based (not binary or jpeg)
	 */


	/*
	 * Is this a userPassword attribute?
	 */
	if( 0 == strcasecmp( $attr, 'userpassword' ) ) { 
		$user_password = $vals[0];
        $enc_type = get_enc_type( $user_password );

        // Set the default hashing type if the password is blank (must be newly created)
        if( $user_password == '' ) {
            $enc_type = get_default_hash();
        } 
        ?>

		<input type="hidden"
		       name="old_values[userpassword]" 
		       value="<?php echo htmlspecialchars($user_password); ?>" />

		<!-- Special case of enc_type to detect changes when user changes enc_type but not the password value -->
		<input size="38"
		       type="hidden"
		       name="old_enc_type"
		       value="<?php echo ($enc_type==''?'clear':$enc_type); ?>" />

        <br />
		<input style="width: 260px"
		       type="password"
		       name="new_values[userpassword]" 
               value="<?php echo htmlspecialchars( $user_password ); ?>" />

		<select name="enc_type">
			<option>clear</option>
			<option<?php echo $enc_type=='crypt'?' selected="true"':''; ?>>crypt</option>
			<option<?php echo $enc_type=='md5'?' selected="true"':''; ?>>md5</option>
			<option<?php echo $enc_type=='smd5'?' selected="true"':''; ?>>smd5</option>
			<option<?php echo $enc_type=='md5crypt'?' selected="true"':''; ?>>md5crypt</option>
			<option<?php echo $enc_type=='blowfish'?' selected="true"':''; ?>>blowfish</option>
			<option<?php echo $enc_type=='sha'?' selected="true"':''; ?>>sha</option>
			<option<?php echo $enc_type=='ssha'?' selected="true"':''; ?>>ssha</option>
			</select>

            <br />

        </td></tr>

		<?php 
        if( $is_modified_attr ) 
            echo '<tr class="updated_attr"><td class="bottom" colspan="2"></td></tr>';
        continue; 
	} 
	
	/*
	 * Is this a boolean attribute? 
	 */
     if( is_attr_boolean( $attr) ) {
		$val = $vals[0];
		?>

		<input type="hidden"
		       name="old_values[<?php echo htmlspecialchars( $attr ); ?>]" 
		       value="<?php echo htmlspecialchars($val); ?>" />

			<select name="new_values[<?php echo htmlspecialchars( $attr ); ?>]">
			<option value="TRUE"<?php echo ($val=='TRUE' ?  ' selected' : ''); ?>>
				<?php echo _('true'); ?></option>
			<option value="FALSE"<?php echo ($val=='FALSE' ? ' selected' : ''); ?>>
				<?php echo _('false'); ?></option>
			<option value="">(<?php echo _('none, remove value'); ?>)</option>
		</select>
        </td>
        </tr>

		<?php 
        if( $is_modified_attr ) 
            echo '<tr class="updated_attr"><td class="bottom" colspan="2"></td></tr>';
		continue; 
	}

	/*
	 * End of special case attributes (non plain text).
	 */


	/*
	 * This is a plain text attribute, to be displayed and edited in plain text.
	 */
	foreach( $vals as $i => $val ) {

        $input_name = "new_values[" . htmlspecialchars( $attr ) . "][$i]";
        // We smack an id="..." tag in here that doesn't have [][] in it to allow the 
        // draw_chooser_link() to identify it after the user clicks.
        $input_id =  "new_values_" . htmlspecialchars($attr) . "_" . $i;

        ?>

		<!-- The old_values array will let update.php know if the entry contents changed
		     between the time the user loaded this page and saved their changes. -->
		<input type="hidden"
		       name="old_values[<?php echo htmlspecialchars( $attr ); ?>][<?php echo $i; ?>]" 
		       value="<?php echo htmlspecialchars($val); ?>" />
        <?php

		// Is this value is a structural objectClass, make it read-only
		if( 0 == strcasecmp( $attr, 'objectClass' ) ) {
            ?>
            <a
              href="../schema/schema.php?view=objectClasses&amp;viewvalue=<?php echo htmlspecialchars( $val ); ?>"><img
                src="../../graphics/tree_info.png" /></a>
            <?php
			$schema_object = get_schema_objectclass( $val);
			if ($schema_object->type == 'structural') {
				echo "$val <small>(<acronym>" .
                        _('structural') . "</acronym>)</small><br />";
                ?>
        	<input type="hidden"
		       	name="<?php echo $input_name; ?>"
                id="<?php echo $input_id; ?>"
         	    value="<?php echo htmlspecialchars($val); ?>" />
                <?php
                continue;
			}
		}

		?>
			       
        <?php if( is_mail_string( $val ) ) { ?>
             <a 
                href="mailto:<?php echo htmlspecialchars($val); ?>"><img 
                        style="vertical-align: center" src="../../graphics/mail.png" /></a>
        <?php } elseif( is_url_string( $val ) ) { ?>
             <a 
                href="<?php echo htmlspecialchars($val); ?>"
                target="new"><img 
                        style="vertical-align: center" src="../../graphics/dc.png" /></a>

        <?php } ?>

        <?php if( is_multi_line_attr( $attr, $val ) ) { ?>
            <textarea
                class="val"
                rows="3"
         	    cols="50"
		       	name="<?php echo $input_name; ?>"
                id="<?php echo $input_id; ?>"><?php echo htmlspecialchars($val); ?></textarea>
        <?php } else { ?>
        	<input type="text"
                class="val"
		       	name="<?php echo $input_name; ?>"
                id="<?php echo $input_id; ?>"
         	    value="<?php echo htmlspecialchars($val); ?>" />
        <?php } ?>


		<?php 
		// draw a link for popping up the entry browser if this is the type of attribute
		// that houses DNs. 
		if( is_dn_attr( $attr ) )
			draw_chooser_link( "edit_form.$input_id", false );

        ?>

		<br />
	       
	<?php  } /* end foreach value */ ?>

		<?php 
		/* Draw the "add value" link under the list of values for this attributes */

		if(	( $schema_attr = get_schema_attribute( $attr, $dn ) ) &&
			! $schema_attr->getIsSingleValue() )
		{ 
			$add_href = "add_value_form.php?dn=$encoded_dn&amp;attr=" . rawurlencode( $attr ); 
			echo "<div class=\"add_value\">(<a href=\"$add_href\">" . 
				_('add value') . "</a>)</div>\n";
		} 
			   
		?>

	</td>
	</tr>

    <?php if( $is_modified_attr ) { ?>
		<tr class="updated_attr"><td class="bottom" colspan="2"></td></tr>
	<?php  } ?>

	<?php  

	flush();

} /* End foreach( $attrs as $attr => $vals ) */ ?>

	<tr><td colspan="2"><center><input type="submit" value="<?php echo _('Save'); ?>" /></center></td></tr></form>
	
<?php 
?>


</table>

<?php /* If this entry has a binary attribute, we need to provide a form for it to submit when deleting it. */ ?>
<script language="javascript">
//<!--
function deleteAttribute( attrName )
{
	if( confirm( "<?php echo _('Really delete attribute?'); ?> '" + attrName + "'" ) ) {
		document.delete_attribute_form.attr.value = attrName;
		document.delete_attribute_form.submit();
	}
}
//-->
</script>

<!-- This form is submitted by JavaScript when the user clicks "Delete attribute" on a binary attribute -->
<form name="delete_attribute_form" action="delete_attr.php" method="post">
	<input type="hidden" name="dn" value="<?php echo $dn; ?>" />
	<input type="hidden" name="attr" value="FILLED IN BY JAVASCRIPT" />
</form>

<?php 

echo "</body>\n</html>";
?>
