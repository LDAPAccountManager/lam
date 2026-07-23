#   tc-lib-color

> PHP color toolkit for conversion and normalization across common color models.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-color/version)](https://packagist.org/packages/tecnickcom/tc-lib-color)
[![Build](https://github.com/tecnickcom/tc-lib-color/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-color/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-color/graph/badge.svg?token=l3UCVbShmc)](https://codecov.io/gh/tecnickcom/tc-lib-color)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-color/license)](https://packagist.org/packages/tecnickcom/tc-lib-color)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-color/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-color)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-color` provides utilities for parsing, converting, and formatting color values used in web and PDF rendering pipelines.

The library is designed to centralize color logic so applications avoid ad-hoc conversion code and rounding drift across modules. It provides a consistent normalization layer that helps keep visual output aligned between browser previews and final PDF rendering.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Color` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-color> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-color> |

---

## Features

### Color Models
- Grayscale (GRAY)
- RGB/RGBA and hexadecimal color handling (RGB)
- HSL/HSLA conversion workflows (HSL)
- CMYK conversion workflows (CMYK)
- CIE Lab color handling and conversion workflows (LAB)
- Spot colors (Separation), with DeviceCMYK and Lab alternate color-space support for PDF output

### Integration Helpers
- CSS-ready color output
- PDF-oriented color conversion helpers
- Cross-model conversion helpers on all color models
- Named web color lookup and normalization

---

## Requirements

- PHP 8.2 or later
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-color
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$web = new \Com\Tecnick\Color\Web();
$rgb = $web->getRgbObjFromHex('#336699');

echo $rgb->getCssColor();
```

See `example/index.php` for a complete conversion showcase.

---

## Development

```bash
make deps
make help
make qa
make server
```

Run `make server` and open <http://127.0.0.1:8000/example/index.php> to check the example in a browser.

---

## Packaging

```bash
make rpm
make deb
```

For system packages, bootstrap with:

```php
require_once '/usr/share/php/Com/Tecnick/Color/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

