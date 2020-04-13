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
		$mainCfg = $this->_getMainConfiguration();
		$jsonData = array();
		$jsonData['mainConfig'] = $this->_getMainConfigData($mainCfg);
		$jsonData['certificates'] = $this->_getCertificates($mainCfg);
		/**
		 * TODO
		 *
		 * webauthn
		 * server profiles
		 * server profile job database
		 * account profiles
		 * PDF profiles
		 * self service profiles
		 */
		return json_encode($jsonData);
	}

	/**
	 * Returns the main configuration.
	 *
	 * @return LAMCfgMain main config
	 */
	public function _getMainConfiguration() {
		return new LAMCfgMain();
	}

	/**
	 * Internal function to read master configuration.
	 *
	 * @param LAMCfgMain $mainCfg main config
	 * @return array data
	 */
	public function _getMainConfigData($mainCfg) {
		return $mainCfg->exportData();
	}

	/**
	 * Returns the certificate file content.
	 *
	 * @param LAMCfgMain $mainCfg main config
	 * @return array data
	 */
	public function _getCertificates($mainCfg) {
		return $mainCfg->exportCertificates();
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
				case 'certificates':
					$steps[] = new ImporterStep(_('SSL certificates'), 'certificates', $value);
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
	 * @throws LAMException if error occurred
	 */
	public function runImport($steps) {
		foreach ($steps as $step) {
			if (!$step->isActive()) {
				continue;
			}
			$key = $step->getKey();
			switch ($key) {
				case 'mainConfig':
					$this->importMainConfig($step->getValue());
					break;
				case 'certificates':
					$this->importCertificates($step->getValue());
					break;
				default:
					logNewMessage(LOG_WARNING, 'Unknown import type: ' . $key);
			}
		}
	}

	/**
	 * Imports the main configuration.
	 *
	 * @param array $data main config data
	 * @throws LAMException error during import
	 */
	private function importMainConfig($data) {
		$cfgMain = new LAMCfgMain();
		$cfgMain->importData($data);
		$cfgMain->save();
	}

	/**
	 * Imports the SSL certificates.
	 *
	 * @param null|string $data file content
	 * @throws LAMException error during import
	 */
	private function importCertificates($data) {
		$cfgMain = new LAMCfgMain();
		$cfgMain->importCertificates($data);
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
