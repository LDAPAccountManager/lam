<?php

use LAM\PDF\PDFTable;
use LAM\PDF\PDFTableCell;
use LAM\PDF\PDFTableRow;
use LAM\REMOTE\Remote;
use LAM\REMOTE\RemoteServerConfiguration;
use LAM\TYPES\ConfiguredType;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2003 - 2006  Tilo Lutz
                2007 - 2026  Roland Gruber

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
 * Manages quotas for users and groups.
 *
 * @package modules
 *
 * @author Tilo Lutz
 * @author Roland Gruber
 * @author Michael Duergner
 */

/**
 * Manages quotas for users and groups.
 *
 * @package modules
 */
class quota extends baseModule {

	/** delimiter for lamdaemon commands */
	private static string $SPLIT_DELIMITER = "###x##y##x###";
	/** prefix for lamdaemon results */
	private static string $QUOTA_PREFIX = 'QUOTA_ENTRY ';

	/**
	 * {@inheritDoc}
	 */
	protected function load_Messages(): void {
		// error messages for input checks
		$this->messages['softblock'][0] = ['ERROR', _('Block soft quota'), _('Block soft quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['softblock'][1] = ['ERROR', _('Account %s:') . ' %s', _('Block soft quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['hardblock'][0] = ['ERROR', _('Block hard quota'), _('Block hard quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['hardblock'][1] = ['ERROR', _('Account %s:') . ' %s', _('Block hard quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['softinode'][0] = ['ERROR', _('Inode soft quota'), _('Inode soft quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['softinode'][1] = ['ERROR', _('Account %s:') . ' %s', _('Inode soft quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['hardinode'][0] = ['ERROR', _('Inode hard quota'), _('Inode hard quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['hardinode'][1] = ['ERROR', _('Account %s:') . ' %s', _('Inode hard quota contains invalid characters. Only natural numbers are allowed.')];
		$this->messages['block_cmp'][0] = ['ERROR', _('Block quota'), _('Block soft quota must be smaller than block hard quota.')];
		$this->messages['block_cmp'][1] = ['ERROR', _('Account %s:') . ' %s', _('Block soft quota must be smaller than block hard quota.')];
		$this->messages['inode_cmp'][0] = ['ERROR', _('Inode quota'), _('Inode soft quota must be smaller than inode hard quota.')];
		$this->messages['inode_cmp'][1] = ['ERROR', _('Account %s:') . ' %s', _('Inode soft quota must be smaller than inode hard quota.')];
		$this->messages['upload'][0] = ['ERROR', _('Account %s:') . ' %s', _('Quota has wrong format!')];
	}

	/**
	 * {@inheritDoc}
	 */
	public function can_manage(): bool {
		return in_array($this->get_scope(), ['user', 'group']);
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_metaData(): array {
		$return = [];
		// icon
		$return['icon'] = 'hard-drive.svg';
		// alias name
		$return["alias"] = _('Quota');
		if ($this->get_scope() == 'group') {
			// module dependencies
			$return['dependencies'] = ['depends' => [['posixGroup', 'rfc2307bisPosixGroup', 'windowsPosixGroup']], 'conflicts' => []];
		}
		if ($this->get_scope() == 'user') {
			// module dependencies
			$return['dependencies'] = ['depends' => ['posixAccount'], 'conflicts' => []];
		}
		// managed attributes
		$return['attributes'] = ['uid', 'cn'];
		// available PDF fields
		$return['PDF_fields'] = [
			'quotas' => _('Quota')
		];
		// help entries
		$numbersHelp = _('Symbols K, M, G and T can be appended to numeric value to express kibibytes, mebibytes, gibibytes and tebibytes for blocks.')
			. _(' Symbols k, m, g and t can be appended to numeric value to express multiples of 10^3, 10^6, 10^9 and 10^12 inodes.');
		$return['help'] = [
			"UsedBlocks" => [
				"Headline" => _("Used blocks"),
				"Text" => _("Used blocks. 1000 blocks are usually 1MB")
			],
			"SoftBlockLimit" => [
				"Headline" => _("Soft block limit"),
				"Text" => _("Soft block limit.") . '<br>' . $numbersHelp, "SeeAlso" => [
					'link' => 'https://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4',
					'text' => 'Quota How-To']
			],
			"HardBlockLimit" => [
				"Headline" => _("Hard block limit"),
				"Text" => _("Hard block limit") . '<br>' . $numbersHelp, "SeeAlso" => [
					'link' => 'https://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5',
					'text' => 'Quota How-To']
			],
			"GraceBlockPeriod" => [
				"Headline" => _("Grace block period"),
				"Text" => _("Grace block period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => [
					'link' => 'https://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6',
					'text' => 'Quota How-To']
			],
			"UsedInodes" => [
				"Headline" => _("Used inodes"),
				"Text" => _("Used inodes (files)") . '.'
			],
			"SoftInodeLimit" => [
				"Headline" => _("Soft inode limit"),
				"Text" => _("Soft inode (files) limit.") . '<br>' . $numbersHelp, "SeeAlso" => [
					'link' => 'https://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.4',
					'text' => 'Quota How-To']
			],
			"HardInodeLimit" => [
				"Headline" => _("Hard inode limit"),
				"Text" => _("Hard inode (files) limit") . '<br>' . $numbersHelp, "SeeAlso" => [
					'link' => 'https://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.5',
					'text' => 'Quota How-To']
			],
			"GraceInodePeriod" => [
				"Headline" => _("Grace inode period"),
				"Text" => _("Grace inode (files) period. Most filesystems use a fixed maximum value of 7 days."), "SeeAlso" => [
					'link' => 'https://www.ibiblio.org/pub/Linux/docs/HOWTO/other-formats/html_single/Quota.html#ss4.6',
					'text' => 'Quota How-To']
			],
			"upload" => [
				"Headline" => _("Quota"),
				"Text" => _("Please enter the quota settings for this mount point. The syntax is: {soft block limit},{hard block limit},{soft inode limit},{hard inode limit}")
			]
		];
		// self-service field settings
		$return['selfServiceFieldSettings'] = ['quota' => _('Quota (read-only)')];
		return $return;
	}

	/** @var array<string, array<int, int|string>[]> Saves the quota settings */
	private ?array $quota = null;

	/**
	 * Initializes the quota values.
	 *
	 * @param string $userName user/group name
	 * @param RemoteServerConfiguration[] $lamdaemonServers server configurations
	 */
	private function initQuotas($userName = null, $lamdaemonServers = null): void {
		if ($this->quota !== null) {
			return;
		}
		if ($userName === null) {
			$userName = '+';
			if (($this->getAccountContainer() != null) && !$this->getAccountContainer()->isNewAccount) {
				if ($this->get_scope() == 'user') {
					if (!isset($this->attributes['uid'][0])) {
						return;
					}
					$userName = $this->attributes['uid'][0];
				}
				elseif ($this->get_scope() == 'group') {
					if (!isset($this->attributes['cn'][0])) {
						return;
					}
					$userName = $this->attributes['cn'][0];
				}
			}
		}
		// get list of lamdaemon servers
		if ($lamdaemonServers === null) {
			$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
		}
		foreach ($lamdaemonServers as $lamdaemonServer) {
			$server = $lamdaemonServer->getServer();
			// get quotas
			$remote = new Remote();
			try {
				$remote->connect($lamdaemonServer);
			}
			catch (LAMException $e) {
				logNewMessage(LOG_ERR, $e->getMessage() . ' ' . $e->getTitle());
				continue;
			}
			$quotas = $remote->execute(implode(self::$SPLIT_DELIMITER, [$userName, "quota", "get", $this->get_scope()]));
			$remote->disconnect();
			if (empty($quotas)) {
				continue;
			}
			$allQuotas = explode(":", $quotas);
			array_pop($allQuotas); // remove empty element at the end
			for ($i = 0; $i < count($allQuotas); $i++) {
				if (!str_starts_with($allQuotas[$i], self::$QUOTA_PREFIX)) {
					continue;
				}
				$allQuotas[$i] = substr($allQuotas[$i], strlen(self::$QUOTA_PREFIX));
				$singleQuota = explode(",", $allQuotas[$i]);
				$singleQuota[1] = $this->formatBlockUsage((int) $singleQuota[1]);
				$singleQuota[2] = $this->addBlockUnits((int) $singleQuota[2]);
				$singleQuota[3] = $this->addBlockUnits((int) $singleQuota[3]);
				$singleQuota[5] = $this->formatInodeUsage((int) $singleQuota[5]);
				$singleQuota[6] = $this->addInodeUnits((int) $singleQuota[6]);
				$singleQuota[7] = $this->addInodeUnits((int) $singleQuota[7]);
				$this->quota[$server][$i] = $singleQuota;
				if ($this->quota[$server][$i][4] < time()) {
					$this->quota[$server][$i][4] = '-';
				}
				else {
					$this->quota[$server][$i][4] = intval((((int) $this->quota[$server][$i][4]) - time()) / 3600) . ' ' . _('hours');
				}
				if ($this->quota[$server][$i][8] < time()) {
					$this->quota[$server][$i][8] = '-';
				}
				else {
					$this->quota[$server][$i][8] = intval((((int) $this->quota[$server][$i][8]) - time()) / 3600) . ' ' . _('hours');
				}
			}
		}
	}

	/**
	 * Adds units (M/G/T) for block numbers.
	 *
	 * @param int $value raw value
	 * @return string value with unit
	 */
	public function addBlockUnits($value): string {
		$mebibytes = 1024;
		$gibibytes = 1024 * $mebibytes;
		$tebibytes = 1024 * $gibibytes;
		if (empty($value) || !get_preg((string) $value, 'digit') || ($value < $mebibytes)) {
			return (string) $value;
		}
		if (($value >= $tebibytes) && (($value % $tebibytes) === 0)) {
			return ($value / $tebibytes) . 'T';
		}
		if (($value >= $gibibytes) && (($value % $gibibytes) === 0)) {
			return ($value / $gibibytes) . 'G';
		}
		if (($value % $mebibytes) === 0) {
			return ($value / $mebibytes) . 'M';
		}
		return (string) $value;
	}

	/**
	 * Formats block usage.
	 *
	 * @param int $value raw value
	 * @return string block usage
	 */
	public function formatBlockUsage($value): string {
		$mebibytes = 1024;
		$gibibytes = 1024 * $mebibytes;
		$tebibytes = 1024 * $gibibytes;
		if (empty($value) || !get_preg((string) $value, 'digit') || ($value < $mebibytes)) {
			return (string) $value;
		}
		if ($value >= $tebibytes) {
			return round($value / $tebibytes, 2) . 'T';
		}
		if ($value >= $gibibytes) {
			return round($value / $gibibytes, 2) . 'G';
		}
		return round($value / $mebibytes, 2) . 'M';
	}

	/**
	 * Adds units (m/g/t) for inode numbers.
	 *
	 * @param int $value raw value
	 * @return string value with unit
	 */
	public function addInodeUnits($value): string {
		$kilo = 1000;
		$million = 1000 * $kilo;
		$billion = 1000 * $million;
		$trillion = 1000 * $billion;
		if (empty($value) || !get_preg((string) $value, 'digit')) {
			return (string) $value;
		}
		if (($value >= $trillion) && (($value % $trillion) === 0)) {
			return ($value / $trillion) . 't';
		}
		if (($value >= $billion) && (($value % $billion) === 0)) {
			return ($value / $billion) . 'g';
		}
		if (($value >= $million) && (($value % $million) === 0)) {
			return ($value / $million) . 'm';
		}
		if (($value >= $kilo) && (($value % $kilo) === 0)) {
			return ($value / $kilo) . 'k';
		}
		return (string) $value;
	}

	/**
	 * Formats the inode usage.
	 *
	 * @param int $value raw value
	 * @return string value with unit
	 */
	public function formatInodeUsage($value): string {
		$kilo = 1000;
		$million = 1000 * $kilo;
		$billion = 1000 * $million;
		$trillion = 1000 * $billion;
		if (empty($value) || !get_preg((string) $value, 'digit')) {
			return '-';
		}
		if ($value >= $trillion) {
			return round($value / $trillion, 2) . 't';
		}
		if ($value >= $billion) {
			return round($value / $billion, 2) . 'g';
		}
		if ($value >= $million) {
			return round($value / $million, 2) . 'm';
		}
		if ($value >= $kilo) {
			return round($value / $kilo, 2) . 'k';
		}
		return (string) $value;
	}

	/**
	 * Gets the cn from the Unix group module.
	 *
	 * @return string|null cn attribute
	 */
	private function getCn(): ?string {
		$modules = ['posixGroup', 'windowsPosixGroup', 'rfc2307bisPosixGroup', 'groupOfNames', 'groupOfUniqueNames', 'windowsGroup'];
		for ($i = 0; $i < count($modules); $i++) {
			if ($this->getAccountContainer()->getAccountModule($modules[$i]) != null) {
				$attrs = $this->getAccountContainer()->getAccountModule($modules[$i])->getAttributes();
				if (isset($attrs['cn'][0])) {
					return $attrs['cn'][0];
				}
			}
		}
		return null;
	}

	/**
	 * {@inheritDoc}
	 */
	public function module_ready(): bool {
		if ($this->get_scope() == 'user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			if (empty($attrs['uid'][0])) {
				return false;
			}
		}
		return ($this->get_scope() != 'group') || !empty($this->getCn());
	}

	/**
	 * {@inheritDoc}
	 */
	public function postModifyActions(bool $newAccount, array $attributes): array {
		$messages = [];
		if (!is_array($this->quota)
			|| ($this->getAccountContainer() === null)
			|| ($this->getAccountContainer()->finalDN === null)) {
			return $messages;
		}
		// determine if this is a user or group account
		if ($this->get_scope() === 'user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$id = $attrs['uid'][0];
		}
		else {
			$id = $this->getCn();
		}
		// get list of lamdaemon servers
		$servers = array_keys($this->quota);
		for ($q = 0; $q < count($servers); $q++) {
			$server = $servers[$q];
			$quotastring = "";
			for ($i = 0; $i < count($this->quota[$server]); $i++) {
				$quotastring .= $this->quota[$server][$i][0];
				$quotastring .= ',' . $this->getQuotaNumber($this->quota[$server][$i][2]);
				$quotastring .= ',' . $this->getQuotaNumber($this->quota[$server][$i][3]);
				$quotastring .= ',' . $this->getQuotaNumber($this->quota[$server][$i][6]);
				$quotastring .= ',' . $this->getQuotaNumber($this->quota[$server][$i][7]) . ':';
			}
			$remote = new Remote();
			$remoteServer = $_SESSION['config']->getScriptServerByName($server);
			try {
				$remote->connect($remoteServer);
			}
			catch (LAMException $e) {
				$messages[] = ['ERROR', $e->getTitle(), $e->getMessage()];
				continue;
			}
			$output = $remote->execute(implode(self::$SPLIT_DELIMITER, [$id, "quota", "set", $this->get_scope(), "$quotastring\n"]));
			if (str_starts_with($output, 'ERROR,')) {
				$messages[] = ['ERROR', $server, _('Unable to set quota.')];
			}
			$remote->disconnect();
		}
		return $messages;
	}

	/**
	 * Returns the quota value as plain number.
	 *
	 * @param string $quotaInput quota value (might contain e.g. "K" for KB)
	 * @return int quota number
	 */
	public function getQuotaNumber($quotaInput) {
		if (empty($quotaInput) || (strlen($quotaInput) < 2)) {
			return (int) $quotaInput;
		}
		if (str_ends_with($quotaInput, 'K')) {
			return (int) substr($quotaInput, 0, -1);
		}
		if (str_ends_with($quotaInput, 'M')) {
			return 1024 * ((int) substr($quotaInput, 0, -1));
		}
		if (str_ends_with($quotaInput, 'G')) {
			return 1024 * 1024 * ((int) substr($quotaInput, 0, -1));
		}
		if (str_ends_with($quotaInput, 'T')) {
			return 1024 * 1024 * 1024 * ((int) substr($quotaInput, 0, -1));
		}
		if (str_ends_with($quotaInput, 'k')) {
			return 1000 * ((int) substr($quotaInput, 0, -1));
		}
		if (str_ends_with($quotaInput, 'm')) {
			return 1000 * 1000 * ((int) substr($quotaInput, 0, -1));
		}
		if (str_ends_with($quotaInput, 'g')) {
			return 1000 * 1000 * 1000 * ((int) substr($quotaInput, 0, -1));
		}
		if (str_ends_with($quotaInput, 't')) {
			return 1000 * 1000 * 1000 * 1000 * ((int) substr($quotaInput, 0, -1));
		}
		return (int) $quotaInput;
	}

	/**
	 * {@inheritDoc}
	 */
	public function preDeleteActions(): array {
		try {
			$this->initQuotas();
		}
		catch (LAMException $e) {
			return [['ERROR', $e->getTitle(), $e->getMessage()]];
		}
		if (!is_array($this->quota)) {
			return [];
		}
		// determine if this is a user or group account
		if ($this->get_scope() === 'user') {
			$attrs = $this->getAccountContainer()->getAccountModule('posixAccount')->getAttributes();
			$id = $attrs['uid'][0];
		}
		else {
			$id = $this->getCn();
		}
		$messages = [];
		// get list of lamdaemon servers
		$servers = array_keys($this->quota);
		for ($q = 0; $q < count($servers); $q++) {
			$server = $servers[$q];
			$i = 0;
			$quotastring = "";
			while (isset($this->quota[$server][$i][0])) {
				$quotastring = $quotastring . $this->quota[$server][$i][0] . ',0,0,0,0:';
				$i++;
			}
			$remote = new Remote();
			$remoteServer = $_SESSION['config']->getScriptServerByName($server);
			try {
				$remote->connect($remoteServer);
			}
			catch (LAMException $e) {
				$messages[] = ['ERROR', $e->getTitle(), $e->getMessage()];
				continue;
			}
			$remote->execute(implode(self::$SPLIT_DELIMITER, [$id, "quota", "set", $this->get_scope(), "$quotastring\n"]));
			$remote->disconnect();
		}
		return $messages;
	}

	/**
	 * {@inheritDoc}
	 */
	public function process_attributes(): array {
		if (!is_array($this->quota)) {
			return [];
		}
		$errors = [];
		// get list of lamdaemon servers
		$servers = array_keys($this->quota);
		for ($q = 0; $q < count($servers); $q++) {
			$server = $servers[$q];
			$id = $this->replaceSpecialChars($server);
			$i = 0;
			// loop for every mointpoint with quotas
			while (isset($this->quota[$server][$i][0])) {
				$this->quota[$server][$i][2] = $_POST[$i . '_2_' . $id];
				$this->quota[$server][$i][3] = $_POST[$i . '_3_' . $id];
				$this->quota[$server][$i][6] = $_POST[$i . '_6_' . $id];
				$this->quota[$server][$i][7] = $_POST[$i . '_7_' . $id];
				// Check if values are OK and set automatic values. if not error-variable will be set
				if (!get_preg($this->quota[$server][$i][2], 'quotaNumber')) {
					$errors[] = $this->messages['softblock'][0];
				}
				if (!get_preg($this->quota[$server][$i][3], 'quotaNumber')) {
					$errors[] = $this->messages['hardblock'][0];
				}
				if (!get_preg($this->quota[$server][$i][6], 'quotaNumber')) {
					$errors[] = $this->messages['softinode'][0];
				}
				if (!get_preg($this->quota[$server][$i][7], 'quotaNumber')) {
					$errors[] = $this->messages['hardinode'][0];
				}
				if ($this->getQuotaNumber($this->quota[$server][$i][2]) > $this->getQuotaNumber($this->quota[$server][$i][3])) {
					$errors[] = $this->messages['block_cmp'][0];
				}
				if ($this->getQuotaNumber($this->quota[$server][$i][6]) > $this->getQuotaNumber($this->quota[$server][$i][7])) {
					$errors[] = $this->messages['inode_cmp'][0];
				}
				$i++;
			}
		}
		return $errors;
	}

	/**
	 * {@inheritDoc}
	 */
	public function display_html_attributes(): htmlElement|array {
		$return = new htmlResponsiveRow();
		try {
			$this->initQuotas();
		}
		catch (LAMException $e) {
			$return->add(new htmlStatusMessage('ERROR', $e->getTitle(), $e->getMessage()));
			return $return;
		}
		if (!is_array($this->quota)) {
			$return->add(new htmlStatusMessage('ERROR', _('Unable to read quota.')));
			return $return;
		}
		// get list of lamdaemon servers
		$serverDescriptions = [];
		$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
		for ($s = 0; $s < count($lamdaemonServers); $s++) {
			$lamdaemonServer = $lamdaemonServers[$s];
			$serverDescriptions[$lamdaemonServer->getServer()] = $lamdaemonServer->getLabel();
		}
		$servers = array_keys($this->quota);
		for ($q = 0; $q < count($servers); $q++) {
			$server = $servers[$q];
			$id = $this->replaceSpecialChars($server);
			$title = $server;
			if ($serverDescriptions[$server] != $server) {
				$title = $serverDescriptions[$server] . " (" . $server . ")";
			}
			$return->add(new htmlSubTitle($title));

			$i = 0;
			// loop for every mointpoint with enabled quotas
			while (isset($this->quota[$server][$i][0])) {
				$return->addLabel(new htmlOutputText(_('Mountpoint')));
				$return->addField(new htmlOutputText($this->quota[$server][$i][0]));
				$return->addVerticalSpacer('0.5rem');
				$usedBlockGroup = new htmlGroup();
				$usedBlockGroup->addElement(new htmlOutputText($this->quota[$server][$i][1]));
				$usedBlockGroup->addElement(new htmlSpacer('0.5rem'));
				$usedBlockGroup->addElement(new htmlHelpLink('UsedBlocks'));
				$return->addLabel(new htmlOutputText(_('Used blocks')));
				$return->addField($usedBlockGroup);
				$return->add(new htmlResponsiveInputField(_('Soft block limit'), $i . '_2_' . $id, $this->quota[$server][$i][2], 'SoftBlockLimit'));
				$return->add(new htmlResponsiveInputField(_('Hard block limit'), $i . '_3_' . $id, $this->quota[$server][$i][3], 'HardBlockLimit'));
				$graceBlockPeriodGroup = new htmlGroup();
				$graceBlockPeriodGroup->addElement(new htmlOutputText($this->quota[$server][$i][4]));
				$graceBlockPeriodGroup->addElement(new htmlSpacer('0.5rem'));
				$graceBlockPeriodGroup->addElement(new htmlHelpLink('GraceBlockPeriod'));
				$return->addLabel(new htmlOutputText(_('Grace block period')));
				$return->addField($graceBlockPeriodGroup);
				$usedInodesGroup = new htmlGroup();
				$usedInodesGroup->addElement(new htmlOutputText($this->quota[$server][$i][5]));
				$usedInodesGroup->addElement(new htmlSpacer('0.5rem'));
				$usedInodesGroup->addElement(new htmlHelpLink('UsedInodes'));
				$return->addLabel(new htmlOutputText(_('Used inodes')));
				$return->addField($usedInodesGroup);
				$return->add(new htmlResponsiveInputField(_('Soft inode limit'), $i . '_6_' . $id, $this->quota[$server][$i][6], 'SoftInodeLimit'));
				$return->add(new htmlResponsiveInputField(_('Hard inode limit'), $i . '_7_' . $id, $this->quota[$server][$i][7], 'HardInodeLimit'));
				$graceInodePeriodGroup = new htmlGroup();
				$graceInodePeriodGroup->addElement(new htmlOutputText($this->quota[$server][$i][8]));
				$graceInodePeriodGroup->addElement(new htmlSpacer('0.5rem'));
				$graceInodePeriodGroup->addElement(new htmlHelpLink('GraceInodePeriod'));
				$return->addLabel(new htmlOutputText(_('Grace inode period')));
				$return->addField($graceInodePeriodGroup);
				$return->addVerticalSpacer('3rem');
				$i++;
			}
		}
		return $return;
	}

	/**
	 * Replaces special characters in HTML name values.
	 *
	 * @param string $input input string
	 * @return string output string
	 */
	private function replaceSpecialChars($input) {
		return str_replace(".", "_", $input);
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_profileOptions(string $typeId): ?htmlResponsiveRow {
		$return = new htmlResponsiveRow();
		$optionsAvailable = false;
		// get list of lamdaemon servers
		$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
		try {
			foreach ($lamdaemonServers as $lamdaemonServer) {
				$server = $lamdaemonServer->getServer();
				$id = $this->replaceSpecialChars($server);
				$description = $lamdaemonServer->getLabel();
				if ($description != $server) {
					$description = $description . ' (' . $server . ')';
				}
				// Get quotas
				$remote = new Remote();
				$remote->connect($lamdaemonServer);
				$quotas = $remote->execute(implode(self::$SPLIT_DELIMITER, ["+", "quota", "get", $this->get_scope()]));
				$remote->disconnect();
				if (empty($quotas)) {
					continue;
				}
				$dirs = explode(":", $quotas);
				array_pop($dirs); // remove the empty element at the end
				if (empty($dirs)) {
					continue;
				}
				for ($i = 0; $i < count($dirs); $i++) {
					if (!str_starts_with($dirs[$i], self::$QUOTA_PREFIX)) {
						unset($dirs[$i]);
						$dirs = array_values($dirs);
						$i--;
						continue;
					}
					$dirs[$i] = substr($dirs[$i], strlen(self::$QUOTA_PREFIX));
					$dirs[$i] = explode(",", $dirs[$i]);
					$dirs[$i] = $dirs[$i][0];
				}
				$dirs = array_values($dirs);
				if (count($dirs) < 1) {
					continue; // stop if no quota directories were found
				}
				$optionsAvailable = true;
				$return->add(new htmlSubTitle($description));
				for ($i = 0; $i < count($dirs); $i++) {
					$return->add(new htmlOutputText($dirs[$i]));
					$sbLimit = new htmlResponsiveInputField(_('Soft block limit'), "quota_softblock_" . $id . "_" . $dirs[$i], null, 'SoftBlockLimit');
					$sbLimit->setFieldMaxLength(20);
					$return->add($sbLimit);
					$hbLimit = new htmlResponsiveInputField(_('Hard block limit'), "quota_hardblock_" . $id . "_" . $dirs[$i], null, 'HardBlockLimit');
					$hbLimit->setFieldMaxLength(20);
					$return->add($hbLimit);
					$siLimit = new htmlResponsiveInputField(_('Soft inode limit'), "quota_softinode_" . $id . "_" . $dirs[$i], null, 'SoftInodeLimit');
					$siLimit->setFieldMaxLength(20);
					$return->add($siLimit);
					$hiLimit = new htmlResponsiveInputField(_('Hard inode limit'), "quota_hardinode_" . $id . "_" . $dirs[$i], null, 'HardInodeLimit');
					$hiLimit->setFieldMaxLength(20);
					$return->add($hiLimit);
				}
			}
		}
		catch (LAMException $e) {
			$return->add(new htmlStatusMessage('WARN', $e->getTitle()));
			return $return;
		}
		if (!$optionsAvailable) {
			return null;
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function check_profileOptions(array $options, string $typeId): array {
		$return = [];
		// get list of lamdaemon servers
		$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
		foreach ($lamdaemonServers as $lamdaemonServer) {
			$server = $lamdaemonServer->getServer();
			$id = $this->replaceSpecialChars($server);
			// Get quotas
			$remote = new Remote();
			try {
				$remote->connect($lamdaemonServer);
			}
			catch (LAMException $e) {
				$return[] = ['ERROR', $e->getTitle(), $e->getMessage()];
				continue;
			}
			$quotas = $remote->execute(implode(self::$SPLIT_DELIMITER, ["+", "quota", "get", $this->get_scope()]));
			$remote->disconnect();
			$dirs = explode(":", $quotas);
			array_pop($dirs); // remove empty element at the end
			if (empty($dirs)) {
				continue;
			}
			$quotaDirs = [];
			foreach ($dirs as $dirString) {
				if (!str_starts_with($dirString, self::$QUOTA_PREFIX)) {
					continue;
				}
				$dirData = explode(",", substr($dirString, strlen(self::$QUOTA_PREFIX)));
				$quotaDirs[] = $dirData[0];
			}
			foreach ($quotaDirs as $dirName) {
				if (!empty($options["quota_softblock_" . $id . "_" . $dirName][0]) && !get_preg($options["quota_softblock_" . $id . "_" . $dirName][0], 'quotaNumber')) {
					$return[] = $this->messages['softblock'][0];
				}
				if (!empty($options["quota_hardblock_" . $id . "_" . $dirName][0]) && !get_preg($options["quota_hardblock_" . $id . "_" . $dirName][0], 'quotaNumber')) {
					$return[] = $this->messages['hardblock'][0];
				}
				if (!empty($options["quota_softinode_" . $id . "_" . $dirName][0]) && !get_preg($options["quota_softinode_" . $id . "_" . $dirName][0], 'quotaNumber')) {
					$return[] = $this->messages['softinode'][0];
				}
				if (!empty($options["quota_hardinode_" . $id . "_" . $dirName][0]) && !get_preg($options["quota_hardinode_" . $id . "_" . $dirName][0], 'quotaNumber')) {
					$return[] = $this->messages['hardinode'][0];
				}
				if (!empty($options["quota_softblock_" . $id . "_" . $dirName][0])
					&& !empty($options["quota_hardblock_" . $id . "_" . $dirName][0])
					&& ($this->getQuotaNumber($options["quota_softblock_" . $id . "_" . $dirName][0]) > $this->getQuotaNumber($options["quota_hardblock_" . $id . "_" . $dirName][0]))) {
					$return[] = $this->messages['block_cmp'][0];
				}
				if (!empty($options["quota_softinode_" . $id . "_" . $dirName][0])
					&& !empty($options["quota_hardinode_" . $id . "_" . $dirName][0])
					&& ($this->getQuotaNumber($options["quota_softinode_" . $id . "_" . $dirName][0]) > $this->getQuotaNumber($options["quota_hardinode_" . $id . "_" . $dirName][0]))) {
					$return[] = $this->messages['inode_cmp'][0];
				}
			}
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function load_profile(array $profile): void {
		try {
			$this->initQuotas();
		}
		catch (LAMException $e) {
			logNewMessage(LOG_ERR, $e->getTitle() . ' - ' . $e->getMessage());
		}
		if (!is_array($this->quota)) {
			return;
		}
		$servers = array_keys($this->quota);
		for ($s = 0; $s < count($servers); $s++) {
			$server = $servers[$s];
			$id = $this->replaceSpecialChars($server);
			for ($i = 0; $i < count($this->quota[$server]); $i++) {
				$dir = $this->quota[$server][$i][0];
				if (isset($profile["quota_softblock_" . $id . "_" . $dir])) {
					$this->quota[$server][$i][2] = $profile["quota_softblock_" . $id . "_" . $dir][0];
				}
				if (isset($profile["quota_hardblock_" . $id . "_" . $dir])) {
					$this->quota[$server][$i][3] = $profile["quota_hardblock_" . $id . "_" . $dir][0];
				}
				if (isset($profile["quota_softinode_" . $id . "_" . $dir])) {
					$this->quota[$server][$i][6] = $profile["quota_softinode_" . $id . "_" . $dir][0];
				}
				if (isset($profile["quota_hardinode_" . $id . "_" . $dir])) {
					$this->quota[$server][$i][7] = $profile["quota_hardinode_" . $id . "_" . $dir][0];
				}
			}
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_pdfEntries(array $pdfKeys, string $typeId): array {
		try {
			$this->initQuotas();
		}
		catch (LAMException $e) {
			logNewMessage(LOG_ERR, $e->getTitle() . ' - ' . $e->getMessage());
			return [];
		}
		if (!is_array($this->quota)) {
			return [];
		}
		if ($this->quota !== []) {
			$pdfTable = new PDFTable();
			// get list of lamdaemon servers
			$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
			foreach ($lamdaemonServers as $lamdaemonServer) {
				$server = $lamdaemonServer->getServer();
				$description = $lamdaemonServer->getLabel();
				if ($description != $server) {
					$description = $description . " (" . $server . ")";
				}
				if (!isset($this->quota[$server]) || (count($this->quota[$server]) < 1)) {
					continue;
				}
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell($description, null, null, true);
				$pdfTable->rows[] = $pdfRow;
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell(_('Mountpoint'), '28%', null, true);
				$pdfRow->cells[] = new PDFTableCell(_('Soft block'), '18%', null, true);
				$pdfRow->cells[] = new PDFTableCell(_('Hard block'), '18%', null, true);
				$pdfRow->cells[] = new PDFTableCell(_('Soft inode'), '18%', null, true);
				$pdfRow->cells[] = new PDFTableCell(_('Hard inode'), '18%', null, true);
				$pdfTable->rows[] = $pdfRow;
				for ($i = 0; $i < count($this->quota[$server]); $i++) {
					$pdfRow = new PDFTableRow();
					$pdfRow->cells[] = new PDFTableCell($this->quota[$server][$i][0], '28%');
					$pdfRow->cells[] = new PDFTableCell($this->quota[$server][$i][2], '18%');
					$pdfRow->cells[] = new PDFTableCell($this->quota[$server][$i][3], '18%');
					$pdfRow->cells[] = new PDFTableCell($this->quota[$server][$i][6], '18%');
					$pdfRow->cells[] = new PDFTableCell($this->quota[$server][$i][7], '18%');
					$pdfTable->rows[] = $pdfRow;
				}
				$pdfRow = new PDFTableRow();
				$pdfRow->cells[] = new PDFTableCell(' ');
				$pdfTable->rows[] = $pdfRow;
			}
			$return = [];
			$this->addPDFTable($return, 'quotas', $pdfTable);
			return $return;
		}
		else {
			return [];
		}
	}

	/**
	 * {@inheritDoc}
	 */
	public function get_uploadColumns(array $selectedModules, ConfiguredType $type): array {
		try {
			$this->initQuotas();
		}
		catch (LAMException $e) {
			logNewMessage(LOG_ERR, $e->getTitle() . ' - ' . $e->getMessage());
			return [];
		}
		if (!is_array($this->quota)) {
			return [];
		}
		$return = [];
		if ($this->quota !== []) {
			// get list of lamdaemon servers
			$lamdaemonServers = $_SESSION['config']->getConfiguredScriptServers();
			foreach ($lamdaemonServers as $lamdaemonServer) {
				$server = $lamdaemonServer->getServer();
				// Get quotas
				$remote = new Remote();
				try {
					$remote->connect($lamdaemonServer);
				}
				catch (LAMException $e) {
					logNewMessage(LOG_ERR, $e->getTitle() . ' - ' . $e->getMessage());
					continue;
				}
				$quotas = $remote->execute(implode(self::$SPLIT_DELIMITER, ["+", "quota", "get", $this->get_scope()]));
				$remote->disconnect();
				$dirs = explode(":", $quotas);
				array_pop($dirs); // remove empty element at the end
				for ($i = 0; $i < count($dirs); $i++) {
					if (!str_starts_with($dirs[$i], self::$QUOTA_PREFIX)) {
						unset($dirs[$i]);
						$i--;
						continue;
					}
					$dirs[$i] = substr($dirs[$i], strlen(self::$QUOTA_PREFIX));
					$dirs[$i] = explode(",", $dirs[$i]);
					$dirs[$i] = $dirs[$i][0];
				}
				$dirs = array_values($dirs);
				for ($i = 0; $i < count($dirs); $i++) {
					$return[] = [
						'name' => 'quota_' . $server . ':' . $dirs[$i],
						'description' => sprintf(_('Quota for %s on %s'), $dirs[$i], $server),
						'help' => 'upload',
						'example' => '2000,2500,3000,3500'];
				}
			}
		}
		return $return;
	}

	/**
	 * {@inheritDoc}
	 */
	public function doUploadPostActions(array &$data, array $ids, array $failed, array &$temp, array &$accounts,
										array $selectedModules, ConfiguredType $type): array {
		$errors = [];
		// first call, get list of user names and quota values
		if (!isset($temp['counter'])) {
			$temp['counter'] = 0;
			// create list of quota columns
			$temp['quotas'] = [];
			$columns = array_keys($ids);
			for ($i = 0; $i < count($columns); $i++) {
				if (str_starts_with($columns[$i], 'quota_')) {
					$temp['quotas'][] = substr($columns[$i], 6);
				}
			}
			// select user/group name depending on current scope
			$temp['accounts'] = [];
			$col = 'invalid';
			if ($this->get_scope() == 'user') {
				$col = $ids['posixAccount_userName'];
			}
			elseif ($this->get_scope() == 'group') {
				$col = $ids['posixGroup_cn'];
			}
			// create list of account names and their quota values
			for ($i = 0; $i < count($data); $i++) {
				if (in_array($i, $failed)) {
					continue; // ignore failed accounts
				}
				$name = $data[$i][$col];
				for ($m = 0; $m < count($temp['quotas']); $m++) {
					if ($data[$i][$ids['quota_' . $temp['quotas'][$m]]] != '') {
						$parts = explode(',', $data[$i][$ids['quota_' . $temp['quotas'][$m]]]);
						// check syntax
						if (count($parts) !== 4) {
							$errMsg = $this->messages['upload'][0];
							$errMsg[] = [$i, 'quota_' . $temp['quotas'][$m]];
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[0], 'quotaNumber')) {
							$errMsg = $this->messages['softblock'][1];
							$errMsg[] = [$i, 'quota_' . $temp['quotas'][$m]];
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[1], 'quotaNumber')) {
							$errMsg = $this->messages['hardblock'][1];
							$errMsg[] = [$i, 'quota_' . $temp['quotas'][$m]];
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[2], 'quotaNumber')) {
							$errMsg = $this->messages['softinode'][1];
							$errMsg[] = [$i, 'quota_' . $temp['quotas'][$m]];
							$errors[] = $errMsg;
							continue;
						}
						if (!get_preg($parts[3], 'quotaNumber')) {
							$errMsg = $this->messages['hardinode'][1];
							$errMsg[] = [$i, 'quota_' . $temp['quotas'][$m]];
							$errors[] = $errMsg;
							continue;
						}
						if ($this->getQuotaNumber($parts[0]) > $this->getQuotaNumber($parts[1])) {
							$errMsg = $this->messages['block_cmp'][1];
							$errMsg[] = [$i, 'quota_' . $temp['quotas'][$m]];
							$errors[] = $errMsg;
							continue;
						}
						if ($this->getQuotaNumber($parts[2]) > $this->getQuotaNumber($parts[3])) {
							$errMsg = $this->messages['inode_cmp'][1];
							$errMsg[] = [$i, 'quota_' . $temp['quotas'][$m]];
							$errors[] = $errMsg;
							continue;
						}
						$parts[0] = $this->getQuotaNumber($parts[0]);
						$parts[1] = $this->getQuotaNumber($parts[1]);
						$parts[2] = $this->getQuotaNumber($parts[2]);
						$parts[3] = $this->getQuotaNumber($parts[3]);
						// save quota settings
						$temp['accounts'][$name][$temp['quotas'][$m]] = $parts;
					}
				}
			}
			return ['status' => 'inProgress', 'progress' => 5, 'errors' => $errors];
		}
		// quotas are ready to set
		elseif ($temp['counter'] < count($temp['accounts'])) {
			$names = array_keys($temp['accounts']);
			$name = $names[$temp['counter']];
			$mountPoints = array_keys($temp['accounts'][$name]);
			// set quota
			for ($m = 0; $m < count($mountPoints); $m++) {
				$mpParts = explode(":", $mountPoints[$m]);
				$server = $mpParts[0];
				$dir = $mpParts[1];
				$quotaString = implode(self::$SPLIT_DELIMITER, [$name, "quota", "set", $this->get_scope(), $dir . ',' .
					implode(',', $temp['accounts'][$name][$mountPoints[$m]]) . "\n"]);
				$remote = new Remote();
				$remoteServer = $_SESSION['config']->getScriptServerByName($server);
				try {
					$remote->connect($remoteServer);
				}
				catch (LAMException $e) {
					$errors[] = ['ERROR', $e->getTitle(), $e->getMessage()];
					continue;
				}
				$result = $remote->execute($quotaString);
				$remote->disconnect();
				if (!empty($result)) {
					$parts = explode(",", $result);
					if ($parts[0] === 'ERROR') {
						$errors[] = ['ERROR', $parts[1], $parts[2]];
					}
				}
			}
			// set counters to next account/mount point
			$temp['counter']++;
			return [
				'status' => 'inProgress',
				'progress' => 5 + (95 * ($temp['counter'] / count($temp['accounts']))),
				'errors' => $errors];
		}
		return ['status' => 'finished'];
	}

	/**
	 * {@inheritDoc}
	 */
	public function save_attributes(): array {
		// no LDAP changes
		return $this->getAccountContainer()->save_module_attributes([], []);
	}

	/**
	 * {@inheritDoc}
	 */
	public function getSelfServiceOptions(array $fields, array $attributes, bool $passwordChangeOnly, array $readOnlyFields): array {
		$return = [];
		if ($passwordChangeOnly) {
			return $return; // no fields as long no LDAP content can be read
		}
		if (in_array('quota', $fields) && !empty($attributes['uid'][0])) {
			$profile = $this->selfServiceSettings;
			$lamdaemonServers = LAMConfig::parseConfiguredScriptServers($profile->scriptServer, $profile->scriptPath,
				$profile->scriptUserName, $profile->scriptSSHKey, deobfuscateText($profile->scriptSSHKeyPassword));
			$this->initQuotas($attributes['uid'][0], $lamdaemonServers);
			if (is_array($this->quota)) {
				$serverDescriptions = [];
				foreach ($lamdaemonServers as $lamdaemonServer) {
					$serverDescriptions[$lamdaemonServer->getServer()] = $lamdaemonServer->getLabel();
				}
				$quotaRow = new htmlResponsiveRow();
				foreach ($this->quota as $server => $quotaDatas) {
					$title = $server;
					if ($serverDescriptions[$server] != $server) {
						$title = $serverDescriptions[$server] . " (" . $server . ")";
					}
					$quotaRow->add(new htmlSubTitle($title));

					$titles = [
						_('Mountpoint'), _('Used blocks'), _('Soft block limit'), _('Hard block limit'),
						_('Used inodes'), _('Soft inode limit'), _('Hard inode limit')
					];
					$data = [];
					// loop for every mointpoint with enabled quotas
					foreach ($quotaDatas as $quotaData) {
						$dataRow = [];
						$mountPoint = new htmlOutputText($quotaData[0]);
						$mountPoint->setNoWrap(true);
						$dataRow[] = $mountPoint;
						$dataRow[] = new htmlOutputText($quotaData[1]);
						$dataRow[] = new htmlOutputText($quotaData[2]);
						$dataRow[] = new htmlOutputText($quotaData[3]);
						$dataRow[] = new htmlOutputText($quotaData[5]);
						$dataRow[] = new htmlOutputText($quotaData[6]);
						$dataRow[] = new htmlOutputText($quotaData[7]);
						$data[] = $dataRow;
					}
					$table = new htmlResponsiveTable($titles, $data);
					$table->setCSSClasses(['responsive-table-wrap-th']);
					$table->setWidths(['20%', '5%', '10%', '10%', '5%', '5%', '10%', '10%', '5%']);
					$quotaRow->add($table);
				}
				$return['quota'] = $quotaRow;
			}
		}
		return $return;
	}

}
