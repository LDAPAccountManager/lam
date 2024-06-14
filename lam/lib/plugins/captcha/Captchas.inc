<?php

namespace LAM\PLUGINS\CAPTCHA;

use htmlElement;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2022 - 2024  Roland Gruber

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
 * Captcha providers.
 *
 * @author Roland Gruber
 */

/**
 * Provides captcha services.
 *
 * @package LAM\PLUGINS\CAPTCHA
 */
class Captchas {

	public const NO_CAPTCHA = '-';

	/**
	 * Includes all plugin files.
	 */
	protected function includeFiles() {
		$pluginDir = dir(__DIR__);
		while ($entry = $pluginDir->read()) {
			if ((str_starts_with($entry, '.')) || ($entry === basename(__FILE__))) {
				continue;
			}
			include_once(__DIR__ . '/' . $entry);
		}
	}

	/**
	 * Returns a list of CaptchaProvider objects.
	 *
	 * @return CaptchaProvider[] providers (id => provider object)
	 */
	public function findProviders(): array {
		$this->includeFiles();
		$providers = [];
		foreach (get_declared_classes() as $declaredClass) {
			if (in_array('LAM\PLUGINS\CAPTCHA\CaptchaProvider', class_implements($declaredClass))) {
				$provider = new $declaredClass();
				$providers[$provider->getId()] = $provider;
			}
		}
		return $providers;
	}

}

/**
 * Interface for providers of captchas.
 *
 * @package LAM\PLUGINS\CAPTCHAS
 */
interface CaptchaProvider {

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
	 * Returns the HTML element to render for the captcha.
	 *
	 * @param string $siteKey site key
	 * @return htmlElement HTML element
	 */
	public function getCaptchaElement(string $siteKey): htmlElement;

	/**
	 * Checks if the POST data contains a valid captcha result.
	 *
	 * @param string $secretKey secret key
	 * @param string $siteKey site key
	 * @return bool user solved captcha
	 */
	public function isValid(string $secretKey, string $siteKey): bool;

}

