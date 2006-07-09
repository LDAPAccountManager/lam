lamdaemon.pl is used to modify quota and homedirs
on a remote or local host via ssh.
If you want wo use it you have to set up many
thins to get it work.

1. Set values in LDAP Account manager
   * Set the remote or local host in the configuration
    (e.g. 127.0.0.1)
   * Path to lamdaemon.pl, e.g. /srv/www/htdocs/lam/lib/lamdaemon.pl  


2. Set up sudo
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
   
3. Set up perl
   We need some external perl-modules, Quota and Net::SSH::Perl
   Th install them, run:
   perl -MCPAN -e shell
   install Quota
   install Net::SSH::Perl
   Please answer all questions to describe your system
   Every additional needed module should be installed
   automaticly
   LDAP isn't used by lamdaemon.pl anymore 

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

4. Set up ssh
   On my System, Suse 9.0 I had to set usePAM no in /etc/ssh/sshd_config
   to get lamdaemon.pl work
   I had some problems to log in with ssh if the password hash of the
   admin-user was encrypted with {SSHA}. I had to change encryption
   for admin-accounts to {CRYPT} to get ssh work.
   
5. Test lamdaemon.pl
   I've installed a test-function in lamdaemon.pl. Please run lamdaemon.pl
   with the following attributes to test it:
   lamdaemon.pl $ssh-server $lam_path_on_host $admin-username $admin-password *test
   $ssh-server is the remote host lamdaemon.pl should be run
   $lam_path_on_host is the path to lamdaemon.pl on remote host
   $admin-username is the name of the user which is allowed to run lamdaemon.pl
                   as root. It's the same user in /etc/sudoers
   $admin-password is the password of admin-user
   *test is the command which tells lamdaemon.pl to test settings
   
   You have to run the coammd as the user your webserver is running as, e.g.

   wwwrun@tilo:/srv/www/htdocs/lam/lib> /srv/www/htdocs/lam/lib/lamdaemon.pl \
     127.0.0.1 /srv/www/htdocs/lam/lib/lamdaemon.pl root secret *test

   You should get the following response:
     Net::SSH::Perl successfully installed.
     sudo set up correctly.
     Perl quota module successfully installed.
     If you have'nt seen any error lamdaemon.pl should set up successfully.
   
Now everything should work fine

This is a very incomplete Documention for Beta-Release only.
Pleas send a mail to TiloLutz@gmx.de if you have any suggsestion
