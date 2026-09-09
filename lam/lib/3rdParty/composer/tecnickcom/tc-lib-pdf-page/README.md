# tc-lib-pdf-page

> PHP library to define and render PDF pages.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-page/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-page)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-page/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-page/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-page/graph/badge.svg?token=F6CPFHI3ED)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-page)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-page/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-page)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-page/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-page)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-page` keeps the page stack of a PDF document and outputs the page
tree, the page objects and the content streams.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Page` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-page> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-page> |

---

## Features

### Page geometry
- Named page formats (ISO, ANSI, JIS, envelopes, photographic and other series) and custom sizes
- Portrait, landscape and size-derived orientation
- Conversion between points, millimeters, centimeters and inches
- MediaBox, CropBox, BleedBox, TrimBox and ArtBox, each with its BoxColorInfo guidelines
- Page, header, content and footer margins, with mirrored gutters in booklet mode
- Writable regions: equal columns, or rectangles computed around no-write areas

### Page stack
- Add, clone, move, delete and select pages, grouped for per-group numbering
- Page content stack with marks, and `~#PN` / `~#PT` page number aliases
- Annotation references, rotation, preferred zoom and page transitions
- Per-page transparency group policy, disabled in PDF/A mode
- PDF output with optional Flate compression and encryption

### Enums
- `Unit`, `Orientation`, `PageBoxType`, `PageLayout`, `PageDisplayMode`, `TransparencyGroupMode`

---

## Requirements

- PHP 8.2 or later
- Extension: `zlib`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-page
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$page = new \Com\Tecnick\Pdf\Page\Page(
    'mm',
    new \Com\Tecnick\Color\Pdf(),
    new \Com\Tecnick\Pdf\Encrypt\Encrypt(false),
);

$page->add([
    'format' => 'A4',
    'orientation' => 'P',
    'margin' => ['PL' => 20, 'PR' => 20, 'PT' => 15, 'PB' => 15],
    'columns' => 2,
]);

$page->addContent('BT /F1 12 Tf 100 700 Td (Page ~#PN of ~#PT) Tj ET');

$pon = 0;
$pdfpages = $page->getPdfPages($pon);
```

`add()` returns the sanitized page data, including the page boxes in points, the
normalized margins and the writable regions. `getPdfPages()` returns the PDF
objects of the whole stack and advances `$pon` past the last object number used.

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
require_once '/usr/share/php/Com/Tecnick/Pdf/Page/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).
