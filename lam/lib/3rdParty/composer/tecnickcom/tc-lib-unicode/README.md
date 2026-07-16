# tc-lib-unicode

> UTF-8 and Unicode processing utilities, including bidirectional text handling.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-unicode/version)](https://packagist.org/packages/tecnickcom/tc-lib-unicode)
[![Build](https://github.com/tecnickcom/tc-lib-unicode/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-unicode/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-unicode/graph/badge.svg?token=XLM0QWY9BE)](https://codecov.io/gh/tecnickcom/tc-lib-unicode)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-unicode/license)](https://packagist.org/packages/tecnickcom/tc-lib-unicode)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-unicode/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-unicode)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-unicode` provides Unicode conversion helpers and bidirectional algorithm support for robust multilingual text processing.

It is built to handle multilingual text paths where normalization, code-point handling, and bidirectional ordering directly affect rendering quality. By isolating Unicode-heavy operations, dependent libraries can keep text processing accurate and easier to audit.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Unicode` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-unicode> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-unicode> |

---

## Features

### Unicode Utilities
- UTF-8 character and ordinal conversion helpers
- String/character array transformations
- Integration-ready conversion methods for document engines

### Bidirectional Support
- Unicode Bidirectional Algorithm implementation
- Right-to-left and mixed-direction text processing
- Supporting shaping/step logic for complex scripts

### Character Substitution
- Context-sensitive codepoint-level substitution via `Substitution::replaceChars()`
- **Thai** — repositions leading vowels (Sara E/AE/O/AI, U+0E40–U+0E44, U+0E4D) to follow their base consonant, matching PDF visual-order glyph streams
- **Devanagari** — moves left-positional matras (U+093F) to precede their base consonant cluster, including conjuncts joined by Virama (U+094D)
- **Hangul** — composes Hangul Jamo sequences (U+1100–U+11FF, U+A960–U+A97F, U+D7B0–U+D7FF) into precomposed syllables (U+AC00–U+D7A3) per Unicode Standard §3.12

---

## Requirements

- PHP 8.2 or later
- Extension: `mbstring`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-unicode
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$bidi = new \Com\Tecnick\Unicode\Bidi('hello ', null, null, 'R', false);
echo $bidi->getString();
```

---

## Character substitution

`Substitution::replaceChars()` takes an array of Unicode codepoints and returns a transformed array with script-specific substitutions applied. It is a pure codepoint-level transform with no font or PDF dependency.

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$sub = new \Com\Tecnick\Unicode\Substitution();

// Thai: leading vowel repositioned after its base consonant
// Logical order:  [U+0E40 SARA E, U+0E01 KO KAI]
// Visual order:   [U+0E01 KO KAI, U+0E40 SARA E]
$result = $sub->replaceChars([0x0E40, 0x0E01]);
// $result === [0x0E01, 0x0E40]

// Devanagari: left matra repositioned before its base consonant cluster
// Logical order:  [U+0915 KA, U+093F VOWEL SIGN I]
// Visual order:   [U+093F VOWEL SIGN I, U+0915 KA]
$result = $sub->replaceChars([0x0915, 0x093F]);
// $result === [0x093F, 0x0915]

// Hangul: Jamo composed into a precomposed syllable
// [U+1100 KIYEOK, U+1161 JUNGSEONG A, U+11A8 JONGSEONG KIYEOK] → [U+AC01 각]
$result = $sub->replaceChars([0x1100, 0x1161, 0x11A8]);
// $result === [0xAC01]
```

### Supported scripts and Unicode ranges

| Script | Unicode range(s) | Transformation |
|---|---|---|
| Thai | U+0E00–U+0E7F | Leading vowels repositioned after base consonant |
| Devanagari | U+0900–U+097F | Left matras repositioned before consonant cluster |
| Hangul Jamo | U+1100–U+11FF, U+A960–U+A97F, U+D7B0–U+D7FF | Jamo composed to precomposed syllables (U+AC00–U+D7A3) |

Codepoints belonging to unsupported scripts are passed through unchanged.

---

## Development

```bash
make deps
make help
make qa
make server
```

`make server` starts the local PHP development server for the `example/` directory on `http://localhost:8000`.
Use a custom port with `make server PORT=8080`.

---

## Packaging

```bash
make rpm
make deb
```

For system packages, bootstrap with:

```php
require_once '/usr/share/php/Com/Tecnick/Unicode/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).
