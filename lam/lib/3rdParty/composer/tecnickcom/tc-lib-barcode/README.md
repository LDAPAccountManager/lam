# tc-lib-barcode

> PHP library for generating linear and 2D barcodes.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-barcode/version)](https://packagist.org/packages/tecnickcom/tc-lib-barcode)
[![Build](https://github.com/tecnickcom/tc-lib-barcode/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-barcode/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-barcode/graph/badge.svg?token=PW6r97iVuW)](https://codecov.io/gh/tecnickcom/tc-lib-barcode)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-barcode/license)](https://packagist.org/packages/tecnickcom/tc-lib-barcode)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-barcode/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-barcode)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-barcode` is a pure-PHP barcode generation library with broad support for industrial, retail, logistics, and document automation use cases.

It focuses on deterministic output and specification-driven encoding, making it suitable for labels, tickets, warehouse flows, and compliance documents. The API is structured so applications can generate barcode data once and render it as vectors or raster images depending on their output target.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Barcode` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-barcode> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-barcode> |

---

## Supported Formats

### Linear

| Format | Description |
|--------|-------------|
| C39 | CODE 39 - ANSI MH10.8M-1983 - USD-3 - 3 of 9 |
| C39+ | CODE 39 with checksum |
| C39E | CODE 39 EXTENDED |
| C39E+ | CODE 39 EXTENDED + CHECKSUM |
| C93 | CODE 93 - USS-93 |
| S25 | Standard 2 of 5 |
| S25+ | Standard 2 of 5 + CHECKSUM |
| I25 | Interleaved 2 of 5 |
| I25+ | Interleaved 2 of 5 + CHECKSUM |
| C128 | CODE 128 |
| C128A | CODE 128 A |
| C128B | CODE 128 B |
| C128C | CODE 128 C |
| EAN2 | 2-Digits UPC-Based Extension |
| EAN5 | 5-Digits UPC-Based Extension |
| EAN8 | EAN 8 |
| EAN13 | EAN 13 |
| UPCA | UPC-A |
| UPCE | UPC-E |
| MSI | MSI (Variation of Plessey code) |
| MSI+ | MSI + CHECKSUM (modulo 11) |
| CODABAR | CODABAR |
| CODE11 | CODE 11 |
| PHARMA | PHARMACODE |
| PHARMA2T | PHARMACODE TWO-TRACKS |

### 2D

| Format | Description |
|--------|-------------|
| AZTEC | AZTEC Code (ISO/IEC 24778:2008) |
| DATAMATRIX | DATAMATRIX (ISO/IEC 16022) |
| PDF417 | PDF417 (ISO/IEC 15438:2006) |
| QRCODE | QR-CODE |
| SRAW | 2D RAW MODE (comma-separated rows of 01 strings) |

### Postal

| Format | Description |
|--------|-------------|
| POSTNET | POSTNET |
| PLANET | PLANET |
| RMS4CC | RMS4CC (Royal Mail 4-state Customer Code) - CBC (Customer Bar Code) |
| KIX | KIX (Klant index - Customer index) |
| IMB | IMB - Intelligent Mail Barcode - Onecode - USPS-B-3200 |
| IMBPRE | IMB - Intelligent Mail Barcode - Onecode - USPS-B-3200 - pre-processed |

## Rendering
- HTML output for web previews
- Image-based rendering for downstream processing
- Configurable dimensions, padding, and color

### Output Formats
- PNG Image
- SVG Image
- HTML DIV
- Unicode String
- Binary String

---

## Requirements

- PHP 8.2 or later
- Extensions: `bcmath`, `gd`
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

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md) before submitting a pull request.

