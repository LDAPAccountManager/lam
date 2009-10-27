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
 * Creates custom LDAP objects.
 *
 * @package lists
 * @subpackage tree
 * @author David Smith
 * @author Roland Gruber
 */

// Common to all templates
$rdn = isset( $_POST['rdn'] ) ? $_POST['rdn'] : null;
$container = $_POST['container'];

// Unique to this template
$step = isset( $_POST['step'] ) ? $_POST['step'] : 1;

if( $step == 1 )
{
	$oclasses = get_schema_objectClasses();
    if( ! $oclasses || ! is_array( $oclasses ) ) {
        StatusMessage("ERROR", "Unable to retrieve the schema from your LDAP server. Cannot continue with creation.", '');
    }
	?>

	<h4><?php echo _('Step 1 of 2: Name and object class(es)'); ?></h4>

	<form action="creation_template.php" method="post" name="creation_form">
	<input type="hidden" name="step" value="2" />
	<input type="hidden" name="template" value="<?php echo htmlspecialchars( $_POST['template'] ); ?>" />

	<table class="create">
	<tr>
		<td class="heading"><acronym title="<?php echo _('Relative distinguished name'); ?>"><?php echo _('RDN'); ?></acronym>:</td>
		<td><input type="text" name="rdn" value="<?php echo htmlspecialchars( $rdn ); ?>" size="30" /> <?php echo _('(example: cn=MyNewPerson)'); ?></td>
	</tr>
	<tr>
		<td class="heading"><?php echo _('Container'); ?></td>
		<td><input type="text" name="container" size="40" value="<?php echo htmlspecialchars( $container ); ?>" /></td>
	</tr>
	<tr>
		<td class="heading"><?php echo _('Object classes'); ?></td>
		<td>
			<select name="object_classes[]" multiple="true" size="15">
			<?php  foreach( $oclasses as $name => $oclass ) { 
                if( 0 == strcasecmp( "top", $name ) ) continue; ?>
				<option <?php if( $oclass->getType() == 'structural' ) echo 'style="font-weight: bold" '; ?> 
                    value="<?php echo htmlspecialchars($oclass->getName()); ?>">
					<?php echo htmlspecialchars($oclass->getName()); ?>
				</option>
				<?php  } ?>
			</select>
		</td>
	</tr>

	<tr>
		<td></td>
		<td><input type="submit" value="<?php echo _('Next'); ?>" /></td>
	</tr>
	</table>
	</form>
	
	<?php
}
if( $step == 2 )
{
	strlen( trim( $rdn ) ) != 0 or
		StatusMessage("ERROR", _("Data field for RDN is empty!"), '');

	if ((strlen( trim( $container ) ) == 0) || !(dn_exists( $container ))) {
		StatusMessage("ERROR", _('The container you specified (%s) does not exist. Please try again.'), "", array(htmlspecialchars($container)));
		echo "</body></html>";
		exit;
	}

	$oclasses = isset( $_POST['object_classes'] ) ? $_POST['object_classes'] : null;
	if( count( $oclasses ) == 0 ) {
		StatusMessage("ERROR", _('You did not select any object classes for this object. Please go back and do so.'), '');
		echo "</body></html>";
		exit;
	}
	$dn = trim( $container ) ? $rdn . ',' . $container : $rdn;

	// incrementally build up the all_attrs and required_attrs arrays
	$schema_oclasses = get_schema_objectclasses();
	$required_attrs = array();
	$all_attrs = array();
	foreach( $oclasses as $oclass_name ) {
		$oclass = get_schema_objectclass( $oclass_name  );
		if( $oclass ) {
			$required_attrs = array_merge( $required_attrs, 
						$oclass->getMustAttrNames( $schema_oclasses ) );
			$all_attrs = array_merge( $all_attrs, 
						$oclass->getMustAttrNames( $schema_oclasses ), 
						$oclass->getMayAttrNames( $schema_oclasses ) );
		} 
	}

	$required_attrs = array_unique( $required_attrs );
	$all_attrs = array_unique( $all_attrs );
    remove_aliases( $required_attrs);
    remove_aliases( $all_attrs);
	sort( $required_attrs );
	sort( $all_attrs );

    // if for some reason "ObjectClass" ends up in the list of
    // $all_attrs or $required_attrs, remove it! This is a fix
    // for bug 927487 
    foreach( $all_attrs as $i => $attr_name )
        if( 0 == strcasecmp( $attr_name, 'objectClass' ) ) {
            unset( $all_attrs[$i] );
            $all_attrs = array_values( $all_attrs );
            break;
        }

    foreach( $required_attrs as $i => $attr_name )
        if( 0 == strcasecmp( $attr_name, 'objectClass' ) ) {
            unset( $required_attrs[$i] );
            $required_attrs = array_values( $required_attrs );
            break;
        }

	// remove binary attributes and add them to the binary_attrs array
	$binary_attrs = array();
	foreach( $all_attrs as $i => $attr_name ) {
		if( is_attr_binary( $attr_name )  ) {
			unset( $all_attrs[ $i ] );
			$binary_attrs[] = $attr_name;
		}
	}

    // If we trim any attrs out above, then we will have a gap in the index
    // sequence and will get an "undefined index" error below. This prevents
    // that from happening.
    $all_attrs = array_values( $all_attrs );
	
	// add the required attribute based on the RDN provided by the user
	// (ie, if the user specifies "cn=Bob" for their RDN, make sure "cn" is
       	// in the list of required attributes.
	$rdn_attr = trim( substr( $rdn, 0, strpos( $rdn, '=' ) ) );
	$rdn_value = trim( substr( $rdn, strpos( $rdn, '=' ) + 1 ) );
    $rdn_value = @pla_explode_dn( $rdn );
    $rdn_value = @explode( '=', $rdn_value[0], 2 );
    $rdn_value = @$rdn_value[1];
	if( in_array( $rdn_attr, $all_attrs ) && ! in_array( $rdn_attr, $required_attrs ) )
		$required_attrs[] = $rdn_attr;
	?>

	<h4><?php echo _('Step 2 of 2: Specify attributes and values'); ?></h4>
	
	<form action="create.php" method="post"  enctype="multipart/form-data">
	<input type="hidden" name="step" value="2" />
	<input type="hidden" name="new_dn" value="<?php echo htmlspecialchars( $dn ); ?>" />
	<input type="hidden" name="new_rdn" value="<?php echo htmlspecialchars( $rdn ); ?>" />
	<input type="hidden" name="container" value="<?php echo htmlspecialchars( $container ); ?>" />
	<input type="hidden" name="object_classes" value="<?php echo rawurlencode(serialize($oclasses)); ?>" />
	
	<table class="edit_dn" cellspacing="0">
	<tr><th colspan="2"><?php echo _('Required attributes'); ?></th></tr>
	<?php  if( count( $required_attrs ) == 0 ) {
			echo "<tr class=\"row1\"><td colspan=\"2\"><center>(" . _('none') . ")</center></td></tr>\n";
		} else 
	
		foreach( $required_attrs as $count => $attr ) { ?>
			<tr>
		    <td class="attr"><b><?php 
		
			$attr_display = htmlspecialchars( $attr );

			echo $attr_display;
			
			?></b></td></tr>
            <tr>
		<td class="val"><input 	type="<?php echo (is_attr_binary( $attr ) ? "file" : "text"); ?>"
					name="required_attrs[<?php echo htmlspecialchars($attr); ?>]"
					value="<?php echo ($attr == $rdn_attr ? htmlspecialchars($rdn_value) : '')  ?>" size="40" />
	</tr>
	<?php  } ?>
	
	<tr><th colspan="2">&nbsp;</th></tr>
	<tr><th colspan="2"><?php echo _('Optional attributes'); ?></th></tr>
	
	<?php if( count( $all_attrs ) == 0 ) { ?>
		<tr><td colspan="2"><center>(<?php echo _('none'); ?>)</center></td></tr>
	<?php } else { ?>
		<?php  for($i=0; $i<min( count( $all_attrs ), 10 ); $i++ ) { $attr = $all_attrs[$i] ?>
            <tr>
			<td class="attr"><select style="background-color: #ddd; font-weight: bold" name="attrs[<?php echo $i; ?>]"><?php echo get_attr_select_html( $all_attrs, $attr ); ?></select></td>
            </tr>
            <tr>
			<td class="val"><input type="text" name="vals[<?php echo $i; ?>]" value="" size="40" />
    		</tr>
		<?php } ?>
	<?php  } ?>
	
	<?php if( count( $binary_attrs ) > 0 ) { ?>
	<tr><th colspan="2"><?php echo _('Optional binary attributes'); ?></th></tr>
		<?php for( $k=$i; $k<$i+count($binary_attrs); $k++ ) { $attr = $binary_attrs[$k-$i]; ?>
		<tr><td class="attr"><select style="background-color: #ddd; font-weight: bold" name="attrs[<?php echo $k; ?>]"><?php echo get_binary_attr_select_html( $binary_attrs, $attr );?></select></td></tr>
		<tr><td class="val"><input type="file" name="vals[<?php echo $k; ?>]" value="" size="25" /></td></tr>
		<?php } ?>
	<?php } ?>

    <tr><td>
	<center>
		<input type="submit" name="submit" value="<?php echo _('Create'); ?>" />
	</center>
    </td></tr>

	</table>

<?php } 

/**
* Returns option values.
*/
function get_attr_select_html( $all_attrs, $highlight_attr=null )
{
	$attr_select_html = "";
    if( ! is_array( $all_attrs ) )
        return null;
	foreach( $all_attrs as $a ) {
		$attr_display = htmlspecialchars( $a );
		$a = htmlspecialchars( $a );
		$attr_select_html .= "<option value=\"$a\"";
        if( 0 == strcasecmp( $highlight_attr, $a ) )
            $attr_select_html .= " selected";
        $attr_select_html .= ">$attr_display</option>\n";
	}
    return $attr_select_html;
}

/**
* Returns option values.
*/
function get_binary_attr_select_html( $binary_attrs, $highlight_attr=null )
{
	$binary_attr_select_html = "";
    if( ! is_array( $binary_attrs ) )
        return null;
	if( count( $binary_attrs ) == 0 ) 
        return null;
    foreach( $binary_attrs as $a ) {
        $attr_display = htmlspecialchars( $a );
        $binary_attr_select_html .= "<option";
        if( 0 == strcasecmp( $highlight_attr, $a ) )
            $binary_attr_select_html .= " selected";
        $binary_attr_select_html .= ">$attr_display</option>\n";
    }
    return $binary_attr_select_html;
}

/**
 * Removes attributes from the array that are aliases for eachother 
 * (just removes the second instance of the aliased attr)
 */
function remove_aliases( &$attribute_list)
{
    // remove aliases from the attribute_list array
    for( $i=0; $i<count( $attribute_list ); $i++  ) {
        if( ! isset( $attribute_list[ $i ] ) )
            continue;
        $attr_name1 = $attribute_list[ $i ];
        for( $k=0; $k<count( $attribute_list ); $k++  ) {
            if( ! isset( $attribute_list[ $k ] ) )
                continue;
            if( $i == $k )
                continue;
            $attr_name2 = $attribute_list[ $k ];
            //echo "Comparing $attr_name1 and $attr_name2<br>";
            $attr1 = get_schema_attribute( $attr_name1 );	
            if( null == $attr1 )
                continue;
            if( $attr1->isAliasFor( $attr_name2 ) ) {
                //echo "* Removing attribute ". $attribute_list[ $k ] . "<br>";
                unset( $attribute_list[ $k ] );
            }
        }
    }
    $attribute_list = array_values( $attribute_list );
}
?>

