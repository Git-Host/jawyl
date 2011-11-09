<?php
/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("controllers::TOrmStructure");

/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 * Nested orm items structure
 * @property TOrmList $Navigation master list object, which selection becomes current opened item of controller, can be set by name  
 * @property TOrmNested $Partner nested structure which opened item is taken as container when moving items, can be set by name 
 */
class TOrmNested extends TOrmStructure implements INestedItems {
	const AN_MOVECHILD = "MoveAsChild";
/**
 * @var TOrmList
 */	
	protected $_ioc_navigation_;
/**
 * @var TOrmNested 
 */	
	protected $_ioc_partner_;
/**
 * @var mixed
 */	
	protected $currentContainer;

/**
 * adjusts current container to navigation list current item 
 */	
	protected function syncNavigation(){
		if ($this->_ioc_navigation_ instanceof TOrmList){
			if ($this->currentContainer != $this->_ioc_navigation_->CurrentItemId()){
				$this->currentContainer = $this->_ioc_navigation_->CurrentItemId();
				$this->saveStateParameter("container",$this->currentContainer);
			}
		}
	}
	
	protected function onSelectEvent(TItemSelectEvent $event){
		if ($event->Sender === $this->Navigation)
			$this->_inner_open_item($event->NodeId);
		else if ($event->Sender === $this->MasterList){
			$this->currentContainer = null;
			$this->saveStateParameter("container",$this->currentContainer);
			parent::onSelectEvent($event);
		}	
	}
	
	protected function onMoveEvent(TItemMoveEvent $event){
		if (($event->Sender === $this->Navigation) || ($event->Sender === $this->Partner)){
			if ($this->currentContainer == $event->ParentId)
				new TItemAddEvent($event->NodeId, $event->ParentId, $this);
			else 
				new TItemRemoveEvent($event->NodeId,$this);	
		}	
	}
	
	protected function onCreateEvent(TItemCreateEvent $event){
		if (($event->Sender === $this->Navigation) || ($event->Sender === $this->Partner))
			if ($event->ParentId == $this->currentContainer)
				$event->ReSend($this);
	}
	
	protected function onAddEvent(TItemAddEvent $event){
		if (($event->Sender === $this->Navigation) || ($event->Sender === $this->Partner))
			if ($event->ParentId == $this->currentContainer)
				$event->ReSend($this);
	}
	
	protected function onDeleteEvent(TItemDeleteEvent $event){
		if (($event->Sender === $this->Navigation) || ($event->Sender === $this->Partner))
				$event->ReSend($this);
	}
	
	protected function onRemoveEvent(TItemRemoveEvent $event){
		if (($event->Sender === $this->Navigation) || ($event->Sender === $this->Partner))
				$event->ReSend($this);
	}

/**
 * @ignore
 */	
	public function __set($nm,$value){
		switch ($nm){
			case "Navigation":{
				parent::__set($nm,$value);
				$this->syncNavigation();
			}break;
			case "Partner":{
				if ($this->_ioc_partner_ != $value){
					parent::__set($nm, $value);
					if ($this->_ioc_partner_ instanceof TOrmNested)
						$this->_ioc_partner_->Partner = $this;
				}	
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "Navigation":{
				$n = parent::__get($nm);
				$this->syncNavigation();
				return $n;
			}break;
			case "Partner":{
				$p = parent::__get($nm);
				if ($p instanceof TOrmNested)
					$p->Partner = $this;
				return $p;
			}break;
			default:return parent::__get($nm);break;
		}
	}
/**
 * constructor
 * @param string $name
 */	
	public function __construct($name, TResponse $response, array $settings = array()){
		parent::__construct($name,$response,$settings);
		$this->currentContainer = $this->loadStateParameter("container");
	}
/**
 * Moving action is added to TOrmStructure actions
 * @see IActionList::Actions()
 * @return array
 */	
	protected function actionsList(){
		$result = parent::actionsList();
		$moveaction = new TControllerAction($this,TOrmNested::AN_MOVECHILD,"MoveAsChild","Move");
		$result[$moveaction->LocalName()] = $moveaction;
		return $result;
	}

/**
 * @see IActionList::ActionAvailable()
 * @return boolean
 */	
	public function ActionAvailable(IAction $action){
		if ($action->LocalName() == "MoveAsChild"){
			if ($this->Partner)
				return !is_null($this->CurrentItem());
			return false;
		}
		return parent::ActionAvailable($action);
	}	
	
/**
 * moves orm item within structure, using controller parent as acceptor
 * @return boolean
 */	
	public function MoveAsChild(){
		$result = false;
		if ($this->Partner instanceof IItemsList){
			if (is_null($this->Partner->CurrentContainer()))
				$result = $this->MoveItem($this->CurrentItemId(),null);
			else
				$result = $this->MoveItem($this->CurrentItemId(),$this->Partner->CurrentContainer()->ItemId());
			$this->SelectItem(null);
		} else 
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		return $result;
	}	
			
	private function _inner_open_item($id){
		$this->currentContainer = $id;
		$this->resetPageInfo();	
		$this->saveStateParameter("container",$this->currentContainer);
		//$this->Invoke(
		new TItemOpenEvent($id,$this);
		//);
		//$this->Invoke(
		new TDataReloadEvent($this);
		//);
		$this->SelectItem(null);
	}
	
/**
 * @see INestedItems::OpenItem()
 * @param mixed $id
 */	
	public function OpenItem($id){
		$this->_inner_open_item($id);
		if ($this->Navigation){
			$this->Navigation->SelectItem($this->currentContainer);
		}
	}
	
/**
 * @see INestedItems::LevelUp()
 */	
	public function LevelUp(){
		if ($this->currentContainer){
			$p = $this->getStructParent($this->CurrentContainer());
			if ($p)
			$this->OpenItem($p->ItemId());
			else
			$this->OpenItem(null);
		}
	}
	
	public function ItemContainers(IIdentity $item){
		return $this->DataSource->ItemContainers($this->Agregation,$item);
	}
	
/**
 * @see INestedItems::CurrentContainer()
 * @return TOrmItem
 */	
	public function CurrentContainer(){
		$result = null;
		if (!is_null($this->currentContainer))
			$result = $this->Item($this->currentContainer);
		return $result;
	}
	
/**
 * gets child items of current opened item
 * @param int $pagesize
 */	
	protected function getSubItems($pagesize){
		$result = $this->DataSource->FetchChildren($this->Agregation,$this->BaseClass,$this->CurrentContainer(),true,$this->getFilter(),$this->getSorting());
		if (!is_null($pagesize)){
			TApplicationSession::Set($this->UniqueId()."_items_count",$result->ItemCount());
			if ($this->PAGE)
				return new TPagedIterator($result,$this->PAGE,pagesize); 
		}
		return $result;
	}
	
/**
 * @see TOrmList::CurrentPage()
 * @param int $pagesize
 * @return IIterator iterator of TOrmItem 
 */		
	public function CurrentPage($pagesize){
		$check = false;
		if ($this->currentContainer)
			$check = $this->CheckPermission($this->CurrentContainer(),TOrmItemPermission::ORM_LIST);
		else  
			$check = $this->CheckPermission($this->DataSource->GetClass($this->BaseClass),TOrmClassPermission::ORM_INSTLIST);
		
		if (!$check){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return null; 
		}
		return $this->getSubItems($pagesize);
	}
	
/**
 * @see TOrmStructure::getCreateParent()
 * @return TOrmItem
 */	
	protected function getCreateParent(){
		return $this->CurrentContainer();
	}	
}

?>