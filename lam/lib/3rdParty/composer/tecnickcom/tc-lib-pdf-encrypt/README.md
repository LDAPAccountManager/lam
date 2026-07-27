# tc-lib-pdf-encrypt

> PDF encryption primitives for password protection and permission control.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-encrypt/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-encrypt)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-encrypt/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-encrypt/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-encrypt/graph/badge.svg?token=Pv1MNH3X3v)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-encrypt)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-encrypt/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-encrypt)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-encrypt/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-encrypt)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-encrypt` implements core encryption routines used by PDF generation and processing stacks, including password handling and permission flags.

The package encapsulates PDF security mechanics behind a focused API so consuming libraries can apply encryption policies without reimplementing cryptographic details. It is built for interoperability with standard PDF readers and for clear separation between document logic and security concerns.

| | |
|---|---|
| **Namespace** | `\Com\Tecnick\Pdf\Encrypt` |
| **Author** | Nicola Asuni <info@tecnick.com> |
| **License** | [GNU LGPL v3](https://www.gnu.org/copyleft/lesser.html) - see [LICENSE](LICENSE) |
| **API docs** | <https://tcpdf.org/docs/srcdoc/tc-lib-pdf-encrypt> |
| **Packagist** | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-encrypt> |

---

## Security Notice

> **RC4 modes (0 and 1) are cryptographically broken and deprecated.**
> RC4-40 (mode 0) and RC4-128 (mode 1) are no longer considered secure.
> Both modes emit an `E_USER_DEPRECATED` notice at runtime.
> **Use AES-128 (mode 2), AES-256 R5 (mode 3), or AES-256 R6 / PDF 2.0 (mode 4) for all new documents.**

| Mode | Algorithm | Security |
|------|-----------|----------|
| 0    | RC4-40    | **Broken: do not use** |
| 1    | RC4-128   | **Broken: do not use** |
| 2    | AES-128   | Acceptable for legacy compatibility |
| 3    | AES-256 R5 (PDF 1.7 ext.) | Recommended |
| 4    | AES-256 R6 (PDF 2.0 / ISO 32000-2) | Recommended (most current) |

---

## Features

### Encryption
- RC4 and AES variants for PDF object/string encryption (modes 0–4; see Security Notice above)
- AES-256 R6 (PDF 2.0 / ISO 32000-2, mode 4) support with Algorithm 2.B (ISO 32000-2 §7.6.4.3.4) key derivation
- User and owner password workflows
- Permission flag handling for document operations
- Optional metadata encryption control (`$encryptMetadata`)
- Optional embedded-file stream encryption (`$encryptEmbeddedFiles`, `/EFF` dictionary entry)
- Public-key (certificate) encryption for multiple recipients

### Decryption
- Password authentication for all encryption modes (RC4-40, RC4-128, AES-128, AES-256 R5/R6)
- Public-key (PKCS#7 / S/MIME) decryption for recipient private keys
- Per-object key derivation for AES-128 streams
- Round-trip `decryptString()` companion to `encryptString()`

### Integration
- Designed for direct use by PDF writer and reader components
- Helpers for PDF date formatting and hex/string transforms
- Exception-driven error handling

---

## Requirements

- PHP 8.2 or later
- Extensions: `hash`, `openssl`
- Composer

---

## Installation

```bash
composer require tecnickcom/tc-lib-pdf-encrypt
```

---

## Quick Start

### Encrypting a string

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

// AES-256 R6 (mode 4, recommended)
$encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
    enabled: true,
    file_id: md5('unique-file-id'),
    mode: 4,
    permissions: ['print', 'copy'],
    user_pass: 'userpassword',
    owner_pass: 'ownerpassword',
);

$cipher = $encrypt->encryptString('secret payload', $objectNumber = 1);
echo bin2hex($cipher);
```

### Decrypting a string

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

// Pass the encryption dictionary produced by the Encrypt instance.
$decrypt = new \Com\Tecnick\Pdf\Encrypt\Decrypt($encrypt->getEncryptionData());

if ($decrypt->authenticate('userpassword')) {
    // For AES modes the PKCS#7 padding is stripped automatically, so the
    // exact original plaintext is returned. RC4 modes are symmetric.
    $plain = $decrypt->decryptString($cipher, $objectNumber = 1);
    echo $plain;
}
```

### OpenSSL 3 Note

On OpenSSL 3 systems, legacy providers may be disabled by default. Enable legacy support when required by your runtime policy.

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
require_once '/usr/share/php/Com/Tecnick/Pdf/Encrypt/autoload.php';
```

---

## Contributing

Contributions are welcome. Please review [CONTRIBUTING.md](CONTRIBUTING.md), [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), and [SECURITY.md](SECURITY.md).

