<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael D�rgner

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
* @author Michael D�rgner
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
	metaRefresh('pdfpage.php?type=' . $_POST['scope']);
	exit();
}

$scopes = $_SESSION['config']->get_ActiveTypes();

// get list of account types
$availableScopes = '';
$templateClasses = array();
for ($i = 0; $i < sizeof($scopes); $i++) {
	$templateClasses[] = array(
		'scope' => $scopes[$i],
		'title' => getTypeAlias($scopes[$i]),
		'templates' => "");
	$availableScopes .= '<option value="' . $scopes[$i] . '">' . getTypeAlias($scopes[$i]) . "</option>\n";
}
// get list of templates for each account type
for ($i = 0; $i < sizeof($templateClasses); $i++) {
	$templateList = getPDFStructureDefinitions($templateClasses[$i]['scope']);
	$templates = "";
	for ($l = 0; $l < sizeof($templateList); $l++) {
		$templates = $templates . "<option>" . $templateList[$l] . "</option>\n";
	}
	$templateClasses[$i]['templates'] = $templates;
}

// check if a template should be edited
for ($i = 0; $i < sizeof($templateClasses); $i++) {
	if (isset($_POST['editTemplate_' . $templateClasses[$i]['scope']]) || isset($_POST['editTemplate_' . $templateClasses[$i]['scope'] . '_x'])) {
		metaRefresh('pdfpage.php?type=' . $templateClasses[$i]['scope'] . '&edit=' . $_POST['template_' . $templateClasses[$i]['scope']]);
		exit;
	}
}
// check if a profile should be deleted
for ($i = 0; $i < sizeof($templateClasses); $i++) {
	if (isset($_POST['deleteTemplate_' . $templateClasses[$i]['scope']]) || isset($_POST['deleteTemplate_' . $templateClasses[$i]['scope'] . '_x'])) {
		metaRefresh('pdfdelete.php?type=' . $templateClasses[$i]['scope'] . '&delete=' . $_POST['template_' . $templateClasses[$i]['scope']]);
		exit;
	}
}


echo $_SESSION['header'];
?>
		<title>LDAP Account Manager</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
		<link rel="stylesheet" type="text/css" href="../../style/type_user.css">
	</head>
	<body>
		<br>
	<h1><?php echo _('PDF editor'); ?></h1>
	<br>
		<form action="pdfmain.php" method="post">
		
		<!-- new template -->		
		<fieldset class="useredit">
		<legend>
		<b><?php echo _('Create a new PDF structure'); ?></b>
		</legend>
		<br><table border=0>
			<tr><td>
				<select class="user" name="scope">
					<?php echo $availableScopes; ?>
				</select>
			</td>
			<td>
				<input type="submit" name="createNewTemplate" value="<?php echo _('Create'); ?>">
			</td></tr>
		</table>
		</fieldset>
		
		<br>


		<!-- existing templates -->
		<fieldset class="useredit">
		<legend>
			<b><?php echo _("Manage existing PDF structures"); ?></b>
		</legend>
		<br><table border=0>
		<?php
		for ($i = 0; $i < sizeof($templateClasses); $i++) {
			if ($i > 0) {
				echo "<tr><td colspan=3>&nbsp;</td></tr>\n";
			}
			echo "<tr>\n";
				echo "<td>";
					echo "<img alt=\"" . $templateClasses[$i]['title'] . "\" src=\"../../graphics/" . $templateClasses[$i]['scope'] . ".png\">&nbsp;\n";
					echo $templateClasses[$i]['title'];
				echo "</td>\n";
				echo "<td>&nbsp;";
					echo "<select class=\"user\" style=\"width: 20em;\" name=\"template_" . $templateClasses[$i]['scope'] . "\">\n";
						echo $templateClasses[$i]['templates'];
					echo "</select>\n";
				echo "</td>\n";
				echo "<td>&nbsp;";
					echo "<input type=\"image\" src=\"../../graphics/edit.png\" name=\"editTemplate_" . $templateClasses[$i]['scope'] . "\" " .
					 "alt=\"" . _('Edit') . "\" title=\"" . _('Edit') . "\">";
					echo "&nbsp;";
					echo "<input type=\"image\" src=\"../../graphics/delete.png\" name=\"deleteTemplate_" . $templateClasses[$i]['scope'] . "\" " .
					"alt=\"" . _('Delete') . "\" title=\"" . _('Delete') . "\">";
				echo "</td>\n";
			echo "</tr>\n";
		}
		?>
		</table>
		</fieldset>
		<br>
		
		</form>
	</body>
</html>
