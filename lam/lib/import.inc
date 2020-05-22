<?php
namespace LAM\TOOLS\IMPORT_EXPORT;
use \htmlStatusMessage;
use \LAMException;
/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2018 - 2020  Roland Gruber

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
 * Imports LDIF files.
 *
 * @author Roland Gruber
 * @package tools
 */
class Importer {

	const SESSION_KEY_TASKS = 'import_tasks';
	const SESSION_KEY_COUNT = 'import_count';
	const SESSION_KEY_STOP_ON_ERROR = 'import_stop_on_error';
	const STATUS = 'status';
	const PROGRESS = 'progress';
	const DATA = 'data';
	const TIME_LIMIT = 10;
	const KEY = 0;
	const VALUE = 1;
	const CHANGETYPE = 'changeType';

	/**
	 * Extracts the single entries in the file.
	 *
	 * @param string[] $lines LDIF lines
	 * @return string|array array of string[]
	 * @throws LAMException invalid format
	 */
	private function extractImportChunks($lines) {
		$entries = array();
		$currentEntry = array();
		foreach ($lines as $line) {
			$trimmedLine = trim($line);
			if (substr($trimmedLine, 0, 1) === '#') {
				// skip comments
				continue;
			}
			if (empty($trimmedLine)) {
				// end of entry
				if (!empty($currentEntry)) {
					$entries[] = $currentEntry;
					$currentEntry = array();
				}
			}
			elseif (substr($line, 0, 1) === ' ') {
				// append to last line if starting with a space
				if (empty($currentEntry)) {
					throw new LAMException(_('Invalid data'), htmlspecialchars($line));
				}
				else {
					$currentEntry[sizeof($currentEntry) - 1] .= substr($line, 1);
				}
			}
			elseif ($line === '-') {
				$currentEntry[] = $line;
			}
			else {
				$parts = explode(':', $line, 2);
				if (sizeof($parts) < 2) {
					throw new LAMException(_('Invalid data'), htmlspecialchars($line));
				}
				$currentEntry[] = $line;
			}
		}
		if (!empty($currentEntry)) {
			$entries[] = $currentEntry;
		}
		return $entries;
	}

	/**
	 * Converts the lines to LDAP import tasks.
	 *
	 * @param string[] $lines import lines
	 * @throws LAMException if invalid format was found
	 * @return ImporterTask[] tasks
	 */
	public function getTasks($lines) {
		$chunks = $this->extractImportChunks($lines);
		return $this->convertToTasks($chunks);
	}

	/**
	 * Processes the import data stored in session.
	 */
	public function doImport() {
		$data = '';
		$tasks = &$_SESSION[Importer::SESSION_KEY_TASKS];
		$stopOnError = $_SESSION[Importer::SESSION_KEY_STOP_ON_ERROR];
		// check if any actions are needed at all
		if (empty($tasks)) {
			return $this->getStatus($data);
		}
		$endTime = $this->getEndTime();
		while ((!empty($tasks)) && ($endTime > time())) {
			$task = array_shift($tasks);
			try {
				$data .= $task->run();
			}
			catch (LAMException $e) {
				if ($stopOnError) {
					return $this->stopImport($data, $e);
				}
				else {
					$data .= Importer::formatMessage('ERROR', $e->getTitle(), $e->getMessage());
				}
			}
		}
		return $this->getStatus($data);
	}

	/**
	 * Stops the import process because of an exception.
	 *
	 * @param string $data HTML output
	 * @param LAMException $e exception
	 * @return string JSON status
	 */
	private function stopImport($data, LAMException $e) {
		$data .= Importer::formatMessage('ERROR', $e->getTitle(), $e->getMessage());
		if (isset($_SESSION[Importer::SESSION_KEY_TASKS])) {
			unset($_SESSION[Importer::SESSION_KEY_TASKS]);
		}
		$status = array(
			Importer::STATUS => 'failed',
			Importer::DATA => $data
		);
		return json_encode($status);
	}

	/**
	 * Returns the current status as JSON.
	 *
	 * @param string $data HTML output to display
	 * @return string JSON status
	 */
	private function getStatus($data) {
		if (empty($_SESSION[Importer::SESSION_KEY_TASKS])) {
			if (isset($_SESSION[Importer::SESSION_KEY_TASKS])) {
				unset($_SESSION[Importer::SESSION_KEY_TASKS]);
			}
			$status = array(
				Importer::STATUS => 'done',
				Importer::DATA => $data
			);
			return json_encode($status);
		}
		$progress = (sizeof($_SESSION[Importer::SESSION_KEY_TASKS]) / $_SESSION[Importer::SESSION_KEY_COUNT]) * 100.0;
		$progress = floor(100 - $progress);
		$status = array(
			Importer::STATUS => 'inProgress',
			Importer::PROGRESS => $progress,
			Importer::DATA => $data
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
	 * @return ImporterTask[] tasks
	 */
	private function convertToTasks($entries) {
		$tasks = array();
		$count = sizeof($entries);
		for ($i = 0; $i < $count; $i++) {
			$entry = $entries[$i];
			$firstParts = explode(':', $entry[0], 2);
			if ($firstParts[Importer::KEY] == 'version') {
				if ($i > 0) {
					// allow version only as first chunk
					throw new LAMException(_('Invalid data'), _('Duplicate version entry found.'));
				}
				$this->processVersion($entry);
			}
			elseif ($firstParts[Importer::KEY] == 'dn') {
				$tasks[] = $this->processDnEntry($entry);
			}
			else {
				throw new LAMException(_('A valid dn line is required'), htmlspecialchars($entry[0]));
			}
		}
		return $tasks;
	}

	/**
	 * Checks a version entry.
	 *
	 * @param string[] $entry entry
	 * @throws LAMException if version is invalid
	 */
	private function processVersion($entry) {
		$keyValue = $this->getLineKeyValue($entry[0]);
		if (($keyValue[Importer::VALUE] != '1') || (sizeof($entry) > 1)) {
			$escapedLines = array_map('htmlspecialchars', $entry);
			throw new LAMException(_('LDIF import only supports version 1'), implode('<br>', $escapedLines));
		}
	}

	/**
	 * Checks a dn entry.
	 *
	 * @param string[] $entry entry
	 * @return ImporterTask task
	 * @throws LAMException if invalid format
	 */
	private function processDnEntry($entry) {
		$dnLine = array_shift($entry);
		$keyValue = $this->getLineKeyValue($dnLine);
		$dn = $keyValue[Importer::VALUE];
		if (empty($entry)) {
			throw new LAMException(_('Invalid data'), htmlspecialchars($dnLine));
		}
		$firstAttributeLine = array_shift($entry);
		$firstAttribute = $this->getLineKeyValue($firstAttributeLine);
		if ($firstAttribute[Importer::KEY] != Importer::CHANGETYPE) {
			// complete DN
			$attributes = array(
				$firstAttribute[Importer::KEY] => array($firstAttribute[Importer::VALUE])
			);
			foreach ($entry as $attributeLine) {
				$attribute = $this->getLineKeyValue($attributeLine);
				$attributes[$attribute[Importer::KEY]][] = $attribute[Importer::VALUE];
			}
			return new AddEntryTask($dn, $attributes);
		}
		else {
			$type = $firstAttribute[Importer::VALUE];
			if ($type === 'add') {
				$attributes = array();
				foreach ($entry as $line) {
					$lineData = $this->getLineKeyValue($line);
					$attributes[$lineData[Importer::KEY]][] = $lineData[Importer::VALUE];
				}
				return new AddEntryTask($dn, $attributes);
			}
			elseif ($type === 'modrdn') {
				return $this->createModRdnTask($dn, $entry);
			}
			elseif ($type === 'delete') {
				if (!empty($entry)) {
					throw new LAMException(_('Invalid data'), htmlspecialchars($dn));
				}
				return new DeleteEntryTask($dn);
			}
			elseif ($type !== 'modify') {
				throw new LAMException(_('Invalid data'), htmlspecialchars($dn) . ' - changeType: ' . htmlspecialchars($type));
			}
			$subtasks = array();
			$currentLines = array();
			$linesCount = sizeof($entry);
			for ($i = 0; $i < $linesCount; $i++) {
				$line = $entry[$i];
				if ($line === '-') {
					$subtasks[] = $this->getChangeTypeTask($dn, $currentLines);
					$currentLines = array();
				}
				else {
					$currentLines[] = $line;
				}
			}
			$subtasks[] = $this->getChangeTypeTask($dn, $currentLines);
			return new MultiTask($subtasks, $dn);
		}
	}

	/**
	 * Returns a modrdn task.
	 *
	 * @param string $dn DN
	 * @param string[] $entry entry lines
	 * @return
	 * @throws LAMException syntax error
	 */
	private function createModRdnTask($dn, $entry) {
		if (sizeof($entry) !== 2) {
			throw new LAMException(_('Invalid data'), htmlspecialchars($dn));
		}
		$newRdnData = $this->getLineKeyValue($entry[0]);
		if ($newRdnData[Importer::KEY] !== 'newrdn') {
			throw new LAMException(_('Invalid data'), htmlspecialchars($dn) . '<br>' . $newRdnData);
		}
		$newRdn = $newRdnData[Importer::VALUE];
		$delOldRdnData = $this->getLineKeyValue($entry[1]);
		if (($delOldRdnData[Importer::KEY] !== 'deleteoldrdn') || !in_array($delOldRdnData[Importer::VALUE], array('0', '1'), true)) {
			throw new LAMException(_('Invalid data'), htmlspecialchars($dn) . '<br>' . $entry[1]);
		}
		$delOldRdn = ($delOldRdnData[Importer::VALUE] !== '0');
		return new RenameEntryTask($dn, $newRdn, $delOldRdn);
	}

	/**
	 * Returns a task for LDIF changeType entry.
	 *
	 * @param string $dn DN
	 * @param string $lines lines
	 * @return ImporterTask task
	 */
	private function getChangeTypeTask($dn, $lines) {
		$firstLine = array_shift($lines);
		$firstLineData = $this->getLineKeyValue($firstLine);
		$type = $firstLineData[Importer::KEY];
		$attributeName = $firstLineData[Importer::VALUE];
		$attributes = array();
		foreach ($lines as $line) {
			$lineData = $this->getLineKeyValue($line);
			if ($lineData[Importer::KEY] !== $attributeName) {
				throw new LAMException(_('Invalid data'), htmlspecialchars($dn) . ' - ' . htmlspecialchars($type));
			}
			$attributes[$attributeName][] = $lineData[Importer::VALUE];
		}
		if ($type === 'add') {
			return new AddAttributesTask($dn, $attributes);
		}
		elseif ($type === 'delete') {
			return new DeleteAttributesTask($dn, $attributeName, $attributes);
		}
		elseif ($type === 'replace') {
			return new ReplaceAttributesTask($dn, $attributes);
		}
		throw new LAMException(_('Invalid data'), htmlspecialchars($dn) . ' - ' . htmlspecialchars($type));
	}

	/**
	 * Returns the HTML for an error message.
	 *
	 * @param string $type message type (e.g. INFO)
	 * @param string $title title
	 * @param string $message message
	 * @return string HTML
	 */
	public static function formatMessage($type, $title, $message) {
		$msg = new htmlStatusMessage($type, $title, $message);
		$tabindex = 0;
		ob_start();
		$msg->generateHTML(null, array($msg), array(), true, $tabindex, 'user');
		$data = ob_get_contents();
		ob_clean();
		return $data;
	}

	/**
	 * Returns the key and value part of the line.
	 *
	 * @param string $line line
	 * @return string[] array(key, value)
	 */
	private function getLineKeyValue($line) {
		$parts = explode(':', $line, 2);
		if (sizeof($parts) !== 2) {
			throw new LAMException(_('Invalid data'), htmlspecialchars($line));
		}
		if (substr($parts[Importer::VALUE], 0, 1) == ':') {
			$value = base64_decode(trim(substr($parts[Importer::VALUE], 1)));
		}
		else {
			$value = trim($parts[Importer::VALUE]);
		}
		return array($parts[Importer::KEY], $value);
	}

}

/**
 * A single import task.
 *
 * @author Roland Gruber
 */
interface ImporterTask {

	/**
	 * Runs the task.
	 *
	 * @return string HTML output or LAMException if error occurred
	 */
	public function run();

}

/**
 * Adds a complete LDAP entry.
 *
 * @author Roland Gruber
 */
class AddEntryTask implements ImporterTask {

	private $dn = '';
	private $attributes = array();

	/**
	 * Constructor
	 *
	 * @param string $dn DN
	 * @param array[string[]] $attributes list of attributes
	 */
	public function __construct($dn, $attributes) {
		$this->dn = $dn;
		$this->attributes = $attributes;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\TOOLS\IMPORT_EXPORT\ImporterTask::run()
	 */
	public function run() {
		$ldap = $_SESSION['ldap']->server();
		$success = @ldap_add($ldap, $this->dn, $this->attributes);
		if ($success) {
			return Importer::formatMessage('INFO', _('Entry created'), htmlspecialchars($this->dn));
		}
		throw new LAMException(sprintf(_('Was unable to create DN: %s.'), $this->dn), getExtendedLDAPErrorMessage($ldap));
	}

}

/**
 * Renames an LDAP entry.
 *
 * @author Roland Gruber
 */
class RenameEntryTask implements ImporterTask {

	private $dn = '';
	private $newRdn = '';
	private $deleteOldRdn = true;

	/**
	 * Constructor
	 *
	 * @param string $dn DN
	 * @param string $newRdn new RDN value
	 * @param bool $deleteOldRdn delete old RDN value
	 */
	public function __construct($dn, $newRdn, $deleteOldRdn) {
		$this->dn = $dn;
		$this->newRdn = $newRdn;
		$this->deleteOldRdn = $deleteOldRdn;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\TOOLS\IMPORT_EXPORT\ImporterTask::run()
	 */
	public function run() {
		$ldap = $_SESSION['ldap']->server();
		$success = @ldap_rename($ldap, $this->dn, $this->newRdn, null, $this->deleteOldRdn);
		if ($success) {
			return Importer::formatMessage('INFO', _('Rename successful!'), htmlspecialchars($this->dn));
		}
		throw new LAMException(_('Could not rename the entry.') . '<br>' . $this->dn, getExtendedLDAPErrorMessage($ldap));
	}

}

/**
 * Deletes an LDAP entry.
 *
 * @author Roland Gruber
 */
class DeleteEntryTask implements ImporterTask {

	private $dn = '';

	/**
	 * Constructor
	 *
	 * @param string $dn DN
	 */
	public function __construct($dn) {
		$this->dn = $dn;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\TOOLS\IMPORT_EXPORT\ImporterTask::run()
	 */
	public function run() {
		$ldap = $_SESSION['ldap']->server();
		$success = @ldap_delete($ldap, $this->dn);
		if ($success) {
			return Importer::formatMessage('INFO', sprintf(_('Successfully deleted DN %s'), $this->dn), '');
		}
		throw new LAMException(_('Could not delete the entry.') . '<br>' . $this->dn, getExtendedLDAPErrorMessage($ldap));
	}

}

/**
 * Combines multiple import tasks.
 *
 * @author Roland Gruber
 */
class MultiTask implements ImporterTask {

	/**
	 * @var ImporterTask[] tasks
	 */
	private $tasks = array();

	/**
	 * @var string DN
	 */
	private $dn = null;

	/**
	 * Constructor
	 *
	 * @param ImporterTask[] $tasks tasks
	 */
	public function __construct($tasks, $dn) {
		$this->tasks = $tasks;
		$this->dn = $dn;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\TOOLS\IMPORT_EXPORT\ImporterTask::run()
	 */
	public function run() {
		foreach ($this->tasks as $task) {
			$task->run();
		}
		return Importer::formatMessage('INFO', _('LDAP operation successful.'), htmlspecialchars($this->dn));
	}

	/**
	 * Returns the list of subtasks.
	 *
	 * @return ImporterTask[]
	 */
	public function getTasks() {
		return $this->tasks;
	}

}

/**
 * Adds attributes to an existing LDAP entry.
 *
 * @author Roland Gruber
 */
class AddAttributesTask implements ImporterTask {

	private $dn = '';
	private $attributes = array();

	/**
	 * Constructor
	 *
	 * @param string $dn DN
	 * @param array[string[]] $attributes list of attributes
	 */
	public function __construct($dn, $attributes) {
		$this->dn = $dn;
		$this->attributes = $attributes;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\TOOLS\IMPORT_EXPORT\ImporterTask::run()
	 */
	public function run() {
		$ldap = $_SESSION['ldap']->server();
		$success = @ldap_mod_add($ldap, $this->dn, $this->attributes);
		if ($success) {
			return '';
		}
		throw new LAMException(sprintf(_('Was unable to create DN: %s.'), $this->dn), getExtendedLDAPErrorMessage($ldap));
	}

	/**
	 * Returns the DN.
	 *
	 * @return string DN
	 */
	public function getDn() {
		return $this->dn;
	}

	/**
	 * Returns the attributes to add.
	 *
	 * @return string[] attributes (array('attr' => array('val1', 'val2')))
	 */
	public function getAttributes() {
		return $this->attributes;
	}

}

/**
 * Deletes attributes from an existing LDAP entry.
 *
 * @author Roland Gruber
 */
class DeleteAttributesTask implements ImporterTask {

	private $dn = '';
	private $attributes = array();
	private $attributeName = null;

	/**
	 * Constructor
	 *
	 * @param string $dn DN
	 * @param string $attributeName attribute name
	 * @param array[string[]] $attributes list of attributes
	 */
	public function __construct($dn, $attributeName, $attributes) {
		$this->dn = $dn;
		$this->attributeName = $attributeName;
		$this->attributes = $attributes;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\TOOLS\IMPORT_EXPORT\ImporterTask::run()
	 */
	public function run() {
		$ldap = $_SESSION['ldap']->server();
		if (!empty($this->attributes)) {
			$success = @ldap_mod_del($ldap, $this->dn, $this->attributes);
		}
		else {
			$success = @ldap_modify($ldap, $this->dn, array($this->attributeName => array()));
		}
		if ($success) {
			return '';
		}
		throw new LAMException(sprintf(_('Was unable to create DN: %s.'), $this->dn), getExtendedLDAPErrorMessage($ldap));
	}

	/**
	 * Returns the DN.
	 *
	 * @return string DN
	 */
	public function getDn() {
		return $this->dn;
	}

	/**
	 * Returns the attributes to add.
	 *
	 * @return string[] attributes (array('attr' => array('val1', 'val2')))
	 */
	public function getAttributes() {
		return $this->attributes;
	}

	/**
	 * Returns the attributes name.
	 *
	 * @return string name
	 */
	public function getAttributeName() {
		return $this->attributeName;
	}

}

/**
 * Replaces attributes in an existing LDAP entry.
 *
 * @author Roland Gruber
 */
class ReplaceAttributesTask implements ImporterTask {

	private $dn = '';
	private $attributes = array();

	/**
	 * Constructor
	 *
	 * @param string $dn DN
	 * @param array[string[]] $attributes list of attributes
	 */
	public function __construct($dn, $attributes) {
		$this->dn = $dn;
		$this->attributes = $attributes;
	}

	/**
	 * {@inheritDoc}
	 * @see \LAM\TOOLS\IMPORT_EXPORT\ImporterTask::run()
	 */
	public function run() {
		$ldap = $_SESSION['ldap']->server();
		$success = @ldap_modify($ldap, $this->dn, $this->attributes);
		if ($success) {
			return '';
		}
		throw new LAMException(sprintf(_('Was unable to create DN: %s.'), $this->dn), getExtendedLDAPErrorMessage($ldap));
	}

	/**
	 * Returns the DN.
	 *
	 * @return string DN
	 */
	public function getDn() {
		return $this->dn;
	}

	/**
	 * Returns the attributes to add.
	 *
	 * @return string[] attributes (array('attr' => array('val1', 'val2')))
	 */
	public function getAttributes() {
		return $this->attributes;
	}

	/**
	 * Returns the attributes name.
	 *
	 * @return string name
	 */
	public function getAttributeName() {
		return $this->attributeName;
	}

}

?>
