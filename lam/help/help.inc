<?php
/*
$Id$

  This code is part of LDAP Account Manager (http://www.sourceforge.net/projects/lam)
  Copyright (C) 2003  Michael Duergner

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


  LDAP Account Manager help topics.
*/

$helpArray = array (
				// 0 - 99
				// any developer
				// for testing purposes
				"0" => array ("ext" => "FALSE", "Headline" => _("Hilfetest 1"), "Text" => _("blablabla 1"), "SeeAlso" => "<a href=\"./help.php?HelpNumber=02\" target=\"_self\">02</a>"),
				"02" => array ("ext" => "FALSE", "Headline" => _("Hilfetest 2"), "Text" => _("blablabla 2"), "SeeAlso" => ""),
				"03" => array ("ext" => "TRUE", "Link" => "help/extFileExample.php"),
				// 200 - 299
				// Roland Gruber
				// configuration wizard
				// configuration login
				"200" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Login"),
					"Text" => _("Please enter the configuration password. This is NOT your LDAP password.".
						" It is stored in lam.conf. If this is the first time you log in, enter \"lam\".")),
				"201" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Server Address"),
					"Text" => _("This is the Server Address of your LDAP server. Use ldap:// for standard LDAP connections".
						" and ldaps:// for encrypted (require server certificates) connections. The port value is optional.<br><br>".
						"<b>Examples:</b><br><br> ldap://localhost:389 connects to localhost using a standard LDAP connection on port 389<br>".
						"ldaps://141.40.146.133 connects to 141.40.146.133 using an encrypted LDAP connection.<br><br>".
						"<br><b>Note:</b><br><br>When using ldaps:// be sure to use exactly the same IP/domain name as in your certificate!")),
				"202" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - User/Group/Host suffix"),
					"Text" => _("This is the suffix of the LDAP tree from where to search for user/group/host entries.".
						" Only entries in these subtrees will be displayed in the user/group/host list. When creating a".
						" new accont this will be the DN where it is saved.<br><br><b>Examples:</b><br><br>ou=People,dc=yourcompany,dc=com".
						" will read and store all accounts in this subtree.")),
				"203" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - UID Number"),
					"Text" => _("These are the minimum and maximum numbers to use for user IDs when creating new user accounts.".
						"The range has to be different from that of machines. New user accounts will always get the highest number in use plus one.")),
				"204" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - GID Number"),
					"Text" => _("These are the minimum and maximum numbers to use for group IDs when creating new group accounts.".
						"New group accounts will always get the highest number in use plus one.")),
				"205" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Machine Number"),
					"Text" => _("These are the minimum and maximum numbers to use for machine IDs when creating new accounts for Samba hosts.".
						"The range has to be different from that of users. New host accounts will always get the highest number in use plus one.")),
				"206" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - List Attributes"),
					"Text" => _("This is the list of attributes to show in the user/group/host list.".
						" The entries can either be predefined values, \"#value\", or individual ones, \"value:description\".".
						"Several entries are seperated by semicolons.<br><br>".
						"<b>Example: </b>#homeDirectory;#uid;#cn;#uidNumber;#gidNumber;description:UserDescription<br><br> ".
						"<br><u><b>Predefined values:</b></u><br>".
						"<br><b>Users: </b>#uid, #uidNumber, #gidNumber, #cn, #host, #givenName, #sn, #homeDirectory, #loginShell, #mail, #gecos".
						"<br><b>Groups: </b>#cn, #gidNumber, #memberUID, #member, #description".
						"<br><b>Groups: </b>#uid, #cn, #rid, #description")),
				"207" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Valid Users"),
					"Text" => _("This is a list of valid DN entries of all users that are allowed to login to LDAP Account Manager.".
						"The user names have to be separated by semicolons.<br><br>".
						"<b>Example: </b>cn=admin,dc=yourdomain,dc=org;cn=manager,dc=yourdomain,dc=org")),
				"208" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Maximum List Entries"),
					"Text" => _("This is the number of rows to show in the user/group/host list. If more entries are found the list".
					" will be split to several pages.")),
				"209" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Default Language"),
					"Text" => _("Defines the language of the login window and is sets this language the default language.".
					" Users can change the language at login.")),
				"210" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Script Path"),
					"Text" => _("This is the absolute path to an external script for setting quotas and creating home directories.".
						"<br><br><b>Use it at your own risk and read the documentation for lamdaemon before you use it!</b>"),
						"SeeAlso" => "<a href=\"./help.php?HelpNumber=210\" target=\"_self\">TODO link to lamdaemon doku</a>"),
				"211" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Script Server"),
					"Text" => _("This is the server where the lamdaemon script is stored. LDAP Account Manager will make a SSH connection".
						" to this server with username and password provided at login.".
						"<br><br><b>Use it at your own risk and read the documentation for lamdaemon before you use it!</b>"),
						"SeeAlso" => "<a href=\"./help.php?HelpNumber=210\" target=\"_self\">TODO link to lamdaemon doku</a>"),
				"212" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Change Password"),
					"Text" => _("If you want to change the current preferences password, please enter it here."))
/* This is a sample help entry. Just copy this line an modify the vakues between the [] brackets.
	Help text is located in the array:
		"[Helpnumber]" => array ("ext" => "FALSE", "Headline" => _("[Headline]"), "Text" => _("[Text]"), "SeeAlso" => "[SeeAlso link]"),
	Help text is located in an extra file; path must be relative to lam_root; File should be stored in help directory
		"[Helpnumber]" => array ("ext" => "TRUE", "Link" => "[help/pathToFile]),
*/
				);
?>
