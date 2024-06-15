<?php
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2009 - 2024  Roland Gruber

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
 * This file includes functions to manage LAM tools.
 *
 * @package tools
 * @author Roland Gruber
 */

/**
 * Returns the tools which are available for LAM.
 *
 * @return array list of LAMTool objects
 */
function getTools(): array {
	$toolsDirName = __DIR__ . '/tools';
	$toolsDir = dir($toolsDirName);
	$entry = $toolsDir->read();
	// include all files in the tools directory
	while ($entry) {
		if ((substr($entry, strlen($entry) - 4, 4) == '.inc') && is_file($toolsDirName . '/' . $entry)) {
			include_once($toolsDirName . '/' . $entry);
		}
		$entry = $toolsDir->read();
	}
	// find tools classes
	$classList = get_declared_classes();
	$return = [];
	foreach ($classList as $classCandidate) {
		if (in_array('LAMTool', class_parents($classCandidate))) {
			$return[] = new $classCandidate();
		}
	}
	return $return;
}

/**
 * Represents a tool.
 * LAM will scan lib/tools/*.inc for classes which implement this interface. This allows to
 * dynamically plugin additional tools. There will be an entry on the tools page inside LAM
 * for each found class (if it matches the security level).
 * A LAMTool only specifies name, description and location of a tool. The tool functionality
 * is provided by the tool's target page.
 *
 * @author Roland Gruber
 * @package tools
 */
abstract class LAMTool {

	/**
	 * Returns the name of the tool.
	 *
	 * @return string name
	 */
	public abstract function getName(): string;

	/**
	 * returns a description text for the tool.
	 *
	 * @return string description
	 */
	public abstract function getDescription(): string;

	/**
	 * Returns a link to the tool page (relative to templates/).
	 *
	 * @return string link
	 */
	public abstract function getLink(): string;

	/**
	 * Returns if the tool requires write access to LDAP.
	 *
	 * @return bool true if write access is needed
	 */
	public abstract function getRequiresWriteAccess(): bool;

	/**
	 * Returns if the tool requires password change rights.
	 *
	 * @return bool true if password change rights are needed
	 */
	public abstract function getRequiresPasswordChangeRights(): bool;

	/**
	 * Returns the link to the tool image (relative to graphics/)
	 *
	 * @return string image URL
	 */
	public abstract function getImageLink(): string;

	/**
	 * Returns the preferred position of this tool on the tools page.
	 * The position may be between 0 and 1000. 0 is the top position.
	 *
	 * @return int preferred position
	 */
	public abstract function getPosition(): int;

	/**
	 * Returns if the tool is visible in the menu.
	 *
	 * @return bool visible
	 */
	public abstract function isVisible(): bool;

	/**
	 * Returns if a tool may be hidden by configuration in the LAM server profile.
	 *
	 * @return bool hideable
	 */
	public function isHideable(): bool {
		return true;
	}

	/**
	 * Returns the configuration options for this tool.
	 *
	 * @param array $settings current tool settings
	 * @return htmlElement|null configuration options
	 */
	public function getConfigOptions(array $settings): ?htmlElement {
		return null;
	}

	/**
	 * Checks the configuration options.
	 *
	 * @param array $settings hash array (option name => value) that contains the input. The option values are all arrays containing one or more elements.
	 * @return array list of error messages
	 */
	public function checkConfigurationOptions(array $settings): array {
		return [];
	}

}

/**
 * Represents a subtool.
 *
 * @author Roland Gruber
 * @package tools
 */
class LAMSubTool {

	/** visible tool name */
	public $name;
	/** tool description */
	public $description;
	/** tool link (relative to templates/) */
	public $link;
	/** image URL (relative to graphics/) */
	public $image;

}
