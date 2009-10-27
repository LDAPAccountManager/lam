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
 * Adds an attribute/value pair to an object
 *
 * Variables that come in as POST vars:
 *  - dn
 *  - attr
 *  - val
 *  - binary
 *
 * @package lists
 * @subpackage tree
 * @author David Smith
 * @author Roland Gruber
 */

/** security functions */
include_once('../../lib/security.inc');
/** tree functions */
include_once('../../lib/tree.inc');
/** access to configuration */
include_once('../../lib/config.inc');
/** LDAP functions */
include_once('../../lib/ldap.inc');
/** status messages */
include_once('../../lib/status.inc');
/** common functions */
include_once('../../lib/account.inc');

// start session
startSecureSession();

// die if no write access
if (!checkIfWriteAccessIsAllowed()) die();

setlanguage();

$attr = $_POST['attr'];
$val  = isset( $_POST['val'] ) ? $_POST['val'] : false;;
$dn = $_POST['dn'] ;
$encoded_dn = rawurlencode( $dn );
$encoded_attr = rawurlencode( $attr );
$is_binary_val = isset( $_POST['binary'] ) ? true : false;

if( ! $is_binary_val && $val == "" ) {
	echo $_SESSION['header'];
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
	echo "</head><body>\n";
	StatusMessage("ERROR", _('You left the attribute value blank. Please go back and try again.'), '');
	echo "</body></html>";
	exit;
}

// special case for binary attributes (like jpegPhoto and userCertificate): 
// we must go read the data from the file and override $val with the binary data
// Secondly, we must check if the ";binary" option has to be appended to the name
// of the attribute.

if( $is_binary_val ) {
    if (( 0 == $_FILES['val']['size'] ) || (! is_uploaded_file( $_FILES['val']['tmp_name'] ))) {
		echo $_SESSION['header'];
		echo "<title>LDAP Account Manager</title>\n";
		echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
		echo "</head><body>\n";
		StatusMessage("ERROR", _('File upload failed!'), '');
		echo "</body></html>";
		exit;
    }
	$file = $_FILES['val']['tmp_name'];
    $f = fopen( $file, 'r' );
    $binary_data = fread( $f, filesize( $file ) );
    fclose( $f );
    $val = $binary_data;

	if( is_binary_option_required( $attr ) )
	  $attr .=";binary";
}

// Automagically hash new userPassword attributes according to the 
// chosen in config.php. 
if( 0 == strcasecmp( $attr, 'userpassword' ) ) {
		$val = pwd_hash($val);
}
elseif(0 == strcasecmp( $attr , 'sambalmpassword') ) {
	$val = lmPassword($val);
}
elseif (0 == strcasecmp( $attr , 'sambantpassword' )) {
	$val = ntPassword($val);
}

$ds = $_SESSION['ldap']->server();
$new_entry = array( $attr => $val );
$result = @ldap_mod_add( $ds, $dn, $new_entry );

if( $result )
     header( "Location: edit.php?dn=$encoded_dn&amp;modified_attrs[]=$encoded_attr" );
else {
	echo $_SESSION['header'];
	echo "<title>LDAP Account Manager</title>\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
	echo "</head><body>\n";
	StatusMessage("ERROR", _('Failed to add the attribute.'), ldap_error($ds));
	echo "</body></html>";
	exit;
}

// check if we need to append the ;binary option to the name 
// of some binary attribute

function is_binary_option_required( $attr ){

  // list of the binary attributes which need the ";binary" option
  $binary_attributes_with_options = array(
      // Superior: Ldapv3 Syntaxes (1.3.6.1.4.1.1466.115.121.1)
      '1.3.6.1.4.1.1466.115.121.1.8'  =>  "userCertificate",
      '1.3.6.1.4.1.1466.115.121.1.8'  =>  "caCertificate",
      '1.3.6.1.4.1.1466.115.121.1.10' =>  "crossCertificatePair",
      '1.3.6.1.4.1.1466.115.121.1.9'  =>  "certificateRevocationList",
      '1.3.6.1.4.1.1466.115.121.1.9'  =>  "authorityRevocationList",
      // Superior: Netscape Ldap attributes types (2.16.840.1.113730.3.1)
      '2.16.840.1.113730.3.1.40'      =>  "userSMIMECertificate" 
  );
  
  // quick check by attr name (short circuits the schema check if possible)
  //foreach( $binary_attributes_with_options as $oid => $name )
    //if( 0 == strcasecmp( $attr, $name ) )
        //return true;

  $schema_attr = get_schema_attribute( $attr );
  if( ! $schema_attr )
    return false;

  $syntax = $schema_attr->getSyntaxOID();
  if( isset( $binary_attributes_with_options[ $syntax ] ) )
    return true;

  return false;
}

?>
