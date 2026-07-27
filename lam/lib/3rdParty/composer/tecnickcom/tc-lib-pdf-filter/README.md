# tc-lib-pdf-filter

> Decoder library for standard PDF stream filters.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-filter/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-filter/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-filter/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-filter/graph/badge.svg?token=23KB9T46HA)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-filter)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-filter/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-filter/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-filter` decodes compression and transformation filters defined by the PDF specification.

It is intended for parsing workflows where encoded PDF streams must be decoded according to the standard filter pipeline. By isolating filter logic in one component, callers get predictable behavior and easier testing across different document inputs.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Filter` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-filter> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-filter> |

---

## Features

### PDF Filters
- `FlateDecode`, `LZWDecode`, `RunLengthDecode`
- `ASCIIHexDecode`, `ASCII85Decode`
- `CCITTFaxDecode`, `DCTDecode`, `JPXDecode`, `JBIG2Decode`

### API Design
- Decode one filter or apply multiple filters in sequence
- Pure-PHP implementation suitable for parser integration
- Typed exceptions for unknown/invalid filter handling

---

## Requirements

- PHP 8.2 or later
- Extension: `zlib`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-filter
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$filter = new \Com\Tecnick\Pdf\Filter\Filter();
$decoded = $filter->decodeAll(['ASCIIHexDecode', 'FlateDecode'], $data);
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
require_once '/usr/share/php/Com/Tecnick/Pdf/Filter/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

