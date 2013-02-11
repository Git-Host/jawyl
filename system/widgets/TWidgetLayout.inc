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
 * widget displaying widgets in table layout
 */	
	class TWidgetLayout extends TXmlWidgetContainer {
		const LAYOUT_VERTICAL = "v";
		const LAYOUT_HORIZONTAL = "h";
		
		private $_sizes_ = array();
				
		public $Layout;
		
		public function __set($nm,$value){
			switch ($nm){
				case "Sizes":$this->_sizes_ = TConvertions::ConvertToArray($value,";");break;
				case "Size":$this->_sizes_[] = $value;break;
				default:return parent::_get($nm);break;
			}
		} 
		
		public function __get($nm){
			switch ($nm){
				case "Sizes":return $this->_sizes_;break;
				default:return parent::_get($nm);break;
			}
		}
		
/**
 * @see TWidget::DefaultOutput()
 */		
		protected function defaultOutput(IThemeEngine $engine){
?>
<table id="<?php echo $this->Name(); ?>" class="ui-widget-<?php echo $this->Layout; ?>layout">
<?php if ($this->Layout == self::LAYOUT_HORIZONTAL) { ?><tr><?php } ?>
<?php 
	$i = 0;
foreach ($this->Widgets() as $w) { 
	if ($this->Layout == self::LAYOUT_VERTICAL) { ?><tr<?php echo ((key_exists($i,$this->Sizes))?' style="height:'.$this->Sizes[$i].';" ':''); ?>><?php } ?>
<td<?php echo (($this->Layout == self::LAYOUT_HORIZONTAL) && (key_exists($i,$this->Sizes))?' style="width:'.$this->Sizes[$i].';" ':''); ?>><?php $w->Output($engine); ?></td>
<?php if ($this->Layout == self::LAYOUT_VERTICAL) { ?></tr><?php } 
	$i++; 
} 
 if ($this->Layout == self::LAYOUT_HORIZONTAL) { ?></tr><?php } ?>
</table>
<?php			
		}		
	}
?>