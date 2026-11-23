# tc-lib-pdf-font

> Font import, metrics, and stack management utilities for PDF generation.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-font/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-font)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-font/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-font/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-font/graph/badge.svg?token=wGN6UnOAFo)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-font)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-font/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-font)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-font/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-font)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-font` provides font import and runtime font-stack services used by PDF composition engines.

It handles font metrics, encodings, and font program references, turning static font assets into the definition files and PDF objects a document needs.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Font` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-font> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-font> |

---

## Features

### Font Processing
- Import support for core, Type1, and TrueType sources
- Font metadata extraction and normalization
- Utilities for subset and output dictionary generation
- Optional injectable cache to reuse computed TrueType font subsets

### Runtime Font Stack
- Font stack insertion and switching
- Glyph width/bounding-box helpers
- Character replacement and fallback handling
- Glyph index encoding of composite fonts, with support for the supplementary planes
- Unicode Variation Sequences (cmap format 14), including Ideographic Variation Sequences

---

## Requirements

- PHP 8.2 or later
- Extensions: `json`, `zlib`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-font
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$font = new \Com\Tecnick\Pdf\Font\Import('/path/to/font.ttf');
$metrics = $font->getFontMetrics();

var_dump($font->getFontName(), $metrics['type']);
```

For larger examples, refer to `test/OutputTest.php` and the conversion tooling in this repository.

---

## Character Encoding of Composite Fonts

A `TrueTypeUnicode` font is emitted as a composite (Type0) font with the `Identity-H` encoding, so every character code in a content stream is 2 bytes wide.

The character code is the glyph index of the font (`CID == GID`): the font dictionary declares `/CIDToGIDMap /Identity`, the `/W` array is keyed by glyph index, and a `/ToUnicode` CMap is generated for the glyphs used by the document so that the text stays searchable and extractable. The alias codes of the variation sequences are the exception (see below).

### Encoding a string

The consumer of this library must encode composite text through the font stack, so that the glyphs used are recorded for the `/W` array and the `/ToUnicode` CMap:

```php
$stack = new \Com\Tecnick\Pdf\Font\Stack(1.0);
$stack->insert($objnum, 'dejavusans', '', 12);

if ($stack->isCurrentGidEncoded()) {
    // 2-byte big-endian glyph indices, and the glyphs are recorded on the font
    $codes = $stack->ordArrToGidStr([0x41, 0x20, 0x1D703]);
} else {
    // CID-0 fonts keep the UTF-16BE encoding expected by their predefined CMap
    $codes = $uniconv->toUTF16BE($str);
}
```

The text object written to the page must also select the font the string was encoded with, otherwise the glyph indices are resolved against a different font. This does not apply to the fonts that are not GID encoded, whose character codes mean the same thing in every font.

Relevant methods:

- `Stack::isCurrentGidEncoded()`: true when the current font encodes text as glyph indices.
- `Stack::ordArrToGidStr(array $uniarr)`: encodes codepoints as 2-byte glyph indices and records them.
- `Stack::getGidForOrd(int $ord)`: glyph index of a codepoint, `0` when the font has no glyph for it.
- `Buffer::addUsedGid(string $key, int $gid, int $ord)`: records a glyph and the codepoint it was encoded from.
- `Buffer::addUsedGlyph(string $key, int $gid, array $text)`: records a glyph and the codepoints it was selected for, and returns its character code.
- `Buffer::setSubsetMode(bool $subset)`: sets the subset mode of the fonts loaded afterwards without an explicit mode.

### Variation sequences

A base character followed by a variation selector (U+180B..U+180D, U+180F, U+FE00..U+FE0F, U+E0100..U+E01EF) is shown with the glyph the font maps the sequence to in its cmap format 14 subtable, as used by the Ideographic Variation Sequences of CJK fonts:

```php
// U+908A followed by VARIATION SELECTOR-18
$codes = $stack->ordArrToGidStr([0x908A, 0xE0101]);
```

- The selector shows no glyph of its own and takes no space.
- A sequence the font does not map is shown with the glyph of the base character.
- The `/ToUnicode` CMap maps the code to the whole sequence, so copied text keeps the selector.
- A glyph used both for a sequence and for another text gets an alias code above the glyph count of the font, so each text is extracted correctly. A font with alias codes writes a `/CIDToGIDMap` stream instead of `/Identity`.

Relevant methods:

- `Stack::isVariationSelector(int $ord)`: true for a variation selector.
- `Stack::getVariationGid(int $base, int $selector)`: glyph index of a sequence, `0` when the font does not map it.
- `Stack::getOrdArrCharWidths(array $uniarr)`: width of each codepoint, with the variant glyph widths.

### Font definition files

The glyph index of a codepoint is read at runtime from the `.ctg.z` artifact of the font, which covers the whole Basic Multilingual Plane. The file must remain next to the `.json` definition file.

Codepoints above U+FFFF do not fit that table and are stored in the definition file under the `ctgu` key:

```json
"ctgu": {"120579": 1588, "119886": 941}
```

That key is only written when the font is converted with `--encoding_id=10`, which selects the format 12 cmap subtable.

A font with a cmap format 14 subtable also stores its glyph count, its variation sequences (base, then selector, then glyph index) and the widths of their glyphs (keyed by glyph index):

```json
"numGlyphs": 8, "uvs": {"37002": {"917760": 6, "917761": 7}}, "gw": {"6": 1000, "7": 900}
```

A font converted with an earlier version has no such keys and must be converted again to use its variation sequences.

### Font lookup

A font definition named by its family is searched, together with the immediate subdirectories of each directory, in:

1. the `K_PATH_FONTS` directory, when defined;
2. the package input directory (`fonts/`) and the generated font set (`target/fonts/`), when they exist.

A font in `K_PATH_FONTS` takes precedence over a package font with the same name.

Files inside the two package directories are read through a file helper whose allowlist is those directories (`FontPaths::getPackagePaths()`), so a file helper passed to the library does not need to list them. Every other font file is read through the file helper passed to the library and must be inside its allowlist.

`Import` writes to the given output path, or to `K_PATH_FONTS` when no path is given. It throws an exception when neither is a writable directory.

---

## Subset Caching

TrueType font subsetting is computational and memory intensive. When the same
fonts and character sets are embedded repeatedly (for example across many
generated documents), you can inject an optional cache into
`\Com\Tecnick\Pdf\Font\Output` to reuse previously computed subset programs.

Provide any object implementing
`\Com\Tecnick\Pdf\Font\FontSubsetCacheInterface`:

```php
interface FontSubsetCacheInterface
{
    public function get(string $key): ?string;          // null on cache miss
    public function set(string $key, string $subsetFont): void;
}
```

Pass it as the fifth constructor argument:

```php
$output = new \Com\Tecnick\Pdf\Font\Output(
    $fonts,
    $objectNumber,
    $encrypt,
    $fileHelper,   // or null
    $subsetCache,  // your FontSubsetCacheInterface implementation
);
```

When no cache is supplied (the default), every subset is recomputed. The
library never evicts entries: the injected backend owns expiration and size
limits. The cache key accounts for the font program bytes, the cmap-selection
metrics, and the requested subset characters, so distinct inputs never collide.

### Using a PSR-16 cache

No PSR-16 adapter is bundled. A wrapper around any backend (PSR-16/PSR-6,
Symfony Cache, Laravel, Redis, APCu) is a few lines:

```php
use Com\Tecnick\Pdf\Font\FontSubsetCacheInterface;
use Psr\SimpleCache\CacheInterface;

final class Psr16SubsetCache implements FontSubsetCacheInterface
{
    public function __construct(private CacheInterface $cache) {}

    public function get(string $key): ?string
    {
        $value = $this->cache->get($this->normalize($key));

        return \is_string($value) ? $value : null;
    }

    public function set(string $key, string $subsetFont): void
    {
        $this->cache->set($this->normalize($key), $subsetFont);
    }

    /**
     * PSR-16 only guarantees keys matching [A-Za-z0-9_.] up to 64 chars and
     * reserves {}()/\@:, so the raw key is hashed into a legal, fixed-length one.
     */
    private function normalize(string $key): string
    {
        return 'tclpf_' . \sha1($key);
    }
}
```

The raw key of the library is human-readable and works with permissive
backends, while stricter PSR-16 implementations may reject or mangle it.
Hashing discards the readable namespace, so scope the underlying PSR-16 pool to
this library if you share it with other consumers.

---

## PDF/A-1 CIDSet

ISO 19005-1 requires a `/CIDSet` stream in the font descriptor of every subset
CIDFont. Set the `cidset` argument of `Output` to add it to each subset
TrueTypeUnicode font:

```php
$output = new \Com\Tecnick\Pdf\Font\Output(
    $fonts,
    $objectNumber,
    $encrypt,
    cidset: true,
);
```

The stream lists CID 0 and the CIDs the document references. It is omitted by
default, as PDF/A-2 and later require a CIDSet, when present, to list every
glyph of the embedded program.

## Converting Existing Fonts

Use the CLI utilities in `util/` to convert existing font files into the JSON/Z format consumed by this library.

### Convert One or More Fonts

Run `util/convert.php` and pass one or more input files with `--fonts`:

```bash
php util/convert.php \
	--outpath=./target/fonts/custom/ \
	--type=TrueTypeUnicode \
	--flags=32 \
	--encoding_id=10 \
	--fonts=/path/to/MyFont-Regular.ttf,/path/to/MyFont-Bold.ttf
```

The command writes generated font definition files to `--outpath`.

Common options:

- `--type`: Explicit font type (`TrueTypeUnicode`, `TrueType`, `Type1`, `CID0JP`, `CID0KR`, `CID0CS`, `CID0CT`). Leave empty for autodetect.
- `--encoding`: Encoding table (for example `cp1252` for many non-Unicode Type1/Core cases). Omit for Unicode and symbolic fonts.
- `--flags`: PDF descriptor flags. Default is `32` (non-symbolic).
- `--platform_id` and `--encoding_id`: CMAP selection for TrueType Unicode imports (defaults: `3` and `1`). Use `--encoding_id=10` to read the format 12 subtable, which is required for the characters above U+FFFF. It is a safe default: a font without that subtable falls back to the BMP one.
- `--linked`: Link to system font file instead of embedding/copying it (not transportable).

To see full usage help:

```bash
php util/convert.php --help
```

### Bulk Conversion

For batch generation from the mirrored font set, run from a clone of this repository:

```bash
php util/build_fonts.php
```

The script:

1. Installs the library dependencies with `composer install --no-dev` when no Composer autoloader is found.
2. Installs the `util` dependencies, which download the font sources from GitHub.
3. Clears `target/fonts/`.
4. Runs `bulk_convert.php`, which scans the mirror package and writes converted fonts under `target/fonts/`.

It requires PHP, Composer and network access. It does not need make or a POSIX shell, so it also runs on Windows. In a repository clone the same step is available as `composer fonts` and `make fonts`.

When the package is installed as a dependency, run the script from its location in the vendor directory, for example from the project root:

```bash
php vendor/tecnickcom/tc-lib-pdf-font/util/build_fonts.php
```

The fonts are written to `vendor/tecnickcom/tc-lib-pdf-font/target/fonts/`. Composer replaces the package directory on update or reinstall, which removes them, so run the script again afterwards.

Notes:

- `build_fonts.php` takes no options. To write the fonts to another directory, run `php util/bulk_convert.php --outpath=/your/path/` after `build_fonts.php` has installed the `util` dependencies at least once.
- `bulk_convert.php --indir=/your/fonts/` reads the font families from another directory, with one sub directory per family, instead of the mirror package.
- `bulk_convert.php` also attempts OTF conversion via FontForge (`fontforge -script otf2ttf.ff ...`) before import. Without FontForge, OTF files are reported and skipped. The mirrored font set contains no OTF files.

### Font Licences

Each family directory under `target/fonts/` contains:

- `LICENSE`: the full terms of the family source, made of the source `NOTICE` file (when present) followed by the source `LICENSE` file;
- `README`: the source location and an `SPDX-License-Identifier:` line.

| Family | Source | SPDX expression | Font embedding exception |
|---|---|---|---|
| `core` | Adobe Core14 AFM files | `APAFML` | not applicable (metrics only) |
| `pdfa` | tc-font-pdfa (derived from GNU FreeFont) | `GPL-3.0-or-later WITH Font-exception-2.0` | yes |
| `freefont` | GNU FreeFont | `GPL-3.0-or-later WITH Font-exception-2.0` | yes |
| `dejavu` | DejaVu Fonts | `Bitstream-Vera AND LicenseRef-Arev` | not applicable |
| `unifont` | GNU Unifont | `GPL-2.0-or-later WITH Font-exception-2.0 OR OFL-1.1` | yes |
| `cid0` | GNU Unifont | `GPL-2.0-or-later WITH Font-exception-2.0` | yes |

`bulk_convert.php` does not convert a family when its `LICENSE` file is missing or empty, its `NOTICE` file is empty, or the terms lack the phrases expected for that family (for example the embedding exception). It reports the family, converts the remaining ones and exits with code `9`. A family without an entry in the table needs only a non-empty `LICENSE` file, and its `README` states `NOASSERTION`.

The RPM and DEB font packages (`make rpm_fonts`, `make deb_fonts`) take their licence field from the `README` of each family, and the build stops when it is missing.

---

## Upgrading from 3.x to 4.0

Version 4.0 changes the character codes emitted for `TrueTypeUnicode` fonts from Unicode codepoints to glyph indices, as described in [Character Encoding of Composite Fonts](#character-encoding-of-composite-fonts).

A consumer of this library must be updated together with it:

- Encode composite text with `Stack::ordArrToGidStr()` instead of writing UTF-16BE codepoints, and make the text object select the font the string was encoded with. A consumer that keeps writing codepoints produces PDFs whose text renders as unrelated glyphs, with no error raised.
- A `TrueTypeUnicode` font emits one PDF object less, since the `CIDToGIDMap` stream is not embedded any more. Baselines that compare PDF output byte by byte, or that assert on object numbers, must be regenerated.
- The `TFontData` shape gained the `ctgu`, `gidenc` and `usedgid` keys. Code that builds that array must add them.

Unchanged:

- No method was removed or changed signature. The new members are additions.
- Font definition files and `.ctg.z` artifacts generated by 3.x keep working for the whole Basic Multilingual Plane. Regenerate a font only to gain the characters above U+FFFF, with `--encoding_id=10`.
- The output of `Core`, `TrueType`, `Type1` and `cidfont0` fonts is unchanged.

---

## Development

```bash
make deps
make fonts
make help
make qa
```

`make fonts` runs the [Bulk Conversion](#bulk-conversion). The unit tests require the generated fonts.

---

## Packaging

```bash
make rpm
make deb
```

For system packages, bootstrap with:

```php
require_once '/usr/share/php/Com/Tecnick/Pdf/Font/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

