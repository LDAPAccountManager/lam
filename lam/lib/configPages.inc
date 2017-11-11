<?php
namespace LAM\CONFIG;
/*
  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2017  Roland Gruber

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
		<table border=0 width="100%" class="lamHeader ui-corner-all">
			<tr>
				<td align="left" height="30">
					<a class="lamLogo" href="http://www.ldap-account-manager.org/" target="new_window">LDAP Account Manager</a>
				</td>
				<td align="right">
					<?php echo _('Server profile') . ': ' . $conf->getName(); ?>
					&nbsp;&nbsp;
				</td>
			</tr>
		</table>
		<br>
	<?php
}

/**
 * Prints the tab list.
 *
 * @param ConfigurationPageTab $active active tab
 */
function printConfigurationPageTabs($active) {
	$tabs = array(
		ConfigurationPageTab::GENERAL => array(
			'id' => 'generalSettingsButton',
			'icon' => 'tools.png',
			'label' => _('General settings')
		),
		ConfigurationPageTab::TYPES => array(
			'id' => 'edittypes',
			'icon' => 'gear.png',
			'label' => _('Account types')
		),
		ConfigurationPageTab::MODULES => array(
			'id' => 'editmodules',
			'icon' => 'modules.png',
			'label' => _('Modules')
		),
		ConfigurationPageTab::MODULE_SETTINGS => array(
			'id' => 'moduleSettings',
			'icon' => 'moduleSettings.png',
			'label' => _('Module settings')
		),
	);
	if (isLAMProVersion()) {
		$tabs[ConfigurationPageTab::JOBS] = array(
			'id' => 'jobs',
			'icon' => 'clock.png',
			'label' => _('Jobs')
		);
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
	<div class="ui-tabs ui-widget ui-widget-content ui-corner-all">

		<ul class="ui-tabs-nav ui-helper-reset ui-helper-clearfix ui-widget-header ui-corner-all">
	<?php
		foreach ($tabs as $tab => $settings) {
			$isActive = ($tab === $active);
			$liClasses = 'ui-state-default ui-corner-top';
			$hover = ' onmouseover="jQuery(this).addClass(\'tabs-hover\');" onmouseout="jQuery(this).removeClass(\'tabs-hover\');"';
			if ($isActive) {
				$liClasses .= ' lam-active-tab';
				$hover = '';
			}
			echo '<li id="' . $settings['id'] . '" class="' . $liClasses . '"' . $hover . '>';
				echo '<a href="#" onclick="document.getElementsByName(\'' . $settings['id'] . '\')[0].click();"><img src="../../graphics/' . $settings['icon'] . '" alt=""> ';
					echo '<span class="hide-on-mobile">' . $settings['label'] . '</span>';
				echo '</a>';
			echo '</li>';
		}
	?>
		</ul>
		<div class="ui-tabs-panel ui-widget-content ui-corner-bottom user-bright">
	<?php
}

?>
