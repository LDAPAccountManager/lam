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
 * Twilio SMS provider.
 *
 * @author Roland Gruber
 */

/**
 * Twilio SMS provider.
 *
 * @package LAM\PLUGINS\SMS
 */
class TwilioSms implements SmsProvider {

	/**
	 * @inheritDoc
	 */
	public function getLabel(): string {
		return 'Twilio';
	}

	/**
	 * @inheritDoc
	 */
	public function getId(): string {
		return 'twilio';
	}

	/**
	 * @inheritDoc
	 */
	public function sendSms(string $message, string $number, ?string $apiKey = '', ?string $apiToken = '',
							?string $accountId = '', ?string $region = '', ?string $from = '', ?string $subject = ''): void {
		$curl = curl_init();
		$postData = [
			"To" => $number,
			"Body" => $message,
		];
		if (preg_match('/^[0-9+]+/', $from)) {
			$postData['From'] = $from;
		}
		else {
			$postData['MessagingServiceSid'] = $from;
		}
		curl_setopt_array($curl, [
			CURLOPT_URL => 'https://api.twilio.com/2010-04-01/Accounts/' . urlencode($accountId) . '/Messages.json',
			CURLOPT_RETURNTRANSFER => true,
			CURLOPT_POST => true,
			CURLOPT_POSTFIELDS => $postData,
			CURLOPT_USERNAME => $accountId,
			CURLOPT_PASSWORD => $apiToken,
		]);
		$response = curl_exec($curl);
		$httpCode = curl_getinfo($curl, CURLINFO_HTTP_CODE);
		logNewMessage(LOG_DEBUG, 'SMS gateway response: ' . $response);
		$responseJson = json_decode($response, true);
		if ($httpCode !== 201) {
			logNewMessage(LOG_ERR, 'Unable to send SMS: ' . $response);
			$message = $responseJson['message'] ?? $responseJson['error_message'] ?? '';
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
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function usesFrom(): bool {
		return true;
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
