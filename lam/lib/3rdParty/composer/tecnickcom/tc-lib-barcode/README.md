# tc-lib-barcode

> PHP library for generating linear, 2D and postal barcodes.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-barcode/version)](https://packagist.org/packages/tecnickcom/tc-lib-barcode)
[![Build](https://github.com/tecnickcom/tc-lib-barcode/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-barcode/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-barcode/graph/badge.svg?token=PW6r97iVuW)](https://codecov.io/gh/tecnickcom/tc-lib-barcode)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-barcode/license)](https://packagist.org/packages/tecnickcom/tc-lib-barcode)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-barcode/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-barcode)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-barcode` is a pure-PHP library that encodes data in 73 linear, 2D and postal barcode formats and outputs them as SVG, PNG, HTML, text grids, GD images or arrays of bar coordinates.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Barcode` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-barcode> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-barcode> |

---

## Why this library

- **One API for every format.** `getBarcodeObj()` returns the same object for a
  UPC-A and a Han Xin Code; the same `getInlineSvgCode()`, `getPngData()`,
  `getGrid()` and `getArray()` methods work across all 73 types.
- **Pure PHP, one runtime dependency.** Only `tecnickcom/tc-lib-color`, plus the
  `gd`, `ctype` and `pcre` extensions. `bcmath` and `imagick` are optional
  accelerators, never required.
- **Written to the published standards.** Each symbology follows its own
  published specification, named alongside the format where one applies.
- **Covered by tests.** Per-format encoding, ECC, size and input-validation
  suites, run under strict static analysis on every commit.
- **LGPL-3.0.** Usable in closed-source applications.
- **Maintained.** Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf)
  ecosystem, in continuous development since 2015.

---

## Supported Formats

### Linear

| Format | Description |
|--------|-------------|
| C39 | CODE 39 - ANSI MH10.8M-1983 - USD-3 - 3 of 9 |
| C39+ | CODE 39 + CHECKSUM |
| C39E | CODE 39 EXTENDED |
| C39E+ | CODE 39 EXTENDED + CHECKSUM |
| C32 | CODE 32 (Italian Pharmacode - IMH - Radix 32) |
| C49 | CODE 49 (ANSI/AIM BC6) |
| PZN | PZN (Pharmazentralnummer - IFA coding system) |
| LOGMARS | LOGMARS (CODE 39 profile of MIL-STD-1189B) |
| C93 | CODE 93 - USS-93 |
| S25 | Standard 2 of 5 |
| S25+ | Standard 2 of 5 + CHECKSUM |
| S25IATA | 2 of 5 IATA (Computer Identics 2 of 5) |
| S25MATRIX | 2 of 5 Matrix |
| S25DATALOGIC | 2 of 5 Datalogic (China Post Code) |
| I25 | Interleaved 2 of 5 |
| I25+ | Interleaved 2 of 5 + CHECKSUM |
| ITF14 | ITF-14 (GTIN-14 - GS1 General Specifications) |
| C128 | CODE 128 |
| C128A | CODE 128 A |
| C128B | CODE 128 B |
| C128C | CODE 128 C |
| C16K | CODE 16K (stacked CODE 128) |
| GS1128 | GS1-128 (CODE 128 with GS1 Application Identifiers) |
| SSCC18 | SSCC-18 (GS1-128 with the Application Identifier 00) |
| GS114 | GS1-14 - EAN-14 - SCC-14 (GS1-128 with the Application Identifier 01) |
| DATABAR | GS1 DataBar Omnidirectional (ISO/IEC 24724) |
| DATABARTRUNC | GS1 DataBar Truncated (ISO/IEC 24724) |
| DATABARSTACK | GS1 DataBar Stacked (ISO/IEC 24724) |
| DATABARSTACKOMNI | GS1 DataBar Stacked Omnidirectional (ISO/IEC 24724) |
| DATABARLIMITED | GS1 DataBar Limited (ISO/IEC 24724) |
| DATABAREXP | GS1 DataBar Expanded (ISO/IEC 24724) |
| DATABAREXPSTACK | GS1 DataBar Expanded Stacked (ISO/IEC 24724) |
| EAN2 | EAN 2-Digits UPC-Based Extension |
| EAN5 | EAN 5-Digits UPC-Based Extension |
| EAN8 | EAN 8 |
| EAN13 | EAN 13 |
| UPCA | UPC-A |
| UPCE | UPC-E |
| PLESSEY | Plessey Code |
| MSI | MSI (Variation of Plessey code) |
| MSI+ | MSI + CHECKSUM (modulo 11) |
| TELEPEN | Telepen (full ASCII) |
| CODABAR | CODABAR |
| CODE11 | CODE 11 |
| PHARMA | PHARMACODE |
| PHARMA2T | PHARMACODE TWO-TRACKS |
| HIBC39 | HIBC in CODE 39 (ANSI/HIBC 2.6 and ANSI/HIBC 1.3) |
| HIBC128 | HIBC in CODE 128 (ANSI/HIBC 2.6 and ANSI/HIBC 1.3) |
| LRAW | 1D RAW MODE (comma-separated rows of 01 strings) |

### 2D

| Format | Description |
|--------|-------------|
| AZTEC | AZTEC Code (ISO/IEC 24778:2008) |
| AZTECRUNE | AZTEC Rune (ISO/IEC 24778:2008 Annex A) |
| DATAMATRIX | DATAMATRIX (ISO/IEC 16022) |
| DMRE | Data Matrix Rectangular Extension (ISO/IEC 21471) |
| PDF417 | PDF417 (ISO/IEC 15438:2006) |
| PDF417C | Compact PDF417 - truncated (ISO/IEC 15438:2006) |
| QRCODE | QR-CODE |
| MICROQR | Micro QR Code (ISO/IEC 18004) |
| HANXIN | Han Xin Code (GB/T 21049, ISO/IEC 20830) |
| HIBCDM | HIBC in DATAMATRIX (ANSI/HIBC 2.6 and ANSI/HIBC 1.3) |
| HIBCQR | HIBC in QR-CODE (ANSI/HIBC 2.6 and ANSI/HIBC 1.3) |
| HIBCAZ | HIBC in AZTEC Code (ANSI/HIBC 2.6 and ANSI/HIBC 1.3) |
| SRAW | 2D RAW MODE (comma-separated rows of 01 strings) |

### Postal

| Format | Description |
|--------|-------------|
| POSTNET | POSTNET |
| PLANET | PLANET |
| RMS4CC | RMS4CC (Royal Mail 4-state Customer Bar Code) |
| KIX | KIX (Klant index - Customer index) |
| JPPOST | Japan Post Customer Barcode |
| MAILMARK | Royal Mail Mailmark 4-state barcode (types C and L) |
| IDENTCODE | Deutsche Post Identcode |
| LEITCODE | Deutsche Post Leitcode |
| AUSPOST | Australia Post 4-State Customer Barcode |
| IMB | IMB - Intelligent Mail Barcode - Onecode - USPS-B-3200 |
| IMBPRE | IMB - Intelligent Mail Barcode pre-processed |

---

## Output Formats

- SVG image (file, inline code, or standalone document)
- PNG image (via GD or Imagick)
- GD image object
- HTML `div` elements
- Character grid string
- Array of bar coordinates

Width, height, padding, foreground and background color are set per barcode.

---

## Requirements

- PHP 8.2 or later
- Extensions: `gd`
- Optional extensions: `bcmath` (speeds up the arbitrary precision arithmetic used by the IMB and PDF417 types, a pure-PHP implementation is used when it is missing)
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-barcode
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$barcode = new \Com\Tecnick\Barcode\Barcode();
$bobj = $barcode->getBarcodeObj(
    type: 'QRCODE,H',
    code: 'https://tecnick.com',
    width: -4,
    height: -4,
    color: 'black',
    padding: [-2, -2, -2, -2],
)->setBackgroundColor('white');

echo $bobj->getInlineSvgCode();
```

For more formats and rendering options, see the `example/` directory.

---

## Development

```bash
make deps
make help
make qa
```

To preview the example app in a browser:

```bash
make server
```

Then open <http://localhost:8000/> (served from the `example/` directory).

To use a different port:

```bash
make server PORT=8080
```

Build artifacts and reports are generated in `target/`.

---

## Packaging

```bash
make rpm
make deb
```

For system packages, bootstrap with:

```php
require_once '/usr/share/php/Com/Tecnick/Barcode/autoload.php';
```

---

## Identifiers issued by a registration body

The library encodes the data it is given. Several formats carry a number that
only its registration body can issue, so a symbol is valid in its scheme only
once that number has been obtained:

| Format | Issued by |
|--------|-----------|
| EAN, UPC, ITF-14, GS1-128, SSCC-18, GS1-14, GS1 DataBar | a GS1 member organisation, which issues the company prefix |
| HIBC39, HIBC128, HIBCDM, HIBCQR, HIBCAZ | HIBCC, which assigns the Labeler Identification Code |
| MAILMARK | Royal Mail, which issues the Supply Chain ID carried in the barcode |
| PZN | IFA GmbH, which allocates the Pharmazentralnummer |
| IDENTCODE, LEITCODE | Deutsche Post DHL, which assigns the customer number and the street codes |
| C32 | AIFA, which assigns the Italian AIC number |

---

## Trademarks

The names below identify the symbologies this library encodes and belong to
their respective owners. They are used descriptively; no affiliation with or
endorsement by their owners is claimed or implied.

| Name | Owner |
|------|-------|
| QR Code | DENSO WAVE INCORPORATED |
| GS1, GS1-128, GS1 DataBar | GS1 AISBL |
| Mailmark | Royal Mail Group Ltd |
| Intelligent Mail | United States Postal Service |
| PHARMA-CODE | Laetus GmbH |
| Telepen | S.B. Electronic Systems Ltd |
| HIBC, HIBCC | Health Industry Business Communications Council |
| KIX | PostNL |
| Identcode, Leitcode | Deutsche Post DHL |

GS1 licenses the claims necessary to implement its standards under its
[IP Policy](https://www.gs1.org/standards/ip), to GS1 members and to the
participants of the work group that developed the standard, rather than to the
public at large.

---

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md) before submitting a pull request.

