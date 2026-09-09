# tc-lib-file

> PHP utilities for low-level file access and byte-level reading.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-file/version)](https://packagist.org/packages/tecnickcom/tc-lib-file)
[![Build](https://github.com/tecnickcom/tc-lib-file/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-file/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-file/graph/badge.svg?token=iZ9snwgkUA)](https://codecov.io/gh/tecnickcom/tc-lib-file)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-file/license)](https://packagist.org/packages/tecnickcom/tc-lib-file)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-file/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-file)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-file` provides primitives for opening files, reading bytes, and caching
temporary files, used by higher-level PDF and document libraries.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\File` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-file> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-file> |

---

## Features

### File Access
- Local and URL-backed file reading helpers
- Path-safety checks for local operations
- cURL-based retrieval options for remote resources

### Binary Utilities
- Byte, integer, and structured binary reads
- Helpers used by parser and image/font import stacks
- Error handling via typed exceptions

### Classes
| Class | Purpose |
|---|---|
| [`File`](src/File.php) | Local and remote reads behind host/path allowlists |
| [`Byte`](src/Byte.php) | Big-endian byte-level reads from a binary string |
| [`Cache`](src/Cache.php) | Prefixed temporary file cache |
| [`Dir`](src/Dir.php) | Writable parent-directory lookup |
| [`Exception`](src/Exception.php) | Library exception type |

---

## Requirements

- PHP 8.2 or later
- Extensions: `curl`, `pcre`
- Optional extension: `intl`, which enables Unicode NFC folding when matching
  paths against the allowlist (without it that folding is a no-op)
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-file
```

---

## Quick Start

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

$file = new \Com\Tecnick\File\File(
	allowedHosts: ['example.com', 'cdn.example.com'],
	allowedPaths: [__DIR__, '/var/app/uploads'],
	curlopts: [
		CURLOPT_MAXREDIRS => 3,
	],
);
$fh = $file->fopenLocal(__FILE__, 'rb');
$header = $file->fReadInt($fh);

var_dump($header);
```

## Security Configuration (Required)

`File` defaults to strict-deny behavior for host and path validation.

- `allowedHosts` defaults to an empty array, so remote URLs and host-based alternate path resolution are rejected unless you explicitly trust hosts.
- `allowedPaths` defaults to an empty array, so local file operations are rejected unless you explicitly trust path prefixes.

You should always pass explicit allowlists in the constructor (or set them immediately via setters) for production use.

Example:

```php
$file = new \Com\Tecnick\File\File(
	allowedHosts: ['example.com'],
	allowedPaths: ['/srv/my-app/data'],
);

// Equivalent runtime configuration:
$file
	->setAllowedHosts(['example.com'])
	->setAllowedPaths(['/srv/my-app/data']);
```

Avoid wildcard trust (`'*'`) unless you fully control all inputs and deployment boundaries.

Allowlist entries are normalized once, when they are set:

- **Hosts** are matched case-insensitively (per RFC 4343) and a trailing root dot is
  ignored, so `example.com`, `EXAMPLE.COM` and `example.com.` are the same entry.
  An entry may take either of two forms:
  - `example.com` constrains the host only and matches it on any port.
  - `example.com:8080` names one origin and matches only that port. This is the form
    a non-default `HTTP_HOST` value needs, and it authorizes
    `https://example.com:8080/...` too. A URL that omits the port is matched against
    the scheme default, so `example.com:443` matches `https://example.com/`.

  An IPv6 host must use the bracketed form (`[::1]`, `[::1]:8080`), and an
  internationalized domain must be listed as its A-label (punycode) form.
- **Paths** are stored in both their literal and canonical (`realpath()`) form, so a
  root that traverses a symlink still matches files inside it. This matters on macOS
  (`/tmp` and `/var` are symlinks into `/private`), in containers, and with
  release-directory symlinks such as `current -> releases/42`. Redundant separators
  and `.` segments are collapsed, so `/srv//data` and `/srv/./data` name the same root.

### Validating without side effects

`isValidURL()` and `isValidFile()` take their argument **by reference** and rewrite it
(trimming, and adding the `file://` scheme), so they only accept a variable. Use the
by-value counterparts for a literal or any other expression:

```php
$file->isAllowedUrl('https://example.com/logo.png'); // bool, argument untouched
$file->isAllowedFile('/srv/my-app/data/report.pdf'); // bool, argument untouched
```

Both validators reject rather than raise for input that no path or URL function
accepts, so they keep their `bool` contract for any string a caller can supply:

- A path containing a NUL byte is invalid. PHP throws a `ValueError` for one in any
  path argument, so `isValidFile()`, `resolveLocalPath()` and every reader that goes
  through them report it as "not valid" / "unreadable" instead.
- A URL containing a C0 control character or DEL is invalid. `parse_url()` tolerates
  CR, LF and TAB inside a URL; they enable response splitting if a validated URL is
  later emitted into a header, so they are rejected here.

### Remote fetching

Remote URLs are always fetched with cURL, so the `allow_url_fopen` ini setting has no
effect on `getUrlData()`, `getFileData()` and `fileGetContents()`. The `curl` extension
is required; `allowedHosts` is the only gate on which hosts can be reached.

A response is buffered in memory and is bounded by `maxRemoteSize` (default 50 MB),
counted as the bytes that reach PHP memory rather than the bytes received, so a
compressed response is measured by what it inflates to; see [SECURITY.md](SECURITY.md)
for the exact semantics. A cURL option that this ext-curl build does not recognize, or
whose value it refuses, is reported as `\Com\Tecnick\File\Exception` rather than as the
`ValueError` `curl_setopt_array()` raises or the silent `false` it returns.

The library reserves the cURL transfer callbacks it relies on: `CURLOPT_WRITEFUNCTION`,
`CURLOPT_PROGRESSFUNCTION`, `CURLOPT_XFERINFOFUNCTION`, `CURLOPT_NOPROGRESS` and, with
redirects enabled, `CURLOPT_HEADERFUNCTION`. Values supplied for those through
`setCurlOpts()` are replaced.

### Redirect Handling via `CURLOPT_MAXREDIRS`

- `CURLOPT_MAXREDIRS => 0` (default): libcurl refuses every redirect, and a 3xx response
  is reported as an unreadable URL.
- `CURLOPT_MAXREDIRS > 0`: redirects are processed and each `Location` target is
  validated against `allowedHosts` before it is followed. A `Location` header on a
  response that is not a 3xx is ignored, since libcurl never acts on it there.

To allow redirects, set a positive max-redirs value and ensure redirect target hosts are present in `allowedHosts`.

A redirect that is not followed is reported as a failure (`false`), never as content.
This matters when `open_basedir` is set, because PHP then leaves `CURLOPT_FOLLOWLOCATION`
off: the body of the `3xx` response would otherwise be returned as the file.

```php
$file = new \Com\Tecnick\File\File(
	allowedHosts: ['example.com', 'downloads.example.com'],
	curlopts: [
		CURLOPT_MAXREDIRS => 5,
	],
);
```

---

## Other classes

### `Byte`: byte-level reads from a binary string

Immutable reader for big-endian values. Every reader validates its bounds and
throws `\RangeException`.

```php
$byte = new \Com\Tecnick\File\Byte($binaryString);

$byte->getLength();     // int: string length in bytes
$byte->getByte(0);      // uint8
$byte->getUShort(0);    // uint16 (alias: getUFWord)
$byte->getShort(0);     //  int16 (alias: getFWord)
$byte->getULong(0);     // uint32
$byte->getLong(0);      //  int32
$byte->getFixed(0);     // float, 16.16 fixed-point
```

The 32-bit readers assume a 64-bit PHP build.

### `Cache`: temporary file cache

Each instance owns a cache directory and a filename prefix. `delete()` only ever
touches files carrying that instance's prefix.

```php
$cache = new \Com\Tecnick\File\Cache('myapp');   // null = random prefix
$cache->setCachePath('/var/cache/myapp');        // falls back to K_PATH_CACHE

$path = $cache->getNewFileName('image', 'logo'); // create a new cache file
file_put_contents($path, $data);

$cache->delete('image', 'logo');                 // one type/key pair
$cache->delete('image');                         // one type
$cache->delete();                                // every file for this prefix
$cache->deleteOlderThan(3600);                   // by age, in seconds

$cache->delete(null, 'logo');                    // throws: a key needs its type
```

A cache file is named prefix + type + key, so a key can only be matched once a type
narrows the name. `delete(null, $key)` is rejected rather than silently treated as
"delete everything for this prefix".

`_` separates those three fields, so it is stripped from a prefix, type or key along
with every other character outside `[A-Za-z0-9-]` (`+` and `/` map to `-`, so a base64
prefix keeps its entropy). Were `_` kept inside a field, `delete('img')` would also
match `img_thumb`, and a `Cache('app')` would delete the files of a `Cache('app_v2')`.
A prefix left empty by that stripping is replaced with a random one, so it can never
collapse to a value every such instance shares.

`deleteOlderThan()` rejects a negative age: it would put the cutoff in the future and
delete every file for the prefix.

The cache directory is `K_PATH_CACHE` when set, otherwise `upload_tmp_dir`, otherwise
the system temp directory. `setCachePath()` falls back to `K_PATH_CACHE` when the
given path is a stream wrapper or is not a writable directory, and throws
`\Com\Tecnick\File\Exception` if no usable directory can be resolved at all. That
fallback is silent, so compare `getCachePath()` against what you passed if you need to
know it happened. `setCachePath()` returns `$this` and can be chained.

`$type` and `$key` scope `delete()`; they do **not** address a file for retrieval. The
generated name carries a random suffix drawn from the system CSPRNG, so keep the value
`getNewFileName()` returned. The suffix also keeps two calls with the same type and key
from colliding; `getNewFileName()` throws rather than return a file outside the
configured cache directory or one that `delete()` could never reclaim.

### `Dir`: writable parent-directory lookup

```php
$dir = (new \Com\Tecnick\File\Dir())->findParentDir('cache', __DIR__);
```

Walks up from `$dir` looking for a **writable directory** named `$name`. A regular file
of that name is skipped, so the returned path is always one a caller can write into.
`$name` must be a plain directory name: an empty, absolute or separator-bearing name,
and one containing a `..` segment, is reported as "not found" rather than resolved,
since the result would not be an ancestor of `$dir`.
Returns the path with a trailing separator, or `''` when there is no match up to the
filesystem root. Paths outside an active `open_basedir` restriction are skipped rather
than probed, so the search raises no warnings.

### `Exception`

`\Com\Tecnick\File\Exception` extends `\Exception`; it is what every documented
`@throws` in the library refers to. `Byte` is the exception: out-of-bounds reads throw
the SPL `\RangeException`.

---

## Platform notes

The library runs on Linux, macOS, and Windows. Path validation adapts to the
host filesystem; a few platform behaviors are worth knowing.

### Path case-sensitivity

Allowlist matching follows the filesystem's case rules:

- **Linux**: case-sensitive.
- **Windows**: case-insensitive.
- **macOS**: per-volume, the default APFS/HFS+ volume is case-insensitive, but
  case-sensitive volumes exist. The library probes the actual volume and falls
  back to case-insensitive when it cannot.

If auto-detection is wrong for your deployment (for example, data on a
case-sensitive macOS volume, or a case-insensitive mount on Linux), set it
explicitly:

```php
$file = new \Com\Tecnick\File\File(
	allowedPaths: ['/srv/my-app/data'],
	caseSensitivePaths: true, // or false; null (default) = auto-detect
);

// or at runtime:
$file->setCaseSensitivePaths(true);
```

The same override makes the behavior testable on any host, independently of the
platform the tests run on.

### Unicode (macOS)

Default macOS volumes are normalization-insensitive (`é` composed vs. decomposed
name the same file). When `ext-intl` is installed, the library normalizes paths
to NFC before comparison so the allowlist matches consistently; without
`ext-intl` it degrades to a byte comparison.

### Binary reads

`fopenLocal()` forces the binary (`b`) stream flag when absent, so byte-level
reads are not altered by Windows text-mode CRLF translation. POSIX systems are
unaffected.

### Windows known limitations

These inputs are intentionally **not** treated as trusted/canonical and are not
specially expanded before allowlist matching: 8.3 short names (`PROGRA~1`),
Alternate Data Streams (`file.txt:stream`, `::$DATA`), trailing dots/spaces, and
reserved device names (`CON`, `NUL`, ...). UNC paths (`\\server\share`) are
matched only when explicitly allowlisted (note the network-access implication).

---

## Development

Every tool is a Composer dev dependency, so the whole gate runs without the Makefile:

```bash
composer install
composer run qa         # fmt-check + cs-check + analyse + test
composer run cs-fix     # format
```

The Makefile wraps the same commands and adds packaging (Linux only):

```bash
make deps
make help
make qa
```

The remote-fetch tests start a local PHP built-in server on a free port. Set
`TC_LIB_FILE_SKIP_HTTP_SERVER=1` to skip them where loopback networking or
`proc_open()` is unavailable; on CI those tests fail rather than skip silently.

---

## Packaging

```bash
make rpm
make deb
```

For system packages, bootstrap with:

```php
require_once '/usr/share/php/Com/Tecnick/File/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

