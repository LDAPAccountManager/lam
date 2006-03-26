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
 * Displays a form for adding an attribute/value to an LDAP entry.
 *
 * Variables that come in as GET vars:
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

$dn = $_GET['dn'];
$encoded_dn = rawurlencode( $dn );
$rdn = get_rdn( $dn );
?>

<body>

<h3 class="tree_title"><?php echo sprintf( _('Add new attribute'), htmlspecialchars( $rdn ) ); ?></b></h3>
<h3 class="tree_subtitle"><?php echo _('DN'); ?>: <b><?php echo htmlspecialchars( ( $dn ) ); ?></b></h3>

<?php

$attrs = get_object_attrs( $dn );
$oclasses = get_object_attr( $dn, 'objectClass' );
if( ! is_array( $oclasses ) )
	$oclasses = array( $oclasses );
$avail_attrs = array();
$schema_oclasses = get_schema_objectclasses( $dn );
foreach( $oclasses as $oclass ) {
	$schema_oclass = get_schema_objectclass( $oclass, $dn );
	if( $schema_oclass && 0 == strcasecmp( 'objectclass', get_class( $schema_oclass ) ) )
		$avail_attrs = array_merge( $schema_oclass->getMustAttrNames( $schema_oclasses ),
				$schema_oclass->getMayAttrNames( $schema_oclasses ),
				$avail_attrs );
}

$avail_attrs = array_unique( $avail_attrs );
$avail_attrs = array_filter( $avail_attrs, "not_an_attr" );
sort( $avail_attrs );

$avail_binary_attrs = array();
foreach( $avail_attrs as $i => $attr ) {
	if( is_attr_binary( $attr ) ) {
		$avail_binary_attrs[] = $attr;
		unset( $avail_attrs[ $i ] );
	}
}

?>

<br />
<center>


	<?php echo _('Add new attribute'); ?>

	<?php if( is_array( $avail_attrs ) && count( $avail_attrs ) > 0 ) { ?>

		<br />
		<br />
		<form action="add_attr.php" method="post">
		<input type="hidden" name="dn" value="<?php echo htmlspecialchars($dn); ?>" />

		<select name="attr"><?php  
	
		$attr_select_html = '';
		usort($avail_attrs,"sortAttrs");
		foreach( $avail_attrs as $a ) { 
			$attr_display = htmlspecialchars( $a );
			echo $attr_display;
			$attr_select_html .= "<option>$attr_display</option>\n";
			echo "<option value=\"" . htmlspecialchars($a) . "\">$attr_display</option>";
		} ?>
		</select>
		<input type="text" name="val" size="20" />
		<input type="submit" name="submit" value="<?php echo _('Add'); ?>"/>
		</form>
	<?php } else { ?>
	
		<br />
		<br />
		<small><?php echo _('(no new attributes available for this entry)'); ?></small>
		<br />
		<br />
	
	<?php } ?>

<?php echo _('Add new binary attribute'); ?>
<?php if( count( $avail_binary_attrs ) > 0 ) { ?>
	<!-- Form to add a new BINARY attribute to this entry -->
	<form action="add_attr.php" method="post" enctype="multipart/form-data">
	<input type="hidden" name="dn" value="<?php echo $dn; ?>" />
	<input type="hidden" name="binary" value="true" />
	<br />
	<select name="attr">
	<?php  
		$attr_select_html = '';
		usort($avail_binary_attrs,"sortAttrs");
		foreach( $avail_binary_attrs as $a ) { 
			$attr_display = htmlspecialchars( $a );
	
			echo $attr_display;
			$attr_select_html .= "<option>$attr_display</option>\n";
			echo "<option value=\"" . htmlspecialchars($a) . "\">$attr_display</option>";
		} ?>
	</select>
	<input type="file" name="val" size="20" />
	<input type="submit" name="submit" value="<?php echo _('Add'); ?>"/>
    <?php 
        if( ! ini_get( 'file_uploads' ) )
            echo "<br><small><b>" . _('Your PHP configuration has disabled file uploads. Please check php.ini before proceeding.') . "</b></small><br>";
        else
            echo "<br><small><b>" . sprintf( _('Maximum file size: %s'), ini_get( 'upload_max_filesize' ) ) . "</b></small><br>";
    ?>
	</form>
<?php } else { ?>
	
	<br />
	<br />
	<small><?php echo _('(no new binary attributes available for this entry)'); ?></small>
	
<?php } ?>

</center>
</body>
</html>

<?php

/**
 * Given an attribute $x, this returns true if it is NOT already specified
 * in the current entry, returns false otherwise.
 */
function not_an_attr( $x )
{
	global $attrs;
	//return ! isset( $attrs[ strtolower( $x ) ] );
	foreach( $attrs as $attr => $values )
		if( 0 == strcasecmp( $attr, $x ) )
			return false;
	return true;
}


?>
