<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.ldap-account-manager.org/)
  
  This code is based on phpLDAPadmin.
  Copyright (C) 2004  David Smith and phpLDAPadmin developers
  
  The original code was modified to fit for LDAP Account Manager by Roland Gruber.
  Copyright (C) 2005  Roland Gruber

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
 * @package lists
 * @subpackage tree
 * @author The phpLDAPadmin development team
 * @author Roland Gruber
 */

/** security functions */
include_once('../../lib/security.inc');
/** export functions */
require '../../lib/export.inc';
/** common functions */
require '../../lib/tree.inc';
/** access to configuration */
include_once('../../lib/config.inc');
/** LDAP functions */
include_once('../../lib/ldap.inc');
/** status messages */
include_once('../../lib/status.inc');

// start session
startSecureSession();

setlanguage();

$base_dn = isset($_POST['dn']) ? $_POST['dn']:NULL;
$format = isset( $_POST['format'] ) ? $_POST['format'] : "unix";
$scope = isset($_POST['scope']) ? $_POST['scope'] : 'base';
$filter = isset($_POST['filter']) ? $_POST['filter'] : 'objectclass=*';
$target = isset($_POST['target']) ? $_POST['target'] : 'display';
$save_as_file = isset( $_POST['save_as_file'] ) &&  $_POST['save_as_file'] == 'on';

// add system attributes if needed
$attributes = array();
if( isset( $_POST['sys_attr'] ) ){
  array_push($attributes,'*');
  array_push($attributes,'+');
}

$exporter_id = $_POST['exporter_id'];

// Initialisation of other variables
$rdn = get_rdn( $base_dn );
$friendly_rdn = get_rdn( $base_dn, 1 );
$extension = $exporters[$exporter_id]['extension'];

//set the default CRLN to Unix format
$br = "\n";

// default case not really needed
switch( $format ) {
 case 'win': 
   $br = "\r\n"; 
   break;
 case 'mac':
   $br = "\r";
   break;
 case 'unix':
 default:	
   $br = "\n";
}

// get the decoree,ie the source
$plaLdapExporter = new PlaLdapExporter($filter,$base_dn,$scope,$attributes);

// the decorator 
// do it that way for the moment
$exporter = NULL;

switch($exporter_id){
 case 0:
   $exporter = new PlaLdifExporter($plaLdapExporter);
   break;
 case 1:
   $exporter = new PlaDsmlExporter($plaLdapExporter);
   break;
 case 2:
   $exporter = new PlaVcardExporter($plaLdapExporter);
   break;
 case 3:
   $exporter = new PlaCSVExporter($plaLdapExporter);
   break;
}

// set the CLRN
$exporter->setOutputFormat($br);

// prevent script from bailing early for long search
@set_time_limit( 0 );

// send the header
if( $save_as_file ) 
  header( "Content-type: application/download" );
else
  header( "Content-type: text/plain" );
header( "Content-Disposition: filename=$friendly_rdn.".$exporters[$exporter_id]['extension'] ); 
header( "Expires: Mon, 26 Jul 1997 05:00:00 GMT" ); 
header( "Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT" ); 
header( "Cache-Control: post-check=0, pre-check=0", false );

// and export
$exporter->export();
?>
