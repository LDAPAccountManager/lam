<?php
/*
$Id:

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2012  Christian Kropp
                2012  Roland Gruber

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
* This file includes functions to upgrade the pdf/profiles configuration files.
*
* @package main
* @author Christian Kropp
* @author Roland Gruber
*/


/**
 * check the write/read permission for the upgrade
 *
 * @return array - permission messages
 */
function testPermissions() {
	$result = array();
	if (!is_writable('../config')) {
		$result[] = htmlspecialchars(realpath('../config'));
	}
	$result = array_merge($result, testPermissionRecursive('../config/profiles/'));
    $result = array_merge($result, testPermissionRecursive('../config/pdf/'));
    return $result;
}

/**
 * Recursively checks the permissions in a directory.
 * 
 * @param String $dir directory
 * @return array list of files/directories with wrong permission
 */
function testPermissionRecursive($dir) {
	$result = array();
	if (!is_writable($dir)) {
		$result[] = htmlspecialchars(realpath($dir));
	}
	$dirHandle = @opendir($dir);
	if ($dirHandle != null) {
		$file = @readdir($dirHandle);
		while ($file !== false) {
			if (($file != '.') && ($file != '..')) {
				if (is_dir($dir . '/' . $file)) {
					$result = array_merge($result, testPermissionRecursive($dir . '/' . $file));
				}
				elseif (!is_writable($dir . '/' . $file)) {
					$result[] = htmlspecialchars(realpath($dir . '/' . $file));
				}
			}
			$file = @readdir($dirHandle);
		}
	}
	@closedir($dirHandle);
	return $result;	
}

/**
 * Checks if the given directory contains files.
 * This is used to check if config files need to be migrated.
 * 
 * @param String $dir directory path
 */
function containsFiles($dir) {
	$return = false;
	$dirHandle = @opendir($dir);
	if ($dirHandle != null) {
		$file = @readdir($dirHandle);
		while ($file !== false) {
			if (is_file($dir . '/' . $file)) {
				$return = true;
				break;
			}
			$file = @readdir($dirHandle);
		}
	}
	@closedir($dirHandle);
	return $return;	
}


/**
 * Saves an hash array (attribute => value) to an account profile
 *
 * @param array $profiles server profiles
 */
function upgradeConfigToServerProfileFolders($profiles) {
	if (!is_writable('../config')) {
		StatusMessage('ERROR', 'Upgrade failed.', 'The directory \'/config\' has missing write permissions.');
		return;
	}

	// copy default configs
	if (!file_exists('../config/templates')) {
		@mkdir('../config/templates');
		recursiveCopy('../config/pdf/', '../config/templates/pdf/', $profiles, 'default.');
		recursiveCopy('../config/profiles/', '../config/templates/profiles/', $profiles, 'default.');
	}

	foreach ($profiles as $profile) {
		// upgrade PDF configs
		$dir = '../config/pdf/' . $profile;
		if (!file_exists($dir)) {
			recursiveCopy('../config/pdf/', $dir, $profiles);
		}

		// upgrade profiles configs
		$dir = '../config/profiles/' . $profile;
		if (!file_exists($dir)) {
			recursiveCopy('../config/profiles/', $dir, $profiles);
		}
	}

	// delete old files
	recursiveDelete('../config/pdf', $profiles);
	recursiveDelete('../config/profiles', $profiles);
}

/**
 * Saves an hash array (attribute => value) to an account profile
 *
 * @param array $profiles server profiles
 */
function copyConfigTemplates($profiles) {
	foreach ($profiles as $profile) {
		// copy templates but do not override existing files
		recursiveCopy('../config/templates/pdf/', '../config/pdf/' . $profile, $profiles, null, false);
		recursiveCopy('../config/templates/profiles/', '../config/profiles/' . $profile, $profiles, null, false);
	}
}

/**
 * Copy a file or recursively copy a directory
 *
 * @param string $src - source path to file or directory
 * @param string $dst - destination path to file or directory
 * @param array $profiles - server profiles (used to avoid copying of newly created folders)
 * @param string $fileFilter copy only files that start with the given filter
 * @param boolean $overwrite overwrite existing files
 */
function recursiveCopy($src, $dst, $profiles, $fileFilter = null, $overwrite = true) {
	$dir = @opendir($src);
	if (!file_exists($dst)) {
		$tmpState = @mkdir($dst);
		if ($tmpState === false) {
			StatusMessage('ERROR', 'Upgrade failed.', 'The directory \'' . $dst . '\' could not be created.');
		}
	}
	while (false !== ($file = readdir($dir))) {
		if ($file != '.'  && $file != '..'  && !in_array($file, $profiles)) {
			if (is_dir($src . '/' . $file) && ($file == 'logos')) {
				recursiveCopy($src . '/' . $file, $dst . '/' . $file, $profiles, $fileFilter, $overwrite);
			}
			elseif ((isset($fileFilter) && (strpos($file, $fileFilter) === 0 || $file == '.htaccess'))
						|| (!isset($fileFilter))) {
				if (!is_file($src . '/' . $file)) {
					continue;
				}
				if ($overwrite || !file_exists($dst . '/' . $file)) {
					$tmpState = @copy($src . '/' . $file, $dst . '/' . $file);
					if ($tmpState === false) {
						StatusMessage('ERROR', 'Upgrade failed.', 'The file ' . $file . ' could not be copied.');
					}
				}
			}
		}
	}
	closedir($dir);
}


/**
 * Delete a file or recursively delete a directory
 *
 * @param string $src - path to file or directory
 * @param array $profiles - server profiles (used to avoid copying of newly created folders)
 */
function recursiveDelete($src, $profiles) {
	if (is_file($src)) {
		$tmpState = @unlink($src);
		if ($tmpState === false) {
			StatusMessage('ERROR', 'Upgrade failed.', 'The file ' . $src . ' could not be deleted.');
		}
		return;
	} else if (is_dir($src) && is_writable($src)) {
		$dir = @opendir($src);
		while (false !== ($path = readdir($dir))) {
			if ($path != '.'  && $path != '..' && !in_array($path, $profiles)) {
				recursiveDelete($src . '/' . $path, $profiles);
			}
		}
		@closedir($dir);

		if ($src != '../config/pdf' && $src != '../config/profiles') {
			$tmpState = @rmdir($src);
			if ($tmpState === false) {
				StatusMessage('ERROR', 'Upgrade failed.', 'The directory ' . $src . ' could not be deleted.');
			}
		}
		return;
	} else {
		StatusMessage('ERROR', 'Upgrade failed.', 'The directory ' . $src . ' has missing write permissions.');
		return;
	}
}

?>
