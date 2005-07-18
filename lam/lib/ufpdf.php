<?php
/*******************************************************************************
* Software: UFPDF, Unicode Free PDF generator                                  *
* Version:  0.1                                                                *
*           based on FPDF 1.52 by Olivier PLATHEY                              *
* Date:     2004-09-01                                                         *
* Author:   Steven Wittens <steven@acko.net>                                   *
* License:  GPL                                                                *
*                                                                              *
* UFPDF is a modification of FPDF to support Unicode through UTF-8.            *
*                                                                              *
* This file includes modifications from Andrea Rossato which fix line breaking *
* in Write()/MultiCell().                                                      *
*                                                                              *
*******************************************************************************/

if(!class_exists('UFPDF'))
{
define('UFPDF_VERSION','0.1');

include_once 'fpdf.php';

/**
 * Main UFPDF class for creating Unicode PDF documents
 *
 * derives from FPDF class
 * @see FPDF
 */
class UFPDF extends FPDF
{

/*******************************************************************************
*                                                                              *
*                               Public methods                                 *
*                                                                              *
*******************************************************************************/
function UFPDF($orientation='P',$unit='mm',$format='A4')
{
  FPDF::FPDF($orientation, $unit, $format);
}

function GetStringWidth($s)
{
  //Get width of a string in the current font
  $s = (string)$s;
  $codepoints=$this->utf8_to_codepoints(trim($s));
  $cw=&$this->CurrentFont['cw'];
  $w=0;
  foreach($codepoints as $cp) {
    if (isset($cw[$cp])) {
	    $w+=$cw[$cp];
    }
    else if (isset($cw[ord($cp)])) {
	    $w+=$cw[ord($cp)];
    }
    else if (isset($cw[chr($cp)])) {
	    $w+=$cw[chr($cp)];
    }
    //-- adjust width for incorrect hebrew chars
    if ($cp>1480 && $cp < 1550) $w -= $cw[$cp]/1.8;
  }
  return $w*$this->FontSize/1000;
}

function AddFont($family,$style='',$file='')
{
  //Add a TrueType or Type1 font
  $family=strtolower($family);
  if($family=='arial')
    $family='helvetica';
  $style=strtoupper($style);
  if($style=='IB')
    $style='BI';
  if(isset($this->fonts[$family.$style]))
    $this->Error('Font already added: '.$family.' '.$style);
  if($file=='')
    $file=str_replace(' ','',$family).strtolower($style).'.php';
  if(defined('FPDF_FONTPATH'))
    $file=FPDF_FONTPATH.$file;
  include($file);
  if(!isset($name))
    $this->Error('Could not include font definition file');
  $i=count($this->fonts)+1;
  $this->fonts[$family.$style]=array('i'=>$i,'type'=>$type,'name'=>$name,'desc'=>$desc,'up'=>$up,'ut'=>$ut,'cw'=>$cw,'file'=>$file,'ctg'=>$ctg);
  if($file)
  {
    if($type=='TrueTypeUnicode')
      $this->FontFiles[$file]=array('length1'=>$originalsize);
    else
      $this->FontFiles[$file]=array('length1'=>$size1,'length2'=>$size2);
  }
}

function Text($x,$y,$txt)
{
  //Output a string
  $s=sprintf('BT %.2f %.2f Td %s Tj ET',$x*$this->k,($this->h-$y)*$this->k,$this->_escapetext($txt));
  if($this->underline and $txt!='')
    $s.=' '.$this->_dounderline($x,$y,$this->GetStringWidth($txt),$txt);
  if($this->ColorFlag)
    $s='q '.$this->TextColor.' '.$s.' Q';
  $this->_out($s);
}

function AcceptPageBreak()
{
  //Accept automatic page break or not
  return $this->AutoPageBreak;
}

function Cell($w,$h=0,$txt='',$border=0,$ln=0,$align='J',$fill=0,$link='')
{
  //Output a cell
  $k=$this->k;
  if($this->y+$h>$this->PageBreakTrigger and !$this->InFooter and $this->AcceptPageBreak())
  {
    //Automatic page break
    $x=$this->x;
    $ws=$this->ws;
    if($ws>0)
    {
      $this->ws=0;
      $this->_out('0 Tw');
    }
    $this->AddPage($this->CurOrientation);
    $this->x=$x;
    if($ws>0)
    {
      $this->ws=$ws;
      $this->_out(sprintf('%.3f Tw',$ws*$k));
    }
  }
  if($w==0)
    $w=$this->w-$this->rMargin-$this->x;
  $s='';
  if($fill==1 or $border==1)
  {
    if($fill==1)
      $op=($border==1) ? 'B' : 'f';
    else
      $op='S';
    $s=sprintf('%.2f %.2f %.2f %.2f re %s ',$this->x*$k,($this->h-$this->y)*$k,$w*$k,-$h*$k,$op);
  }
  if(is_string($border))
  {
    $x=$this->x;
    $y=$this->y;
    if(is_int(strpos($border,'L')))
      $s.=sprintf('%.2f %.2f m %.2f %.2f l S ',$x*$k,($this->h-$y)*$k,$x*$k,($this->h-($y+$h))*$k);
    if(is_int(strpos($border,'T')))
      $s.=sprintf('%.2f %.2f m %.2f %.2f l S ',$x*$k,($this->h-$y)*$k,($x+$w)*$k,($this->h-$y)*$k);
    if(is_int(strpos($border,'R')))
      $s.=sprintf('%.2f %.2f m %.2f %.2f l S ',($x+$w)*$k,($this->h-$y)*$k,($x+$w)*$k,($this->h-($y+$h))*$k);
    if(is_int(strpos($border,'B')))
      $s.=sprintf('%.2f %.2f m %.2f %.2f l S ',$x*$k,($this->h-($y+$h))*$k,($x+$w)*$k,($this->h-($y+$h))*$k);
  }
  if($txt!='')
  {
    $width = $this->GetStringWidth($txt);
    if($align=='R')
      $dx=$w-$this->cMargin-$width;
    elseif($align=='C')
      $dx=($w-$width)/2;
    else
      $dx=$this->cMargin;
    if($this->ColorFlag)
      $s.='q '.$this->TextColor.' ';
    $txtstring=$this->_escapetext($txt);
    $s.=sprintf('BT %.2f %.2f Td %s Tj ET',($this->x+$dx)*$k,($this->h-($this->y+.5*$h+.3*$this->FontSize))*$k,$txtstring);
    if($this->underline)
      $s.=' '.$this->_dounderline($this->x+$dx,$this->y+.5*$h+.3*$this->FontSize,$width,$txt);
    if($this->ColorFlag)
      $s.=' Q';
    if($link)
      $this->Link($this->x+$dx,$this->y+.5*$h-.5*$this->FontSize,$width,$this->FontSize,$link);
  }
  if($s)
    $this->_out($s);
  $this->lasth=$h;
  if($ln>0)
  {
    //Go to next line
    $this->y+=$h;
    if($ln==1)
      $this->x=$this->lMargin;
  }
  else
    $this->x+=$w;
}

function MultiCell($w,$h,$txt,$border=0,$align='J',$fill=0)
{
	//Output text with automatic or explicit line breaks
	$cw=&$this->CurrentFont['cw'];
	$cp=$this->utf8_to_codepoints(trim($txt));
	//print_r($cp);
	if($w==0)
		$w=$this->w-$this->rMargin-$this->x;
	$wmax=($w-2*$this->cMargin)*1000/$this->FontSize;
	//echo $w;
	$s=str_replace("\r",'',$txt);
	$nb=$this->strlen($s);
	if($nb>0 and $s[$nb-1]=="\n")
		$nb--;
	$b=0;
	if($border)
	{
		if($border==1)
		{
			$border='LTRB';
			$b='LRT';
			$b2='LR';
		}
		else
		{
			$b2='';
			if(is_int(strpos($border,'L')))
				$b2.='L';
			if(is_int(strpos($border,'R')))
				$b2.='R';
			$b=is_int(strpos($border,'T')) ? $b2.'T' : $b2;
		}
	}
	$sep=-1;
	$i=0;
	$j=0;
	$l=0;
	$ns=0;
	$nl=1;
	$char = 0;
	while($i<$nb)
	{
	  //Get next character
	  $c = $this->code2utf($cp[$i]);
	  $charw = $cw[$cp[$i]];
		
		if($c=="\n")
		{
			//Explicit line break
			if($this->ws>0)
			{
				$this->ws=0;
				$this->_out('0 Tw');
			}
			$this->Cell($w,$h,$this->utf8_substr($cp,$j,$i-$j,"UTF-8"),$b,2,$align,$fill);
			$i++;
			$sep=-1;
			$j=$i;
			$l=0;
			$ns=0;
			$nl++;
			if($border and $nl==2)
				$b=$b2;
			continue;
		}
		if($c==' ')
		{
			$sep=$i;
			$ls=$l;
			$ns++;
		}
		$l+=$charw;
		
		if($l>$wmax)
		{
			//Automatic line break
			if($sep==-1)
			{
				if($i==$j)
					$i++;
				if($this->ws>0)
				{
					$this->ws=0;
					$this->_out('0 Tw');
				}
				$this->Cell($w,$h,$this->utf8_substr($cp,$j,$i-$j,"UTF-8"),$b,2,$align,$fill);
			}
			else
			{
				if($align=='J')
				{
				  $len_ligne = $this->GetStringWidth($this->utf8_substr($cp,$j,$sep-$j,"UTF-8"));
				  $nb_carac = $this->strlen($this->utf8_substr($cp,$j,$sep-$j,"UTF-8"));
				  $ecart = (($w-2) - $len_ligne) / $nb_carac;
				  $this->_out(sprintf('BT %.3f Tc ET',$ecart*$this->k));
				  //$this->ws=($ns>1) ? ($wmax-$ls)/1000*$this->FontSize/($ns-1) : 0;
				  //$this->_out(sprintf('%.3f Tw',$this->ws*$this->k));
				  //echo ($wmax-$ls)/1000*$this->FontSize/($ns-1)."=".($wmax-$ls)."<br>";	//$andrea = sprintf('%.3f Tw',$this->ws*$this->k);
				}
				$this->Cell($w,$h,$this->utf8_substr($cp,$j,$sep-$j,"UTF-8"),$b,2,$align,$fill);
				$i=$sep+1;
			}
			$sep=-1;
			$j=$i;
			$l=0;
			$ns=0;
			$nl++;
			if($border and $nl==2)
				$b=$b2;
		}
		else
			$i++;
	}
	//Last chunk
	if($this->ws>0)
	{
		$this->ws=0;
		$this->_out('0 Tw');
	}
	if($border and is_int(strpos($border,'B')))
		$b.='B';
	$this->Cell($w,$h,$this->utf8_substr($cp,$j,$i-$j,"UTF-8"),$b,2,$align,$fill);
	$this->x=$this->lMargin;
}

function Write($h,$txt,$link='')
{
	//Output text in flowing mode
	$cw=&$this->CurrentFont['cw'];
	$cp=$this->utf8_to_codepoints(trim($txt));
	$w=$this->w-$this->rMargin-$this->x;
	$wmax=($w-2*$this->cMargin)*1000/$this->FontSize;
	$s=str_replace("\r",'',$txt);
	$nb=$this->strlen($s);
	$sep=-1;
	$i=0;
	$j=0;
	$l=0;
	$nl=1;
	while($i<$nb)
	{
		//Get next character
		$c=$s{$i};
		$charw = $cw[$cp[$i]];
		if($c=="\n")
		{
			//Explicit line break
			$this->Cell($w,$h,$this->utf8_substr($cp,$j,$i-$j,"UTF-8"),0,2,'',0,$link);
			$i++;
			$sep=-1;
			$j=$i;
			$l=0;
			if($nl==1)
			{
				$this->x=$this->lMargin;
				$w=$this->w-$this->rMargin-$this->x;
				$wmax=($w-2*$this->cMargin)*1000/$this->FontSize;
			}
			$nl++;
			continue;
		}
		if($c==' ')
			$sep=$i;
		$l+=$charw;
		if($l>$wmax)
		{
			//Automatic line break
			if($sep==-1)
			{
				if($this->x>$this->lMargin)
				{
					//Move to next line
					$this->x=$this->lMargin;
					$this->y+=$h;
					$w=$this->w-$this->rMargin-$this->x;
					$wmax=($w-2*$this->cMargin)*1000/$this->FontSize;
					$i++;
					$nl++;
					continue;
				}
				if($i==$j)
					$i++;
				$this->Cell($w,$h,$this->utf8_substr($cp,$j,$i-$j,"UTF-8"),0,2,'',0,$link);
			}
			else
			{
				$this->Cell($w,$h,$this->utf8_substr($cp,$j,$sep-$j,"UTF-8"),0,2,'',0,$link);
				$i=$sep+1;
			}
			$sep=-1;
			$j=$i;
			$l=0;
			if($nl==1)
			{
				$this->x=$this->lMargin;
				$w=$this->w-$this->rMargin-$this->x;
				$wmax=($w-2*$this->cMargin)*1000/$this->FontSize;
			}
			$nl++;
		}
		else
			$i++;
	}
	//Last chunk
	if($i!=$j)
		$this->Cell($l/1000*$this->FontSize,$h,$this->utf8_substr($cp,$j, "", "UTF-8"),0,0,'',0,$link);
}

function AliasNbPages($alias='{nb}')
{
	//Define an alias for total number of pages
	$this->AliasNbPages=$this->utf8_to_utf16be($alias,false);
}

/*******************************************************************************
*                                                                              *
*                              Protected methods                               *
*                                                                              *
*******************************************************************************/

function _puttruetypeunicode($font) {
  //Type0 Font
  $this->_newobj();
  $this->_out('<</Type /Font');
  $this->_out('/Subtype /Type0');
  $this->_out('/BaseFont /'. $font['name'] );
  $this->_out('/Encoding /Identity-H');
  $this->_out('/DescendantFonts ['. ($this->n + 1) .' 0 R]');
  $this->_out('>>');
  $this->_out('endobj');

  //CIDFont
  $this->_newobj();
  $this->_out('<</Type /Font');
  $this->_out('/Subtype /CIDFontType2');
  $this->_out('/BaseFont /'. $font['name']);
  $this->_out('/CIDSystemInfo <</Registry (Adobe) /Ordering (UCS) /Supplement 0>>');
  $this->_out('/FontDescriptor '. ($this->n + 1) .' 0 R');
  $c = 0;
  $widths = "";
  foreach ($font['cw'] as $i => $w) {
    $widths .= $i .' ['. $w.'] ';
  }
  $this->_out('/W ['. $widths .']');
  $this->_out('/CIDToGIDMap '. ($this->n + 2) .' 0 R');
  $this->_out('>>');
  $this->_out('endobj');

  //Font descriptor
  $this->_newobj();
  $this->_out('<</Type /FontDescriptor');
  $this->_out('/FontName /'.$font['name']);
  $s = "";
  foreach ($font['desc'] as $k => $v) {
    $s .= ' /'. $k .' '. $v;
  }
  if ($font['file']) {
		$s .= ' /FontFile2 '. $this->FontFiles[$font['file']]['n'] .' 0 R';
  }
  $this->_out($s);
  $this->_out('>>');
  $this->_out('endobj');

  //Embed CIDToGIDMap
  $this->_newobj();
  if(defined('FPDF_FONTPATH'))
    $file=FPDF_FONTPATH.$font['ctg'];
  else
    $file=$font['ctg'];
  $size=filesize($file);
  if(!$size)
    $this->Error('Font file not found');
  $this->_out('<</Length '.$size);
	if(substr($file,-2) == '.z')
    $this->_out('/Filter /FlateDecode');
  $this->_out('>>');
  $f = fopen($file,'rb');
  $this->_putstream(fread($f,$size));
  fclose($f);
  $this->_out('endobj');
}

function _dounderline($x,$y,$width,$txt)
{
  //Underline text
  $up=$this->CurrentFont['up'];
  $ut=$this->CurrentFont['ut'];
  $w=$width+$this->ws*substr_count($txt,' ');
  return sprintf('%.2f %.2f %.2f %.2f re f',$x*$this->k,($this->h-($y-$up/1000*$this->FontSize))*$this->k,$w*$this->k,-$ut/1000*$this->FontSizePt);
}

function _textstring($s)
{
  //Convert to UTF-16BE
  $s = $this->utf8_to_utf16be($s);
  //Escape necessary characters
  return '('. strtr($s, array(')' => '\\)', '(' => '\\(', '\\' => '\\\\')) .')';
}

function _escapetext($s)
{
  //Convert to UTF-16BE
  $s = $this->utf8_to_utf16be($s, false);
  //Escape necessary characters
  return '('. strtr($s, array(')' => '\\)', '(' => '\\(', '\\' => '\\\\')) .')';
}

function _putinfo()
{
	$this->_out('/Producer '.$this->_textstring('UFPDF '. UFPDF_VERSION));
	if(!empty($this->title))
		$this->_out('/Title '.$this->_textstring($this->title));
	if(!empty($this->subject))
		$this->_out('/Subject '.$this->_textstring($this->subject));
	if(!empty($this->author))
		$this->_out('/Author '.$this->_textstring($this->author));
	if(!empty($this->keywords))
		$this->_out('/Keywords '.$this->_textstring($this->keywords));
	if(!empty($this->creator))
		$this->_out('/Creator '.$this->_textstring($this->creator));
	$this->_out('/CreationDate '.$this->_textstring('D:'.date('YmdHis')));
}

function _putpages()
{
	$nb=$this->page;
	if(!empty($this->AliasNbPages))
	{
		$nbstr = $this->utf8_to_utf16be($nb,false);
		//Replace number of pages
		for($n=1;$n<=$nb;$n++) {
			$this->pages[$n]=str_replace($this->AliasNbPages,$nbstr,$this->pages[$n]);
		}
	}
	if($this->DefOrientation=='P')
	{
		$wPt=$this->fwPt;
		$hPt=$this->fhPt;
	}
	else
	{
		$wPt=$this->fhPt;
		$hPt=$this->fwPt;
	}
	$filter=($this->compress) ? '/Filter /FlateDecode ' : '';
	for($n=1;$n<=$nb;$n++)
	{
		//Page
		$this->_newobj();
		$this->_out('<</Type /Page');
		$this->_out('/Parent 1 0 R');
		if(isset($this->OrientationChanges[$n]))
			$this->_out(sprintf('/MediaBox [0 0 %.2f %.2f]',$hPt,$wPt));
		$this->_out('/Resources 2 0 R');
		if(isset($this->PageLinks[$n]))
		{
			//Links
			$annots='/Annots [';
			foreach($this->PageLinks[$n] as $pl)
			{
				$rect=sprintf('%.2f %.2f %.2f %.2f',$pl[0],$pl[1],$pl[0]+$pl[2],$pl[1]-$pl[3]);
				$annots.='<</Type /Annot /Subtype /Link /Rect ['.$rect.'] /Border [0 0 0] ';
				if(is_string($pl[4]))
					$annots.='/A <</S /URI /URI '.$this->_textstring($pl[4]).'>>>>';
				else
				{
					$l=$this->links[$pl[4]];
					$h=isset($this->OrientationChanges[$l[0]]) ? $wPt : $hPt;
					$annots.=sprintf('/Dest [%d 0 R /XYZ 0 %.2f null]>>',1+2*$l[0],$h-$l[1]*$this->k);
				}
			}
			$this->_out($annots.']');
		}
		$this->_out('/Contents '.($this->n+1).' 0 R>>');
		$this->_out('endobj');
		//Page content
		$p=($this->compress) ? gzcompress($this->pages[$n]) : $this->pages[$n];
		$this->_newobj();
		$this->_out('<<'.$filter.'/Length '.strlen($p).'>>');
		$this->_putstream($p);
		$this->_out('endobj');
	}
	//Pages root
	$this->offsets[1]=strlen($this->buffer);
	$this->_out('1 0 obj');
	$this->_out('<</Type /Pages');
	$kids='/Kids [';
	for($i=0;$i<$nb;$i++)
		$kids.=(3+2*$i).' 0 R ';
	$this->_out($kids.']');
	$this->_out('/Count '.$nb);
	$this->_out(sprintf('/MediaBox [0 0 %.2f %.2f]',$wPt,$hPt));
	$this->_out('>>');
	$this->_out('endobj');
}

// UTF-8 to UTF-16BE conversion.
// Correctly handles all illegal UTF-8 sequences.
function utf8_to_utf16be(&$txt, $bom = true) {
  $l = strlen($txt);
  $txt .= " ";
  $out = $bom ? "\xFE\xFF" : '';
  for ($i = 0; $i < $l; ++$i) {
    $c = ord($txt{$i});
    // ASCII
    if ($c < 0x80) {
      $out .= "\x00". $txt{$i};
    }
    // Lost continuation byte
    else if ($c < 0xC0) {
      $out .= "\xFF\xFD";
      continue;
    }
    // Multibyte sequence leading byte
    else {
      if ($c < 0xE0) {
        $s = 2;
      }
      else if ($c < 0xF0) {
        $s = 3;
      }
      else if ($c < 0xF8) {
        $s = 4;
      }
      // 5/6 byte sequences not possible for Unicode.
      else {
        $out .= "\xFF\xFD";
        while (ord($txt{$i + 1}) >= 0x80 && ord($txt{$i + 1}) < 0xC0) { ++$i; }
        continue;
      }
      
      $q = array($c);
      // Fetch rest of sequence
      while (ord($txt{$i + 1}) >= 0x80 && ord($txt{$i + 1}) < 0xC0) { ++$i; $q[] = ord($txt{$i}); }
      
      // Check length
      if (count($q) != $s) {
        $out .= "\xFF\xFD";        
        continue;
      }
      
      switch ($s) {
        case 2:
          $cp = (($q[0] ^ 0xC0) << 6) | ($q[1] ^ 0x80);
          // Overlong sequence
          if ($cp < 0x80) {
            $out .= "\xFF\xFD";        
          }
          else {
            $out .= chr($cp >> 8);
            $out .= chr($cp & 0xFF);
          }
          continue;

        case 3:
          $cp = (($q[0] ^ 0xE0) << 12) | (($q[1] ^ 0x80) << 6) | ($q[2] ^ 0x80);
          // Overlong sequence
          if ($cp < 0x800) {
            $out .= "\xFF\xFD";        
          }
          // Check for UTF-8 encoded surrogates (caused by a bad UTF-8 encoder)
          else if ($c > 0xD800 && $c < 0xDFFF) {
            $out .= "\xFF\xFD";
          }
          else {
            $out .= chr($cp >> 8);
            $out .= chr($cp & 0xFF);
          }
          continue;

        case 4:
          $cp = (($q[0] ^ 0xF0) << 18) | (($q[1] ^ 0x80) << 12) | (($q[2] ^ 0x80) << 6) | ($q[3] ^ 0x80);
          // Overlong sequence
          if ($cp < 0x10000) {
            $out .= "\xFF\xFD";
          }
          // Outside of the Unicode range
          else if ($cp >= 0x10FFFF) {
            $out .= "\xFF\xFD";            
          }
          else {
            // Use surrogates
            $cp -= 0x10000;
            $s1 = 0xD800 | ($cp >> 10);
            $s2 = 0xDC00 | ($cp & 0x3FF);
            
            $out .= chr($s1 >> 8);
            $out .= chr($s1 & 0xFF);
            $out .= chr($s2 >> 8);
            $out .= chr($s2 & 0xFF);
          }
          continue;
      }
    }
  }
  return $out;
}

function code2utf($num){
  if($num<128)return chr($num);
  if($num<2048)return chr(($num>>6)+192).chr(($num&63)+128);
  if($num<65536)return chr(($num>>12)+224).chr((($num>>6)&63)+128).chr(($num&63)+128);
  if($num<2097152)return chr(($num>>18)+240).chr((($num>>12)&63)+128).chr((($num>>6)&63)+128). chr(($num&63)+128);
  return '';
}

function strlen($s) {
  return strlen(utf8_decode($s));
}

function utf8_substr($str,$start)
{
  if( func_num_args() >= 3 ) {
    $end = func_get_arg( 2 ); 
    for ($i=$start; $i < ($start+$end); $i++)
      $rs .= $this->code2utf($str[$i]);
    
  } else {
    for ($i=$start; $i < count($str); $i++)
      $rs .= $this->code2utf($str[$i]);
  }

  return $rs;
}

// UTF-8 to codepoint array conversion.
// Correctly handles all illegal UTF-8 sequences.
function utf8_to_codepoints(&$txt) {
  $l = strlen($txt);
  $txt .= " ";
  $out = array();
  for ($i = 0; $i < $l; ++$i) {
    $c = ord($txt{$i});
    // ASCII
    if ($c < 0x80) {
      $out[] = ord($txt{$i});
    }
    // Lost continuation byte
    else if ($c < 0xC0) {
      $out[] = 0xFFFD;
      continue;
    }
    // Multibyte sequence leading byte
    else {
      if ($c < 0xE0) {
        $s = 2;
      }
      else if ($c < 0xF0) {
        $s = 3;
      }
      else if ($c < 0xF8) {
        $s = 4;
      }
      // 5/6 byte sequences not possible for Unicode.
      else {
        $out[] = 0xFFFD;
        while (ord($txt{$i + 1}) >= 0x80 && ord($txt{$i + 1}) < 0xC0) { ++$i; }
        continue;
      }
      
      $q = array($c);
      // Fetch rest of sequence
      while (ord($txt{$i + 1}) >= 0x80 && ord($txt{$i + 1}) < 0xC0) { ++$i; $q[] = ord($txt{$i}); }
      
      // Check length
      if (count($q) != $s) {
        $out[] = 0xFFFD;
        continue;
      }
      
      switch ($s) {
        case 2:
          $cp = (($q[0] ^ 0xC0) << 6) | ($q[1] ^ 0x80);
          // Overlong sequence
          if ($cp < 0x80) {
            $out[] = 0xFFFD;
          }
          else {
            $out[] = $cp;
          }
          continue;

        case 3:
          $cp = (($q[0] ^ 0xE0) << 12) | (($q[1] ^ 0x80) << 6) | ($q[2] ^ 0x80);
          // Overlong sequence
          if ($cp < 0x800) {
            $out[] = 0xFFFD;
          }
          // Check for UTF-8 encoded surrogates (caused by a bad UTF-8 encoder)
          else if ($c > 0xD800 && $c < 0xDFFF) {
            $out[] = 0xFFFD;
          }
          else {
            $out[] = $cp;
          }
          continue;

        case 4:
          $cp = (($q[0] ^ 0xF0) << 18) | (($q[1] ^ 0x80) << 12) | (($q[2] ^ 0x80) << 6) | ($q[3] ^ 0x80);
          // Overlong sequence
          if ($cp < 0x10000) {
            $out[] = 0xFFFD;
          }
          // Outside of the Unicode range
          else if ($cp >= 0x10FFFF) {
            $out[] = 0xFFFD;
          }
          else {
            $out[] = $cp;
          }
          continue;
      }
    }
  }
  return $out;
}

//End of class
}

}
?>
