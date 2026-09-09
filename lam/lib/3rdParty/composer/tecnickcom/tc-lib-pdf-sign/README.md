# tc-lib-pdf-sign

> Digital signature primitives for PDF documents (PKCS#7, CAdES, PAdES).

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-sign/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-sign)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-sign/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-sign/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-sign/graph/badge.svg?token=Pv1MNH3X3v)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-sign)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-sign/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-sign)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-sign/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-sign)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-sign` provides the cryptographic building blocks and PDF signature objects used by `tc-lib-pdf` to produce signed PDF documents.
The crypto and the PDF object generation live here, while the host library keeps the ByteRange placement, the incremental update writer, and the public facade.

The package assembles CMS/CAdES signatures natively in pure PHP (via a small DER ASN.1 codec), so it can embed the ESS `signing-certificate-v2` attribute that `openssl_pkcs7_sign()` cannot add. This is what lifts a plain PKCS#7 signature to a PAdES baseline signature.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Sign` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-sign> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-sign> |

---

## Features

Signature profiles (each level builds on the previous one):

| Profile | /SubFilter | What it provides |
|---|---|---|
| **Legacy** | `adbe.pkcs7.detached` | ISO 32000-1 detached CMS, carrying the ESS `signing-certificate-v2` attribute. |
| **PAdES B-B** | `ETSI.CAdES.detached` | CAdES-based CMS with `content-type`, `message-digest`, and `signing-certificate-v2` signed attributes. |
| **PAdES B-T** | `ETSI.CAdES.detached` | B-B plus an RFC 3161 signature timestamp embedded as the `id-aa-signatureTimeStampToken` unsigned attribute. |
| **PAdES B-LT** | `ETSI.CAdES.detached` | B-T plus a Document Security Store (`/DSS`, `/VRI`) with certificate, OCSP, and CRL validation material. |
| **PAdES B-LTA** | `ETSI.CAdES.detached` + `ETSI.RFC3161` | B-LT plus a `/Type /DocTimeStamp` archive timestamp for long-term archival. |

- RSA and ECDSA signing keys, with SHA-256, SHA-384, or SHA-512 digests.
- Local (private key), external, and remote (HSM, smart card, CSC) signing. The CMS is assembled here in one call or in two, and the `tc-lib-pdf` facade drives the placeholder and the injection.
- The PAdES baseline output has been validated against the [EU DSS](https://ec.europa.eu/digital-building-blocks/sites/display/DIGITAL/Digital+Signature+Service+-++DSS) reference validator (B-B, B-T, B-LT, B-LTA all report the expected baseline level).

---

## Components

| Component | Responsibility |
|---|---|
| `Config` | Immutable signature configuration (profile, digest algorithm, certification level) with `/SubFilter` derivation. |
| `SignatureProfile` | Backed enum of the supported profiles, the closed set `Config` validates against. |
| `DigestAlgorithm` | Backed enum of the supported digests, with their lengths and NIST OIDs. |
| `Signer` | Orchestration entry point: builds the detached CAdES CMS and collects the LTV material, tying the pieces below together. |
| `Cms\Builder` | Native detached CAdES-BES `SignedData` builder (signs the DER signed attributes with `openssl_sign()`), in one call or in two. |
| `Cms\SigningRequest` | Validated, immutable inputs for the signed attributes; the state that crosses a two-phase signature. |
| `Cms\SignatureEncoding` | Encoding of an externally produced signature: DER, or the fixed-width ECDSA form (IEEE P1363). |
| `Cms\Asn1` | Minimal DER ASN.1 encoder/decoder for CMS, RFC 3161, and OCSP structures. |
| `Cms\Oid` | The CMS content type and signed attribute type OIDs this library emits and reads. |
| `Cms\Certificate` | X.509 field reader for the issuer, subject, serial, and public key that CMS and OCSP quote verbatim. |
| `Cms\SignatureVerifier` | Verifies a signature over a signed body against the certificate that produced it. |
| `Cms\SignedDataVerifier` | Verifies a CMS `SignedData`, whether it carries its own content, as a timestamp token does, or is detached over content the caller supplies. |
| `Timestamp\Client` / `Timestamp\Config` | RFC 3161 timestamp codec; the returned token is verified and matched against the request. |
| `Ocsp\Client` | RFC 6960 OCSP request builder and response validator. |
| `Ltv\ValidationMaterial` | DSS material collection: certificate dedup, AIA/CRL-DP URL extraction, OCSP/CRL retrieval. |
| `Ltv\Crl` | RFC 5280 CRL reader: issuer match, scope, validity interval, revocation entries, and signature. |
| `Ltv\SkipReason` | Why a revocation URL was discarded: revoked, invalid, unreachable, duplicate, not attempted. |
| `Output\Signature` | The `/Sig` value dictionary, including the `/ByteRange` and `/Contents` placeholders. |
| `Output\Widget` | Signature and empty-field widget annotations. |
| `Output\Dss` | DSS/VRI object emitter. |
| `Output\DocTimeStamp` | The `/Type /DocTimeStamp` value object (B-LTA). |
| `Output\PdfString` | Shared PDF string-token encoder. |
| `Exception` | Library exception type. |
| `RevokedException` | Raised when a responder or a CRL states that a certificate is revoked. |

### Design

The codecs are pure and perform no file or network access. HTTP transports (TSA, OCSP, CRL) and key loading are injected by the host as callables, so the consuming application owns networking and SSRF protection.

Nothing a TSA, an OCSP responder, or a CRL distribution point returns becomes validation material until its signature has been checked:

- **A timestamp token** must carry exactly one `SignerInfo` whose signature verifies against the TSA certificate the token embeds, whose `content-type` and `message-digest` attributes match the encapsulated `TSTInfo` (RFC 5652 sections 5.3 and 11.1), and whose ESS `signing-certificate` attribute, when present, names that same certificate. It must then answer the request that was sent (RFC 3161 section 2.4.2): the same message imprint under the same digest algorithm, the nonce echoed unchanged, the policy that was asked for when one was requested, and a `genTime` near the moment of the request. That certificate must be a TSA certificate: RFC 3161 section 2.3 reserves a TSA key for timestamping, so `id-kp-timeStamping` is required as the single extended key usage, marked critical, and the certificate must have covered the instant the token attests.
- **An OCSP response** is checked against the RFC 6960 section 3.2 acceptance rules: successful status, a basic response type, a signature that verifies against a responder the issuer authorised (itself, or a delegate holding `id-kp-OCSPSigning` that the issuer signed and that is inside its own validity period), a `SingleResponse` whose `CertID` matches the request by value, a good certificate status, and a validity interval covering the moment of use. `thisUpdate` is held to the age limit whether or not the response carries a `nextUpdate`, rule 5 being an acceptance rule of its own and no nonce being sent. Every entry matching the `CertID` is read rather than the first, and a `revoked` among them wins. A `singleExtension` or `responseExtension` marked critical that this codec does not recognise makes the response unusable (RFC 6960 section 4.4).
- **A CRL** must be one complete `CertificateList` issued by the certificate the distribution point came from and signed by it, with matching inner and outer signature algorithms, covering the moment of use, and not narrowed by a `deltaCRLIndicator` or an `issuingDistributionPoint`. The issuing certificate must be one that may sign a list at all (RFC 5280 section 6.3.3 (f)): a CA whose `keyUsage` admits `cRLSign`, the counterpart of the `id-kp-OCSPSigning` check on the OCSP side. A named `distributionPoint` is refused along with the other narrowings, being one shard of a partitioned list. `onlyContainsUserCerts` and `onlyContainsCACerts` split the issuer's certificates in two rather than narrowing the list, so which half answers is decided against the certificate the list was fetched for (RFC 5280 section 6.3.3). An extension type appearing twice is refused rather than collapsed to the last, and a critical extension this reader does not recognise makes the list unusable (RFC 5280 section 6.3.3 (a)(2)). The issuer certificate is required, so `Signer::collectValidationMaterial()` does not fetch a CRL for a chain entry whose issuer it does not hold and reports the URLs it therefore did not try. The certificate the list is fetched for is required too, though it may be passed as `null` to skip the revocation lookup; given one, its serial is looked up among the revoked entries.

Anything that fails is rejected here rather than embedded in the document. The number of URLs taken from one certificate extension is bounded at `Ltv\ValidationMaterial::MAX_URLS`, each one being a call to the host's transport; the excess is reported rather than dropped. Revocation collection is best-effort, so a rejection is a skip rather than an error: `Signer::collectValidationMaterial()` takes an `$onSkip` observer that receives every discarded URL with the reason and an `Ltv\SkipReason`, which separates a revoked verdict from a timeout:

```php
$signer->collectValidationMaterial($chain, $ocsp, $crl, $tokens, $now, function (
    string $source,
    string $url,
    string $reason,
    SkipReason $code,
): void {
    if ($code === SkipReason::Revoked) {
        throw new RuntimeException('Refusing to sign: ' . $reason);
    }

    $this->logger->warning('LTV material skipped', compact('source', 'url', 'reason'));
});
```

The digest and signature algorithms accepted for validation material are SHA-256 and above. SHA-1 is refused; a host that has to accept one from a legacy responder passes `allowSha1: true` to `Cms\SignatureVerifier` or `Cms\SignedDataVerifier` and injects it.

RSA signatures are PKCS#1 v1.5 throughout, identified by `rsaEncryption` as RFC 3370 section 3.2 defines it for CMS and read back under either that identifier or the `sha*WithRSAEncryption` form. **RSASSA-PSS is not supported**, in either direction: `openssl_sign()` cannot produce it and `openssl_verify()` cannot express its parameters, so a PSS-signing key cannot sign through this library, and a PSS-signed timestamp token, OCSP response, or CRL is refused as an unsupported signature algorithm.

Some checks stay with the host. The library does not decide whether a certificate is trusted, and by default it does not refuse to sign with one that has expired or whose key usage forbids signing, since a host may deliberately re-sign historical content. `Cms\Certificate::assertValidAt()` and `Cms\Certificate::assertUsableForSigning()` run those checks on demand, and `new Signer(checkSignerCertificate: true)` runs both on every `sign()` and `prepare()`.

`Timestamp\Config` carries `host`, `timeout`, `verifyPeer`, `username`, `password`, and `cert` for the host to apply to its own HTTP client. This library never reads them, opening no connection.

---

## Requirements

- PHP 8.2 or later
- Extensions: `hash`, `json`, `openssl`, `pcre`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-sign
```

This package is normally pulled in transitively by `tc-lib-pdf`; install it directly only when you need the low-level primitives on their own.

---

## Usage

For signing PDF documents, use the `tc-lib-pdf` fluent `signature()` facade, which drives this package end to end:

```php
$pdf->signature()->configure([
    'profile'          => 'pades-b-t',   // legacy | pades-b-b | pades-b-t | pades-b-lt | pades-b-lta
    'digest_algorithm' => 'sha256',      // sha256 | sha384 | sha512
    'signcert'         => 'file:///path/to/cert.pem',
    'privkey'          => 'file:///path/to/key.pem',
    'password'         => '',
]);
```

See the full guide in [`tc-lib-pdf/doc/DIGITAL_SIGNATURES.md`](https://github.com/tecnickcom/tc-lib-pdf/blob/main/doc/DIGITAL_SIGNATURES.md) and the runnable `E007`/`E008`/`E009`/`E081` signature examples in `tc-lib-pdf`.

### Low-level: building a detached CMS

`Cms\Builder` produces a detached CAdES-BES CMS over arbitrary bytes (the host supplies the ByteRange-covered content). It is the core of PAdES B-B:

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

use Com\Tecnick\Pdf\Sign\Cms\Builder;

$privateKey = openssl_pkey_get_private('file:///path/to/key.pem');
$certDer    = '';   // DER bytes of the signing certificate
$content    = '';   // detached content bytes (the ByteRange-covered document)

$cms = (new Builder())->sign(
    $content,        // detached content bytes (the ByteRange-covered document)
    $certDer,        // DER of the signing certificate
    $privateKey,     // OpenSSLAsymmetricKey (RSA or EC)
    [],              // additional chain certificates (DER), if any
    'sha256',        // digest algorithm
    time(),          // signing time (Unix timestamp)
);

// $cms is a DER-encoded CMS ContentInfo ready for injection into /Contents.
```

### Low-level: two-phase signing

`sign()` needs the private key and the content in this process. When either is unavailable, the same call is available in two halves. `signaturePayload()` returns the DER `SET OF` signed attributes that the signature has to cover, and `buildFromSignature()` turns those plus the signature into the CMS.

This covers a key held in a hardware token, a smart card, or a remote signing service, and a document too large to hold as a string, since the request carries the message digest rather than the content:

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

use Com\Tecnick\Pdf\Sign\Cms\Builder;
use Com\Tecnick\Pdf\Sign\Cms\SigningRequest;

$builder = new Builder();

// The digest can be computed incrementally, so the content is never held in memory.
$context = hash_init('sha256');
hash_update_stream($context, $stream);

$request = new SigningRequest(
    hash_final($context, true),   // digest of the ByteRange-covered content
    $certDer,                     // DER of the signing certificate
    'sha256',                     // digest algorithm
    time(),                       // signing time (Unix timestamp)
    false,                        // include the CMS signing-time attribute (false for PAdES)
);

$signature = $remoteSigner->sign($builder->signaturePayload($request));

$cms = $builder->buildFromSignature($request, $signature, []);
```

`Signer` exposes the same three steps (`prepare()`, `signaturePayload()`, `buildFromSignature()`), so a host driving a remote signer can complete the flow through that class alone while getting the profile rules applied.

`buildFromSignature()` verifies the signature against the signing certificate before it emits anything, so a signature over the wrong bytes, from the wrong key, or in the wrong encoding fails at the call. `Cms\SignedDataVerifier::verify()` reads a finished CMS back, given the content a detached signature covers:

```php
(new SignedDataVerifier())->verify($cms, $byteRangeContent);  // returns the signer certificate DER
```

Omit the content and the message has to carry its own, which is the shape of an RFC 3161 timestamp token; supply it and the message must carry none, because a message with content of its own is not the message being checked. An ECDSA signature returned as the fixed-width `r || s` concatenation is accepted by passing `SignatureEncoding::P1363` as the last argument.

The request is immutable and validated on construction. When the two phases are separate HTTP requests, carry it across with `toArray()` and `fromArray()`, which validate again on the way back in.

Validation is not authentication. Re-running the constructor rejects a payload that is not a valid request, but not one edited into a different valid request. Pass a secret to both calls and the state carries an HMAC that is checked before anything else, or carry it over a channel the host already protects.

```php
$state = $request->toArray($secret);          // adds a 'mac' entry
$request = SigningRequest::fromArray($state, $secret);
```

`Signer::prepare()` and `Signer::buildFromSignature()` are the same pair one level up, applying the profile rules: the signing-time attribute is omitted for PAdES, and a B-T or higher profile requires the timestamp client and transport.

`Signer::collectValidationMaterial()` takes the signer chain leaf-first, one certificate per entry as PEM or DER, and verifies that ordering by signature rather than by name. Taking the signature as the link is what lets an authority that re-issued its own certificate with a Name of the same value in another string type still be recognised as the issuer of the leaves it signed. A PEM entry holding more than one certificate, such as a `fullchain.pem`, is refused rather than decoded as a chain of one. Each entry is parsed as a certificate, and the armour has to say `CERTIFICATE`, since nearly everything a PEM file holds is a DER `SEQUENCE`.

The certificates a timestamp token embeds are ordered into a path starting at the certificate the token was verified against, not at a leaf picked out of the bag: the `certificates` field sits outside `signedAttrs` and is covered by no signature. A bag member outside the path is still collected, since a validator may need it, but nothing can be looked up for it, so its URLs are reported through `$onSkip` with `SkipReason::NotAttempted`.

`Signer::signatureTimestampTokens()` reads the tokens back out of a CMS. The token is produced inside `sign()`, by the provider the profile requires, and the host's transport sees the `TimeStampResp` rather than the token inside it, so this is how the output of `sign()` or `buildFromSignature()` feeds the collector:

```php
$cms = $signer->sign(...);                                    // B-T and above
$tokens = $signer->signatureTimestampTokens($cms);
$material = $signer->collectValidationMaterial($chain, $ocsp, $crl, $tokens, $now, $onSkip);
```

Pass the signature timestamp tokens as the fourth argument: the certificates they embed are ordered into a path, by signature for the same reason, and run through the same OCSP and CRL lookups as the signer's own chain, which is what ETSI EN 319 142-1 requires of a B-LT Document Security Store. Members of the token's certificate bag that do not chain are collected as certificates but not looked up, there being no issuer to build a lookup against. Ordering the bag costs a signature check per pair of members, so one larger than `Signer::MAX_PATH_CERTIFICATES` is refused rather than ordered.

Pass the signing time as the fifth argument so a retried or queued signature collects against the same instant it signs for. It is separate from the `$timestampNow` argument of `sign()` and `buildFromSignature()`, which is the moment the timestamp request is made and what the token's `genTime` is checked against.

`Output\Dss::emit()` returns a `state` entry alongside the objects. A DSS written by an incremental update replaces the one before it, so pass that state back on the next update (a second signature, or the B-LTA archive timestamp) and the earlier VRI entries are carried into the new dictionary instead of being dropped. Material an earlier revision already wrote is referenced again rather than written a second time.

A signed attribute a profile requires, such as the CAdES `signature-policy-identifier`, is passed to the request as an OID-keyed map. The attribute types the builder controls are reserved and cannot be overridden.

---

## Standards

- **ETSI EN 319 142-1** - PAdES baseline profiles (B-B, B-T, B-LT, B-LTA)
- **ISO 32000-1 / ISO 32000-2** - PDF digital signatures and the Document Security Store
- **RFC 5652** - Cryptographic Message Syntax (CMS)
- **RFC 5035** - ESS `signing-certificate-v2` attribute
- **RFC 3161** - Time-Stamp Protocol (TSP)
- **RFC 6960** - Online Certificate Status Protocol (OCSP)
- **RFC 5280** - X.509 certificates and CRLs

---

## Development

```bash
make deps
make help
make qa
```

---

## Packaging

```bash
make rpm
make deb
```

For system packages, bootstrap with:

```php
require_once '/usr/share/php/Com/Tecnick/Pdf/Sign/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).
