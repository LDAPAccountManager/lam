
1. Use of SSL

   The data which is transfered between you and LAM is very sensitive.
   Please always use SSL encrypted connections between LAM and your browser to
   protect yourself against network sniffers.


2. LDAP with SSL and TLS

   SSL will be used if you use ldaps://servername in your configuration profile.
   TLS can be activated with the "Activate TLS" option.

   You will need to setup ldap.conf to trust your server certificate. Some installations
   use /etc/ldap.conf and some use /etc/ldap/ldap.conf. It is a good idea to symlink
   /etc/ldap.conf to /etc/ldap/ldap.conf.
   Specify the server CA certificate with the following option:

   TLS_CACERT /etc/ldap/ca/myCA/cacert.pem

   This needs to be the public part of the signing certificate authority. See "man ldap.conf"
   for additional options.


3. Chrooted servers

   If your server is chrooted and you have no access to /dev/random or /dev/urandom
   this can be a security risk. LAM stores your LDAP password encrypted in the session.
   LAM uses rand() to generate the key if /dev/random and /dev/urandom are not accessible.
   Therefore the key can be easily guessed.
   An attaker needs read access to the session file (e.g. by another Apache instance) to
   exploit this.


4. Protection of your LDAP password and directory contents

   You have to install the MCrypt extension for PHP to enable encryption.

   Your LDAP password is stored encrypted in the session file. The key and IV to decrypt
   it are stored in two cookies. We use MCrypt/AES to encrypt the password.
   All data that was read from LDAP and needs to be stored in the session file is also
   encrypted.


5. Apache configuration

   LAM includes several .htaccess files to protect your configuration files and temporary
   data. Apache is often configured to not use .htaccess files by default.
   Therefore, please check your Apache configuration and change the override setting to:

     AllowOverride All

   If you are experienced in configuring Apache then you can also copy the security settings
   from the .htaccess files to your main Apache configuration.

   If possible, you should not rely on .htaccess files but also move the config and sess
   directory to a place outside of your WWW root. You can put a symbolic link in the LAM
   directory so that LAM finds the configuration/session files.


   Security sensitive directories:

   config: Contains your LAM configuration and account profiles
           - LAM configuration clear text passwords
           - default values for new accounts
           - directory must be accessibly by Apache but needs not to be accessible by the browser

   sess: PHP session files
         - LAM admin password in clear text or MCrypt encrypted
         - cached LDAP entries in clear text or MCrypt encrypted
         - directory must be accessibly by Apache but needs not to be accessible by the browser

   tmp: temporary files
        - PDF documents which may also include passwords
        - images of your users
        - directory contents must be accessible by browser but directory itself must not be browseable
