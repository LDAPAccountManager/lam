<?php

class DurationParserTest extends TestCase
{
    private $parser;

    public function __construct()
    {
        $this->parser = new \Bretterer\IsoDurationConverter\DurationParser();
    }

    /** @test */
    public function it_can_parse_week_durations()
    {
        $this->assertEquals($this->parser->parse('P0W'), 0);
        $this->assertEquals($this->parser->parse('P1W'), 1 * 604800);
        $this->assertEquals($this->parser->parse('P5W'), 5 * 604800);
        $this->assertEquals($this->parser->parse('P10W'), 10 * 604800);
    }

    /** @test */
    public function it_can_compose_week_durations()
    {
        $this->assertEquals($this->parser->compose(0, true), 'P0W');
        $this->assertEquals($this->parser->compose(1 * 604800, true), 'P1W');
        $this->assertEquals($this->parser->compose(5 * 604800, true), 'P5W');
        $this->assertEquals($this->parser->compose(10 * 604800, true), 'P10W');
    }


    /** @test */
    public function it_can_parse_second_durations()
    {
        $this->assertEquals($this->parser->parse('PT0S'), 0);
        $this->assertEquals($this->parser->parse('PT1S'), 1 * 1);
        $this->assertEquals($this->parser->parse('PT5S'), 5 * 1);
        $this->assertEquals($this->parser->parse('PT10S'), 10 * 1);
    }

    /** @test */
    public function it_can_compose_second_durations()
    {
        $this->assertEquals($this->parser->compose(1 * 1), 'PT1S');
        $this->assertEquals($this->parser->compose(5 * 1), 'PT5S');
        $this->assertEquals($this->parser->compose(10 * 1), 'PT10S');
    }

    /** @test */
    public function it_can_parse_minute_durations()
    {
        $this->assertEquals($this->parser->parse('PT0M'), 0);
        $this->assertEquals($this->parser->parse('PT1M'), 1 * 60);
        $this->assertEquals($this->parser->parse('PT5M'), 5 * 60);
        $this->assertEquals($this->parser->parse('PT10M'), 10 * 60);
    }

    /** @test */
    public function it_can_compose_minute_durations()
    {
        $this->assertEquals($this->parser->compose(1 * 60), 'PT1M');
        $this->assertEquals($this->parser->compose(5 * 60), 'PT5M');
        $this->assertEquals($this->parser->compose(10 * 60), 'PT10M');
    }

    /** @test */
    public function it_can_parse_hour_durations()
    {
        $this->assertEquals($this->parser->parse('PT0H'), 0);
        $this->assertEquals($this->parser->parse('PT1H'), 1 * 3600);
        $this->assertEquals($this->parser->parse('PT5H'), 5 * 3600);
        $this->assertEquals($this->parser->parse('PT10H'), 10 * 3600);
    }

    /** @test */
    public function it_can_compose_hour_durations()
    {
        $this->assertEquals($this->parser->compose(1 * 3600), 'PT1H');
        $this->assertEquals($this->parser->compose(5 * 3600), 'PT5H');
        $this->assertEquals($this->parser->compose(10 * 3600), 'PT10H');
    }

    /** @test */
    public function it_can_parse_full_time_durations()
    {
        $this->assertEquals($this->parser->parse('PT0H2M'), 2 * 60);
        $this->assertEquals($this->parser->parse('PT1H5M'), 1 * 3600 + 5 * 60);
        $this->assertEquals($this->parser->parse('PT5H10S'), 5 * 3600 + 10 * 1);
        $this->assertEquals($this->parser->parse('PT10H40M23S'), 10 * 3600 + 40 * 60 + 23 * 1);

    }

    /** @test */
    public function it_can_compose_full_time_durations()
    {
        $this->assertEquals($this->parser->compose(10 * 3600 + 2 * 60), 'PT10H2M');
        $this->assertEquals($this->parser->compose(1 * 3600 + 5 * 60), 'PT1H5M');
        $this->assertEquals($this->parser->compose(5 * 3600 + 10 * 1), 'PT5H10S');
        $this->assertEquals($this->parser->compose(10 * 3600 + 40 * 60 + 23 * 1), 'PT10H40M23S');

    }

    /** @test */
    public function it_can_parse_day_durations()
    {
        $this->assertEquals($this->parser->parse('P0D'), 0 * 86400);
        $this->assertEquals($this->parser->parse('P1D'), 1 * 86400);
        $this->assertEquals($this->parser->parse('P5D'), 5 * 86400);
        $this->assertEquals($this->parser->parse('P10D'), 10 * 86400);

    }

    /** @test */
    public function it_can_compose_day_durations()
    {
        $this->assertEquals($this->parser->compose(1 * 86400), 'P1D');
        $this->assertEquals($this->parser->compose(5 * 86400), 'P5D');
        $this->assertEquals($this->parser->compose(10 * 86400), 'P10D');

    }

    /** @test */
    public function it_can_parse_full_date_time_durations()
    {
        $this->assertEquals($this->parser->parse('P0DT30M'), 0 * 86400 + 30 * 60);
        $this->assertEquals($this->parser->parse('P10DT30S'), 10 * 86400 + 30 * 1);
        $this->assertEquals($this->parser->parse('P12DT28M'), 12 * 86400 + 28 * 60);
        $this->assertEquals($this->parser->parse('P14DT26H'), 14 * 86400 + 26 * 3600);
    }

    /** @test */
    public function it_can_compose_full_date_time_durations()
    {
        $this->assertEquals($this->parser->compose(1 * 86400 + 30 * 60), 'P1DT30M');
        $this->assertEquals($this->parser->compose(10 * 86400 + 30 * 1), 'P10DT30S');
        $this->assertEquals($this->parser->compose(12 * 86400 + 28 * 60), 'P12DT28M');
        $this->assertEquals($this->parser->compose(14 * 86400 + 26 * 3600), 'P15DT2H');
    }
    
    /** @test */
    public function it_should_throw_invalid_argument_exception_when_garbage_is_used()
    {
        $this->setExpectedException('\InvalidArgumentException', 'Invalid duration');
        $this->parser->parse('PT123123');
    }
    
    /** @test */
    public function it_should_throw_invalid_argument_exception_for_ambiguous_durations()
    {
        $this->setExpectedException('\InvalidArgumentException', 'Ambiguous duration');
        $this->parser->parse('P8Y20M10D');
    }

    /** @test */
    public function examples_work()
    {
        $this->assertEquals(8, $this->parser->parse('PT8S'));
        $this->assertEquals(300, $this->parser->parse('PT5M'));
        $this->assertEquals(72000, $this->parser->parse('PT20H'));
        $this->assertEquals(364, $this->parser->parse('PT6M4S'));

        $this->assertEquals('PT8S', $this->parser->compose(8));
        $this->assertEquals('PT5M', $this->parser->compose(300));
        $this->assertEquals('PT20H', $this->parser->compose(72000));
        $this->assertEquals('PT6M4S', $this->parser->compose(364));

        $this->assertEquals(3024000, $this->parser->parse('P5W'));
        $this->assertEquals('P5W', $this->parser->compose(3024000, true));
        $this->assertEquals('P35D', $this->parser->compose(3024000));
    }
    
    

    
    
}
