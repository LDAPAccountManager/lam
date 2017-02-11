<?php
namespace LAM\UPLOAD;
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2004 - 2017  Roland Gruber

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
include_once("../../lib/security.inc");
/** access to configuration */
include_once('../../lib/config.inc');
/** LDAP handle */
include_once('../../lib/ldap.inc');
/** status messages */
include_once('../../lib/status.inc');
/** account modules */
include_once('../../lib/modules.inc');
/** PDF */
include_once('../../lib/pdf.inc');


// Start session
startSecureSession();
enforceUserIsLoggedIn();

// check if this tool may be run
checkIfToolIsActive('toolFileUpload');

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

// Redirect to startpage if user is not loged in
if (!isLoggedIn()) {
	metaRefresh("../login.php");
	exit;
}

// Set correct language, codepages, ....
setlanguage();

include '../main_header.php';
$typeId = htmlspecialchars($_SESSION['mass_typeId']);
$typeManager = new \LAM\TYPES\TypeManager();
$type = $typeManager->getConfiguredType($typeId);

// check if account type is ok
if ($type->isHidden()) {
	logNewMessage(LOG_ERR, 'User tried to access hidden upload: ' . $type->getId());
	die();
}
if (!checkIfNewEntriesAreAllowed($type->getId()) || !checkIfWriteAccessIsAllowed($type->getId())) {
	logNewMessage(LOG_ERR, 'User tried to access forbidden upload: ' . $type->getId());
	die();
}

echo '<div id="uploadContent" class="' . $type->getScope() . '-bright smallPaddingContent">';
$tokenPrefix = '?' . getSecurityTokenName() . '=' . getSecurityTokenValue();
?>
	<script type="text/javascript">
		jQuery(document).ready(function(){
			window.lam.upload.continueUpload('../misc/ajax.php' + '<?php echo $tokenPrefix; ?>' + '&function=upload&typeId=' + '<?php echo $type->getId() ?>');
		});
	</script>

<?php
echo '</div>';
include '../main_footer.php';
?>
