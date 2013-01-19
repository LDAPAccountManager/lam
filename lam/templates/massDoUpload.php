<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2012  Roland Gruber

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
* Creates LDAP accounts for file upload.
*
* @author Roland Gruber
* @package tools
*/

/** security functions */
include_once("../lib/security.inc");
/** access to configuration */
include_once('../lib/config.inc');
/** LDAP handle */
include_once('../lib/ldap.inc');
/** status messages */
include_once('../lib/status.inc');
/** account modules */
include_once('../lib/modules.inc');
/** PDF */
include_once('../lib/pdf.inc');


// Start session
startSecureSession();

// check if this tool may be run
checkIfToolIsActive('toolFileUpload');

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

// Redirect to startpage if user is not loged in
if (!isset($_SESSION['loggedIn']) || ($_SESSION['loggedIn'] !== true)) {
	metaRefresh("login.php");
	exit;
}

// Set correct language, codepages, ....
setlanguage();

include 'main_header.php';
$scope = htmlspecialchars($_SESSION['mass_scope']);

// check if account type is ok
if (isAccountTypeHidden($scope)) {
	logNewMessage(LOG_ERR, 'User tried to access hidden upload: ' . $scope);
	die();
}

echo '<div class="' . $scope . '-bright smallPaddingContent">';

// create accounts
$accounts = unserialize($_SESSION['ldap']->decrypt($_SESSION['mass_accounts']));
if (($_SESSION['mass_counter'] < sizeof($accounts)) || !isset($_SESSION['mass_postActions']['finished']) || !isset($_SESSION['mass_pdf']['finished'])) {
	$startTime = time();
	$maxTime = get_cfg_var('max_execution_time') - 5;
	if ($maxTime > 60) $maxTime = 60;
	if ($maxTime <= 0) $maxTime = 60;
	echo "<div class=\"title\">\n";
	echo "<h2 class=\"titleText\">" . _("LDAP upload in progress. Please wait.") . "</h2>\n";
	echo "</div>";
	$progress = ($_SESSION['mass_counter'] * 100) / sizeof($accounts);
	?>
		<div id="progressbarGeneral"></div>
		<script type="text/javascript">
			$(function() {
				$( "#progressbarGeneral" ).progressbar({
					value: <?php echo $progress; ?>
				});
			});
		</script>
	<?php
	flush();  // send HTML to browser
	// add accounts to LDAP
	while (($_SESSION['mass_counter'] < sizeof($accounts)) && (($startTime + $maxTime) > time())) {
		// create accounts as long as max_execution_time is not near
		$attrs = $accounts[$_SESSION['mass_counter']];
		$dn = $attrs['dn'];
		unset($attrs['dn']);
		// remove informational attributes
		foreach ($attrs as $key => $value) {
			if (strpos($key, 'INFO.') === 0) {
				unset($attrs[$key]);
			}
		}
		// run preactions
		$preAttributes = array();
		foreach ($attrs as $key => $value) {
			$preAttributes[$key] = &$attrs[$key];
		}
		$preAttributes['dn'] = &$dn;
		$preMessages = doUploadPreActions($scope, $_SESSION['mass_selectedModules'], $preAttributes);
		$preActionOk = true;
		for ($i = 0; $i < sizeof($preMessages); $i++) {
			if (($preMessages[$i][0] == 'ERROR') || ($preMessages[$i][0] == 'WARN')) {
				$preActionOk = false;
				$_SESSION['mass_errors'][] = $preMessages[$i];
			}
		}
		if ($preActionOk) {
			// add LDAP entry
			$success = @ldap_add($_SESSION['ldap']->server(), $dn, $attrs);
			if (!$success) {
				$errorMessage = array(
					"ERROR",
					_("LAM was unable to create account %s! An LDAP error occured."),
					ldap_errno($_SESSION['ldap']->server()) . ": " . ldap_error($_SESSION['ldap']->server()),
					array($_SESSION['mass_counter']));
				$_SESSION['mass_errors'][] = $errorMessage;
				$_SESSION['mass_failed'][] = $_SESSION['mass_counter'];
			}
		}
		$_SESSION['mass_counter']++;
	}
	$progress = ($_SESSION['mass_counter'] * 100) / sizeof($accounts);
	?>
		<script type="text/javascript">
			$(function() {
				$( "#progressbarGeneral" ).progressbar({
					value: <?php echo $progress; ?>
				});
			});
		</script>
	<?php
	flush();  // send HTML to browser
	// do post upload actions after all accounts are created
	if (($_SESSION['mass_counter'] >= sizeof($accounts)) && !isset($_SESSION['mass_postActions']['finished'])) {
		$data = unserialize($_SESSION['ldap']->decrypt($_SESSION['mass_data']));
		$return  = doUploadPostActions($scope, $data, $_SESSION['mass_ids'], $_SESSION['mass_failed'], $_SESSION['mass_selectedModules'], $accounts);
		if ($return['status'] == 'finished') {
			$_SESSION['mass_postActions']['finished'] = true;
		}
		for ($i = 0; $i < sizeof($return['errors']); $i++) $_SESSION['mass_errors'][] = $return['errors'][$i];
		echo "<h1>" . _("Additional tasks for module:") . ' ' . getModuleAlias($return['module'], $scope) . "</h1>\n";
		?>
			<div id="progressbar<?php echo $return['module']; ?>"></div>
			<script type="text/javascript">
				$(function() {
					$( "#progressbar<?php echo $return['module']; ?>" ).progressbar({
						value: <?php echo $return['progress']; ?>
					});
				});
			</script>
		<?php
		flush();
		while (!isset($_SESSION['mass_postActions']['finished']) && (($startTime + $maxTime) > time())) {
			$return  = doUploadPostActions($scope, $data, $_SESSION['mass_ids'], $_SESSION['mass_failed'], $_SESSION['mass_selectedModules'], $accounts);
			if ($return['status'] == 'finished') {
				$_SESSION['mass_postActions']['finished'] = true;
			}
			if (isset($return['errors'])) {
				for ($i = 0; $i < sizeof($return['errors']); $i++) {
					$_SESSION['mass_errors'][] = $return['errors'][$i];
				}
			}
		}
	}
	// create PDF when upload post actions are done
	if (isset($_SESSION['mass_postActions']['finished'])) {
		if (($_SESSION['mass_pdf']['structure'] != null) && !isset($_SESSION['mass_pdf']['finished'])) {
			$file = $_SESSION['mass_pdf']['file'];
			$pdfStructure = $_SESSION['mass_pdf']['structure'];
			$pdfZip = new ZipArchive();
			if ($_SESSION['mass_pdf']['counter'] == 0) {
				$pdfZipResult = @$pdfZip->open($_SESSION['mass_pdf']['file'], ZipArchive::CREATE);
				if (!$pdfZipResult === true) {
					$_SESSION['mass_errors'][] = array('ERROR', _('Unable to create ZIP file for PDF export.'), $file);
					$_SESSION['mass_pdf']['finished'] = true;
				}
			}
			else {
				@$pdfZip->open($_SESSION['mass_pdf']['file']);
			}
			// show progress bar
			$progress = ($_SESSION['mass_pdf']['counter'] * 100) / sizeof($accounts);
			echo "<h1>" . _('Create PDF files') . "</h1>\n";
			?>
				<div id="progressbarPDF"></div>
				<script type="text/javascript">
					$(function() {
						$( "#progressbarPDF" ).progressbar({
							value: <?php echo $progress; ?>
						});
					});
				</script>
			<?php
			flush();
			while (!isset($_SESSION['mass_pdf']['finished']) && (($startTime + $maxTime) > time())) {
				$attrs = $accounts[$_SESSION['mass_pdf']['counter']];
				$dn = $attrs['dn'];
				// get informational attributes
				$infoAttributes = array();
				foreach ($attrs as $key => $value) {
					if (strpos($key, 'INFO.') === 0) {
						$infoAttributes[$key] = $value;
					}
				}
				// load account
				$_SESSION['pdfAccount'] = new accountContainer($scope, 'pdfAccount');
				$pdfErrors = $_SESSION['pdfAccount']->load_account($dn, $infoAttributes);
				if (sizeof($pdfErrors) > 0) {
					$_SESSION['mass_errors'] = array_merge($_SESSION['mass_errors'], $pdfErrors);
					$_SESSION['mass_pdf']['finished'] = true;
					break;
				}
				// create and save PDF
				$pdfContent = createModulePDF(array($_SESSION['pdfAccount']), $pdfStructure, true);
				$pdfZip->addFromString($dn, $pdfContent);
				$_SESSION['mass_pdf']['counter'] ++;
				if ($_SESSION['mass_pdf']['counter'] >= sizeof($accounts)) {
					$_SESSION['mass_pdf']['finished'] = true;
				}
			}
			@$pdfZip->close();
		}
		else {
			$_SESSION['mass_pdf']['finished'] = true;
		}
	}
	// refresh with JavaScript
	echo "<script type=\"text/javascript\">\n";
	echo "top.location.href = \"massDoUpload.php\";\n";
	echo "</script>\n";
}
// all accounts have been created
else {
	echo "<div class=\"title\">\n";
	echo "<h2  class=\"titleText\">" . _("Upload has finished") . "</h2>\n";
	echo "</div>";
	if (sizeof($_SESSION['mass_errors']) > 0) {
		echo "<div class=\"subTitle\">\n";
		echo "<h4  class=\"subTitleText\">" . _("There were errors while uploading:") . "</h4>\n";
		echo "</div>";
		for ($i = 0; $i < sizeof($_SESSION['mass_errors']); $i++) {
			call_user_func_array('StatusMessage', $_SESSION['mass_errors'][$i]);
			echo "<br>";
		}
	}
	else {
		// redirect to list if no errors occured
		echo "<script type=\"text/javascript\">\n";
		echo "top.location.href = \"lists/list.php?type=" . $scope . "&uploadAllOk\";\n";
		echo "</script>\n";
	}
}
echo '</div>';
include 'main_footer.php';


?>