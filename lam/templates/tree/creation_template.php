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
 * This file simply acts as a plugin grabber for the creator templates in
 * the directory templates/creation/
 *
 * Expected POST vars:
 *  template
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
/** template configuration */
include_once('templates/templates.inc');

// start session
startSecureSession();

setlanguage();

$template = $_POST['template'];

if( $template == 'custom' ) {
    foreach( $templates as $id => $template ) {
        if( $template['handler'] == 'custom.php' ) {
            $template = $id;
            break;
        }
    }
}

$template_id = $template;
$template = isset( $templates[$template] ) ? $templates[$template_id] : null;
$ds = $_SESSION['ldap']->server;

echo $_SESSION['header'];

echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";

?>

<body>
<h3 class="tree_title"><?php echo _('Create Object')?></h3>
<h3 class="tree_subtitle"><?php echo _('Using template:')?> '<?php echo htmlspecialchars( $template['desc'] ); ?>'</h3>

<?php

$handler = 'templates/creation/' . $template['handler'];
$handler = realpath( $handler );

include $handler;

echo "</body>\n</html>";

?>
