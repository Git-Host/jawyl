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
TApplication::import("common::controller::interfaces");
/**
 * @ignore
 */
TApplication::import("common::widget::interfaces");
/**
 * @ignore
 */
TApplication::import("widgets::areas::TAreasWidget");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * command bar
 */
class TAreasCommandBar extends TAreasWidget {
	private $_block_ = array();

/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "actions":return $this->Actions();break;
			case "BlockActions":return $this->_block_;break;
		}
		return parent::__get($nm);
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "BlockActions":$this->_block_ = TConvertions::ConvertToArray($value,";");break;
			case "BlockAction":$this->_block_[] = $value;break;
			default:parent::__set($nm,$value);break;
		}	
	}
	
	
	protected function checkSource($src){
		return ($src instanceof IActionsList) && parent::checkSource($src);
	}
	
/**
 * gest controller actions
 * @return array of TControllerAction
 */
	public function Actions(){
		$result = array();
		$block = array();
		$sources = $this->Sources;
		foreach ($sources as $src){
			$actions = $src->Actions();	
			foreach ($actions as $key=>$value)
				if (!in_array($key,$this->BlockActions)){
					$group = $src->ActionGroup($value);
					if (!key_exists($group,$result)) $result[$group] = array();
					$result[$group][$key]=$value;
				}
		}
		return $result;
	}
	
/**
 * checks whether action is active
 * @param string $name action name
 * @return boolean
 */	
	public function IsActive($name){
		$action = TConfigurable::Instance($name);
		return $action->Available();
	}
	
	protected function onActionStateEvent(TActionStateEvent $event){
		$this->refreshArea($this->GetArea($event->Action,"Action"));
	}
	
	protected function renderAction(TWidgetArea $area){
		$a = $area->Context();
?>
<button id="<?php echo $this->Name(); ?>_<?php echo $a->Name(); ?>" class="ui-corner-all" <?php echo ($a->Available())?'onclick="javascript:jQuery.areas_AjaxCall(&quot;'.$a->CallLink().'&quot;);"':'disabled=true'; ?>><?php __i($a->Title()); ?></button>
<?php
		
	}
	
	protected function renderContainer(TWidgetArea $area){
		$list = $this->Actions();
		foreach ($list as $group=>$actions){
?>
<div class="areas-command-group">
<div class="areas-command-group-title"><?php echo $group; ?></div>
<?php			
			foreach ($actions as $a) $this->RenderArea($this->GetArea($a,"Action"));
?>
</div>
<?php			
		}
	}

/**
 * @see TWidget::DefaultOutput()
 */	
	protected function defaultOutput(IThemeEngine $engine){
?>
<div id="<?php echo $this->Name() ?>" class="commandbar">
<?php 
$this->renderArea($this->GetArea($this,"Container"));
?>
</div>
<?php		
	}
}
?>