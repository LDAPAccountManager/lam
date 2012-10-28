<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2005 - 2012  Roland Gruber

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
* This is the main window of the pdf structure editor.
*
* @author Michael Duergner
* @author Roland Gruber
* @package PDF
*/

/** security functions */
include_once("../../lib/security.inc");
/** access to PDF configuration files */
include_once("../../lib/pdfstruct.inc");
/** LDAP object */
include_once("../../lib/ldap.inc");
/** for language settings */
include_once("../../lib/config.inc");
/** module functions */
include_once("../../lib/modules.inc");

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

checkIfToolIsActive('toolPDFEditor');

setlanguage();

// Unset pdf structure definitions in session if set
if(isset($_SESSION['currentPDFStructure'])) {
	unset($_SESSION['currentPDFStructure']);
	unset($_SESSION['availablePDFFields']);
	unset($_SESSION['currentPageDefinitions']);
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// check if new template should be created
if(isset($_POST['createNewTemplate'])) {
	metaRefresh('pdfpage.php?type=' . htmlspecialchars($_POST['scope']));
	exit();
}

$scopes = $_SESSION['config']->get_ActiveTypes();
$sortedScopes = array();
for ($i = 0; $i < sizeof($scopes); $i++) {
	$sortedScopes[$scopes[$i]] = getTypeAlias($scopes[$i]);
}
natcasesort($sortedScopes);

$container = new htmlTable();
$container->addElement(new htmlTitle(_('PDF editor')), true);

if (isset($_POST['deleteProfile']) && ($_POST['deleteProfile'] == 'true')) {
	// delete structure
	if (deletePDFStructureDefinition($_POST['profileDeleteType'], $_POST['profileDeleteName'])) {
		$message = new htmlStatusMessage('INFO', _('Deleted PDF structure.'), getTypeAlias($_POST['profileDeleteType']) . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$message->colspan = 10;
		$container->addElement($message, true);
	}
	else {
		$message = new htmlStatusMessage('ERROR', _('Unable to delete PDF structure!'), getTypeAlias($_POST['profileDeleteType']) . ': ' . htmlspecialchars($_POST['profileDeleteName']));
		$message->colspan = 10;
		$container->addElement($message, true);
	}
}

if (isset($_POST['importexport']) && ($_POST['importexport'] === '1')) {
	$cfg = new LAMCfgMain();
	$impExpMessage = null;
	if (isset($_POST['importProfiles_' . $_POST['scope']])) {
		// check master password
		if (!$cfg->checkPassword($_POST['passwd_' . $_POST['scope']])) {
			$impExpMessage = new htmlStatusMessage('ERROR', _('Master password is wrong!'));
		}
		elseif (copyPdfProfiles($_POST['importProfiles_' . $_POST['scope']], $_POST['scope'])) {
			$impExpMessage = new htmlStatusMessage('INFO', _('Import successful'));
		}
	} else if (isset($_POST['exportProfiles'])) {
		// check master password
		if (!$cfg->checkPassword($_POST['passwd'])) {
			$impExpMessage = new htmlStatusMessage('ERROR', _('Master password is wrong!'));
		}
		elseif (copyPdfProfiles($_POST['exportProfiles'], $_POST['scope'], $_POST['destServerProfiles'])) {
			$impExpMessage = new htmlStatusMessage('INFO', _('Export successful'));
		}
	}
	if ($impExpMessage != null) {
		$impExpMessage->colspan = 10;
		$container->addElement($impExpMessage, true);
	}
}

// get list of account types
$availableScopes = '';
$templateClasses = array();
foreach ($sortedScopes as $scope => $title) {
	$templateClasses[] = array(
		'scope' => $scope,
		'title' => $title,
		'templates' => "");
	$availableScopes[$title] = $scope;
}
// get list of templates for each account type
for ($i = 0; $i < sizeof($templateClasses); $i++) {
	$templateClasses[$i]['templates'] = getPDFStructureDefinitions($templateClasses[$i]['scope']);
}

// check if a template should be edited
for ($i = 0; $i < sizeof($templateClasses); $i++) {
	if (isset($_POST['editTemplate_' . $templateClasses[$i]['scope']]) || isset($_POST['editTemplate_' . $templateClasses[$i]['scope'] . '_x'])) {
		metaRefresh('pdfpage.php?type=' . htmlspecialchars($templateClasses[$i]['scope']) . '&edit=' . htmlspecialchars($_POST['template_' . $templateClasses[$i]['scope']]));
		exit;
	}
}

include '../main_header.php';
?>
<div class="userlist-bright smallPaddingContent">
<form action="pdfmain.php" method="post" name="pdfmainForm" >
	<div id="passwordDialogDiv" class="hidden">
	   <?php echo _("Master password")?>
	   <input type="password" name="passwd">
	   <?php echo printHelpLink(getHelp('', '236'), '236')?>
	</div>
	<?php
		if (isset($_GET['savedSuccessfully'])) {
			$message = new htmlStatusMessage("INFO", _("PDF structure was successfully saved."), htmlspecialchars($_GET['savedSuccessfully']));
			$message->colspan = 10;
			$container->addElement($message, true);
		}

		// new template
		$container->addElement(new htmlSubTitle(_('Create a new PDF structure')), true);
		$newPDFContainer = new htmlTable();
		$newScopeSelect = new htmlSelect('scope', $availableScopes);
		$newScopeSelect->setHasDescriptiveElements(true);
		$newScopeSelect->setWidth('15em');
		$newPDFContainer->addElement($newScopeSelect);
		$newPDFContainer->addElement(new htmlSpacer('10px', null));
		$newPDFContainer->addElement(new htmlButton('createNewTemplate', _('Create')));
		$container->addElement($newPDFContainer, true);
		$container->addElement(new htmlSpacer(null, '10px'), true);

		// existing templates
		$configProfiles = getConfigProfiles();

		$container->addElement(new htmlSubTitle(_("Manage existing PDF structures")), true);
		$existingContainer = new htmlTable();
		for ($i = 0; $i < sizeof($templateClasses); $i++) {
			if ($i > 0) {
				$existingContainer->addElement(new htmlSpacer(null, '10px'), true);
			}

			$existingContainer->addElement(new htmlImage('../../graphics/' . $templateClasses[$i]['scope'] . '.png'));
			$existingContainer->addElement(new htmlSpacer('3px', null));
			$existingContainer->addElement(new htmlOutputText($templateClasses[$i]['title']));
			$existingContainer->addElement(new htmlSpacer('3px', null));
			$select = new htmlSelect('template_' . $templateClasses[$i]['scope'], $templateClasses[$i]['templates']);
			$select->setWidth('15em');
			$existingContainer->addElement($select);
			$existingContainer->addElement(new htmlSpacer('3px', null));
			$exEditButton = new htmlButton('editTemplate_' . $templateClasses[$i]['scope'], 'edit.png', true);
			$exEditButton->setTitle(_('Edit'));
			$existingContainer->addElement($exEditButton);
			$deleteLink = new htmlLink(null, '#', '../../graphics/delete.png');
			$deleteLink->setTitle(_('Delete'));
			$deleteLink->setOnClick("profileShowDeleteDialog('" . _('Delete') . "', '" . _('Ok') . "', '" . _('Cancel') . "', '" . $templateClasses[$i]['scope'] . "', '" . 'template_' . $templateClasses[$i]['scope'] . "');");
			$existingContainer->addElement($deleteLink);

			if (count($configProfiles) > 1) {
				$importLink = new htmlLink(null, '#', '../../graphics/import.png');
				$importLink->setTitle(_('Import PDF structures'));
				$importLink->setOnClick("showDistributionDialog('" . _("Import PDF structures") . "', '" .
										_('Ok') . "', '" . _('Cancel') . "', '" . $templateClasses[$i]['scope'] . "', 'import');");
				$existingContainer->addElement($importLink);
			}
			$exportLink = new htmlLink(null, '#', '../../graphics/export.png');
			$exportLink->setTitle(_('Export PDF structure'));
			$exportLink->setOnClick("showDistributionDialog('" . _("Export PDF structure") . "', '" .
									_('Ok') . "', '" . _('Cancel') . "', '" . $templateClasses[$i]['scope'] . "', 'export', '" . 'template_' . $templateClasses[$i]['scope'] . "', '" . $_SESSION['config']->getName() . "');");
			$existingContainer->addElement($exportLink);
			$existingContainer->addNewLine();
		}
		$container->addElement($existingContainer, true);
		$container->addElement(new htmlSpacer(null, '10px'), true);

		// generate content
		$tabindex = 1;
		parseHtml(null, $container, array(), false, $tabindex, 'user');

		echo "</form>\n";
		echo "</div>\n";

		for ($i = 0; $i < sizeof($templateClasses); $i++) {
			$scope = $templateClasses[$i]['scope'];
			$tmpArr = array();
			foreach ($configProfiles as $profile) {
				if ($profile != $_SESSION['config']->getName()) {
					$accountProfiles = getPDFStructureDefinitions($scope, $profile);
					for ($p = 0; $p < sizeof($accountProfiles); $p++) {
						$tmpArr[$profile][$accountProfiles[$p]] = $profile . '##' . $accountProfiles[$p];
					}
				}
			}

			//import dialog
			echo "<div id=\"importDialog_$scope\" class=\"hidden\">\n";
			echo "<form id=\"importDialogForm_$scope\" method=\"post\" action=\"pdfmain.php\">\n";

			$container = new htmlTable();
			$container->addElement(new htmlOutputText(_('PDF structures')), true);

			$select = new htmlSelect('importProfiles_' . $scope, $tmpArr, array(), count($tmpArr, 1) < 15 ? count($tmpArr, 1) : 15);
			$select->setMultiSelect(true);
			$select->setHasDescriptiveElements(true);
			$select->setContainsOptgroups(true);
			$select->setWidth('290px');

			$container->addElement($select);
			$container->addElement(new htmlHelpLink('408'), true);

			$container->addElement(new htmlSpacer(null, '10px'), true);

			$container->addElement(new htmlOutputText(_("Master password")), true);
			$exportPasswd = new htmlInputField('passwd_' . $scope);
			$exportPasswd->setIsPassword(true);
			$container->addElement($exportPasswd);
			$container->addElement(new htmlHelpLink('236'));
			$container->addElement(new htmlHiddenInput('importexport', '1'));
			$container->addElement(new htmlHiddenInput('scope', $scope), true);
			
			parseHtml(null, $container, array(), false, $tabindex, 'user');

			echo '</form>';
			echo "</div>\n";
		}

		//export dialog
		echo "<div id=\"exportDialog\" class=\"hidden\">\n";
		echo "<form id=\"exportDialogForm\" method=\"post\" action=\"pdfmain.php\">\n";

		$container = new htmlTable();
		
		$container->addElement(new htmlOutputText(_('PDF structure')), true);
		$expStructGroup = new htmlTable();
		$expStructGroup->addElement(new htmlSpacer('10px', null));
		$expStructGroup->addElement(new htmlDiv('exportName', ''));
		$container->addElement($expStructGroup, true);
		$container->addElement(new htmlSpacer(null, '10px'), true);
		
		$container->addElement(new htmlOutputText(_("Target server profile")), true);
		foreach ($configProfiles as $key => $value) {
			$tmpProfiles[$value] = $value;
		}
		natcasesort($tmpProfiles);
		$tmpProfiles['*' . _('Global templates')] = 'templates*';
		
		$findProfile = array_search($_SESSION['config']->getName(), $tmpProfiles);
		if ($findProfile !== false) {
			unset($tmpProfiles[$findProfile]);
		}
		$select = new htmlSelect('destServerProfiles', $tmpProfiles, array(), count($tmpProfiles) < 10 ? count($tmpProfiles) : 10);
		$select->setHasDescriptiveElements(true);
		$select->setSortElements(false);
		$select->setMultiSelect(true);

		$container->addElement($select);
		$container->addElement(new htmlHelpLink('409'), true);
		$container->addElement(new htmlSpacer(null, '10px'), true);
		
		$container->addElement(new htmlOutputText(_("Master password")), true);
		$exportPasswd = new htmlInputField('passwd');
		$exportPasswd->setIsPassword(true);
		$container->addElement($exportPasswd);
		$container->addElement(new htmlHelpLink('236'));
		$container->addElement(new htmlHiddenInput('importexport', '1'), true);
		
		parseHtml(null, $container, array(), false, $tabindex, 'user');

		echo '</form>';
		echo "</div>\n";

// form for delete action
echo '<div id="deleteProfileDialog" class="hidden"><form id="deleteProfileForm" action="pdfmain.php" method="post">';
	echo _("Do you really want to delete this PDF structure?");
	echo '<br><br><div class="nowrap">';
	echo _("Structure name") . ': <div id="deleteText" style="display: inline;"></div></div>';
	echo '<input id="profileDeleteType" type="hidden" name="profileDeleteType" value="">';
	echo '<input id="profileDeleteName" type="hidden" name="profileDeleteName" value="">';
	echo '<input type="hidden" name="deleteProfile" value="true">';
echo '</form></div>';

include '../main_footer.php';
?>
