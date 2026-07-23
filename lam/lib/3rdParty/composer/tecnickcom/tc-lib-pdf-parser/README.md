# tc-lib-pdf-parser

> Parser library for reading and extracting PDF document structures.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-parser/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-parser/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-parser/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-parser/graph/badge.svg?token=SIGYQJG8D4)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-parser)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-parser/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-parser/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-parser` parses raw PDF data into structured PHP arrays suitable for extraction, analysis, and downstream processing.

The parser is designed for tooling scenarios such as content inspection, metadata extraction, validation, and migration pipelines. It favors clear structured output so applications can build higher-level analysis features without depending on fragile regular-expression parsing.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Parser` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-parser> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-parser> |

---

## Features

### Parsing Capabilities
- Cross-reference and object stream parsing
- Filter-aware stream decoding integration
- Structured output suitable for custom extractors

### Runtime Design
- Configuration options for tolerant parsing modes
- Pure-PHP parser with no external service dependency
- Typed exceptions for error handling

---

## Requirements

- PHP 8.2 or later
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-parser
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$raw = file_get_contents('/path/to/document.pdf');
$parser = new \Com\Tecnick\Pdf\Parser\Parser(['ignore_filter_errors' => true]);
$data = $parser->parse((string) $raw);

var_dump($data);
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
require_once '/usr/share/php/Com/Tecnick/Pdf/Parser/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

