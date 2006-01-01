<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Dürgner

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
* @author Michael Dürgner
* @package PDF
*/

/** access to PDF configuration files */
include_once("../../lib/pdfstruct.inc");
/** LDAP object */
include_once("../../lib/ldap.inc");
/** for language settings */
include_once("../../lib/config.inc");
/** module functions */
include_once("../../lib/modules.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// Unset pdf structure definitions in session if set
if(isset($_SESSION['currentPDFStructure'])) {
	session_unregister('currentPDFStructure');
	session_unregister('availablePDFFields');
	session_unregister('currentPageDefinitions');
	unset($_SESSION['currentPDFStructure']);
	unset($_SESSION['availablePDFFields']);
	unset($_SESSION['currentPageDefinitions']);
}

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

// check if user has pressed submit or abort button
if ($_POST['forward'] == "yes") {
	// on abort go back to main page
	if ($_POST['abort']) {
		metaRefresh("../lists/listusers.php");
	}
	// on submit forward to other pdf structure pages
	else if($_POST['submit']) {
		if($_POST['pdf'] == 'new') {
			metaRefresh('pdfpage.php?type=' . $_POST['scope']);
		}
		else if($_POST['pdf'] == 'edit') {
			$edit = split(':',$_POST['edit']);
			metaRefresh('pdfpage.php?type=' . $edit[0] . '&edit=' . $edit[1]);
		}
		else if($_POST['pdf'] == 'delete') {
			$delete = split(':',$_POST['delete']);
			metaRefresh('pdfdelete.php?type=' . $delete[0] . '&delete=' . $delete[1]);
		}
	}
	exit;
}

$scopes = $_SESSION['config']->get_ActiveTypes();

$availableStructureDefinitions = '';
$availableScopes = '';

foreach($scopes as $scope) {
	$pdfStructDefs = getPDFStructureDefinitions($scope);
	$availableScopes .= '<option value="' . $scope . '">' . $scope . "</option>\n";
	
	foreach($pdfStructDefs as $pdfStructureDefinition) {
		$availableStructureDefinitions .= '<option value="' . $scope . ':' . $pdfStructureDefinition . '">' . $scope . ' - ' . $pdfStructureDefinition . "</option>\n";
	}
}

echo $_SESSION['header'];
?>
		<title>LDAP Account Manager</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<p></p>
		<form action="pdfmain.php" method="post">
		<!-- pdf structure options -->
		<fieldset>
			<legend>
				<b><?php echo _("PDF structures"); ?></b>
			</legend>
			<table border=0>
				<!-- new pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="new" checked="checked">
					</td>
					<td colspan=2><?php echo _("Create a new PDF structure for scope: "); ?><select name="scope" size="1"><?php echo $availableScopes; ?></select></td>
				</tr>
				<!-- edit pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="edit">
					</td>
					<td>
						<select name="edit" size=1>
							<?php echo $availableStructureDefinitions; ?>
						</select>
					</td>
					<td><?php echo _("Edit PDF structure"); ?></td>
				</tr>
				<!-- delete pdf structure -->
				<tr>
					<td>
						<input type="radio" name="pdf" value="delete">
					</td>
					<td>
						<select name="delete" size=1>
							<?php echo $availableStructureDefinitions; ?>
						</select>
					</td>
					<td><?php echo _("Delete PDF structure"); ?></td>
				</tr>
			</table>
		</fieldset>
		<p></p>
		
		<!-- forward is used to check if buttons were pressed -->
		<p>
		<input type="hidden" name="forward" value="yes">

		<input type="submit" name="submit" value="<?php echo _("Submit"); ?>">
		<input type="submit" name="abort" value="<?php echo _("Abort"); ?>">
		</p>

		</form>
	</body>
</html>