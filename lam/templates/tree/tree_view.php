<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
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
* This file includes the main frame of the LDAP browser.
*
* @package lists
* @subpackage tree
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

// start session
startSecureSession();

// get encoding
$lang = explode(":",$_SESSION['language']);
$lang = $lang[1];

$dn = $_SESSION['config']->get_Suffix('tree');

// init tree
if (! isset($_SESSION['tree'])) {
	initialize_session_tree();
	$tree = $_SESSION['tree'];
	$tree_icons = $_SESSION['tree_icons'];
	$contents = get_container_contents($dn, 0, '(objectClass=*)');
	usort( $contents, 'pla_compare_dns' );
	$tree[$dn] = $contents;
	
	foreach( $contents as $c )
		$tree_icons[$c] = get_icon( $c );
	
	$_SESSION['tree'] = $tree;
	$_SESSION['tree_icons'] = $tree_icons;
}

echo "<?xml version=\"1.0\" encoding=\"$lang\"?>";
echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\" \"http://www.w3.org/TR/html4/frameset.dtd\">";
echo "<html>";
echo "<head>";
echo "<meta http-equiv=\"content-type\" content=\"text/html; charset=$lang\">";
echo "<meta http-equiv=\"pragma\" content=\"no-cache\">";
echo "<meta http-equiv=\"cache-control\" content=\"no-cache\">";
echo "<title>LDAP Account Manager</title>";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\"></head>";
echo "<frameset cols=\"320,*\">";
echo "<frame src=\"./tree.php\" name=\"left_frame\" frameborder=\"0\" scrolling=\"yes\" noresize>";
echo "<frame src=\"./edit.php?dn=$dn\" name=\"right_frame\" frameborder=\"0\" scrolling=\"yes\">";
echo "<noframes>";
echo "This page requires a browser that can show frames!";
echo "</noframes>";
echo "</frameset>";

echo "</html>";

?>