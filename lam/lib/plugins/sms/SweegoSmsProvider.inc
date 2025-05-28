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
 * Sweego SMS provider.
 *
 * @author Roland Gruber
 */

/**
 * Sweego SMS provider.
 *
 * @package LAM\PLUGINS\SMS
 */
class SweegoSms implements SmsProvider {

	/**
	 * @inheritDoc
	 */
	public function getLabel(): string {
		return 'Sweego';
	}

	/**
	 * @inheritDoc
	 */
	public function getId(): string {
		return 'sweego';
	}

	/**
	 * @inheritDoc
	 */
	public function sendSms(string $message, string $number, ?string $apiKey = '', ?string $apiToken = ''): void {
		$curl = curl_init();
		$postData = [
			"campaign_type" => "transac",
			"channel" => "sms",
			"message_txt" => $message,
			"provider" => "sweego",
			"recipients" => [
				(object) [
					"num" => $number,
					"region" => "DE" // TODO
				]
			]
		];
		$postJson = json_encode($postData);
		curl_setopt_array($curl, [
			CURLOPT_URL => 'https://api.sweego.io/send',
			CURLOPT_RETURNTRANSFER => true,
			CURLOPT_ENCODING => '',
			CURLOPT_MAXREDIRS => 10,
			CURLOPT_TIMEOUT => 0,
			CURLOPT_FOLLOWLOCATION => true,
			CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
			CURLOPT_CUSTOMREQUEST => 'POST',
			CURLOPT_POSTFIELDS => $postJson,
			CURLOPT_HTTPHEADER => [
				'Content-Type: application/json',
				'Accept: application/json',
				'Api-Key: ' . $apiKey,
			],
		]);
		logNewMessage(LOG_DEBUG, 'SMS message for: ' . $number);
		$response = curl_exec($curl);
		$httpCode = curl_getinfo($curl, CURLINFO_HTTP_CODE);
		curl_close($curl);
		logNewMessage(LOG_DEBUG, 'SMS gateway response: ' . $response);
		if ($httpCode !== 200) {
			logNewMessage(LOG_ERR, 'Unable to send SMS: ' . $response);
			$response = json_decode($response, true);
			$message = $response['detail'] ?? '';
			throw new LAMException(null, $message);
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

}
