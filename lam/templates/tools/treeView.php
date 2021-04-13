<?php
namespace LAM\TOOLS\TREEVIEW;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021  Roland Gruber

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

use htmlDiv;
use htmlJavaScript;
use htmlOutputText;
use htmlResponsiveRow;

/**
* LDAP tree view.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");
/** access to configuration options */
include_once(__DIR__ . "/../../lib/config.inc");

// start session
startSecureSession();
enforceUserIsLoggedIn();
validateSecurityToken();

checkIfToolIsActive('TreeViewTool');

setlanguage();

include __DIR__ . '/../../lib/adminHeader.inc';
echo '<link rel="stylesheet" href="../../style/jstree/style.css" />';
echo '<script src="../lib/extra/jstree/jstree.js"></script>';
echo '<div class="smallPaddingContent">';

$toolSettings = $_SESSION['config']->getToolSettings();

if (empty($toolSettings[TreeViewTool::TREE_SUFFIX_CONFIG][0])) {
	StatusMessage('ERROR', _('Please configure the tree suffix in your LAM server profile settings.'));
}
else {
	showTree();
}

echo '</div>';
include __DIR__ . '/../../lib/adminFooter.inc';

function showTree() {
	$toolSettings = $_SESSION['config']->getToolSettings();
	$rootDn = $toolSettings[TreeViewTool::TREE_SUFFIX_CONFIG];
	$row = new htmlResponsiveRow();
	$row->add(new htmlDiv('ldap_tree', new htmlOutputText('')), 12);
	$treeScript = new htmlJavaScript('
		jQuery(document).ready(function() {
			jQuery(\'#ldap_tree\').jstree({
				"plugins": [
					"contextmenu"
				],
				"contextmenu": {
					"items": function(node) {
						var tree = jQuery.jstree.reference("#ldap_tree");
						return {
							"refresh": {
								"label": "' . _('Refresh') . '",
								"icon": "../../graphics/refresh.png",
								"action": function(obj) {
									tree.refresh_node(node);
								}
							}
						};
					}
				},
				"core": {
					"worker": false,
					"strings": {
						"Loading ...": "' . _('Loading') . '"
					},
					"data": function(node, callback) {
						var data = {
							jsonInput: ""
						};
						data["' . getSecurityTokenName() . '"] = "' . getSecurityTokenValue() . '";
						data["dn"] = btoa(node.id),
						jQuery.ajax({
							url: "../misc/ajax.php?function=treeview&command=getNodes",
							method: "POST",
							data: data
						})
						.done(function(jsonData) {
							callback.call(this, jsonData);
						})
					}
				}
			});
		});
	');
	$row->add($treeScript, 12);

	$tabIndex = 1;
	parseHtml(null, $row, array(), true, $tabIndex, 'none');
}
