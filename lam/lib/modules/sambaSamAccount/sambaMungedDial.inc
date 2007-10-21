<?php
/*
  This code is part of GOsa (https://gosa.gonicus.de)
  Copyright (C) 2004  Cajus Pollmeier

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/**
* Manages terminal server settings for Samba 3.
*
* @package modules
* @author Cajus Pollmeier
*/

/* File header is treated as constant. It cannot be defined inside the class if $PHP_VERSION < 5 */
define ("SAMBA_MUNGEDDIAL_FILEHEADER",
        "20002000200020002000200020002000".
        "20002000200020002000200020002000".
        "20002000200020002000200020002000".
        "20002000200020002000200020002000".
        "20002000200020002000200020002000".
        "20002000200020002000200020002000".
      	"5000");
/* This is the old header, it is needed to automatically convert old mungedDials to new ones */
define ("SAMBA_MUNGEDDIAL_FILEHEADER_OLD",   
        "6d000800200020002000200020002000".
        "20002000200020002000200020002000".
        "20002000200020002000200064000100".
        "20002000200020002000200020002000".
        "20002000200020002000200020002000".
        "20002000200020002000200020002000".
        "50001000");
/**
* Manages terminal server settings for Samba 3.
*
* @author Cajus Pollmeier
*/
class sambaMungedDial
{
  /* Terminal server variables (samba3) */
  public $ctx= array(
  	'CtxCfgPresent' =>	        	'551e0bb0',
	  'CtxCfgFlags1' =>	          	'00e00010',
  	'CtxCallback' =>	          	'',
  	'CtxShadow' =>	          		'01000000',
  	'CtxMaxConnectionTime' =>   	'',
  	'CtxMaxDisconnectionTime' =>	'',
  	'CtxMaxIdleTime' => 	      	'',
  	'CtxKeyboardLayout' =>	     	'',
  	'CtxMinEncryptionLevel' =>  	'00',
  	'CtxWorkDirectory' =>	      	'',
  	'CtxNWLogonServer' =>	      	'',  
  	'CtxWFHomeDir' =>	           	'',
  	'CtxWFHomeDirDrive' =>	    	'',
  	'CtxWFProfilePath' =>	      	'',
  	'CtxInitialProgram' =>	     	'',
  	'CtxCallbackNumber' =>	    	'');

  /* attribute list for save action */
  public $ctxattributes= array("CtxCfgPresent", "CtxCfgFlags1", "CtxCallback",
        "CtxShadow", "CtxMaxConnectionTime", "CtxMaxDisconnectionTime",
        "CtxMaxIdleTime", "CtxKeyboardLayout", "CtxMinEncryptionLevel",
        "CtxWorkDirectory", "CtxNWLogonServer", "CtxWFHomeDir",
        "CtxWFHomeDirDrive", "CtxWFProfilePath", "CtxInitialProgram",
        "CtxCallbackNumber");	

  /* These parameters are treated as strings and get a trailing zero */
  private $stringParams= array( "CtxWorkDirectory", "CtxNWLogonServer", 
			"CtxWFHomeDir", "CtxWFHomeDirDrive", 
			"CtxWFProfilePath", "CtxInitialProgram", "CtxCallbackNumber");

  /* These parameters are treated as time values and get converted */
  private $timeParams= array("CtxMaxConnectionTime", "CtxMaxDisconnectionTime", "CtxMaxIdleTime");

  private $old_behavior= false;

  function strhex($string)
  {
  	$hex="";

  	for ($i=0; $i<strlen($string); $i++) {
  		$hex.= dechex(ord($string[$i]));
    }
	
  	return ($hex);
  }

  function hexstr($hex)
  {
  	$string="";

  	for ($i=0; $i<strlen($hex)-1; $i+=2) {
  		$string.= chr(hexdec($hex[$i].$hex[$i+1]));
  	}
  
  	return ($string);
  }

  function endian($src)
  {
  	return (substr($src, 2, 2).substr($src, 0, 2));
  }

  function genTime ($minutes)
  {
  	$usec= (int) ($minutes * 60 * 1000);
  	$src=  sprintf('%04x%04x', $usec & 0x0FFFF, ($usec & 0x0FFFF0000) >> 16);
  	return (sambaMungedDial::endian(substr($src, 0, 4)).sambaMungedDial::endian(substr($src, 4, 4)));
  }

  function readTime ($time)
  {
  	$lo= substr($time, 0, 4);
  	$hi= substr($time, 4, 4);
  
  	$usecs= (hexdec(substr($lo, 2, 2)) * 256 + hexdec(substr($lo, 0, 2))) +
  		(hexdec(substr($hi, 2 ,2)) * 256 + hexdec(substr($hi, 0, 2))) * 256 * 256;
  
  	return ((int)($usecs / (60 * 1000)));
  }

  function to8bit($string)
  {
  	$result= "";
  
  	/* Strip zeros */
  	for ($i= 0; $i<strlen($string); $i++){
  		if ($string[$i] != chr(0)){
  			$result.= $string[$i];
  		}
  	}
  
  	return ($result);
  }

  function is_samba_path($path)
  {
  	if ($path == ""){
  		return (TRUE);
  	}

  	if (!preg_match('/^[a-z0-9%\\\\_.:+-\\\\$]+$/i', $path)){
  		return (FALSE);
  	}
  
  	return preg_match ("/\\\\.+$/", $path);
  }

  /* Encode full MungedDial-String */
  function encode_munged ($params)
  {
  	/* Walk through the parameters and convert them */
  	$result= sambaMungedDial::hexstr(SAMBA_MUNGEDDIAL_FILEHEADER);
  
  	// CHANGED: We need to insert the number of attributes right after SAMBA_MUNGEDDIAL_FILEHEADER.
  	$counter= 0;
  	$result_tmp= "";
  	foreach ($params as $paramName => $paramValue) {
  		/* String parameter? */
  		if (in_array($paramName, $this->stringParams)){
  			$isString= TRUE;
  			$paramValue= sambaMungedDial::strhex($paramValue.chr(0).chr(0));
  		} else {
  			$isString= FALSE;
  		}
  
  		/* Time parameter? */
  		if (in_array($paramName, $this->timeParams)){
  			$paramValue= sambaMungedDial::genTime($paramValue);
  		}
  
  		$result_tmp.= sambaMungedDial::munge($paramName, $paramValue, $isString);
  		$counter++;
  	}
  	
  	// First add the number of attributes
  	$result.= sambaMungedDial::hexstr(sprintf("%02x00", $counter));
  
  	// Then the usual stuff
  	$result.= $result_tmp;
  
  	return ($result);
  }

  /* Setup parameter given by paramName to MungedDial-Format */
  function munge($paramName, $paramValue, $isString) 
  {
  	$result= "";
  	
  	/* Encode paramName to UTF-16 */
          if (function_exists("recode")){
                  $utfName= recode("ISO8859-15..UTF-16", $paramName);
          } else {
                  $utfName= iconv("ISO8859-15", "UTF-16BE", $paramName);
          }
  
  	/* Set parameter length, high and low byte */
  	$paramLen= strlen($utfName);
  	$result.= chr($paramLen & 0x0FF);
  	$result.= chr(($paramLen & 0x0FF00) >> 8);
  
  	/* String parameters have additional trailing bytes */
  	$valueLen= strlen($paramValue);
  	$result.= chr($valueLen & 0x0FF);
  	$result.= chr(($valueLen & 0x0FF00) >> 8);
  	
  	/* Length fields have a trailing '01' appended by the UTF-16 converted name */
  	$result.= chr(1);
  	$result.= $utfName;
  
  	/* Parameter is padded with '00' */
  	$result.= chr(0);
  	$result.= $paramValue;
  
  	/* Append a trailing '00' to string parameters */
  	if ($isString && (strlen($paramValue) & 1)){
  		$result.= chr(0);
  	}
  	
  	return ($result);
  }

  /* Takes a base64-encoded MungedDial-String and returns an array of included parameters and values */
  function decode_munged($munge)
  {
  	$result= array();
  		
  	/* 
  	 * Remove base64 encoding and skip SAMBA_MUNGEDDIAL_FILEHEADER.
  	 * The '4' is added, because the SAMBA_MUNGEDDIAL_FILEHEADER has been stripped by 4 chars.
  	 * This is the number of attributes following - we don't need this at read time, only when writing.
  	 */
    if(substr(base64_decode($munge),0,2)=="6d") {
      $this->old_behavior=true;
    }

    $ctxField="";
    if($this->old_behavior==true) {
  	  $ctxField= substr(base64_decode($munge), (strlen(SAMBA_MUNGEDDIAL_FILEHEADER_OLD)) / 2);
    } else {
  	  $ctxField= substr(base64_decode($munge), (strlen(SAMBA_MUNGEDDIAL_FILEHEADER)+4) / 2);
    }
  
  	/* Decode parameters */
  	while ($ctxField!=""){
  
  		/* Read value lengths */
  		$ctxParmNameLength= ord($ctxField[0]) + 16 * ord($ctxField[1]);
  		$ctxParmLength= ord($ctxField[2]) + 16 * ord($ctxField[3]);
  				
  		/* Reposition ctxField on start of parameter name, read parameter name */
  		$ctxField= substr($ctxField, 6);
  		$ctxParmName= sambaMungedDial::to8bit(substr($ctxField, 0, $ctxParmNameLength));
  				
  		/* Reposition ctxField on start of parameter */
  		$ctxField= substr($ctxField, $ctxParmNameLength);
  		$ctxParm= substr($ctxField, 0, $ctxParmLength);
  				
  		/* If string parameter, convert */
  		if (in_array($ctxParmName, $this->stringParams)){
  				$ctxParm= sambaMungedDial::hexstr($ctxParm);
  		}
  		/* If time parameter, convert */
  		if (in_array($ctxParmName, $this->timeParams)){
  			$ctxParm= sambaMungedDial::readTime($ctxParm);
  		}
  
  		/* Assign in result array */
  		$result[$ctxParmName]= trim($ctxParm);
  				
  		/* Reposition ctxField on end of parameter and continue */
  		$ctxField= substr($ctxField, $ctxParmLength);
  	}
  
  	return ($result);
  }

  /* function takes a base64-encoded sambaMungedDial */
  function load ($mungedDial)
  {
  	$this->ctx= $this->decode_munged($mungedDial);
  }

  /* Returns ready-to-run mungedDialString to be filled into ldap */
  function getMunged ()
  {
  	// Do extra check for valid timeParams (they must be set to 0 if disabled)
  	foreach($this->timeParams as $value) {
	  	if(!isset($this->ctx[$value])) {
	  		$this->ctx[$value]= 0;
	  	}
  	}
	  $result= base64_encode($this->encode_munged($this->ctx));
	  
    return $result;
  }

  /* Returns array of flags, which can be set on-demand with activated java-script */
  function getOnDemandFlags ()
  {
  	$result= array();
  	if ($_SESSION["js"]){
  		foreach ($this->timeParams as $value) {
  			if (!isset($this->ctx[$value]) || (isset($this->ctx[$value]) && $this->ctx[$value] == 0)) {
  				$result[$value."Mode"]= "disabled";
  			} else {
  				$result[$value."Mode"]= "";
  			}
  		}
  		
  		if (substr($this->ctx['CtxCfgFlags1'], 6, 1) == "1") {
  			$result['CtxInitialProgramMode'] = "disabled";
  		} else {
  			$result['CtxInitialProgramMode'] = "";
  		}
  	}else{
  		foreach ($this->timeParams as $value) {
  			$result[$value."Mode"]= "";
  		}
  		
  		$result['CtxInitialProgramMode'] = "";
  
  	}
  
  	return $result;
  }

  /*Gets Terminal-Server-Login value: enabled/disabled */
  function getTsLogin ()
  {
   	$flags= ord(substr($this->ctx['CtxCfgFlags1'], 5, 1));
    	
  	if ($flags & 1) {
    		$result= false;
    	} else {
  	  	$result= true;
    }
  
  	return $result;
  }

  /* Sets Terminal-Server-Login value: enabled/disabled */
  function setTsLogin ($checked)
  {
  	$flag= substr($this->ctx['CtxCfgFlags1'], 5, 1);
  	
  	if ($checked) {
   		$flag|= 1;
  	} else {
   		$flag&= 0xFE;
  	}
  	
  	$this->ctx['CtxCfgFlags1'][5]= sprintf('%1x', $flag);
  }

  /* gets Broken-Connection value: disconnect/reset */
  function getBrokenConn ()
  {
  	$flags= ord(substr($this->ctx['CtxCfgFlags1'], 5, 1));
  	if ($flags & 4) {
  		$result= "1";
  	} else {
  		$result= "0";
  	}
  	
  	return $result;
  }

  /* sets Broken-Connection value: disconnect/reset */
  function setBrokenConn ($checked) 
  {
  	$flag= substr($this->ctx['CtxCfgFlags1'], 5, 1);
  	
  	if ($checked) {
  		$flag|= 4;
  	} else {
  		$flag&= 0xFB;
  	}
  	
  	$this->ctx['CtxCfgFlags1'][5]= sprintf('%1x', $flag);
  }

  /* gets Reconnection value: from any client/from previous client only */
  function getReConn ()
  {
  	$flags= ord(substr($this->ctx['CtxCfgFlags1'], 5, 1));
  	if ($flags & 2) {
  		$result= "1";
  	} else {
  		$result= "0";	
  	}
  	
  	return $result;
  }

  /* sets Reconnection value: from any client/from previous client only */
  function setReConn ($checked)
  {
  	$flag= substr($this->ctx['CtxCfgFlags1'], 5, 1);
  	
  	if ($checked) {
  		$flag|= 2;
  	} else {
  		$flag&= 0xFD;
  	}
  	
  	$this->ctx['CtxCfgFlags1'][5]= sprintf('%1x', $flag);
  }

  /* gets Inherit-config-from-client value: enabled/disabled */
  function getInheritMode ()
  {
  	if (substr($this->ctx['CtxCfgFlags1'], 6, 1) == "1") {
  		$result= true;
  	} else {
  		$result= false;
  	}
  
  	return $result;
  }

  /* sets Inherit-config-from-client value: enabled/disabled */
  function setInheritMode ($checked)
  {
  	if ($checked) {
  		$this->ctx['CtxCfgFlags1'][6]= "1";
  	} else {
  		$this->ctx['CtxCfgFlags1'][6]= "0";
  	}
  }

  /* gets shadow value (enum): 0-4
	0: disabled
	1: input on, notify on
	2: input on, notify off
	3: input off, notify on
	4: input off, notify off 
  */
  function getShadow ()
  {
    if($this->old_behavior==true) {
      $result= substr($this->ctx['CtxCfgFlags1'], 1, 1);
    } else {
    	$result= substr($this->ctx['CtxShadow'], 1, 1);
    }
  	return $result;
  }

  /* sets shadow value */
  function setShadow ($checked, $value)
  {
  	if ($checked) {
      if($this->old_behavior==true) {
        // We need to reset the old setting
       	$this->ctx['CtxCfgFlags1'][1]= sprintf('%1X', $value);
      }
  		$this->ctx['CtxShadow'][1]= sprintf('%1x', $value);
  	}
  }

  /* gets connect-client-drive-at-logon value: enabled/disabled */
  function getConnectClientDrives ()
  {
  	$connections= hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));
  	if ($connections & 8) {
  		$result= true;
  	} else {
  		$result= false;
  	}
  
  	return $result;
  }

  /* sets connect-client-drive-at-logon value: enabled/disabled */
  function setConnectClientDrives ($checked)
  {
  	$flag= hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));
  	if ($checked) {
  		$flag|= 8;
  	} else {
  		$flag&= 0xF7;
  	}
  
  	$this->ctx['CtxCfgFlags1'][2]= sprintf('%1x', $flag);
  }

  /* gets connect-client-printers-at-logon value: enabled/disabled */
  function getConnectClientPrinters ()
  {
  	$connections= hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));
  	if ($connections & 4) {
  		$result= true;
  	} else {
  		$result= false;
  	}
  	
  	return $result;
  }

  /* sets connect-client-printers-at-logon value: enabled/disabled */
  function setConnectClientPrinters ($checked)
  {
  	$flag= hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));
  	
  	if ($checked) {
  		$flag|= 4;
  	} else {
  		$flag&= 0xFB;
  	}
  	
  	$this->ctx['CtxCfgFlags1'][2]= sprintf('%1x', $flag);
  }

  /* gets set-client-printer-to-default value: enabled/disabled */
  function getDefaultPrinter ()
  {
  	$connections= hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));
  	if ($connections & 2) {
  		$result= true;
  	} else {
  		$result= false;
  	}
  
  	return $result;
  }

  /* sets set-client-printer-to-default value: enabled/disabled */
  function setDefaultPrinter ($checked)
  {
  	$flag= hexdec(substr($this->ctx['CtxCfgFlags1'], 2, 1));
  	
  	if ($checked) {
  		$flag|= 2;
  	} else {
  		$flag&= 0xFD;
  	}
  	
  	$this->ctx['CtxCfgFlags1'][2]= sprintf('%1x', $flag);
  }

  /* SMARTY: gets the checkbox state of "Connection" */
  function getCtxMaxConnectionTimeF ()
  {
  	// Connection Time is 0 if disabled
  	if (isset($this->ctx['CtxMaxConnectionTime']) && ($this->ctx['CtxMaxConnectionTime'] != 0)) {
  		$result= true;
  	} else {
  		$result= false;
  	}
  
  	return $result;
  }

  /* SMARTY: sets the checkbox "Connection" to unchecked */
  function setCtxMaxConnectionTimeF ($checked)
  {
  	if ($checked) {
  		unset ($this->ctx['CtxMaxConnectionTime']);
  	}
  }

  /* SMARTY: gets the checkbox state of "Disconnection" */
  function getCtxMaxDisconnectionTimeF ()
  {
  	// Connection Time is 0 if disabled
  	if (isset($this->ctx['CtxMaxDisconnectionTime']) && ($this->ctx['CtxMaxDisconnectionTime'] != 0)) {
  		$result= true;
  	} else {
  		$result= false;
  	}

  	return $result;
  }

  /* SMARTY: sets the checkbox "Disconnection" to unchecked */
  function setCtxMaxDisconnectionTimeF ($checked)
  {
  	if ($checked) {
  		unset ($this->ctx['CtxMaxDisconnectionTime']);
  	} 
  }

  /* SMARTY: gets the checkbox state of "Idle" */
  function getCtxMaxIdleTimeF ()
  {
  	// Connection Time is 0 if disabled
  	if (isset($this->ctx['CtxMaxIdleTime']) && ($this->ctx['CtxMaxIdleTime'] != 0)) {
  		$result= true;
  	} else {
  		$result= false;
  	}
  
  	return $result;
  }

  /* SMARTY: sets the checkbox "Idle" to unchecked */
  function setCtxMaxIdleTimeF ($checked)
  {
  	if ($checked) {
  		unset ($this->ctx['CtxMaxIdleTime']);
  	}
  }
}

?>
