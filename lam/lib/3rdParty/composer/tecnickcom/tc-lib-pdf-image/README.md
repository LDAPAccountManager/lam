# tc-lib-pdf-image

> Image import and embedding utilities for PDF streams.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-image/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-image)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-image/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-image/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-image/graph/badge.svg?token=7RH3BDHTL2)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-image)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-image/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-image)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-image/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-image)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-image` imports images, converts them to the formats a PDF can embed, and generates the corresponding PDF image objects.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Image` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-image> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-image> |

---

## Features

### Import
- Native PNG and JPEG parsing
- Other formats re-encoded to PNG or JPEG through GD
- Transparency, palette and ICC profile handling

### PDF integration
- Cache keys to reuse repeated images
- Alternate images for print and display contexts
- Output of the image XObjects and of the XObject dictionary

---

## Requirements

- PHP 8.2 or later
- Extensions: `gd`, `zlib`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-image
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();

// the file helper allows nothing by default: list the directories and hosts
// the images may be loaded from
$fileHelper = new \Com\Tecnick\File\File(
	allowedPaths: ['/path/to'],
);

$img = new \Com\Tecnick\Pdf\Image\Import(
	kunit: 1.0,
	encrypt: $encrypt,
	fileHelper: $fileHelper,
);
$imageId = $img->add('/path/to/image.png');

var_dump($imageId);
```

Remote images require a host allowlist as well:

```php
$fileHelper = new \Com\Tecnick\File\File(
	allowedHosts: ['example.com', 'cdn.example.com'],
	allowedPaths: ['/srv/app/images', __DIR__ . '/images'],
);
```

For full file-loading options, see the `tc-lib-file` documentation:
<https://tcpdf.org/docs/srcdoc/tc-lib-file>

---

## Persistent image cache

The processed image data is cached in memory for the lifetime of the `Import`
instance, so the same image imported twice within one document is processed
once.

To reuse processed images across documents and processes, inject an external
cache. The library defines only the contract,
`\Com\Tecnick\Pdf\Image\ImageCacheInterface`, and the backend (filesystem,
APCu, Redis, a PSR-16 cache, ...) is provided by the application:

```php
interface ImageCacheInterface
{
	/** @return array|null Stored image data, or null on a miss. */
	public function get(string $key): ?array;

	public function set(string $key, array $data): void;
}
```

Pass an implementation to the constructor; the default `null` keeps the
in-memory cache only:

```php
$img = new \Com\Tecnick\Pdf\Image\Import(
	kunit: 1.0,
	encrypt: $encrypt,
	fileHelper: $fileHelper,
	imageCache: $myCache, // any ImageCacheInterface implementation
);
```

On a miss the processed data is written through to the cache; a hit skips the
processing. For local files the key includes the file modification time and
size, so editing an image in place invalidates its stale entry.

> **Security:** the cache store is a trust boundary. The stored bytes (image
> data, palette, ICC profile) are embedded verbatim into the generated PDFs, so
> use a store only the application can write to. An implementation that
> serializes entries must deserialize with object restoration disabled, e.g.
> `unserialize($s, ['allowed_classes' => false])`.

An example implementation:

```php
use Com\Tecnick\Pdf\Image\ImageCacheInterface;

final class FilesystemImageCache implements ImageCacheInterface
{
	public function __construct(private readonly string $dir) {}

	public function get(string $key): ?array
	{
		$file = $this->dir . '/' . hash('xxh128', $key) . '.cache';
		if (!is_file($file)) {
			return null;
		}
		$data = unserialize((string) file_get_contents($file), ['allowed_classes' => false]);
		return is_array($data) ? $data : null;
	}

	public function set(string $key, array $data): void
	{
		$file = $this->dir . '/' . hash('xxh128', $key) . '.cache';
		file_put_contents($file, serialize($data), LOCK_EX);
	}
}
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
require_once '/usr/share/php/Com/Tecnick/Pdf/Image/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

