<?php
namespace LAM\TOOLS\IMPORT_EXPORT;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2018  Roland Gruber

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
 * LDIF import.
 *
 * @author Roland Gruber
 * @package tools
 */

/** LDAP handle */
include_once('ldap.inc');

/**
 * Creates LDAP accounts for file upload.
 *
 * @author Roland Gruber
 * @package tools
 */
class Importer {

	const SESSION_KEY_ENTRIES = 'import_entries';
	const SESSION_KEY_COUNT = 'import_count';
	const STATUS = 'status';
	const PROGRESS = 'progress';
	const DATA = 'data';
	const TIME_LIMIT = 10;

	/**
	 * Processes the import data stored in session.
	 */
	public function doImport() {
		$entries = &$_SESSION[Importer::SESSION_KEY_ENTRIES];
		// check if any actions are needed at all
		if (empty($entries)) {
			return $this->getStatus();
		}
		$endTime = $this->getEndTime();
		while ((!empty($entries)) && ($endTime > time())) {
			$this->continueImport($entries);
		}
		return $this->getStatus();
	}

	/**
	 * Returns the current status as JSON.
	 *
	 * @return string JSON status
	 */
	private function getStatus() {
		if (empty($entries)) {
			if (isset($_SESSION[Importer::SESSION_KEY_ENTRIES])) {
				unset($_SESSION[Importer::SESSION_KEY_ENTRIES]);
			}
			$status = array(
				Importer::STATUS => 'done'
			);
			return json_encode($status);
		}
		$progress = (sizeof($_SESSION[Importer::SESSION_KEY_ENTRIES]) / $_SESSION[Importer::SESSION_KEY_COUNT]) * 100.0;
		$progress = floor(100 - $progress);
		$status = array(
			Importer::STATUS => 'inProgress',
			Importer::PROGRESS => $progress,
			Importer::DATA => ''
		);
		return json_encode($status);
	}

	/**
	 * Returns the time when processing should end.
	 *
	 * @return number end time as Unix timestamp
	 */
	private function getEndTime() {
		$startTime = time();
		$maxTime = get_cfg_var('max_execution_time') - 10;
		if ($maxTime > Importer::TIME_LIMIT) {
			$maxTime = Importer::TIME_LIMIT;
		}
		if ($maxTime <= 0) {
			$maxTime = Importer::TIME_LIMIT;
		}
		return $startTime + $maxTime;
	}

	/**
	 * Continues the import with processing of a single entry.
	 *
	 * @param array[] $entries import entries
	 */
	private function continueImport(&$entries) {
		$entry = array_shift($entries);
	}

}

?>
