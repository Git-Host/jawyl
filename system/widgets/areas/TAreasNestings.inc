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
TApplication::import("widgets::areas::TAreasPagedList");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * nested and paged list view widget, needs source to be INestedItems
 * @see INestedItems
 */
class TAreasNestings extends TAreasPagedList {
	protected function checkSource($src){
		return ($src instanceof INestedItems) && parent::checkSource($src); 
	}	
/**
 * moves item
 * @param string $id item id
 * @param string $parent parent id
 */	
	public function http_MoveItem($id,$parent){
		$this->Source->MoveItem($this->extractWrappedId($id),$this->extractWrappedId($parent));
	}
/**
 * opens item
 * @param string $id
 */	
	public function http_OpenItem($id){
		$this->Source->OpenItem($this->extractWrappedId($id));
	}
/**
 * opens previous nesting level
 */	
	public function http_LevelUp(){
		$this->Source->LevelUp();
	}
/**
 * gets current opened item
 * @return TListItem
 */	
	public function CurrentContainer(){
		if (!($this->Source instanceof INestedItems))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		$cc = $this->Source->CurrentContainer();
		if (!is_null($cc))
			return $this->WrapItem($cc);
		return null;
	}
	
	protected function isSublevel(){
		return $this->CurrentContainer()?true:false;
	}
	
	protected function renderCaption(TWidgetArea $area){
		$context = $area->Context();
		?>
		<span id="<?php echo $area->Id() ?>_caption" <?php echo ($context->IsSelected()?' class="selected"':''); ?> onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $context->SelectLink(); ?>&quot;);" ondblclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"OpenItem",array("id"=>$context->ItemId())); ?>&quot;);"><?php echo $context; ?></span>
		<?php
	}

	protected function renderContainer(TWidgetArea $area){
		if ($this->isSublevel()){
		?>
		<div class="level-up" ondblclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"LevelUp",array()); ?>&quot;);">...</div>
		<?php
		}
		parent::renderContainer($area);
	}	
}
?>