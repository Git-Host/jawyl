<?php
/**
 * @package Controllers
 * @subpackage Meta
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("controllers::TMetaReferedList");

/**
 * @package Controllers
 * @subpackage Meta
 * @category System
 * Meta item associated items
 */
class TMetaDetailsList extends TMetaReferedList {
/**
 * @var string name of attribute to use for detail join to master
 */	
	public $ReferenceAttribute;

/**
 * @see IActionList::ActionAvailable()
 * @return boolean
 */	
	public function ActionAvailable(IAction $action){
		switch ($action->LocalName()){
			case TMetaReferedList::AN_REMOVE:return ($this->ReferenceAttribute) && (!is_null($this->currentItem)) && (!is_null($this->MasterList->CurrentItem()));break;
			case TMetaReferedList::AN_ADD:return !is_null($this->MasterList->CurrentItem());break;
			case TMetaList::AN_CREATE:return parent::ActionAvailable($action) && (!is_null($this->MasterList->CurrentItem()));break;
			default:return parent::ActionAvailable($action);
		}
	}
	
/**
 * Adds item to associations list (associates an item with controller master list selected item)
 * @param mixed $id
 * @return boolean
 */	
	public function AddItem($id){
		if (!($this->MasterList instanceof IItemsList)){ 
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
			return false;
		}
		$mi = $this->MasterList->CurrentItem();
		if (!($mi instanceof IItem)){
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			return false;
		}
		//$item = $this->DataSource->GetItem($id);
		$item = new TIdentity($id);
		if (!$this->CheckPermission($item,TMetaItemPermission::META_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$item->__set($this->ReferenceAttribute,$mi);
		$item->Save();
		//$this->Invoke(
		new TItemAddEvent($id,null,$this);
		//);
		return $result;
	}

/**
 * Removes current controller item from associations (deassociates current item from master list selected item)
 * @return boolean 
 */	
	public function RemoveCurrentItem(){
		$item = $this->CurrentItem();
		$id = $this->CurrentItemId();
		if (!$this->CheckPermission($item,TMetaItemPermission::META_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$item->__set($this->ReferenceAttribute,null);
		$item->Save();
		//$this->Invoke(
		new TItemRemoveEvent($id,$this);
		//);
		return $result;
	}
	
	protected function getFilter(){
		$result = parent::getFilter();
		if ($this->ReferenceAttribute){
			if (!($this->MasterList instanceof IItemsList)){ 
				throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
				return false;
			}
			$mi = $this->MasterList->CurrentItem();
			if (!($mi instanceof IItem)){
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				return false;
			}
			$c = $this->DataSource->GetClass($this->BaseClass);
			$result[] = array(TCondition::C_EQUAL,array($c->GetAttributeDefinition($this->ReferenceAttribute),$mi));
		}
		return $result;
	}
/**
 * @see TMetaList::getItems()
 * @param int $page
 * @param int $pagesize
 * @return IIterator
 */	
	protected function getItems($page = null, $pagesize = null){
		if ($this->ReferenceAttribute)
			if (is_null($this->MasterList->CurrentItem()))
				return new TArrayIterator(array());
		return parent::getItems($page,$pagesize);
	}
/**
 * creates new item and associates it with master list selected item
 * @see TMetaList::ItemCreation()
 * @return TMetaItem
 */	
	protected function itemCreation(IClass $c, array $parameters = array()){
		if ($this->ReferenceAttribute){
			if (!($this->MasterList instanceof IItemsList)){ 
				throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
				return false;
			}
			$mi = $this->MasterList->CurrentItem();
			if (!($mi instanceof IItem)){
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				return false;
			}
			$result = null;
			$parameters[$this->ReferenceAttribute] = $mi;
		}  
		$result = parent::itemCreation($c,$parameters);
		return $result;
	}
}
?>