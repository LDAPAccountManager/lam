# Security Policy

This document describes the security policy for **tc-lib-file**.

---

## Supported Versions

Security fixes are applied only to the **latest stable release** on the `main` branch.

We strongly recommend always running the latest release.

---

## Reporting a Vulnerability

**Please do not open a public GitHub issue for security vulnerabilities.**

If you discover a security vulnerability, or suspect one, follow responsible disclosure:

1. **Email** the maintainer directly at **[info@tecnick.com](mailto:info@tecnick.com)** with the subject line:  
   `[SECURITY] tc-lib-file: <brief description>`
2. Include as much detail as possible (see [What to include](#what-to-include) below).
3. You will receive an acknowledgement as soon as possible.
4. We will work on a fix or mitigation as promptly as the complexity of the issue allows.

If you do not receive a timely response, please follow up by replying to the same email thread.

---

## What to Include

A high-quality report helps us triage and fix issues faster. Please provide:

- **Description**: a clear summary of the vulnerability and its potential impact.
- **Affected component**: which class, method, or feature is involved.
- **Steps to reproduce**: a minimal, self-contained PHP script or unit test that demonstrates the issue.
- **Expected vs. actual behaviour**: what you expected to happen and what actually happened.
- **Environment**: PHP version, OS, library version (output of `composer show tecnickcom/tc-lib-file`).
- **CVE / CWE reference** (optional): if you have already identified a relevant classification.
- **Suggested fix** (optional): a patch or proposed mitigation if you have one.

---

## Security Best Practices for Integrators

Integrators are responsible for sanitising input **before** passing it to the library. We recommend:

- **Validate and sanitise all user-supplied data**. Use a dedicated sanitiser when accepting content from end users.
- **Keep dependencies up to date.** Run `composer update` regularly and monitor advisories via [Packagist Security Advisories](https://packagist.org/packages/tecnickcom/tc-lib-file) or tools such as `composer audit`.
- **Pin versions in production.** Use `composer.lock` and review changes on every update.

### Required Runtime Configuration

By design, `Com\Tecnick\File\File` starts in a restrictive mode:

- `allowedHosts` defaults to `[]` (no host trusted).
- `allowedPaths` defaults to `[]` (no local path trusted).

You must explicitly define trusted values before using remote URL reads or local path reads in production.

```php
$file = new \Com\Tecnick\File\File(
   allowedHosts: ['example.com', 'cdn.example.com'],
   allowedPaths: ['/srv/my-app/files'],
);
```

Avoid wildcard trust (`'*'`) unless you have a tightly controlled environment and fully trusted inputs.

### Remote Read Size Limit (`maxRemoteSize`)

A remote read is buffered in memory, so it is bounded by `maxRemoteSize`, in bytes.
It defaults to 52428800 (50 MB) and must be positive; a non-positive value is
rejected with `\Com\Tecnick\File\Exception`, because it would abort every transfer.

```php
$file = new \Com\Tecnick\File\File(allowedHosts: ['example.com'], maxRemoteSize: 5 * 1024 * 1024);
$file->setMaxRemoteSize(10 * 1024 * 1024);   // or later; returns $this
```

The limit bounds the bytes that reach PHP memory, not the bytes received. It is
enforced from a cURL write callback, which sees each chunk after any content decoding,
so a compressed response is measured by what it inflates to. A response of exactly
`maxRemoteSize` bytes is accepted; the first byte beyond it aborts the transfer, and the
chunk carrying it is never buffered.

A response that declares a size over the limit is refused earlier still, from the
progress callback, before any of its body is read. Either way the transfer raises
`\Com\Tecnick\File\Exception`, it does not return `false`; the message says which of the
two guards acted.

Set this to the largest response your application legitimately expects. The default is
generous for a document-oriented workload.

### Redirect Policy (`CURLOPT_MAXREDIRS`)

Redirect processing is controlled by cURL options:

- `CURLOPT_MAXREDIRS => 0` (the default) makes libcurl refuse every redirect. A 3xx
  response is reported as an unreadable URL, so there is no unvalidated hop to guard.
- `CURLOPT_MAXREDIRS > 0` enables redirect handling, and every `Location` target is
  validated against `allowedHosts` before it is followed.

A `Location` header on a response that is not a 3xx is ignored, because libcurl never
acts on it there.

If you enable redirects, ensure every possible redirect target host is present in `allowedHosts`.

```php
$file = new \Com\Tecnick\File\File(
   allowedHosts: ['example.com', 'downloads.example.com'],
   curlopts: [
      CURLOPT_MAXREDIRS => 5,
   ],
);
```

### The cURL Option Surface Is Trusted Input

`setCurlOpts()` and the `curlopts` / `defaultCurlOpts` / `fixedCurlOpts` constructor
arguments are part of your application's trust boundary. The fixed options
(`CURLOPT_RETURNTRANSFER`, `CURLOPT_FAILONERROR`, `CURLOPT_SSL_VERIFYPEER`,
`CURLOPT_SSL_VERIFYHOST`) are applied last and cannot be weakened through them, and the
request URL, the size guards and the redirect validation callback are all installed after
the merge, so they cannot be displaced either.

Several other libcurl options nevertheless steer the connection independently of the URL
that `allowedHosts` validated, and they are not restricted:

- `CURLOPT_RESOLVE` and `CURLOPT_CONNECT_TO` pin a hostname to an address of your
  choosing, so an allowlisted host can be made to resolve anywhere.
- `CURLOPT_UNIX_SOCKET_PATH` sends the request to a local socket instead.
- `CURLOPT_PROXY` routes it through a proxy.
- `CURLOPT_CAINFO` and `CURLOPT_CAPATH` change the trust anchors TLS verification uses.

None of these is a vulnerability, because the values come from your application. But if
any part of that input can be influenced by untrusted data, `allowedHosts` no longer
constrains where a request goes. Keep cURL options in code or in trusted configuration.

Note also that the library reserves the transfer callbacks it relies on:
`CURLOPT_WRITEFUNCTION`, `CURLOPT_PROGRESSFUNCTION`, `CURLOPT_XFERINFOFUNCTION`,
`CURLOPT_NOPROGRESS` and, when redirects are enabled, `CURLOPT_HEADERFUNCTION`. Values
supplied for those are replaced without warning.

---

## Contact

| Channel | Details |
|---------|---------|
| Security email | [info@tecnick.com](mailto:info@tecnick.com) |
| Project website | <https://tcpdf.org> |
| GitHub repository | <https://github.com/tecnickcom/tc-lib-file> |
| Packagist | <https://packagist.org/packages/tecnickcom/tc-lib-file> |
