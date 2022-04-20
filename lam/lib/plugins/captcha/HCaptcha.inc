<?php
namespace LAM\PLUGINS\CAPTCHA;
use htmlElement;
use htmlScript;

/*

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  Copyright (C) 2022  Roland Gruber

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
 * hCaptcha provider.
 *
 * @author Roland Gruber
 */

/**
 * hCaptcha provider.
 *
 * @package LAM\PLUGINS\CAPTCHA
 */
class HCaptcha implements CaptchaProvider {

	/**
	 * @inheritDoc
	 */
	public function getLabel(): string {
		return 'hCaptcha';
	}

	/**
	 * @inheritDoc
	 */
	public function getId(): string {
		return 'hCaptcha';
	}

	/**
	 * @inheritDoc
	 */
	public function getCaptchaElement(string $siteKey): htmlElement {
		return new htmlHCaptcha($siteKey);
	}

	/**
	 * @inheritDoc
	 */
	public function isValid(string $secretKey, string $siteKey): bool {
		$url = 'https://hcaptcha.com/siteverify';
		$vars = array(
			'secret' => $secretKey,
			'response' => $_POST['h-captcha-response'],
			'sitekey' => $siteKey);
		$options = array(
			'http' => array(
				'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
				'method'  => 'POST',
				'content' => http_build_query($vars)
			)
		);
		$context = stream_context_create($options);
		$result = file_get_contents($url, false, $context);
		if ($result === FALSE) {
			logNewMessage(LOG_ERR, 'hCaptcha validation failed, invalid server response.');
			return false;
		}
		$responseJSON = json_decode($result);
		logNewMessage(LOG_DEBUG, "hCaptcha result: " . $result);
		return $responseJSON->{'success'} === true;
	}

}

/**
 * Creates a hCaptcha element.
 *
 * @package LAM\PLUGINS\CAPTCHA
 */
class htmlHCaptcha extends htmlElement {

	/** site key */
	private $key = null;

	/**
	 * Constructor.
	 *
	 * @param String $key site key
	 */
	function __construct($key) {
		$this->key = htmlspecialchars($key);
	}

	/**
	 * Prints the HTML code for this element.
	 *
	 * @param string $module Name of account module
	 * @param array $input List of meta-HTML elements
	 * @param array $values List of values which override the defaults in $input (name => value)
	 * @param boolean $restricted If true then no buttons will be displayed
	 * @param integer $tabindex Start value of tabulator index for input fields
	 * @param string $scope Account type
	 * @return array List of input field names and their type (name => type)
	 */
	function generateHTML($module, $input, $values, $restricted, &$tabindex, $scope) {
		$script = new htmlScript('https://js.hcaptcha.com/1/api.js');
		$script->generateHTML($module, $input, $values, $restricted, $tabindex, $scope);
		echo '<div class="h-captcha" data-sitekey="' . $this->key . '"></div>';
		return array();
	}

}
