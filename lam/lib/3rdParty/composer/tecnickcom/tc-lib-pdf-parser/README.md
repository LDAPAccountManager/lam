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
| `max_resolution_depth` | `int` | `64` | Maximum number of indirect object resolutions in flight at once; values below `1` are clamped to `1` |
| `max_nesting_depth` | `int` | `256` | Maximum nesting depth of array and dictionary objects; values below `1` are clamped to `1` |
| `strict_limits` | `bool` | `false` | If true, raise a `LimitException` as soon as a limit or a reference cycle leaves an object unresolved |

---

## Parsing Limits

Two limits bound the work a document can ask the parser to do. Both bound a recursion and cannot be
disabled: a value below `1` is clamped to `1`.

Exceeding `max_nesting_depth` always raises `Com\Tecnick\Pdf\Parser\LimitException`, a subclass of
`Com\Tecnick\Pdf\Parser\Exception`: a dictionary or array that cannot be tokenized has no usable
value to fall back to.

Reaching `max_resolution_depth`, or meeting a reference cycle, leaves the reference unresolved and
lets the rest of the document parse. Both cases are recorded and readable after `parse()`:

```php
$parser = new \Com\Tecnick\Pdf\Parser\Parser();
[$xref, $objects] = $parser->parse((string) $raw);

foreach ($parser->getLimitWarnings() as $warning) {
    // "the indirect object resolution depth limit (64) left a reference
    //  unresolved 7 times, first at object 128_0"
}
```

One warning is reported per kind of event, with the number of occurrences and the first object
affected. Setting `strict_limits` turns the first such event into a `LimitException` instead, and
`getLimitWarnings()` then always returns an empty list.

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

