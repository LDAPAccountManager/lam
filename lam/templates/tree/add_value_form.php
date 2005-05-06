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
 * add_value_form.php
 * Displays a form to allow the user to enter a new value to add
 * to the existing list of values for a multi-valued attribute.
 * Variables that come in as GET vars:
 *  - dn (rawurlencoded)
 *  - attr (rawurlencoded) the attribute to which we are adding a value 
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

$dn = isset( $_GET['dn'] ) ? $_GET['dn'] : null;
$encoded_dn = rawurlencode( $dn );
if( null != $dn ) {
	$rdn = get_rdn( $dn );
} else {
	$rdn = null;
}
$attr = $_GET['attr'];
$encoded_attr = rawurlencode( $attr );
$current_values = get_object_attr( $dn, $attr );
$num_current_values = ( is_array($current_values) ? count($current_values) : 1 );
$is_object_class = ( 0 == strcasecmp( $attr, 'objectClass' ) ) ? true : false;
$is_jpeg_photo = is_jpeg_photo( $attr ); //( 0 == strcasecmp( $attr, 'jpegPhoto' ) ) ? true : false;

if( $is_object_class ) { 
	// fetch all available objectClasses and remove those from the list that are already defined in the entry
	$schema_oclasses = get_schema_objectclasses();
	foreach( $current_values as $oclass )
		unset( $schema_oclasses[ strtolower( $oclass ) ] );
} else {
	$schema_attr = get_schema_attribute( $attr );
}

?>

<body>

<h3 class="tree_title">
	<?php echo _('Add new attribute'); ?>:
	<b><?php echo htmlspecialchars($attr); ?></b> 
</h3>
<h3 class="tree_subtitle">
	<?php echo _('DN'); ?>: <b><?php echo htmlspecialchars( $dn ); ?></b></h3>

<?php echo _('Current list of values for attribute:') . " <b>" . htmlspecialchars($attr); ?></b>
		
<?php if( is_attr_binary( $attr ) ) { ?>
	<ul>
	<?php if( is_array( $vals ) ) { for( $i=1; $i<=count($vals); $i++ ) { 
		$href = "download_binary_attr.php?dn=$encoded_dn&amp;attr=$attr&amp;value_num=" . ($i-1); ?>
		<li><a href="<?php echo $href; ?>"><img src="../../graphics/save.png" /> <?php echo _('download value') . ' ' .  $i; ?>)</a></li>
	<?php } } else { 
		$href = "download_binary_attr.php?dn=$encoded_dn&amp;attr=$attr"; ?>
		<li><a href="<?php echo $href; ?>"><img src="../../graphics/save.png" /> <?php echo _('download value'); ?></a></li>
	<?php } ?>
	</ul>
	<!-- Temporary warning until we find a way to add jpegPhoto values without an INAPROPRIATE_MATCHING error -->	
		<p><small>
		<?php echo _('Note: You will get an "inappropriate matching" error if you have not setup an EQUALITY rule on your LDAP server for this attribute.'); ?>
		</small></p>
	<!-- End of temporary warning -->
	
<?php } else { ?>

<ul class="current_values">
	<?php  if( is_array( $current_values ) ) /*$num_current_values > 1 )*/  {
		 foreach( $current_values as $val ) { ?>

			<li><nobr><?php echo htmlspecialchars(($val)); ?></nobr></li>

		<?php  } ?>
	<?php  } else { ?>

		<li><nobr><?php echo htmlspecialchars(($current_values)); ?></nobr></li>

	<?php  } ?>
</ul>

<?php } ?>

<?php echo _('Enter the value you would like to add:'); ?>
<br />
<br />

<?php if( $is_object_class ) { ?>

	<form action="add_oclass_form.php" method="post" class="new_value">
	<input type="hidden" name="dn" value="<?php echo $encoded_dn; ?>" />
	<select name="new_oclass">

	<?php foreach( $schema_oclasses as $name => $oclass ) {

		// exclude any structural ones, as they'll only generate an LDAP_OBJECT_CLASS_VIOLATION
		if ($oclass->type == "structural") continue;
?>

		<option value="<?php echo $oclass->getName(); ?>"><?php echo $oclass->getName(); ?></option>

	<?php } ?>

	</select> <input type="submit" value="<?php echo _('Add'); ?>" />
		
	<br />

<?php } else { ?>

	<form action="add_value.php" method="post" class="new_value" name="new_value_form"<?php 
		if( is_attr_binary( $attr ) ) echo "enctype=\"multipart/form-data\""; ?>>
	<input type="hidden" name="dn" value="<?php echo $encoded_dn; ?>" />
	<input type="hidden" name="attr" value="<?php echo $encoded_attr; ?>" />

	<?php if( is_attr_binary( $attr ) ) { ?>
		<input type="file" name="new_value" />
		<input type="hidden" name="binary" value="true" />
	<?php } else { ?>
        <?php if( is_multi_line_attr( $attr ) ) { ?>
            <textarea name="new_value" rows="3" cols="30"></textarea>
        <?php } else { ?>
		<input type="text" <?php 
				if( $schema_attr->getMaxLength() ) 
					echo "maxlength=\"" . $schema_attr->getMaxLength() . "\" "; 
				?>name="new_value" size="40" value="" />
        <?php } ?>
	<?php } ?>

	<input type="submit" name="submit" value="<?php echo _('Add'); ?>" />
	<br />

	<?php if( $schema_attr->getDescription() ) { ?>
		<small><b><?php echo _('Description'); ?>:</b> <?php echo $schema_attr->getDescription(); ?></small><br />
	<?php } ?>

	<?php if( $schema_attr->getType() ) { ?>
		<small><b><?php echo _('Syntax'); ?>:</b> <?php echo $schema_attr->getType(); ?></small><br />
	<?php } ?>

	<?php if( $schema_attr->getMaxLength() ) { ?>
		<small><b><?php echo _('Maximum length'); ?>:</b> <?php echo number_format( $schema_attr->getMaxLength() ); ?></small><br />
	<?php } ?>

	</form>

<?php } ?>

</body>
</html>
