<?php
/**
 * @package Controllers
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
TApplication::import("common::controller::TSecuredController");

/**
 * @package Controllers
 * @category System
 * abstract class for holding and managing lists.
 * @property IItemsList $MasterList controller master list. You can set it assigning list name or list itself. Selecting item in masterlist forces controller to reload. 
 * 
 * @see TSecuredController
 * @see IItemsList
 */
abstract class TItemsList extends TSecuredController implements IItemsList, IItemAccessor {
/**
 * @var mixed
 * holds current item id
 */	
	protected $currentItem = null;
/**
 * @var IItemsList
 * holds controller master list object.
 */	
	private $_ioc_master_list_;

	protected function onRefresh(TEvent $event){
		$this->innerSelect(null);
		//$this->Invoke(
		new TDataReloadEvent($this);
		//);
	}
	
	protected function onReloadEvent(TDataReloadEvent $event){
		if ($event->Sender === $this->_ioc_master_list_)
			$this->onRefresh($event);
	}
	
	protected function onSelectEvent(TItemSelectEvent $event){
		if ($event->Sender === $this->_ioc_master_list_)
			$this->onRefresh($event);
	}
	
/**
 * constructor. Sets current item if saved in controller state.
 */	
	public function __construct($name, TResponse $response, array $settings = array()){
		parent::__construct($name,$response, $settings);
		$this->currentItem = $this->loadStateParameter("selected");
	}	

/**
 * gets current item id. If no item selected as current returns null.
 */	
	protected function currentItemId(){
		return $this->currentItem;
	}

/**
 * @ignore
 */	
	private function _inner_select($id){
		$this->currentItem = $id;
		$this->saveStateParameter("selected",$this->currentItem);
	}

/**
 * selects item as current. Invokes TItemSelectEvent if item is successfully selected.
 * @param mixed $id item id
 */	
	public function SelectItem($id){
		$previd = $this->currentItemId();
		$this->_inner_select($id);
		//$this->Invoke(
		new TItemSelectEvent($this->currentItemId(),$previd,$this);
		//);
	}

/**
 * gets current list item.
 * @return IItem
 */	
	public function CurrentItem(){
		$result = null;
		if (!is_null($this->currentItemId()))	
			$result = $this->Item($this->currentItemId());
		return $result;
	}
}
?>