# SPEC file

%global c_vendor    %{_vendor}
%global gh_owner    %{_owner}
%global gh_project  %{_project}

Name:      %{_package}
Version:   %{_version}
Release:   %{_release}%{?dist}
Summary:   This library includes PHP classes to read byte-level data from files

License:   LGPLv3+
URL:       https://github.com/%{gh_owner}/%{gh_project}

BuildArch: noarch

Requires:  php(language) >= 8.2.0
Requires:  php-curl
Requires:  php-pcre

Provides:  php-composer(%{c_vendor}/%{gh_project}) = %{version}
Provides:  php-%{gh_project} = %{version}

%description
This library includes PHP classes to read byte-level data from files.

%build
#(cd %{_current_directory} && make build)

%install
rm -rf %{buildroot}
(cd %{_current_directory} && make install DESTDIR=%{buildroot})

%files
%attr(-,root,root) %{_libpath}
%attr(-,root,root) %{_docpath}
%docdir %{_docpath}
# Optional config files can be listed here when used by a project.

%changelog
* Sun Feb 05 2026 Nicola Asuni <info@tecnick.com> 1.6.4-1
- Update dependencies
* Mon Jul 27 2026 Nicola Asuni <info@tecnick.com> 1.0.0-1
- Initial Commit
