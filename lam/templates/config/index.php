<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2020  Roland Gruber

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
if (strtolower(session_module_name()) == 'files') {
	session_save_path("../../sess");
}
lam_start_session();

setlanguage();

echo $_SESSION['header'];
printHeaderContents(_("Configuration overview"), '../..');
$tabindex = 0;
$content = new htmlResponsiveRow();

?>
	</head>
	<body class="admin">
        <?php
            // include all JavaScript files
            printJsIncludes('../..');
        ?>
		<table class="lamTop ui-corner-all">
			<tr>
				<td align="left">
					<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">
						<?php echo getLAMVersionText(); ?>
					</a>
				</td>
			</tr>
		</table>
		<br><br>

        <?php
        $topContent = new htmlResponsiveRow();
        $topContent->setCSSClasses(array('maxrow fullwidth roundedShadowBox spacing5'));
        $mainCfgLink = new htmlLink(_("Edit general settings"), 'mainlogin.php', '../../graphics/bigTools.png');
        $mainCfgLink->setCSSClasses(array('img-padding1 display-as-block'));
        $topContent->add($mainCfgLink, 12);
        $cfgLink = new htmlLink(_("Edit server profiles"), 'conflogin.php', '../../graphics/profiles.png');
        $cfgLink->setCSSClasses(array('img-padding1 display-as-block'));
        $topContent->add($cfgLink, 12);
        if (isLAMProVersion()) {
        	$selfServiceLink = new htmlLink(_("Edit self service"), '../selfService/adminLogin.php', '../../graphics/bigPeople.png');
        	$selfServiceLink->setCSSClasses(array('img-padding1 display-as-block'));
	        $topContent->add($selfServiceLink, 12);
        }
        $content->add($topContent, 12);
        $content->addVerticalSpacer('4rem');
        ?>

		<?php
		if (isLAMProVersion()) {
			include_once(__DIR__ . "/../../lib/env.inc");
			$printer = new \LAM\ENV\LAMLicenseInfoPrinter();
			$content->add($printer->getLicenseInfo(), 12);
			$content->addVerticalSpacer('2rem');
		}

		$content->add(new htmlLink(_("Back to login"), '../login.php', '../../graphics/undo.png'), 12);
		$content->addVerticalSpacer('2rem');

		parseHtml('none', $content, array(), true, $tabindex, 'none');

		?>

	</body>
</html>
