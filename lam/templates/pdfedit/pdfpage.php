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

  Manages creating/changing of pdf structures.

*/

include_once('../../lib/pdfstruct.inc');
include_once('../../lib/ldap.inc');
include_once('../../lib/config.inc');
include_once('../../lib/modules.inc');
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

if(isset($_POST['type'])) {
	$_GET = $_POST;
}

if(isset($_GET['abort'])) {
	unset($_SESSION['currentPDFStructure']);
	unset($_SESSION['availablePDFFields']);
	session_unregister('currentPDFStructure');
	session_unregister('availablePDFFields');
	metarefresh('pdfmain.php');
	exit;
}
elseif(isset($_GET['submit'])) {
	savePDFStructureDefinitions($_GET['type'],$_GET['pdfname'] . '.xml');
	unset($_SESSION['currentPDFStructure']);
	unset($_SESSION['availablePDFFields']);
	session_unregister('currentPDFStructure');
	session_unregister('availablePDFFields');
	metarefresh('pdfmain.php');
	exit;
}
elseif(isset($_GET['add_section'])) {
	$attributes = array();
	if($_GET['section_type'] == 'text') {
		$attributes['NAME'] = $_GET['section_text'];
	}
	elseif($_GET['section_type'] == 'item') {
		$attributes['NAME'] = '_' . $_GET['section_item'];
	}
	$newSectionStart = array('tag' => 'SECTION','type' => 'open','level' => '2','attributes' => $attributes);
	$newSectionEnd = array('tag' => 'SECTION','type' => 'close','level' => '2');
	$_SESSION['currentPDFStructure'][] = $newSectionStart;
	$_SESSION['currentPDFStructure'][] = $newSectionEnd;
}
elseif(isset($_GET['add_field'])) {
	$modules = explode(',',$_GET['modules']);
	$fields = array();
	foreach($modules as $module) {
		if(isset($_GET[$module])) {
			foreach($_GET[$module] as $field) {
				$fields[] = array('tag' => 'ENTRY','type' => 'complete','level' => '3','attributes' => array('NAME' => $module . '_' . $field));
			}
		}
	}
	if(count($fields) > 0) {
		$pos = 0;
		while($pos < $_GET['section']) {
			next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		$current = next($_SESSION['currentPDFStructure']);
		$pos++;
		while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		array_splice($_SESSION['currentPDFStructure'],$pos,0,$fields);
	}
}
elseif(isset($_GET['remove'])) {
	$start = 0;
	while($start < $_GET['remove']) {
		next($_SESSION['currentPDFStructure']);
		$start++;
	}
	$remove = current($_SESSION['currentPDFStructure']);
	if($remove['tag'] == "SECTION") {
		$end = $start;
		$current = next($_SESSION['currentPDFStructure']);
		$end++;
		while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
			$current = next($_SESSION['currentPDFStructure']);
			$end++;
		}
		echo "start: $start\nend: $end\n";
		array_splice($_SESSION['currentPDFStructure'],$start,$end - $start + 1);
	}
	elseif($remove['tag'] == "ENTRY") {
		array_splice($_SESSION['currentPDFStructure'],$start,1);
	}
	elseif($remove['tag'] == "TEXT") {
		array_splice($_SESSION['currentPDFStructure'],$start,1);
	}
}
elseif(isset($_GET['up'])) {
	$tmp = $_SESSION['currentPDFStructure'][$_GET['up']];
	$prev = $_SESSION['currentPDFStructure'][$_GET['up'] - 1];
	if($tmp['tag'] == 'SECTION') {
		$pos = 0;
		$borders = array();
		$current = current($_SESSION['currentPDFStructure']);
		if($current['tag'] == 'SECTION') {
			$borders[$current['type']][] = $pos;
		}
		while($pos < $_GET['up']) {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
			if($current['tag'] == 'SECTION') {
				$borders[$current['type']][] = $pos;
			}
		}
		if(count($borders['close']) > 0) {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
			while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
			}
			$borders['close'][] = $pos;
			$cut_start = $borders['open'][count($borders['open']) - 1];
			$cut_count = $borders['close'][count($borders['close']) - 1] - $borders['open'][count($borders['open']) - 1] + 1;
			$insert_pos = $borders['open'][count($borders['open']) - 2];
			$tomove = array_splice($_SESSION['currentPDFStructure'],$cut_start,$cut_count);
			array_splice($_SESSION['currentPDFStructure'],$insert_pos,0,$tomove);
		}
	}
	elseif($tmp['tag'] == 'ENTRY' && $prev['tag'] == 'ENTRY') {
		$_SESSION['currentPDFStructure'][$_GET['up']] = $_SESSION['currentPDFStructure'][$_GET['up'] - 1];
		$_SESSION['currentPDFStructure'][$_GET['up'] - 1] = $tmp;
	}
	elseif($tmp['tag'] == 'TEXT') {
		if($_GET['up'] != 0) {
			$tomove = array_splice($_SESSION['currentPDFStructure'],$_GET['up'],1);
			array_splice($_SESSION['currentPDFStructure'],0,0,$tomove);
		}
	}
}
elseif(isset($_GET['down'])) {
	$tmp = $_SESSION['currentPDFStructure'][$_GET['down']];
	$next = $_SESSION['currentPDFStructure'][$_GET['down'] + 1];
	if($tmp['tag'] == 'SECTION') {
		$pos = 0;
		$current = current($_SESSION['currentPDFStructure']);
		while($pos < $_GET['down']) {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		$borders = array();
		$borders[$current['type']][] = $pos;
		$current = next($_SESSION['currentPDFStructure']);
		$pos++;
		while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		$borders['close'][] = $pos;
		$current = next($_SESSION['currentPDFStructure']);
		$pos++;
		if($current) {
			$borders[$current['type']][] = $pos;
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
			while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
				if($current['tag'] == 'SECTION') {
					$borders[$current['type']][] = $pos;
				}
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
			}
			$borders['close'][] = $pos;
		}
		if(count($borders['open']) > 1) {
			$cut_start = $borders['open'][count($borders['open']) - 1];
			$cut_count = $borders['close'][count($borders['close']) - 1] - $borders['open'][count($borders['open']) - 1] + 1;
			$insert_pos = $borders['open'][count($borders['open']) - 2];
			$tomove = array_splice($_SESSION['currentPDFStructure'],$cut_start,$cut_count);
			array_splice($_SESSION['currentPDFStructure'],$insert_pos,0,$tomove);
		}
	}
	elseif($tmp['tag'] == 'ENTRY' && $next['tag'] == 'ENTRY') {
		$_SESSION['currentPDFStructure'][$_GET['down']] = $_SESSION['currentPDFStructure'][$_GET['down'] + 1];
		$_SESSION['currentPDFStructure'][$_GET['down'] + 1] = $tmp;
	}
	elseif($tmp['tag'] == 'TEXT') {
		if($_GET['down'] != (count($_SESSION['currentPDFStructure']) -1)) {
			$tomove = array_splice($_SESSION['currentPDFStructure'],$_GET['down'],1);
			array_splice($_SESSION['currentPDFStructure'],count($_SESSION['currentPDFStructure']),0,$tomove);
		}
	}
}
elseif(isset($_GET['add_text'])) {
	if($_GET['text_type'] == 'config') {
		$entry = array('tag' => 'TEXT','type' => 'complete','level' => '2','attributes' => array('type' => $_GET['type']));
	}
	else {
		$entry = array('tag' => 'TEXT','type' => 'complete','level' => '2','value' => $_GET['text_text']);
	}
	if($_GET['text_position'] == 'top') {
		array_splice($_SESSION['currentPDFStructure'],0,0,array($entry));
	}
	else {
		array_push($_SESSION['currentPDFStructure'],$entry);
	}
}

if(!isset($_SESSION['currentPDFStructure'])) {
	if($_GET['edit']) {
		$_SESSION['currentPDFStructure'] = loadPDFStructureDefinitions($_GET['type'],$_GET['edit']);
	}
	else {
		$_SESSION['currentPDFStructure'] = loadPDFStructureDefinitions($_GET['type']);
	}
}

if(!isset($_SESSION['availablePDFFields'])) {
	$_SESSION['availablePDFFields'] = getAvailablePDFFields($_GET['type']);
}

$modules = array();
$section_items = '';
foreach($_SESSION['availablePDFFields'] as $module => $values) {
	$modules[] = $module;
	foreach($values as $attribute) {
		$section_items .= "\t\t\t\t\t\t\t\t\t\t\t\t<option>" . $module . '_' . $attribute . "</option>\n";
	}
}
$modules = join(',',$modules);

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
								<b><?php echo (($_GET['edit']) ? substr($_GET['edit'],0,strlen($GET['edit']) - 4) : _('New') . ' ' . $_GET['type']) . ' ' . _("PDF structure"); ?></b>
							</legend>
							<table>
<?php
foreach($_SESSION['currentPDFStructure'] as $key => $entry) {
	$links = "\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;up=" . $key . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Up') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td width=\"10\">\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;down=" . $key . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Down') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td width=\"10\">\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;remove=" . $key . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Remove') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n";
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
										<input type="radio" name="section" value="<?php echo $key;?>">
									</td>
									<td colspan="2">
										<b><?php echo $section_headline;?></b>
									</td>
									<td width="20">
									</td>
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
		if(!isset($entry['value'])) {
			?>
								<tr>
									<td>
									</td>
									<td colspan="2">
										<b><?php echo _('Static text');?></b>
									</td>
									<td width="20">
									</td>
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
									<?php echo $links;?>
								</tr>
								<tr>
									<td colspan="2">
									</td>
									<td>
										<?php echo $entry['value'];?>
									</td>
										<td colspan="6">
									</td>
								</tr>
			<?php
		}
		?>
								<tr>
									<td colspan="9">
										<br>
									</td>
								</tr>
		<?php
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
									<?php echo $links;?>
								</tr>
		<?php
	}
}
?>
							</table>
							<fieldset>
								<legend>
									<b><?php echo _("Add new section"); ?></b>
								</legend>
 								<table align="left" width="100%"> 
									<tr>
										<td>
											<input type="radio" name="section_type" value="text" checked>
										</td>
										<td colspan="2">
											<input type="text" name="section_text">
										</td>
										<td colspan="6" rowspan="2" align="left">
											<input type="submit" name="add_section" value="<?php echo _('Add');?>">
										</td>
									</tr>
									<tr>
										<td>
											<input type="radio" name="section_type" value="item">
										</td>
										<td>
											<select name="section_item">
												<?php echo $section_items;?>
											</select>
										</td>
									</tr>
								</table>
							</fieldset>
							<p>&nbsp;</p>
							<fieldset>
								<legend>
									<b><?php echo _("Add static text"); ?></b>
								</legend>
								<table align="left" width="100%">
									<tr>
										<td colspan="2">
											<fieldset style="margin:0px;">
												<legend>
													<?php echo _('Position');?>
												</legend>
												<table width="100%" style="margin:0px;">
													<tr>
														<td>
															<input type="radio" name="text_position" value="top" checked>
														</td>
														<td width="50%">
															<?php echo _('Top');?>
														</td>
														<td>
															<input type="radio" name="text_position" value="bottom">
														</td>
														<td width="50%">
															<?php echo _('Bottom');?>
														</td>
													</tr>
												</table>
											</fieldset>
										</td>
										<td rowspan="2">
											<input type="submit" name="add_text" value="<?php echo _('Add');?>">
										</td>
									</tr>
									<tr>
										<td>
											<fieldset>
												<legend>
													<?php echo _('Text');?>
												</legend>
												<table width="100%">
													<tr>
														<td>
															<input type="radio" name="text_type" value="config" checked>
														</td>
														<td>
															<?php echo _('Insert static text from config.');?>
														</td>
													</tr>
													<tr>
														<td>
															<input type="radio" name="text_type" value="textfield">
														</td>
														<td>
															<?php echo _('Use text from field below.');?>
														</td>
													</tr>
													<tr>
														<td>
														</td>
														<td>
															<textarea name="text_text" rows="4" cols="40"></textarea>
 														</td>
 													</tr>
												</table>
											</fieldset>
										</td>
									</tr>
								</table>
							</fieldset>
						</fieldset>
						<p>&nbsp;</p>
						<fieldset>
							<legend>
								<b><?php echo _("Submit"); ?></b>
							</legend>
							<table border="0" align="left">
								<tr>
									<td>
										<b><?php echo _("Structure name"); ?>:</b>
									</td>
									<td>
										<input type="text" name="pdfname" value="<?php echo substr($_GET['edit'],0,strlen($_GET['edit']) -4);?>">
									</td>
									<td>
										<a href="../help.php?HelpNumber=360" target="lamhelp"><?php echo _("Help");?></a>
									</td>
								</tr>
								<tr>
									<td colspan="3">
										&nbsp
									</td>
								</tr>
								<tr>
									<td>
										<input type="submit" name="submit" value="<?php echo _("Save");?>">
									</td>
									<td>
										<input type="submit" name="abort" value="<?php echo _("Abort");?>">
									</td>
									<td>
										&nbsp
									</td>
								</tr>
							</table>
						</fieldset>
					</td>
					
					<td width="10%" align="center">
						<input type="submit" name="add_field" value="<=">
					</td>
					
					<!-- print available fields sorted by modul -->
					<td width="45%" align="center">
						<fieldset>
							<legend>
								<b><?php echo _("Available PDF fields"); ?></b>
							</legend>
							<table>
<?php
foreach($_SESSION['availablePDFFields'] as $module => $fields) {
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
		<input type="hidden" name="modules" value="<?php echo $modules;?>">
		<input type="hidden" name="type" value="<?php echo $_GET['type'];?>">
	</form>
	</body>
</html>
<?php
?>