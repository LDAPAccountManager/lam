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
 * export_form.php
 * --------------------
 *
 * Html form to choose an export format(ldif,...)
 *
 * @package lists
 * @subpackage tree
 * @author The phpLDAPadmin development team
 * @author Roland Gruber
 */

/** export functions */
require '../../lib/export.inc';
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

$format = isset( $_GET['format'] ) ? $_GET['format'] : get_line_end_format();
$scope = isset( $_GET['scope'] ) ? $_GET['scope'] : 'base' ;
$exporter_id = isset( $_GET['exporter_id'] ) ? $_GET['exporter_id'] : 0 ;
$dn = isset( $_GET['dn'] ) ? $_GET['dn'] : null;
$filter = isset( $_GET['filter'] ) ? $_GET['filter'] : '(objectClass=*)';
$attributes = isset( $_GET['attributes'] ) ? $_GET['attributes'] : '*';
$sys_attr = isset( $_GET['sys_attr'] ) && $_GET['sys_attr'] == 'true' ? true : false;

$available_formats = array( 
	'unix' => 'UNIX (Linux, BSD)', 
	'mac'  => 'Macintosh', 
	'win'  => 'Windows'
);

$available_scopes = array(
	'base' => _('Base (base DN only)'),
	'one' => _('One (one level beneath base)'),
	'sub' => _('Sub (entire subtree)')
);


echo $_SESSION['header'];

echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
?>

  <body>
    <h3 class="tree_title"><?php echo _('Export'); ?></h3>
    <br />
    <center>
    <form name="export_form" action="export.php" method="POST">
      <table class="export_form">
        <tr>
	      <td>
            <fieldset>
	          <legend><?php echo _('Export'); ?></legend>
          <table>
          <tr>
            <td style="white-space:nowrap"><?php echo _('Base DN'); ?></td>
	        <td><nobr><input type="text" name="dn" id="dn" style="width:230px" value="<?php echo htmlspecialchars( $dn ); ?>" /></nobr></td>
          </tr>
	      <tr>
            <td><span style="white-space: nowrap"><?php echo _('Search scope'); ?></span></td>
            <td>
            <?php foreach( $available_scopes as $id => $desc ) {
            	$id = htmlspecialchars( $id );
            	$desc = htmlspecialchars( $desc ); ?>

            <input type="radio" name="scope" value="<?php echo $id; ?>" id="<?php echo $id; ?>"<?php if($id==$scope) echo ' checked="true"';?> /><label for="<?php echo $id; ?>"><?php echo $desc; ?></label><br />

            <?php } ?>
            </td>
          </tr>
          <tr>
            <td><?php echo _('Search filter'); ?></td>
            <td><input type="text" name="filter" style="width:300px" value="<?php echo htmlspecialchars($filter); ?>" /></td>
          </tr>
          <tr>
            <td>&nbsp;</td>
            <td><input type="checkbox" name="sys_attr" id="sys_attr" <?php if( $sys_attr ) echo 'checked="true" '; ?>/> <label for="sys_attr"><?php echo _('Include system attributes'); ?></label></td>
          </tr>
          <tr>
            <td>&nbsp;</td>
	        <td><input type="checkbox" id="save_as_file" name="save_as_file" /><label for="save_as_file"><?php echo _('Save as file'); ?></label></td>
          </tr>
        </table>
      </fieldset>
      </td>
    </tr>
    <tr>
      <td>
        <table style="width: 100%">
        <tr><td style="width: 50%">
        <fieldset style="height: 100px">
          <legend><?php echo _('Export format'); ?></legend>

            <?php foreach($exporters as $index => $exporter){?>

            <input type="radio"  name="exporter_id" value="<?php echo htmlspecialchars($index); ?>" id="<?php echo htmlspecialchars($index); ?>" <?php if($index==$exporter_id) echo ' checked="true"'; ?> />
            <label for="<?php echo htmlspecialchars( $index ); ?>"><?php echo htmlspecialchars( $exporter['desc'] ); ?></label><br />

            <?php } ?>

        </fieldset>
        </td>
        <td style="width: 50%">
        <fieldset style="height: 100px">
          <legend><?php echo _('Line ends'); ?></legend>
            <?php foreach( $available_formats as $id => $desc ) { 
            	$id = htmlspecialchars( $id );
            	$desc = htmlspecialchars( $desc );
            ?>	  
    
            <input type="radio" name="format" value="<?php echo $id; ?>"  id="<?php echo $id; ?>"<?php if($format==$id) echo ' checked="true"'; ?> /><label for="<?php echo $id; ?>"><?php echo $desc; ?></label><br />

            <?php } ?>
        </fieldset>
        </td></tr>
        </table>
	  </td>
	</tr>
    <tr>
      <td colspan="2">
	    <center>
          <input type="submit" name="target" value="<?php echo _('Submit'); ?>" />
	    </center>
	  </td>
    </tr>
  </table>
</form>
</center>
</body>
</html>

<?php

/**
 * Helper functoin for fetching the line end format.
 * @return String 'win', 'unix', or 'mac' based on the user's browser..
 */
function get_line_end_format()
{
    if( is_browser_os_windows() )
        return 'win';
    elseif( is_browser_os_unix() )
        return 'unix';
    elseif( is_browser_os_mac() )
        return 'mac';
    else
        return 'unix';
}
