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
 * Displays a last chance confirmation form to delete a dn.
 *
 * Variables that come in as GET vars:
 *  - dn (rawurlencoded)
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

$dn = $_GET['dn'];
$encoded_dn = rawurlencode( $dn );
$rdn = pla_explode_dn( $dn );
$rdn = $rdn[0];

$children = get_container_contents( $dn,0,'(objectClass=*)',LDAP_DEREF_NEVER );
$has_children = count($children)>0 ? true : false;

echo $_SESSION['header'];

echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
?>

<body>

<h3 class="tree_title"><?php echo sprintf( _('Delete %s'), htmlspecialchars( $rdn ) ); ?></b></h3>
<h3 class="tree_subtitle"><?php echo _('DN'); ?>: <b><?php echo htmlspecialchars( ( $dn ) ); ?></b></h3>

<?php  if( $has_children ) { ?>

<center><b><?php echo _('Permanently delete all children, too?'); ?></b><br /><br />

<?php
flush(); // so the user can get something on their screen while we figure out how many children this object has
if( $has_children ) {
	// get the total number of child objects (whole sub-tree)
	$s = pla_ldap_search( 'objectClass=*', $dn, array('dn'), 'sub' );
	$sub_tree_count = count( $s );
}
?>

<table class="tree_delete_confirm">
<td>

<p>
<?php echo sprintf( _('This entry is the root of a sub-tree containing %s entries.'), $sub_tree_count ); ?> 
<br />
<br />

<?php echo sprintf( _('LAM can recursively delete this entry and all of its children. See below for a list of all the entries that this action will delete. Do you want to do this?'), ($sub_tree_count-1) ); ?><br />
<br />

<br />
<br />
<table width="100%">
<tr>
	<td>
	<center>
	<form action="rdelete.php" method="post">
	<input type="hidden" name="dn" value="<?php echo $dn; ?>" />
	<input type="submit" value="<?php echo sprintf( _('Delete all %s objects'), $sub_tree_count ); ?>" />
	</form>
	</td>
	
	<td>
	<center>
	<form action="edit.php" method="get">
	<input type="hidden" name="dn" value="<?php echo htmlspecialchars($dn); ?>" />
	<input type="submit" name="submit" value="<?php echo _('Cancel'); ?>"/>
	</form>
	</center>
	</td>
</tr>
</table>
</td>
</table>
<?php flush(); ?>
<br />
<br />
<?php echo _('List of entries to be deleted:'); ?><br />
<select size="<?php echo min( 10, $sub_tree_count );?>" multiple disabled style="background:white; color:black;width:500px" >
<?php $i=0; ?>
<?php foreach( $s as $dn => $junk ) { ?>
	<?php $i++; ?>
	<option><?php echo $i; ?>. <?php echo htmlspecialchars( ( $dn ) ); ?></option>
<?php } ?>

</select>

<br />

<?php  } else { ?>

<center>

<table class="tree_delete_confirm">
<td>

<?php echo _('Are you sure you want to permanently delete this object?'); ?><br />
<br />
<nobr><acronym><?php echo _('DN'); ?></acronym>:  <b><?php echo pretty_print_dn( $dn ); ?></b><nobr><br />
<br />
<table width="100%">
<tr>
	<td>
	<center>
	<form action="delete.php" method="post">
	<input type="hidden" name="dn" value="<?php echo htmlspecialchars($dn); ?>" />
	<input type="submit" name="submit" value="<?php echo _('Delete'); ?>"/>
	</center>
	</form>
	</td>
	
	<td>
	<center>
	<form action="edit.php" method="get">
	<input type="hidden" name="dn" value="<?php echo $dn; ?>" />
	<input type="submit" name="submit" value="<?php echo _('Cancel'); ?>"/>
	</form>
	</center>
	</td>
</tr>
</table>

</td>
</table>

</center>

<?php  } ?>

</body>

</html>
