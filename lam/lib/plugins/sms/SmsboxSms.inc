<?php

namespace LAM\PLUGINS\SMS;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2025 - 2026  Roland Gruber

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
 * SMSBOX SMS provider.
 *
 * @author Roland Gruber
 */

/**
 * SMSBOX SMS provider.
 *
 * @package LAM\PLUGINS\SMS
 */
class SmsboxSms implements SmsProvider {

	/**
	 * @inheritDoc
	 */
	public function getLabel(): string {
		return 'SMSBOX';
	}

	/**
	 * @inheritDoc
	 */
	public function getId(): string {
		return 'smsbox';
	}

	/**
	 * @inheritDoc
	 */
	public function sendSms(string $message, string $number, ?string $apiKey = '', ?string $apiToken = '',
							?string $accountId = '', ?string $region = '', ?string $from = '', ?string $subject = ''): void {
		$curl = curl_init();
		$postData = [
			"dest" => $number,
			"msg" => $message,
			'charset' => 'utf-8',
			'mode' => 'Standard',
			'strategy' => '2'
		];
		curl_setopt_array($curl, [
			CURLOPT_URL => 'https://api.smsbox.pro/1.1/api.php',
			CURLOPT_RETURNTRANSFER => true,
			CURLOPT_POST => true,
			CURLOPT_POSTFIELDS => http_build_query($postData),
			CURLOPT_HTTPHEADER => [
				'Content-Type: application/x-www-form-urlencoded',
				'Authorization: App ' . $apiKey,
			],
		]);
		$response = curl_exec($curl);
		$httpCode = curl_getinfo($curl, CURLINFO_HTTP_CODE);
		logNewMessage(LOG_DEBUG, 'SMS gateway response: ' . $response);
		if (($httpCode !== 200) || ($response !== 'OK')) {
			logNewMessage(LOG_ERR, 'Unable to send SMS: ' . $response);
			throw new LAMException(null, '');
		}
	}

	/**
	 * @inheritDoc
	 */
	public function usesApiToken(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function usesApiKey(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function usesAccountId(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function usesFrom(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function usesRegion(): bool {
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function usesSubject(): bool {
		return false;
	}

}
