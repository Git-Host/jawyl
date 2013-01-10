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
require_once 'widgets/areas/TAreasPropTable.inc';

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * tabbed properties widget
 */
class TAreasPropTabs extends TAreasPropTable {
/**
 * @see TJQueryWidget::linkCustomResources()
 */	
	protected function linkCustomResources(){
		parent::linkCustomResources();
		self::LoadTabControl();
	}
/**
 * @see TJQueryWidget::jsCode()
 */	
	protected function javaScript(){
		?>jQuery(document).one("ready",function(e){$("#<?php echo $this->Name(); ?>").tabcontrol().construct();});<?php	
	}
	
	protected function renderContents($context){
			foreach ($this->Properties() as $p){ 
				?>
				<div>
					<span><?php echo htmlentities(_i($p->Caption()),ENT_QUOTES,"UTF-8"); ?></span>
					<?php $this->RenderArea($this->GetArea($p,"Property")); ?>
				</div>
				<?php		
			}
	}
}
?>