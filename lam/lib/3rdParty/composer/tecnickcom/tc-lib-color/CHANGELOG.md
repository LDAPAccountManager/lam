# Changelog

This project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
A change that alters a public signature, the bytes a method emits, or whether an
input parses, goes in a major release.

## 3.0.0

### Breaking: API

- `Pdf::getPdfColor()`, `getPdfStrokeColor()`, `getPdfFillColor()`,
  `getColorObject()`, `getPdfRgbComponents()` and `getPdfCmykComponents()` take a
  trailing `bool $allowSpot = true`, which forces a device color when false.
  A subclass overriding any of these must mirror the new signature.
- Every public method of `Web`, `Spot` and `Pdf` is `final` except the documented
  extension points, `Web::getColorObj()`, `Spot::getSpotColor()`,
  `Pdf::getPdfColor()` and `Pdf::getColorObject()`.
- `Model::__construct()` is gone from the abstract base; each model defines its
  own constructor and validates the component names it is given.
- An unknown component name raises `UnknownComponentException`.
- `Model\Template::invertColor()` returns `static` rather than `self`, and the
  interface gained `withInvertedColor()`.
- `Css::normalizeValue()` is no longer abstract. Scaling moved to the new
  `ComponentNormalizer`, which `Css` holds as a collaborator;
  `Web::normalizeValue()` delegates to it.
- `Css::__construct()` accepts an optional `ComponentNormalizer`.

### Breaking: output

- `Rgb::getCssColor()` emits channels as integers in [0..255]
  (`rgb(255,0,0)`) instead of whole percentages (`rgb(100%,0%,0%)`), and
  `Gray::getCssColor()` emits `g(128)` instead of `g(50%)`.
- Alpha is emitted with up to 4 decimals (`rgba(18,52,86,0.4706)`) instead of the
  full float repr.
- The Lab Separation tint transform written by `getPdfSpotObjects()` carries a
  `/Range` array, matching the DeviceCMYK one.
- `getPdfSpotResources()` and `getPdfSpotResourcesByKeys()` raise when a
  registered spot color has no PDF object.

### Breaking: parsing

- `rebeccapurple` is recognised.
- Malformed numbers are rejected: `rgb(1.2.3,4,5)` raises.
- Mixing comma and space separators raises: `rgb(64, 128 191)`.
- Out-of-range components are clamped and hues wrap, as CSS Color Level 4
  requires: `rgb(-10,128,191)` is `#0080bf` and `hsl(-150,50%,50%)` is
  `hsl(210,50%,50%)`.

### Added

- `Model::withInvertedColor()`, a non-mutating counterpart to `invertColor()`.
- `Web::getClosestWebColorByDeltaE()`, `getClosestWebColorByDeltaEFromString()`
  and `getLabSquareDistance()`: nearest-color search in CIE Lab.
- `ExceptionInterface`, implemented by both exception types.
- `UnknownComponentException` for a component name a model does not define.
- `ColorModelType`, a backed enum for the model type, and `Model::getTypeEnum()`.
- `ColorParserInterface`, `SpotRegistryInterface` and
  `PdfColorWriterInterface`, one per role of the class hierarchy.
- `ComponentNormalizer`, the component scaler the CSS parser delegates to.
- CSS angle units for hue (`deg`, `grad`, `rad`, `turn`), the slash-alpha form
  and percentage alpha.

### Fixed

- The spot color accessors (`getSpotColorObj()`, `getSpotLabColorObj()`,
  `getSpotColors()`, `Pdf::getColorObject()`) return copies of the registered
  models.
- `addSpotLabColor()` narrows the range option to [-128..127], the interval the
  Lab model represents, so `/Range`, `/C0` and `/C1` agree.
- `Cmyk::toRgbArray()` returns floats for saturated channels, which used to be
  returned as `int`.
- A spot color name with no usable characters is reported as
  `invalid spot color name: <name>` by the lookup path too.
- An unrecognised color function raises `unsupported color syntax`.
- `NAN` is rejected as a component value; `INF` and `-INF` clamp.

### Documentation

- `$tint` applies to spot colors only; a device color is always written at full
  intensity.
- Eight, not eleven, of the default spot color names are also CSS color names.
  `key`, `all` and `none` are spot-only and do not resolve with
  `$allowSpot = false`.
