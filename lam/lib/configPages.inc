<?php

namespace LAM\CONFIG;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017 - 2024  Roland Gruber

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
 * Common functions for configuration pages.
 *
 * @package configuration
 * @author Roland Gruber
 */

/**
 * List of tabs.
 *
 * @author Roland Gruber
 */
class ConfigurationPageTab {

	/** general settings */
	const GENERAL = 'general';
	/** account types */
	const TYPES = 'types';
	/** account modules */
	const MODULES = 'modules';
	/** module settings */
	const MODULE_SETTINGS = 'moduleSettings';
	/** jobs */
	const JOBS = 'jobs';
	/** job history */
	const JOB_HISTORY = 'jobHistory';

}


/**
 * Prints the header bar.
 *
 * @param \LAMConfig $conf configuration object
 */
function printConfigurationPageHeaderBar($conf) {
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
            <span class="lam-margin-small">
                <span class="hide-on-mobile">
                <?php echo _('Server profile') . ': '; ?>
                </span>
			    <?php echo $conf->getName(); ?>
            </span>
        </div>
		<?php
		if (is_dir(__DIR__ . '/../docs/manual')) {
			?>
            <a class="lam-header-right lam-menu-icon hide-on-tablet" href="javascript:void(0);" class="icon"
               onclick="window.lam.topmenu.toggle();">
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
    <br>
	<?php
}

/**
 * Prints the tab list.
 *
 * @param ConfigurationPageTab $active active tab
 */
function printConfigurationPageTabs($active) {
	$tabs = [
		ConfigurationPageTab::GENERAL => [
			'id' => 'generalSettingsButton',
			'icon' => 'configure.svg',
			'label' => _('General settings')
		],
		ConfigurationPageTab::TYPES => [
			'id' => 'edittypes',
			'icon' => 'types.svg',
			'label' => _('Account types')
		],
		ConfigurationPageTab::MODULES => [
			'id' => 'editmodules',
			'icon' => 'modules.svg',
			'label' => _('Modules')
		],
		ConfigurationPageTab::MODULE_SETTINGS => [
			'id' => 'moduleSettings',
			'icon' => 'gears.svg',
			'label' => _('Module settings')
		],
	];
	if (isLAMProVersion()) {
		$tabs[ConfigurationPageTab::JOBS] = [
			'id' => 'jobs',
			'icon' => 'clock.svg',
			'label' => _('Jobs')
		];
	}
	// hidden submit buttons which are clicked by tabs
	?>
    <div style="display: none;">
        <input name="generalSettingsButton" type="submit" value=" ">
        <input name="edittypes" type="submit" value=" ">
        <input name="editmodules" type="submit" value=" ">
        <input name="moduleSettings" type="submit" value=" ">
        <input name="jobs" type="submit" value=" ">
    </div>
	<?php

	// tabs
	?>
    <div class="lam-tab-container">
    <ul class="lam-tab-navigation">
		<?php
		foreach ($tabs as $tab => $settings) {
			$isActive = ($tab === $active);
			$liClasses = 'lam-tab';
			if ($isActive) {
				$liClasses .= ' lam-tab-active';
			}
			echo '<li id="' . $settings['id'] . '" class="' . $liClasses . '">';
			echo '<a class="lam-tab-anchor" href="#" onclick="document.getElementsByName(\'' . $settings['id'] . '\')[0].click();"><img src="../../graphics/' . $settings['icon'] . '" alt=""> ';
			echo '<span class="hide-on-mobile">' . $settings['label'] . '</span>';
			echo '</a>';
			echo '</li>';
		}
		?>
    </ul>
    <div class="lam-tab-content lam-tab-active">
	<?php
}
