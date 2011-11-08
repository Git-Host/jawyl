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
TApplication::import("common::service::interfaces");
/**
 * @ignore
 */
TApplication::import("widgets::areas::TAreasWidget");


/**
 * @package View
 * @subpackage Widgets
 * @category System
 * @property IIterator $Items list items iterator
 * @property IItemNavigation $ItemNavigator
 * list view widget 
 */
class TAreasList extends TAreasWidget implements IList {
/**
 * @var IItemNavigation
 */	
	private $_ioc_item_navigator_;
/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "Items":return $this->Items();break;
		}	
		return parent::__get($nm);
	}
	
	protected function preSetting(){
		if ($this instanceof IItemNavigation)
			$this->_ioc_item_navigator_ = $this;
		else if ($this->response instanceof IItemNavigation)
			$this->_ioc_item_navigator_ = $this->response;		
		else if ($this->response->Service() instanceof IItemNavigation)
			$this->_ioc_item_navigator_ = $this->response->Service();		
	}
/**
 * currently selected list item
 * @return TListItem
 */	
	public function SelectedItem(){
		if (!is_null($this->Source->CurrentItem()))
			return $this->WrapItem($this->Source->CurrentItem());
		return null;
	}
/**
 * selects list item
 * @param string $id
 */	
	public function http_SelectItem($id){
		$selected = $this->SelectedItem();
		if (!is_null($selected)){
			if ($selected->ItemId() == $id){
				$this->Source->SelectItem(null);
				return;
			}
		}
		$this->Source->SelectItem($this->extractWrappedId($id));
	}
	
	public function http_AcceptItem($id){
		if ($this->Source instanceof IItemsCollection)
			$this->Source->AddItem($id);
	} 
	
	public function WrapItem(IItem $item){
		return new TListItem($item,$this);
	}
	
	protected function itemCaption(TListItem $node){
		return htmlentities($node->__toString(),ENT_QUOTES,"UTF-8");
	}	

/**
 * converts source item id to list item id
 * @param mixed $itemid
 * @return string 
 */	
	public function WrapId($itemid){
		return $this->Name().$itemid;
	}
	
	public function SelectionLink(IItem $item){
		return $this->response->Url($this->Name(),"SelectItem",array("id"=>$item->ItemId()));
	}
	
	public function OpenLink(IItem $item){
		if ($this->ItemNavigator instanceof IItemNavigation)
			return $this->ItemNavigator->ItemNavigationUrl($item->Item());
		return "";
	}	
/**
 * extracts wrapped item id from list item id
 * @param string $listitemid
 * @return mixed
 */	
	protected function extractWrappedId($listitemid){
		return preg_replace('/\\A'.$this->Name().'/','',$listitemid);
	}
/**
 * gets list orm items
 * @return IIterator iterator of TOrmItem
 * @see TOrmItem
 */	
	protected function getItems(){
		if (!($this->Source instanceof IItemsList))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE); 
		return $this->Source->Items();
	}
/**
 * gets list items
 * @return IIterator iterator of TListItem
 */	
	public function Items(){
		return new TListIteratorAdapter($this->getItems(),$this);
	}

/**
 * gets list filter action object
 * @return TControllerAction
 */	
	public function FilterAction(){
		if ($this->Source instanceof IFilterableItems)
			return $this->Source->FilterAction();
		return null;
	}
/**
 * gets list sorting action object
 * @return TControllerAction
 */	
	public function SortingAction(){
		if ($this->Source instanceof ISortableItems)
			return $this->Source->SortingAction();
		return null;
	}
	
	private function _on_add_event(TControllerEvent $event){
		$ci = $this->WrapItem($this->Source->Item($event->NodeId));
		$this->addArea($this->GetArea($ci,"Item",$this->GetArea($this,"Container")));
	}
	
	private function _on_remove_event(TControllerEvent $event){
		$this->deleteArea($this->WrapId($event->NodeId));
	}
	
	protected function onAddEvent(TItemAddEvent $event){
		$this->_on_add_event($event);
	}
	
	protected function onCreateEvent(TItemCreateEvent $event){
		$this->_on_add_event($event);
	}
	
	protected function onRemoveEvent(TItemRemoveEvent $event){
		$this->_on_remove_event($event);
	}
	
	protected function onDeleteEvent(TItemDeleteEvent $event){
		$this->_on_remove_event($event);
	}
	
	protected function onEditEvent(TItemEditEvent $event){
		$ei = $this->WrapItem($this->Source->Item($event->NodeId));
		$this->refreshArea($this->GetArea($ei,"Caption"));
	}
	
	protected function onSelectEvent(TItemSelectEvent $event){
		if (!is_null($event->PreviousNodeId)){
			$ri = $this->WrapItem($this->Source->Item($event->PreviousNodeId));
			$this->refreshArea($this->GetArea($ri,"Item",$this->GetArea($this,"Container")));
		}
		if (!is_null($event->NodeId)){
			$ri = $this->WrapItem($this->Source->Item($event->NodeId));
			$this->refreshArea($this->GetArea($ri,"Item",$this->GetArea($this,"Container")));
		}
	}
	
	protected function onReloadEvent(TDataReloadEvent $event){
		$this->refreshArea($this->GetArea($this,"Container"));
	}
	
	protected function renderCaption(TWidgetArea $area){
		$context = $area->Context();
		?>
		<span id="<?php echo $area->Id() ?>_caption" <?php echo ($context->IsSelected()?' class="selected"':''); ?> onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $context->SelectLink(); ?>&quot;);"><?php echo $context; ?></span>
		<?php
	}
	
	protected function renderItem(TWidgetArea $area){
		$context = $area->Context();
		$openlink = $context->OpenLink();
		if ($openlink){
			?><a href="<?php echo $openlink; ?>" class="open-link"><?php __i("Open"); ?></a><?php
		}
		$capt = $this->GetArea($context,"Caption",$area);
		$this->RenderArea($capt);
		?>
		<script language="javascript">jQuery(document).one("ready",function(e){jQuery("#<?php echo $capt->Id() ?>").draggable().construct({id:"<?php echo $context->ItemId(); ?>"});});</script>
		<?php		
	}
	
	protected function renderContainer(TWidgetArea $area){
		$items = $this->Items();
		foreach ($items as $item)
			$this->RenderArea($this->GetArea($item,"Item",$area));
	}
	
/**
 * @see TWidget::DefaultOutput()
 */	
	protected function defaultOutput(IThemeEngine $engine){
?>
<div id="<?php echo $this->Name() ?>" class="listview">
<?php 
$container = $this->GetArea($this,"Container");
$this->RenderArea($this->GetArea($this,"Container"));
if ($this->Source instanceof IItemsCollection){
?>
<script language="javascript">jQuery("#<?php echo $container->Id() ?>").droppable().construct(ondrop:function(data){$.areas_AjaxCall("<?php echo $this->response->Url($this,"AcceptItem"); ?>",data);});</script>
<?php } ?> 
</div>
<?php		
	}
	
	protected function linkCustomResources(){
		self::LoadDraggable();
		parent::linkCustomResources();
	}	
}
?>