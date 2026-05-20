<?php
/*
  This code is part of GOsa (https://gosa.gonicus.de)
  Copyright (C) 2004  Cajus Pollmeier
                2024 - 2026  Roland Gruber

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
 * Manages terminal server settings for Samba 3.
 *
 * @package modules
 * @author Cajus Pollmeier
 */

/**
 * File header
 */
const SAMBA_MUNGEDDIAL_FILEHEADER = "20002000200020002000200020002000" .
	"20002000200020002000200020002000" .
	"20002000200020002000200020002000" .
	"20002000200020002000200020002000" .
	"20002000200020002000200020002000" .
	"20002000200020002000200020002000" .
	"5000";
/**
 * File header for old format.
 */
const SAMBA_MUNGEDDIAL_FILEHEADER_OLD = "6d000800200020002000200020002000" .
	"20002000200020002000200020002000" .
	"20002000200020002000200064000100" .
	"20002000200020002000200020002000" .
	"20002000200020002000200020002000" .
	"20002000200020002000200020002000" .
	"50001000";

/**
 * Manages terminal server settings for Samba 3.
 *
 * @author Cajus Pollmeier
 * @package modules
 */
class sambaMungedDial {
	/** @var array<string, string> $ctx Terminal server variables (samba3) */
	public array $ctx = [
		'CtxCfgPresent' => '551e0bb0',
		'CtxCfgFlags1' => '00e00010',
		'CtxCallback' => '',
		'CtxShadow' => '01000000',
		'CtxMaxConnectionTime' => '',
		'CtxMaxDisconnectionTime' => '',
		'CtxMaxIdleTime' => '',
		'CtxKeyboardLayout' => '',
		'CtxMinEncryptionLevel' => '00',
		'CtxWorkDirectory' => '',
		'CtxNWLogonServer' => '',
		'CtxWFHomeDir' => '',
		'CtxWFHomeDirDrive' => '',
		'CtxWFProfilePath' => '',
		'CtxInitialProgram' => '',
		'CtxCallbackNumber' => ''];

	/** @var string[] $stringParams These parameters are treated as strings and get a trailing zero */
	private array $stringParams = ["CtxWorkDirectory", "CtxNWLogonServer",
		"CtxWFHomeDir", "CtxWFHomeDirDrive",
		"CtxWFProfilePath", "CtxInitialProgram", "CtxCallbackNumber"];

	/** @var string[] $timeParams These parameters are treated as time values and get converted */
	private array $timeParams = ["CtxMaxConnectionTime", "CtxMaxDisconnectionTime", "CtxMaxIdleTime"];

	private bool $old_behavior = false;

	/** strhex
	 * @param string $string string
	 * @return string hex
	 */
	public function strhex($string): string {
		$hex = "";

		for ($i = 0; $i < strlen($string); $i++) {
			$hex .= dechex(ord($string[$i]));
		}

		return ($hex);
	}

	/** hexstr
	 * @param string $hex hex value
	 * @return string value
	 */
	public function hexstr($hex): string {
		$string = "";

		for ($i = 0; $i < strlen($hex) - 1; $i += 2) {
			$string .= chr(hexdec($hex[$i] . $hex[$i + 1]));
		}

		return ($string);
	}

	/** endian
	 * @param string $src source
	 * @return string endian changed
	 */
	public function endian($src): string {
		return (substr($src, 2, 2) . substr($src, 0, 2));
	}

	/** genTime
	 * @param string $minutes minutes
	 * @return string time
	 */
	public function genTime($minutes): string {
		if ($minutes === '') {
			$minutes = 0;
		}
		$usec = (int) ($minutes * 60 * 1000);
		$src = sprintf('%04x%04x', $usec & 0x0FFFF, ($usec & 0x0FFFF0000) >> 16);
		return (sambaMungedDial::endian(substr($src, 0, 4)) . sambaMungedDial::endian(substr($src, 4, 4)));
	}

	/** readTime
	 * @param string $time time
	 * @return int time
	 */
	public function readTime($time): int {
		$lo = substr($time, 0, 4);
		$hi = substr($time, 4, 4);

		$usecs = (hexdec(substr($lo, 2, 2)) * 256 + hexdec(substr($lo, 0, 2))) +
			(hexdec(substr($hi, 2, 2)) * 256 + hexdec(substr($hi, 0, 2))) * 256 * 256;

		return ((int) ($usecs / (60 * 1000)));
	}

	/** to8bit
	 * @param string $string string
	 * @return string 8bit string
	 */
	public function to8bit($string): string {
		$result = "";

		/* Strip zeros */
		for ($i = 0; $i < strlen($string); $i++) {
			if ($string[$i] != chr(0)) {
				$result .= $string[$i];
			}
		}

		return ($result);
	}

	/** Encode full MungedDial-String
	 * @param array<string, string> $params parameters
	 * @return string result
	 */
	public function encode_munged($params): string {
		/* Walk through the parameters and convert them */
		$result = sambaMungedDial::hexstr(SAMBA_MUNGEDDIAL_FILEHEADER);

		// CHANGED: We need to insert the number of attributes right after SAMBA_MUNGEDDIAL_FILEHEADER.
		$counter = 0;
		$result_tmp = "";
		foreach ($params as $paramName => $paramValue) {
			/* String parameter? */
			if (in_array($paramName, $this->stringParams)) {
				$isString = TRUE;
				$paramValue = sambaMungedDial::strhex($paramValue . chr(0) . chr(0));
			}
			else {
				$isString = FALSE;
			}

			/* Time parameter? */
			if (in_array($paramName, $this->timeParams)) {
				$paramValue = sambaMungedDial::genTime($paramValue);
			}

			$result_tmp .= sambaMungedDial::munge($paramName, $paramValue, $isString);
			$counter++;
		}

		// First add the number of attributes
		$result .= sambaMungedDial::hexstr(sprintf("%02x00", $counter));

		// Then the usual stuff
		$result .= $result_tmp;

		return ($result);
	}

	/** Setup parameter given by paramName to MungedDial-Format
	 * @param string $paramName param name
	 * @param string $paramValue value
	 * @param bool $isString is string
	 * @return string value
	 */
	public function munge($paramName, $paramValue, $isString): string {
		$result = "";

		/* Encode paramName to UTF-16 */
		if (function_exists("recode")) {
			$utfName = recode("ISO8859-15..UTF-16", $paramName);
		}
		else {
			$utfName = iconv("ISO8859-15", "UTF-16BE", $paramName);
		}

		/* Set parameter length, high and low byte */
		$paramLen = strlen($utfName);
		$result .= chr($paramLen & 0x0FF);
		$result .= chr(($paramLen & 0x0FF00) >> 8);

		/* String parameters have additional trailing bytes */
		$valueLen = strlen($paramValue);
		$result .= chr($valueLen & 0x0FF);
		$result .= chr(($valueLen & 0x0FF00) >> 8);

		/* Length fields have a trailing '01' appended by the UTF-16 converted name */
		$result .= chr(1);
		$result .= $utfName;

		/* Parameter is padded with '00' */
		$result .= chr(0);
		$result .= $paramValue;

		/* Append a trailing '00' to string parameters */
		if ($isString && (strlen($paramValue) & 1)) {
			$result .= chr(0);
		}

		return ($result);
	}

	/** Takes a base64-encoded MungedDial-String and returns an array of included parameters and values
	 * @param string $munge munge
	 * @return array<string, string> values
	 */
	public function decode_munged($munge): array {
		$result = [];

		/*
		 * Remove base64 encoding and skip SAMBA_MUNGEDDIAL_FILEHEADER.
		 * The '4' is added, because the SAMBA_MUNGEDDIAL_FILEHEADER has been stripped by 4 chars.
		 * This is the number of attributes following - we don't need this at read time, only when writing.
		 */
		if (str_starts_with(base64_decode($munge), "6d")) {
			$this->old_behavior = true;
		}

		if ($this->old_behavior) {
			$ctxField = substr(base64_decode($munge), (strlen(SAMBA_MUNGEDDIAL_FILEHEADER_OLD)) / 2);
		}
		else {
			$ctxField = substr(base64_decode($munge), (strlen(SAMBA_MUNGEDDIAL_FILEHEADER) + 4) / 2);
		}

		/* Decode parameters */
		while ($ctxField !== "") {

			/* Read value lengths */
			$ctxParmNameLength = ord($ctxField[0]) + 16 * ord($ctxField[1]);
			$ctxParmLength = ord($ctxField[2]) + 16 * ord($ctxField[3]);

			/* Reposition ctxField on start of parameter name, read parameter name */
			$ctxField = substr($ctxField, 6);
			$ctxParmName = sambaMungedDial::to8bit(substr($ctxField, 0, $ctxParmNameLength));

			/* Reposition ctxField on start of parameter */
			$ctxField = substr($ctxField, $ctxParmNameLength);
			$ctxParm = substr($ctxField, 0, $ctxParmLength);

			/* If string parameter, convert */
			if (in_array($ctxParmName, $this->stringParams)) {
				$ctxParm = sambaMungedDial::hexstr($ctxParm);
			}
			/* If time parameter, convert */
			if (in_array($ctxParmName, $this->timeParams)) {
				$ctxParm = sambaMungedDial::readTime($ctxParm);
			}

			/* Assign in result array */
			$result[$ctxParmName] = trim($ctxParm);

			/* Reposition ctxField on end of parameter and continue */
			$ctxField = substr($ctxField, $ctxParmLength);
		}

		return ($result);
	}

	/** function takes a base64-encoded sambaMungedDial
	 * @param string $mungedDial munged
	 */
	public function load($mungedDial): void {
		$this->ctx = $this->decode_munged($mungedDial);
	}

	/** Returns ready-to-run mungedDialString to be filled into ldap
	 * @return string value
	 */
	public function getMunged(): string {
		// Do extra check for valid timeParams (they must be set to 0 if disabled)
		foreach ($this->timeParams as $value) {
			if (!isset($this->ctx[$value])) {
				$this->ctx[$value] = 0;
			}
		}
		return base64_encode($this->encode_munged($this->ctx));
	}

	/** Returns array of flags, which can be set on-demand with activated java-script
	 * @return array<string, string> values
	 */
	public function getOnDemandFlags(): array {
		$result = [];
		foreach ($this->timeParams as $value) {
			$result[$value . "Mode"] = "";
		}
		$result['CtxInitialProgramMode'] = "";
		return $result;
	}

	/** Gets Terminal-Server-Login value: enabled/disabled
	 * @return bool enabled
	 */
	public function getTsLogin(): bool {
		$flags = ord(substr($this->ctx['CtxCfgFlags1'], 5, 1));
		return ($flags & 1) === 0;
	}

	/** Sets Terminal-Server-Login value: enabled/disabled
	 * @param bool $checked checked
	 */
	public function setTsLogin($checked): void {
		$flag = (int) substr($this->ctx['CtxCfgFlags1'], 5, 1);

		if ($checked) {
			$flag |= 1;
		}
		else {
			$flag &= 0xFE;
		}

		$this->ctx['CtxCfgFlags1'][5] = sprintf('%1x', $flag);
	}

	/** gets Broken-Connection value: disconnect/reset
	 * @return string value
	 */
	public function getBrokenConn(): string {
		$flags = ord(substr($this->ctx['CtxCfgFlags1'], 5, 1));
		return (($flags & 4) !== 0) ? "1" : "0";
	}

	/** sets Broken-Connection value: disconnect/reset
	 * @param bool $checked checked
	 */
	public function setBrokenConn($checked): void {
		$flag = (int) substr($this->ctx['CtxCfgFlags1'], 5, 1);

		if ($checked) {
			$flag |= 4;
		}
		else {
			$flag &= 0xFB;
		}

		$this->ctx['CtxCfgFlags1'][5] = sprintf('%1x', $flag);
	}

	/** gets Reconnection value: from any client/from previous client only
	 * @return string value
	 */
	public function getReConn(): string {
		$flags = ord(substr($this->ctx['CtxCfgFlags1'], 5, 1));
		return (($flags & 2) !== 0) ? "1" : "0";
	}

	/** sets Reconnection value: from any client/from previous client only
	 * @param bool $checked checked
	 */
	public function setReConn($checked): void {
		$flag = (int) substr($this->ctx['CtxCfgFlags1'], 5, 1);

		if ($checked) {
			$flag |= 2;
		}
		else {
			$flag &= 0xFD;
		}

		$this->ctx['CtxCfgFlags1'][5] = sprintf('%1x', $flag);
	}

	/** gets Inherit-config-from-client value: enabled/disabled
	 * @return bool enabled
	 */
	public function getInheritMode(): bool {
		return substr($this->ctx['CtxCfgFlags1'], 6, 1) === "1";
	}

	/** sets Inherit-config-from-client value: enabled/disabled
	 * @param bool $checked checked
	 */
	public function setInheritMode($checked): void {
		$this->ctx['CtxCfgFlags1'][6] = $checked ? "1" : "0";
	}

	/** gets shadow value (enum): 0-4
	 * 0: disabled
	 * 1: input on, notify on
	 * 2: input on, notify off
	 * 3: input off, notify on
	 * 4: input off, notify off
	 * @return string value
	 */
	public function getShadow(): string {
		return $this->old_behavior ? substr($this->ctx['CtxCfgFlags1'], 1, 1) : substr($this->ctx['CtxShadow'], 1, 1);
	}

	/** sets shadow value
	 * @param bool $checked checked
	 * @param string $value value
	 */
	public function setShadow($checked, $value): void {
		if ($checked) {
			if ($this->old_behavior) {
				// We need to reset the old setting
				$this->ctx['CtxCfgFlags1'][1] = sprintf('%1X', $value);
			}
			$this->ctx['CtxShadow'][1] = sprintf('%1x', $value);
		}
	}

	/** gets connect-client-drive-at-logon value: enabled/disabled
	 * @return int enabled
	 */
	public function getConnectClientDrives(): int {
		$connections = hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));

		return $connections & 8;
	}

	/** sets connect-client-drive-at-logon value: enabled/disabled
	 * @param bool $checked checked
	 */
	public function setConnectClientDrives($checked): void {
		$flag = hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));
		if ($checked) {
			$flag |= 8;
		}
		else {
			$flag &= 0xF7;
		}

		$this->ctx['CtxCfgFlags1'][2] = sprintf('%1x', $flag);
	}

	/** gets connect-client-printers-at-logon value: enabled/disabled
	 * @return int enabled
	 */
	public function getConnectClientPrinters(): int {
		$connections = hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));

		return $connections & 4;
	}

	/** sets connect-client-printers-at-logon value: enabled/disabled
	 * @param bool $checked checked
	 */
	public function setConnectClientPrinters($checked): void {
		$flag = hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));

		if ($checked) {
			$flag |= 4;
		}
		else {
			$flag &= 0xFB;
		}

		$this->ctx['CtxCfgFlags1'][2] = sprintf('%1x', $flag);
	}

	/** gets set-client-printer-to-default value: enabled/disabled
	 * @return int enabled
	 */
	public function getDefaultPrinter(): int {
		$connections = hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));

		return $connections & 2;
	}

	/** sets set-client-printer-to-default value: enabled/disabled
	 * @param bool $checked checked
	 */
	public function setDefaultPrinter($checked): void {
		$flag = hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));

		if ($checked) {
			$flag |= 2;
		}
		else {
			$flag &= 0xFD;
		}

		$this->ctx['CtxCfgFlags1'][2] = sprintf('%1x', $flag);
	}

	/** SMARTY: gets the checkbox state of "Connection" */
	public function getCtxMaxConnectionTimeF(): bool {
		// Connection Time is 0 if disabled
		return isset($this->ctx['CtxMaxConnectionTime']) && ($this->ctx['CtxMaxConnectionTime'] != 0);
	}

	/** SMARTY: sets the checkbox "Connection" to unchecked
	 * @param bool $checked checked
	 */
	public function setCtxMaxConnectionTimeF($checked): void {
		if ($checked) {
			unset ($this->ctx['CtxMaxConnectionTime']);
		}
	}

	/** SMARTY: gets the checkbox state of "Disconnection"
	 * @return bool disconnection time set
	 */
	public function getCtxMaxDisconnectionTimeF(): bool {
		// Connection Time is 0 if disabled
		return isset($this->ctx['CtxMaxDisconnectionTime']) && ($this->ctx['CtxMaxDisconnectionTime'] != 0);
	}

	/** SMARTY: sets the checkbox "Disconnection" to unchecked
	 * @param bool $checked checked
	 */
	public function setCtxMaxDisconnectionTimeF($checked): void {
		if ($checked) {
			unset ($this->ctx['CtxMaxDisconnectionTime']);
		}
	}

	/** SMARTY: gets the checkbox state of "Idle"
	 * @return bool is idle time set
	 */
	public function getCtxMaxIdleTimeF(): bool {
		// Connection Time is 0 if disabled
		return isset($this->ctx['CtxMaxIdleTime']) && ($this->ctx['CtxMaxIdleTime'] != 0);
	}

	/** SMARTY: sets the checkbox "Idle" to unchecked
	 * @param bool $checked checked
	 */
	public function setCtxMaxIdleTimeF($checked): void {
		if ($checked) {
			unset ($this->ctx['CtxMaxIdleTime']);
		}
	}
}
