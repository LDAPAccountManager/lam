<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Leonhard Walchshäusl

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more detaexils.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


// class representing local user entry with attributes of ldap user entry
class UserEntry {

  var $uid;
  var $cn;
  var $givenName;
  var $sn;
  var $homeDirectory;

  function UserEntry () {
  }

  function getSn() {
    return $this->sn;
  }

  function setSn ($in_value) {
    $this->sn = $in_value;
  }

  function getCn() {
    return $this->cn;
  }

  function setCn ($in_value) {
    $this->cn = $in_value;
  }

  function getGivenName() {
    return $this->givenName;
  }

  function setGivenName ($in_value) {
    $this->givenName = $in_value;
  }

  function getUid() {
    return $this->uid;
  }

  function setUid ($in_value) {
    $this->uid = $in_value;
  }

  function getHomeDirectory() {
    return $this->homeDirectory;
  }

  function setHomeDirectory ($in_value) {
    $this->homeDirectory = $in_value;
  }

}

?>
 
