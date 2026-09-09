#   tc-lib-color

> PHP library to manipulate various color representations.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-color/version)](https://packagist.org/packages/tecnickcom/tc-lib-color)
[![Build](https://github.com/tecnickcom/tc-lib-color/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-color/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-color/graph/badge.svg?token=l3UCVbShmc)](https://codecov.io/gh/tecnickcom/tc-lib-color)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-color/license)](https://packagist.org/packages/tecnickcom/tc-lib-color)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-color/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-color)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-color` parses, converts and formats the color values used in web and PDF output.

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
- RGB/RGBA, including hexadecimal notations (RGB)
- HSL/HSLA (HSL)
- CMYK/CMYKA (CMYK)
- CIE Lab (LAB)
- Spot colors (Separation), with DeviceCMYK and Lab alternate color spaces for PDF output

### Integration Helpers
- CSS output that parses back to the same color
- PDF and Acrobat JavaScript color output
- Cross-model conversion on all color models
- Named web color lookup (CSS Color Module Level 4 names)
- `ColorModelType` enum and `Model::create()` factory

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

echo $rgb->getCssColor();      // rgb(51,102,153)
echo $rgb->getRgbHexColor();   // #336699
echo $rgb->getPdfColor();      // "0.200000 0.400000 0.600000 rg\n"
```

Any supported notation can be parsed through the same entry point:

```php
$web->getColorObj('rgb(51 102 153)');
$web->getColorObj('hsl(210,50%,40%)');
$web->getColorObj('cmyk(67%,33%,0%,40%)');
$web->getColorObj('lab(41% -2 -25)');
$web->getColorObj('steelblue');
```

`getCssColor()` emits a value that `getColorObj()` parses back to the same color.

### Supported color notations

| Notation | Examples |
|---|---|
| Hexadecimal | `#RGB`, `#RGBA`, `#RRGGBB`, `#RRGGBBAA` |
| Name | `steelblue`, `color.steelblue`, `transparent` |
| Gray | `g(128)`, `g(50%)` |
| RGB | `rgb(51,102,153)`, `rgb(20% 40% 60%)`, `rgba(51,102,153,0.85)`, `rgb(51 102 153 / 85%)` |
| HSL | `hsl(210,50%,40%)`, `hsl(210deg 50% 40%)`, `hsla(210,50%,40%,0.85)` |
| CMYK | `cmyk(67%,33%,0%,40%)`, `cmyka(67,33,0,40,0.85)` |
| CIE Lab | `lab(41% -2 -25)`, `lab(41 -2 -25 / 0.85)` |
| Acrobat JavaScript | `["T"]`, `["G",0.5]`, `["RGB",0.2,0.4,0.6]`, `["CMYK",0.67,0.33,0,0.4]` |

Components are separated either by commas or by spaces, not by a mixture of the
two, and the alpha channel by a comma in the first form or by a slash in the
second. A hue accepts the CSS angle units (`deg`, `grad`, `rad`, `turn`).
Out-of-range values are clamped and a hue wraps, as CSS Color Level 4 requires,
so `rgb(-10,0,0)` is black and `hsl(-150,50%,50%)` is the same as
`hsl(210,50%,50%)`. Anything else is rejected with a
`Com\Tecnick\Color\Exception`.

`g()`, `cmyk()` and `cmyka()` are notations of this library, not CSS functions,
and round-trip through `getCssColor()` and `getColorObj()` like the others.

Models can also be built directly by type:

```php
use Com\Tecnick\Color\ColorModelType;
use Com\Tecnick\Color\Model;

$cmyk = Model::create(ColorModelType::Cmyk, [
    'cyan' => 0.67,
    'magenta' => 0.33,
    'yellow' => 0.0,
    'key' => 0.4,
    'alpha' => 1.0,
]);
```

### Nearest named color

```php
$web = new \Com\Tecnick\Color\Web();

$web->getClosestWebColorFromString('#9577a6');          // 'lightslategray'
$web->getClosestWebColorByDeltaEFromString('#9577a6');  // 'plum'
```

`getClosestWebColor()` measures the Euclidean distance in sRGB, which is not
perceptually uniform. `getClosestWebColorByDeltaE()` measures the CIE76
difference in CIE Lab, which tracks perceived difference.

### PDF and spot colors

```php
$pdf = new \Com\Tecnick\Color\Pdf();

$pdf->getPdfFillColor('steelblue');       // "0.274510 0.509804 0.705882 rg\n"
$pdf->addSpotColorFromArray('My Spot', ['cyan' => 1]);
$pon = 0;
$objects = $pdf->getPdfSpotObjects($pon); // write these first
$resources = $pdf->getPdfSpotResources(); // then reference them
```

Eight of the eleven default spot color names are also CSS color names (`red`,
`green`, `blue`, `cyan`, `magenta`, `yellow`, `black`, `white`) and resolve to
the spot color. They agree with their CSS namesake except `green`: the spot
Green is `#00ff00` while CSS green is `#008000`. Pass `$allowSpot = false` to
`getPdfColor()`, `getPdfFillColor()`, `getPdfStrokeColor()`, `getColorObject()`,
`getPdfRgbComponents()` or `getPdfCmykComponents()` to force the device color.

The remaining three, `key`, `all` and `none`, are spot-only names. With
`$allowSpot = false` they do not resolve: `getPdfColor()` and the component
accessors return an empty string and `getColorObject()` returns `null`.

Every registered spot color must be emitted by `getPdfSpotObjects()` before
`getPdfSpotResources()` is called; otherwise the resource writers raise a
`Com\Tecnick\Color\Exception`.

### Exceptions

`Com\Tecnick\Color\Exception` (a `\Exception`) signals an invalid input.
`Com\Tecnick\Color\UnknownComponentException` (a `\LogicException`) signals a
component name a model does not define, and is not swallowed by the lenient
accessors `tryGetColorObj()` and `getColorObject()`. Both implement
`Com\Tecnick\Color\ExceptionInterface`, so a single `catch` covers them.

See `example/index.php` for a web-color conversion table covering RGB, HSL, the
Acrobat JavaScript form and the nearest-color inversion.

### Roles and extension points

`Pdf` extends `Spot` extends `Web` extends `Css`, so one object offers the
parser, the spot registry and the PDF writer. Each role also has its own
interface to type against: `ColorParserInterface`, `SpotRegistryInterface` and
`PdfColorWriterInterface`.

Four methods can be overridden: `Web::getColorObj()`, `Spot::getSpotColor()`,
`Pdf::getPdfColor()` and `Pdf::getColorObject()`, plus the protected parser
methods on `Css` and `Spot::resolveSpotColorData()`. Every other public method
is `final`. A subclass that overrides one of the four must mirror its signature
exactly.

---

## Versioning

This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
A change that alters a public signature, the bytes a method emits, or whether an
input parses, goes in a major release. See
[CHANGELOG.md](https://github.com/tecnickcom/tc-lib-color/blob/main/CHANGELOG.md).

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
require_once '/usr/share/php/Com/Tecnick/Color/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](https://github.com/tecnickcom/tc-lib-color/blob/main/CONTRIBUTING.md), [CODE_OF_CONDUCT.md](https://github.com/tecnickcom/tc-lib-color/blob/main/CODE_OF_CONDUCT.md), and [SECURITY.md](https://github.com/tecnickcom/tc-lib-color/blob/main/SECURITY.md).

