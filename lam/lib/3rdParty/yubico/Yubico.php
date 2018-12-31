<?php
/**
 * Class for verifying Yubico One-Time-Passcodes
 *
 * @category Auth
 * @package Auth_Yubico
 * @author Simon Josefsson <simon@yubico.com>, Olov Danielson <olov@yubico.com>
 * @author Roland Gruber
 * @copyright 2007-2015 Yubico AB
 * @copyright 2018 Roland Gruber
 * @license https://opensource.org/licenses/bsd-license.php New BSD License
 * @version 2.0
 * @link https://www.yubico.com/
 *
 * Adapted for LAM.
 */

/**
 * Class for verifying Yubico One-Time-Passcodes
 */
class Auth_Yubico {

	/**
	 * Yubico client ID
	 *
	 * @var string
	 */
	private $clientId;

	/**
	 * Yubico client key
	 *
	 * @var string
	 */
	private $clientKey;

	/**
	 * URL part of validation server
	 *
	 * @var string
	 */
	private $url;

	/**
	 * Flag whether to verify HTTPS server certificates or not.
	 *
	 * @var boolean
	 */
	private $httpsVerify;

	/**
	 * Constructor
	 *
	 * Sets up the object
	 *
	 * @param string $id The client identity
	 * @param string $key The client MAC key
	 * @param string $url URL
	 * @param boolean $httpsverify Flag whether to use verify HTTPS
	 *        server certificates
	 */
	public function __construct($id, $key, $url, $httpsverify) {
		$this->clientId = $id;
		$this->clientKey = base64_decode($key);
		$this->httpsVerify = $httpsverify;
		$this->url = $url;
	}

	/**
	 * Parse input string into password, yubikey prefix,
	 * ciphertext, and OTP.
	 *
	 * @param string Input string to parse
	 * @param string Optional delimiter re-class, default is '[:]'
	 * @return array Keyed array with fields
	 */
	private function parsePasswordOTP($str, $delim = '[:]') {
		if (!preg_match("/^((.*)" . $delim . ")?(([cbdefghijklnrtuv]{0,12})([cbdefghijklnrtuv]{32}))$/i", $str, $matches)) {
			/* Dvorak? */
			if (!preg_match("/^((.*)" . $delim . ")?(([jxe\\.uidchtnbpygk]{0,12})([jxe\\.uidchtnbpygk]{32}))$/i", $str, $matches)) {
				return false;
			}
			else {
				$ret['otp'] = strtr($matches[3], "jxe.uidchtnbpygk", "cbdefghijklnrtuv");
			}
		}
		else {
			$ret['otp'] = $matches[3];
		}
		$ret['password'] = $matches[2];
		$ret['prefix'] = $matches[4];
		$ret['ciphertext'] = $matches[5];
		return $ret;
	}

	/**
	 * Verify Yubico OTP against multiple URLs
	 * Protocol specification 2.0 is used to construct validation requests
	 *
	 * @param string $token Yubico OTP
	 * @param int $use_timestamp 1=>send request with &timestamp=1 to
	 *        get timestamp and session information
	 *        in the response
	 * @throws LAMException if verification failed
	 */
	public function verify($token, $use_timestamp = null) {
		$timeout = 10;
		/* Construct parameters string */
		$ret = $this->parsePasswordOTP($token);
		if (!$ret) {
			throw new LAMException(_('Error'), 'Could not parse Yubikey OTP');
		}
		$params = array(
			'id' => $this->clientId,
			'otp' => $ret['otp'],
			'nonce' => md5(uniqid(getRandomNumber()))
		);
		/* Take care of protocol version 2 parameters */
		if ($use_timestamp) {
			$params['timestamp'] = 1;
		}
		$params['timeout'] = $timeout;
		ksort($params);
		$parameters = '';
		foreach ($params as $p => $v) {
			$parameters .= "&" . $p . "=" . $v;
		}
		$parameters = ltrim($parameters, "&");

		/* Generate signature. */
		if ($this->clientKey != "") {
			$signature = base64_encode(hash_hmac('sha1', $parameters, $this->clientKey, true));
			$signature = preg_replace('/\+/', '%2B', $signature);
			$parameters .= '&h=' . $signature;
		}

		$query = $this->url . "?" . $parameters;

		logNewMessage(LOG_DEBUG, 'Yubico url: ' . $query);

		$handle = curl_init($query);
		curl_setopt($handle, CURLOPT_USERAGENT, "LAM Auth Yubico");
		curl_setopt($handle, CURLOPT_RETURNTRANSFER, 1);
		if (!$this->httpsVerify) {
			curl_setopt($handle, CURLOPT_SSL_VERIFYPEER, 0);
			curl_setopt($handle, CURLOPT_SSL_VERIFYHOST, 0);
		}
		curl_setopt($handle, CURLOPT_FAILONERROR, true);
		curl_setopt($handle, CURLOPT_TIMEOUT, $timeout);

		/* Execute and read request. */
		$this->response = null;
		$str = curl_exec($handle);
		$httpCode = curl_getinfo($handle, CURLINFO_HTTP_CODE);
		curl_close($handle);
		logNewMessage(LOG_DEBUG, 'Server answer: ' . $str);
		if (is_string($str) && ($httpCode == 200) && preg_match("/status=([a-zA-Z0-9_]+)/", $str, $out)) {
			$status = $out[1];

			/*
			 * There are 3 cases.
			 *
			 * 1. OTP or Nonce values doesn't match - ignore
			 * response.
			 *
			 * 2. We have a HMAC key. If signature is invalid -
			 * ignore response. Return if status=OK/REPLAYED_OTP/BAD_OTP.
			 *
			 * 3. Return if status=OK or status=REPLAYED_OTP.
			 */
			if (!preg_match("/otp=" . $params['otp'] . "/", $str) || !preg_match("/nonce=" . $params['nonce'] . "/", $str)) {
				if ($status == 'BAD_OTP') {
					throw new LAMException(_('Error'), 'OTP not accepted. Maybe key is not registered.');
				}
				throw new LAMException(_('Error'), 'Invalid answer ' . $str);
			}
			elseif ($this->clientKey != "") {
				/* Case 2. Verify signature first */
				$rows = explode("\r\n", trim($str));
				$response = array();
				foreach ($rows as $val) {
					/*
					 * '=' is also used in BASE64 encoding so we only replace the first = by # which is not
					 * used in BASE64
					 */
					$val = preg_replace('/=/', '#', $val, 1);
					$row = explode("#", $val);
					$response[$row[0]] = $row[1];
				}

				$parameters = array(
					'nonce',
					'otp',
					'sessioncounter',
					'sessionuse',
					'sl',
					'status',
					't',
					'timeout',
					'timestamp'
				);
				sort($parameters);
				$check = Null;
				foreach ($parameters as $param) {
					if (array_key_exists($param, $response)) {
						if ($check) {
							$check = $check . '&';
						}
						$check = $check . $param . '=' . $response[$param];
					}
				}

				$checksignature = base64_encode(hash_hmac('sha1', utf8_encode($check), $this->clientKey, true));

				if ($response['h'] == $checksignature) {
					$this->checkStatus($status);
					return;
				}
				else {
					throw new LAMException(_('Error'), 'Invalid signature, expected ' . $checksignature);
				}
			}
			else {
				/* Case 3. We check the status directly */
				$this->checkStatus($status);
				return;
			}
		}
		throw new LAMException(_('Error'), 'Call to verification service failed with ' . $httpCode);
	}

	/**
	 * Checks if the status is ok.
	 *
	 * @param string $status status
	 * @throws LAMException invalid status
	 */
	private function checkStatus($status) {
		if ($status == 'REPLAYED_OTP') {
			throw new LAMException(_('Error'), 'OTP replay detected.');
		}
		elseif ($status == 'BAD_OTP') {
			throw new LAMException(_('Error'), 'OTP not accepted. Maybe key is not registered.');
		}
		elseif ($status == 'OK') {
			return;
		}
		throw new LAMException(_('Error'), 'Invalid status: ' . $status);
	}

}
?>
