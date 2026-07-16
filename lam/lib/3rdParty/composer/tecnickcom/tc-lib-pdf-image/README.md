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

`tc-lib-pdf-image` handles image import, conversion, and output structures used by PDF generators.

The library isolates image pipeline concerns such as format handling, normalization, and object generation for PDF embedding. Keeping this logic separate helps reduce complexity in document-level code and makes image behavior easier to validate and maintain.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Image` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-image> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-image> |

---

## Features

### Import Support
- Native handling for PNG and JPEG
- Additional format handling through GD processing paths
- Transparency and palette-related metadata handling

### PDF Integration
- Image caching keys for repeated assets
- Alternate image support for print/display contexts
- Output helpers for embedding image objects

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
$fileHelper = new \Com\Tecnick\File\File();

$img = new \Com\Tecnick\Pdf\Image\Import(
	kunit: 1.0,
	encrypt: $encrypt,
	fileHelper: $fileHelper,
);
$imageId = $img->add('/path/to/image.png');

var_dump($imageId);
```

You can configure the `\Com\Tecnick\File\File` helper with host/path allowlists:

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

Processing an image (decode, resize, re-encode, alpha-mask extraction) is the
expensive part of importing. By default the result is cached **in memory for the
lifetime of the `Import` instance**, so reusing the same image within one
document is cheap.

To reuse processed images **across documents and processes** (e.g. brand assets
reused on thousands of PDFs), inject an optional external cache. The library
ships only the contract — `\Com\Tecnick\Pdf\Image\ImageCacheInterface` — and you
provide the backend (filesystem, APCu, Redis, a PSR-16 cache, ...):

```php
interface ImageCacheInterface
{
	/** @return array|null Stored image data, or null on a miss. */
	public function get(string $key): ?array;

	public function set(string $key, array $data): void;
}
```

Pass an implementation to the constructor (default `null` keeps the current
in-memory-only behaviour):

```php
$img = new \Com\Tecnick\Pdf\Image\Import(
	kunit: 1.0,
	encrypt: $encrypt,
	fileHelper: $fileHelper,
	imageCache: $myCache, // any ImageCacheInterface implementation
);
```

On a miss the processed data is written through to the cache; on a later run a
hit short-circuits all processing. For local files the persistent key folds in
the file modification time and size, so editing an image in place invalidates
its stale entry automatically.

> **Security:** the cache store is a trust boundary. Stored bytes (image data,
> palette, ICC profile) are embedded verbatim into generated PDFs, so use a
> store only your application can write to. If your implementation serializes
> entries, deserialize with object restoration disabled, e.g.
> `unserialize($s, ['allowed_classes' => false])`.

A minimal example implementation:

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

