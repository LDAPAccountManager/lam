<?php
namespace LAM\CONFIG;
use htmlLink;
use htmlResponsiveRow;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2024  Roland Gruber

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
* Displays links to all configuration pages.
*
* @package configuration
* @author Roland Gruber
*/


/** Access to config functions */
include_once('../../lib/config.inc');

// start session
if (isFileBasedSession()) {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

echo $_SESSION['header'];
printHeaderContents(_("Configuration overview"), '../..');
$content = new htmlResponsiveRow();

?>
	</head>
	<body>
        <?php
            // include all JavaScript files
            printJsIncludes('../..');
        ?>
        <div id="lam-topnav" class="lam-header">
            <div class="lam-header-left lam-menu-stay">
                <a href="https://www.ldap-account-manager.org/" target="new_window">
                    <img class="align-middle" width="24" height="24" alt="help" src="../../graphics/logo24.png">
                    <span class="hide-on-mobile">
                        <?php
                        echo getLAMVersionText();
                        ?>
                    </span>
                </a>
            </div>
	        <?php
	        if (is_dir(__DIR__ . '/../../docs/manual')) {
		        ?>
                <a class="lam-header-right lam-menu-icon hide-on-tablet" href="javascript:void(0);" class="icon" onclick="window.lam.topmenu.toggle();">
                    <img class="align-middle" width="16" height="16" alt="menu" src="../../graphics/menu.svg">
                    <span class="padding0">&nbsp;</span>
                </a>
                <a class="lam-header-right lam-menu-entry" target="_blank" href="../../docs/manual/index.html">
                    <span class="padding0"><?php echo _("Help") ?></span>
                </a>
		        <?php
	        }
	        ?>
        </div>
		<br><br>

        <?php
        $topContent = new htmlResponsiveRow();
        $topContent->setCSSClasses(['maxrow fullwidth roundedShadowBox spacing5']);
        $mainCfgLink = new htmlLink(_("Edit general settings"), 'mainlogin.php', '../../graphics/configure.svg');
        $mainCfgLink->setCSSClasses(['lam-margin-large display-as-block']);
        $topContent->add($mainCfgLink);
        $cfgLink = new htmlLink(_("Edit server profiles"), 'conflogin.php', '../../graphics/world.svg');
        $cfgLink->setCSSClasses(['lam-margin-large display-as-block']);
        $topContent->add($cfgLink);
        if (isLAMProVersion()) {
        	$selfServiceLink = new htmlLink(_("Edit self service"), '../selfService/adminLogin.php', '../../graphics/people.svg');
        	$selfServiceLink->setCSSClasses(['lam-margin-large display-as-block']);
	        $topContent->add($selfServiceLink);
        }
        $topContent->addVerticalSpacer('1rem');
        $importExportLink = new htmlLink(_("Import and export configuration"), 'confImportExport.php', '../../graphics/export.svg');
        $importExportLink->setCSSClasses(['lam-margin-large display-as-block']);
        $topContent->add($importExportLink);
        $content->add($topContent);
        $content->addVerticalSpacer('4rem');
        ?>

		<?php
		if (isLAMProVersion()) {
			include_once(__DIR__ . "/../../lib/env.inc");
			$printer = new \LAM\ENV\LAMLicenseInfoPrinter();
			$content->add($printer->getLicenseInfo());
			$content->addVerticalSpacer('2rem');
		}

		$content->add(new htmlLink(_("Back to login"), '../login.php'));
		$content->addVerticalSpacer('2rem');

		parseHtml(null, $content, [], true, null);

		?>

	</body>
</html>
