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
				"03" => array ("ext" => "TRUE", "Link" => "extFileExample.php"),
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
					"Text" => _("If you want to change the current preferences password, please enter it here.")),
				"213" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Samba version"),
					"Text" => _("If you use Samba 3.x with the new LDAP schema say \"yes\" here, otherwise \"no\".".
						"<br><br><b>LAM will not work if version is wrong!</b>")),
				"214" => array ("ext" => "FALSE", "Headline" => _("Configuration Wizard - Samba domain SID"),
					"Text" => _("If you use the new Samba 3.x schema enter your domain SID here.".
						"<br><br>You can get your domain SID with this command: <b>net getlocalsid</b>")),
				// 300 - 399
				// Roland Gruber
				// profile editor
				"300" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Set Samba Password"),
					"Text" => _("If set to \"no\" the NT and LM passwords will be set to \"NO PASSWORD*****\" and the user will be unable to login.".
						"<br><br>If unsure say \"yes\" here.")),
				"301" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Set Unix Password for Samba"),
					"Text" => _("If set to \"yes\" the Windows password will be the same as the Unix one.".
						"<br><br>If unsure say \"yes\" here.")),
				"302" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - User can change password"),
					"Text" => _("If set to \"yes\" the user will be able to change his Windows password.".
						"<br><br>If unsure say \"yes\" here.")),
				"303" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - User must change password"),
					"Text" => _("If set to \"yes\" the user will need to change his Windows password at next login.".
						"<br><br>If unsure say \"no\" here.")),
				"304" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Home drive"),
					"Text" => _("This is the drive letter for the user's home drive. The home directory will be connected with this drive letter at login."),
					"SeeAlso" => "<a href=\"./help.php?HelpNumber=305\" target=\"_self\">Path to home directory</a>"),
				"305" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Path to home directory"),
					"Text" => _("This is the path to the user's home directory. The home directory will be connected at login."),
					"SeeAlso" => "<a href=\"./help.php?HelpNumber=304\" target=\"_self\">Home drive</a>"),
				"306" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Profile path"),
					"Text" => _("This is the path to the user's profile directory. If you do not want profiles to be saved on the server leave this empty.")),
				"307" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Script path"),
					"Text" => _("This is the path to the logon script directory.")),
				"308" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - User Workstations"),
					"Text" => _("These are the workstations on which the user is allowed to login. They have to be seperated by spaces".
						" (e.g. \"PC01 PC09 PC34\"). If the user is allowed to login on all workstations, leave this field empty.")),
				"309" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Domain"),
					"Text" => _("This is the Windows domain name.")),
				"330" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Primary group"),
					"Text" => _("This is the user's primary group.")),
				"331" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Additional groups"),
					"Text" => _("Other groups the user will be member of.")),
				"332" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Home Directory"),
					"Text" => _("This is the path to the user's Unix home directory.")),
				"333" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Login Shell"),
					"Text" => _("This is the user's login shell. Set to \"/bin/bash\" if unsure.")),
				"334" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Set Unix Password"),
					"Text" => _("If set to \"no\" the Unix password will not be set and the user will be unable to login.".
						"<br><br>If unsure say \"yes\" here.")),
				"335" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Password warning"),
					"Text" => _("Number of days a user will be warned before his password expires.")),
				"336" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Password expiry"),
					"Text" => _("Number of days a user will be able to login after his password expired.")),
				"337" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Maximum password age"),
					"Text" => _("Number of days after a user will be forced to change his password.")),
				"338" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Minimum password age"),
					"Text" => _("Number of days a user has to wait to change his password again.")),
				"339" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Account expires on"),
					"Text" => _("After this date the user will no longer be able to login.")),
				"340" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Account is deactivated"),
					"Text" => _("If set to \"yes\" the user will not be able to login. This does not affect the Windows account.")),
				"360" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Profile Name"),
					"Text" => _("Name under which the profile will be saved. If a profile with the same name exists, it will be overwritten.")),
				"370" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Primary Group"),
					"Text" => _("This is primary group for the Samba machine account.")),
				"371" => array ("ext" => "FALSE", "Headline" => _("Profile Editor - Domain"),
					"Text" => _("This is the Windows domain name.")),
				// 400 - 499
				// Tilo Lutz
				// account.php
				"400" => array ("ext" => "FALSE", "Headline" => _("Username"),
					"Text" => _("Username of the user which should be created. Valid characters are: a-z,0-9, .-_. Lam doesn't allow a number as first
					 character because it's impossible to create a homedirectory starting with a number. Lam doesn't allow capital letters A-Z because it
					 can cause several problems. If username is allready used username will expanded with a number. The next free number will be used.")),
				"401" => array ("ext" => "FALSE", "Headline" => _("UID Number"),
					"Text" => _("If empty UID Number will be generated automaticly. Valid values are between ").$_SESSION['config']->get_minUID()._(" and ").$_SESSION['config']->get_maxUID()."."),
				"402" => array ("ext" => "FALSE", "Headline" => _("Additional Groupmembership"),
					"Text" => _("Can be left empty. Hold the CTRL-key to (de)select multiple groups.")),
				"403" => array ("ext" => "FALSE", "Headline" => _("Home Directory"),
					"Text" => _("$user and $group are replaced with username or primary groupname.")),
				"404" => array ("ext" => "FALSE", "Headline" => _("Gecos"),
					"Text" => _("User descriptopn. If left empty sur- and givename will be used.")),
				"405" => array ("ext" => "FALSE", "Headline" => _("Login shell"),
					"Text" => _("To disable login use /bin/false.")),
				"406" => array ("ext" => "FALSE", "Headline" => _("Primary Group"),
					"Text" => _("The Primary Group the user should be member of.")),
				"407" => array ("ext" => "FALSE", "Headline" => _("Groupname"),
					"Text" => _("Groupname of the group which should be created. Valid characters are: a-z,0-9, .-_. Lam doesn't allow a number as first
					 character because it's impossible to create a homedirectory starting with a number. Lam doesn't allow capital letters A-Z because it
					 can cause several problems. If groupname is allready used username will expanded with a number. The next free number will be used.")),
				"408" => array ("ext" => "FALSE", "Headline" => _("GID Number"),
					"Text" => _("If empty GID Number will be generated automaticly. Valid values are between ").$_SESSION['config']->get_minGID()._(" and ").$_SESSION['config']->get_maxGID()."."),
				"409" => array ("ext" => "FALSE", "Headline" => _("Gecos"),
					"Text" => _("Group descriptopn. If left empty groupname will be used.")),
				"410" => array ("ext" => "FALSE", "Headline" => _("Hostname"),
					"Text" => _("Groupname of the group which should be created. Valid characters are: a-z,0-9, .-_$. Lam doesn't allow a number as first
					 character because it's impossible to create a homedirectory starting with a number. Lam doesn't allow capital letters A-Z because it
					 can cause several problems. Hostnames are always ending with $. If last character isn't $ it will be added. If hostname is allready used username will expanded with a number. The next free number will be used.")),
				"411" => array ("ext" => "FALSE", "Headline" => _("UID Number"),
					"Text" => _("If empty UID Number will be generated automaticly. Valid values are between ").$_SESSION['config']->get_minMachine()._(" and ").$_SESSION['config']->get_maxMachine()."."),
				"412" => array ("ext" => "FALSE", "Headline" => _("Primary Group"),
					"Text" => _("The Prmary Group the host should be member of.")),
				"413" => array ("ext" => "FALSE", "Headline" => _("Gecos"),
					"Text" => _("Host descriptopn. If left empty hostname will be used.")),
				"414" => array ("ext" => "FALSE", "Headline" => _("Password Warn"),
					"Text" => _("Days before password is to expire that user is warned of pending password expiration. Value must be 0<.")),
				"415" => array ("ext" => "FALSE", "Headline" => _("Password Expire"),
					"Text" => _("Number of days a user can login even his password has expired. -1=always")),
				"416" => array ("ext" => "FALSE", "Headline" => _("Maximum Passwordage"),
					"Text" => _("Number of days after a user has to change his password again Value must be 0<.")),
				"417" => array ("ext" => "FALSE", "Headline" => _("Minimum Passwordage"),
					"Text" => _("Number of days a user has to wait until he\'s allowed to change his password again. Value must be 0<.")),
				"418" => array ("ext" => "FALSE", "Headline" => _("Expire date"),
					"Text" => _("Account expire date.")),
				"419" => array ("ext" => "FALSE", "Headline" => _("Password Warn"),
					"Text" => _("Days before password is to expire that host is warned of pending password expiration. Value must be 0<.")),
				"420" => array ("ext" => "FALSE", "Headline" => _("Password Expire"),
					"Text" => _("Number of days a host can login even his password has expired. -1=always")),
				"421" => array ("ext" => "FALSE", "Headline" => _("Maximum Passwordage"),
					"Text" => _("Number of days after a host has to change his password again Value must be 0< and should be higher as the value on client-side.")),
				"422" => array ("ext" => "FALSE", "Headline" => _("Minimum Passwordage"),
					"Text" => _("Number of days a host has to wait until he\'s allowed to change his password again. Value must be 0<.")),
				"423" => array ("ext" => "FALSE", "Headline" => _("Expire Date"),
					"Text" => _("Account expire date.")),
				"424" => array ("ext" => "FALSE", "Headline" => _("Surname"),
					"Text" => _("Surname")),
				"425" => array ("ext" => "FALSE", "Headline" => _("Givenname"),
					"Text" => _("Givenname")),
				"426" => array ("ext" => "FALSE", "Headline" => _("Use no password"),
					"Text" => _("Use no password")),
				"427" => array ("ext" => "FALSE", "Headline" => _("Account deactivated"),
					"Text" => _("Account deactivated")),
				"428" => array ("ext" => "FALSE", "Headline" => _("Use no Password"),
					"Text" => _("Use no Password.")),
				"429" => array ("ext" => "FALSE", "Headline" => _("Password doesn't expire"),
					"Text" => _("Password doesn't expire.")),
				"430" => array ("ext" => "FALSE", "Headline" => _("User can change password"),
					"Text" => _("User can change password.")),
				"431" => array ("ext" => "FALSE", "Headline" => _("User must change password"),
					"Text" => _("User must change password.")),
				"432" => array ("ext" => "FALSE", "Headline" => _("Account is deactivated"),
					"Text" => _("Account is deactivated.")),
				"433" => array ("ext" => "FALSE", "Headline" => _("Home Drive"),
					"Text" => _("Driveletter assigned on Windows-Workstations as Homedirectory.")),
				"434" => array ("ext" => "FALSE", "Headline" => _("Script Path"),
					"Text" => _("Filename and -path relative to netlogon-share which should be executed on logon. $user and $group are replaced with user- and groupname. Can be left empty.")),
				"435" => array ("ext" => "FALSE", "Headline" => _("Profile Path"),
					"Text" => _("Path of the userprofile. Can be a local absolute path or a UNC-path (\\\\server\\share). $user and $group are replaced with user- and groupname. Can be left empty.")),
				"436" => array ("ext" => "FALSE", "Headline" => _("User Workstations"),
					"Text" => _("Komma-separated list of workstations the user is allowed to login. Empty means every workstation. Can be left empty.")),
				"437" => array ("ext" => "FALSE", "Headline" => _("smb Home"),
					"Text" => _("UNC-path (\\\\server\\share) of homedirectory. $user and $group are replaced with user- and groupname. Can be left empty.")),
				"438" => array ("ext" => "FALSE", "Headline" => _("Domain"),
					"Text" => _("Windows-Domain of user. Can be left empty.")),
				"439" => array ("ext" => "FALSE", "Headline" => _("Mountpoint"),
					"Text" => _("Mountpoint")),
				"440" => array ("ext" => "FALSE", "Headline" => _("Used blocks"),
					"Text" => _("Used blocks.")),
				"441" => array ("ext" => "FALSE", "Headline" => _("soft block limit"),
					"Text" => _("soft block limit")),
				"442" => array ("ext" => "FALSE", "Headline" => _("hard block limit"),
					"Text" => _("hard block limit")),
				"443" => array ("ext" => "FALSE", "Headline" => _("grace block period"),
					"Text" => _("grace block period")),
				"444" => array ("ext" => "FALSE", "Headline" => _("used inodes"),
					"Text" => _("used inodes")),
				"445" => array ("ext" => "FALSE", "Headline" => _("soft inode limit"),
					"Text" => _("sofr inode limit")),
				"446" => array ("ext" => "FALSE", "Headline" => _("hard inode limit"),
					"Text" => _("hard inode limit")),
				"447" => array ("ext" => "FALSE", "Headline" => _("grace inode period"),
					"Text" => _("grace inode period")),
				"448" => array ("ext" => "FALSE", "Headline" => _("Title"),
					"Text" => _("Title")),
				"449" => array ("ext" => "FALSE", "Headline" => _("Employee Type"),
					"Text" => _("Employee Type")),
				"450" => array ("ext" => "FALSE", "Headline" => _("Street"),
					"Text" => _("Street")),
				"451" => array ("ext" => "FALSE", "Headline" => _("Postal Code"),
					"Text" => _("Postal Code")),
				"452" => array ("ext" => "FALSE", "Headline" => _("Postal Address"),
					"Text" => _("Postal Address")),
				"453" => array ("ext" => "FALSE", "Headline" => _("Telephone Number"),
					"Text" => _("Telephone Number")),
				"454" => array ("ext" => "FALSE", "Headline" => _("Mobile Phonenumber"),
					"Text" => _("Mobile Phonenumber")),
				"455" => array ("ext" => "FALSE", "Headline" => _("Facsimile Number"),
					"Text" => _("Facsimile Number")),
				"456" => array ("ext" => "FALSE", "Headline" => _("eMail Address"),
					"Text" => _("eMail Address")),
				"457" => array ("ext" => "FALSE", "Headline" => _("Save Profile"),
					"Text" => _("Save Profile")),
				"458" => array ("ext" => "FALSE", "Headline" => _("Host can change Password"),
					"Text" => _("Host can change Password")),
				"459" => array ("ext" => "FALSE", "Headline" => _("Host must change Password"),
					"Text" => _("Host must change Password")),
				"460" => array ("ext" => "FALSE", "Headline" => _("Windows Domain"),
					"Text" => _("Windows Domain of host. Can be left empty.")),
				"461" => array ("ext" => "FALSE", "Headline" => _("Suffix"),
					"Text" => _("Suffix")),
				"462" => array ("ext" => "FALSE", "Headline" => _("Suffix"),
					"Text" => _("Suffix")),
				"463" => array ("ext" => "FALSE", "Headline" => _("Suffix"),
					"Text" => _("Suffix"))
/* This is a sample help entry. Just copy this line an modify the vakues between the [] brackets.
	Help text is located in the array:
		"[Helpnumber]" => array ("ext" => "FALSE", "Headline" => _("[Headline]"), "Text" => _("[Text]"), "SeeAlso" => "[SeeAlso link]"),
	Help text is located in an extra file; File must be stored in help directory; Link has to be the path to the file if it is stored in a subdirectory and the filename with a leading slash;
		"[Helpnumber]" => array ("ext" => "TRUE", "Link" => "[path/FileName]),
*/
				);
?>
