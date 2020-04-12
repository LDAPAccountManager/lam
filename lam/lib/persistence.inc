<?php
namespace LAM\PERSISTENCE;
/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020  Roland Gruber

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
 * This file includes functions to manage the persistence of LAM's configuration files.
 *
 * @package configuration
 * @author Roland Gruber
 */

include_once __DIR__ . '/config.inc';

/**
 * Exprter for LAM's configuration data.
 */
class ConfigDataExporter {

	/**
	 * Exports LAM's configuration data in JSON format.
	 */
	public function exportAsJson() {
		$jsonData = array();
		$jsonData['mainconfig'] = $this->_getMainConfigData();
		return json_encode($jsonData);
	}

	/**
	 * Internal function to read master configuration.
	 *
	 * @return array data
	 */
	public function _getMainConfigData() {
		$mainCfg = new \LAMCfgMain();
		return $mainCfg->exportData();
	}

}

