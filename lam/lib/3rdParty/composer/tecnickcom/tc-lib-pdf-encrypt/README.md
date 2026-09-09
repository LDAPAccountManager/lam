# tc-lib-pdf-encrypt

> PHP library to encrypt and decrypt PDF data.

[![Latest Stable Version](https://poser.pugx.org/tecnickcom/tc-lib-pdf-encrypt/version)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-encrypt)
[![Build](https://github.com/tecnickcom/tc-lib-pdf-encrypt/actions/workflows/check.yml/badge.svg)](https://github.com/tecnickcom/tc-lib-pdf-encrypt/actions/workflows/check.yml)
[![Coverage](https://codecov.io/gh/tecnickcom/tc-lib-pdf-encrypt/graph/badge.svg?token=Pv1MNH3X3v)](https://codecov.io/gh/tecnickcom/tc-lib-pdf-encrypt)
[![License](https://poser.pugx.org/tecnickcom/tc-lib-pdf-encrypt/license)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-encrypt)
[![Downloads](https://poser.pugx.org/tecnickcom/tc-lib-pdf-encrypt/downloads)](https://packagist.org/packages/tecnickcom/tc-lib-pdf-encrypt)

[![Sponsor on GitHub](https://img.shields.io/badge/sponsor-github-EA4AAA.svg?logo=githubsponsors&logoColor=white)](https://github.com/sponsors/tecnickcom)

> 💖 Part of the [tc-lib-pdf / TCPDF](https://github.com/tecnickcom/tc-lib-pdf) ecosystem (100M+ installs). [Sponsor its maintenance →](https://github.com/sponsors/tecnickcom)

---

## Overview

`tc-lib-pdf-encrypt` implements the encryption and decryption routines of the PDF format: password and certificate handling, key derivation, permission flags and the encryption dictionary.

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
> RC4-40 (mode 0) and RC4-128 (mode 1) both emit an `E_USER_DEPRECATED` notice at runtime.
> **Use AES-128 (mode 2), AES-256 R5 (mode 3), or AES-256 R6 / PDF 2.0 (mode 4) for all new documents.**
>
> Mode 0 is the constructor default: pass `mode` explicitly.
>
> RC4 is also two orders of magnitude slower, because OpenSSL 3 no longer offers
> the cipher and the bundled implementation runs in PHP. Measured on one 1 MiB
> stream: RC4-128 3 MiB/s, AES-128 940 MiB/s, AES-256 1000 MiB/s.

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
- RC4 and AES variants for PDF object/string encryption (modes 0 to 4; see Security Notice above)
- AES-256 R6 (PDF 2.0 / ISO 32000-2, mode 4) support with Algorithm 2.B (ISO 32000-2 §7.6.4.3.4) key derivation
- User and owner password workflows
- Permission flag handling for document operations
- Optional metadata encryption control (`$encryptMetadata`, requires mode 2, 3 or 4)
- Embedded-file crypt filter selection (`$encryptEmbeddedFiles`, `/EFF` dictionary entry, V 4 and V 5
  only). False writes `/EFF /Identity`, and the caller must then write those streams without calling
  `encryptString()` on them. Reader support varies: qpdf 12 reports the entry under
  `--show-encryption` but still applies `/StmF` when it extracts an attachment.
- Public-key (certificate) encryption for multiple recipients

### Decryption
- Password authentication for all encryption modes (RC4-40, RC4-128, AES-128, AES-256 R5/R6)
- Public-key (PKCS#7 / S/MIME) decryption for recipient private keys
- Per-object key derivation for RC4 and AES-128 streams, with object generation numbers
- `Decrypt::fromEncryptionDictionary()` maps `/V`, `/R` and `/CFM` to the algorithm, and
  recognises the public-key handler, which carries no `/R`, from `/Filter` or `/Recipients`
- Round-trip `decryptString()` companion to `encryptString()`
- `/Perms` verification for AES-256: a document whose permission bits were
  rewritten fails authentication
- `getAuthenticatedRole()` reports whether the user or the owner password matched.
  The owner password is tried first, so a document that uses one string for both
  is reported as the owner
- `getRecipientPermissions()` returns the permission bits of the matching recipient
  in public-key mode, where the document carries no `/P` entry

### Interoperability
Encryption dictionaries are verified against [qpdf](https://qpdf.sourceforge.io/)
fixtures and against vectors computed from ISO 32000, in both directions, for
every revision from R2 to R6.

Passwords are used as supplied. ISO 32000-2 calls for SASLprep (RFC 4013) on
R5/R6 passwords and ISO 32000-1 expects PDFDocEncoding for R2 to R4; neither is
applied, so non-ASCII passwords may not match other implementations. ASCII
passwords are unaffected.

### Integration
- Helpers for PDF date formatting and hexadecimal/string conversion
- Exception-driven error handling

---

## Requirements

- PHP 8.2 or later
- Extensions: `ctype`, `hash`, `openssl`, `pcre`
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
    // permissions lists what to BLOCK: this document cannot be printed or copied
    permissions: ['print', 'copy'],
    user_pass: 'userpassword',
    owner_pass: 'ownerpassword',
);

$cipher = $encrypt->encryptString('secret payload', $objectNumber = 1);
echo bin2hex($cipher);

// The first element of the trailer /ID array must carry this value:
// revisions 2 to 4 derive the key from it.
echo $encrypt->getFileId();
```

### Decrypting a string

```php
<?php

require_once __DIR__ . '/vendor/autoload.php';

// Pass the encryption dictionary produced by the Encrypt instance.
$decrypt = new \Com\Tecnick\Pdf\Encrypt\Decrypt($encrypt->getEncryptionData());

if ($decrypt->authenticate('userpassword')) {
    echo $decrypt->getAuthenticatedRole(); // 'user', 'owner' or 'recipient'

    // For AES modes the PKCS#7 padding is stripped, so the exact original
    // plaintext is returned; RC4 modes are symmetric. Throws when called
    // without a successful authenticate() or on malformed data.
    $plain = $decrypt->decryptString($cipher, $objectNumber = 1, $generationNumber = 0);
    echo $plain;
}
```

### Decrypting a document written elsewhere

Pass the raw dictionary entries and let the library derive the mode from `/V`,
`/R` and `/CFM`:

```php
$decrypt = \Com\Tecnick\Pdf\Encrypt\Decrypt::fromEncryptionDictionary([
    'V' => 5,
    'R' => 6,
    'O' => $oBytes,      // raw binary, not hexadecimal
    'U' => $uBytes,
    'OE' => $oeBytes,
    'UE' => $ueBytes,
    'Perms' => $permsBytes,
    'P' => -3904,
    'fileid' => $firstTrailerIdElement,
]);
```

`/Length` may be omitted when `/V` implies it. Every required entry is checked
for presence and type, so a malformed dictionary raises
`Com\Tecnick\Pdf\Encrypt\Exception`.

### OpenSSL Note

RC4 is applied by the bundled implementation rather than OpenSSL, so no legacy
provider is required on OpenSSL 3. AES uses OpenSSL.

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

