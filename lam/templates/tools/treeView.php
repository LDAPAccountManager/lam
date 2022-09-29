<?php
namespace LAM\TOOLS\TREEVIEW;
use htmlDiv;
use htmlForm;
use htmlJavaScript;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlGroup;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021 - 2022  Roland Gruber

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

$roots = TreeViewTool::getRootDns();
if (empty($roots)) {
	StatusMessage('ERROR', _('Please configure the tree suffix in your LAM server profile settings.'));
}
else {
	showTree();
}

echo '</div>';
include __DIR__ . '/../../lib/adminFooter.inc';

function showTree(): void {
	$openInitial = array();
	if (isset($_GET['dn'])) {
		$initialDn = base64_decode($_GET['dn']);
		$roots = TreeViewTool::getRootDns();
		foreach ($roots as $rootDn) {
			if ((strlen($initialDn) > strlen($rootDn)) && substr($initialDn, -1 * strlen($rootDn)) === $rootDn) {
				$extraDnPart = substr($initialDn, 0, (-1 * strlen($rootDn)) - 1);
				$dnParts = ldap_explode_dn($extraDnPart, 0);
				if ($dnParts !== false) {
					unset($dnParts['count']);
					$dnPartsCount = sizeof($dnParts);
					for ($i = 0; $i < $dnPartsCount; $i++) {
						$currentParts = array_slice($dnParts, $dnPartsCount - ($i + 1));
						$openInitial[] = '"' . base64_encode(implode(',', $currentParts) . ',' . $rootDn) . '"';
					}
				}
			}
		}
	}
	$openInitialJsArray = '[' . implode(', ', $openInitial) . ']';
	$row = new htmlResponsiveRow();
	$row->setCSSClasses(array('maxrow'));
	$row->add(new htmlDiv('ldap_tree', new htmlOutputText(''), array('tree-view--tree')), 12, 5, 5, 'tree-left-area');
	$row->add(new htmlDiv('ldap_actionarea', new htmlOutputText(''), array('tree-view--actionarea')), 12, 7, 7, 'tree-right-area');
	$newMenu = '';
	if (checkIfWriteAccessIsAllowed()) {
		$newMenu = '"createNode": {
								"label": "' . _('Create a child entry') . '",
								"icon": "../../graphics/add.svg",
								"action": function(obj) {
									window.lam.treeview.createNode("' . getSecurityTokenName() . '",
										"' . getSecurityTokenValue() . '",
										node,
										tree)
								}
							},';
	}
	$copyMenu = '';
	if (checkIfWriteAccessIsAllowed()) {
		$copyMenu = '"copyNode": {
								"separator_before": true,
								"label": "' . _('Copy') . '",
								"icon": "../../graphics/copy.svg",
								"action": function(obj) {
									window.lam.treeview.copyNode(node,
										tree)
								}
							},';
	}
	$cutMenu = '';
	if (checkIfWriteAccessIsAllowed()) {
		$cutMenu = '"cutNode": {
								"label": "' . _('Cut') . '",
								"icon": "../../graphics/cut.svg",
								"action": function(obj) {
									window.lam.treeview.cutNode(node,
										tree)
								}
							},';
	}
	$pasteMenu = '';
	if (checkIfWriteAccessIsAllowed()) {
		$pasteMenu = '"pasteNode": {
								"separator_after": true,
								"_disabled": window.lam.treeview.contextMenuPasteDisabled,
								"label": "' . _('Paste') . '",
								"icon": "../../graphics/paste.svg",
								"action": function(obj) {
									window.lam.treeview.pasteNode("' . getSecurityTokenName() . '",
										"' . getSecurityTokenValue() . '",
										node,
										tree)
								}
							},';
	}
	$deleteMenu = '';
	if (checkIfWriteAccessIsAllowed()) {
		$deleteMenu = '"deleteNode": {
								"label": "' . _('Delete') . '",
								"icon": "../../graphics/del.svg",
								"action": function(obj) {
									window.lam.treeview.deleteNode("' . getSecurityTokenName() . '",
										"' . getSecurityTokenValue() . '",
										node,
										tree,
										"' . _('Delete') . '",
										"' . _('Cancel') . '",
										"' . _('Delete this entry') . '",
										"' . _('Ok') . '",
										"' . _('Error') . '")
								}
							},';
	}
	$exportMenu = '';
	if ($_SESSION['config']->isToolActive('ImportExport')) {
		$exportMenu = '"exportNode": {
								"label": "' . _('Export') . '",
								"icon": "../../graphics/export.svg",
								"action": function(obj) {
									window.location.href = "../tools/importexport.php?tab=export&dn=" + node.id;
								}
							},';
	}
	$refreshMenu = '"refreshNode": {
								"label": "' . _('Refresh') . '",
								"icon": "../../graphics/refresh.svg",
								"action": function(obj) {
									tree.refresh_node(node);
									window.lam.treeview.getNodeContent("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '", node.id);
								}
							},';
	$searchMenu = '"search": {
								"label": "' . _('Search') . '",
								"icon": "../../graphics/search.svg",
								"action": function(obj) {
									window.lam.treeview.search("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '", node.id);
								}
							},';
	$treeScript = new htmlJavaScript('
		jQuery(document).ready(function() {
			var maxHeight = jQuery(document).height() - jQuery("#ldap_tree").offset().top - 50;
			jQuery("#ldap_tree").css("max-height", maxHeight);
			jQuery("#ldap_actionarea").css("max-height", maxHeight);
			window.lam.treeview.contextMenuPasteDisabled = true;
			jQuery(\'#ldap_tree\').jstree({
				"plugins": [
					"contextmenu",
					"changed"
				],
				"contextmenu": {
					"select_node": false,
					"items": function(node) {
						var tree = jQuery.jstree.reference("#ldap_tree");
						var menuItems = {' .
							$newMenu .
							$deleteMenu .
							$refreshMenu .
							$copyMenu .
							$cutMenu .
							$pasteMenu .
							$searchMenu .
							$exportMenu .
						'};
						return menuItems;
					}
				},
				"core": {
					"worker": false,
					"strings": {
						"Loading ...": "' . _('Loading') . '"
					},
					"data": function(node, callback) {
						window.lam.treeview.getNodes("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '", node, callback);
					}
				}
			})
			.on("changed.jstree", function (e, data) {
				if (data && data.action && (data.action == "select_node")) {
					var node = data.node;
					window.lam.treeview.getNodeContent("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '", node.id);
				}
			})
			.on("ready.jstree", function (e, data) {
				var tree = jQuery.jstree.reference("#ldap_tree");
				window.lam.treeview.openInitial(tree, ' . $openInitialJsArray . ');
			});
		});
	');
	$row->add($treeScript, 12);

	$deleteDialogContent = new htmlResponsiveRow();
	$deleteDialogContent->add(new htmlOutputText(_('Do you really want to delete this entry?')), 12);
	$deleteDialogContent->addVerticalSpacer('0.5rem');
	$deleteDialogEntryText = new htmlOutputText('');
	$deleteDialogEntryText->setCSSClasses(array('treeview-delete-entry'));
	$deleteDialogContent->add($deleteDialogEntryText, 12);
	$deleteDialogDiv = new htmlDiv('treeview_delete_dlg', $deleteDialogContent, array('hidden'));
	$row->add($deleteDialogDiv, 12);

	$errorDialogContent = new htmlResponsiveRow();
	$errorDialogEntryTitle = new htmlOutputText('');
	$errorDialogEntryTitle->setCSSClasses(array('treeview-error-title'));
	$errorDialogContent->add($errorDialogEntryTitle, 12);
	$errorDialogEntryText = new htmlOutputText('');
	$errorDialogEntryText->setCSSClasses(array('treeview-error-text'));
	$errorDialogContent->add($errorDialogEntryText, 12);
	$errorDialogDiv = new htmlDiv('treeview_error_dlg', $errorDialogContent, array('hidden'));
	$row->add($errorDialogDiv);

	$pwdCheckRow = new htmlResponsiveRow();
	$pwdCheckInput = new htmlResponsiveInputField(_('Password'), 'lam_pwd_check');
	$pwdCheckInput->setIsPassword(true);
	$pwdCheckInput->setCSSClasses(array('lam_pwd_check'));
	$pwdCheckRow->add($pwdCheckInput);
	$pwdCheckRow->addVerticalSpacer('1rem');
	$pwdCheckRow->add(new htmlDiv('lam-pwd-check-dialog-result', new htmlGroup()));
	$pwdCheckDiv = new htmlDiv('lam-pwd-check-dialog', $pwdCheckRow, array('hidden'));
	$row->add($pwdCheckDiv);

	$row->add(new htmlJavaScript('jQuery(document).ready(function() {
					jQuery(\'form[name="actionarea"]\').validationEngine({promptPosition: "topLeft", addFailureCssClassToField: "lam-input-error", autoHidePrompt: true, autoHideDelay: 5000});
				});'), 12);

	$tabIndex = 1;
	$form = new htmlForm('actionarea', 'treeView.php', $row);
	parseHtml(null, $form, array(), true, $tabIndex, null);
}
