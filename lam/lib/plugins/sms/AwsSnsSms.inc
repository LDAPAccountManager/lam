<?php

namespace LAM\PLUGINS\SMS;

use Aws\Exception\AwsException;
use Aws\Sns\SnsClient;
use LAMException;

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

/**
 * AWS SNS SMS provider.
 *
 * @author Roland Gruber
 */

/**
 * AWS SNS SMS provider.
 *
 * @package LAM\PLUGINS\SMS
 */
class AwsSnsSms implements SmsProvider {

	/**
	 * @inheritDoc
	 */
	public function getLabel(): string {
		return 'AWS SNS';
	}

	/**
	 * @inheritDoc
	 */
	public function getId(): string {
		return 'aws-sns';
	}

	/**
	 * @inheritDoc
	 */
	public function sendSms(string $message, string $number, ?string $apiKey = '', ?string $apiToken = '',
							?string $accountId = '', ?string $region = '', ?string $from = '', ?string $subject = ''): void {
		$options = [
			'Message' => $message,
			'PhoneNumber' => $number,
		];
		include_once __DIR__ . '/../../3rdParty/composer/autoload.php';
		$clientOptions = [
			'version' => '2010-03-31',
			'region' => $region,
			'credentials' => [
				'key' => $accountId,
				'secret' => $apiToken,
			],
		];
		$snsClient = new SnsClient($clientOptions);
		try {
			$result = $snsClient->publish($options);
			logNewMessage(LOG_DEBUG, 'SMS sending response: ' . print_r($result, true));
		}
		catch (AwsException $e) {
			logNewMessage(LOG_ERR, 'Unable to send SMS to ' . $number . ': ' . $e->getMessage());
			throw new LAMException(null, $e->getMessage(), $e);
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
		return false;
	}

	/**
	 * @inheritDoc
	 */
	public function usesRegion(): bool {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	public function usesSubject(): bool {
		return false;
	}

}
