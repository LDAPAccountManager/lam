lamdaemon.pl is used to modify quota and homedirs
on a remote or local host via ssh.
If you want wo use it you have to set up many
thins to get it work.

1. Set values in LDAP Account manager
   * Set the remote or local host in the configuration
    (e.g. 127.0.0.1)
  

3. Set up sudo
   The perlskript has to run as root (very ugly I know but
   I haven't found any other solution). Therefor we need
   a wrapper, sudo.
   Edit /etc/sudoers and add the following line:
   $admin All= NOPASSWD: $path
   $admin is the adminuser from lam and $path
   is the path include the filename of lamdaemon.pl
   e.g. $admin All= NOPASSWD: /srv/www/htdocs/lam/lib/lamdaemon.pl
   
4. Set up perl
   We need some external perl-modules, Quota and Net::LDAP
   Th install them, run:
   perl -MCPAN -e shell
   install Quota
   install Net::LDAP
   install Net:SSH
   Please answer all questions to describe your system
   Every additional needed module should be installed
   automaticly

5. Set up lamdaemon.pl
   Make all needed changes in lamdaemon.pl
      
Now everything should work fine

This is a very incomplete Documention for Alpha-Release only.
Pleas send a mail to TiloLutz@gmx.de if you have any suggsestion
