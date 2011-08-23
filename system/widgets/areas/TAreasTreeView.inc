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
TApplication::import("widgets::areas::TAreasList");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
 *
 * tree view widget, needs source to be IItemsStructure
 * @property boolean $DeprecateMove indicates whether nodes moving is deprecated
 * @property string $FetchMode
 * @see IItemsStructure
 */
class TAreasTreeView extends TAreasList implements ITree {
	const FETCH_FULL = "full";
	const FETCH_PARTIAL = "partial";
	
	private $_expanded_;
	private $_deprecate_move_ = true;	
	private $_fetch_mode_ = TAreasTreeView::FETCH_PARTIAL;
	
	public function __set($nm,$value){
		switch ($nm){
			case "DeprecateMove":$this->_deprecate_move_ = TConvertions::ConvertToBoolean($value);break;
			case "FetchMode":{
				if (($value == self::FETCH_FULL) || ($value == self::FETCH_PARTIAL))
					$this->_fetch_mode_ = $value;
				else throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			}break;
		}
	}
	
	public function __construct ($name,TResponse $response, array $settings = array(), IConfigurable $container = null){
		parent::__construct($name,$response,$settings,$container);
		$this->_expanded_ = $this->loadStateParameter("expanded");
		if (!is_array($this->_expanded_)) $this->_expanded_ = array();
	}
	
	protected function checkSource($src){
		return ($src instanceof IItemsStructure) && parent::checkSource($src);
	}	

/**
 * moves item within tree view
 * @param string $id item id
 * @param string $parent new parent id
 */	
	public function http_MoveItem($id,$parent){
		$this->Source->MoveItem($this->extractWrappedId($id),$this->extractWrappedId($parent));
	}
	
	public function WrapItem(IItem $item){
		return new TTreeItem($item,$this);
	}	

	public function SubItems(TTreeItem $node){
		$subs = $this->Source->SubItems($node->Item()->ItemId());
		return new TListIteratorAdapter($subs,$this);
	}
/**
 * gets tree view root items
 * @return IIterator 
 */	
	protected function rootItems(){
		$result = array();
		$roots = $this->Source->SubItems();
		return new TListIteratorAdapter($roots,$this);
	}
	
	private function _handle_item_add(TControllerEvent $event){
			$ci = $this->WrapItem($this->Source->Item($event->NodeId));
			if ($event->ParentId)
				$parent = $this->GetArea($this->WrapItem($this->Source->Item($event->ParentId)),"Item");
			else	
				$parent = $this->GetArea($this,"Container");
				
			if ($event->NAME == TControllerEvent::MOVE_EVENT_NAME)
				$this->moveArea($this->GetArea($ci,"Item",$parent));
			else	
				$this->addArea($this->GetArea($ci,"Item",$parent));
	}

	protected function onAddEvent(TItemAddEvent $event){
		$this->_handle_item_add($event);
	}
	
	protected function onMoveEvent(TItemMoveEvent $event){
		$this->_handle_item_add($event);
	}
	
	protected function onCreateEvent(TItemCreateEvent $event){
		$this->_handle_item_add($event);
	} 
	
	protected function onReloadEvent(TDataReloadEvent $event){
			$this->refreshArea($this->GetArea($this,"Container"));
	}
	
	protected function onEditEvent(TItemEditEvent $event){
		if ($event->Sender === $this->Source){
			$ei = $this->WrapItem($this->Source->Item($event->NodeId));
			$this->refreshArea($this->GetArea($ei,"Caption"));
		}
	}
	
	protected function onSelectEvent(TItemSelectEvent $event){
		if (!is_null($event->PreviousNodeId)){
			$ri = $this->WrapItem($this->Source->Item($event->PreviousNodeId));
			$this->refreshArea($this->GetArea($ri,"Caption"));
		}
		if (!is_null($event->NodeId)){
			$ri = $this->WrapItem($this->Source->Item($event->NodeId));
			$this->refreshArea($this->GetArea($ri,"Caption"));
		}
	}
	
	protected function areaStyle(TWidgetArea $area){
		$context = $area->Context();
		switch ($area->Skin()){ 
			case "Subs":{
			if ($this->_is_expanded($context->ItemId()))
				return "display:block;";
			else
				return "display:none;";
			}break;
			case "Caption":return "display:inline;";
			case "Toggle":return "display:inline;";
		}
		return null;
	}
	
	protected function areaCssClass(TWidgetArea $area){
		$context = $area->Context();
		switch ($area->Skin()){
			case "Subs":return "node-subs";break;
			case "Item":return "treenode";break;
			case "Caption":return "caption";break;
		}	
	}
	
	public function http_Toggle($id){
		if (isset($this->_expanded_[$id]))
			unset($this->_expanded_[$id]);	
		else
			$this->_expanded_[$id] = 1;
		$this->saveStateParameter("expanded",$this->_expanded_);
	}
	
	private function _is_expanded($id){
		return isset($this->_expanded_[$id]);
	} 
	
	public function http_Expand($id){
		$this->http_Toggle($id);
		$item = $this->WrapItem($this->Source->Item($id));
		$this->RefreshArea($this->GetArea($item,"Toggle"));
		$this->AddArea($this->GetArea($item,"Subs",$this->GetArea($item,"Item")));
	}
	
	protected function renderCaption(TWidgetArea $area){
		$context = $area->Context();
		?>
		<span class="<?php echo $context->ClassName(); ?><?php echo $context->IsSelected()?" selected":""; ?>" onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $context->SelectLink(); ?>&quot;);">
		<?php echo $context; ?>
		</span>
		<?php
	}
	
	protected function renderSubs(TWidgetArea $area){
		$context = $area->Context();
		$subs = $context->SubNodes();
		while ($subs->Next())
			$this->RenderArea($this->GetArea($subs->Item(),"Item",$area));
	}
	
	protected function renderToggle(TWidgetArea $area){
		$context = $area->Context();
		$expand = (!$this->_is_expanded($context->ItemId())) && ($this->_fetch_mode_ == self::FETCH_PARTIAL); 
		?>
		<span class="exp-button<?php echo (($context->SubNodes()->ItemCount() > 0)?" exp-active":" exp-inactive"); ?>" onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,$expand?"Expand":"Toggle",array("id"=>$context->ItemId())); ?>&quot;);jQuery(&quot;#<?php echo $subs->Id(); ?>&quot;).toggle(&quot;fast&quot;);">
		+
		</span>
		<?php
	}
	
	protected function renderItem(TWidgetArea $area){
		$context = $area->Context();
		$subs = $this->GetArea($context,"Subs",$area);
		$this->RenderArea($this->GetArea($context,"Toggle",$area));
		$capt = $this->GetArea($context,"Caption",$area);
		$this->RenderArea($capt);
		?>
		<script language="javascript">
		jQuery(document).one("ready",function(e){
			jQuery("#<?php echo $capt->Id() ?>").draggable().construct({id:"<?php echo $context->ItemId(); ?>"});
			jQuery("#<?php echo $capt->Id() ?>").droppable({ondrop:function(data){data.parent = "<?php echo $context->ItemId(); ?>";$.areas_AjaxCall("<?php echo $this->response->Url($this,"MoveItem"); ?>",data);}}).construct();
		});
		</script>
		<?php
		$ol = $context->OpenLink();
		if ($ol){ ?>
		<a class="openlink" href="<?php echo $ol;  ?>"><?php __i("open"); ?></a>
		<?php } 
		if (($this->_fetch_mode_ == self::FETCH_FULL) || ($this->_is_expanded($context->ItemId())))
			$this->RenderArea($subs);
	}
	
	protected function renderContainer(TWidgetArea $area){
		$items = $this->RootItems();
		while ($items->Next()) {
			$this->renderArea($this->GetArea($items->Item(),"Item",$area));
		}
	}
	
/**
 * @see TWidget::DefaultOutput()
 */	
	protected function defaultOutput(IThemeEngine $engine){
?>
<div id="<?php echo $this->Name(); ?>" class="areas-treeview">
<?php
	$this->RenderArea($this->GetArea($this,"Container"));
?>
</div>
<?php	
	}
}
?>