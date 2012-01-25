/**
 * CKEditor configuration for LAM
 */

CKEDITOR.editorConfig = function( config ) {

/* reduced toolbar */
config.toolbar = 'LAM';
config.toolbar_LAM = [
	{ name: 'basicstyles', items : [ 'Bold','Italic','Underline','Strike','Subscript','Superscript','-','RemoveFormat' ] },
	{ name: 'paragraph', items : [ 'NumberedList','BulletedList','-','Outdent','Indent','-','Blockquote','CreateDiv','-',
	'JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock','-','BidiLtr','BidiRtl' ] },
	{ name: 'links', items : [ 'Link','Unlink' ] },
	{ name: 'insert', items : [ 'Image','Table','HorizontalRule','SpecialChar','PageBreak','Iframe' ] },
	'/',
	{ name: 'styles', items : [ 'Styles','Format','Font','FontSize' ] },
	{ name: 'colors', items : [ 'TextColor','BGColor' ] },
	{ name: 'document', items : [ 'Source','-','DocProps' ] },
	{ name: 'clipboard', items : [ 'Undo','Redo' ] },
	{ name: 'tools', items : [ 'Maximize', 'ShowBlocks' ] },
];

/* style */
config.skin = 'office2003';
config.width = '70em';
config.height = 100;

/* no bottom bar */
config.removePlugins = 'elementspath';
config.resize_enabled = false;

};
