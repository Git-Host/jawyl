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
TApplication::import("controllers::TOrmCollection");

/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 */
class TLevelIterator extends TArrayIterator {
	public function Item(){
		$item = parent::Item();
		return $item["item"];
	} 
}

/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 * flat orm structure items
 */
class TOrmStructure extends TOrmList implements IItemsStructure {
/**
 * @var array
 */	
	protected $structures = null;
/*
 * @var string orm agregation name to use
 */	
	public $Agregation;
					
/**
 * gets container for newly created items
 * @return TOrmItem
 */	
	protected function getCreateParent(){
		return $this->CurrentItem();
	}	

/**
 * @see TOrmList::getItems()
 * @param int $page
 * @param int $pagesize
 * @return TOrmItem
 */	
	protected function getItems($page = null, $pagesize = null){
		$result = $this->DataSource->FetchChildren($this->Agregation,$this->BaseClass,null,false,$this->getFilter(),$this->getSorting());
		if (!is_null($pagesize)){
			TApplicationSession::Set($this->UniqueId()."_items_count",$result->ItemCount());
			if ($this->PAGE)
				return new TPagedIterator($result,$this->PAGE,pagesize); 
		}
		return $result;
	}
	
	protected function checkDataSource($src){
		return ($src instanceof IStructuresManager) && parent::CheckDataSource($src);
	}	

/**
 * @see IItemsStructure::ItemParent()
 * @param mixed $id
 * @return TOrmItem
 */	
	public function ItemParent($id){
		if (is_null($this->STRUCTURE))
			return $this->getStructParent($this->Item($id));
		else
			return $this->STRUCTURE["index"][$id]["parent"]["item"];
	} 
	
/**
 * @see TOrmList::DeleteItem()
 * @param mixed $id
 * @return boolean
 */	
	protected function deleteItem($id){
		$item = $this->Item($id);
		if ($this->itemMovement($item)){
			return parent::deleteItem($id);
		}
		return false;
	}	
	
/**
 * @see IItemsStructure::MoveItem()
 * @param mixed $id
 * @param mixed $parent
 * @return boolean
 * @uses TOrmStructure::ItemMovement()
 */	
	public function MoveItem($id,$parent){
		$item = new TIdentity($id);
		if (!$this->CheckPermission($item,TOrmItemPermission::ORM_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return false;
		}
		if (!is_null($parent)){
			$p = new TIdentity($parent);
			if (!$this->CheckPermission($p,TOrmItemPermission::ORM_RELATE)){
				throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
				return false;
			}
		}
		
		if ($this->ItemMovement($item,$p))
			new TItemMoveEvent($item->ItemId(),$parent,$this);
			//$this->Invoke($event);//
	}
	
/**
 * @see TOrmList::CreateItem()
 * @return TOrmItem
 */	
	public function CreateItem(TCreateInfo $createinfo = null){
		$result = null;
		if ($result = $this->Create($createinfo)){
			$p = $this->getCreateParent();
			if (!is_null($p)) $p = $p->ItemId();
			//$this->Invoke(
			new TItemCreateEvent($result->ItemId(),$p,$this);
			//);
			$this->SelectItem($result->ItemId());
		}
		return $result;
	}	
	
/**
 * performs orm item movement within a structure, can be overrden in descenders
 * @return boolean
 */	
	protected function itemMovement(IIdentity $item, IIdentity $parent = null){
		if (is_null($this->Agregation)){
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			return false;
		}
		if (is_null($parent))
			$this->DataSource->MoveItem($this->Agregation,$item);
		return $this->DataSource->MoveItem($this->Agregation,$item,$parent);	
	}

/**
 * gets child items by parent
 * @param mixed $parent parent item id
 * @param int $page optional page number (meaningful only when $pagesize is also set)
 * @param int $pagesize optional page size
 * @return IIterator iterator of TOrmItem
 */	
	public function SubItems($parent, $page = null, $pagesize = null){
		$result = $this->DataSource->FetchChildren($this->Agregation,$this->BaseClass,new TIdentity($parent),true,$this->getFilter(),$this->getSorting());
		if (!is_null($pagesize)){
			TApplicationSession::Set($this->UniqueId()."_items_count",$result->ItemCount());
			if ($this->PAGE)
				return new TPagedIterator($result,$this->PAGE,pagesize); 
		}
		return $result;
	}

/**
 * creates new item and puts it into current container
 * @see TOrmList::ItemCreation()
 * @return TOrmItem
 */	
	protected function itemCreation(IClass $c, array $parameters = array()){
		$result = parent::itemCreation($c,$parameters);
		if (!is_null($result)){
			$p = $this->getCreateParent();
			if (!is_null($p))
				$this->itemMovement($result,$p);
		}
		return $result;
	}
		
/**
 * gets item container
 * @return TOrmItem
 */	
	protected function getStructParent(Identity $item){
		if (is_null($this->Agregation)){
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			return false;
		}
		return $this->DataSource->ItemContainer($this->Agregation,$item);
	}
}
?>