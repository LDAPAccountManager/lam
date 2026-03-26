<?php
namespace LAM\TOOLS\TREEVIEW;
use htmlDiv;
use htmlForm;
use htmlInputField;
use htmlJavaScript;
use htmlOutputText;
use htmlResponsiveInputField;
use htmlResponsiveRow;
use htmlGroup;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2021 - 2025  Roland Gruber

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
echo '<link rel="stylesheet" href="../../style/wunderbaum/wunderbaum.css" />';
echo '<script src="../lib/extra/wunderbaum/wunderbaum.umd.js"></script>';
echo '<link rel="stylesheet" href="../../style/bootstrap-icons/bootstrap-icons.css" />';
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
	$openInitial = [];
	if (isset($_GET['dn'])) {
		$initialDn = base64_decode($_GET['dn']);
		$roots = TreeViewTool::getRootDns();
		foreach ($roots as $rootDn) {
			if ((strlen($initialDn) > strlen($rootDn)) && str_ends_with($initialDn, $rootDn)) {
				$extraDnPart = substr($initialDn, 0, (-1 * strlen($rootDn)) - 1);
				$dnParts = ldap_explode_dn($extraDnPart, 0);
				if ($dnParts !== false) {
					unset($dnParts['count']);
					$dnPartsCount = count($dnParts);
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
	$row->setCSSClasses(['maxrow']);
	$row->add(new htmlDiv('ldap_tree', new htmlOutputText(''), ['tree-view--tree']), 12, 4, 4, 'tree-left-area');
	$row->add(new htmlDiv('ldap_actionarea', new htmlOutputText(''), ['tree-view--actionarea']), 12, 8, 8, 'tree-right-area');
	$treeScript = new htmlJavaScript('
		window.lam.utility.documentReady(function() {
			window.sessionStorage.removeItem("LAM_COPY_PASTE_ACTION");
			window.sessionStorage.removeItem("LAM_COPY_PASTE_DN");
			var maxHeight = document.documentElement.scrollHeight - (document.querySelector("#ldap_tree").getBoundingClientRect().top - window.scrollY) - 50;
			document.getElementById("ldap_tree").style.maxHeight = maxHeight;
			document.getElementById("ldap_actionarea").style.maxHeight = maxHeight;
			const tree = new mar10.Wunderbaum({
				element: document.getElementById("ldap_tree"),
				id: "ldap_tree",
				strings: {
					loading: "' . addslashes(_('Loading')) . '",
					loadError: "' . addslashes(_('Error')) . '",
					noData: "' . addslashes(_('No objects found!')) . '"
				},
				debugLevel: 2,
				source: "../misc/ajax.php?function=treeview&command=getRootNodes",
				init: (e) => {
					tree.expandAll(true, {depth: 1, });
					window.lam.treeview.openInitial(tree, ' . $openInitialJsArray . ');
				},
				lazyLoad: function(e) {
					return {url: "../misc/ajax.php?function=treeview&command=getNodes&dn=" + e.node.key};
				},
				iconBadge: (e) => {
					if (e.node.data.badge) {
						const badgeSpan = document.createElement("span");
						badgeSpan.className = "tree-badge";
						const badgeImg = document.createElement("img");
						badgeImg.src = e.node.data.badge;
						badgeSpan.appendChild(badgeImg);
						return badgeSpan;
					}
				},
				activate: function(e) {
					const node = e.node;
					window.lam.treeview.getNodeContent("' . getSecurityTokenName() . '", "' . getSecurityTokenValue() . '", node.key);
				}
			  });
		});
	');
	$row->add($treeScript);

	$deleteDialogContent = new htmlResponsiveRow();
	$deleteDialogContent->add(new htmlOutputText(_('Do you really want to delete this entry?')));
	$deleteDialogContent->addVerticalSpacer('0.5rem');
	$deleteDialogEntryText = new htmlOutputText('');
	$deleteDialogEntryText->setCSSClasses(['treeview-delete-entry']);
	$deleteDialogContent->add($deleteDialogEntryText);
	$deleteDialogDiv = new htmlDiv('treeview_delete_dlg', $deleteDialogContent, ['hidden']);
	$row->add($deleteDialogDiv);

	$restoreDialogContent = new htmlResponsiveRow();
	$restoreDialogContent->add(new htmlOutputText(_('Do you really want to restore this entry?')));
	$restoreDialogContent->addVerticalSpacer('0.5rem');
	$restoreDialogEntryText = new htmlOutputText('');
	$restoreDialogEntryText->setCSSClasses(['treeview-restore-entry']);
	$restoreDialogContent->add($restoreDialogEntryText);
	$restoreDialogDiv = new htmlDiv('treeview_restore_dlg', $restoreDialogContent, ['hidden']);
	$row->add($restoreDialogDiv);

	$errorDialogContent = new htmlResponsiveRow();
	$errorDialogEntryTitle = new htmlOutputText('');
	$errorDialogEntryTitle->setCSSClasses(['treeview-error-title']);
	$errorDialogContent->add($errorDialogEntryTitle);
	$errorDialogContent->addVerticalSpacer('0.5rem');
	$errorDialogEntryText = new htmlOutputText('');
	$errorDialogEntryText->setCSSClasses(['treeview-error-text']);
	$errorDialogContent->add($errorDialogEntryText);
	$errorDialogDiv = new htmlDiv('treeview_error_dlg', $errorDialogContent, ['hidden']);
	$row->add($errorDialogDiv);

	$pwdCheckRow = new htmlResponsiveRow();
	$pwdCheckInput = new htmlResponsiveInputField(_('Password'), 'lam_pwd_check');
	$pwdCheckInput->setIsPassword(true);
	$pwdCheckInput->setCSSClasses(['lam_pwd_check']);
	$pwdCheckRow->add($pwdCheckInput);
	$pwdCheckRow->addVerticalSpacer('1rem');
	$pwdCheckRow->add(new htmlDiv('lam-pwd-check-dialog-result', new htmlGroup()));
	$pwdCheckDiv = new htmlDiv('lam-pwd-check-dialog', $pwdCheckRow, ['hidden']);
	$row->add($pwdCheckDiv);

	$form = new htmlForm('actionarea', 'treeView.php', $row);
	parseHtml(null, $form, [], true);
}
