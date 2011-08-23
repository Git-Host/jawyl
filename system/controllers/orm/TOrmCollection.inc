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
TApplication::import("controllers::TOrmReferedList");

/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 * items in orm collection 
 */
class TOrmCollection extends TOrmReferedList {
/**
 * @var string orm collection name to use
 */	
	public $Collection;
	
	protected function checkDataSource($src){
		return ($src instanceof ICollectionsManager) && parent::CheckDataSource($src);
	}	

/**
 * @see IActionList::ActionAvailable()
 * @return boolean
 */	
	public function ActionAvailable(IAction $action){
		if ($action->LocalName() == TOrmReferedList::AN_REMOVE)
			return !is_null($this->currentItem);
		return parent::ActionAvailable($action);
	}
	
/**
 * Adds orm item to collection
 * @param mixed $id item id
 * @return boolean  
 */	
	public function AddItem($id){
		if (is_null($this->Collection)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$item = new TIdentity($id);
		if (!$this->CheckPermission($item,TOrmItemPermission::ORM_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $this->DataSource->AddToCollection($this->Collection,$item);
		//$this->Invoke(
		new TItemAddEvent($id,null,$this);
		//);
		return $result;
	}

/**
 * Removes controller current item from collection
 * @return boolean
 */	
	public function RemoveCurrentItem(){
		if (is_null($this->Collection)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$item = $this->CurrentItem();
		$id = $this->CurrentItemId();
		if (!$this->CheckPermission($item,TOrmItemPermission::ORM_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $this->DataSource->DeleteFromCollection($this->Collection,$item);
		//$this->Invoke(
		new TItemRemoveEvent($id,$this);
		//);
		return $result;
	}
	
/**
 * @see TOrmList::getItems()
 * @param int $page
 * @param int $pagesize
 * @return IIterator
 */	
	protected function getItems($page = null, $pagesize = null){
		$result = array();
		$c = $this->DataSource->GetClass($this->BaseClass);
		if (is_null($this->Collection)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$ss = $this->DataSource->CollectionItems($this->Collection,$c,$this->getFilter(),$this->getSorting());
		if (!is_null($pagesize)){
			TApplicationSession::Set($this->UniqueId()."_items_count",$ss->ItemCount());
			if ($page)
				return new TPagedIterator($ss,$page,$pagesize);
		}
		return $ss;
	}
	
/**
 * creates new item and adds it to collection
 * @see TOrmList::ItemCreation()
 * @return TOrmItem
 */	
	protected function itemCreation(IClass $c, array $parameters = array()){
		$result = null;
		if ($result = parent::itemCreation($c,$parameters)){
			if (is_null($this->Collection)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			$this->DataSource->AddToCollection($this->Collection, $result);
		}
		return $result;
	}
}
?>