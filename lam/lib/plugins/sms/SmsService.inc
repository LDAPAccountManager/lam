<?php

namespace LAM\PLUGINS\SMS;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025  Roland Gruber

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

use LAMException;

/**
 * SMS service.
 *
 * @author Roland Gruber
 */

/**
 * Provides SMS services.
 *
 * @package LAM\PLUGINS\SMS
 */
class SmsService {

	/**
	 * Sender name for SMS display.
	 */
	public const SENDER_NAME = 'LAM Pro';

	/**
	 * Includes all plugin files.
	 */
	private function includeFiles(): void {
		$pluginDir = dir(__DIR__);
		while ($entry = $pluginDir->read()) {
			if ((str_starts_with($entry, '.')) || ($entry === basename(__FILE__))) {
				continue;
			}
			include_once __DIR__ . '/' . $entry;
		}
	}

	/**
	 * Returns a list of SmsProvider objects.
	 *
	 * @return SmsProvider[] providers (id => provider object)
	 */
	public function findProviders(): array {
		$this->includeFiles();
		$providers = [];
		foreach (get_declared_classes() as $declaredClass) {
			if (in_array('LAM\PLUGINS\SMS\SmsProvider', class_implements($declaredClass))) {
				$provider = new $declaredClass();
				$providers[$provider->getId()] = $provider;
			}
		}
		return $providers;
	}

	/**
	 * Sends out an SMS.
	 *
	 * @param string $message message text
	 * @param string $number mobile phone number
	 * @param string|null $providerId provider ID
	 * @param string|null $apiKey API key
	 * @param string|null $apiToken API token
	 * @param string|null $accountId account ID
	 * @param string|null $region region
	 * @param string|null $from from
	 * @param string|null $subject subject text
	 * @throws LAMException error sending SMS
	 */
	public function sendSms(string $message, string $number, ?string $providerId = null, ?string $apiKey = null,
							?string $apiToken = null, ?string $accountId = null, ?string $region = null,
							?string $from = null, ?string $subject = null): void {
		if ($providerId === null) {
			$providerId = $_SESSION['cfgMain']->smsProvider;
			$apiKey = $_SESSION['cfgMain']->smsApiKey;
			$apiToken = $_SESSION['cfgMain']->smsToken;
			$accountId = $_SESSION['cfgMain']->smsAccountId;
			$region = $_SESSION['cfgMain']->smsRegion;
			$from = $_SESSION['cfgMain']->smsFrom;
			$subject = $_SESSION['cfgMain']->smsSubject;
		}
		$providers = $this->findProviders();
		if (!isset($providers[$providerId])) {
			logNewMessage(LOG_ERR, 'SMS provider not found: ' . $providerId);
			throw new LAMException('Provider not found');
		}
		$provider = $providers[$providerId];
		$number = $this->cleanNumber($number);
		$number = $this->addCountryPrefixIfMissing($number);
		logNewMessage(LOG_DEBUG, 'SMS message for: ' . $number);
		$provider->sendSms($message, $number, $apiKey, $apiToken, $accountId, $region, $from, $subject);
	}

	/**
	 * Cleans any extra characters from the telephone number.
	 *
	 * @param string $number number
	 * @return string cleaned value
	 */
	private function cleanNumber(string $number): string {
		return preg_replace('/[^0-9+]+/', '', $number);
	}

	/**
	 * Adds the default country prefix to the telephone number if needed.
	 *
	 * @param string $number number
	 * @return string prefixed value
	 */
	private function addCountryPrefixIfMissing(string $number) {
		$prefix = $_SESSION['cfgMain']->smsDefaultCountryPrefix;
		if (empty($prefix) || str_starts_with($number, '+') || str_starts_with($number, '00')) {
			return $number;
		}
		if (str_starts_with($number, '0')) {
			$number = substr($number, 1);
		}
		return $prefix . $number;
	}

}

/**
 * Interface for providers of SMS services.
 *
 * @package LAM\PLUGINS\SMS
 */
interface SmsProvider {

	/**
	 * Returns the label of the service
	 *
	 * @return string label
	 */
	public function getLabel(): string;

	/**
	 * Returns the id of the service
	 *
	 * @return string id
	 */
	public function getId(): string;

	/**
	 * Returns if the provider requires an account ID.
	 *
	 * @return bool account id required
	 */
	public function usesAccountId(): bool;

	/**
	 * Returns if the provider requires a token.
	 *
	 * @return bool token required
	 */
	public function usesApiToken(): bool;

	/**
	 * Returns if the provider requires an API key.
	 *
	 * @return bool key required
	 */
	public function usesApiKey(): bool;

	/**
	 * Returns if the provider requires a FROM field.
	 *
	 * @return bool key required
	 */
	public function usesFrom(): bool;

	/**
	 * Returns if the provider requires a region field.
	 *
	 * @return bool region required
	 */
	public function usesRegion(): bool;

	/**
	 * Service uses a subject field.
	 *
	 * @return bool use subject
	 */
	public function usesSubject(): bool;

	/**
	 * Sends out an SMS.
	 *
	 * @param string $message message text
	 * @param string $number mobile phone number
	 * @param string|null $apiKey API key
	 * @param string|null $apiToken API token
	 * @param string|null $accountId account ID
	 * @param string|null $from from
	 * @param string|null $subject subject text
	 * @throws LAMException error sending SMS
	 */
	public function sendSms(string $message, string $number, ?string $apiKey = '',
							?string $apiToken = '', ?string $accountId = '',
							?string $region = '', ?string $from = '', ?string $subject = ''): void;

}

