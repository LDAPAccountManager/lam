<?php

app_session_start();
$lamConfig = $_SESSION['config'];
$lamLdap = $_SESSION['ldap'];
$lamLogin = $lamLdap->decrypt_login();

$servers = new Datastore();
$servers->newServer('ldap_pla');
$servers->setValue('server','name',null);
$servers->setValue('server','host',$lamConfig->get_ServerURL());
$servers->setValue('server','base',array($lamConfig->get_Suffix('tree')));
$servers->setValue('login','auth_type','config');
$servers->setValue('login','bind_id',$lamLogin[0]);
$servers->setValue('login','bind_pass',$lamLogin[1]);
if ($lamConfig->getUseTLS() == 'yes') {
	$servers->setValue('server','tls',true);
}
$config->custom->commands['cmd'] = array(
	'entry_internal_attributes_show' => true,
	'entry_refresh' => true,
	'oslinks' => false,
	'switch_template' => false
);
$config->custom->commands['script'] = array(
	'add_attr_form' => true,
	'add_oclass_form' => true,
	'add_value_form' => true,
	'collapse' => true,
	'compare' => true,
	'compare_form' => true,
	'copy' => true,
	'copy_form' => true,
	'create' => true,
	'create_confirm' => true,
	'delete' => true,
	'delete_attr' => true,
	'delete_form' => true,
	'draw_tree_node' => true,
	'expand' => true,
	'export' => true,
	'export_form' => true,
	'import' => true,
	'import_form' => true,
	'login' => true,
	'logout' => true,
	'login_form' => true,
	'mass_delete' => true,
	'mass_edit' => true,
	'mass_update' => true,
	'modify_member_form' => true,
	'monitor' => false,
	'purge_cache' => false,
	'query_engine' => true,
	'rename' => true,
	'rename_form' => true,
	'rdelete' => true,
	'refresh' => true,
	'schema' => false,
	'server_info' => false,
	'show_cache' => false,
	'template_engine' => true,
	'update_confirm' => true,
	'update' => true
);
$config->custom->appearance['show_schema_link'] = false;
if (!checkIfWriteAccessIsAllowed()) {
	$servers->setValue('server','read_only',true);
}
$servers->setValue('unique','attrs',array());
?>
