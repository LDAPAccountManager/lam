# tc-lib-pdf-parser

> Parser library for reading and extracting PDF document structures.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-parser/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-parser/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-parser/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-parser/graph/badge.svg?token=SIGYQJG8D4)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-parser)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-parser/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-parser/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-parser` parses raw PDF data into the cross-reference and trailer data of the document and an array of its objects, each decoded into a token array.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Parser` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-parser> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser> |

---

## Features

### Parsing
- Cross-reference tables and cross-reference streams, including `/Prev` chains and incremental updates
- Object streams (`/ObjStm`)
- Indirect references, resolved on demand and bounded against cycles
- Stream decoding through [tc-lib-pdf-filter](https://github.com/tecnickcom/tc-lib-pdf-filter), including the `Predictor` of `DecodeParms`

### Limits
- Maximum decoded size of a single stream
- Maximum nesting depth of arrays and dictionaries
- Maximum depth of indirect object resolutions
- Maximum number of chained cross-reference sections

---

## Requirements

- PHP 8.2 or later
- Extension: `pcre`
- Composer

---

## Configuration

The constructor accepts an array of parameters:

| Parameter | Type | Default | Meaning |
|---|---|---|---|
| `ignore_filter_errors` | `bool` | `false` | If true, a stream that fails to decode is kept as raw data instead of raising an exception |
| `decode_streams` | `bool` | `true` | If true, decode the stream payloads of the indirect objects while parsing |
| `max_stream_size` | `int` | `33554432` | Maximum size in bytes of a single decoded stream; `0` means unlimited |

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-parser
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$raw = file_get_contents('/path/to/document.pdf');
$parser = new \Com\Tecnick\Pdf\Parser\Parser(['ignore_filter_errors' => true]);

// $xref holds the cross-reference and trailer data,
// $objects the parsed objects keyed as "[object number]_[generation number]"
[$xref, $objects] = $parser->parse((string) $raw);
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
require_once '/usr/share/php/Com/Tecnick/Pdf/Parser/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

