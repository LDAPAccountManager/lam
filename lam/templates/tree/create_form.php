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
 * The menu where the user chooses an RDN, Container, and Template for creating a new entry.
 * After submitting this form, the user is taken to their chosen Template handler.
 *
 * Variables that come in as GET vars
 *  - container (rawurlencoded) (optional)
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
/** template configuration */
include_once('templates/templates.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

$step = isset( $_REQUEST['step'] ) ? $_REQUEST['step'] : 1; // defaults to 1
$container = $_REQUEST['container'];


echo $_SESSION['header'];

echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";
?>

<body>

<h3 class="tree_title"><?php echo _('Create Object')?></h3>
<h3 class="tree_subtitle"><?php echo _('Choose a template')?></h3>
	<center><h3><?php echo _('Select a template for the creation process')?></h3></center>
	<form action="creation_template.php" method="post">
	<input type="hidden" name="container" value="<?php echo htmlspecialchars( $container ); ?>" />
	<table class="tree_create">
	<tr>
      <td class="heading">
        <?php echo _('Template'); ?>:
      </td>
      <td>
        <table class="template_display">
          <tr>
            <td>
              <table class="templates">
                <?php
                $count = count( $templates );
                $i = -1;
                foreach( $templates as $name => $template ) {
                    $i++;
                    // Balance the columns properly
                    if( ( count( $templates ) % 2 == 0 && $i == intval( $count / 2 ) ) ||
                            ( count( $templates ) % 2 == 1 && $i == intval( $count / 2 ) + 1 ) )
                        echo "</table></td><td><table class=\"templates\">";
     				// Check and see if this template should be shown in the list
     				$isValid = false;
     				if( isset($template['regexp'] ) ) {
     					if( @preg_match( "/".$template['regexp']."/i", $container ) ) {
     						$isValid = true;
     					}
     				} else {
     					$isValid = true;
     				}
     
     				?>
     				<tr>
                      <td><input type="radio" name="template" value="<?php echo htmlspecialchars($name);?>" 
                                 id="<?php echo htmlspecialchars($name); ?>"
                                <?php if( 0 == strcasecmp( 'custom.php', $template['handler'] ) ) echo ' checked';
                                if( ! $isValid ) echo ' disabled'; ?> />
                                </td>
                      <td class="icon"><label for="<?php echo htmlspecialchars($name);?>"><img src="<?php echo $template['icon']; ?>" /></label></td>
                      <td>
                        <label for="<?php echo htmlspecialchars($name);?>">
                        <?php if( 0 == strcasecmp( 'Custom', $template['desc'] ) ) echo '<b>';
                              if( ! $isValid ) echo "<span style=\"color: gray\"><acronym title=\"This template is not allowed in this container\">";
                              echo htmlspecialchars( $template['desc'] ); 
                              if( ! $isValid ) echo "</acronym></span>";
                              if( 0 == strcasecmp( 'Custom', $template['desc'] ) ) echo '</b>'; ?>
                        </label></td>
                    </tr>
                  <?php 
             }
             ?>

			</table>
            </td>
            </tr>
            </table>
		</td>
	</tr>

	<tr>
		<td colspan="2"><center><input type="submit" name="submit" value="<?php echo _('Next')?>" /></center></td>
	</tr>

	</table>

	</form>

</body>
</html>
