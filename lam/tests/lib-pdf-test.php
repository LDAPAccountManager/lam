<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Tilo Lutz

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


  LDAP Account Manager test for lib/pdf.inc
*/
include_once("../lib/account.inc");
include_once("../lib/pdf.inc");

$accounts = array();
$account = new Account();
$account->type = "user";
$account->general_username = "mamu1";
$account->general_uidnumber = "501";
$account->general_surname = "Mustermann";
$account->general_givenname = "Max";
$account->general_group = "tg1";
$account->general_groupadd = array("tg2","tg3");
$account->general_homedir = "/home/m/mamu1";
$account->general_shell = array("/bin/bash","/bin/sh");
$account->unix_password = "secret1";
$account->unix_password_no = "1";
$account->smb_password_no = "1";
array_push($accounts,$account);
$account = new Account();
$account->type = "user";
$account->general_username = "mamu1";
$account->general_uidnumber = "501";
$account->general_surname = "Mustermann";
$account->general_givenname = "Max";
$account->general_group = "tg1";
$account->general_groupadd = array("tg2","tg3");
$account->general_homedir = "/home/m/mamu1";
$account->general_shell = array("/bin/bash");
$account->unix_password = "secret1";
$account->unix_password_no = "0";
$account->smb_useunixpwd = "1";
array_push($accounts,$account);
$account = new Account();
$account->type = "user";
$account->general_username = "mamu1";
$account->general_uidnumber = "501";
$account->general_surname = "Mustermann";
$account->general_givenname = "Max";
$account->general_group = "tg1";
$account->general_groupadd = array("tg2","tg3");
$account->general_homedir = "/home/m/mamu1";
$account->general_shell = array("/bin/bash","/bin/sh");
$account->unix_password = "secret1";
$account->unix_password_no = "0";
$account->smb_useunixpwd = "0";
array_push($accounts,$account);

createUserPDF($accounts);
?>
