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

class Jwt
{
    public function __construct(
        string $jwt,
        array $claims
    )
    {
        $this->jwt = $jwt;
        $this->claims = $claims;
    }

    public function getJwt()
    {
        return $this->jwt;
    }

    public function getClaims()
    {
        return $this->claims;
    }

    public function getExpirationTime($carbonInstance = true)
    {
        $ts = $this->toJson()->exp;
        if(class_exists(\Carbon\Carbon::class) && $carbonInstance) {
            return \Carbon\Carbon::createFromTimestampUTC($ts);
        }

        return $ts;
    }

    public function getIssuedAt($carbonInstance = true)
    {
        $ts = $this->toJson()->iat;
        if(class_exists(\Carbon\Carbon::class) && $carbonInstance) {
            return \Carbon\Carbon::createFromTimestampUTC($ts);
        }

        return $ts;
    }

    public function toJson()
    {
        if(is_resource($this->claims)) {
            throw new \InvalidArgumentException('Could not convert to JSON');
        }

        return json_decode(json_encode($this->claims));

    }

}