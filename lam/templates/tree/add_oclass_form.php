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
 * This page may simply add the objectClass and take you back to the edit page,
 * but, in one condition it may prompt the user for input. That condition is this:
 *
 *    If the user has requested to add an objectClass that requires a set of
 *    attributes with 1 or more not defined by the object. In that case, we will
 *    present a form for the user to add those attributes to the object.
 *
 * Variables that come in as POST vars:
 *  - dn (rawurlencoded)
 *  - new_oclass
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

$dn = rawurldecode( $_POST['dn'] );
$encoded_dn = rawurlencode( $dn );
$new_oclass = $_POST['new_oclass'];

/* Ensure that the object has defined all MUST attrs for this objectClass.
 * If it hasn't, present a form to have the user enter values for all the
 * newly required attrs. */

$entry = get_object_attrs( $dn, true );
$current_attrs = array();
foreach( $entry as $attr => $junk )
	$current_attrs[] = strtolower($attr);

// grab the required attributes for the new objectClass
$schema_oclasses = get_schema_objectclasses();
$must_attrs = array();
foreach($new_oclass as $oclass_name) {
	$oclass = get_schema_objectclass($oclass_name);
	if($oclass)
		$must_attrs = array_merge($must_attrs, $oclass->getMustAttrNames($schema_oclasses));
}
$must_attrs = array_unique( $must_attrs );

// We don't want any of the attr meta-data, just the string
//foreach( $must_attrs as $i => $attr )
	//$must_attrs[$i] = $attr->getName();

// build a list of the attributes that this new objectClass requires,
// but that the object does not currently contain
$needed_attrs = array();
foreach( $must_attrs as $attr ) {
    $attr = get_schema_attribute($attr);
    //echo "<pre>"; var_dump( $attr ); echo "</pre>";
    // First, check if one of this attr's aliases is already an attribute of this entry
    foreach( $attr->getAliases() as $alias_attr_name )
        if( in_array( strtolower( $alias_attr_name ), $current_attrs ) )
            // Skip this attribute since it's already in the entry
            continue;
	if( in_array( strtolower($attr->getName()), $current_attrs ) )
        continue;

    // We made it this far, so the attribute needs to be added to this entry in order 
    // to add this objectClass
    $needed_attrs[] = $attr;
}

if( count( $needed_attrs ) > 0 )
{
	echo $_SESSION['header'];
	
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
	echo "</head>\n";
	?>
	<body>
	
	<h3 class="tree_title"><?php echo _('DN') . ": " . $dn; ?></h3>
	<h3 class="tree_subtitle"><?php echo _('There are new required attributes which need to be set.'); ?></h3>

	<br />
	
	<form action="add_oclass.php" method="post">
	<input type="hidden" name="new_oclass" value="<?php echo rawurlencode(serialize($new_oclass)); ?>" />
	<input type="hidden" name="dn" value="<?php echo $encoded_dn; ?>" />
	
	<table class="tree_edit_dn" cellspacing="0">
	<tr><th colspan="2"><?php echo _('New required attributes:'); ?></th></tr>

	<?php foreach( $needed_attrs as $count => $attr ) { ?>
        <tr><td class="attr"><b><?php echo htmlspecialchars($attr->getName()); ?></b></td></tr>
		<tr><td class="val"><input type="text" name="new_attrs[<?php echo htmlspecialchars($attr->getName()); ?>]" value="" size="40" /></tr>
	<?php  } ?>

	</table>
	<br />
	<br />
	<center><input type="submit" value="<?php echo _('Add'); ?>" /></center>
	</form>

	</body>
	</html>

	<?php
}
else
{
	$ds = $_SESSION['ldap']->server();
	$add_res = @ldap_mod_add( $ds, $dn, array( 'objectClass' => $new_oclass ) );
	if( ! $add_res ) {
		echo $_SESSION['header'];
		
		echo "<title>LDAP Account Manager</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head>\n";
		StatusMessage('ERROR', _("Was unable to modify attribtues from DN: %s."), ldap_error( $ds ), array($dn));
		echo "</body></html>";
	}
	else
		header( "Location: edit.php?dn=$encoded_dn&amp;modified_attrs[]=objectClass" );

}

?>
