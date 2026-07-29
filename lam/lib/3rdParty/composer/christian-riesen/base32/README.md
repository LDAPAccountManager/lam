base32
======

Base32 Encoder/Decoder for PHP according to [RFC 4648](https://tools.ietf.org/html/rfc4648).

![CI](https://github.com/ChristianRiesen/base32/workflows/CI/badge.svg)

[![Latest Stable Version](https://poser.pugx.org/christian-riesen/base32/v/stable.png)](https://packagist.org/packages/christian-riesen/base32) [![Total Downloads](https://poser.pugx.org/christian-riesen/base32/downloads.png)](https://packagist.org/packages/christian-riesen/base32) [![Latest Unstable Version](https://poser.pugx.org/christian-riesen/base32/v/unstable.png)](https://packagist.org/packages/christian-riesen/base32) [![License](https://poser.pugx.org/christian-riesen/base32/license.png)](https://packagist.org/packages/christian-riesen/base32)


Installation
-----

Use composer:

```bash
composer require christian-riesen/base32
```

Usage
-----

```php
<?php

// Include class or user autoloader
use Base32\Base32;

$string = 'fooba';

// $encoded contains now 'MZXW6YTB'
$encoded = Base32::encode($string);

// $decoded is again 'fooba'
$decoded = Base32::decode($encoded);
```

You can also use the extended hex alphabet by using the `Base32Hex` class instead.

Strict decoding (2.0)
---------------------

As of 2.0, `decode()` is strictly [RFC 4648](https://tools.ietf.org/html/rfc4648)
conformant and **rejects** malformed input by throwing an
`\InvalidArgumentException`, instead of silently stripping unknown characters as
1.x did. Decoding is case-sensitive and requires correct padding. It throws when
the input is not uppercase, has a length that is not a multiple of 8, contains
characters outside the alphabet or misplaced padding, or carries non-zero
trailing bits (a non-canonical encoding).

`encode()` is unchanged and remains conformant, so anything produced by
`encode()` still decodes cleanly. If you decode user-supplied strings (e.g. TOTP
secrets that may be lowercase or spaced), normalize them first:

```php
$secret = strtoupper(preg_replace('/\s+/', '', $userInput));
$bytes  = Base32::decode($secret);
```

See [CHANGELOG.md](CHANGELOG.md) for the full 2.0 migration notes.

About
=====

Initially created to work with the [one time password project](https://github.com/ChristianRiesen/otp), yet it can stand alone just as well as [Jordi Boggiano](https://seld.be/) kindly pointed out. It's the only Base32 implementation that passes the test vectors and contains unit tests as well.

Goal
----
Have a RFC compliant Base32 encoder and decoder. The implementation could be improved, but for now, it does the job and has unit tests. Ideally, the class can be enhanced while the unit tests keep passing.

Requirements
------------

Works on PHP 8.1 and later (PHP 8.1, 8.2, 8.3 and 8.4 are tested in CI).

The 1.x releases support PHP 7.2 – 8.x; use those if you need to run on an
older PHP version.

Tests run on PHPUnit 10.5.

Author
------

Christian Riesen <chris.riesen@gmail.com> https://christianriesen.com

Acknowledgements
----------------

Base32 is mostly based on the work of https://github.com/NTICompass/PHP-Base32
