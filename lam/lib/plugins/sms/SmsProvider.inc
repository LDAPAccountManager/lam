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

/**
 * SMS providers.
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
	 * Includes all plugin files.
	 */
	private function includeFiles(): void {
		$pluginDir = dir(__DIR__);
		while ($entry = $pluginDir->read()) {
			if ((str_starts_with($entry, '.')) || ($entry === basename(__FILE__))) {
				continue;
			}
			include_once(__DIR__ . '/' . $entry);
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
	 * Returns the connection string for the SMS service.
	 *
	 * @param string|null $apiKey API key
	 * @param string|null $apiToken API token
	 * @return string connection string
	 */
	public function getConnectionString(?string $apiKey = '', ?string $apiToken = ''): string;

}

