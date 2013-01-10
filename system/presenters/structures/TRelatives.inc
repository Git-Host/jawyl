<?php
/**
 * @package Presenters
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
require_once 'presenters/TMetaReferedList.inc';

/**
 * @package Presenters
 * @subpackage Meta
 * @category System
 * Meta item associated items
 */
class TMetaRelatives extends TMetaReferedList {
/**
 * @var string name of association structure to use
 */	
	public $Association;

	protected function checkDataSource($src){
		return ($src instanceof IAssociationsManager) && parent::CheckDataSource($src);
	}	
	
/**
 * @see IActionList::ActionAvailable()
 * @return boolean
 */	
	public function ActionAvailable(IAction $action){
		if ($action->LocalName() == TMetaReferedList::AN_REMOVE)
			return (!is_null($this->currentItem)) && (!is_null($this->MasterList->CurrentItem()));
		else if ($action->LocalName() == TMetaReferedList::AN_ADD)
			return !is_null($this->MasterList->CurrentItem());
		return parent::ActionAvailable($action);
	}

/**
 * Adds item to associations list (associates an item with presenter master list selected item)
 * @param mixed $id
 * @return boolean
 */	
	public function AddItem($id){
		if (is_null($this->Association)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$item = new TIdentity($id);
		if (!$this->CheckPermission($item,TMetaItemPermission::META_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $this->DataSource->Associate($this->Association,$this->MasterList->CurrentItem(), $item);
		//$this->Invoke(
		new TItemAddEvent($id,null,$this);
		//);
		return $result;
	}

/**
 * Removes current presenter item from associations (deassociates current item from master list selected item)
 * @return boolean 
 */	
	public function RemoveCurrentItem(){
		if (is_null($this->Association)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$item = $this->CurrentItem();
		$id = $this->CurrentItemId();
		if (!$this->CheckPermission($item,TMetaItemPermission::META_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $this->DataSource->DeassociateItems($this->Association,$this->MasterList->CurrentItem(), $item);
		//$this->Invoke(
		new TItemRemoveEvent($id,$this);
		//);
		return $result;
	}
/**
 * @see TMetaList::getItems()
 * @param int $page
 * @param int $pagesize
 * @return IIterator
 */	
	protected function getItems($page = null, $pagesize = null){
		$c = $this->DataSource->GetClass($this->BaseClass);
		if (is_null($this->Association)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		if (!is_null($this->MasterList->CurrentItem()))
			$ss = $this->DataSource->Relatives($this->Association,$this->BaseClass,$this->MasterList->CurrentItem(),$this->getFilter(),$this->getSorting());
		else 
			$ss = new TArrayIterator(array());
		if (!is_null($pagesize)){
			$this->Application()->Session()->Set($this->UniqueId()."_items_count",$ss->ItemCount());
			if ($page)
				return new TPagedIterator($ss,$page,$pagesize);
		}
		return $ss;
	}
/**
 * creates new item and associates it with master list selected item
 * @see TMetaList::ItemCreation()
 * @return TMetaItem
 */	
	protected function itemCreation(IClass $c, array $parameters = array()){
		$result = null;
		if ($result = parent::ItemCreation($c,$parameters)){
			if (is_null($this->Association)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				$this->DataSource->Associate($this->Association,$this->MasterList->CurrentItem(), $result);
		}
		return $result;
	}
}
?>