# tc-lib-unicode-data

> Unicode data tables and constants used by the Tecnick text stack.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-unicode-data/version)](https://packagist.org/packages/tecnickcom/tc-lib-unicode-data)
[![Build](https://github.com/tecnickcom/tc-lib-unicode-data/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-unicode-data/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-unicode-data/graph/badge.svg?token=12SAG9XRFK)](https://codecov.io/gh/tecnickcom/tc-lib-unicode-data)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-unicode-data/license)](https://packagist.org/packages/tecnickcom/tc-lib-unicode-data)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-unicode-data/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-unicode-data)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-unicode-data` is a data-centric package that provides Unicode lookup tables, mappings, and constants consumed by `tc-lib-unicode` and related libraries.

It externalizes large Unicode datasets into a dedicated package so runtime libraries can stay focused on algorithms instead of data distribution. Versioned data updates also become easier to manage and review as Unicode standards evolve.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Unicode\Data` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-unicode-data> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-unicode-data> |

---

## Features

### Data Coverage
- Unicode property and identity constants
- Script/category mapping data
- Bracket, mirroring, and shaping-related tables

### Integration Role
- Runtime dependency for higher-level Unicode processing
- Pure data distribution, no heavy runtime logic
- Deterministic, versioned updates

---

## Requirements

- PHP 8.2 or later
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-unicode-data
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

echo md5(\Com\Tecnick\Unicode\Data\Identity::CIDHMAP);
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
require_once '/usr/share/php/Com/Tecnick/Unicode/Data/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

