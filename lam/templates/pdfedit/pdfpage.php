<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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

  Manages creating/changing of pdf structures.

*/

include_once("../../lib/pdfstruct.inc");
include_once("../../lib/ldap.inc");
include_once("../../lib/config.inc");
include_once("../../lib/modules.inc");
include_once('../../lib/xml_parser.inc');

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

// check if user is logged in, if not go to login
if (!$_SESSION['ldap'] || !$_SESSION['ldap']->server()) {
	metaRefresh("../login.php");
	exit;
}

if($_GET['edit']) {
	$currentStructure = loadPDFStructureDefinitions($_GET['type'],$_GET['edit']);
}
else {
	$currentStructure = loadPDFStructureDefinitions($_GET['type']);
}

$availableFields = getAvailablePDFFields($_GET['type']);

// print header
echo $_SESSION['header'];
?>
		<title>LDAP Account Manager</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<br>
		<form action="pdfpage.php" method="post">
			<table width="100%">
				<tr>
					<!-- print current structure -->
					<td width="45%" align="center">
						<fieldset>
							<legend>
								<b><?php echo _("Current PDF structure"); ?></b>
							</legend>
							<table>
<?php
$i = 0;
foreach($currentStructure as $entry) {
	$links = "\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;up=" . $i . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Up') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td width=\"10\">\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;down=" . $i . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Down') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td width=\"10\">\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;remove=" . $i . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Remove') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n";
	$uplink = 'pdfpage.php?type=' . $_GET['type'] . '&amp;up=' . $i . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '');
	$downlink = 'pdfpage.php?type=' . $_GET['type'] . '&amp;down=' . urlencode($entry['tag']) . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '');
	// We have a new section to start
	if($entry['tag'] == "SECTION" && $entry['type'] == "open") {
		$name = $entry['attributes']['NAME'];
		if(preg_match("/^\_[a-zA-Z\_]+/",$name)) {
			$section_headline = substr($name,1);
		}
		else {
			$section_headline = $name;
		}
		?>
								<tr>
									<td width="20" align="left">
										<input type="radio" name="section">
									</td>
									<td colspan="2">
										<b><?php echo $section_headline;?></b>
									</td>
									<td width="20">
									</td>
									<!-- <td>
										<a href="<?php echo $uplink;?>"><?php echo _('Up');?></a>
									</td>
									<td width="10">
									</td>
									<td>
										<a href="<?php echo $downlink;?>"><?php echo _('Down');?></a>
									</td> -->
									<?php echo $links;?>
								</tr>
		<?php
	}
	// We have a section to end
	elseif($entry['tag'] == "SECTION" && $entry['type'] == "close") {
		?>
								<tr>
									<td colspan="9">
										<br>
									</td>
								</tr>
		<?php
	}
	// We have to include a static text.
	elseif($entry['tag'] == "TEXT") {
		if($entry['type'] == "complete") {
			?>
								<tr>
									<td>
									</td>
									<td colspan="2">
										<b><?php echo _('Static text');?></b>
									</td>
									<td width="20">
									</td>
									<!-- <td>
										<a href="<?php echo $uplink;?>"><?php echo _('Up');?></a>
									</td>
									<td width="10">
									</td>
									<td>
										<a href="<?php echo $downlink;?>"><?php echo _('Down');?></a>
									</td> -->
									<?php echo $links;?>
								</tr>
								<tr>
									<td colspan="2">
									</td>
									<td>
										<?php echo _('Print PDF text from config.');?>
									</td>
									<td colspan="6">
									</td>
								</tr>
			<?php
		}
		else {
			?>
								<tr>
									<td>
									</td>
									<td colspan="2">
										<b><?php echo _('Static text');?></b>
									</td>
									<td width="20">
									</td>
									<!-- <td>
										<a href="<?php echo $uplink;?>"><?php echo _('Up');?></a>
									</td>
									<td width="10">
									</td>
									<td>
										<a href="<?php echo $downlink;?>"><?php echo _('Down');?></a>
									</td> -->
									<?php echo $links;?>
								</tr>
								<tr>
									<td colspan="3">
									</td>
									<td>
										<textarea name="pdftext" rows="10" cols="50" wrap="off">
											<?php echo $entry['value'];?>
										</textarea>
									</td>
										<td colspan="6">
									</td>
								</tr>
			<?php
		}
	}
	// We have to include an entry from the account
	elseif($entry['tag'] == "ENTRY") {
		// Get name of current entry
		$name = $entry['attributes']['NAME'];
		?>
								<tr>
									<td>
									</td>
									<td width="20">
									</td>
									<td>
										<?php echo $name;?>
									</td>
									<td width="20">
									</td>
									<!-- <td>
										<a href="<?php echo $uplink;?>"><?php echo _('Up');?></a>
									</td>
									<td width="10">
									</td>
									<td>
										<a href="<?php echo $downlink;?>"><?php echo _('Down');?></a>
									</td> -->
									<?php echo $links;?>
								</tr>
		<?php
	}
	$i++;
}
?>
							</table>
						</fieldset>
					</td>
					<td width="10%" align="center">
						<input type="button" name="add" value="<=">
					</td>
					<!-- print available fields sorted by modul -->
					<td width="45%" align="center">
						<fieldset>
							<legend>
								<b><?php echo _("Available PDF fields"); ?></b>
							</legend>
							<table>
<?php
foreach($availableFields as $module => $fields) {
	?>
								<tr>
									<td colspan="2">
										<b><?php echo $module;?></b>
									</td>
								</tr>
								<tr>
									<td width="20">
									</td>
									<td>
										<select name="<?php echo $module?>[]" size="7" multiple>
	<?php
	foreach($fields as $field) {
		?>
											<option><?php echo $field;?></option>
		<?php
	}
	?>
										</select>
									</td>
								</tr>
								<tr>
									<td colspan="2">
										<br>
									</td>
								</tr>
	<?php
}
?>
							</table>
						</fieldset>
					</td>
				</tr>
			</table>
			<p>&nbsp;</p>
			<table border="0">
			<tr>
				<td>
					<b><?php echo _("Structure name"); ?>:</b>
				</td>
				<td>
					<input type="text" name="profname" value="<?php echo $_GET['edit'];?>">
				</td>
				<td>
					<a href="../help.php?HelpNumber=360" target="lamhelp"><?php echo _("Help");?></a>
				</td>
			</tr>
			<tr>
				<td colspan=3>
					&nbsp
				</td>
			</tr>
			<tr>
				<td>
					<input type="submit" name="submit" value="<?php echo _("Save");?>">
				</td>
				<td>
					<input type="reset" name="reset" value="<?php echo _("Reset");?>">
					<input type="submit" name="abort" value="<?php echo _("Abort");?>">
				</td>
				<td>
					&nbsp
				</td>
			</tr>
		</table>
		<input type="hidden" name="accounttype" value="<?php echo $type;?>">
	</form>
	</body>
</html>
<?php
?>