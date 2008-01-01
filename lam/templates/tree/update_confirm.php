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
 * Takes the results of clicking "Save" in edit.php and determines which 
 * attributes need to be updated (ie, which ones actually changed). Then,
 * we present a confirmation table to the user outlining the changes they
 * are about to make. That form submits directly to update.php, which 
 * makes the change.
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
/** common functions */
include_once('../../lib/account.inc');

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

$dn = $_POST['dn'];
$encoded_dn = rawurlencode( $dn );
$rdn = get_rdn( $dn );
$old_values = $_POST['old_values'];
$new_values = $_POST['new_values'];
$mkntPassword = NULL;
$samba_password_step = 0;

echo $_SESSION['header'];

echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
?>
<body>
<h3 class="tree_title"><?php echo htmlspecialchars( ( $rdn ) ); ?></h3>
<h3 class="tree_subtitle"><?php echo _('DN'); ?>: <b><?php echo htmlspecialchars( ( $dn ) ); ?></b></h3>
<?php
$update_array = array();
foreach( $old_values as $attr => $old_val )
{
	// Did the user delete the field?
	if( ! isset( $new_values[ $attr ] ) ) {
		$update_array[ $attr ] = '';
	}
	// did the user change the field?
	elseif( $old_val != $new_values[ $attr ] ) {

		$new_val = $new_values[ $attr ];

		// special case for userPassword attributes
		if( 0 == strcasecmp( $attr, 'userPassword' ) && $new_val != '' ) {
		  $new_val = pwd_hash($new_val, true, $_POST['enc_type'] );
		  $password_already_hashed = true;
		}
		// special case for samba password
		else if (( 0 == strcasecmp($attr,'sambaNTPassword') || 0 == strcasecmp($attr,'sambaLMPassword')) && trim($new_val[0]) != '' ){
			if ( 0 == strcasecmp($attr,'sambaNTPassword')) {
				$new_val = ntPassword($new_val[0]);
			}
			else {
				$new_val = lmPassword($new_val[0]);
			}
		}
		$update_array[ $attr ] = $new_val;
	}
}

// special case check for a new enc_type for userPassword (not otherwise detected)
if(	isset( $_POST['enc_type'] ) && 
    ! isset( $password_already_hashed ) &&
	$_POST['enc_type'] != $_POST['old_enc_type'] && 
	$_POST['enc_type'] != 'clear' &&
	$_POST['new_values']['userpassword'] != '' ) {

	$new_password = pwd_hash( $_POST['new_values']['userpassword'], true, $_POST['enc_type'] );
	$update_array[ 'userpassword' ] = $new_password;
}

// strip empty vals from update_array and ensure consecutive indices for each attribute
foreach( $update_array as $attr => $val ) {
	if( is_array( $val ) ) {
		foreach( $val as $i => $v )
			if( null == $v || 0 == strlen( $v ) )
				unset( $update_array[$attr][$i] );
		$update_array[$attr] = array_values( $update_array[$attr] );
	}
}

// at this point, the update_array should look like this (example):
// Array (
//    cn => Array( 
//           [0] => 'Dave',
//           [1] => 'Bob' )
//    sn => 'Smith',
//    telephoneNumber => '555-1234' )
//  This array should be ready to be passed to ldap_modify()

?>
<?php if( count( $update_array ) > 0 ) { ?>

	<br />
	<center>
	<?php echo _('Do you want to make these changes?'); ?>
	<br />
	<br />

	<table class="tree_confirm">
	<tr>
		<th><?php echo _('Attribute'); ?></th>
		<th><?php echo _('Old value'); ?></th>
		<th><?php echo _('New value'); ?></th>
	</tr>

	<?php $counter=0; foreach( $update_array as $attr => $new_val ) { $counter++ ?>
	
		<tr class="<?php echo $counter%2 ? 'even' : 'odd'; ?>">
		<td><b><?php echo htmlspecialchars( $attr ); ?></b></td>
		<td><nobr>
		<?php
		if( is_array( $old_values[ $attr ] ) ) 
			foreach( $old_values[ $attr ] as $v )
				echo nl2br( htmlspecialchars( $v ) ) . "<br />";
		else  
			if( 0 == strcasecmp( $attr, 'userPassword' ) && ( is_null( get_enc_type( $old_values[ $attr ] ) ) ) ) {
				echo preg_replace( '/./', '*', $old_values[ $attr ] ) . "<br />";
			}
			else {
				echo nl2br( htmlspecialchars( $old_values[ $attr ] ) ) . "<br />";
			}
		echo "</nobr></td><td><nobr>";

		// is this a multi-valued attribute?
		if( is_array( $new_val ) ) {
			foreach( $new_val as $i => $v ) {
				if( $v == '' ) {
					// remove it from the update array if it's empty
					unset( $update_array[ $attr ][ $i ] );
					$update_array[ $attr ] = array_values( $update_array[ $attr ] );
				} else {
					echo nl2br( htmlspecialchars( $v ) ) . "<br />";
				}
			}

			// was this a multi-valued attribute deletion? If so,
			// fix the $update_array to reflect that per update_confirm.php's
			// expectations
			if( $update_array[ $attr ] == array( 0 => '' ) || $update_array[ $attr ] == array() ) {
				$update_array[ $attr ] = '';
				echo '<span style="color: red">' . _('[attribute deleted]') . '</span>';
			}
		}
		else 
			if( $new_val != '' ) 
				if( 0 == strcasecmp( $attr, 'userPassword' ) && ( is_null( get_enc_type( $new_values[ $attr ] ) ) ) ) {
					echo preg_replace( '/./', '*', $new_val ) . "<br />";
				}
				else {
					echo htmlspecialchars( $new_val ) . "<br />";
				}
			else 
				echo '<span style="color: red">' . _('[attribute deleted]') . '</span>';
		echo "</nobr></td></tr>\n\n";
	}

	?>

	</table>
	<br />

	<table>
	<tr>
		<td>
			<!-- Commit button and acompanying form -->
			<form action="update.php" method="post">
			<input type="hidden" name="dn" value="<?php echo $dn; ?>" />
			<?php foreach( $update_array as $attr => $val ) { ?>
				<?php if( is_array( $val ) ) { ?>				
					<?php foreach( $val as $i => $v ) { ?>

						<input  type="hidden"
							name="update_array[<?php echo htmlspecialchars( $attr ); ?>][<?php echo $i; ?>]"
							value="<?php echo htmlspecialchars( $v ); ?>" />
					<?php } ?> 
				<?php } else { ?>				

					<input  type="hidden"
						name="update_array[<?php echo htmlspecialchars( $attr ); ?>]"
						value="<?php echo htmlspecialchars( $val ); ?>" />
				<?php } ?>				
			<?php } ?>
			<input type="submit" value="<?php echo _('Commit'); ?>"/>
			</form>
		</td>
		<td>
			<!-- Cancel button -->
			<form action="edit.php" method="get">
			<input type="hidden" name="dn" value="<?php echo $dn; ?>" />
			<input type="submit" value="<?php echo _('Cancel'); ?>"/>
			</form>
		</td>
	</tr>
	</table>		
	</center>
	</body>

	<?php

} else { ?>
	
	<center>
	<?php echo _('You made no changes.'); ?>
	<br><br><a href="edit.php?dn=<?php echo $encoded_dn; ?>"><?php echo _('Back'); ?></a>
	</center>

<?php } ?>

</form>



