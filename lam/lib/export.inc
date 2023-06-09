<?php
namespace LAM\TOOLS\IMPORT_EXPORT;
use \htmlStatusMessage;
use \LAMException;
use LamTemporaryFilesManager;

/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2018 - 2023  Roland Gruber

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
 * LDIF export.
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
class Exporter {

	const DATA = 'data';
	const STATUS = 'status';
	const FILE = 'file';
	const OUTPUT = 'output';

	private $baseDn = null;
	private $searchScope = null;
	private $filter = null;
	private $attributes = null;
	private $includeSystem = false;
	private $saveAsFile = false;
	private $format = null;
	private $ending = null;

	/**
	 * Constructor.
	 *
	 * @param string $baseDn base DN
	 * @param string $searchScope search scope (base, one, sub)
	 * @param string $filter search filter
	 * @param string $attributes attributes to return
	 * @param bool $includeSystem include system attributes
	 * @param bool $saveAsFile return as file
	 * @param string $format output format (LDIF, CSV)
	 * @param string $ending line endings (windows, unix)
	 */
	public function __construct($baseDn, $searchScope, $filter, $attributes, $includeSystem, $saveAsFile, $format, $ending) {
		$this->baseDn = $baseDn;
		$this->searchScope = $searchScope;
		$this->filter = $filter;
		$this->attributes = $attributes;
		$this->includeSystem = $includeSystem;
		$this->saveAsFile = $saveAsFile;
		$this->format = $format;
		$this->ending = $ending;
	}

	/**
	 * Starts the export process.
	 *
	 * @return string JSON result
	 */
	public function doExport() {
		try {
			$this->checkParameters();
			$results = $this->getLDAPData();
			return $this->writeDataAndReturnJson($results);
		}
		catch (LAMException $e) {
			$data = Exporter::formatMessage('ERROR', $e->getTitle(), $e->getMessage());
			$status = array(
				Exporter::STATUS => 'failed',
				Exporter::DATA => $data
			);
			return json_encode($status);
		}
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
	 * Checks the input parameters for validity.
	 *
	 * @throws LAMException in case of errors
	 */
	private function checkParameters() {
		if (!get_preg($this->baseDn, 'dn')) {
			throw new LAMException(_('Please enter a valid DN in the field:'), _('Base DN'));
		}
	}

	/**
	 * Returns the LDAP entries
	 *
	 * @return array[] LDAP entries
	 * @throws LAMException error reading data
	 */
	private function getLDAPData() {
		$attributes = preg_split('/,[ ]*/', $this->attributes);
		if ($this->includeSystem) {
			$attributes = array_merge($attributes, array('+', 'passwordRetryCount', 'accountUnlockTime', 'nsAccountLock',
				'nsRoleDN', 'passwordExpirationTime', 'pwdChangedTime'));
		}
		$attributes = array_unique($attributes);
		switch ($this->searchScope) {
			case 'base':
				return array(ldapGetDN($this->baseDn, $attributes));
			case 'one':
				$entries = ldapListDN($this->baseDn, $this->filter, $attributes);
				usort($entries, 'compareLDAPEntriesByDn');
				return $entries;
			case 'sub':
				$entries = searchLDAP($this->baseDn, $this->filter, $attributes);
				usort($entries, 'compareLDAPEntriesByDn');
				return $entries;
			default:
				throw new LAMException('Invalid scope');
		}
	}

	/**
	 * Writes the entries to file/response and prints JSON.
	 *
	 * @param array $entries LDAP entries
	 * @throws LAMException error writing PDF
	 */
	private function writeDataAndReturnJson(&$entries) {
		$lineEnding = ($this->ending === 'windows') ? "\r\n" : "\n";
		if ($this->format === 'csv') {
			$output = $this->getCsvOutput($entries, $lineEnding);
		}
		elseif ($this->format === 'ldif') {
			$output = $this->getLdifOutput($entries, $lineEnding);
		}
		else {
			throw new LAMException(_('Invalid format'));
		}
		if ($this->saveAsFile) {
			$temporaryFilesManager = new LamTemporaryFilesManager();
			$filename = $temporaryFilesManager->registerTemporaryFile('.' . $this->format);
			$handle = $temporaryFilesManager->openTemporaryFileForWrite($filename);
			fwrite($handle, $output);
			fclose($handle);
			return json_encode(array(
				Exporter::FILE => $temporaryFilesManager->getDownloadLink($filename),
				Exporter::STATUS => 'done'
			));
		}
		return json_encode(array(
			Exporter::OUTPUT => htmlspecialchars($output, ENT_NOQUOTES),
			Exporter::STATUS => 'done'
		));
	}

	/**
	 * Converts the given LDAP entries to CSV format.
	 *
	 * @param string $entries entries
	 * @param string $lineEnding line ending
	 */
	private function getCsvOutput(&$entries, $lineEnding) {
		$attributeNames = array();
		foreach ($entries as $entry) {
			$entryAttributeNames = array_keys($entry);
			foreach ($entryAttributeNames as $name) {
				if (!in_array($name, $attributeNames)) {
					$attributeNames[] = $name;
				}
			}
		}
		$attributeNames = array_delete(array('dn'), $attributeNames);
		sort($attributeNames);
		array_unshift($attributeNames, 'dn');

		$attributeNamesQuoted = array_map(array($this, 'escapeCsvAndAddQuotes'), $attributeNames);
		$output = '';
		// header
		$output .= implode(',', $attributeNamesQuoted) . $lineEnding;
		// content
		foreach ($entries as $entry) {
			$values = array();
			foreach ($attributeNames as $name) {
				if (!isset($entry[$name])) {
					$values[] = $this->escapeCsvAndAddQuotes('');
				}
				elseif (is_array($entry[$name])) {
					$values[] = $this->escapeCsvAndAddQuotes(implode(' | ', $entry[$name]));
				}
				else {
					$values[] = $this->escapeCsvAndAddQuotes($entry[$name]);
				}
			}
			$output .= implode(',', $values) . $lineEnding;
		}
		return $output;
	}

	/**
	 * Escapes a CSV value and adds quotes around it.
	 *
	 * @param string $value CSV value
	 * @return string escaped and quoted value
	 */
	private function escapeCsvAndAddQuotes($value) {
		return '"' . str_replace('"', '""', $value) . '"';
	}

	/**
	 * Converts the given LDAP entries to LDIF format.
	 *
	 * @param string $entries entries
	 * @param string $lineEnding line ending
	 */
	private function getLdifOutput(&$entries, $lineEnding) {
		$output = '';
		$output .= '#' . $lineEnding;
		$output .= '# ' . _('Base DN') . ': ' . $this->baseDn . $lineEnding;
		$output .= '# ' . _('Search scope') . ': ' . $this->searchScope . $lineEnding;
		$output .= '# ' . _('Search filter') . ': ' . $this->filter . $lineEnding;
		$output .= '# ' . _('Total entries') . ': ' . sizeof($entries) . $lineEnding;
		$output .= '#' . $lineEnding;
		$output .= '# Generated by LDAP Account Manager on ' . date('Y-m-d H:i:s') . $lineEnding;
		$output .= $lineEnding;
		$output .= $lineEnding;
		$output .= 'version: 1';
		$output .= $lineEnding;
		$output .= $lineEnding;
		foreach ($entries as $entry) {
			$output .= 'dn: ' . $entry['dn'] . $lineEnding;
			unset($entry['dn']);
			ksort($entry);
			foreach ($entry as $attributeName => $values) {
				foreach ($values as $value) {
					if ($this->isPlainAscii($value)) {
						$output .= $this->wrapLdif($attributeName . ': ' . $value, $lineEnding) . $lineEnding;
					}
					else {
						$output .= $this->wrapLdif($attributeName . ':: ' . base64_encode($value), $lineEnding) . $lineEnding;
					}
				}
			}
			$output .= $lineEnding;
		}

		return $output;
	}

	/**
	 * Splits the LDIF line if needed.
	 *
	 * @param string $content line content
	 * @param string $lineEnding line ending
	 */
	private function wrapLdif($content, $lineEnding) {
		$line_length = 76;
		if (strlen($content) <= $line_length) {
			return $content;
		}
		$wrappedContent = substr($content, 0, $line_length) . $lineEnding;
		$contentLeft = substr($content, $line_length);
		$line_length = $line_length - 1;
		$lines = str_split($contentLeft, $line_length);
		foreach ($lines as $line) {
			$wrappedContent .= ' ' . $line . $lineEnding;
		}
		return trim($wrappedContent);
	}

	/**
	 * Checks if the value is plain ASCII.
	 *
	 * @param string $content content to check
	 * @return bool is plain ASCII
	 */
	private function isPlainAscii($content) {
		for ($i=0; $i < strlen($content); $i++) {
			if (ord($content[$i]) < 32 || ord($content[$i]) > 127) {
				return false;
			}
		}
		return true;
	}

}
