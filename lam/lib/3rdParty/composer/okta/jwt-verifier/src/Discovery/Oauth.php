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

namespace Okta\JwtVerifier\Discovery;

use Okta\JwtVerifier\Discovery\DiscoveryMethod as Discovery;

class Oauth extends Discovery
{

    protected $wellKnownUri = '/.well-known/oauth-authorization-server';

    /**
     * Get the defined well-known URI.  This is the URI
     * that is concatenated to the issuer URL.
     *
     * @return string
     */
    public function getWellKnownUri(): string
    {
        return $this->wellKnownUri;
    }

}