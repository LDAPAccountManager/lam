<?php 
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Roland Gruber

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

	This is the main window. The user and group lists will be shown in this frame.  
  
*/

session_start();
?>
 
<html>
	<head>
		<title>LDAP Account Manager</title>
	</head>
		<frameset rows="150,*">
			<frame src="./main_header.php" name="head">
			<frame src="./lists.php?list=users" name="mainpart">
		<noframes>
			This page requires a browser that can show frames!
		</noframes>
	</frameset>
</html>
