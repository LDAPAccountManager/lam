<?php

namespace Bretterer\IsoDurationConverter;

class DurationParser
{
    const WEEK = '/^P([0-9]+W)$/';
    const DATE_TIME = '/^P(([0-9]+Y)?([0-9]+M)?([0-9]+D)?)?(T([0-9]+H)?([0-9]+M)?([0-9]+S)?)?$/';

    private $totalTime;
    private $composedDuration;

    public function parse($duration)
    {
        $m = [];
        preg_match(self::WEEK, $duration, $m);

        if ($m) {
            return $this->parsePart('week', $m[1]);
        }

        preg_match(self::DATE_TIME, $duration, $m);

        if ($m) {
            return (
                $this->parsePart('date', isset($m[2]) ? $m[2] : null) +
                $this->parsePart('date', isset($m[3]) ? $m[3] : null) +
                $this->parsePart('date', isset($m[4]) ? $m[4] : null) +
                $this->parsePart('time', isset($m[6]) ? $m[6] : null) +
                $this->parsePart('time', isset($m[7]) ? $m[7] : null) +
                $this->parsePart('time', isset($m[8]) ? $m[8] : null)
            );
        }

        throw new \InvalidArgumentException('Invalid duration');

    }

    public function compose($time, $weekMode = false)
    {
        $this->totalTime = $time;
        $this->composedDuration = 'P';

        if($weekMode) {
            $this->composePart('week W');
            return $this->composedDuration;
        }

        $this->composePart('date D');
        $this->composePart('time H');
        $this->composePart('time M');
        $this->composePart('time S');

        return $this->composedDuration;
    }

    private function parsePart($mode, $string)
    {
        if(!$string) return 0;

        $n = $this->extractInt($string);
        $id = $mode . ' ' . $string[strlen($string)-1];

        if ($n === 0) return 0;

        switch($id) {
            case 'time S':
                return $n * 1;
            case 'time M':
                return $n * 60;
            case 'time H':
                return $n * 3600;
            case 'date D':
                return $n * 86400;
            case 'week W':
                return $n * 604800;
        }
        throw new \InvalidArgumentException('Ambiguous duration');
    }

    private function extractInt($string)
    {
        return (int) substr($string, 0, strlen($string)-1);
    }

    private function composePart($mode)
    {
        $time = true;

        switch($mode) {
            case 'time S':
                $string = max((int)floor($this->totalTime / 1), 0);
                $this->totalTime -= $string*1;
                $string = $string > 0 ? $string.'S' : '';
                break;
            case 'time M':
                $string = max((int)floor($this->totalTime / 60), 0);
                $this->totalTime -= $string*60;
                $string = $string > 0 ? $string.'M' : '';
                break;
            case 'time H':
                $string = max((int)floor($this->totalTime / 3600), 0);
                $this->totalTime -= $string*3600;
                $string = $string > 0 ? $string.'H' : '';
                break;
            case 'date D':
                $string = max((int)floor($this->totalTime / 86400), 0);
                $this->totalTime -= $string*86400;
                $string = $string > 0 ? $string.'D' : '';
                $time = false;
                break;
            case 'week W':
                $string = max((int)floor($this->totalTime / 604800), 0) . 'W';
                $this->totalTime -= $string*604800;
                $time = false;
                break;
        }

        if(($time && !strpos($this->composedDuration, 'T')) && $string != '') {
            $this->composedDuration .= 'T';
        }



        $this->composedDuration .= $string;

    }

}