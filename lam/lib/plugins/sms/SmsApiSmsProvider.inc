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
 * SMSAPI SMS provider.
 *
 * @author Roland Gruber
 */

/**
 * SMSAPI SMS provider.
 *
 * @package LAM\PLUGINS\SMS
 */
class SmsApiSms implements SmsProvider {

	/**
	 * @inheritDoc
	 */
	public function getLabel(): string {
		return 'SMSAPI';
	}

	/**
	 * @inheritDoc
	 */
	public function getId(): string {
		return 'smsapi';
	}

	/**
	 * @inheritDoc
	 */
	public function sendSms(string $message, string $number, ?string $apiKey = '', ?string $apiToken = '',
							?string $accountId = '', ?string $region = '', ?string $from = '', ?string $subject = ''): void {
		$curl = curl_init();
		$postData = [
			"to" => $number,
			"format" => "json",
			"message" => $message,
		];
		$postJson = json_encode($postData);
		curl_setopt_array($curl, [
			CURLOPT_URL => 'https://api.smsapi.com/sms.do',
			CURLOPT_RETURNTRANSFER => true,
			CURLOPT_POST => true,
			CURLOPT_POSTFIELDS => $postJson,
			CURLOPT_HTTPHEADER => [
				'Content-Type: application/json',
				'Accept: application/json',
				'Authorization: Bearer ' . $apiToken,
			],
		]);
		$response = curl_exec($curl);
		$httpCode = curl_getinfo($curl, CURLINFO_HTTP_CODE);
		logNewMessage(LOG_DEBUG, 'SMS gateway response: ' . $response);
		$responseJson = json_decode($response, true);
		if (($httpCode !== 200) || isset($responseJson['error'])) {
			logNewMessage(LOG_ERR, 'Unable to send SMS: ' . $response);
			$message = $responseJson['message'] ?? $responseJson['list'][0]['error'] ?? '';
			throw new LAMException(null, $message);
		}
	}

	/**
	 * @inheritDoc
	 */
	public function usesApiToken(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function usesApiKey(): bool {
		return false;
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
