<?php
namespace LAM\DOWNLOAD;
use LAMException;
use LamTemporaryFilesManager;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2023  Roland Gruber

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
 * Manages download of files.
 *
 * @author Roland Gruber
 * @package misc
 */

/** security functions */
include_once(__DIR__ . "/../../lib/security.inc");

// start session
if (isset($_GET['selfservice'])) {
	// self service uses a different session name
	session_name('SELFSERVICE');
}

// return 403 if session expired
if (startSecureSession(false, true) === false) {
	http_response_code(403);
	die();
}

setlanguage();

$temporaryFilesManager = new LamTemporaryFilesManager();
$fileParamName = 'file';

if (empty($_GET[$fileParamName]) || !$temporaryFilesManager->isRegisteredFile($_GET[$fileParamName])) {
	http_response_code(403);
	die();
}

try {
	$fileName = $_GET[$fileParamName];
	$handle = $temporaryFilesManager->openTemporaryFileForRead($fileName);
	setMimeType($fileName);
	if (isset($_GET['download']) && ($_GET['download'] === 'true')) {
		header('content-disposition: attachment; filename="' . $fileName . '"');
	}
	$content = fread($handle, 100000);
	while (($content !== false) && ($content !== '')) {
		echo $content;
		$content = fread($handle, 100000);
	}
} catch (LAMException $e) {
	logNewMessage(LOG_ERR, 'Unable to open file ' . $fileName . ': ' . $e->getMessage());
	http_response_code(403);
	die();
}

/**
 * Sets the mime type.
 *
 * @param string $fileName file name
 */
function setMimeType(string $fileName): void {
	if (headers_sent()) {
		return;
	}
	$extension = substr($fileName, strrpos($fileName, '.') + 1);
	$mimeType = null;
	switch ($extension) {
		case 'crt':
			$mimeType = 'application/x-x509-user-cert';
			break;
		case 'csv':
			$mimeType = 'text/csv; charset=UTF-8';
			break;
		case 'jpg':
			$mimeType = 'image/jpeg';
			break;
		case 'ldif':
		case 'pem':
			$mimeType = 'text/plain; charset=UTF-8';
			break;
		case 'svg':
			$mimeType = 'image/svg+xml';
			break;
		case 'zip':
			$mimeType = 'application/zip';
			break;
	}
	if ($mimeType !== null) {
		header('Content-Type: ' . $mimeType);
	}
}
