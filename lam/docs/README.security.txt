
1. Use of SSL

   The data which is transfered between you and LAM is very sensitive.
   Please always use SSL encrypted connections between LAM and your browser to
   protect yourself against network sniffers.


2. LDAP+SSL and TLS

   LAM should start TLS automatically if possible. LDAP+SSL will be used if you use
   ldaps://servername in your configuration profile.


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

