# Okta JWT Verifier for PHP

This library helps you verify tokens that have been issued by Okta.  To learn more about verification cases and Okta's tokens please read [Working With OAuth 2.0 Tokens](https://developer.okta.com/authentication-guide/tokens/)


## Release status

This library uses semantic versioning and follows Okta's [library version policy](https://developer.okta.com/code/library-versions/).

| Version | Status                             |
| ------- | ---------------------------------- |
| 0.x     |  :warning: Beta Release (Retired)  |
| 1.x     |  :heavy_check_mark: Release        |

The latest release can always be found on the [releases page][github-releases].

## Need help?

If you run into problems using the SDK, you can

* Ask questions on the [Okta Developer Forums][devforum]
* Post [issues][github-issues] here on GitHub


## Installation
The Okta JWT Verifier can be installed through composer.

```bash
composer require okta/jwt-verifier
```

This library requires a JWT library. We currently support
[firebase/php-jwt](https://packagist.org/packages/firebase/php-jwt). You will have to install this or create
your own adaptor.

```bash
composer require firebase/php-jwt
```

To create your own adaptor, just implement the `Okta/JwtVerifier/Adaptors/Adaptor` in your own class.

You will also need to install a PSR-7 compliant library. We suggest that you use `guzzlehttp/psr7` in your project.

```bash
composer require guzzlehttp/psr7
```

## Usage

```php
<?php
$jwt = 'eyJhbGciOiJSUzI1Nqd0FfRzh6X0ZsOGlJRnNoUlRuQUkweVUifQ.eyJ2ZXIiOjEsiOiJwaHBAb2t0YS5jb20ifQ.ZGrn4fvIoCq0QdSyA';

$jwtVerifier = (new \Okta\JwtVerifier\JwtVerifierBuilder())
    ->setDiscovery(new \Okta\JwtVerifier\Discovery\Oauth) // This is not needed if using oauth.  The other option is OIDC
    ->setAdaptor(new \Okta\JwtVerifier\Adaptors\FirebasePhpJwt)
    ->setAudience('api://default')
    ->setClientId('{clientId}')
    ->setIssuer('https://{yourOktaDomain}.com/oauth2/default')
    ->build();

$jwt = $jwtVerifier->verify($jwt);

dump($jwt); //Returns instance of \Okta\JwtVerifier\JWT

dump($jwt->toJson()); // Returns Claims as JSON Object

dump($jwt->getClaims()); // Returns Claims as they come from the JWT Package used

dump($jwt->getIssuedAt()); // returns Carbon instance of issued at time
dump($jwt->getIssuedAt(false)); // returns timestamp of issued at time

dump($jwt->getExpirationTime()); //returns Carbon instance of Expiration Time
dump($jwt->getExpirationTime(false)); //returns timestamp of Expiration Time

```


[devforum]: https://devforum.okta.com/
[lang-landing]: https://developer.okta.com/code/php/
[github-issues]: /okta/okta-jwt-verifier-php/issues
[github-releases]: /okta/okta-jwt-verifier-php/releases