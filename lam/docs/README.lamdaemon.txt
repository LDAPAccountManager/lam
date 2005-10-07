
   Setting up lamdaemon:


 Lamdaemon.pl is used to modify quota and home directories on a remote or local host via ssh.
 If you want wo use it you have to set up some things to get it to work:


1. Setup values in LDAP Account Manager
   * Set the remote or local host in the configuration
    (e.g. 127.0.0.1)
   * Path to lamdaemon.pl, e.g. /srv/www/htdocs/lam/lib/lamdaemon.pl


2. Set up sudo
   The perl script has to run as root. Therefore we need
   a wrapper, sudo.
   Edit /etc/sudoers on host where homedirs or quotas should be used
   and add the following line:
   
   $admin All= NOPASSWD: $path
   
   $admin is the adminuser from LAM and $path is the path to lamdaemon.pl
   e.g. "$admin All= NOPASSWD: /srv/www/htdocs/lam/lib/lamdaemon.pl"
   At the moment the password is a paramteter of lamdaemon.pl
   therefore you should disable logging so the password does not
   appear in any logfile.
   This can be done by adding the following line to /etc/sudoers:

   Defaults:$admin !syslog


3. Set up Perl
   We need some external Perl modules, Quota and Net::SSH::Perl
   To install them, run:

   perl -MCPAN -e shell
   install Quota
   install Net::SSH::Perl

   If your Perl executable is not located in /usr/bin/perl you will have to edit
   the path in the first line of lamdaemon.pl.

   Debian users can install Net::SSH:Perl with dh-make-perl:

   apt-get install dh-make-perl
   dh-make-perl --build --cpan Net::SSH::Perl
   dpkg -i install libnet-ssh-perl_1.25-1_all.deb


4. Set up SSH
   Your SSH daemon must offer the password authentication method.
   To activate it just use this configuration option in /etc/ssh/sshd_config:

   PasswordAuthentication yes


5. Test lamdaemon.pl
   There is a test-function in lamdaemon.pl. Please run lamdaemon.pl
   with the following parameters to test it:

   lamdaemon.pl $ssh-server $lam_path_on_host $admin-username $admin-password *test

   $ssh-server is the remote host lamdaemon.pl should be run on
   $lam_path_on_host is the path to lamdaemon.pl on remote host
   $admin-username is the name of the user which is allowed to run lamdaemon.pl
                   as root. It is the same user as in /etc/sudoers
   $admin-password is the password of the admin user
   *test is the command which tells lamdaemon.pl to test settings

   You have to run the command as the user your webserver is running, e.g.

   wwwrun@tilo:/srv/www/htdocs/lam/lib> /srv/www/htdocs/lam/lib/lamdaemon.pl \
     127.0.0.1 /srv/www/htdocs/lam/lib/lamdaemon.pl adminuser secret *test

   You should get the following response:

     Net::SSH::Perl successfully installed.
     Perl quota module successfully installed.
     If you have not seen any error lamdaemon.pl should be set up successfully.
 
 
   !!! Attention !!!
   Your password in LDAP has to be hashed with CRYPT. If you use something like SSHA
   you will probably get "Access denied.".

   Now everything should work fine.


6. Debugging lamdaemon
   If you set up all things as documented before and still get "Access denied"
   then you can try to debug the problem.

   - Check /var/log/auth.log or the equivalent on your system
     This file contains messages about all logins. If the ssh login
     failed then you will find a description about the reason here.

   - Enable debug output in lamdaemon
     In line 235 of lamdaemon.pl change the SSH options like this:

     my $ssh = Net::SSH::Perl->new($hostname, options=>[
       "UserKnownHostsFile /dev/null"],
       protocol => "2,1", debug => 1 );

     This will produce a lot of output when you do the lamdaemon test.
     Check that there is a line like this:
  
     Authentication methods that can continue: publickey,password,keyboard-interactive.

     The "password" is the one which is important.

   - Set sshd in debug mode
     In /etc/ssh/sshd_conf add these lines:

     SyslogFacility AUTH
     LogLevel DEBUG3

     Now check /var/log/syslog for messages from sshd.

   - Update Openssh
     A Suse Linux user reported that upgrading Openssh solved the problem.


Security warning:
-----------------

   If you use PHP < 4.3 your admin user and password are passed as commandline argument.
   This can be a security risk. Upgrade your PHP version for productive use.


Please send a mail to TiloLutz@gmx.de if you have any suggestions.
