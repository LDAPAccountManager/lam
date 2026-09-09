# tc-lib-unicode-data

> Unicode data tables, mappings and constants for PHP text processing.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-unicode-data/version)](https://packagist.org/packages/tecnickcom/tc-lib-unicode-data)
[![Build](https://github.com/tecnickcom/tc-lib-unicode-data/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-unicode-data/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-unicode-data/graph/badge.svg?token=12SAG9XRFK)](https://codecov.io/gh/tecnickcom/tc-lib-unicode-data)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-unicode-data/license)](https://packagist.org/packages/tecnickcom/tc-lib-unicode-data)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-unicode-data/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-unicode-data)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-unicode-data` provides the Unicode lookup tables, mappings and constants used by `tc-lib-unicode` and related libraries.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Unicode\Data` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-unicode-data> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-unicode-data> |

---

## Features

### Classes

| Class | Content |
|---|---|
| `Type` | Bidi_Class of every code point, with `getType()` and `getBidiClass()` |
| `BidiClass` | Backed enum of the strong, weak and neutral bidirectional classes |
| `Constant` | Code points of the bidirectional formatting characters and of some common separators |
| `Pattern` | Regular expressions matching right-to-left and Arabic text |
| `Mirror` | Mirrored form of the characters mirrored in a right-to-left context |
| `Bracket` | Paired brackets, by opening and by closing code point |
| `Arabic` | Joining types, presentation forms and ligatures, with `getJoiningType()` |
| `Encoding` | Character code to glyph name maps of 22 font encodings |
| `Latin` | Unicode to Latin1 character substitutions |
| `Identity` | CMap stream for the Identity-H encoding |

---

## Requirements

- PHP 8.2 or later
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-unicode-data
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

echo \Com\Tecnick\Unicode\Data\Type::getType(0x05D0);          // R
echo \Com\Tecnick\Unicode\Data\Arabic::getJoiningType(0x0628); // D
echo \Com\Tecnick\Unicode\Data\Encoding::MAP['cp1252'][128];   // Euro
```

`Type::getBidiClass()` returns the same class as a `BidiClass` enum case, and `null` for the explicit formatting codes (LRE, LRO, RLE, RLO, PDF, LRI, RLI, FSI, PDI).

---

## Generated data

`Type`, `Pattern`, `Mirror`, `Bracket` and `Arabic` are generated from the Unicode Character Database by `tools/generate.php`; `Type::UNICODE_VERSION` reports the version they derive from.

| Class | UCD source |
|---|---|
| `Type` | `extracted/DerivedBidiClass.txt` |
| `Pattern` | `extracted/DerivedBidiClass.txt` |
| `Mirror` | `BidiMirroring.txt` |
| `Bracket` | `BidiBrackets.txt` |
| `Arabic` | `ArabicShaping.txt`, `UnicodeData.txt` |

`Type::UNI` only lists the code points whose Bidi_Class is not `L`; `Type::getType()` resolves any code point, including the blocks whose unassigned code points default to `R`, `AL` or `ET`.

To rebuild the tables from a UCD release:

```bash
make gendata UCDVERSION=17.0.0
make qa
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
require_once '/usr/share/php/Com/Tecnick/Unicode/Data/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

