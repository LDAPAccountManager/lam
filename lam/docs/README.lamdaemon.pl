lamdaemon.pl is used to modify quota and homedirs
on a remote or local host via ssh.
If you want wo use it you have to set up many
thins to get it work.

1. Set values in LDAP Account manager
   * Set the remote or local host in the configuration
    (e.g. 127.0.0.1)
   * Path to lamdaemon.pl, e.g. /srv/www/htdocs/lam/lib/lamdaemon.pl  

2. Set up SSH
   I don't know if this step is really needed but I had some
   problems using Net::SSH without keys.
   * Log in on remote host as $admin
   * run "ssh-keygen -t dsa" to create all needed keys
     if not yet done

3. Set up sudo
   The perlskript has to run as root (very ugly I know but
   I haven't found any other solution). Therefor we need
   a wrapper, sudo.
   Edit /etc/sudoers on host homedirs or quotas should be used
   and add the following line:
   $admin All= NOPASSWD: $path
   $admin is the adminuser from lam and $path
   is the path include the filename of lamdaemon.pl
   e.g. $admin All= NOPASSWD: /srv/www/htdocs/lam/lib/lamdaemon.pl
   At the moment the password is a paramteter of lamdaemon.pl
   Therefore you should disable logging so the password doesn't
   apear in any logfile
   This can be done by adding the following line:
   Defaults:$admin !syslog
   
4. Set up perl
   We need some external perl-modules, Quota and Net::LDAP
   Th install them, run:
   perl -MCPAN -e shell
   install Quota
   install Net::SSH::Perl
   Please answer all questions to describe your system
   Every additional needed module should be installed
   automaticly
   LDAP isn't used in perl anymore 

   I installed Math::Pari, a needed module, by hand.
   I had many problems to install Math::Pari, a module needed
   by Net:SSH::Perl. The reason is a bug in gcc 3.3 (In my case).
   I found the following solution to prevent this bug:
   * Download and untar pari (http://www.parigp-home.de)
   * Download and untar Math::Pari
   * run perl Makefile.PL
   * edit Makefile and libPARI/Makefile
     Replace line "OPTIMIZE = -O3 --pipe" with
     "OPTIMIZE = -O1 --pipe".
   * run make
   * run make install
   
5. Set up lamdaemon.pl
   Make all needed changes in lamdaemon.pl
      
Now everything should work fine

This is a very incomplete Documention for Alpha-Release only.
Pleas send a mail to TiloLutz@gmx.de if you have any suggsestion
