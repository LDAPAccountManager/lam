# tc-lib-pdf-filter

> Decoder library for standard PDF stream filters.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-filter/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-filter/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-filter/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-filter/graph/badge.svg?token=23KB9T46HA)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-filter)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-filter/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-filter/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-filter` decodes the stream filters defined by the PDF specification (PDF 32000-1:2008 §7.4), one at a time or as a chain, with the `DecodeParms` of each filter.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Filter` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-filter> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter> |

---

## Features

### PDF Filters
- `FlateDecode`, `LZWDecode`, `RunLengthDecode`
- `ASCIIHexDecode`, `ASCII85Decode`
- `CCITTFaxDecode`, `DCTDecode`, `JPXDecode`, `JBIG2Decode`
- `Crypt` (`Identity` and `None`)

### API
- Decode one filter, or a chain of filters applied in order
- `DecodeParms` as a single dictionary or as an array parallel to `Filter`
- `Predictor` (TIFF and PNG) for `FlateDecode` and `LZWDecode`
- Filter names as the full name, the inline-image abbreviation, or a `FilterType` enum case
- A single exception class for unknown filters and malformed streams

---

## Requirements

- PHP 8.2 or later
- Extensions: `zlib`, `pcre`
- Composer

Optional:

- Extension `imagick`: required by `CCITTFaxDecode` and `JPXDecode`
- CLI tool `jbig2dec`: required by `JBIG2Decode`

Without them those three filters throw; the others are unaffected.

---

## Decode parameters

`DecodeParms` entries are passed to `decode()` and `decodeAll()` as an array.
Beyond the parameters defined by the PDF specification, three filters accept
one more:

| Parameter | Filters | Meaning |
|---|---|---|
| `MaxOutputSize` | `FlateDecode`, `LZWDecode`, `RunLengthDecode` | Decoded-size cap in bytes; `0` (default) = unlimited |

`FlateDecode` reaches compression ratios above 1000:1, so callers decoding
untrusted documents should set `MaxOutputSize` to bound decompression bombs.

`CCITTFaxDecode` derives the image height from `Rows`. When `Rows` is absent the
height is estimated from the encoded length, which under-estimates it and
truncates the image, so pass `Rows` (the image dictionary `/Height`).

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-filter
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$filter = new \Com\Tecnick\Pdf\Filter\Filter();
$decoded = $filter->decodeAll(['ASCIIHexDecode', 'FlateDecode'], $data);
```

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
require_once '/usr/share/php/Com/Tecnick/Pdf/Filter/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

