<?
echo '<tr><td>Userinformation</td></tr>';
echo '<tr><td><br></td></tr>';
echo '<tr><td>'.$_SESSION['account']->personal_title .' '. $_SESSION['account']->personal_surname .' '. $_SESSION['account']->personal_givenname . '</td></tr>';
echo '<tr><td>'.$_SESSION['account']->personal_employeeType.'</td></tr>';
echo '<tr><td>'.$_SESSION['account']->personal_street.'</td></tr>';
echo '<tr><td>'.$_SESSION['account']->personal_postalCode.$_SESSION['account']->personal_postalAddress.'</td></tr>';
echo '<tr><td><br></td></tr>';
echo '<tr><td>Telephone: '.$_SESSION['account']->personal_telephoneNumber.'</td></tr>';
echo '<tr><td>Mobile Phone: '.$_SESSION['account']->personal_mobileTelephoneNumber.'</td></tr>';
echo '<tr><td>Fax Number: '.$_SESSION['account']->personal_facsimileTelephoneNumber.'</td></tr>';
echo '<tr><td>eMail-Address: '.$_SESSION['account']->personal_mail.'</td></tr>';
echo '<tr><td><br></td></tr>';
echo '<tr><td>Username: '.$_SESSION['account']->general_username.'</td><td>UID-Number: '.$_SESSION['account']->general_uidNumber.'</td></tr>';
echo '<tr><td>Unix-Password: '.$_SESSION['account']->unix_password.'</td></tr>';
echo '<tr><td>Groupname: '.$_SESSION['account']->general_group.'</td><td>GID-Number: </td></tr>';
echo '<tr><td>User is also member of the groups: ';
foreach ($_SESSION['account']->general_groupadd[] as $group) echo $group.' ';
echo '</td></tr>';
echo '<tr><td>Homedirectory: '.$_SESSION['account']->general_homedir.'</td><td>Shell: '.$_SESSION['account']->general_shell.'</td></tr>';
echo '<tr><td><br></td></tr>';
echo '<tr><td>Windows-Password: '.$_SESSION['account']->smb_password.'</td></tr>';
echo '<tr><td>Windows-Domain: '.$_SESSION['account']->smb_domain.'</td></tr>';
echo '<tr><td>Allowed workstations: '.$_SESSION['account']->smb_smbuserworkstations.'</td></tr>';
echo '<tr><td>Windows-Homedir: '.$_SESSION['account']->smb_smbhome.'</td></tr>';
echo '</body></html>';
?>