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

use Okta\JwtVerifier\JwtVerifier;
use PHPUnit\Framework\TestCase;

class JwtVerifierTest extends BaseTestCase
{
    /** @test */
    public function can_get_issuer_off_object()
    {
        $this->response
            ->method('getBody')
            ->willreturn('{"issuer": "https://example.com"}');


        $httpClient = new \Http\Mock\Client;
        $httpClient->addResponse($this->response);
        $request = new \Okta\JwtVerifier\Request($httpClient);

        $verifier = new JwtVerifier(
            'https://my.issuer.com',
            new \Okta\JwtVerifier\Discovery\Oauth(),
            new \Okta\JwtVerifier\Adaptors\FirebasePhpJwt(),
            $request
        );

        $this->assertEquals(
            'https://my.issuer.com',
            $verifier->getIssuer(),
            'Does not return issuer correctly'
        );
    }

    /** @test */
    public function can_get_discovery_off_object()
    {
        $this->response
            ->method('getBody')
            ->willreturn('{"issuer": "https://example.com"}');


        $httpClient = new \Http\Mock\Client;
        $httpClient->addResponse($this->response);
        $request = new \Okta\JwtVerifier\Request($httpClient);

        $verifier = new JwtVerifier(
            'https://my.issuer.com',
            new \Okta\JwtVerifier\Discovery\Oauth(),
            new \Okta\JwtVerifier\Adaptors\FirebasePhpJwt(),
            $request
        );

        $this->assertInstanceOf(
            \Okta\JwtVerifier\Discovery\Oauth::class,
            $verifier->getDiscovery(),
            'Does not return discovery correctly'
        );
    }

    /** @test */
    public function will_get_meta_data_when_verifier_is_constructed()
    {
        $this->response
            ->method('getBody')
            ->willreturn('{"issuer": "https://example.com"}');


        $httpClient = new \Http\Mock\Client;
        $httpClient->addResponse($this->response);
        $request = new \Okta\JwtVerifier\Request($httpClient);

        $verifier = new JwtVerifier(
            'https://my.issuer.com',
            new \Okta\JwtVerifier\Discovery\Oauth(),
            new \Okta\JwtVerifier\Adaptors\FirebasePhpJwt(),
            $request
        );

        $metaData = $verifier->getMetaData();

        $this->assertEquals(
            'https://example.com',
            $metaData->issuer,
            'Metadata was not accessed.'
        );

    }


}
