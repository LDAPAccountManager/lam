%define httpd_confdir @@HTTP_CONF_DIR@@
%define lam_dir ldap-account-manager
%define lam_uid @@USER@@
%define lam_gid @@GROUP@@
%define lam_distribution @@DISTRIBUTION@@
%define is_suse %(test suse = %lam_distribution && echo 1 || echo 0)
%define is_fedora %(test fedora = %lam_distribution && echo 1 || echo 0)
%define _binary_payload w9.bzdio
%define _source_payload w9.bzdio

Name:         ldap-account-manager
License:      GPL
Group:        Productivity/Networking/Web/Frontends
Version:      @@VERSION@@
Release:      0.%lam_distribution.1
Source0:      ldap-account-manager-%{version}.tar.gz
URL:          http://www.ldap-account-manager.org/
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}
Summary:      Administration of LDAP users, groups and hosts via Web GUI
Summary(de):  Administration von Benutzern, Gruppen und Hosts für LDAP-Server
Vendor:       Roland Gruber
Packager:     Roland Gruber <post@rolandgruber.de>
BuildArchitectures: noarch
AutoReqProv:  no
%if %is_suse
Requires:      php5
Requires:      php5-ldap
Requires:      php5-hash
Requires:      php5-gd
Requires:      php5-gettext
Requires:      perl
%endif
%if %is_fedora
Requires:      php
Requires:      perl
%endif


%description
LDAP Account Manager (LAM) runs on an existing webserver.
It manages user, group and host accounts. Currently LAM supports
these account types: Samba 3, Unix, Kolab 2, address book
entries, NIS mail aliases and MAC addresses. There is an integrated LDAP browser
to allow access to the raw LDAP attributes. You
can use templates for account creation and use multiple configuration
profiles. Account information can be exported as PDF file. There is also
a script included which manages quota and homedirectories.

%description -l de
LDAP Account Manager (LAM) läuft auf einem exisierenden Webserver.
LAM verwaltet Benutzer, Gruppen und Hosts. Zur Zeit werden folgende Account-Typen
unterstützt: Samba 3, Unix, Kolab 2, Addressbuch Einträge, NIS
mail Aliase und MAC-Addressen. Es gibt einen integrierten LDAP-Browser mit dem
man die LDAP-Einträge direkt bearbeiten kann. Zum Anlegen von Accounts können
Vorlagen definiert werden. Es können mehrere Konfigurations-Profile
definiert werden. Account-Informationen können als PDF exportiert
werden. Außerdem exisitiert ein Script mit dem man Quotas und
Home-Verzeichnisse verwalten kann.

%prep
pwd
cp $RPM_SOURCE_DIR/lam.apache.conf $RPM_BUILD_DIR/
%setup -n ldap-account-manager-%{version}

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/share/%{lam_dir}
cp -dR * $RPM_BUILD_ROOT/usr/share/%{lam_dir}
mkdir -p $RPM_BUILD_ROOT/var/lib/%{lam_dir}
mv $RPM_BUILD_ROOT/usr/share/%{lam_dir}/config $RPM_BUILD_ROOT/var/lib/%{lam_dir}
ln -s /var/lib/%{lam_dir}/config $RPM_BUILD_ROOT/usr/share/%{lam_dir}/config
mv $RPM_BUILD_ROOT/usr/share/%{lam_dir}/tmp $RPM_BUILD_ROOT/var/lib/%{lam_dir}
ln -s /var/lib/%{lam_dir}/tmp $RPM_BUILD_ROOT/usr/share/%{lam_dir}/tmp
mv $RPM_BUILD_ROOT/usr/share/%{lam_dir}/sess $RPM_BUILD_ROOT/var/lib/%{lam_dir}
ln -s /var/lib/%{lam_dir}/sess $RPM_BUILD_ROOT/usr/share/%{lam_dir}/sess
mkdir -p $RPM_BUILD_ROOT%{httpd_confdir}
cp $RPM_BUILD_DIR/lam.apache.conf $RPM_BUILD_ROOT%{httpd_confdir}/

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT

%post
if [ ! -f /var/lib/%{lam_dir}/config/config.cfg ]; then
	cp /var/lib/%{lam_dir}/config/config.cfg_sample /var/lib/%{lam_dir}/config/config.cfg
	chmod 600 /var/lib/%{lam_dir}/config/config.cfg
	chown %{lam_uid}:%{lam_gid} /var/lib/%{lam_dir}/config/config.cfg
	if [ ! -f /var/lib/%{lam_dir}/config/lam.conf ]; then
		cp /var/lib/%{lam_dir}/config/lam.conf_sample /var/lib/%{lam_dir}/config/lam.conf
		chmod 600 /var/lib/%{lam_dir}/config/lam.conf
		chown %{lam_uid}:%{lam_gid} /var/lib/%{lam_dir}/config/lam.conf
	fi
fi
%if %is_suse
/etc/init.d/apache2 restart
%endif
%if %is_fedora
/etc/init.d/httpd restart
%endif

%postun
%if %is_suse
/etc/init.d/apache2 restart
%endif
%if %is_fedora
/etc/init.d/httpd restart
%endif

%files
%defattr(-, root, root)
%dir %attr(700, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}
%doc COPYING HISTORY README VERSION docs/*
%attr(700, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/sess
%attr(700, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/tmp
%dir %attr(700, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config
%dir %attr(700, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config/pdf
%dir %attr(700, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config/profiles
%dir %attr(700, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config/selfService
/var/lib/%{lam_dir}/config/*_sample
/var/lib/%{lam_dir}/config/.htaccess
/var/lib/%{lam_dir}/config/language
/var/lib/%{lam_dir}/config/shells
/var/lib/%{lam_dir}/config/pdf/.htaccess
%attr(-, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config/pdf/logos
/var/lib/%{lam_dir}/config/profiles/.htaccess
/var/lib/%{lam_dir}/config/selfService/.htaccess
%config(noreplace) %attr(-, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config/pdf/default.*
%config(noreplace) %attr(-, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config/profiles/default.*
%config(noreplace) %attr(-, %{lam_uid}, %{lam_gid}) /var/lib/%{lam_dir}/config/selfService/default.*
/usr/share/%{lam_dir}
%{httpd_confdir}

%changelog
* Wed Jun 08 2011 - Roland Gruber post@rolandgruber.de
- Install into /usr/share/ldap-account-manager

* Sat Apr 09 2011 - Roland Gruber post@rolandgruber.de
- Do not overwrite config files

* Sat Nov 07 2009 - Roland Gruber post@rolandgruber.de
- Added LAM manuals

* Sat Jul 26 2008 - Roland Gruber post@rolandgruber.de
- Added subpackage for lamdaemon

* Wed Jan 11 2006 - Iain Lea iain@bricbrac.de
- Updated for 1.0 series on Fedora Core 

* Mon Dec 12 2005 - Iain Lea iain@bricbrac.de
- Updated for 0.5.x series on Fedora Core 

* Sun Mar 21 2004 - TiloLutz@gmx.de
- Initial release 0.1.0 - 0.4.5


%package lamdaemon

Summary:      Quota and home directory management for LDAP Account Manager
Summary(de):  Verwaltung von Quotas und Heimatverzeichnissen für LDAP Account Manager
Group:        Productivity/Networking/Web/Frontends
AutoReqProv:  no
%if %is_suse
Requires:      perl
Requires:      sudo
%endif
%if %is_fedora
Requires:      perl
Requires:      sudo
%endif

%description lamdaemon
Lamdaemon is part of LDAP Account Manager. This package
needs to be installed on the server where the home directories
reside and/or quotas should be managed.

%description lamdaemon -l de
Lamdaemon ist Teil von LDAP Account Manager. Dieses Paket
wird auf dem Server installiert, auf dem Quotas und
Heimatverzeichnisse verwaltet werden sollen.

%files lamdaemon
%dir /usr/share/%{lam_dir}
%dir /usr/share/%{lam_dir}/lib
/usr/share/%{lam_dir}/lib/lamdaemon.pl
%doc COPYING HISTORY README VERSION docs/*

