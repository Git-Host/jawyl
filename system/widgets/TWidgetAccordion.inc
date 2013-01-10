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
	require_once 'widgets/TWidgetContainer.inc';

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * widget displaying widgets in accordion
 */	
	class TWidgetAccordion extends TWidgetContainer {
		
		protected function inlineJavaScript(){
			?>
		jQuery(document).one("ready",function(e){jQuery("#<?php echo $this->Name(); ?>").accordion2().construct();});
			<?php
		}
/**
 * @see TJQueryWidget::linkCustomResources()
 */	
		protected function linkCustomResources(){
			$this->LoadAccordion();
		}
/**
 * @see TWidget::DefaultOutput()
 */				
		protected function defaultOutput(IThemeEngine $engine){
?>
<div id="<?php echo $this->Name(); ?>">
<?php foreach ($this->Widgets() as $w){ ?>
<div>
<span><?php __i($this->Title($w->Name())); ?></span>
<div><?php $w->Output($engine); ?></div>
</div>
<?php 
} 
?>
</div>
<?php			
		}
	}
?>