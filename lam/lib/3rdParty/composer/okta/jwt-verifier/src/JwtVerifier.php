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

namespace Okta\JwtVerifier;

use Http\Client\Common\PluginClient;
use Http\Client\HttpClient;
use Http\Discovery\HttpClientDiscovery;
use Http\Discovery\MessageFactoryDiscovery;
use Http\Discovery\UriFactoryDiscovery;
use Okta\JwtVerifier\Adaptors\Adaptor;
use Okta\JwtVerifier\Adaptors\AutoDiscover;
use Okta\JwtVerifier\Discovery\DiscoveryMethod;
use Okta\JwtVerifier\Discovery\Oauth;

class JwtVerifier
{
    /**
     * @var string
     */
    protected $issuer;

    /**
     * @var DiscoveryMethod
     */
    protected $discovery;

    /**
     * @var array
     */
    protected $claimsToValidate;

    /**
     * @var string
     */
    protected $wellknown;

    /**
     * @var mixed
     */
    protected $metaData;

    /**
     * @var Adaptor
     */
    protected $adaptor;

    public function __construct(
        string $issuer,
        DiscoveryMethod $discovery = null,
        Adaptor $adaptor = null,
        Request $request = null,
        int $leeway = 120,
        array $claimsToValidate = []
    ) {
        $this->issuer = $issuer;
        $this->discovery = $discovery ?: new Oauth;
        $this->adaptor = $adaptor ?: AutoDiscover::getAdaptor();
        $request = $request ?: new Request;
        $this->wellknown = $this->issuer.$this->discovery->getWellKnown();
        $this->metaData = json_decode($request->setUrl($this->wellknown)->get()
            ->getBody());

        $this->claimsToValidate = $claimsToValidate;
    }

    public function getIssuer()
    {
        return $this->issuer;
    }

    public function getDiscovery()
    {
        return $this->discovery;
    }

    public function getMetaData()
    {
        return $this->metaData;
    }

    public function verify($jwt)
    {
        if($this->metaData->jwks_uri == null) {
            throw new \DomainException("Could not access a valid JWKS_URI from the metadata.  We made a call to {$this->wellknown} endpoint, but jwks_uri was null. Please make sure you are using a custom authorization server for the jwt verifier.");
        }

        $keys = $this->adaptor->getKeys($this->metaData->jwks_uri);

        $decoded =  $this->adaptor->decode($jwt, $keys);

        $this->validateClaims($decoded->getClaims(), "access"); // This is hard coded to access token since this was the original functionality.

        return $decoded;
    }

    public function verifyIdToken($jwt)
    {

        if($this->metaData->jwks_uri == null) {
            throw new \DomainException("Could not access a valid JWKS_URI from the metadata.  We made a call to {$this->wellknown} endpoint, but jwks_uri was null. Please make sure you are using a custom authorization server for the jwt verifier.");
        }

        $keys = $this->adaptor->getKeys($this->metaData->jwks_uri);

        $decoded =  $this->adaptor->decode($jwt, $keys);

        $this->validateClaims($decoded->getClaims(), "id");

        return $decoded;
    }

    public function verifyAccessToken($jwt)
    {
        if($this->metaData->jwks_uri == null) {
            throw new \DomainException("Could not access a valid JWKS_URI from the metadata.  We made a call to {$this->wellknown} endpoint, but jwks_uri was null. Please make sure you are using a custom authorization server for the jwt verifier.");
        }

        $keys = $this->adaptor->getKeys($this->metaData->jwks_uri);

        $decoded =  $this->adaptor->decode($jwt, $keys);

        $this->validateClaims($decoded->getClaims(), "access");

        return $decoded;
    }

    private function validateClaims(array $claims, string $type)
    {
        switch($type) {
            case 'id':
                $this->validateAudience($claims);
                $this->validateNonce($claims);
                break;
            case 'access':
                $this->validateAudience($claims);
                $this->validateClientId($claims);
                $this->validateNonce($claims);
                break;
        }
    }

    private function validateNonce($claims)
    {
        if(!isset($claims['nonce']) && $this->claimsToValidate['nonce'] == null) {
            return false;
        }

        if($claims['nonce'] != $this->claimsToValidate['nonce']) {
            throw new \Exception('Nonce does not match what is expected. Make sure to provide the nonce with
            `setNonce()` from the JwtVerifierBuilder.');
        }
    }

    private function validateAudience($claims)
    {
        if(!isset($claims['aud']) && $this->claimsToValidate['audience'] == null) {
            return false;
        }

        if($claims['aud'] != $this->claimsToValidate['audience']) {
            throw new \Exception('Audience does not match what is expected. Make sure to provide the audience with
            `setAudience()` from the JwtVerifierBuilder.');
        }
    }

    private function validateClientId($claims)
    {
        if(!isset($claims['cid']) && $this->claimsToValidate['clientId'] == null) {
            return false;
        }

        if($claims['cid'] != $this->claimsToValidate['clientId']) {
            throw new \Exception('ClientId does not match what is expected. Make sure to provide the client id with
            `setClientId()` from the JwtVerifierBuilder.');
        }
    }
}
