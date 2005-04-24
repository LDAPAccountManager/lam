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

/**
 * LDAP Account Manager help entries.
 * 
 * @author Michael Dürgner
 * @version 0.5
 * @package Help
 * @copyright Copyright (C) 2003-2004 Michael Dürgner
 * @license GPL
 */

setlanguage();

$helpArray = array (
				// 0 - 99
				// any developer
				// 200 - 299
				// Roland Gruber
				// configuration wizard
				// configuration login
				// config profile management
				"200" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Login"),
					"Text" => _("Please enter the configuration password. This is NOT your LDAP password. It is stored in your .conf-file. If this is the first time you log in, enter \"lam\".")),
				"201" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Server address"),
					"Text" => _("This is the server address of your LDAP server. Use ldap:// for standard LDAP connections and ldaps:// for encrypted (require server certificates) connections. The port value is optional.") .
						"<br><br><b> " .
						_("Examples") .
						":</b><br><br> " .
						_("ldap://localhost:389 connects to localhost using a standard LDAP connection on port 389") .
						"<br> " .
						_("ldaps://141.40.146.133 connects to 141.40.146.133 using an encrypted LDAP connection.") .
						"<br><br><br><b> " .
						_("Note") .
						":</b><br><br>" .
						_("When using ldaps:// be sure to use exactly the same IP/domain name as in your certificate!")),
				"202" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("User/Group/Host suffix"),
					"Text" => _("This is the suffix of the LDAP tree from where to search for user/group/host entries. Only entries in these subtrees will be displayed in the user/group/host list. When creating a new accont this will be the DN where it is saved.") .
						"<br><br><b>".
						_("Examples").
						":</b><br><br>".
						_("ou=People,dc=yourcompany,dc=com will read and store all accounts in this subtree.")),
				"203" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Tree suffix"),
					"Text" => _("This is the suffix for the LDAP tree viewer.") .
						"<br><br><b>".
						_("Example").
						":</b><br><br>".
						_("dc=yourcompany,dc=com")),
				"206" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("List attributes"),
					"Text" => _("This is the list of attributes to show in the user/group/host list. The entries can either be predefined values, \"#attribute\", or individual ones, \"attribute:description\". Several entries are seperated by semicolons.") .
						"<br><br><b>" .
						_("Example") .
						": </b>#homeDirectory;#uid;#uidNumber;#gidNumber;mail:Mail address<br><br><br><u><b>" .
						_("Predefined values") .
						":</b></u><br><br><b>" .
						_("Users") .
						": </b>#uid, #uidNumber, #gidNumber, #cn, #host, #givenName, #sn, #homeDirectory, #loginShell, #mail, #gecos".
						"<br><b>" .
						_("Groups") .
						": </b>#cn, #gidNumber, #memberUID, #member, #description".
						"<br><b>" .
						_("Hosts") .
						": </b>#uid, #cn, #rid, #description"),
				"207" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Valid users"),
					"Text" => _("This is a list of valid DN entries of all users that are allowed to login to LDAP Account Manager. The user names have to be separated by semicolons.") .
						"<br><br><b>" .
						_("Example") .
						": </b>cn=admin,dc=yourdomain,dc=org;cn=manager,dc=yourdomain,dc=org"),
				"208" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Maximum list entries"),
					"Text" => _("This is the number of rows to show in the user/group/host list. If more entries are found the list will be split into several pages.")),
				"209" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Default language"),
					"Text" => _("This defines the language of the login window and sets this language as the default language. Users can change the language at login.")),
				"210" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Script path"),
					"Text" => _("This is the absolute path to an external script for setting quotas and creating home directories.").
							"<br><br><b>".
							_("Use it at your own risk and read the documentation for lamdaemon before you use it!").
							"</b>"),
				"211" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Script server"),
					"Text" => _("This is the server where the lamdaemon script is stored. LDAP Account Manager will make a SSH connection to this server with username and password provided at login.").
						"<br><br><b>".
						_("Use it at your own risk and read the documentation for lamdaemon before you use it!").
						"</b>"),
				"212" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Change password"),
					"Text" => _("If you want to change the current preferences password, please enter it here.")),
				"214" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Cache timeout"),
					"Text" => _("This is the time in minutes which LAM caches its LDAP searches. Shorter times will stress LDAP more but decrease the possibility that changes are not identified.")),
				"216" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Text for user PDF"),
					"Text" => _("This text will appear on top of every user PDF file.")),
				"217" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Account modules"),
					"Text" => _("Here you can select which plugins you want to use for account management.")),
				"230" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Add profile"),
					"Text" => _("Please enter the name of the new profile and the password to change its settings. Profile names may contain letters, numbers and -/_.")),
				"231" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Rename profile"),
					"Text" => _("Please enter the new name of the profile. The name may contain letters, numbers and -/_.")),
				"232" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Delete profile"),
					"Text" => _("This will delete the selected profile.")),
				"233" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Set profile password"),
					"Text" => _("This changes the password of the selected profile.")),
				"234" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Change default profile"),
					"Text" => _("This changes the profile which is selected by default at login.")),
				"235" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Change master password"),
					"Text" => _("If you want to change your master configuration password, please enter it here.")),
				"236" => array ("ext" => "FALSE", "Headline" => _("Profile management") . " - " . _("Master password"),
					"Text" => _("Please enter the master configuration password. This is NOT your LDAP password. It is stored in your config.cfg file. If this is the first time you log in, enter \"lam\".")),
				"237" => array ("ext" => "FALSE", "Headline" => _("Configuration wizard") . " - " . _("Base module"),
					"Text" => _("Every account type needs exactly one base module. This module provides a structural object class.")),
				"250" => array ("ext" => "FALSE", "Headline" => _("Account lists - Filters"),
					"Text" => _("Here you can input small filter expressions (e.g. 'value' or 'v*'). LAM will filter case-insensitive.")),
				// 300 - 399
				// Roland Gruber
				// profile editor, file upload
				"301" => array ("ext" => "FALSE", "Headline" => _("RDN identifier"),
					"Text" => _("This is the identifier for the relative DN value. It must be one of the given allowed LDAP attributes (e.g. user accounts usually use \"uid\" while groups use \"cn\").")),
				"360" => array ("ext" => "FALSE", "Headline" => _("Profile name"),
					"Text" => _("Name under which the profile will be saved. If a profile with the same name exists, it will be overwritten.")),
				"361" => array ("ext" => "FALSE", "Headline" => _("DN suffix"),
					"Text" => _("The new account will be saved under this LDAP suffix.")),
				// 400 - 499
				// Tilo Lutz
				// account.php
				"400" => array ("ext" => "FALSE", "Headline" => _("Username"),
					"Text" => _("Username of the user who should be created. Valid characters are: a-z,0-9, .-_. Lam does not allow a number as first character because useradd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. If username is already used username will be expanded with a number. The next free number will be used. Warning: Older systems have problems with usernames longer than 8 characters. You can not log in to Windows if username is longer than 16 characters.")),
				"401" => array ("ext" => "FALSE", "Headline" => _("UID number"),
					"Text" => _("If empty UID number will be generated automaticly.")),
				"402" => array ("ext" => "FALSE", "Headline" => _("Additional groups"),
					"Text" => _("Hold the CTRL-key to (de)select multiple groups."). ' '. _("Can be left empty.")),
				"403" => array ("ext" => "FALSE", "Headline" => _("Home directory"),
					"Text" => _("$user and $group are replaced with username or primary groupname.")),
				"404" => array ("ext" => "FALSE", "Headline" => _("Gecos"),
					"Text" => _("User description. If left empty sur- and give name will be used.")),
				"405" => array ("ext" => "FALSE", "Headline" => _("Login shell"),
					"Text" => _("To disable login use /bin/false. List of shells is read from lam/config/shells")),
				"406" => array ("ext" => "FALSE", "Headline" => _("Primary group"),
					"Text" => _("The Primary Group the user should be member of.")),
				"407" => array ("ext" => "FALSE", "Headline" => _("Groupname"),
					"Text" => _("Group name of the group which should be created. Valid characters are: a-z,0-9, .-_. Lam does not allow a number as first character because groupadd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. If groupname is already used groupname will be expanded with a number. The next free number will be used.")),
				"408" => array ("ext" => "FALSE", "Headline" => _("GID number"),
					"Text" => _("If empty GID number will be generated automaticly depending on your configuration settings.")),
				"409" => array ("ext" => "FALSE", "Headline" => _("Gecos"),
					"Text" => _("Group description. If left empty group name will be used.")),
				"410" => array ("ext" => "FALSE", "Headline" => _("Host name"),
					"Text" => _("Host name of the host which should be created. Valid characters are: a-z,0-9, .-_$. Lam does not allow a number as first character because useradd also does not allow it. Lam does not allow capital letters A-Z because it can cause several problems. Hostnames are always ending with $. If last character is not $ it will be added. If hostname is already used hostname will be expanded with a number. The next free number will be used.")),
				"411" => array ("ext" => "FALSE", "Headline" => _("UID number"),
					"Text" => _("If empty UID number will be generated automaticly.")),
				"412" => array ("ext" => "FALSE", "Headline" => _("Primary group"),
					"Text" => _("The Primary group the host should be member of.")),
				"413" => array ("ext" => "FALSE", "Headline" => _("Gecos"),
					"Text" => _("Host description. If left empty host name will be used.")),
				"414" => array ("ext" => "FALSE", "Headline" => _("Password warn"),
					"Text" => _("Days before password is to expire that user is warned of pending password expiration. If set value must be 0<."). ' '. _("Can be left empty.")),
				"415" => array ("ext" => "FALSE", "Headline" => _("Password expire"),
					"Text" => _("Number of days a user can login even his password has expired. -1=always."). ' '. _("Can be left empty.")),
				"416" => array ("ext" => "FALSE", "Headline" => _("Maximum password age"),
					"Text" => _("Number of days after a user has to change his password again. If set value must be 0<."). ' '. _("Can be left empty.")),
				"417" => array ("ext" => "FALSE", "Headline" => _("Minimum password age"),
					"Text" => _("Number of days a user has to wait until he\'s allowed to change his password again. If set value must be 0<."). ' '. _("Can be left empty.")),
				"418" => array ("ext" => "FALSE", "Headline" => _("Expire date"),
					"Text" => _("Account expire date. Format: DD-MM-YYYY")),
				"419" => array ("ext" => "FALSE", "Headline" => _("Group members"),
					"Text" => _("Users wich are also members of group.")),
				"420" => array ("ext" => "FALSE", "Headline" => _("Display name"),
					"Text" => _("Windows clients will show display name as group description.")),
				"421" => array ("ext" => "FALSE", "Headline" => _("Load profile"),
					"Text" => _("You can select a previous defined profile here. This will set all fields to the profile values.")),
				"422" => array ("ext" => "FALSE", "Headline" => _("Expand suffix with primary groupname"),
					"Text" => _("If selected users will be added with OUs expanded with their primary group. E.g. if a user is in group admin the user suffix will be ou=admin,+user suffix.")),
				"423" => array ("ext" => "FALSE", "Headline" => _("Group suffix"),
					"Text" => _("If a not yet existing group is defined in csv-file, a new group in the selected group suffix will be created.")),
				"424" => array ("ext" => "FALSE", "Headline" => _("Last name"),
					"Text" => _("Last name of user. Only letters, - and spaces are allowed.")),
				"425" => array ("ext" => "FALSE", "Headline" => _("First name"),
					"Text" => _("First name of user. Only letters, - and spaces are allowed.")),
				"426" => array ("ext" => "FALSE", "Headline" => _("Use no password"),
					"Text" => _("If checked no password will be used.")),
				"428" => array ("ext" => "FALSE", "Headline" => _("Use unix password"),
					"Text" => _("If checked unix password will also be used as samba password.")),
				"429" => array ("ext" => "FALSE", "Headline" => _("Password does not expire"),
					"Text" => _("If checked password does not expire. (Setting X-Flag)")),
				"430" => array ("ext" => "FALSE", "Headline" => _("User can change password"),
					"Text" => _("Date after the user is able to change his password. Format: DD-MM-YYYY")),
				"431" => array ("ext" => "FALSE", "Headline" => _("User must change password"),
					"Text" => _("Date after the user must change his password. Format: DD-MM-YYYY")),
				"432" => array ("ext" => "FALSE", "Headline" => _("Account is deactivated"),
					"Text" => _("If checked account will be deactivated. (Setting D-Flag)")),
				"433" => array ("ext" => "FALSE", "Headline" => _("Home drive"),
					"Text" => _("Driveletter assigned on windows workstations as homedirectory.")),
				"434" => array ("ext" => "FALSE", "Headline" => _("Script path"),
					"Text" => _('Filename and -path relative to netlogon-share which should be executed on logon. $user and $group are replaced with user- and groupname.'). ' '. _("Can be left empty.")),
				"435" => array ("ext" => "FALSE", "Headline" => _("Profile path"),
					"Text" => _('Path of the user profile. Can be a local absolute path or a UNC-path (\\\\server\\share). $user and $group are replaced with user- and groupname.'). ' '. _("Can be left empty.")),
				"436" => array ("ext" => "FALSE", "Headline" => _("Samba workstations"),
					"Text" => _("List of Samba workstations the user is allowed to login. Empty means every workstation."). ' '. _("Can be left empty.")),
				"437" => array ("ext" => "FALSE", "Headline" => _("Home path"),
					"Text" => _('UNC-path (\\\\server\\share) of homedirectory. $user and $group are replaced with user- and groupname.'). ' '. _("Can be left empty.")),
				"438" => array ("ext" => "FALSE", "Headline" => _("Domain"),
					"Text" => _("Windows-Domain of user."). ' '. _("Can be left empty.")),
				"457" => array ("ext" => "FALSE", "Headline" => _("Save profile"),
					"Text" => _("This will make a profile of the current account. The saved values are the same as in the profile editor. Profile names may contain the letters a-z, 0-9 and -_.")),
				"458" => array ("ext" => "FALSE", "Headline" => _("Select group profile"),
					"Text" => _("If a not yet existing group is defined in csv-file, a new group with the selected group profile will be created.")),
				"460" => array ("ext" => "FALSE", "Headline" => _("Windows domain"),
					"Text" => _("Windows domain of host."). ' '. _("Can be left empty.")),
				"461" => array ("ext" => "FALSE", "Headline" => _("Suffix"),
					"Text" => _("Position in ldap-tree where the user should be created.")),
				"462" => array ("ext" => "FALSE", "Headline" => _("Suffix"),
					"Text" => _("Position in ldap-tree where the group should be created.")),
				"463" => array ("ext" => "FALSE", "Headline" => _("Suffix"),
					"Text" => _("Position in ldap-tree where the host should be created.")),
				"464" => array ("ext" => "FALSE", "Headline" => _("Windows group name"),
					"Text" => _("If you want to use a well known RID you can selcet a well known group.")),
				"466" => array ("ext" => "FALSE", "Headline" => _("Unix workstations"),
					"Text" => _("Comma separated list of unix workstations the user is allowed to login."). ' '. _("Can be left empty.")),
				"467" => array ("ext" => "FALSE", "Headline" => _("Domain"),
					"Text" => _("Windows-Domain of group."). ' '. _("Can be left empty.")),
				"468" => array ("ext" => "FALSE", "Headline" => _("Group members"),
					"Text" => _("Users also being member of the current group."). ' '. _("Can be left empty.")),
				// 600 - 699
				// Roland Gruber
				// OU-editor, domain page
				"601" => array ("ext" => "FALSE", "Headline" => _("OU-Editor") . " - " . _("New organizational unit"),
					"Text" => _("This will create a new organizational unit under the selected one.")),
				"602" => array ("ext" => "FALSE", "Headline" => _("OU-Editor") . " - " . _("Delete organizational unit"),
					"Text" => _("This will delete the selected organizational unit. The OU has to be empty.")),
				"651" => array ("ext" => "FALSE", "Headline" => _("Domain name"),
					"Text" => _("The name of your Windows domain or workgroup.")),
				"652" => array ("ext" => "FALSE", "Headline" => _("Suffix"),
					"Text" => _("The domain entry will be saved under this suffix.")),
				"653" => array ("ext" => "FALSE", "Headline" => _("Domain SID"),
					"Text" => _("The SID of your Samba server. Get it with \"net getlocalsid\".")),
				"654" => array ("ext" => "FALSE", "Headline" => _("Next RID"),
					"Text" => _("Next RID to use when creating accounts.")),
				"655" => array ("ext" => "FALSE", "Headline" => _("Next User RID"),
					"Text" => _("Next RID to use when creating user accounts.")),
				"656" => array ("ext" => "FALSE", "Headline" => _("Next Group RID"),
					"Text" => _("Next RID to use when creating groups.")),
				"657" => array ("ext" => "FALSE", "Headline" => _("Algorithmic RID Base"),
					"Text" => _("Used for calculating RIDs from UID/GID. Do not change if unsure."))

/* This is a sample help entry. Just copy this line an modify the vakues between the [] brackets.
	Help text is located in the array:
		"[Helpnumber]" => array ("ext" => "FALSE", "Headline" => _("[Headline]"), "Text" => _("[Text]"), "SeeAlso" => "[SeeAlso link]"),
	Help text is located in an extra file; File must be stored in help directory; Link has to be the path to the file if it is stored in a subdirectory and the filename with a leading slash;
		"[Helpnumber]" => array ("ext" => "TRUE", "Link" => "[path/FileName]),
*/
				);
?>
