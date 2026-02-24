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
 * Email2SMS provider.
 *
 * @author Roland Gruber
 */

/**
 * Email2SMS provider.
 *
 * @package LAM\PLUGINS\SMS
 */
class Email2SmsProvider implements SmsProvider {

	/**
	 * @inheritDoc
	 */
	public function getLabel(): string {
		return 'EMail2SMS';
	}

	/**
	 * @inheritDoc
	 */
	public function getId(): string {
		return 'email2sms';
	}

	/**
	 * @inheritDoc
	 */
	public function sendSms(string $message, string $number, ?string $apiKey = '', ?string $apiToken = '',
							?string $accountId = '', ?string $region = '', ?string $from = '', ?string $subject = ''): void {
		if (str_starts_with($number, '+')) {
			$number = '00' . substr($number, 1);
		}
		$to = str_replace('$number', $number, $accountId);
		$success = sendEMail($to, $subject ?? '', $message, $from, false);
		if ($success) {
			logNewMessage(LOG_DEBUG, 'Email2Sms sent to ' . $to);
		}
		else {
			logNewMessage(LOG_ERR, 'Unable to send SMS via Email2SMS for ' . $to);
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
		return true;
	}

}
