<?php
namespace Duo;

/*
 * https://duo.com/docs/duoweb
 */

class Web
{
    const DUO_PREFIX = "TX";
    const APP_PREFIX = "APP";
    const AUTH_PREFIX = "AUTH";

    const DUO_EXPIRE = 300;
    const APP_EXPIRE = 3600;
    const INIT_EXPIRE = 300;

    const IKEY_LEN = 20;
    const SKEY_LEN = 40;
    const AKEY_LEN = 40; // if this changes you have to change ERR_AKEY

    const ERR_USER = 'ERR|The username specified is invalid.';
    const ERR_IKEY = 'ERR|The Duo integration key specified is invalid.';
    const ERR_SKEY = 'ERR|The Duo secret key specified is invalid.';
    const ERR_AKEY = 'ERR|The application secret key specified must be at least 40 characters.';

    const LIBRARY_NAME = 'duo_php';
    const VERSION = '1.0.0';

    private static function signVals($key, $vals, $prefix, $expire, $time = null, $algo = "sha1")
    {
        $exp = ($time ? $time : time()) + $expire;
        $val = $vals . '|' . $exp;
        $b64 = base64_encode($val);
        $cookie = $prefix . '|' . $b64;

        $sig = hash_hmac($algo, $cookie, $key);
        return $cookie . '|' . $sig;
    }

    private static function parseVals($key, $val, $prefix, $ikey, $time = null, $algo = "sha1")
    {
        $ts = ($time ? $time : time());

        $parts = explode('|', $val);
        if (count($parts) !== 3) {
            return null;
        }
        list($u_prefix, $u_b64, $u_sig) = $parts;

        $sig = hash_hmac($algo, $u_prefix . '|' . $u_b64, $key);
        if (hash_hmac($algo, $sig, $key) !== hash_hmac($algo, $u_sig, $key)) {
            return null;
        }

        if ($u_prefix !== $prefix) {
            return null;
        }

        $cookie_parts = explode('|', base64_decode($u_b64));
        if (count($cookie_parts) !== 3) {
            return null;
        }
        list($user, $u_ikey, $exp) = $cookie_parts;

        if ($u_ikey !== $ikey) {
            return null;
        }
        if ($ts >= intval($exp)) {
            return null;
        }

        return $user;
    }

    public static function signRequest($ikey, $skey, $akey, $username, $time = null)
    {
        if (!isset($username) || strlen($username) === 0) {
            return self::ERR_USER;
        }
        if (strpos($username, '|') !== false) {
            return self::ERR_USER;
        }
        if (!isset($ikey) || strlen($ikey) !== self::IKEY_LEN) {
            return self::ERR_IKEY;
        }
        if (!isset($skey) || strlen($skey) !== self::SKEY_LEN) {
            return self::ERR_SKEY;
        }
        if (!isset($akey) || strlen($akey) < self::AKEY_LEN) {
            return self::ERR_AKEY;
        }

        $vals = $username . '|' . $ikey;

        $duo_sig = self::signVals($skey, $vals, self::DUO_PREFIX, self::DUO_EXPIRE, $time);
        $app_sig = self::signVals($akey, $vals, self::APP_PREFIX, self::APP_EXPIRE, $time);

        return $duo_sig . ':' . $app_sig;
    }

    public static function verifyResponse($ikey, $skey, $akey, $sig_response, $time = null)
    {
        list($auth_sig, $app_sig) = explode(':', $sig_response);

        $auth_user = self::parseVals($skey, $auth_sig, self::AUTH_PREFIX, $ikey, $time);
        $app_user = self::parseVals($akey, $app_sig, self::APP_PREFIX, $ikey, $time);

        if ($auth_user !== $app_user) {
            return null;
        }

        return $auth_user;
    }

    public static function initAuth($client, $ikey, $akey, $username, $enroll_only = false)
    {
        if (!isset($username) || strlen($username) === 0) {
            return self::ERR_USER;
        }
        if (strpos($username, '|') !== false) {
            return self::ERR_USER;
        }
        if (!isset($ikey) || strlen($ikey) !== self::IKEY_LEN) {
            return self::ERR_IKEY;
        }
        if (!isset($akey) || strlen($akey) < self::AKEY_LEN) {
            return self::ERR_AKEY;
        }

        $blob = $username . '|' . $ikey;
        $signed_blob = self::signVals(
            $akey,
            $blob,
            self::APP_PREFIX,
            self::APP_EXPIRE,
            null,
            'sha512'
        );
        $expire = time() + self::INIT_EXPIRE;
        $client_version = self::LIBRARY_NAME . '/' . self::VERSION;

        $response = $client->init(
            $username,
            $signed_blob,
            $expire,
            $client_version,
            $enroll_only
        );

        return $response['response']['response']['txid'];
    }

    public static function verifyAuth($client, $ikey, $akey, $response_txid)
    {
        $response = $client->auth_response($response_txid);

        $username = $response['response']['response']['uname'];
        $signed_blob = $response['response']['response']['app_blob'];

        $parsed_user = self::parseVals(
            $akey,
            $signed_blob,
            self::APP_PREFIX,
            $ikey,
            null,
            'sha512'
        );

        if ($username !== $parsed_user) {
            return null;
        }

        return $username;
    }
}
