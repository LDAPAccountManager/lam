<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  
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
 * Configuration for creation templates.
 *
 * @package lists
 * @subpackage tree
 * @author David Smith
 * @author Roland Gruber
*/ 


// template list

$templates = array();

$templates[] = array(
	'desc' => 'Organizational Unit',
	'icon' => '../../graphics/ou.png',
	'handler' => 'new_ou_template.php');

$templates[] = array(
	'desc' => 'Custom',
	'icon' => '../../graphics/object.png',
	'handler' => 'custom.php');


?>