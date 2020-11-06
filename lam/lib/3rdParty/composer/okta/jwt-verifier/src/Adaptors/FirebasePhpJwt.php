<?php
/******************************************************************************
 * Copyright 2017 Okta, Inc.                                                  *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *      http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 ******************************************************************************/

namespace Okta\JwtVerifier\Adaptors;

use Firebase\JWT\JWT as FirebaseJWT;
use Okta\JwtVerifier\Jwt;
use Okta\JwtVerifier\Request;
use UnexpectedValueException;

class FirebasePhpJwt implements Adaptor
{
    /**
     * @var Request
     */
    private $request;

    /**
     * Leeway in seconds
     *
     * @var int
     */
    private $leeway;

    public function __construct(Request $request = null, int $leeway = 120)
    {
        $this->request = $request ?: new Request();
        $this->leeway = $leeway;
    }

    public function getKeys($jku)
    {
        $keys = json_decode($this->request->setUrl($jku)->get()->getBody()->getContents());
        return self::parseKeySet($keys);
    }

    public function decode($jwt, $keys): Jwt
    {
        FirebaseJWT::$leeway = $this->leeway;
        $decoded = (array)FirebaseJWT::decode($jwt, $keys, ['RS256']);
        return (new Jwt($jwt, $decoded));
    }

    public static function isPackageAvailable()
    {
        return class_exists(FirebaseJWT::class);
    }

    /**
     * Parse a set of JWK keys
     * @param $source
     * @return array an associative array represents the set of keys
     */
    public static function parseKeySet($source)
    {
        $keys = [];
        if (is_string($source)) {
            $source = json_decode($source, true);
        } else if (is_object($source)) {
            if (property_exists($source, 'keys'))
                $source = (array)$source;
            else
                $source = [$source];
        }
        if (is_array($source)) {
            if (isset($source['keys']))
                $source = $source['keys'];

            foreach ($source as $k => $v) {
                if (!is_string($k)) {
                    if (is_array($v) && isset($v['kid']))
                        $k = $v['kid'];
                    elseif (is_object($v) && property_exists($v, 'kid'))
                        $k = $v->{'kid'};
                }
                try {
                    $v = self::parseKey($v);
                    $keys[$k] = $v;
                } catch (UnexpectedValueException $e) {
                    //Do nothing
                }
            }
        }
        if (0 < count($keys)) {
            return $keys;
        }
        throw new UnexpectedValueException('Failed to parse JWK');
    }

    /**
     * Parse a JWK key
     * @param $source
     * @return resource|array an associative array represents the key
     */
    public static function parseKey($source)
    {
        if (!is_array($source))
            $source = (array)$source;
        if (!empty($source) && isset($source['kty']) && isset($source['n']) && isset($source['e'])) {
            switch ($source['kty']) {
                case 'RSA':
                    if (array_key_exists('d', $source))
                        throw new UnexpectedValueException('Failed to parse JWK: RSA private key is not supported');

                    $pem = self::createPemFromModulusAndExponent($source['n'], $source['e']);
                    $pKey = openssl_pkey_get_public($pem);
                    if ($pKey !== false)
                        return $pKey;
                    break;
                default:
                    //Currently only RSA is supported
                    break;
            }
        }

        throw new UnexpectedValueException('Failed to parse JWK');
    }

    /**
     *
     * Create a public key represented in PEM format from RSA modulus and exponent information
     *
     * @param string $n the RSA modulus encoded in Base64
     * @param string $e the RSA exponent encoded in Base64
     * @return string the RSA public key represented in PEM format
     */
    private static function createPemFromModulusAndExponent($n, $e)
    {
        $modulus = FirebaseJWT::urlsafeB64Decode($n);
        $publicExponent = FirebaseJWT::urlsafeB64Decode($e);


        $components = array(
            'modulus' => pack('Ca*a*', 2, self::encodeLength(strlen($modulus)), $modulus),
            'publicExponent' => pack('Ca*a*', 2, self::encodeLength(strlen($publicExponent)), $publicExponent)
        );

        $RSAPublicKey = pack(
            'Ca*a*a*',
            48,
            self::encodeLength(strlen($components['modulus']) + strlen($components['publicExponent'])),
            $components['modulus'],
            $components['publicExponent']
        );


        // sequence(oid(1.2.840.113549.1.1.1), null)) = rsaEncryption.
        $rsaOID = pack('H*', '300d06092a864886f70d0101010500'); // hex version of MA0GCSqGSIb3DQEBAQUA
        $RSAPublicKey = chr(0) . $RSAPublicKey;
        $RSAPublicKey = chr(3) . self::encodeLength(strlen($RSAPublicKey)) . $RSAPublicKey;

        $RSAPublicKey = pack(
            'Ca*a*',
            48,
            self::encodeLength(strlen($rsaOID . $RSAPublicKey)),
            $rsaOID . $RSAPublicKey
        );

        $RSAPublicKey = "-----BEGIN PUBLIC KEY-----\r\n" .
            chunk_split(base64_encode($RSAPublicKey), 64) .
            '-----END PUBLIC KEY-----';

        return $RSAPublicKey;
    }

    /**
     * DER-encode the length
     *
     * DER supports lengths up to (2**8)**127, however, we'll only support lengths up to (2**8)**4.  See
     * {@link http://itu.int/ITU-T/studygroups/com17/languages/X.690-0207.pdf#p=13 X.690 paragraph 8.1.3} for more information.
     *
     * @access private
     * @param int $length
     * @return string
     */
    private static function encodeLength($length)
    {
        if ($length <= 0x7F) {
            return chr($length);
        }

        $temp = ltrim(pack('N', $length), chr(0));
        return pack('Ca*', 0x80 | strlen($temp), $temp);
    }
}
