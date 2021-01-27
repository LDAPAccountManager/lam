<?php
namespace LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2020 - 2021  Roland Gruber

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
 * Extra messages for invalid credentials.
 *
 * @author Roland Gruber
 */

/**
 * Provides additional messages if login failed.
 *
 * @package LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS
 */
class ExtraInvalidCredentials {

	/**
	 * Tries to get additional information why invalid credentials was returned. E.g. account is locked.
	 *
	 * @param handle $ldap LDAP object to connect for getting extra data
	 * @param string $userDn failed DN
	 * @return null|string extra message
	 */
	public function getExtraMessage($ldap, string $userDn) {
		$this->includeFiles();
		$providers = $this->findProviders();
		$attributes = $this->getAttributeNames($providers);
		$userData = $this->getLdapData($userDn, $attributes, $ldap);
		return $this->getMessageFromProviders($providers, $userData, $ldap);
	}

	/**
	 * Includes all plugin files.
	 */
	protected function includeFiles() {
		$pluginDir = dir(__DIR__);
		while ($entry = $pluginDir->read()) {
			if ((strpos($entry, '.') === 0) || ($entry === basename(__FILE__))) {
				continue;
			}
			include_once(__DIR__ . '/' . $entry);
		}
	}

	/**
	 * Returns a list of ExtraInvalidCredentialsProvider objects.
	 *
	 * @return ExtraInvalidCredentialsProvider[] providers
	 */
	protected function findProviders() {
		$providers = array();
		foreach (get_declared_classes() as $declaredClass) {
			if (in_array('LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS\ExtraInvalidCredentialsProvider', class_implements($declaredClass))) {
				$providers[] = new $declaredClass();
			}
		}
		return $providers;
	}

	/**
	 * Returns the attribute names for the LDAP search.
	 *
	 * @param ExtraInvalidCredentialsProvider[] $providers
	 */
	protected function getAttributeNames(array $providers) {
		$attributeNames = array();
		foreach ($providers as $provider) {
			$attributeNames = array_merge($attributeNames, $provider->getAttributeNamesToRead());
		}
		$attributeNames = array_unique($attributeNames);
		return $attributeNames;
	}

	/**
	 * Reads the LDAP data.
	 *
	 * @param string $userDn user DN
	 * @param array $attributes attribute names
	 * @param handle $ldap LDAP handle
	 * @return array|null attribute values
	 */
	protected function getLdapData(string $userDn, array $attributes, $ldap) {
		$data = ldapGetDN($userDn, $attributes, $ldap);
		if ($data === null) {
			$data = array();
		}
		return $data;
	}

	/**
	 * Returns the message from the providers.
	 *
	 * @param ExtraInvalidCredentialsProvider[] $providers providers
	 * @param array $userData LDAP data
	 * @param handle $ldap LDAP handle
	 * @return null|string extra message
	 */
	protected function getMessageFromProviders(array $providers, array $userData, $ldap) {
		foreach ($providers as $provider) {
			$message = $provider->getExtraMessage($userData, $ldap);
			if ($message !== null) {
				return $message;
			}
		}
		return null;
	}

}

/**
 * Interface for provides of extra messages.
 *
 * @package LAM\PLUGINS\EXTRA_INVALID_CREDENTIALS
 */
interface ExtraInvalidCredentialsProvider {

	/**
	 * Returns the list of attribute names to read from LDAP.
	 *
	 * @return string[] attribute names
	 */
	public function getAttributeNamesToRead(): array;

	/**
	 * Returns an extra message if any.
	 *
	 * @param array $attributes LDAP attributes
	 * @param handle $ldap LDAP handle
	 * @return null|string message
	 */
	public function getExtraMessage(array $attributes, $ldap);

}

