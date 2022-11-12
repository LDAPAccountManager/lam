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

use Okta\JwtVerifier\Discovery\Oauth;
use Okta\JwtVerifier\Adaptors\Adaptor;
use Okta\JwtVerifier\Discovery\DiscoveryMethod;
use Bretterer\IsoDurationConverter\DurationParser;

class JwtVerifierBuilder
{
    protected $issuer;
    protected $discovery;
    protected $request;
    protected $adaptor;
    protected $audience;
    protected $clientId;
    protected $nonce;
    protected $leeway = 120;

    public function __construct(Request $request = null)
    {
        $this->request = $request;
    }

    /**
     * Sets the issuer URI.
     *
     * @param string $issuer The issuer URI
     * @return JwtVerifierBuilder
     */
    public function setIssuer(string $issuer): self
    {
        $this->issuer = rtrim($issuer, "/");

        return $this;
    }

    /**
     * Set the Discovery class. This class should be an instance of DiscoveryMethod.
     *
     * @param DiscoveryMethod $discoveryMethod The DiscoveryMethod instance.
     * @return JwtVerifierBuilder
     */
    public function setDiscovery(DiscoveryMethod $discoveryMethod): self
    {
        $this->discovery = $discoveryMethod;

        return $this;
    }

    /**
     * Set the Adaptor class. This class should be an interface of Adaptor.
     *
     * @param Adaptor $adaptor The adaptor of the JWT library you are using.
     * @return JwtVerifierBuilder
     */
    public function setAdaptor(Adaptor $adaptor): self
    {
        $this->adaptor = $adaptor;

        return $this;
    }

    public function setAudience($audience)
    {
        $this->audience = $audience;

        return $this;
    }

    public function setClientId($clientId)
    {
        $this->clientId = $clientId;

        return $this;
    }

    public function setNonce($nonce)
    {
        $this->nonce = $nonce;

        return $this;
    }

    /**
     * Set the leeway using ISO_8601 Duration string. ie: PT2M
     *
     * @param string $leeway ISO_8601 Duration format. Default: PT2M
     * @return self
     * @throws \InvalidArgumentException
     */
    public function setLeeway(string $leeway = "PT2M"): self
    {
        if (strstr($leeway, "P")) {
            $msg = "It appears that the leeway provided is not in ISO_8601 Duration Format.";
            $msg .= "Please privide a duration in the format of `PT(n)S`";
            throw new \InvalidArgumentException($msg);
        }

        $leeway = (new DurationParser)->parse($leeway);
        $this->leeway = $leeway;

        return $this;
    }

    /**
     * Build and return the JwtVerifier.
     *
     * @throws \InvalidArgumentException
     * @return JwtVerifier
     */
    public function build(): JwtVerifier
    {
        $this->validateIssuer($this->issuer);

        $this->validateClientId($this->clientId);

        return new JwtVerifier(
            $this->issuer,
            $this->discovery,
            $this->adaptor,
            $this->request,
            $this->leeway,
            [
                'nonce' => $this->nonce,
                'audience' => $this->audience,
                'clientId' => $this->clientId
            ]
        );
    }

    /**
     * Validate the issuer
     *
     * @param string $issuer
     * @throws \InvalidArgumentException
     * @return void
     */
    private function validateIssuer($issuer): void
    {
        if (null === $issuer || "" == $issuer) {
            $msg = "Your Issuer is missing. ";
            $msg .= "You can find your issuer from your authorization server settings in the Okta Developer Console. ";
            $msg .= "Find out more information aobut Authorization Servers at ";
            $msg .= "https://developer.okta.com/docs/guides/customize-authz-server/overview/";
            throw new \InvalidArgumentException($msg);
        }

        if (strstr($issuer, "https://") == false) {
            $msg = "Your Issuer must start with https. Current value: {$issuer}. ";
            $msg .= "You can copy your issuer from your authorization server settings in the Okta Developer Console. ";
            $msg .= "Find out more information aobut Authorization Servers at ";
            $msg .= "https://developer.okta.com/docs/guides/customize-authz-server/overview/";
            throw new \InvalidArgumentException($msg);
        }

        if (strstr($issuer, "{yourOktaDomain}") != false) {
            $msg = "Replace {yourOktaDomain} with your Okta domain. ";
            $msg .= "You can copy your domain from the Okta Developer Console. Follow these instructions to find it: ";
            $msg .= "https://bit.ly/finding-okta-domain";
            throw new \InvalidArgumentException($msg);
        }
    }

    /**
     * Validate the client id
     *
     * @param string $cid
     * @throws \InvalidArgumentException
     * @return void
     */
    private function validateClientId($cid): void
    {
        if (null === $cid || "" == $cid) {
            $msg = "Your client ID is missing. You can copy it from the Okta Developer Console in the details for the ";
            $msg .= "Application you created. Follow these instructions to find it: ";
            $msg .= "https://bit.ly/finding-okta-app-credentials";
            throw new \InvalidArgumentException($msg);
        }

        if (strstr($cid, "{clientId}") != false) {
            $msg = "Replace {clientId} with the client ID of your Application. You can copy it from the Okta Developer";
            $msg .= "Console in the details for the Application you created. Follow these instructions to find it: ";
            $msg .= "https://bit.ly/finding-okta-app-credentials";
            throw new \InvalidArgumentException($msg);
        }
    }
}
