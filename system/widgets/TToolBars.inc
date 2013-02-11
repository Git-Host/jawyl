<?php
/**
 * @package View
 * @subpackage Widgets
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
	require_once 'widgets/TXmlWidgetContainer.inc';

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * widget that displays toolbars (command bars)
 */	
	class TToolBars extends TXmlWidgetContainer {
/**
 * @see TWidget::DefaultOutput()
 */		
		protected function defaultOutput(IThemeEngine $engine){
?>
<div id="<?php echo $this->Name(); ?>" class="panel">
<?php foreach ($this->Widgets() as $w){ ?>
<div class="toolbar">
<div class="title">
<?php echo _i($this->Title($w->Name())); ?>:
</div>
<?php $w->Output($engine) ?>
</div>
<?php } ?>
</div>
<?php			
		}	
	}
?>