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

// Write $_POST variables to $_GET when form was submitted via post
if(isset($_POST['type'])) {
	$_GET = $_POST;
	if($_POST['pdfname'] == '') {
		unset($_GET['pdfname']);
	}
}

// Abort and go back to main pdf structure page
if(isset($_GET['abort'])) {
	metarefresh('pdfmain.php');
	exit;
}
// Check if pdfname is valid, then save current structure to file and go to
// main pdf structure page
elseif(isset($_GET['submit'])) {
	if(!isset($_GET['pdfname']) || !preg_match('/[a-zA-Z0-9\-\_\.]+/',$_GET['pdfname'])) {
		StatusMessage('ERROR',_('PDF-structure name not valid'),_('The name for that PDF-structure you submitted is not valid. A valid name must constist at least of one of the following characters \'a-z\',\'A-Z\',\'0-9\',\'_\',\'-\',\'.\'.'));
	}
	else {
		savePDFStructureDefinitions($_GET['type'],$_GET['pdfname'] . '.xml');
		metarefresh('pdfmain.php');
		exit;
	}
}
// Add a new section or static text
elseif(isset($_GET['add'])) {
	// Check if name for new section is specified when needed
	if($_GET['add_type'] == 'section' && $_GET['section_type'] == 'text' && (!isset($_GET['section_text']) || $_GET['section_text'] == '')) {
		StatusMessage('ERROR',_('No section text specified'),_('The headline for a new section must contain at least one character.'));
	}
	// Check if text for static text field is specified
	elseif($_GET['add_type'] == 'text' && (!isset($_GET['text_text']) || $_GET['text_text'] == '')) {
		StatusMessage('ERROR',_('No static text specified'),_('The static text must contain at least one character.'));
	}
	else {
		// Add a new section
		if($_GET['add_type'] == 'section') {
			$attributes = array();
			// Add a new section with user headline
			if($_GET['section_type'] == 'text') {
				$attributes['NAME'] = $_GET['section_text'];
			}
			// Add a new section with a module value headline
			elseif($_GET['section_type'] == 'item') {
				$attributes['NAME'] = '_' . $_GET['section_item'];
			}
			$entry = array(array('tag' => 'SECTION','type' => 'open','level' => '2','attributes' => $attributes),array('tag' => 'SECTION','type' => 'close','level' => '2'));
		}
		// Add new static text field
		elseif($_GET['add_type'] == 'text') {
			$entry = array(array('tag' => 'TEXT','type' => 'complete','level' => '2','value' => $_GET['text_text']));
		}
		// Insert new field in structure
		array_splice($_SESSION['currentPDFStructure'],$_GET['add_position'],0,$entry);
	}
}
// Add a new value field
elseif(isset($_GET['add_field'])) {
	// Get available modules
	$modules = explode(',',$_GET['modules']);
	$fields = array();
	// Search each module for selected values
	foreach($modules as $module) {
		if(isset($_GET[$module])) {
			foreach($_GET[$module] as $field) {
				// Create ne value entry
				$fields[] = array('tag' => 'ENTRY','type' => 'complete','level' => '3','attributes' => array('NAME' => $module . '_' . $field));
			}
		}
	}
	if(count($fields) > 0) {
		$pos = 0;
		// Find begin section to insert into
		while($pos < $_GET['section']) {
			next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		$current = next($_SESSION['currentPDFStructure']);
		$pos++;
		// End of section to insert into
		while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		// Insert new entry before closing section tag
		array_splice($_SESSION['currentPDFStructure'],$pos,0,$fields);
	}
}
// Change section headline
elseif(isset($_GET['change'])) {
	$alter = explode('_',$_GET['change']);
	$newvalue = $_GET['section_' . $alter[0]];
	if($alter[1] == 'item') {
		$newvalue = '_' . $newvalue;
	}
	$_SESSION['currentPDFStructure'][$alter[0]]['attributes']['NAME'] = $newvalue;
}
// Remove section, static text or value entry from structure
elseif(isset($_GET['remove'])) {
	$start = 0;
	// Find element to remove
	while($start < $_GET['remove']) {
		next($_SESSION['currentPDFStructure']);
		$start++;
	}
	$remove = current($_SESSION['currentPDFStructure']);
	// We have a section to remove
	if($remove['tag'] == "SECTION") {
		$end = $start;
		$current = next($_SESSION['currentPDFStructure']);
		$end++;
		// Find end of section to remove
		while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
			$current = next($_SESSION['currentPDFStructure']);
			$end++;
		}
		// Remove complete section with all value entries in it from structure
		array_splice($_SESSION['currentPDFStructure'],$start,$end - $start + 1);
	}
	// We have a value entry to remove
	elseif($remove['tag'] == "ENTRY") {
		array_splice($_SESSION['currentPDFStructure'],$start,1);
	}
	// We hava a static text to remove
	elseif($remove['tag'] == "TEXT") {
		array_splice($_SESSION['currentPDFStructure'],$start,1);
	}
}
// Move a section, static text or value entry upwards
elseif(isset($_GET['up'])) {
	$tmp = $_SESSION['currentPDFStructure'][$_GET['up']];
	$prev = $_SESSION['currentPDFStructure'][$_GET['up'] - 1];
	// We have a section or static text to move
	if($tmp['tag'] == 'SECTION' || $tmp['tag'] == 'TEXT') {
		$pos = 0;
		$borders = array();
		$current = current($_SESSION['currentPDFStructure']);
		// Add borders of sections and static text entry to array
		if($current['tag'] == 'SECTION') {
			$borders[$current['type']][] = $pos;
		}
		elseif($current['tag'] == 'TEXT') {
			$borders['open'][] = $pos;
			$borders['close'][] = $pos;
		}
		// Find all sections and statci text fields before the section or static
		// text entry to move upwards
		while($pos < $_GET['up']) {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
			if($current['tag'] == 'SECTION') {
				$borders[$current['type']][] = $pos;
			}
			elseif($current['tag'] == 'TEXT') {
				$borders['open'][] = $pos;
				$borders['close'][] = $pos;
			}
		}
		// Move only when not topmost element
		if(count($borders['close']) > 0) {
			// We have a section to move up
			if($current['tag'] == 'SECTION') {
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
				// Find end of section to move
				while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
					$current = next($_SESSION['currentPDFStructure']);
					$pos++;
				}
				$borders['close'][] = $pos;
			}
			// Calculate the entries to move and move them
			$cut_start = $borders['open'][count($borders['open']) - 1];
			$cut_count = $borders['close'][count($borders['close']) - 1] - $borders['open'][count($borders['open']) - 1] + 1;
			$insert_pos = $borders['open'][count($borders['open']) - 2];
			$tomove = array_splice($_SESSION['currentPDFStructure'],$cut_start,$cut_count);
			array_splice($_SESSION['currentPDFStructure'],$insert_pos,0,$tomove);
		}
	}
	// We have a value entry to move; move it only if its not the topmost
	// entry in this section
	elseif($tmp['tag'] == 'ENTRY' && $prev['tag'] == 'ENTRY') {
		$_SESSION['currentPDFStructure'][$_GET['up']] = $prev;
		$_SESSION['currentPDFStructure'][$_GET['up'] - 1] = $tmp;
	}
}
// Move a section, static text field or value entry downwards
elseif(isset($_GET['down'])) {
	$tmp = $_SESSION['currentPDFStructure'][$_GET['down']];
	$next = $_SESSION['currentPDFStructure'][$_GET['down'] + 1];
	// We have a section or static text to move
	if($tmp['tag'] == 'SECTION' || $tmp['tag'] == 'TEXT') {
		$pos = 0;
		$current = current($_SESSION['currentPDFStructure']);
		// Find section or static text entry to move
		while($pos < $_GET['down']) {
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
		}
		$borders = array();
		// We have a section to move
		if($current['tag'] == 'SECTION'){
			$borders[$current['type']][] = $pos;
			$current = next($_SESSION['currentPDFStructure']);
			$pos++;
			// Find end of section to move
			while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
			}
			$borders['close'][] = $pos;
		}
		// We have a static text entry to move
		elseif($current['tag'] == 'TEXT') {
			$borders['open'][] = $pos;
			$borders['close'][] = $pos;
		}
		$current = next($_SESSION['currentPDFStructure']);
		$pos++;
		// Find next section or static text entry in structure
		if($current) {
			// Next is a section
			if($current['tag'] == 'SECTION') {
				$borders[$current['type']][] = $pos;
				$current = next($_SESSION['currentPDFStructure']);
				$pos++;
				// Find end of this section
				while($current && $current['tag'] != 'SECTION' && $current['type'] != 'close') {
					if($current['tag'] == 'SECTION') {
						$borders[$current['type']][] = $pos;
					}
					$current = next($_SESSION['currentPDFStructure']);
					$pos++;
				}
			}
			// Next is static text entry
			elseif($current['tag'] == 'TEXT') {
				$borders['open'][] = $pos;
			}
			$borders['close'][] = $pos;
		}
		// Move only downwars if not bottenmost element of this structure
		if(count($borders['open']) > 1) {
			// Calculate entries to move and move them
			$cut_start = $borders['open'][count($borders['open']) - 1];
			$cut_count = $borders['close'][count($borders['close']) - 1] - $borders['open'][count($borders['open']) - 1] + 1;
			$insert_pos = $borders['open'][count($borders['open']) - 2];
			$tomove = array_splice($_SESSION['currentPDFStructure'],$cut_start,$cut_count);
			array_splice($_SESSION['currentPDFStructure'],$insert_pos,0,$tomove);
		}
	}
	// We have a value entry to move; move it only if it is not the bottmmost
	// element of this section.
	elseif($tmp['tag'] == 'ENTRY' && $next['tag'] == 'ENTRY') {
		$_SESSION['currentPDFStructure'][$_GET['down']] = $_SESSION['currentPDFStructure'][$_GET['down'] + 1];
		$_SESSION['currentPDFStructure'][$_GET['down'] + 1] = $tmp;
	}
}
// TODO implement page handling
elseif(isset($_POST['page'])) {
	if($_POST['logoFile'] != 'printLogo.jpg' && $_POST['logoFile'] != $_SESSION['currentPageDefinitions']['filename']) {
		$_SESSION['currentPageDefinitions']['filename'] = $_POST['logoFile'];
	}
	if($_POST['logo-width'] != '50' && $_POST['logo-width'] != $_SESSION['currentPageDefinitions']['logo-width']) {
		if($_POST['logo-width'] <= 50 && $_POST['logo-width'] > 0) {
			$_SESSION['currentPageDefinitions']['logo-width'] = $_POST['logo-width'];
		}
	}
	if($_POST['logo-height'] != '20' && $_POST['logo-height'] != $_SESSION['currentPageDefinitions']['logo-height']) {
		if($_POST['logo-height'] <= 20 && $_POST['logo-height'] > 0) {
			$_SESSION['currentPageDefinitions']['logo-height'] = $_POST['logo-height'];
		}
	}
	if(isset($_POST['logo-max']) && !isset($_SESSION['currentPageDefinitions']['logo-max'])) {
		$_SESSION['currentPageDefinitions']['logo-max'] = true;
	}
	if($_POST['headline'] != 'LDAP Account Manager' && $_POST['headline'] != $_SESSION['currentPageDefinitions']['headline']) {
		$_SESSION['currentPageDefinitions']['headline'] = str_replace('<','',str_replace('>','',$_POST['headline']));
	}
	if($_POST['margin-top'] != '10.0' && $_SESSION['currentPageDefinitions']['margin-top'] != $_POST['margin-top']) {
		$_SESSION['currentPageDefinitions']['margin-top'] = $_POST['margin-top'];
	}
	if($_POST['margin-bottom'] != '20.0' && $_SESSION['currentPageDefinitions']['margin-bottom'] != $_POST['margin-bottom']) {
		$_SESSION['currentPageDefinitions']['margin-bottom'] = $_POST['margin-bottom'];
	}
	if($_POST['margin-left'] != '10.0' && $_SESSION['currentPageDefinitions']['margin-left'] != $_POST['margin-left']) {
		$_SESSION['currentPageDefinitions']['margin-left'] = $_POST['margin-left'];
	}
	if($_POST['margin-right'] != '10.0' && $_SESSION['currentPageDefinitions']['margin-right'] != $_POST['margin-right']) {
		$_SESSION['currentPageDefinitions']['margin-right'] = $_POST['margin-right'];
	}
	if(isset($_POST['defaults'])) {
		foreach($_POST['defaults'] as $default) {
			switch($default) {
				case 'logoFile':
					unset($_SESSION['currentPageDefinitions']['filename']);
					break;
				case 'logoSize':
					unset($_SESSION['currentPageDefinitions']['logo-width']);
					unset($_SESSION['currentPageDefinitions']['logo-height']);
					unset($_SESSION['currentPageDefinitions']['logo-max']);
					break;
				case 'headline':
					unset($_SESSION['currentPageDefinitions']['headline']);
					break;
				case 'margin-top':
					unset($_SESSION['currentPageDefinitions']['margin-top']);
					break;
				case 'margin-bottom':
					unset($_SESSION['currentPageDefinitions']['margin-bottom']);
					break;
				case 'margin-left':
					unset($_SESSION['currentPageDefinitions']['margin-left']);
					break;
				case 'margin-right':
					unset($_SESSION['currentPageDefinitions']['margin-right']);
					break;
				default:
					break;
			}
		}
		if(count($_SESSION['currentPageDefinitions']['margin']) == 0) {
			unset($_SESSION['currentPageDefinitions']['margin']);
		}
	}
}

// Load PDF structure from file if it is not defined in session
if(!isset($_SESSION['currentPDFStructure'])) {
	// Load structure file to be edit
	if($_GET['edit']) {
		$load = loadPDFStructureDefinitions($_GET['type'],$_GET['edit']);
		$_SESSION['currentPDFStructure'] = $load['structure'];
		$_SESSION['currentPageDefinitions'] = $load['page_definitions'];
		$_GET['pdfname'] = substr($_GET['edit'],0,strlen($_GET['edit']) - 4);
	}
	// Load default structure file when creating a new one
	else {
		$load = loadPDFStructureDefinitions($_GET['type']);
		$_SESSION['currentPDFStructure'] = $load['structure'];
		$_SESSION['currentPageDefinitions'] = $load['page_definitions'];
	}
}

// Load available fields from modules when not set in session
if(!isset($_SESSION['availablePDFFields'])) {
	$_SESSION['availablePDFFields'] = getAvailablePDFFields($_GET['type']);
}

// Create the values for the dropdown boxes for section headline defined by
// value entries and fetch all available modules
$modules = array();
$section_items_array = array();
$section_items = '';
foreach($_SESSION['availablePDFFields'] as $module => $values) {
	$modules[] = $module;
	foreach($values as $attribute) {
		$section_items_array[] = $module . '_' . $attribute;
		$section_items .= "\t\t\t\t\t\t\t\t\t\t\t\t<option>" . $module . '_' . $attribute . "</option>\n";
	}
}
$modules = join(',',$modules);

$logoFiles = getAvailableLogos();
$logos = '<option value="none"' . (($_SESSION['currentPageDefinitions']['filename'] == 'none') ? ' selected="selected"' : '') . '>' . _('No logo') . "</option>\n";
foreach($logoFiles as $logoFile) {
	$logos .= "\t\t\t\t\t\t\t\t\t\t\t<option value=\"" . $logoFile['filename'] . "\"" .(($_SESSION['currentPageDefinitions']['filename'] == $logoFile['filename'] || (!isset($_SESSION['currentPageDefinitions']['filename']) && $logoFile['filename'] == 'printLogo.jpg')) ? ' selected="selected"' : '') . '">' . $logoFile['filename'] . ' (' . $logoFile['infos'][0] . ' x ' . $logoFile['infos'][1] . ")</option>\n";  
}

// print header
echo $_SESSION['header'];
// TODO Change enctype of form
?>
		<title>LDAP Account Manager</title>
		<link rel="stylesheet" type="text/css" href="../../style/layout.css">
	</head>
	<body>
		<br>
		<form action="pdfpage.php" method="post">
			<table width="100%">
				<tr>
					<td width="100%" colspan="3" align="center">
						<fieldset>
							<legend>
								<b><?php echo _('Page settings'); ?></b>
							</legend>
							<table width="100%">
								<tr>
									<td>
										<b><?php echo _('Logo'); ?>:</b>
									</td>
									<td>
										<select name="logoFile" size="1">
											<?php echo $logos; ?>
										</select>
									</td>
									<td>
										<?php echo _('Use default') ?> <input type="checkbox" name="defaults[]" value="logoFile">
									</td>
									<td rowspan="4" align="center">
										<input type="submit" name="page" value="<?php echo _('Submit page settings'); ?>">
									</td>
								</tr>
								<tr>
									<td>
									</td>
									<td>
										<p>
											<fieldset>
												<legend>
													<?php echo _('Size'); ?>
												</legend>
											<table width="100%">
												<tr>
													<td>
															<?php echo _('Width') . ':'; ?>
													</td>
													<td>
															<input type="text" name="logo-width" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['logo-width'])) ? $_SESSION['currentPageDefinitions']['logo-width'] : '50'); ?>" size="5"> cm
													</td>
												</tr>
												<tr>
													<td>
															<?php echo _('Height') . ':'; ?>
													</td>
													<td>
															<input type="text" name="logo-height" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['logo-height'])) ? $_SESSION['currentPageDefinitions']['logo-height'] : '20'); ?>" size="5"> cm
													</td>
												</tr>
												<tr>
													<td>
															<input type="checkbox" name="logo-max"<?php echo ((isset($_SESSION['currentPageDefinitions']['logo-max'])) ? ' checked="checked"' : ''); ?>>
													</td>
													<td>
															<?php echo _('Maximize with correct ratio'); ?>
													</td>
												</tr>
											</table>
											</fieldset>
										</p>
									</td>
									<td>
										<?php echo _('Use default') ?> <input type="checkbox" name="defaults[]" value="logoSize">
									</td>
									<!-- <td rowspan="3" align="center">
										<input type="submit" name="page" value="<?php echo _('Submit page settings'); ?>">
									</td> -->
								</tr>
								<tr>
									<td>
										<b><?php echo _('Headline'); ?>:</b>
									</td>
									<td>
										<input type="text" name="headline" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['headline'])) ? $_SESSION['currentPageDefinitions']['headline'] : 'LDAP Account Manager'); ?>">
									</td>
									<td>
										<?php echo _('Use default') ?> <input type="checkbox" name="defaults[]" value="headline">
									</td>
								</tr>
								<tr>
									<td colspan="3">
										<fieldset>
											<legend>
												<?php echo _('Margin'); ?>
											</legend>
											<table width="100%">
												<tr>
													<td>
														<b><?php echo _('Top'); ?>:</b>
													</td>
													<td>
														<input type="text" name="margin-top" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['margin-top'])) ? $_SESSION['currentPageDefinitions']['margin-top'] : '10.0'); ?>" size="5">
													</td>
													<td>
														<?php echo _('Use default') ?> <input type="checkbox" name="defaults[]" value="margin-top">
													</td>
												</tr>
												<tr>
													<td>
														<b><?php echo _('Bottom'); ?>:</b>
													</td>
													<td>
														<input type="text" name="margin-bottom" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['margin-bottom'])) ? $_SESSION['currentPageDefinitions']['margin-bottom'] : '20.0'); ?>" size="5">
													</td>
													<td>
														<?php echo _('Use default') ?> <input type="checkbox" name="defaults[]" value="margin-bottom">
													</td>
												</tr>
												<tr>
													<td>
														<b><?php echo _('Left'); ?>:</b>
													</td>
													<td>
														<input type="text" name="margin-left" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['margin-left'])) ? $_SESSION['currentPageDefinitions']['margin-left'] : '10.0'); ?>" size="5">
													</td>
													<td>
														<?php echo _('Use default') ?> <input type="checkbox" name="defaults[]" value="margin-left">
													</td>
												</tr>
												<tr>
													<td>
														<b><?php echo _('right'); ?>:</b>
													</td>
													<td>
														<input type="text" name="margin-right" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['margin-right'])) ? $_SESSION['currentPageDefinitions']['margin-right'] : '10.0'); ?>" size="5">
													</td>
													<td>
														<?php echo _('Use default') ?> <input type="checkbox" name="defaults[]" value="margin-right">
													</td>
												</tr>
											</table>
										</fieldset>
									</td>
								</tr>
							</table>
						</fieldset>
					</td>
				</tr>
				<tr>
					<td colspan="3">
						<br>
					</td>
				</tr>
				<tr>
					<!-- print current structure -->
					<td width="45%" align="center">
						<fieldset>
							<legend>
								<b><?php echo _("PDF structure"); ?></b>
							</legend>
							<table>
<?php
$sections = '<option value="0">' . _('Beginning') . "</option>\n";
// Print every entry in the current structure
foreach($_SESSION['currentPDFStructure'] as $key => $entry) {
	// Create the up/down/remove links
	$links = "\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;up=" . $key . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Up') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td width=\"10\">\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;down=" . $key . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Down') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td width=\"10\">\n\t\t\t\t\t\t\t\t\t</td>\n\t\t\t\t\t\t\t\t\t<td>\n\t\t\t\t\t\t\t\t\t\t<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;remove=" . $key . (($_GET['edit']) ? 'edit=' . $_GET['edit'] : '') . "\">" . _('Remove') . "</a>\n\t\t\t\t\t\t\t\t\t</td>\n";
	// We have a new section to start
	if($entry['tag'] == "SECTION" && $entry['type'] == "open") {
		$name = $entry['attributes']['NAME'];
		?>
								<tr>
									<td width="20" align="left">
										<input type="radio" name="section" value="<?php echo $key;?>">
									</td>
									<td colspan="2">
		<?php
		// Section headline is a value entry
		if(preg_match("/^\_[a-zA-Z\_]+/",$name)) {
			?>
										<select name="section_<?php echo $key;?>">
											<!-- <?php echo $section_items;?> -->
			<?php
			foreach($section_items_array as $item) {
				?>
											<option value="_<?php echo $item;?>"<?php echo ((substr($name,1) == $item) ? ' selected' : '');?>><?php echo $item;?></option>
				<?php
			}
			?>
										</select>
										<button type="submit" name="change" value="<?php echo $key;?>_item"><?php echo _('Change');?></button> 
			<?php
		}
		// Section headline is a user text
		else {
			?>
										<input type="text" name="section_<?php echo $key;?>" value="<?php echo $name;?>">&nbsp;&nbsp;<button type="submit" name="change" value="<?php echo $key;?>"><?php echo _('Change');?></button>
			<?php
		}
		?>
									</td>
									<td width="20">
									</td>
									<?php echo $links;?>
								</tr>
		<?php
	}
	// We have a section to end
	elseif($entry['tag'] == "SECTION" && $entry['type'] == "close") {
		if(preg_match("/^\_[a-zA-Z\_]+/",$name)) {
			$section_headline = substr($name,1);
		}
		else {
			$section_headline = $name;
		}
		// Add current section for dropdown box needed for the position when inserting a new
		// section or static text entry
		$sections .= '<option value="' . ($key + 1) . '">' . $section_headline . "</option>\n";
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
		// Add current satic text for dropdown box needed for the position when inserting a new
		// section or static text entry
		$sections .= '<option value="' . ($key + 1) . '">' . _('Static text') . "</option>\n";
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
// Print the boxes for adding new sections and static text entries
// Print save and abort buttons
?>
							</table>
							<fieldset>
								<legend>
									<b><?php echo _('Add section or static text');?></b>
								<legend>
								<table width="100%">
									<tr>
										<td width="10">
										</td>
										<td>
											<fieldset style="margin:0px;">
												<legend>
													<?php echo _('Position');?>
												</legend>
												<table width="100%" style="margin:0px;">
													<tr>
														<td>
															<?php echo _('Add after');?>:
														</td>
														<td width="50%">
															<select name="add_position">
																<?php echo $sections;?>
															</select>
														</td>
													</tr>
												</table>
											</fieldset>
										<td>
										<td rowspan="5">
											<input type="submit" name="add" value="<?php echo _('Add');?>">
										</td>
									</tr>
									<tr>
										<td colspan="2">
											<br>
										</td>
									</tr>
									<tr>
										<td valign="center">
											<input type="radio" name="add_type" value="section" checked>
										</td>
										<td>
											<fieldset>
												<legend>
													<b><?php echo _("Section"); ?></b>
												</legend>
 												<table align="left" width="100%"> 
													<tr>
														<td>
															<input type="radio" name="section_type" value="text" checked>
														</td>
														<td colspan="2">
															<input type="text" name="section_text">
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
										</td>
									</tr>
									<tr>
										<td colspan="2">
											<br/>
										</td>
									</tr>
									<tr>
										<td width="10" valign="center">
											<input type="radio" name="add_type" value="text">
										</td>
										<td>
											<fieldset>
												<legend>
													<b><?php echo _("Static text"); ?></b>
												</legend>
												<table width="100%">
													<tr>
														<td width="10">
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
							<?php
							if(!isset($_GET['pdfname'])) {
							?>
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
							<?php
							}
							?>
								<tr>
									<td>
									<?php
									if(isset($_GET['pdfname'])) {
									?>
										<input type="hidden" name="pdfname" value="<?php echo $_GET['pdfname']; ?>">
									<?php
									}
									?>
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
// Print all available modules with the value fieds
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
	// Print each value field
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