# PHP ISO8601 Duration Converter
Easily converts ISO 8601 Durations to Seconds and Seconds to ISO 8601 Durations

## Installation
```sh
composer require bretterer/iso_duration_converter
```

## Usage
```php
$converter = new \Bretterer\IsoDurationConverter\DurationParser();
$converter->parse('PT8S'); // Returns 8
$converter->parse('PT5M'); // Returns 300
$converter->parse('PT20H'); // Returns 72000
$converter->parse('PT6M4S'); // Returns 364

$converter->compose(8); // Returns PT8S
$converter->composer(300); // Returns PT5M
$converter->composer(7200); // Returns PT20H
$converter->compose(364); //Returns PT6M4S

$converter->parse('P5W'); // Returns 3024000
// To Returns Weeks, The second argument should be true
$converter->compose(3024000, true); // Returns P5W
$converter->compose(3024000); // Returns P35D

$converter->parse('Hello World'); // Throws 'Invalid Argument Exception' with Message 'Invalid Duration'
$converter->parse('P10Y10M10D'); // Throws 'Invalid Argument Exception' with Message 'Ambiguous Duration'
```

## Years and Months
If years are passed into the `parse` method, an `invalid argument exception` will be thrown.

If you are wanting to convert seconds into months, pass true as the second argument in the `compose` method

## License
MIT