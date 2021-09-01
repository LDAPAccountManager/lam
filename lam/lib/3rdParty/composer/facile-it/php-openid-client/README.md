# php-openid-client

Full OpenID client implementation.

[![Latest Stable Version](https://poser.pugx.org/facile-it/php-openid-client/v/stable)](https://packagist.org/packages/facile-it/php-openid-client)
[![Total Downloads](https://poser.pugx.org/facile-it/php-openid-client/downloads)](https://packagist.org/packages/facile-it/php-openid-client)
[![License](https://poser.pugx.org/facile-it/php-openid-client/license)](https://packagist.org/packages/facile-it/php-openid-client)
[![codecov](https://codecov.io/gh/facile-it/php-openid-client/branch/master/graph/badge.svg?token=T59S02D2KM)](https://codecov.io/gh/facile-it/php-openid-client)
[![Build Status](https://github.com/facile-it/php-openid-client/workflows/Continuous%20Integration/badge.svg?branch=master)](https://github.com/facile-it/php-openid-client/actions)


Most of the library code is based on the awesome [`node-openid-client`](https://github.com/panva/node-openid-client).

**The PHP extension `gmp` could be required.**

## Implemented specs and features

- [OAuth 2.0 RFC 6749](https://tools.ietf.org/html/rfc6749) & [OpenID Connect Core 1.0](https://openid.net/specs/openid-connect-core-1_0.html)
  - Authorization (Authorization Code Flow, Implicit Flow, Hybrid Flow)
  - UserInfo Endpoint and ID Tokens including Signing and Encryption (using the [JWT Framework](https://github.com/web-token/jwt-framework) library)
  - Passing a Request Object by Value or Reference including Signing and Encryption
  - Offline Access / Refresh Token Grant
  - Client Credentials Grant
  - Client Authentication incl. `client_secret_jwt` and `private_key_jwt` methods
- [OpenID Connect Discovery 1.0](https://openid.net/specs/openid-connect-discovery-1_0.html)
- [OpenID Connect Dynamic Client Registration 1.0](https://openid.net/specs/openid-connect-registration-1_0.html) and [RFC7591 OAuth 2.0 Dynamic Client Registration Protocol](https://tools.ietf.org/html/rfc7591)
- [OAuth 2.0 Form Post Response Mode](https://openid.net/specs/oauth-v2-form-post-response-mode-1_0.html)
- [RFC7009 - OAuth 2.0 Token Revocation](https://tools.ietf.org/html/rfc7009)
- [RFC7662 - OAuth 2.0 Token Introspection](https://tools.ietf.org/html/rfc7662)
- [RFC7592 - OAuth 2.0 Dynamic Client Registration Management Protocol](https://tools.ietf.org/html/rfc7592)


### Supports of the following draft specifications

- [JWT Response for OAuth Token Introspection - draft 03](https://tools.ietf.org/html/draft-ietf-oauth-jwt-introspection-response-03)
- [JWT Secured Authorization Response Mode for OAuth 2.0 (JARM) - draft 02](https://openid.net/specs/openid-financial-api-jarm-wd-02.html)
- [OAuth 2.0 JWT Secured Authorization Request (JAR)](https://tools.ietf.org/html/draft-ietf-oauth-jwsreq-19)
- [OAuth 2.0 Mutual TLS Client Authentication and Certificate Bound Access Tokens (MTLS) - draft 15](https://tools.ietf.org/html/draft-ietf-oauth-mtls-15)


## Installation

Requirements:
- `psr/http-client-implementation` implementation
- `psr/http-factory-implementation` implementation
- `psr/http-message-implementation` implementation

```
composer require facile-it/php-openid-client
```

`RSA` signing algorithms are already included from the JWT Framework package`. 
If you need other algorithms you should install it manually.

## Basic Usage

For a basic usage you shouldn't require any other dependency package.

Every builder have methods to customize instances with other dependencies.

```php

use Facile\OpenIDClient\Client\ClientBuilder;
use Facile\OpenIDClient\Issuer\IssuerBuilder;
use Facile\OpenIDClient\Client\Metadata\ClientMetadata;
use Facile\OpenIDClient\Service\Builder\AuthorizationServiceBuilder;
use Facile\OpenIDClient\Service\Builder\UserInfoServiceBuilder;
use Psr\Http\Message\ServerRequestInterface;

$issuer = (new IssuerBuilder())
    ->build('https://example.com/.well-known/openid-configuration');
$clientMetadata = ClientMetadata::fromArray([
    'client_id' => 'client-id',
    'client_secret' => 'my-client-secret',
    'token_endpoint_auth_method' => 'client_secret_basic', // the auth method tor the token endpoint
    'redirect_uris' => [
        'https://my-rp.com/callback',    
    ],
]);
$client = (new ClientBuilder())
    ->setIssuer($issuer)
    ->setClientMetadata($clientMetadata)
    ->build();

// Authorization

$authorizationService = (new AuthorizationServiceBuilder())->build();
$redirectAuthorizationUri = $authorizationService->getAuthorizationUri(
    $client,
    ['login_hint' => 'user_username'] // custom params
);
// you can use this uri to redirect the user


// Get access token

/** @var ServerRequestInterface::class $serverRequest */
$serverRequest = null; // get your server request
$callbackParams = $authorizationService->getCallbackParams($serverRequest, $client);
$tokenSet = $authorizationService->callback($client, $callbackParams);

$idToken = $tokenSet->getIdToken(); // Unencrypted id_token
$accessToken = $tokenSet->getAccessToken(); // Access token
$refreshToken = $tokenSet->getRefreshToken(); // Refresh token

$claims = $tokenSet->claims(); // IdToken claims (if id_token is available)


// Refresh token
$tokenSet = $authorizationService->refresh($client, $tokenSet->getRefreshToken());


// Get user info
$userInfoService = (new UserInfoServiceBuilder())->build();
$userInfo = $userInfoService->getUserInfo($client, $tokenSet);

```


## Client registration

See [OpenID Connect Dynamic Client Registration 1.0](https://openid.net/specs/openid-connect-registration-1_0.html) 
and [RFC7591 OAuth 2.0 Dynamic Client Registration Protocol](https://tools.ietf.org/html/rfc7591).

```php

use Facile\OpenIDClient\Service\Builder\RegistrationServiceBuilder;

$registration = (new RegistrationServiceBuilder())->build();

// registration
$metadata = $registration->register(
    $issuer,
    [
        'client_name' => 'My client name',
        'redirect_uris' => ['https://my-rp.com/callback'],
    ],
    'my-initial-token'
);

// read
$metadata = $registration->read($metadata['registration_client_uri'], $metadata['registration_access_token']);

// update
$metadata = $registration->update(
    $metadata['registration_client_uri'],
    $metadata['registration_access_token'],
    array_merge($metadata, [
        // new metadata
    ])
);

// delete
$registration->delete($metadata['registration_client_uri'], $metadata['registration_access_token']);

```


## Token Introspection

See [RFC7662 - OAuth 2.0 Token Introspection](https://tools.ietf.org/html/rfc7662).

```php
use Facile\OpenIDClient\Service\Builder\IntrospectionServiceBuilder;

$service = (new IntrospectionServiceBuilder())->build();

$params = $service->introspect($client, $token);
```


## Token Revocation

See [RFC7009 - OAuth 2.0 Token Revocation](https://tools.ietf.org/html/rfc7009).

```php
use Facile\OpenIDClient\Service\Builder\RevocationServiceBuilder;

$service = (new RevocationServiceBuilder())->build();

$params = $service->revoke($client, $token);
```


## Request Object

You can create a [request object](https://openid.net/specs/openid-connect-core-1_0.html#RequestUriParameter) authorization request with the
`Facile\OpenIDClient\RequestObject\RequestObjectFactory` class.

This will create a signed (and optionally encrypted) JWT token based on
your client metadata.

```php
use Facile\OpenIDClient\RequestObject\RequestObjectFactory;

$factory = new RequestObjectFactory();
$requestObject = $factory->create($client, [/* custom claims to include in the JWT*/]);
```

Then you can use it to create the AuthRequest:

```php
use Facile\OpenIDClient\Authorization\AuthRequest;

$authRequest = AuthRequest::fromParams([
    'client_id' => $client->getMetadata()->getClientId(),
    'redirect_uri' => $client->getMetadata()->getRedirectUris()[0],
    'request' => $requestObject,
]);
```


## Aggregated and Distributed Claims

The library can handle [aggregated and distributed claims](https://openid.net/specs/openid-connect-core-1_0.html#AggregatedDistributedClaims):

```php
use Facile\OpenIDClient\Claims\AggregateParser;
use Facile\OpenIDClient\Claims\DistributedParser;

$aggregatedParser = new AggregateParser();

$claims = $aggregatedParser->unpack($client, $userInfo);

$distributedParser = new DistributedParser();
$claims = $distributedParser->fetch($client, $userInfo);
````


## Using middlewares

There are some middlewares and handles available:

### SessionCookieMiddleware

This middleware should always be on top of middlewares chain to provide
a session for `state` and `nonce` parameters.

To use it you should install the `dflydev/fig-cookies` package:

```
$ composer require "dflydev/fig-cookies:^2.0"
```

```php
use Facile\OpenIDClient\Middleware\SessionCookieMiddleware;
use Psr\SimpleCache\CacheInterface;

// Use your PSR-16 simple-cache implementation to persist sessions
/** @var CacheInterface $cache */
$middleware = new SessionCookieMiddleware($cache/* , $cookieName = "openid", $ttl = 300 */);
```

The middleware provides a `Facile\OpenIDClient\Session\AuthSessionInterface`
attribute with an `Facile\OpenIDClient\Session\AuthSessionInterface` stateful 
instance used to persist session data.

#### Using another session storage

If you have another session storage, you can handle it and provide a
`Facile\OpenIDClient\Session\AuthSessionInterface` instance in the
`Facile\OpenIDClient\Session\AuthSessionInterface` attribute.


### ClientProviderMiddleware

This middleware should always be on top of middlewares chain to provide the client to the other middlewares.

```php
use Facile\OpenIDClient\Middleware\ClientProviderMiddleware;

$client = $container->get('openid.clients.default');
$middleware = new ClientProviderMiddleware($client);
```

### AuthRequestProviderMiddleware

This middleware provide the auth request to use with the `AuthRedirectHandler`.

```php
use Facile\OpenIDClient\Middleware\AuthRequestProviderMiddleware;
use Facile\OpenIDClient\Authorization\AuthRequest;

$authRequest = AuthRequest::fromParams([
    'scope' => 'openid',
    // other params...
]);
$middleware = new AuthRequestProviderMiddleware($authRequest);
```


### AuthRedirectHandler

This handler will redirect the user to the OpenID authorization page.

```php
use Facile\OpenIDClient\Middleware\AuthRedirectHandler;
use Facile\OpenIDClient\Service\AuthorizationService;

/** @var AuthorizationService $authorizationService */
$authorizationService = $container->get(AuthorizationService::class);
$middleware = new AuthRedirectHandler($authorizationService);
```

### CallbackMiddleware

This middleware will handle the callback from the OpenID provider.

It will provide a `Facile\OpenIDClient\Token\TokenSetInterface` attribute
with the final TokenSet object.


```php
use Facile\OpenIDClient\Middleware\CallbackMiddleware;
use Facile\OpenIDClient\Service\AuthorizationService;

/** @var AuthorizationService $authorizationService */
$authorizationService = $container->get(AuthorizationService::class);
$middleware = new CallbackMiddleware($authorizationService);
```

### UserInfoMiddleware

This middleware will fetch user data from the userinfo endpoint and will
provide an `Facile\OpenIDClient\Middleware\UserInfoMiddleware` attribute 
with user infos as array.

```php
use Facile\OpenIDClient\Middleware\UserInfoMiddleware;
use Facile\OpenIDClient\Service\UserInfoService;

/** @var UserInfoService $userInfoService */
$userInfoService = $container->get(UserInfoService::class);
$middleware = new UserInfoMiddleware($userInfoService);
```


## Performance improvements for production environment

It's important to use a cache to avoid to fetch issuer configuration and keys on every request.

```php
use Psr\SimpleCache\CacheInterface;
use Facile\OpenIDClient\Issuer\IssuerBuilder;
use Facile\OpenIDClient\Issuer\Metadata\Provider\MetadataProviderBuilder;
use Facile\JoseVerifier\JWK\JwksProviderBuilder;

/** @var CacheInterface $cache */
$cache = $container->get(CacheInterface::class); // get your simple-cache implementation

$metadataProviderBuilder = (new MetadataProviderBuilder())
    ->setCache($cache)
    ->setCacheTtl(86400*30); // Cache metadata for 30 days 
$jwksProviderBuilder = (new JwksProviderBuilder())
    ->setCache($cache)
    ->setCacheTtl(86400); // Cache JWKS for 1 day
$issuerBuilder = (new IssuerBuilder())
    ->setMetadataProviderBuilder($metadataProviderBuilder)
    ->setJwksProviderBuilder($jwksProviderBuilder);

$issuer = $issuerBuilder->build('https://example.com/.well-known/openid-configuration');
```

## Using Psalm

If you need to use Psalm you can include the plugin in your `psalm.xml`.

```
<plugins>
    <pluginClass class="Facile\JoseVerifier\Psalm\Plugin" />
</plugins>
```
