# tc-lib-pdf-page

> Page geometry, boxes, and format definitions for PDF documents.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-page/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-page)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-page/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-page/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-page/graph/badge.svg?token=F6CPFHI3ED)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-page)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-page/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-page)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-page/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-page)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-page` manages page sizing, orientation, box definitions, and related geometry metadata.

It provides the structural model that document builders use to define media boxes, orientation changes, and page-level defaults consistently. Centralizing these rules improves correctness in multi-page layouts and simplifies downstream rendering code.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Page` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-page> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-page> |

---

## Features

### Page Model
- Standard and custom page size handling
- Orientation and unit conversion helpers
- Region and box definitions (CropBox, TrimBox, and related metadata)

### Integration
- Supports PDF composition stacks that need deterministic page geometry
- Pairs with color/encryption libraries for complete page objects
- Typed exceptions for invalid layout parameters

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
    false,
    false
);

$dims = $page->setBox([], 'CropBox', 0, 0, 210, 297);
var_dump($dims['CropBox']);
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
require_once '/usr/share/php/Com/Tecnick/Pdf/Page/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

