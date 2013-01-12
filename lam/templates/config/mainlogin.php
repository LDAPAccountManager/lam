<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2013  Roland Gruber

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
* Login page to change the main preferences.
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
	session_save_path(dirname(__FILE__) . '/../../sess');
}
session_start();
session_regenerate_id(true);

setlanguage();

// remove settings from session
if (isset($_SESSION["mainconf_password"])) unset($_SESSION["mainconf_password"]);

$cfgMain = new LAMCfgMain();
// check if user entered a password
if (isset($_POST['passwd'])) {
	if (isset($_POST['passwd']) && ($cfgMain->checkPassword($_POST['passwd']))) {
		$_SESSION["mainconf_password"] = $_POST['passwd'];
		metaRefresh("mainmanage.php");
		exit();
	}
	else {
		$message = _("The password is invalid! Please try again.");
	}
}


echo $_SESSION['header'];

?>

		<title>
			<?php
				echo _("Login");
			?>
		</title>
	<?php 
		// include all CSS files
		$cssDirName = dirname(__FILE__) . '/../../style';
		$cssDir = dir($cssDirName);
		$cssFiles = array();
		$cssEntry = $cssDir->read();
		while ($cssEntry !== false) {
			if (substr($cssEntry, strlen($cssEntry) - 4, 4) == '.css') {
				$cssFiles[] = $cssEntry;
			}
			$cssEntry = $cssDir->read();
		}
		sort($cssFiles);
		foreach ($cssFiles as $cssEntry) {
			echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/" . $cssEntry . "\">\n";
		}
	?>
		<link rel="shortcut icon" type="image/x-icon" href="../../graphics/favicon.ico">
	</head>
	<body>
		<?php
			// set focus on password field
			echo "<script type=\"text/javascript\" language=\"javascript\">\n";
			echo "<!--\n";
			echo "window.onload = function() {\n";
				echo "loginField = document.getElementsByName('passwd')[0];\n";
				echo "loginField.focus();\n";
			echo "}\n";
			echo "//-->\n";
			echo "</script>\n";
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
		?>
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
			// check if config file is writable
			if (!$cfgMain->isWritable()) {
				StatusMessage('WARN', 'The config file is not writable.', 'Your changes cannot be saved until you make the file writable for the webserver user.');
			}
		?>
		<br>
		<!-- form to change main options -->
		<form action="mainlogin.php" method="post">
		<table align="center" border="2" rules="none" bgcolor="white" class="ui-corner-all" style="padding: 20px;">
		<tr><td>
		<?php
		$spacer = new htmlSpacer('20px', '20px');
		$table = new htmlTable();
		$caption = new htmlOutputText(_("Please enter the master password to change the general preferences:"));
		$table->addElement($caption, true);
		$table->addElement($spacer, true);
		// print message if login was incorrect or no config profiles are present
		if (isset($message)) {  // $message is set by confmain.php (requires conflogin.php then)
			$messageField = new htmlStatusMessage('ERROR', $message);
			$table->addElement($messageField, true);
			$table->addElement($spacer, true);
		}
		// password field
		$gap = new htmlSpacer('1px', null);
		$passwordGroup = new htmlGroup();
		$passwordGroup->alignment = htmlElement::ALIGN_CENTER;
		$passwordGroup->addElement(new htmlOutputText(_('Master password')));
		$passwordGroup->addElement($gap);
		$passwordField = new htmlInputField('passwd');
		$passwordField->setFieldSize(15);
		$passwordField->setIsPassword(true);
		$passwordGroup->addElement($passwordField);
		$passwordGroup->addElement($gap);
		$passwordGroup->addElement(new htmlButton('submit', _("Ok")));
		$passwordGroup->addElement($gap);
		$passwordGroup->addElement(new htmlHelpLink('236'));
		$table->addElement($passwordGroup, true);
		
		
		$tabindex = 1;
		parseHtml(null, $table, array(), false, $tabindex, 'user');
		?>
		</td></tr>
		</table>
		</form>

		<p><br><br></p>


	</body>
</html>
