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
 * @package LAM\PLUGINS\CAPTCHA
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
		curl_setopt_array($curl, [
			CURLOPT_URL => 'https://api.sweego.io/send',
			CURLOPT_RETURNTRANSFER => true,
			CURLOPT_ENCODING => '',
			CURLOPT_MAXREDIRS => 10,
			CURLOPT_TIMEOUT => 0,
			CURLOPT_FOLLOWLOCATION => true,
			CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
			CURLOPT_CUSTOMREQUEST => 'POST',
			CURLOPT_POSTFIELDS => '{
			  "campaign_type": "transac",
			  "channel": "sms",
			  "message_txt": "' . $message . '",
			  "provider": "sweego",
			  "recipients": [
				{
				  "num": "' . $number . '",
				  "region": "DE"
				}
			  ]
			}',
			CURLOPT_HTTPHEADER => [
				'Content-Type: application/json',
				'Accept: application/json',
				'Api-Key: ' . $apiKey,
			],
		]);
		$response = curl_exec($curl);
		$httpCode = curl_getinfo($curl, CURLINFO_HTTP_CODE);
		curl_close($curl);
		if ($httpCode !== 200) {
			$response = json_decode($response, true);
			throw new LAMException(null, $response['detail']);
		}
		echo $response;
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
