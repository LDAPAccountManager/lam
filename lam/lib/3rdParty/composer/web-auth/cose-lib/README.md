# COSE Library for PHP

[![CI](https://github.com/web-auth/cose-lib/actions/workflows/ci.yml/badge.svg)](https://github.com/web-auth/cose-lib/actions/workflows/ci.yml)
[![Latest Stable Version](https://poser.pugx.org/web-auth/cose-lib/v)](https://packagist.org/packages/web-auth/cose-lib)
[![Total Downloads](https://poser.pugx.org/web-auth/cose-lib/downloads)](https://packagist.org/packages/web-auth/cose-lib)
[![License](https://poser.pugx.org/web-auth/cose-lib/license)](https://packagist.org/packages/web-auth/cose-lib)

**CBOR Object Signing and Encryption (COSE) for PHP** is a comprehensive library that provides full support for COSE operations including signing, encryption, and MAC (Message Authentication Code) operations.

This library implements:
- **[RFC 9052](https://datatracker.ietf.org/doc/html/rfc9052)** - COSE: Structures and Process
- **[RFC 9053](https://datatracker.ietf.org/doc/html/rfc9053)** - COSE: Initial Algorithms

## Features

✅ **Complete COSE Tag Support**
- COSE_Sign1 (tag 18) - Single signature
- COSE_Sign (tag 98) - Multiple signatures
- COSE_Encrypt0 (tag 16) - Single recipient encryption
- COSE_Encrypt (tag 96) - Multiple recipients encryption
- COSE_Mac0 (tag 17) - MAC without recipients
- COSE_Mac (tag 97) - MAC with recipients

✅ **Cryptographic Algorithms**
- **Signatures**: ECDSA (ES256, ES384, ES512, ES256K), EdDSA (Ed25519, Ed448), RSA (RS256/384/512, PS256/384/512)
- **MAC**: HMAC with SHA-256/384/512
- Compatible with WebAuthn, FIDO2, and digital COVID certificates

✅ **Modern PHP**
- PHP 8.1+ with strict types
- Full type safety and PHPStan compliance
- Comprehensive test coverage

## Installation

Install the library with Composer:

```bash
composer require web-auth/cose-lib
```

For COSE tag support (Sign, Encrypt, Mac operations), also install:

```bash
composer require spomky-labs/cbor-php
```

## Quick Start

### Verifying a COSE_Sign1 Signature

```php
use CBOR\Decoder;
use CBOR\OtherObject\OtherObjectManager;
use CBOR\StringStream;
use CBOR\Tag\TagManager;
use Cose\Signature\CoseSign1Tag;
use Cose\Signature\Signature1;

// Setup decoder with COSE tag support
$tagManager = TagManager::create()->add(CoseSign1Tag::class);
$decoder = Decoder::create($tagManager, OtherObjectManager::create());

// Decode COSE_Sign1 message
$stream = new StringStream($encodedData);
$coseSign1 = $decoder->decode($stream);

// Extract components
$protectedHeader = $coseSign1->getProtectedHeader();
$payload = $coseSign1->getPayload();
$signature = $coseSign1->getSignature();

// Create signature structure for verification
$sigStructure = Signature1::create($protectedHeader, $payload);

// Verify (example with OpenSSL)
$isValid = openssl_verify(
    (string) $sigStructure,
    $derSignature,
    $publicKey,
    'sha256'
);
```

### Creating a COSE_Sign1 Message

```php
use CBOR\ByteStringObject;
use CBOR\MapItem;
use CBOR\MapObject;
use CBOR\NegativeIntegerObject;
use CBOR\UnsignedIntegerObject;
use Cose\Signature\CoseSign1Tag;

// Define headers
$protectedHeader = MapObject::create([
    MapItem::create(
        UnsignedIntegerObject::create(1), // alg
        NegativeIntegerObject::create(-7) // ES256
    ),
]);

$unprotectedHeader = MapObject::create([
    MapItem::create(
        UnsignedIntegerObject::create(4), // kid
        ByteStringObject::create('my-key-id')
    ),
]);

// Create COSE_Sign1
$coseSign1 = CoseSign1Tag::create(
    $protectedHeader,
    $unprotectedHeader,
    ByteStringObject::create('Message to sign'),
    ByteStringObject::create($signatureBytes)
);

// Encode to CBOR
$encoded = (string) $coseSign1;
```

## Documentation

- **[Usage Guide](doc/Usage.md)** - Complete documentation with examples
- **[RFC 9052](https://datatracker.ietf.org/doc/html/rfc9052)** - COSE Structures
- **[RFC 9053](https://datatracker.ietf.org/doc/html/rfc9053)** - COSE Algorithms

## Use Cases

This library is perfect for:

- 🏥 **Digital Health Certificates** - COVID-19 vaccination passes (EU Digital COVID Certificate)
- 🔐 **WebAuthn/FIDO2** - Authenticator attestation and assertion signatures
- 📱 **IoT Security** - Secure messaging for constrained devices
- 🌐 **Web PKI** - CBOR-based certificate chains
- 📄 **Document Signing** - Compact digital signatures

## Supported Algorithms

### Signature Algorithms

| Algorithm | Identifier | Description |
|-----------|------------|-------------|
| ES256 | -7 | ECDSA with SHA-256 |
| ES384 | -35 | ECDSA with SHA-384 |
| ES512 | -36 | ECDSA with SHA-512 |
| ES256K | -47 | ECDSA with secp256k1 |
| EdDSA | -8 | EdDSA |
| Ed25519 | - | EdDSA with Curve25519 |
| RS256 | -257 | RSASSA-PKCS1-v1_5 with SHA-256 |
| RS384 | -258 | RSASSA-PKCS1-v1_5 with SHA-384 |
| RS512 | -259 | RSASSA-PKCS1-v1_5 with SHA-512 |
| PS256 | -37 | RSASSA-PSS with SHA-256 |
| PS384 | -38 | RSASSA-PSS with SHA-384 |
| PS512 | -39 | RSASSA-PSS with SHA-512 |

### MAC Algorithms

| Algorithm | Identifier | Description |
|-----------|------------|-------------|
| HS256 | 5 | HMAC with SHA-256 |
| HS384 | 6 | HMAC with SHA-384 |
| HS512 | 7 | HMAC with SHA-512 |
| HS256/64 | 4 | HMAC with SHA-256 truncated to 64 bits |

## Testing

Run the test suite with:

```bash
composer test
```

Or using Castor:

```bash
castor phpunit
```

The library includes comprehensive tests including:
- Unit tests for all COSE tag types
- Integration tests with real cryptographic operations
- COVID-19 certificate verification examples
- Test fixtures with actual certificates

## Requirements

- PHP 8.1 or higher
- ext-json
- ext-openssl
- brick/math
- spomky-labs/pki-framework
- spomky-labs/cbor-php (for COSE tag support)

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](doc/Contributing.md) for details.

For security vulnerabilities, please email **security [at] spomky-labs.com** instead of using the issue tracker.

## Support

I bring solutions to your problems and answer your questions.

If you really love this project and the work I have done, or if you want me to prioritize your issues, you can support me:

- [Become a sponsor on GitHub](https://github.com/sponsors/Spomky)
- [Become a Patreon](https://www.patreon.com/FlorentMorselli)

## License

This software is released under the [MIT License](LICENSE).

## Credits

Maintained by [Florent Morselli](https://github.com/Spomky) and [contributors](https://github.com/web-auth/cose-lib/contributors).

---

Made with ❤️ for the PHP community
