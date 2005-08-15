<?php
/*
$Id$

  Copyright (C) 2004 David Smith
  modified to fit for LDAP Account Manager 2005 Roland Gruber

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


/**
 * Displays the LDAP schema of the server
 *
 * @package tools
 * @author David Smith
 * @author Roland Gruber
 */
 
 
/** access to LDAP server */
include_once("../../lib/ldap.inc");
/** access to configuration options */
include_once("../../lib/config.inc");
/** schema functions */
require_once("../../lib/schema.inc");

// start session
session_save_path("../../sess");
@session_start();

setlanguage();

echo $_SESSION['header'];


echo "<title>LDAP Account Manager</title>\n";
echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/layout.css\">\n";
echo "</head>\n";


$view = isset( $_GET['view'] ) ? $_GET['view'] : 'objectClasses';
$viewvalue = isset( $_GET['viewvalue'] ) ? $_GET['viewvalue'] : null; 
if( trim( $viewvalue ) == "" )
    $viewvalue = null;
if( ! is_null( $viewvalue ) )
    $viewed = false;

?>

<body>

<br />
<center><big>
	<?php echo ( $view=='objectClasses' ?
		_('Object classes') :
		'<a href="?view=objectClasses">' . _('Object classes').'</a>' ); ?>
		|
	<?php echo ( $view=='attributes' ?
		_('Attribute types'):
		'<a href="?view=attributes">' .	_('Attribute types').'</a>' ); ?>
		|
	<?php echo ( $view=='syntaxes' ?
		_('Syntaxes') :
		'<a href="?view=syntaxes">' . _('Syntaxes').'</a>' ); ?>
		|
	<?php echo ( $view=='matching_rules' ?
		_('Matching rules') :
		'<a href="?view=matching_rules">' . _('Matching rules').'</a>' ); ?>
</big></center>
<br />

<?php flush(); ?>

<?php

if( $view == 'syntaxes' ) {
	$highlight_oid = isset( $_GET['highlight_oid'] ) ? $_GET['highlight_oid'] : false;
	echo "\n\n<table class=\"schema_attr\" width=\"100%\">\n";
	echo "<tr><th>" . _('Syntax OID') . "</th><th>" . _('Description') . "</th></tr>\n";
	flush();
	$counter=1;
	$schema_syntaxes = get_schema_syntaxes(null); 
	if( ! $schema_syntaxes ) StatusMessage("ERROR", _("Unable to retrieve schema!"), "");
	foreach( $schema_syntaxes as $syntax ) {
		$counter++;
		$oid =  htmlspecialchars( $syntax->getOID() );
		$desc = htmlspecialchars( $syntax->getDescription() );
		if( $highlight_oid && $highlight_oid == $oid )
			echo "<tr class=\"highlight\">";
		else
			echo "<tr class=\"" . ($counter%2==0?'even':'odd'). "\">";
		echo "<td><a name=\"$oid\">$oid</a></td><td>$desc</td></tr>\n\n";
	}
	echo "</table>\n";

} elseif( $view == 'attributes' ) {
	flush();
	$schema_attrs = get_schema_attributes(null);
	$schema_object_classes = get_schema_objectclasses(null);
	if( ! $schema_attrs || ! $schema_object_classes ) 
		StatusMessage("ERROR", _("Unable to retrieve schema!"), "");

	?>
	<small><?php echo _('Jump to an attribute type'); ?>:</small>
	<form action="schema.php" method="get"><input type="hidden" name="view" value="<?php echo $view; ?>" />
        <select name="viewvalue" onChange="submit()">
	<option value=""> - all -</option>

	<?php foreach( $schema_attrs as $attr ) { 		
                    echo( '<option value="'
                         .$attr->getName()
                         .'" '
                         .( 0 == strcasecmp( $attr->getName(), $viewvalue ) ? ' selected ' : '' )
                         .'>'
                         . $attr->getName()
		         .'</option>' . "\n" );
	 } ?>
	</select><input type="submit" value="<?php echo _('Go'); ?>" /></form>

	<br />
	<table class="schema_attr" width="100%">

	<?php 
    foreach( $schema_attrs  as $attr ) {
	  if ( is_null( $viewvalue ) || 0 == strcasecmp( $viewvalue, $attr->getName() ) ) {
        if( ! is_null( $viewvalue ) )
            $viewed = true;
		flush();
		echo "<tr><th colspan=\"2\">" . $attr->getName() . "</th></tr>\n\n";
		$counter = 0;

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Description')."</td>\n";
		echo "<td>" . ( $attr->getDescription() == null ? '('._('No description').')' : $attr->getDescription() ). "</td>\n";
		echo "</tr>\n\n";
		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td><acronym title=\"Object Identier\">"._('OID')."</acronym></td>\n";
		echo "<td>" .  $attr->getOID() . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo '<td>'._('Obsolete')."?</td>\n";
		echo "<td>" . ( $attr->getIsObsolete() ? '<b>' . _('Yes') . '</b>' : _('No') ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Inherits from')."</td>\n";
		echo "<td>";
		if( $attr->getSupAttribute()==null )
			echo '('._('none').')';
		else
			echo "<a href=\"?view=$view&amp;viewvalue=" . strtolower( $attr->getSupAttribute() ) . "\">" . $attr->getSupAttribute()  . "</a></td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Equality')."</td>\n";
		echo "<td>" .  ( $attr->getEquality() == null ? '('._('not specified').')' : "<a href=\"?view=matching_rules&amp;viewvalue=".$attr->getEquality()."\">".$attr->getEquality()."</a>" ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Ordering')."</td>\n";
		echo "<td>" .  ( $attr->getOrdering()==null? '('._('not specified').')' : $attr->getOrdering() ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Substring Rule')."</td>\n";
		echo "<td>" .  ( $attr->getSubstr()==null? '('._('not specified').')' : $attr->getSubstr() ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Syntax')."</td>\n";
		echo "<td>";
		if( null != $attr->getType() ) {
			echo "<a href=\"?view=syntaxes&amp;highlight_oid=";
			echo $attr->getSyntaxOID() . "#" .  $attr->getSyntaxOID();
			echo "\">" . $attr->getType() . " (" . $attr->getSyntaxOID() . ")</a>";
		} else {
			echo $attr->getSyntaxOID();
		}
		echo "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Single valued')."</td>\n";
		echo "<td>" .  ( $attr->getIsSingleValue() ? _('Yes') : _('No') ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Collective')."?</td>\n";
		echo "<td>" .  ( $attr->getIsCollective() ? _('Yes') : _('No') ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('User modification')."</td>\n";
		echo "<td>" . ( $attr->getIsNoUserModification() ? _('No') : _('Yes') ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Usage')."</td>\n";
		echo "<td>" .  ( $attr->getUsage() ? $attr->getUsage() : '('._('not specified').')' ) . "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Maximum length')."</td>\n";
		echo "<td>";
		if ( $attr->getMaxLength() === null ) { echo '('._('not applicable').')';}
		else {
		  echo number_format( $attr->getMaxLength() ) ." ";
		  if (  $attr->getMaxLength()>1) {echo _('characters');}
		  else { echo _('character')  ;}
	                        } 
		echo "</td>\n";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Aliases')."</td>\n";
		echo "<td>"; 
		if( count( $attr->getAliases() ) == 0 )
			echo '('._('none').')';
		else
			foreach( $attr->getAliases() as $alias_attr_name )
				echo "<a href=\"?view=attributes&amp;viewvalue=" . $alias_attr_name. "\">$alias_attr_name</a> ";
		echo "</td>";
		echo "</tr>\n\n";

		echo "<tr class=\"" . (++$counter%2==0?'even':'odd') . "\">\n";
		echo "<td>"._('Used by object classes')."</td>\n";
		echo "<td>";
		if( count( $attr->getUsedInObjectClasses() ) == 0 )
			echo '('._('none').')';
		else
			foreach( $attr->getUsedInObjectClasses() as $used_in_oclass)
				echo "<a href=\"?view=objectClasses&amp;viewvalue=" .
					$used_in_oclass. "\">$used_in_oclass</a> ";
		echo "</td>";
		echo "</tr>\n\n";

		flush();
	  }
	}
	echo "</table>\n";

} elseif( $view == 'matching_rules' ) {
        $schema_matching_rules = get_schema_matching_rules(null); 
	echo '<small>' . _('Jump to a matching rule').'</small><br />';
	echo '<form  action="schema.php" method="get">';
        echo '<input type="hidden" name="view" value="matching_rules" />';
        echo '<select name="viewvalue" onChange="submit()">';
        echo '<option value=""> - all -</option>';
		foreach( $schema_matching_rules as $rule ) {
		  echo '<option value="'.$rule->getName().'"'.($rule->getName()==$viewvalue? ' selected ': '').'>'.$rule->getName().'</option>';
		}
        
        echo '</select>';
       	echo '<input type="submit" value="'._('Go').'" />';
	echo '</form>';
	echo "\n\n<table class=\"schema_attr\" width=\"100%\">\n";
	echo "<tr><th>" . _('Matching rule OID') . "</th><th>" . _('Name') . "</th><th>"._('Used by attributes')."</th></tr>\n";
	flush();
	$counter=1;
	$schema_matching_rules = get_schema_matching_rules(null); 
	if( ! $schema_matching_rules ) StatusMessage("ERROR", _("Unable to retrieve schema!"), "");
	foreach( $schema_matching_rules as $rule ) {
		$counter++;
		$oid = htmlspecialchars( $rule->getOID() );
		$desc = htmlspecialchars( $rule->getName() );
		if ( $viewvalue==null || $viewvalue==($rule->getName() )) {
        if( ! is_null( $viewvalue ) )
            $viewed = true;
		if( null != $rule->getDescription() )
			$desc .= ' (' . $rule->getDescription() . ')';
		if( true === $rule->getIsObsolete() )
			$desc .= ' <span style="color:red">' . _('Obsolete') . '</span>';
		echo "<tr class=\"" . ($counter%2==0?'even':'odd'). "\">";
		echo "<td>$oid</td>";
		echo "<td>$desc</td>";
		echo "<td>";
		if( count( $rule->getUsedByAttrs() ) == 0 ) {
			echo "<center>(" . _('none') . ")</center><br /><br />\n";
		} else {
			echo "<table><tr><td style=\"text-align: right\"><form action=\"schema.php\" method=\"get\">";
			echo "<input type=\"hidden\" name=\"view\" value=\"attributes\" />";
			echo "<select style=\"width: 150px; color:black; background-color: #eee\" size=\"4\" name=\"viewvalue\">\n";
			foreach( $rule->getUsedByAttrs() as $attr )
				echo "<option>$attr</option>\n";
			echo "</select><br /><input type=\"submit\" value=\""._('Go')."\" /></form></td></tr></table>\n";
		}
		echo "</td></tr>\n";
		  }
	}
	echo "</table>\n";

} elseif( $view == 'objectClasses' ) { 
	flush();
	$schema_oclasses = get_schema_objectclasses(null);
	if( ! $schema_oclasses ) StatusMessage("ERROR", _("Unable to retrieve schema!"), "");
	?>
	<small><?php echo _('Jump to an object class'); ?>:</small>
	<form action="schema.php" method="get"><input type="hidden" name="view" value="<?php echo $view; ?>" />
	<select name="viewvalue"
	onChange="submit()">
        <option value=""> - all - </option>
	<?php foreach( $schema_oclasses as $name => $oclass ) { 
		echo '<option value="'
		     .$oclass->getName()
                     .'"'
                     . ( 0 == strcasecmp( $oclass->getName(), $viewvalue ) ? ' selected ':'')
                     .'>'.$oclass->getName()
		     .'</option>';
	 } ?>
        </select><input type="submit" value="<?php echo _('Go'); ?>" />
        </form>

        <?php flush(); ?>

    <?php foreach( $schema_oclasses as $name => $oclass ) { 
        foreach( $oclass->getSupClasses() as $parent_name ) { 
            $parent_name = $parent_name;
            if( isset( $schema_oclasses[ $parent_name ] ) ) {
                $schema_oclasses[ $parent_name ]->addChildObjectClass( $oclass->getName() );
            }
        }

    } ?>

	<br />
	<?php foreach( $schema_oclasses as $name => $oclass ) {
	  if ( $viewvalue==null || 0 == strcasecmp( $viewvalue, $oclass->getName() ) ){
        if( ! is_null( $viewvalue ) )
            $viewed = true; 
        ?>
        
		<h4 class="schema_oclass"><?php echo $oclass->getName(); ?></h4>
		<h4 class="schema_oclass_sub"><?php echo _('OID'); ?>: <b><?php echo $oclass->getOID(); ?></b></h4>
		<?php if( $oclass->getDescription() ) { ?>
			<h4 class="schema_oclass_sub"><?php echo _('Description'); ?>: <b><?php echo $oclass->getDescription(); ?></b></h4>
		<?php } ?>
		<h4 class="schema_oclass_sub"><?php echo _('Type'); ?>: <b><?php echo $oclass->getType(); ?></b></h4>
		<?php if( $oclass->getIsObsolete() == true ) { ?>
			<h4 class="schema_oclass_sub"><?php echo _('This object class is obsolete.'); ?></h4>
		<?php } ?>

		<h4 class="schema_oclass_sub"><?php echo _('Inherits from'); ?>: <b><?php 
		if( count( $oclass->getSupClasses() ) == 0 )
			echo "(" . _('none') . ")";
		else
			foreach( $oclass->getSupClasses() as $i => $object_class ) {
				echo '<a title="' . _('Jump to an object class') . ' " 
					href="?view='.$view.'&amp;viewvalue='.htmlspecialchars( $object_class ) ;
				echo '">' . htmlspecialchars( $object_class ) . '</a>';
				if( $i < count( $oclass->getSupClasses() ) - 1 )
					echo ', ';
		}
		?></b></h4>

		<h4 class="schema_oclass_sub"><?php echo _('Parent to'); ?>: <b><?php 
        if( 0 == strcasecmp( $oclass->getName(), 'top' ) )
            echo "(<a href=\"schema.php?view=objectClasses\">all</a>)";
		elseif( count( $oclass->getChildObjectClasses() ) == 0 )
			echo "(" . _('none') . ")";
		else
			foreach( $oclass->getChildObjectClasses() as $i => $object_class ) {
				echo '<a title="' . _('Jump to an object class') . ' " 
					href="?view='.$view.'&amp;viewvalue='.htmlspecialchars( $object_class ) ;
				echo '">' . htmlspecialchars( $object_class ) . '</a>';
				if( $i < count( $oclass->getChildObjectClasses() ) - 1 )
					echo ', ';
		}
		?></b></h4>

		<table width="100%" class="schema_oclasses">
		<tr>
			<th width="50%"><b><?php echo _('Required attributes'); ?></b></th>
			<th width="50%"><b><?php echo _('Optional attributes'); ?></b></th>
		</tr>
		<tr>
			<td>
			<?php if( count( $oclass->getMustAttrs($schema_oclasses) ) > 0 ) {
				echo '<ul class="schema">';
				foreach( $oclass->getMustAttrs($schema_oclasses) as $attr ) {
					echo "<li><a href=\"?view=attributes&amp;viewvalue=";
					echo rawurlencode( $attr->getName()  ). "\">" . htmlspecialchars($attr->getName());
					echo "</a>";
					if( $attr->getSource() != $oclass->getName() )
					{
						echo "<br /><small>&nbsp;&nbsp;("._('Inherited from')." ";
						echo "<a href=\"?view=objectClasses&amp;viewvalue=" . $attr->getSource()  . "\">" . $attr->getSource() . "</a>";
						echo ")</small>";
					}
					echo "</li>\n";
				}
				echo "</ul>";
			} else				
				echo "<center>(" . _('none') . ")</center>\n";
			?>
		</td>
		<td width="50%">
		<?php 
		if( count( $oclass->getMayAttrs($schema_oclasses) ) > 0 ) {
			echo '<ul class="schema">';
			foreach( $oclass->getMayAttrs($schema_oclasses) as $attr ) {
				echo "<li><a href=\"?view=attributes&amp;viewvalue=";
				echo rawurlencode( $attr->getName() ) . "\">" . htmlspecialchars($attr->getName() );
				echo "</a>\n";
				if( $attr->getSource() != $oclass->getName() )
				{
					echo "<br /><small>&nbsp;&nbsp; ("._('Inherited from')." ";
					echo "<a href=\"?view=objectClasses&amp;viewvalue=" . $attr->getSource()  . "\">" . $attr->getSource() . "</a>";
					echo ")</small>";
				}
				echo "</li>";
			}
			echo "</ul>";
		}
		else				
			echo "<center>(" . _('none') . ")</center>\n";
	?>

	</td>
	</tr>
	</table>

	<?php }  } /* End foreach objectClass */ ?>
<?php } /* End else (displaying objectClasses */ ?>

<?php if( ! is_null( $viewvalue ) && ! $viewed )
    StatusMessage("ERROR",  sprintf( _('No such schema item: "%s"'), htmlspecialchars( $viewvalue ) ) );
?>

</body>

</html>
