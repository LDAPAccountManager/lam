# Facile JOSE Verifier

A library to validate JWT tokens.

[![Build Status](https://github.com/facile-it/php-jose-verifier/workflows/Continuous%20Integration/badge.svg?branch=master)](https://github.com/facile-it/php-jose-verifier/actions)
[![codecov](https://codecov.io/gh/facile-it/php-jose-verifier/branch/master/graph/badge.svg?token=1RHS0NWD2L)](https://codecov.io/gh/facile-it/php-jose-verifier)
[![Latest Stable Version](https://poser.pugx.org/facile-it/php-jose-verifier/v/stable)](https://packagist.org/packages/facile-it/php-jose-verifier)
[![Total Downloads](https://poser.pugx.org/facile-it/php-jose-verifier/downloads)](https://packagist.org/packages/facile-it/php-jose-verifier)
[![Latest Unstable Version](https://poser.pugx.org/facile-it/php-jose-verifier/v/unstable)](https://packagist.org/packages/facile-it/php-jose-verifier)
[![License](https://poser.pugx.org/facile-it/php-jose-verifier/license)](https://packagist.org/packages/facile-it/php-jose-verifier)

## How To Use

The suggested and simply way to use it (specially for OAuth2 and OpenID tokens) is using builders.

For better performance you should install `ext-gmp`.

## Create verifiers from Issuer and Client Metadata

Usually an OpenID provider provides an openid-configuration (`/.well-known/openid-configuration`).

You can fetch the configuration and use it with the builders, but usually only `issuer` and `jwks_uri` are necessary.

```php
// Fetched issuer metadata:
$issuerMetadata = [
    'issuer' => 'https://issuer-name', // The Issuer name
    'jwks_uri' => 'https://jwks_uri', // The Issuer's JWK Set URI
];
```

The remote `jwks_uri` is the remote endpoint where the issuer public keys are exposed.

You also need the Client Metadata, usually the same provided from the [OpenID Dynamic Registration](https://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata)
but **you can just provide the `client_id`** and optionally the `client_secret` (in case the tokens are signed with symmetric key using the client secret).

Verfiers and decrypters are automatically configured using the [OpenID Dynamic Registration](https://openid.net/specs/openid-connect-registration-1_0.html#ClientMetadata)
client metadata.

If you use encryption, you should inject your JWK Set in the configuration `jwks` keys.

```php
// Client Metadata (complete configuration example)
$clientMetadata = [
    'client_id' => 'my-client-id',
    'client_secret' => 'my-client-secret',
    'id_token_signed_response_alg' => 'RS256',
    'id_token_encrypted_response_alg' => 'RSA-OAEP',
    'id_token_encrypted_response_enc' => 'A128GCM',
    'userinfo_signed_response_alg' => 'RS256',
    'userinfo_encrypted_response_alg' => 'RSA-OAEP',
    'userinfo_encrypted_response_enc' => 'A128GCM',
    'jwks' => [
        'keys' => [
            // client JWKs
        ],
    ],
];
```

```php
use Facile\JoseVerifier\AccessTokenVerifierBuilder;

$builder = new AccessTokenVerifierBuilder();
$builder->setIssuerMetadata($issuerMetadata);
$builder->setClientMetadata($clientMetadata);

$verifier = $builder->build();
$payload = $verifier->verify($token);
```

The verifier will decrypt and validate the token for you. The result is the token payload.

## Using cache to fetch remote JWK Set

Obviously you should not fetch the remote JWK Set on every request.
In order to use cache you can inject a partially configured 
`JwksProviderBuilder`.

```php
use Facile\JoseVerifier\JWK\JwksProviderBuilder;
use Facile\JoseVerifier\AccessTokenVerifierBuilder;

// Use your PSR SimpleCache implementation
$cache = $container->get(\Psr\SimpleCache\CacheInterface::class);

$jwksProviderBuilder = new JwksProviderBuilder();
$jwksProviderBuilder->setCache($cache);
$jwksProviderBuilder->setCacheTtl(86400); // 86400 is the default value

$builder = new AccessTokenVerifierBuilder();
$builder->setIssuerMetadata($issuerMetadata);
$builder->setClientMetadata($clientMetadata);
$builder->setJwksProviderBuilder($jwksProviderBuilder);

$verifier = $builder->build();
$payload = $verifier->verify($token);
```

## Provided verifiers

### Access Token Verifier

The AccessTokenVerifier will validate a JWT access token.

```php
use Facile\JoseVerifier\AccessTokenVerifierBuilder;

$builder = new AccessTokenVerifierBuilder();
$builder->setIssuerMetadata($issuerMetadata);
$builder->setClientMetadata($clientMetadata);

$verifier = $builder->build();
$payload = $verifier->verify($token);
```

### ID Token Verifier

The IdTokenVerifier will validate an OpenID `id_token`.

Create the verifier:

```php
use Facile\JoseVerifier\IdTokenVerifierBuilder;

$builder = new IdTokenVerifierBuilder();
$builder->setIssuerMetadata($issuerMetadata);
$builder->setClientMetadata($clientMetadata);

$verifier = $builder->build();
```

In order to validate an `id_token` you must provide some other parameters to the verifier 
(note that all verifiers are immutable).

```php
use Facile\JoseVerifier\IdTokenVerifierInterface;

/** @var IdTokenVerifierInterface $verifier */

// Provide the `state` used in the Code Grant Flow (this should be provided id the `id_token` contains the `s_hash` claim)
$verifier = $verifier->withState($state);

// Optionally provide these parameters to validate the correct hash values:

// Provide the `access_token` used in the Code Grant Flow
$verifier = $verifier->withAccessToken($accessToken);
// Provide the `code` used in the Code Grant Flow
$verifier = $verifier->withCode($code);

$payload = $verifier->verify($token);
``` 

### UserInfo Verifier

When UserInfo returns a signed (and maybe encrypted) JWT as response content of the userinfo endpoint you can use
this verifier to decrypt, verify, and obtain user info claims.

```php
use Facile\JoseVerifier\UserInfoVerifierBuilder;

$builder = new UserInfoVerifierBuilder();
$builder->setIssuerMetadata($issuerMetadata);
$builder->setClientMetadata($clientMetadata);

$verifier = $builder->build();
$payload = $verifier->verify($jwt);
```

## Using Psalm

If you need to use Psalm you can include the plugin in your `psalm.xml`.

```
<plugins>
    <pluginClass class="Facile\JoseVerifier\Psalm\Plugin" />
</plugins>
```
