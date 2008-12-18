 This document describes the installation of lamdaemon which is responsible
 for managing quotas and creating home directories.


Attention! The old version of lamdaemon is no longer supported. However,
if you do not install libssh2 then LAM will fall back to the old mechanismn.
If you want to stay with the old lamdaemon then change your /etc/sudoers entries
to point to lamdaemonOld.pl.
Do NOT mix lamdaemon and lamdaemonOld.pl!


 Setting up lamdaemon:
 =====================


 Lamdaemon.pl is used to modify quota and home directories on a remote or local host via ssh.
 If you want wo use it you have to set up some things to get it to work:


1. Setup values in LDAP Account Manager
=======================================

   * Set the remote or local host in the configuration
    (e.g. 127.0.0.1)

   * Path to lamdaemon.pl, e.g. /srv/www/htdocs/lam/lib/lamdaemon.pl
     If you installed a Debian or RPM package then the script may be located at
     /usr/share/ldap-account-manager/lib or /var/www/html/lam/lib.

   * Your LAM admin user must be a valid Unix account. It needs to have the object class
     "posixAccount" and an attribute "uid". This account must be accepted by the
     SSH daemon of your home directory server.
     Do not create a second local account but change your system to accept LDAP users.
     You can use LAM to add the Unix account part to your admin user.


2. Setup sudo
=============

   The perl script has to run as root. Therefore we need
   a wrapper, sudo.
   Edit /etc/sudoers on host where homedirs or quotas should be used
   and add the following line:

   $admin All= NOPASSWD: $path

   $admin is the admin user from LAM (must be a valid Unix account)
   and $path is the path to lamdaemon.pl

     e.g.: myAdmin ALL= NOPASSWD: /srv/www/htdocs/lam/lib/lamdaemon.pl


3. Setup Perl
==============

   We need an extra Perl module - Quota
   To install it, run:

   perl -MCPAN -e shell
   install Quota

   If your Perl executable is not located in /usr/bin/perl you will have to edit
   the path in the first line of lamdaemon.pl.
   If you have problems compiling the Perl modules try installing a newer release
   of your GCC compiler and the "make" application.

   Several Linux distributions already include a quota package for Perl.


4. Install libssh2
==================

   4.1 Install libssh2
       You can get libssh2 here: http://www.libssh2.org
       Unpack the package and install it by executing the commands
       "./configure", "make" and "make install" in the extracted directory.

   4.2 Install SSH2 for PHP
       The easiest way is to run "pecl install ssh2-beta". If you have no pecl command then install
       the PHP Pear package (e.g. php-pear or php5-pear) for your distribution.

       If you want to compile it yourself, get the sources here: http://pecl.php.net/package/ssh2

       After installing the PHP module please add this line to your php.ini:
       extension=ssh2.so


5. Set up SSH
=============

   Your SSH daemon must offer the password authentication method.
   To activate it just use this configuration option in /etc/ssh/sshd_config:

   PasswordAuthentication yes


Now everything should work fine.


6. Troubleshooting
======================

   - There is a test page for lamdaemon:
     Login to LAM and open Tools -> Tests -> Lamdaemon test

   - If you get garbage characters at the test page then PHP and your php5-ssh2 library may not
     fit together. Try recompiling the library and libssh2.

     This combination was tested successfully: libssh2 0.13 with php5-ssh2 0.10
     php5-ssh2 0.11 should have no problems with recent libssh2 releases.

   - Check /var/log/auth.log or the equivalent on your system
     This file contains messages about all logins. If the ssh login
     failed then you will find a description about the reason here.

   - Set sshd in debug mode
     In /etc/ssh/sshd_conf add these lines:

     SyslogFacility AUTH
     LogLevel DEBUG3

     Now check /var/log/syslog for messages from sshd.

   - Update Openssh
     A Suse Linux user reported that upgrading Openssh solved the problem.

