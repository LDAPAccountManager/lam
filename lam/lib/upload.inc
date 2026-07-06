<?php

namespace LAM\UPLOAD;

use LAM\TYPES\ConfiguredType;
use LAMException;
use LamTemporaryFilesManager;
use ZipArchive;
use accountContainer;
use function LAM\PDF\createModulePDF;

/*

 This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
 Copyright (C) 2016 - 2025  Roland Gruber

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
 * Creates LDAP accounts for file upload.
 *
 * @author Roland Gruber
 * @package tools
 */

/** LDAP handle */
include_once(__DIR__ . '/ldap.inc');
/** PDF */
include_once(__DIR__ . '/pdf.inc');

/**
 * Creates LDAP accounts for file upload.
 *
 * @author Roland Gruber
 * @package tools
 */
class Uploader {

	/**
	 * @var array<string, string|string[]>[] $accounts LDAP accounts
	 */
	private array $accounts;
	/**
	 * @var array<int, string>[] $data data
	 */
	private array $data;
	private ConfiguredType $type;
	private int $endTime;

	private const TIME_LIMIT = 10;

	/**
	 * Cleans old session data.
	 */
	public static function cleanSession(): void {
		foreach (array_keys($_SESSION) as $key) {
			if (str_starts_with($key, 'mass_')) {
				unset($_SESSION[$key]);
			}
		}
	}

	/**
	 * Constructor
	 *
	 * @param ConfiguredType $type account type
	 */
	public function __construct(ConfiguredType $type) {
		$this->accounts = json_decode(lamDecrypt($_SESSION['mass_accounts']), true);
		$this->data = json_decode(lamDecrypt($_SESSION['mass_data']), true);
		$this->type = $type;
		$startTime = time();
		$maxTime = get_cfg_var('max_execution_time') - 5;
		if ($maxTime > self::TIME_LIMIT) {
			$maxTime = self::TIME_LIMIT;
		}
		if ($maxTime <= 0) {
			$maxTime = self::TIME_LIMIT;
		}
		$this->endTime = $startTime + $maxTime;
	}

	/**
	 * Start or continues the file upload.
	 *
	 * @return string JSON of upload status
	 */
	public function doUpload(): string {
		$this->securityCheck();
		if (!isset($_SESSION['mass_counter'])) {
			return $this->startUpload();
		}
		elseif ($_SESSION['mass_counter'] < count($this->accounts)) {
			return $this->continueUpload();
		}
		elseif (!isset($_SESSION['mass_postActions']['finished'])) {
			return $this->runPostActions();
		}
		elseif (($_SESSION['mass_pdf']['structure'] != null) && !isset($_SESSION['mass_pdf']['finished'])) {
			return $this->createPDF();
		}
		return $this->buildUploadStatus(100, true, '', 100, true, 100);
	}

	/**
	 * Continues to upload accounts.
	 *
	 * @return string JSON of upload status
	 */
	private function continueUpload(): string {
		while (($_SESSION['mass_counter'] < count($this->accounts)) && ($this->endTime > time())) {
			$this->uploadEntry($_SESSION['mass_counter']);
			$_SESSION['mass_counter']++;
		}
		$accountsProgress = round(($_SESSION['mass_counter'] * 100) / count($this->accounts));
		$accountsFinished = ($_SESSION['mass_counter'] == count($this->accounts));
		return $this->buildUploadStatus($accountsProgress, $accountsFinished);
	}

	/**
	 * Performs the upload of a single LDAP entry.
	 *
	 * @param int $position position in $this->accounts.
	 */
	private function uploadEntry($position): void {
		$attrs = $this->accounts[$position];
		$dn = $attrs['dn'];
		unset($attrs['dn']);
		// remove informational attributes
		foreach ($attrs as $key => $value) {
			if (str_starts_with($key, 'INFO.')) {
				unset($attrs[$key]);
			}
		}
		// run preactions
		$preAttributes = [];
		foreach ($attrs as $key => $value) {
			$preAttributes[$key] = &$attrs[$key];
		}
		$preAttributes['dn'] = &$dn;
		$preMessages = doUploadPreActions($this->type, $_SESSION['mass_selectedModules'], $preAttributes);
		$preActionOk = true;
		for ($i = 0; $i < count($preMessages); $i++) {
			if (($preMessages[$i][0] == 'ERROR') || ($preMessages[$i][0] == 'WARN')) {
				$preActionOk = false;
				$_SESSION['mass_errors'][] = $preMessages[$i];
			}
		}
		if ($preActionOk) {
			$errors = $this->deleteOldEntryIfNeeded($dn, $this->accounts[$position]);
			if (!empty($errors)) {
				$_SESSION['mass_errors'] = array_merge($_SESSION['mass_errors'], $errors);
				$_SESSION['mass_failed'][] = $position;
				return;
			}
			// add LDAP entry
			$success = ldapAddNewEntry($_SESSION['ldap']->server(), $dn, $attrs);
			if (!$success) {
				$errorMessage = [
					"ERROR",
					_("LAM was unable to create account %s! An LDAP error occurred."),
					getDefaultLDAPErrorString($_SESSION['ldap']->server()),
					[$position]];
				$_SESSION['mass_errors'][] = $errorMessage;
				$_SESSION['mass_failed'][] = $position;
			}
		}
	}

	/**
	 * Deletes an existing entry if the overwrite option is used.
	 *
	 * @param string $dn DN
	 * @param array<string, string|string[]> $attributes attribute list
	 * @return string[][] list of error messages
	 */
	private function deleteOldEntryIfNeeded($dn, $attributes): array {
		if (isset($attributes['INFO.overwrite']) && ($attributes['INFO.overwrite'] === 'true')) {
			$attrs = ldapGetDN($dn, ['dn']);
			if ($attrs !== null) {
				return deleteDN($dn, true);
			}
		}
		return [];
	}

	/**
	 * Starts the file upload.
	 *
	 * @return string JSON of upload status
	 */
	private function startUpload(): string {
		$_SESSION['mass_counter'] = 0;
		return $this->buildUploadStatus();
	}

	/**
	 * Returns the upload status as JSON.
	 *
	 * @param number $accountsProgress progress for LDAP entries
	 * @param bool $accountsFinished all entries in LDAP
	 * @param string $postActionsTitle title for current post action
	 * @param number $postActionsProgress progress for post actions
	 * @param bool $postActionsFinished post actions finished
	 * @param number $pdfProgress PDF creation progress
	 * @return string JSON status
	 */
	private function buildUploadStatus($accountsProgress = 0, $accountsFinished = false,
									   $postActionsTitle = '', $postActionsProgress = 0, $postActionsFinished = false,
									   $pdfProgress = 0): string {
		$pdfFinished = ($_SESSION['mass_pdf']['structure'] == null) || isset($_SESSION['mass_pdf']['finished']);
		$allDone = $accountsFinished && $postActionsFinished && $pdfFinished;
		$errorHtml = '';
		if ($allDone && !empty($_SESSION['mass_errors'])) {
			foreach ($_SESSION['mass_errors'] as $error) {
				$text = $error[2] ?? '';
				$vars = $error[3] ?? [];
				$errorHtml .= StatusMessage($error[0], $error[1], $text, $vars, true);
			}
		}
		$status = [
			'title' => _("LDAP upload in progress. Please wait."),
			'titleFinished' => _("Upload has finished"),
			'titleErrors' => _("There were errors while uploading:"),
			'titlePDF' => _('Create PDF files'),
			'accountsProgress' => $accountsProgress,
			'accountsFinished' => $accountsFinished,
			'postActionsTitle' => $postActionsTitle,
			'postActionsProgress' => $postActionsProgress,
			'postActionsFinished' => $postActionsFinished,
			'pdfProgress' => $pdfProgress,
			'pdfFinished' => $pdfFinished,
			'allDone' => $allDone,
			'errorHtml' => $errorHtml,
			'typeId' => $this->type->getId()
		];
		return json_encode($status);
	}

	/**
	 * Checks for security violations and stops processing if needed.
	 */
	private function securityCheck(): void {
		if (!isLoggedIn()
			|| $this->type->isHidden()
			|| !checkIfNewEntriesAreAllowed($this->type->getId())
			|| !checkIfWriteAccessIsAllowed($this->type->getId())) {
			die;
		}
	}

	/**
	 * Performs any post-create actions by modules.
	 *
	 * @return string JSON of upload status
	 */
	private function runPostActions(): string {
		$return = $this->runModulePostActions();
		while (!isset($_SESSION['mass_postActions']['finished']) && ($this->endTime > time())) {
			$return = $this->runModulePostActions();
		}
		$title = _("Additional tasks for module:") . ' ' . getModuleAlias($return['module'], $this->type->getScope());
		$progress = round($return['progress']);
		$finished = isset($_SESSION['mass_postActions']['finished']);
		return $this->buildUploadStatus(100, true, $title, $progress, $finished);
	}

	/**
	 * Runs a single post create action by modules.
	 *
	 * @return array<string, string|int|string[]> status array
	 */
	private function runModulePostActions(): array {
		$return = doUploadPostActions($this->type, $this->data, $_SESSION['mass_ids'], $_SESSION['mass_failed'], $_SESSION['mass_selectedModules'], $this->accounts);
		if ($return['status'] == 'finished') {
			$_SESSION['mass_postActions']['finished'] = true;
		}
		if (isset($return['errors'])) {
			for ($i = 0; $i < count($return['errors']); $i++) {
				$_SESSION['mass_errors'][] = $return['errors'][$i];
			}
		}
		return $return;
	}

	/**
	 * Creates the PDF files.
	 *
	 * @return string JSON of upload status
	 * @throws LAMException error during PDF creation
	 */
	private function createPDF(): string {
		$file = $_SESSION['mass_pdf']['file'];
		$pdfStructure = $_SESSION['mass_pdf']['structure'];
		$font = $_SESSION['mass_pdf']['font'];
		$pdfZip = new ZipArchive();
		$tempFilesManager = new LamTemporaryFilesManager();
		if ($_SESSION['mass_pdf']['counter'] == 0) {
			$pdfZipResult = @$pdfZip->open($tempFilesManager->getFileSystemPath($file), ZipArchive::OVERWRITE);
			if (!$pdfZipResult) {
				$_SESSION['mass_errors'][] = ['ERROR', _('Unable to create ZIP file for PDF export.')];
				$_SESSION['mass_pdf']['finished'] = true;
			}
		}
		else {
			@$pdfZip->open($tempFilesManager->getFileSystemPath($file));
		}
		while (!isset($_SESSION['mass_pdf']['finished']) && ($this->endTime > time())) {
			$attrs = $this->accounts[$_SESSION['mass_pdf']['counter']];
			$dn = $attrs['dn'];
			// get informational attributes
			$infoAttributes = [];
			foreach ($attrs as $key => $value) {
				if (str_starts_with($key, 'INFO.')) {
					$infoAttributes[$key] = $value;
				}
			}
			// load account
			$_SESSION['mass_pdfAccount'] = new accountContainer($this->type, 'mass_pdfAccount');
			$pdfErrors = $_SESSION['mass_pdfAccount']->load_account($dn, $infoAttributes);
			if (count($pdfErrors) > 0) {
				$_SESSION['mass_errors'] = array_merge($_SESSION['mass_errors'], $pdfErrors);
				$_SESSION['mass_pdf']['finished'] = true;
				break;
			}
			// create and save PDF
			$pdfContent = createModulePDF([$_SESSION['mass_pdfAccount']], $pdfStructure, $font, true);
			$fileName = $dn . '.pdf';
			$pdfZip->addFromString($fileName, $pdfContent);
			$_SESSION['mass_pdf']['counter']++;
			if ($_SESSION['mass_pdf']['counter'] >= count($this->accounts)) {
				$_SESSION['mass_pdf']['finished'] = true;
			}
		}
		@$pdfZip->close();
		$progress = round(($_SESSION['mass_pdf']['counter'] * 100) / count($this->accounts));
		return $this->buildUploadStatus(100, true, '', 100, true, $progress);
	}

}
