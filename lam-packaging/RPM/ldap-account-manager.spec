%define httpd_rootdir /var/www/html
%define lam_dir lam
%define lam_uid apache
%define lam_gid apache

Name:         ldap-account-manager
License:      GPL
Group:        Productivity/Networking/Web/Frontends
Version:      @@VERSION@@
Release:      1
Source0:      ldap-account-manager-%{version}.tar.gz
URL:          http://lam.sourceforge.net
BuildRoot:    %{_tmppath}/%{name}-%{version}-%{release}
Summary:      Administration of LDAP users, groups and hosts via Web GUI
BuildArchitectures: noarch
# Requires:     mod_php perl
# Autoreqprov:  on

%description
LDAP Account Manager (LAM) runs on an existing webserver. LAM
supports LDAP connections via SSL and TLS. It manages user, group
and host accounts. Currently LAM supports these account types:
Samba 2 and 3, Unix, Kolab 2, address book entries, NIS mail
aliases and MAC addresses. There is a tree viewer included to
allow access to the raw LDAP attributes. You can use templates
for account creation and use multiple configuration profiles.
Account information can be exported as PDF file. There is also
a script included which manages quota and homedirectories, you
have to setup sudo if you want to use it. LAM is translated to
Catalan, Chinese (Traditional), English, French, German,
Hungarian, Italian, Spanish and Japanese.

%prep
%setup -n ldap-account-manager-%{version}

%build

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}
cp -dR * $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}

%clean
[ "$RPM_BUILD_ROOT" != "/" ] && [ -d $RPM_BUILD_ROOT ] && rm -rf $RPM_BUILD_ROOT

%post
chown %{lam_uid}.%{lam_gid} -R $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}/config
chown %{lam_uid}.%{lam_gid} -R $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}/tmp
chown %{lam_uid}.%{lam_gid} -R $RPM_BUILD_ROOT%{httpd_rootdir}/%{lam_dir}/sess

%files
%defattr(-, root, root)
%doc docs/*txt COPYING HISTORY INSTALL README TODO VERSION
%{httpd_rootdir}/%{lam_dir}

%changelog -n lam
* Wed Jan 11 2006 - Iain Lea iain@bricbrac.de
- Updated for 1.0 series on Fedora Core 

* Mon Dec 12 2005 - Iain Lea iain@bricbrac.de
- Updated for 0.5.x series on Fedora Core 

* Sun Mar 21 2004 - TiloLutz@gmx.de
- Initial release 0.1.0 - 0.4.5
