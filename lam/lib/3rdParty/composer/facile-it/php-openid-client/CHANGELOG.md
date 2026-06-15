# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Release notes are also published on [GitHub Releases](https://github.com/facile-it/php-openid-client/releases).

## [1.0.0] - 2026-06-12

### Changed
- Drop support for PHP 8.1
- [BC] Upgrade to `web-token/jwt-library` 4.x - since this contains breaking changes, refer to the [upgrade guide](https://web-token.spomky-labs.com/migration/from-v3.x-to-v4.0)

## [0.4.0] - 2026-01-02

### Changed

- Drop support for PHP < 8.1
- Updated dependencies
- Replace abandoned packages ([#47](https://github.com/facile-it/php-openid-client/pull/47))

## [0.4.0-beta1] - 2025-05-30

### Changed

- Replaced abandoned web-token packages
- Dropped support for PHP 7.4

## [0.3.5] - 2023-11-21

### Fixed

- Fix unable to set client JWKS ([#32](https://github.com/facile-it/php-openid-client/pull/32))

## [0.3.4] - 2023-07-28

### Changed

- Allow PSR-7 v2

## [0.3.3] - 2023-02-21

### Added

- Add support for authorization endpoints containing query parameters ([#26](https://github.com/facile-it/php-openid-client/pull/26)) — thanks [@antonioeatgoat](https://github.com/antonioeatgoat)

## [0.3.2] - 2023-02-06

### Fixed

- Fix JWT `aud` claim for `client_secret_jwt` and `private_key_jwt` authentication methods

## [0.3.1] - 2022-09-27

### Fixed

- Filter unallowed parameters in callback

## [0.3.0] - 2022-06-24

### Changed

- Bump minimum required PHP version to 7.4
- Updated dependencies to allow PHP 8.1

## [0.2.0] - 2021-05-07

### Added

- Added service builders
- Added Psalm integration (see the [README](https://github.com/facile-it/php-openid-client))
- Allow users to customize sent claims ([#8](https://github.com/facile-it/php-openid-client/pull/8)) — thanks [@drupol](https://github.com/drupol)

### Changed

- `MetadataProviderBuilder::setClient()` is now deprecated; use `setHttpClient()` instead ([#10](https://github.com/facile-it/php-openid-client/pull/10)) — thanks [@drupol](https://github.com/drupol)

### Fixed

- Fixed token request body building ([#2](https://github.com/facile-it/php-openid-client/pull/2)) — thanks [@drupol](https://github.com/drupol)
- Fixed introspection endpoint ([#4](https://github.com/facile-it/php-openid-client/pull/4)) — thanks [@drupol](https://github.com/drupol)
- Provide the default `aud` claim in introspection if not set ([#12](https://github.com/facile-it/php-openid-client/pull/12)) — thanks [@drupol](https://github.com/drupol)

### Deprecated

- `MetadataProviderBuilder::setClient()` — use `setHttpClient()` instead ([#10](https://github.com/facile-it/php-openid-client/pull/10)) — thanks [@drupol](https://github.com/drupol)

Special thanks to [@drupol](https://github.com/drupol) for his contributions.

## [0.1.4] - 2021-02-02

### Fixed

- Fixed an issue with credentials not being URL-encoded

## [0.1.3] - 2020-12-04

### Fixed

- Fixed a bug with fetching issuer metadata

## [0.2.0-beta1] - 2020-08-03

### Added

- Added builders to create services

### Changed

- Services no longer instantiate dependencies when not provided; use builders
- Every variable, class, and interface named `userinfo` (lowercase) is renamed to `userInfo` or `UserInfo`
- `SessionCookieMiddleware` is no longer deprecated and now supports sessions with a PSR-16 simple-cache implementation

[0.4.0]: https://github.com/facile-it/php-openid-client/releases/tag/0.4.0
[0.4.0-beta1]: https://github.com/facile-it/php-openid-client/releases/tag/0.4.0-beta1
[0.3.5]: https://github.com/facile-it/php-openid-client/releases/tag/0.3.5
[0.3.4]: https://github.com/facile-it/php-openid-client/releases/tag/0.3.4
[0.3.3]: https://github.com/facile-it/php-openid-client/releases/tag/0.3.3
[0.3.2]: https://github.com/facile-it/php-openid-client/releases/tag/0.3.2
[0.3.1]: https://github.com/facile-it/php-openid-client/releases/tag/0.3.1
[0.3.0]: https://github.com/facile-it/php-openid-client/releases/tag/0.3.0
[0.2.0]: https://github.com/facile-it/php-openid-client/releases/tag/0.2.0
[0.1.4]: https://github.com/facile-it/php-openid-client/releases/tag/0.1.4
[0.1.3]: https://github.com/facile-it/php-openid-client/releases/tag/0.1.3
[0.2.0-beta1]: https://github.com/facile-it/php-openid-client/releases/tag/0.2.0-beta1
