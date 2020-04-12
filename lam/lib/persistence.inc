<?php
namespace LAM\PERSISTENCE;
use LAMCfgMain;
use LAMException;

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
 * Exporter for LAM's configuration data.
 */
class ConfigDataExporter {

	/**
	 * Exports LAM's configuration data in JSON format.
	 */
	public function exportAsJson() {
		$jsonData = array();
		$jsonData['mainConfig'] = $this->_getMainConfigData();
		return json_encode($jsonData);
	}

	/**
	 * Internal function to read master configuration.
	 *
	 * @return array data
	 */
	public function _getMainConfigData() {
		$mainCfg = new LAMCfgMain();
		return $mainCfg->exportData();
	}

}

/**
 * Importer for LAM's configuration data.
 */
class ConfigDataImporter {

	/**
	 * Returns a list of possible import objects.
	 *
	 * @param string $json JSON data
	 * @return ImporterStep[] steps
	 * @throws LAMException if invalid format
	 */
	public function getPossibleImportSteps($json) {
		$data = json_decode($json);
		if ($data === null) {
			throw new LAMException(_('Unable to read import file.'));
		}
		$steps = array();
		foreach ($data as $key => $value) {
			switch ($key) {
				case 'mainConfig':
					$steps[] = new ImporterStep(_('General settings'), 'mainConfig', $value);
					break;
				default:
					logNewMessage(LOG_WARNING, 'Unknown import type: ' . $key);
			}
		}
		if (empty($steps)) {
			throw new LAMException(_('Unable to read import file.'));
		}
		return $steps;
	}

	/**
	 * Runs the actual import.
	 *
	 * @param ImporterStep[] $steps import steps
	 * @throws LAMException if error occured
	 */
	public function runImport($steps) {
		foreach ($steps as $step) {
			if (!$step->isActive()) {
				continue;
			}
			$key = $step->getKey();
			switch ($key) {
				case 'mainConfig':
					$cfgMain = new LAMCfgMain();
					$cfgMain->importData($step->getValue());
					$cfgMain->save();
					break;
				default:
					logNewMessage(LOG_WARNING, 'Unknown import type: ' . $key);
			}
		}
	}

}

/**
 * Step of the import process.
 */
class ImporterStep {

	private $label;
	private $key;
	private $value;
	private $active = false;

	/**
	 * Constructor.
	 *
	 * @param string $label label
	 * @param string $key key
	 * @param array $value value
	 */
	public function __construct($label, $key, &$value) {
		$this->label = $label;
		$this->key = $key;
		$this->value = $value;
	}

	/**
	 * Returns the label.
	 *
	 * @return string label
	 */
	public function getLabel() {
		return $this->label;
	}

	/**
	 * Returns the key.
	 *
	 * @return string key
	 */
	public function getKey() {
		return $this->key;
	}

	/**
	 * Returns if this step should be executed.
	 *
	 * @return bool active
	 */
	public function isActive(): bool {
		return $this->active;
	}

	/**
	 * Sets if this step should be executed.
	 *
	 * @param bool $active active
	 */
	public function setActive(bool $active) {
		$this->active = $active;
	}

	/**
	 * Returns the value.
	 *
	 * @return string value
	 */
	public function getValue() {
		return $this->value;
	}

}
