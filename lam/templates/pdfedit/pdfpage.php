<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Michael Duergner
                2007 - 2010  Roland Gruber

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

/**
* Displays the main page of the PDF editor where the user can select the displayed entries.
*
* @author Michael Duergner
* @author Roland Gruber
* @package PDF
*/

/** security functions */
include_once("../../lib/security.inc");
/** access to PDF configuration files */
include_once('../../lib/pdfstruct.inc');
/** LDAP object */
include_once('../../lib/ldap.inc');
/** LAM configuration */
include_once('../../lib/config.inc');
/** module functions */
include_once('../../lib/modules.inc');
/** XML functions */
include_once('../../lib/xml_parser.inc');

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

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

// set new logo and headline
if ((isset($_GET['headline'])) && ($_GET['logoFile'] != $_SESSION['currentPageDefinitions']['filename'])) {
	$_SESSION['currentPageDefinitions']['filename'] = $_GET['logoFile'];
}
if ((isset($_GET['headline'])) && ($_GET['headline'] != $_SESSION['currentPageDefinitions']['headline'])) {
	$_SESSION['currentPageDefinitions']['headline'] = str_replace('<','',str_replace('>','',$_GET['headline']));
}

// Check if pdfname is valid, then save current structure to file and go to
// main pdf structure page
$saveErrors = array();
if(isset($_GET['submit'])) {
	if(!isset($_GET['pdfname']) || !preg_match('/[a-zA-Z0-9\-\_]+/',$_GET['pdfname'])) {
		$saveErrors[] = array('ERROR', _('PDF structure name not valid'), _('The name for that PDF-structure you submitted is not valid. A valid name must consist of the following characters: \'a-z\',\'A-Z\',\'0-9\',\'_\',\'-\'.'));
	}
	else {
		$return = savePDFStructureDefinitions($_GET['type'],$_GET['pdfname']);
		if($return == 'ok') {
			metaRefresh('pdfmain.php?savedSuccessfully=' . $_GET['pdfname']);
			exit;
		} 
		elseif($return == 'no perms'){
			$saveErrors[] = array('ERROR', _("Could not save PDF structure, access denied."), $_GET['pdfname']);
		}
	}
}
// add a new text field
elseif(isset($_GET['add_text'])) {
	// Check if text for static text field is specified
	if(!isset($_GET['text_text']) || ($_GET['text_text'] == '')) {
		StatusMessage('ERROR',_('No static text specified'),_('The static text must contain at least one character.'));
	}
	else {
		$entry = array(array('tag' => 'TEXT','type' => 'complete','level' => '2','value' => $_GET['text_text']));
		// Insert new field in structure
		array_splice($_SESSION['currentPDFStructure'],$_GET['add_text_position'],0,$entry);
	}
}
// add a new section with text headline
elseif(isset($_GET['add_sectionText'])) {
	// Check if name for new section is specified when needed
	if(!isset($_GET['section_text']) || ($_GET['section_text'] == '')) {
		StatusMessage('ERROR',_('No section text specified'),_('The headline for a new section must contain at least one character.'));
	}
	else {
		$attributes = array();		
		$attributes['NAME'] = $_GET['section_text'];
		$entry = array(array('tag' => 'SECTION','type' => 'open','level' => '2','attributes' => $attributes),array('tag' => 'SECTION','type' => 'close','level' => '2'));
		// Insert new field in structure
		array_splice($_SESSION['currentPDFStructure'],$_GET['add_sectionText_position'],0,$entry);
	}
}
// Add a new section with item as headline
elseif(isset($_GET['add_section'])) {
		$attributes = array();
		$attributes['NAME'] = '_' . $_GET['section_item'];
		$entry = array(array('tag' => 'SECTION','type' => 'open','level' => '2','attributes' => $attributes),array('tag' => 'SECTION','type' => 'close','level' => '2'));
		// Insert new field in structure
		array_splice($_SESSION['currentPDFStructure'],$_GET['add_section_position'],0,$entry);
}
// Add a new value field
elseif(isset($_GET['add_new_field'])) {
	$field = array('tag' => 'ENTRY','type' => 'complete','level' => '3','attributes' => array('NAME' => $_GET['new_field']));
	$pos = 0;
	// Find begin section to insert into
	while($pos < $_GET['add_field_position']) {
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
	array_splice($_SESSION['currentPDFStructure'],$pos,0,array($field));
}
// Change section headline
elseif(isset($_GET['change'])) {
	$alter = explode('_',$_GET['change']);
	$newvalue = $_GET['section_' . $alter[0]];
	if (isset($alter[1]) && ($alter[1] == 'item') && ($newvalue[0] != '_')) {
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

// Load PDF structure from file if it is not defined in session
if(!isset($_SESSION['currentPDFStructure'])) {
	// Load structure file to be edit
	if(isset($_GET['edit'])) {
		$load = loadPDFStructureDefinitions($_GET['type'],$_GET['edit']);
		$_SESSION['currentPDFStructure'] = $load['structure'];
		$_SESSION['currentPageDefinitions'] = $load['page_definitions'];
		$_GET['pdfname'] = $_GET['edit'];
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
$sortedModules = array();
foreach($_SESSION['availablePDFFields'] as $module => $fields) {
	if ($module != 'main') {
		$title = getModuleAlias($module, $_GET['type']);
	}
	else {
		$title = _('Main');
	}
	$sortedModules[$module] = $title;
}
natcasesort($sortedModules);
foreach($sortedModules as $module => $title) {
	$values = $_SESSION['availablePDFFields'][$module];
	if (!is_array($values) || (sizeof($values) < 1)) {
		continue;
	}
	$modules[] = $module;
	$section_items .= "<optgroup label=\"" . $title . "\"\n>";
	natcasesort($values);
	foreach($values as $attribute => $attributeLabel) {
		$section_items_array[] = $module . '_' . $attribute;
		$section_items .= "<option value=\"" . $module . '_' . $attribute . "\">" . $attributeLabel . "</option>\n";
	}
	$section_items .= "</optgroup>\n";
}
$modules = join(',',$modules);

$logoFiles = getAvailableLogos();
$logos = '<option value="none"' . (($_SESSION['currentPageDefinitions']['filename'] == 'none') ? ' selected="selected"' : '') . '>' . _('No logo') . "</option>\n";
foreach($logoFiles as $logoFile) {
	$logos .= "<option value=\"" . $logoFile['filename'] . "\"" .(($_SESSION['currentPageDefinitions']['filename'] == $logoFile['filename'] || (!isset($_SESSION['currentPageDefinitions']['filename']) && $logoFile['filename'] == 'printLogo.jpg')) ? ' selected="selected"' : '') . '>' . $logoFile['filename'] . ' (' . $logoFile['infos'][0] . ' x ' . $logoFile['infos'][1] . ")</option>\n";  
}

// print header
include '../main_header.php';

// print error messages if any
if (sizeof($saveErrors) > 0) {
	for ($i = 0; $i < sizeof($saveErrors); $i++) {
		call_user_func_array('StatusMessage', $saveErrors[$i]);
	}
	echo "<br>\n";
}

?>
		<br>
		<form action="pdfpage.php" method="post">
			<table>
				<tr><td>
			<table width="100%">
				<tr><td align="left">
					<?php echo _("Structure name"); ?>:
					<?php
						$structureName = '';
						if (isset($_GET['edit'])) {
							$structureName = $_GET['edit'];
						}
						elseif (isset($_GET['pdfname'])) {
							$structureName = $_GET['pdfname'];
						}
						else if (isset($_POST['pdfname'])) {
							$structureName = $_POST['pdfname'];
						}
					?>
					<input type="text" name="pdfname" value="<?php echo $structureName;?>">
					<?php
						printHelpLink(getHelp('', '360'), '360');
					?>
					</td><td align="right">
					<button id="saveButton" name="submit"><?php echo _("Save");?></button>
					&nbsp;
					<button id="cancelButton" name="abort"><?php echo _("Cancel");?></button>
					<script type="text/javascript">
						jQuery(document).ready(function() {
							jQuery('#saveButton').button({
						        icons: {
						      	  primary: 'saveButton'
						    	}
							});
							jQuery('#cancelButton').button({
						        icons: {
						    	  primary: 'cancelButton'
						  	}
							});
						});
					</script>
				</td></tr>
			</table>
			</td></tr><tr>
					<!-- print current structure -->
					<td align="left" valign="top">
						<fieldset class="<?php echo $_GET['type']; ?>edit">
							<legend>
								<b><?php if (isset($_GET['pdfname'])) echo $_GET['pdfname']; ?></b>
							</legend>
							<BR>
							<b><?php echo _('Headline'); ?>:</b>
							<input type="text" name="headline" value="<?php echo ((isset($_SESSION['currentPageDefinitions']['headline'])) ? $_SESSION['currentPageDefinitions']['headline'] : 'LDAP Account Manager'); ?>">
							&nbsp;&nbsp;&nbsp;&nbsp;
							<b><?php echo _('Logo'); ?>:</b>
							<select name="logoFile" size="1">
								<?php echo $logos; ?>
							</select>
							<BR><HR><BR>
							<table width="100%">
<?php
$sections = '<option value="0">' . _('Beginning') . "</option>\n";
$nonTextSections = '';
// Print every entry in the current structure
foreach($_SESSION['currentPDFStructure'] as $key => $entry) {
	// Create the up/down/remove links
	$links = "<td width=18>\n<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;up=" . $key .
			(isset($_GET['pdfname']) ? '&amp;pdfname=' . $_GET['pdfname'] : '') .
			(isset($_GET['headline']) ? '&amp;headline=' . $_GET['headline'] : '') .
			(isset($_GET['logoFile']) ? '&amp;logoFile=' . $_GET['logoFile'] : '') . "\">" .
		"<img src=\"../../graphics/up.gif\" alt=\"" . _("Up") . "\" border=\"0\"></a>\n</td>\n" .
		"<td width=18>\n<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;down=" . $key .
			(isset($_GET['pdfname']) ? '&amp;pdfname=' . $_GET['pdfname'] : '') .
			(isset($_GET['headline']) ? '&amp;headline=' . $_GET['headline'] : '') .
			(isset($_GET['logoFile']) ? '&amp;logoFile=' . $_GET['logoFile'] : '') . "\">" .
		"<img src=\"../../graphics/down.gif\" alt=\"" . _("Down") . "\" border=\"0\"></a>\n</td>\n" .
		"<td width=18>\n<a href=\"pdfpage.php?type=" . $_GET['type'] . "&amp;remove=" . $key .
			(isset($_GET['pdfname']) ? '&amp;pdfname=' . $_GET['pdfname'] : '') .
			(isset($_GET['headline']) ? '&amp;headline=' . $_GET['headline'] : '') .
			(isset($_GET['logoFile']) ? '&amp;logoFile=' . $_GET['logoFile'] : '') . "\">" .
		"<img src=\"../../graphics/delete.gif\" alt=\"" . _("Remove") . "\" border=\"0\"></a>\n</td>\n" .
		"<td width=\"100%\">&nbsp;</td>";
	// We have a new section to start
	if($entry['tag'] == "SECTION" && $entry['type'] == "open") {
		$name = $entry['attributes']['NAME'];
		if(preg_match("/^_[a-zA-Z0-9_]+_[a-zA-Z0-9_]+/",$name)) {
			$section_headline = translateFieldIDToName(substr($name,1), $_GET['type']);
		}
		else {
			$section_headline = $name;
		}
		$nonTextSections .= '<option value="' . $key . '">' . $section_headline . "</option>\n";
		$sections .= '<option value="' . ($key) . '">' . $section_headline . "</option>\n";
		?>
								<tr>
									<td nowrap colspan="2">
		<?php
		// Section headline is a value entry
		if(preg_match("/^_[a-zA-Z0-9_]+_[a-zA-Z0-9_]+/",$name)) {
			?>
										<select name="section_<?php echo $key;?>">
			<?php
			foreach($section_items_array as $item) {
				?>
											<option value="_<?php echo $item;?>"<?php echo ((substr($name,1) == $item) ? ' selected' : '');?>><?php echo translateFieldIDToName($item, $_GET['type']);?></option>
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
										<input type="text" name="section_<?php echo $key;?>" value="<?php echo $section_headline;?>">&nbsp;&nbsp;<button type="submit" name="change" value="<?php echo $key;?>"><?php echo _('Change');?></button>
			<?php
		}
		?>
									</td>
									<td width="20">&nbsp;</td>
									<?php echo $links;?>
								</tr>
		<?php
	}
	// We have a section to end
	elseif($entry['tag'] == "SECTION" && $entry['type'] == "close") {
		?>
								<tr>
									<td colspan="7">
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
									<td nowrap colspan="2" width="400">
										<b><?php echo _('Static text');?></b>
									</td>
									<td width="20">
									</td>
									<?php echo $links;?>
								</tr>
								<tr>
									<td colspan="7">
										<br>
									</td>
								</tr>
								<tr>
									<td>
									</td>
									<td  colspan="6">
										<?php echo $entry['value'];?>
									</td>
								</tr>
								<tr>
									<td colspan="7">
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
									<td width="20">
									</td>
									<td>
										<?php echo translateFieldIDToName($name, $_GET['type']);?>
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
						</fieldset>
						<p>&nbsp;</p>
					</td>
					
				</tr>
				<tr>
					<td colspan="3">
							<fieldset class="<?php echo $_GET['type']; ?>edit">
								<legend>
									<b><?php echo _('New section');?></b>
								</legend><BR>
								<table>
									<tr>
										<td>
											<fieldset class="<?php echo $_GET['type']; ?>edit">
												<legend>
													<b><?php echo _("Section"); ?></b>
												</legend>
 												<table align="left" width="100%"> 
													<tr>
														<td>
															<?php echo _("Headline"); ?>: <input type="text" name="section_text">&nbsp;&nbsp;&nbsp;
														</td>
														<td>
															<B><?php echo _('Position');?>: </B>
															<select name="add_sectionText_position">
																<?php echo $sections;?>
															</select>
														</td>
														<td>
															<input type="submit" name="add_sectionText" value="<?php echo _('Add');?>">
														</td>
													</tr>
													<tr>
														<td>
															<?php echo _("Headline"); ?>: 
															<select name="section_item">
																<?php echo $section_items;?>
															</select>&nbsp;&nbsp;&nbsp;
														</td>
														<td>
															<B><?php echo _('Position');?>: </B>
															<select name="add_section_position">
																<?php echo $sections;?>
															</select>
														</td>
														<td>
															<input type="submit" name="add_section" value="<?php echo _('Add');?>">
														</td>
													</tr>
												</table>
											</fieldset>
										</td>
									</tr>
									<tr>
										<td colspan="2">&nbsp;</td>
									</tr>
									<tr>
										<td>
											<fieldset class="<?php echo $_GET['type']; ?>edit">
												<legend>
													<b><?php echo _("Text field"); ?></b>
												</legend>
												<table width="100%">
													<tr>
														<td>
															<textarea name="text_text" rows="3" cols="40"></textarea>&nbsp;&nbsp;&nbsp;
														</td>
														<td>
															<B><?php echo _('Position');?>: </B>
															<select name="add_text_position">
																<?php echo $sections;?>
															</select>
														</td>
														<td>
															<input type="submit" name="add_text" value="<?php echo _('Add');?>">
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
							<fieldset class="<?php echo $_GET['type']; ?>edit">
								<legend>
									<b><?php echo _('New field');?></b>
								</legend><BR>
								<select name="new_field">
								<?php
									foreach($sortedModules as $module => $title) {
										$fields = $_SESSION['availablePDFFields'][$module];
										if (isset($fields) && is_array($fields) && (sizeof($fields) > 0)) {
											natcasesort($fields);
											echo "<optgroup label=\"$title\">\n";
											foreach ($fields as $field => $fieldLabel) {
												echo "<option value=\"" . $module . "_" . $field . "\" label=\"$fieldLabel\">$fieldLabel</option>\n";
											}
											echo "</optgroup>\n";
										}
									}
								?>
								</select>
								<B><?php echo _('Position');?>: </B>
								<select name="add_field_position">
									<?php echo $nonTextSections;?>
								</select>
								<input type="submit" name="add_new_field" value="<?php echo _('Add');?>">
							</fieldset>
					</td>
				</tr>
			</table>
		<input type="hidden" name="modules" value="<?php echo $modules;?>">
		<input type="hidden" name="type" value="<?php echo $_GET['type'];?>">
	</form>

<?php
include '../main_footer.php';


/**
 * Translates a given field ID (e.g. inetOrgPerson_givenName) to its descriptive name.
 *
 * @param String $id field ID
 * @param String $scope account type
 */
function translateFieldIDToName($id, $scope) {
	foreach ($_SESSION['availablePDFFields'] as $module => $fields) {
		if (!(strpos($id, $module . '_') === 0)) {
			continue;
		}
		foreach ($fields as $name => $label) {
			if ($id == $module . '_' . $name) {
				if ($module == 'main') {
					return _('Main') . ': ' . $label;
				}
				else  {
					return getModuleAlias($module, $scope) . ': ' . $label;
				}
			}
		}
	}
	return $id;
}

?>
