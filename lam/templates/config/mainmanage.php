<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2010  Roland Gruber

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
* Manages the main configuration options.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');
/** Used to print status messages */
include_once('../../lib/status.inc');

// start session
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
@session_start();

setlanguage();

$cfg = new LAMCfgMain();

// check if user is logged in
if (!isset($_SESSION["mainconf_password"]) || (!$cfg->checkPassword($_SESSION["mainconf_password"]))) {
	require('mainlogin.php');
	exit();
}

if (isset($_POST['cancel'])) {
	// back to login
	metaRefresh('../login.php');
	exit();
}

$errors = array();
// check if submit button was pressed
if (isset($_POST['submit'])) {
	// remove double slashes if magic quotes are on
	if (get_magic_quotes_gpc() == 1) {
		$postKeys = array_keys($_POST);
		for ($i = 0; $i < sizeof($postKeys); $i++) {
			if (is_string($_POST[$postKeys[$i]])) $_POST[$postKeys[$i]] = stripslashes($_POST[$postKeys[$i]]);
		}
	}
	// set master password
	if (isset($_POST['masterpassword']) && ($_POST['masterpassword'] != "")) {
		if ($_POST['masterpassword'] && $_POST['masterpassword2'] && ($_POST['masterpassword'] == $_POST['masterpassword2'])) {
			$cfg->setPassword($_POST['masterpassword']);
			$msg = _("New master password set successfully.");
			unset($_SESSION["mainconf_password"]);
		}
		else $errors[] = _("Master passwords are different or empty!");
	}
	// set session timeout
	$cfg->sessionTimeout = $_POST['sessionTimeout'];
	// set allowed hosts
	if (isset($_POST['allowedHosts'])) {
		$allowedHosts = $_POST['allowedHosts'];
		$allowedHostsList = explode("\n", $allowedHosts);
		for ($i = 0; $i < sizeof($allowedHostsList); $i++) {
			$allowedHostsList[$i] = trim($allowedHostsList[$i]);
			// ignore empty lines
			if ($allowedHostsList[$i] == "") {
				unset($allowedHostsList[$i]);
				continue;
			}
			// check each line
			$ipRegex = '/^[0-9\\.\\*]+$/';
			if (!preg_match($ipRegex, $allowedHostsList[$i]) || (strlen($allowedHostsList[$i]) > 15)) {
				$errors[] = sprintf(_("The IP address %s is invalid!"), $allowedHostsList[$i]);
			}
		}
		$allowedHosts = implode(",", $allowedHostsList);
	}
	else $allowedHosts = "";
	$cfg->allowedHosts = $allowedHosts;
	// set log level
	$cfg->logLevel = $_POST['logLevel'];
	// set log destination
	if ($_POST['logDestination'] == "none") $cfg->logDestination = "NONE";
	elseif ($_POST['logDestination'] == "syslog") $cfg->logDestination = "SYSLOG";
	else {
		if (isset($_POST['logFile']) && ($_POST['logFile'] != "") && preg_match("/^[a-z0-9\\/\\\\:\\._-]+$/i", $_POST['logFile'])) {
			$cfg->logDestination = $_POST['logFile'];
		}
		else $errors[] = _("The log file is empty or contains invalid characters! Valid characters are: a-z, A-Z, 0-9, /, \\, ., :, _ and -.");
	}
	// password policies
	$cfg->passwordMinLength = $_POST['passwordMinLength'];
	$cfg->passwordMinLower = $_POST['passwordMinLower'];
	$cfg->passwordMinUpper = $_POST['passwordMinUpper'];
	$cfg->passwordMinNumeric = $_POST['passwordMinNumeric'];
	$cfg->passwordMinSymbol = $_POST['passwordMinSymbol'];
	$cfg->passwordMinClasses = $_POST['passwordMinClasses'];
	// save settings
	$cfg->save();
	if (sizeof($errors) == 0) {
		metaRefresh('../login.php?confMainSavedOk=1');
		exit();
	}
}

echo $_SESSION['header'];

?>

		<title>
			<?php
				echo _("Edit general settings");
			?>
		</title>
	<?php 
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../../style';
		$cssDir = dir($cssDirName);
		while ($cssEntry = $cssDir->read()) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) != '.css') continue;
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
		}
	?>
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
	</head>
	<body>
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamHeader" href="http://www.ldap-account-manager.org/" target="new_window">&nbsp;<img src="../../graphics/logo32.png" width=24 height=24 class="align-middle" alt="LDAP Account Manager">&nbsp;&nbsp;LDAP Account Manager</a>
				</td>
				<td align="right" height=20>
					<a href="../login.php"><IMG alt="configuration" src="../../graphics/undo.png">&nbsp;<?php echo _("Back to login") ?></a>
				</td>
			</tr>
		</table>
		<br>

<?php
// include all JavaScript files
$jsDirName = dirname(__FILE__) . '/../lib';
$jsDir = dir($jsDirName);
$jsFiles = array();
while ($jsEntry = $jsDir->read()) {
	if (substr($jsEntry, strlen($jsEntry) - 3, 3) != '.js') continue;
	$jsFiles[] = $jsEntry;
}
sort($jsFiles);
foreach ($jsFiles as $jsEntry) {
	echo "<script type=\"text/javascript\" src=\"../lib/" . $jsEntry . "\"></script>\n";
}

// print messages
for ($i = 0; $i < sizeof($errors); $i++) {
	StatusMessage("ERROR", $errors[$i]);
}

// check if config file is writable
if (!$cfg->isWritable()) {
	StatusMessage('WARN', 'The config file is not writable.', 'Your changes cannot be saved until you make the file writable for the webserver user.');
}
?>

		<br>
		<!-- form for adding/renaming/deleting profiles -->
		<form action="mainmanage.php" method="post">
		<table border="0" align="center">
		<tr><td>
		<fieldset>
			<legend> <?php echo _("Security settings"); ?> </legend>
			<br>
			<table cellspacing="0" border="0">
				<!-- session timeout -->
				<tr>
					<td align="left">
						<?php echo _("Session timeout"); ?>
					</td>
					<td>
						<SELECT name="sessionTimeout">
						<?php
						$options = array(5, 10, 20, 30, 60);
						for ($i = 0; $i < sizeof($options); $i++) {
							if ($cfg->sessionTimeout == $options[$i]) {
								echo "<option selected>" . $cfg->sessionTimeout . "</option>";
							}
							else {
								echo "<option>" . $options[$i] . "</option>";
							}
						}
						?>
						</SELECT>
					</td>
					<td>&nbsp;
					<?PHP
						printHelpLink(getHelp('', '238'), '238');
					?>
					</td>
				</tr>
				<!-- allowed hosts -->
				<tr>
					<td align="left">
						<?php echo _("Allowed hosts"); ?>
					</td>
					<td>
						<TEXTAREA cols="30" rows="7" name="allowedHosts"><?php echo implode("\n", explode(",", $cfg->allowedHosts)); ?></TEXTAREA>
					</td>
					<td>&nbsp;
					<?PHP
						printHelpLink(getHelp('', '241'), '241');
					?>
					</td>
				</tr>
			</table>
		</fieldset>
		<BR>
		<fieldset>
			<legend> <?php echo _("Password policy"); ?> </legend>
			<br>
			<table cellspacing="0" border="0">
			<?php
				$options = array(
					array('passwordMinLength', _('Minimum password length'), 20),
					array('passwordMinLower', _('Minimum lowercase characters'), 20),
					array('passwordMinUpper', _('Minimum uppercase characters'), 20),
					array('passwordMinNumeric', _('Minimum numeric characters'), 20),
					array('passwordMinSymbol', _('Minimum symbolic characters'), 20),
					array('passwordMinClasses', _('Minimum character classes'), 4)
				);
				for ($i = 0; $i < sizeof($options); $i++) {
					echo "<tr>\n";
						echo "<td>\n";
							echo $options[$i][1] . "&nbsp;&nbsp;";
						echo "</td>\n";
						echo "<td>\n";
							echo "<select name=\"" . $options[$i][0] . "\">\n";
								for ($o = 0; $o <= $options[$i][2]; $o++) {
									$selected = '';
									if ($cfg->$options[$i][0] == $o) {
										$selected = ' selected';
									}
									echo "<option" . $selected . ">" . $o . "</option>\n";
								}
							echo "</select>\n";
						echo "</td>\n";
						echo "<td>\n";
							printHelpLink(getHelp('', '242'), '242');
						echo "</td>\n";
					echo "</tr>\n";
				}
			?>
			</table>
			<br>
		</fieldset>
		<BR>
		<fieldset>
			<legend> <?php echo _("Logging"); ?> </legend>
			<br>
			<table cellspacing="0" border="0">
				<!-- log level -->
				<tr>
					<td>
						<?php echo _("Log level"); ?>
						<SELECT name="logLevel">
						<?php
						$options = array(_("Debug") => LOG_DEBUG, _("Notice") => LOG_NOTICE, _("Warning") => LOG_WARNING, _("Error") => LOG_ERR);
						foreach ($options as $key => $value) {
							if ($cfg->logLevel == $value) {
								echo "<option selected value=\"" . $value . "\">" . $key . "</option>";
							}
							else {
								echo "<option value=\"" . $value . "\">" . $key . "</option>";
							}
						}
						?>
						</SELECT>
					</td>
					<td>&nbsp;
					<?PHP
						printHelpLink(getHelp('', '239'), '239');
					?>
					</td>
				</tr>
				<TR><TD colspan="2">&nbsp;</TD></TR>
				<TR>
					<TD>
						<?PHP
							echo _("Log destination") . ":";
						?>
					</TD>
					<TD>&nbsp;
					<?PHP
						printHelpLink(getHelp('', '240'), '240');
					?>
					</TD>
				</TR>
				<TR>
					<TD colspan="2">
					<?PHP
						$noLogChecked = false;
						if ($cfg->logDestination == "NONE") $noLogChecked = true;
						echo "<input type=\"radio\" name=\"logDestination\" value=\"none\"";
						if ($noLogChecked) echo " checked";
						echo ">" . _("No logging") . "\n";
					?>
					</TD>
				</TR>
				<TR>
					<TD colspan="2">
					<?PHP
						$syslogChecked = false;
						if ($cfg->logDestination == "SYSLOG") {
							$syslogChecked = true;
						}
						echo "<input type=\"radio\" name=\"logDestination\" value=\"syslog\"";
						if ($syslogChecked) echo " checked";
						echo ">" . _("System logging") . "\n";
					?>
					</TD>
				</TR>
				<TR>
					<TD colspan="2">
					<?PHP
						$logFile = "";
						$logFileChecked = false;
						if (($cfg->logDestination != "NONE") && ($cfg->logDestination != "SYSLOG")) {
							$logFile = $cfg->logDestination;
							$logFileChecked = true;
						}
						echo "<input type=\"radio\" name=\"logDestination\" value=\"file\"";
						if ($logFileChecked) echo " checked";
						echo ">" . _("File") . "\n";
						echo "<input type=\"text\" name=\"logFile\" value=\"" . $logFile . "\">\n";
					?>
					</TD>
				</TR>
			</table>
		</fieldset>
		<BR>
		<fieldset>
			<legend> <?php echo _("Change master password"); ?> </legend>
			<br>
			<table cellspacing="0" border="0">
				<!-- set master password -->
				<tr>
					<td align="right">
						<FONT color="Red"><B>
						<?php echo _("New master password"); ?>
						</B></FONT>
						<input type="password" name="masterpassword">
					</td>
					<td>&nbsp;
					<?PHP
						printHelpLink(getHelp('', '235'), '235');
					?>
					</td>
				</tr>
				<tr>
					<td align="right">
						<FONT color="Red"><B>
						<?php echo _("Reenter new master password"); ?>
						</B></FONT>
						<input type="password" name="masterpassword2">
					</td>
					<td>&nbsp;</td>
				</tr>

			</table>
			</fieldset>
			</td></tr>
			<TR>
				<TD>
					<BR>
					<?php if ($cfg->isWritable()) { ?>
					<button id="submitButton" name="submit" class="smallPadding"><?php echo _("Ok"); ?></button>
					<?php } ?>
					<button id="cancelButton" name="cancel" class="smallPadding"><?php echo _("Cancel"); ?></button>
					<script type="text/javascript" language="javascript">
					jQuery(document).ready(function() {
						jQuery('#submitButton').button();
						jQuery('#cancelButton').button();
					});
					</script>
				</TD>
			</TR>
			</table>

		</form>
		<p><br></p>

	</body>
</html>

