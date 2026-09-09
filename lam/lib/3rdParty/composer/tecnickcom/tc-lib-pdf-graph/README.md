# tc-lib-pdf-graph

> Geometric drawing and transformation primitives for PDF content streams.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-graph/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-graph)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-graph/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-graph/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-graph/graph/badge.svg?token=LqxfwhPB8G)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-graph)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-graph/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-graph)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-graph/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-graph)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-graph` generates the content stream commands for the graphic and geometric primitives of a PDF document.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Graph` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-graph> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-graph> |

---

## Features

### Drawing Primitives
- Points, lines, rectangles and Bezier curves
- Ellipses, circles, elliptical arcs and pie sectors
- Polygons, regular polygons, star polygons and rounded rectangles
- Arrows, crop marks and registration marks

### Styles and Painting
- Style stack with line width, cap, join, miter limit, dash pattern and colors
- Path-painting and clipping operators
- Transparency, blend modes and overprint through ExtGState objects
- PDF/A mode that suppresses the ExtGState and shading output

### Gradients
- Axial and radial shadings with multiple color stops and per-stop opacity
- Coons patch meshes and color registration bars

### Transformations
- Scaling, rotation, mirroring, reflection, translation and skewing
- Transformation matrix product and transformation stack

---

## Requirements

- PHP 8.2 or later
- Extension: `zlib`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-graph
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$draw = new \Com\Tecnick\Pdf\Graph\Draw(
    1.0,
    210,
    297,
    new \Com\Tecnick\Color\Pdf(),
    new \Com\Tecnick\Pdf\Encrypt\Encrypt(),
    false
);

echo $draw->getClippingRect(10, 10, 50, 20);
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
require_once '/usr/share/php/Com/Tecnick/Pdf/Graph/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

