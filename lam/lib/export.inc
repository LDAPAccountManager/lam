<?php
namespace LAM\TOOLS\IMPORT_EXPORT;
use \htmlStatusMessage;
use \LAMException;
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

}
