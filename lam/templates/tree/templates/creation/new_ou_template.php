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
 * Creates new OUs.
 *
 * @package lists
 * @subpackage tree
 * @author David Smith
 * @author Roland Gruber
 */

// Common to all templates
$container = $_POST['container'];

// Unique to this template: which step of the ou creation process are we on
$step = isset( $_POST['step'] ) ? $_POST['step'] : 1;

?>

<center><h2><?PHP echo _('New organizational unit');?></h2></center>

<?php if( $step == 1 ) { ?>

<form action="creation_template.php" method="post" name="ou_form">
<input type="hidden" name="step" value="2" />
<input type="hidden" name="template" value="<?php echo htmlspecialchars( $_POST['template'] ); ?>" />

<center>
<table class="confirm">
<tr>
	<td></td>
	<td class="heading"><?PHP echo _('Name');?>:</td>
	<td><input type="text" name="ou_name" value="" /> <small><?PHP echo _('(hint: do not include "ou=")');?></small></td>
</tr>
<tr>
	<td></td>
	<td class="heading"><?PHP echo _("Container DN") ?>:</td>
	<td><input type="text" name="container" size="40" value="<?php echo htmlspecialchars( $container ); ?>" />
	</td>
</tr>
<tr>
	<td colspan="3"><center><br /><input type="submit" value="<?php echo _('Next'); ?>" /></center></td>
</tr>
</table>
</center>
</form>

<?php } elseif( $step == 2 ) {

	$ou_name = trim( $_POST['ou_name'] );
	$container = trim( $_POST['container'] );
	
	if (!dn_exists( $container )) {
		StatusMessage("ERROR", "The container you specified does not exist. ", $container );
		echo "</body></html>";
		exit();
	}

	?>
	<form action="create.php" method="post">
	<input type="hidden" name="new_dn" value="<?php echo htmlspecialchars( 'ou=' . $ou_name . ',' . $container ); ?>" />

	<!-- ObjectClasses  -->
	<?php $object_classes = rawurlencode( serialize( array( 'top', 'organizationalUnit' ) ) ); ?>

	<input type="hidden" name="object_classes" value="<?php echo $object_classes; ?>" />
		
	<!-- The array of attributes/values -->
	<input type="hidden" name="attrs[]" value="ou" />
		<input type="hidden" name="vals[]" value="<?php echo htmlspecialchars($ou_name);?>" />

	<center><?php echo _("Really create this new OU?") ?>
	<br />
	<br />

	<table class="confirm">
	<tr class="even"><td><?PHP echo _('Name'); ?></td><td><b><?php echo htmlspecialchars($ou_name); ?></b></td></tr>
	<tr class="odd"><td><?PHP echo _('Container'); ?></td><td><b><?php echo htmlspecialchars( $container ); ?></b></td></tr>
	</table>
	<br /><input type="submit" value="<?php echo _('Create'); ?>" />
	</center>

<?php } ?>

