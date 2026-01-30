# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased] - yyyy-mm-dd

Here we write upgrading notes for brands. It's a team effort to make them as
straightforward as possible.

## [0.5.1] - 2026-01-08

### Fixed
- Fixed jwt-library warning porting commits from v0.4.4
- Add support for php 8.5


## [0.5.0] - 2026-01-02

### Added
- Add support for PHP 8.2 and 8.3

### Changed
- Drop support for PHP < 8.1
- Changed [`psalm`](https://psalm.dev) types
- Moved builders in `Facile\JoseVerifier\Builder` namespace
- Immutable builders
- Replaced deprecated `web-token/*` packages with `web-token/jwt-library`
- Drop support for JWE compression (deprecated dependency)

### Fixed
- Fixed check for mandatory `auth_time` when `require_auth_time` is `true`

### Removed
- Removed [`psalm`](https://psalm.dev) plugin


## [0.5.0-beta1] - 2024-04-27

### Added
- Add support for PHP 8.2 and 8.3

### Changed
- Drop support for PHP < 8.1
- Changed [`psalm`](https://psalm.dev) types
- Moved builders in `Facile\JoseVerifier\Builder` namespace
- Immutable builders
- Replaced deprecated `web-token/*` packages with `web-token/jwt-library`
- Drop support for JWE compression (deprecated dependency)

### Fixed
- Fixed check for mandatory `auth_time` when `require_auth_time` is `true`

### Removed
- Removed [`psalm`](https://psalm.dev) plugin
