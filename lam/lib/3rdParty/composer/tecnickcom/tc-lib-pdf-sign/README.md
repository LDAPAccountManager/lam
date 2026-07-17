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
| **Legacy** | `adbe.pkcs7.detached` | ISO 32000-1 detached CMS (now carrying the ESS `signing-certificate-v2` attribute). |
| **PAdES B-B** | `ETSI.CAdES.detached` | CAdES-based CMS with `content-type`, `message-digest`, and `signing-certificate-v2` signed attributes. |
| **PAdES B-T** | `ETSI.CAdES.detached` | B-B plus an RFC 3161 signature timestamp embedded as the `id-aa-signatureTimeStampToken` unsigned attribute. |
| **PAdES B-LT** | `ETSI.CAdES.detached` | B-T plus a Document Security Store (`/DSS`, `/VRI`) with certificate, OCSP, and CRL validation material. |
| **PAdES B-LTA** | `ETSI.CAdES.detached` + `ETSI.RFC3161` | B-LT plus a `/Type /DocTimeStamp` archive timestamp for long-term archival. |

- RSA and ECDSA signing keys, with SHA-256, SHA-384, or SHA-512 digests.
- Both the local (private key) and the external/remote (HSM) signing workflows are supported through the `tc-lib-pdf` facade, which builds on these primitives.
- The PAdES baseline output has been validated against the [EU DSS](https://ec.europa.eu/digital-building-blocks/sites/display/DIGITAL/Digital+Signature+Service+-++DSS) reference validator (B-B, B-T, B-LT, B-LTA all report the expected baseline level).

---

## Components

| Component | Responsibility |
|---|---|
| `Config` | Immutable signature configuration (profile, digest algorithm, certification level) with `/SubFilter` derivation. |
| `Signer` | Orchestration entry point: builds the detached CAdES CMS and collects the LTV material, tying the pieces below together. |
| `Cms\Builder` | Native detached CAdES-BES `SignedData` builder (signs the DER signed attributes with `openssl_sign()`). |
| `Cms\Asn1` | Minimal DER ASN.1 encoder/decoder for CMS, RFC 3161, and OCSP structures. |
| `Timestamp\Client` / `Timestamp\Config` | RFC 3161 timestamp request/response codec. |
| `Ocsp\Client` | RFC 6960 OCSP request builder and response fetcher. |
| `Ltv\ValidationMaterial` | DSS material collection: certificate dedup, AIA/CRL-DP URL extraction, OCSP/CRL retrieval. |
| `Output\Signature` | The `/Sig` value dictionary, including the `/ByteRange` and `/Contents` placeholders. |
| `Output\Widget` | Signature and empty-field widget annotations. |
| `Output\Dss` | DSS/VRI object emitter. |
| `Output\DocTimeStamp` | The `/Type /DocTimeStamp` value object (B-LTA). |
| `Output\PdfString` | Shared PDF string-token encoder. |
| `Exception` | Library exception type. |

### Design

The codecs are pure and perform no file or network access. HTTP transports (TSA, OCSP, CRL) and key loading are injected by the host as callables, so the consuming application owns networking and SSRF protection. This keeps the package deterministic and testable, and lets the host reuse its existing HTTP stack and URL allow-list.

---

## Requirements

- PHP 8.2 or later
- Extensions: `hash`, `openssl`
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
