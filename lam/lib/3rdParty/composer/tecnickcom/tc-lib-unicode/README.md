# tc-lib-unicode

> PHP library to process UTF-8 and Unicode text.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-unicode/version)](https://packagist.org/packages/tecnickcom/tc-lib-unicode)
[![Build](https://github.com/tecnickcom/tc-lib-unicode/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-unicode/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-unicode/graph/badge.svg?token=XLM0QWY9BE)](https://codecov.io/gh/tecnickcom/tc-lib-unicode)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-unicode/license)](https://packagist.org/packages/tecnickcom/tc-lib-unicode)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-unicode/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-unicode)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-unicode` converts between UTF-8 strings, character arrays and code point arrays, reorders text with the Unicode Bidirectional Algorithm (UAX #9) and applies script-specific character substitutions.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Unicode` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-unicode> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-unicode> |

---

## Features

### Conversion
- Conversions between UTF-8 strings, character arrays and code point arrays
- Latin1, UTF-16BE and hexadecimal string conversions

### Bidirectional Support
- Unicode Bidirectional Algorithm (UAX #9), verified against the official `BidiCharacterTest.txt` and `BidiTest.txt` conformance suites
- Right-to-left and mixed-direction text processing
- Arabic shaping driven by the Joining_Type property

### Character Substitution
- Context-sensitive codepoint-level substitution via `Substitution::replaceChars()`
- **Devanagari**: moves left-positional matras (U+093F, U+094E) to precede their base consonant cluster, including conjuncts joined by Virama (U+094D)
- **Hangul**: composes Hangul Jamo sequences (U+1100-U+11FF, U+A960-U+A97F, U+D7B0-U+D7FF) into precomposed syllables (U+AC00-U+D7A3) per section 3.12 of the Unicode standard
- **Thai**: returned unchanged, because Thai preposed vowels are already stored in visual order

---

## Requirements

- PHP 8.2 or later
- Extensions: `ctype`, `mbstring`, `pcre`
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
| Devanagari | U+0900-U+097F | Left matras repositioned before consonant cluster |
| Hangul Jamo | U+1100-U+11FF, U+A960-U+A97F, U+D7B0-U+D7FF | Jamo composed to precomposed syllables (U+AC00-U+D7A3) |
| Thai | U+0E00-U+0E7F | None: the stored order is the display order |

Codepoints belonging to unsupported scripts are passed through unchanged.

---

## Limitations

- The paragraph separator is emitted at the end of the paragraph output instead of being
  reset by L1 and reordered by L2, which would place it at the visual left edge of a
  right-to-left paragraph.
- Rule L3 (combining marks applied to characters shown in a different order) is not
  implemented.
- Shaping is Arabic only. The other cursive scripts (Syriac, N'Ko, Mandaic, Adlam) are
  returned unshaped.
- `Bidi` and `Convert` require valid UTF-8: malformed byte sequences raise an exception,
  while code points that cannot be encoded are replaced with '?'.

---

## Development

```bash
make deps
make help
make qa
make server
```

`make server` serves the `example/` directory on <http://localhost:8000>. Use a custom port with `make server PORT=8080`.

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
