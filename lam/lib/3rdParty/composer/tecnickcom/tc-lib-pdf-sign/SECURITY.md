# Security Policy

This document describes the security policy for **tc-lib-pdf-sign**.

---

## Supported Versions

Security fixes are applied only to the **latest stable release** on the `main` branch.

We strongly recommend always running the latest release.

---

## Reporting a Vulnerability

**Please do not open a public GitHub issue for security vulnerabilities.**

If you discover a security vulnerability, or suspect one, follow responsible disclosure:

1. **Email** the maintainer directly at **[info@tecnick.com](mailto:info@tecnick.com)** with the subject line:  
   `[SECURITY] tc-lib-pdf-sign: <brief description>`
2. Include as much detail as possible (see [What to include](#what-to-include) below).
3. You will receive an acknowledgement as soon as possible.
4. We will work on a fix or mitigation as promptly as the complexity of the issue allows.

If you do not receive a timely response, please follow up by replying to the same email thread.

---

## What to Include

A high-quality report helps us triage and fix issues faster. Please provide:

- **Description**: a clear summary of the vulnerability and its potential impact.
- **Affected component**: which class, method, or feature is involved (e.g. `Cms\SignedDataVerifier::verify()`, OCSP response validation, CRL scope checks).
- **Steps to reproduce**: a minimal, self-contained PHP script or unit test that demonstrates the issue.
- **Expected vs. actual behaviour**: what you expected to happen and what actually happened.
- **Environment**: PHP version, OS, library version (output of `composer show tecnickcom/tc-lib-pdf-sign`).
- **CVE / CWE reference** (optional): if you have already identified a relevant classification.
- **Suggested fix** (optional): a patch or proposed mitigation if you have one.

---

## Security Best Practices for Integrators

This library performs no network access: the TSA, OCSP, and CRL transports are injected by the host. Integrators are therefore responsible for what those callables do. We recommend:

- **Own the SSRF question.** The URLs passed to a transport are read out of certificate extensions the library does not choose, so apply an allow-list, a timeout, and a response size limit in the transport itself.
- **Act on the `$onSkip` observer.** `Signer::collectValidationMaterial()` reports every discarded revocation URL with an `Ltv\SkipReason`; `SkipReason::Revoked` means the certificate should not be signed with.
- **Keep dependencies up to date.** Run `composer update` regularly and monitor advisories via [Packagist Security Advisories](https://packagist.org/packages/tecnickcom/tc-lib-pdf-sign) or tools such as `composer audit`.
- **Pin versions in production.** Use `composer.lock` and review changes on every update.

---

## Contact

| Channel | Details |
|---------|---------|
| Security email | [info@tecnick.com](mailto:info@tecnick.com) |
| Project website | <https://tcpdf.org> |
| GitHub repository | <https://github.com/tecnickcom/tc-lib-pdf-sign> |
| Packagist | <https://packagist.org/packages/tecnickcom/tc-lib-pdf-sign> |
