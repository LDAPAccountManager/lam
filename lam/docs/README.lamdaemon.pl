
lamdaemon.pl is used to modify quota and homedirs
on a remote or local host via ssh.
If you want wo use it you have to set up many
thins to get it work.

1. Set values in LDAP Account manager
   * Set the remote or local host in the configuration
    (e.g. 127.0.0.1)
   * Set the remote-path include filename of the script
    (/srv/www/htdocs/lam/lib/lamdaemon.pl)

2. Set up ssh
   We have to connect to the remote host as the user
   your webserver is running. Because we can't enter
   the password for it we have to authenticate without
   entering a password
   * Switch to the user your webserver is running as
    (e.g. su wwwrun)
   * switch to homedir of the webserver user
    (e.g. cd ~)
   * create the ssh-keys, just press enter if you'll asked
     for a password
    (e.g. ssh-keygen -t dsa)
   * Check if the user your webserver is running as does
     also exists on remote-host
   * Copy the content of ~/.ssh/id_dsa.pub from the system
     LDAP Account manager into ~/.ssh/known_hosts on the
     remote machine
   * Connect to the remote server via ssh $remotehost
     Answer the next question with yes if the remote key is
     valid. You should be asked for a password

3. Set up sudo
   The perlskript has to run as root (very ugly I know but
   I haven't found any other solution). Therefor we need
   a wrapper, sudo.
   Edit /etc/sudoers and add the following line:
   $wwwrun All= NOPASSWD: $path
   $wwwrun is the user your webserer is running and $path
   is the path include the filename of lamdaemon.pl
   e.g. wwwrun All= NOPASSWD: /srv/www/htdocs/lam/lib/lamdaemon.pl
   
4. Set up perl
   We need some external perl-modules, Quota and Net::LDAP
   Th install them, run:
   perl -MCPAN -e shell
   install Quota
   install Net::LDAP
   Please answer all questions to describe your system
   Every additional needed module should be installed
   automaticly

5. Set up lamdaemon.pl
   Make all needed changes in lamdaemon.pl
      
Now everything should work fine

This is a very incomplete Documention for Alpha-Release only.
Pleas send a mail to TiloLutz@gmx.de if you have any suggsestion
