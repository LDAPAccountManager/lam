<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2010 - 2013  Roland Gruber

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
* Checks if the environment fits to run LAM.
* This code checks if the PHP version is ok, the required extensions are active
* and the memory limit is high enough.
*
* @author Roland Gruber
* @package main
*/

/** status messages */
include_once("../lib/status.inc");

/** config */
include_once("../lib/config.inc");

// check if PHP >= 5.2.4
if (version_compare(phpversion(), '5.2.4') < 0) {
	echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";
	echo "<html>\n<head>\n";
	echo "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";
	echo "<meta http-equiv=\"pragma\" content=\"no-cache\">\n		<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/500_layout.css\">\n";
	echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../graphics/favicon.ico\">\n";
	echo "<title>LDAP Account Manager</title>\n";
	echo "</head><body>\n";
	StatusMessage("ERROR", "LAM needs a PHP 5 version which is greater or equal than 5.2.4.", "Please upgrade your PHP installation. The found version is " . phpversion());
	echo "<br><br>";
	echo "</body></html>";
	exit();
}

// check environment
$criticalErrors = array();
// check if PHP has session support
if (! function_exists('session_start')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no session support!", "Please install the session extension for PHP.");
}
// check if PHP has SPL support
if (! function_exists('class_implements')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no SPL support!", "Please install the SPL extension for PHP.");
}
// check if PHP has LDAP support
if (! function_exists('ldap_search')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no LDAP support!", "Please install the LDAP extension for PHP.");
}
// check if PHP has gettext support
if (! function_exists('gettext') || !function_exists('_')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no gettext support!", "Please install gettext for PHP.");
}
// check if PHP has XML support
if (! function_exists('utf8_decode')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no XML support!", "Please install the XML extension for PHP.");
}
// check if PHP has GD support
if (! function_exists('getimagesize')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no GD support!", "Please install the GD extension for PHP.");
}
// check if PHP has JSON support
if (! function_exists('json_encode')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no JSON support!", "Please install the JSON extension for PHP.");
}
// check file permissions
$writableDirs = array('sess', 'tmp');
for ($i = 0; $i < sizeof($writableDirs); $i++) {
	$path = realpath('../') . "/" . $writableDirs[$i];
	if (!is_writable($path)) {
		$criticalErrors[] = array("ERROR", 'The directory %s is not writable for the web server. Please change your file permissions.', '', array($path));
	}
}
// check session auto start
if (ini_get("session.auto_start") == "1") {
	$criticalErrors[] = array("ERROR", "Please deactivate session.auto_start in your php.ini. LAM will not work if it is activated.");
}
// check memory limit
$memLimit = ini_get('memory_limit');
if (isset($memLimit) && ($memLimit != '') && (substr(strtoupper($memLimit), strlen($memLimit) - 1) == 'M')) {
	if (intval(substr($memLimit, 0, strlen($memLimit) - 1)) < 64) {
		$criticalErrors[] = array("ERROR", "Please increase the \"memory_limit\" parameter in your php.ini to at least \"64M\".",
			"Your current memory limit is $memLimit.");	
	}
}
// check PCRE regex system
if (!@preg_match('/^\p{L}+$/u', "abc")) {
	$criticalErrors[] = array("ERROR", "Your PCRE library has no complete Unicode support. Please upgrade libpcre or compile with \"--enable-unicode-properties\".");
}
// check ZIP support for PDF files in file upload
if (!extension_loaded('zip')) {
	$criticalErrors[] = array("ERROR", "Your PHP has no ZIP support.", "Please install the ZIP extension for PHP.");
}
// check for main config
$mainConfig = new LAMCfgMain();
if (!$mainConfig->isConfigFileExisting()) {
	$criticalErrors[] = array("ERROR", "The main config file (config.cfg) does not exist.", "Please see the manual for installation instructions.");
}
// stop login if critical errors occured
if (sizeof($criticalErrors) > 0) {
	echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\n";
	echo "<html>\n<head>\n";
	echo "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n";
	echo "<meta http-equiv=\"pragma\" content=\"no-cache\">\n		<meta http-equiv=\"cache-control\" content=\"no-cache\">\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../style/500_layout.css\">\n";
	echo "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"../graphics/favicon.ico\">\n";
	echo "<title>LDAP Account Manager</title>\n";
	echo "</head><body>\n";
	for ($i = 0; $i < sizeof($criticalErrors); $i++) {
		call_user_func_array("StatusMessage", $criticalErrors[$i]);
		echo "<br><br>";
	}
	echo "</body></html>";
	exit();
}


?>