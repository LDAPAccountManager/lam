<!-- $Horde: nms/templates/notconfigured.inc,v 1.2 2002/06/15 05:05:30 chuck Exp $-->
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "DTD/xhtml1-transitional.dtd">
<html>
<head>
<title><?php echo _("Nms module is not properly configured") ?></title>
<style type="text/css">
<!--
<?php include NMS_BASE . '/templates/css.inc'; ?>
-->
</style>
</head>
<body>
<table border="0" align="center" width="500" cellpadding="2" cellspacing="4">
<tr><td colspan="2" class="header"><b><?php echo _("Some of NMS module's configuration files are missing:") ?></b></td></tr>

<?php if (!@is_readable('./config/conf.php')): ?>
<tr>
  <td align="right" class="smallheader"><b>conf.php</b></td>
  <td class="light"><?php echo _("This is the main NMS module configuration file. It contains paths and options for all NMS module scripts.") ?></td>
</tr>
<?php endif; ?>

<?php if (!@is_readable('./config/html.php')): ?>
<tr>
  <td align="right" class="smallheader"><b>html.php</b></td>
  <td class="light"><?php echo _("This file controls the stylesheet that is used to set colors and fonts in addition to or overriding Horde defaults.") ?></td>
</tr>
<?php endif; ?>

<?php if (!@is_readable('./config/prefs.php')): ?>
<tr>
  <td align="right" class="smallheader"><b>prefs.php</b></td>
  <td class="light"><?php echo _("This file controls the default preferences for NMS module, and also controls which preferences users can alter.") ?></td>
</tr>
<?php endif; ?>

</table>
</body>
</html>
