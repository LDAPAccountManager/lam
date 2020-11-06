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

use Okta\JwtVerifier\JwtVerifierBuilder;

class JwtVerifierBuilderTest extends BaseTestCase
{
    /** @test */
    public function when_setting_issuer_self_is_returned()
    {
        $verifier = new JwtVerifierBuilder();
        $this->assertInstanceOf(
            JwtVerifierBuilder::class,
            $verifier->setIssuer('https://my.issuer.com'),
            'Setting the issuer does not return self.'
        );
    }

    /** @test */
    public function when_setting_discovery_self_is_returned()
    {
        $verifier = new JwtVerifierBuilder();
        $this->assertInstanceOf(
            JwtVerifierBuilder::class,
            $verifier->setDiscovery(new \Okta\JwtVerifier\Discovery\Oauth()),
            'Settings discovery does not return self.'
        );
    }

    /** @test */
    public function building_the_jwt_verifier_throws_exception_if_issuer_not_set()
    {
        $this->expectException(\InvalidArgumentException::class);
        $verifier = new JwtVerifierBuilder();
        $verifier->build();
    }

    /** @test */
    public function discovery_defaults_to_oauth_when_building()
    {
        $this->response
            ->method('getBody')
            ->willreturn('{"issuer": "https://example.com"}');


        $httpClient = new \Http\Mock\Client;
        $httpClient->addResponse($this->response);
        $request = new \Okta\JwtVerifier\Request($httpClient);

        $verifier = new JwtVerifierBuilder($request);
        $verifier = $verifier->setIssuer('https://my.issuer.com')->setClientId("abc123")
            ->setAdaptor(new \Okta\JwtVerifier\Adaptors\FirebasePhpJwt())->build();

        $this->assertInstanceOf(
            \Okta\JwtVerifier\Discovery\Oauth::class,
            $verifier->getDiscovery(),
            'The builder is not defaulting to oauth2 discovery'
        );
    }

    /** @test */
    public function building_the_verifier_returns_instance_of_jwt_verifier()
    {
        $this->response
            ->method('getBody')
            ->willreturn('{"issuer": "https://example.com"}');


        $httpClient = new \Http\Mock\Client;
        $httpClient->addResponse($this->response);
        $request = new \Okta\JwtVerifier\Request($httpClient);

        $verifier = new JwtVerifierBuilder($request);
        $verifier = $verifier->setIssuer('https://my.issuer.com')->setClientId("abc123")
            ->setAdaptor(new \Okta\JwtVerifier\Adaptors\FirebasePhpJwt())->build();

        $this->assertInstanceOf(
            \Okta\JwtVerifier\JwtVerifier::class,
            $verifier,
            'The verifier builder is not returning an instance of JwtVerifier'
        );
    }


}
