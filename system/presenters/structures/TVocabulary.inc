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
TApplication::import("presenters::TMetaReferedList");

/**
 * @package Presenters
 * @subpackage Meta
 * @category System
 * 
 * Meta vocabulary items
 */
class TMetaVocabulary extends TMetaReferedList {
/**
 * @var string name of vocabulary structure to use
 */		
	public $Vocabulary;
/**
 * @var string key to use for filtering
 */	
	public $Key;
	
	protected function checkDataSource($src){
		return ($src instanceof IVocabulariesManager) && parent::CheckDataSource($src);
	}	

/**
 * @see IActionList::ActionAvailable()
 * @return boolean
 */	
	public function ActionAvailable(IAction $action){
		if ($action->LocalName() == TMetaReferedList::AN_REMOVE)
			return !is_null($this->currentItem);
		return parent::ActionAvailable($action);
	}
	
/**
 * Adds item to vocabulary using presenter key
 * @param mixed $id
 * @return boolean
 */	
	public function AddItem($id){
		if (is_null($this->Vocabulary)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$item = new TIdentity($id);
		if (!$this->CheckPermission($item,TMetaItemPermission::META_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $this->DataSource->AddToVocabulary($this->Vocabulary,$this->Key,$item);
		//$this->Invoke(
		new TDataReloadEvent($this);
		//);
		return $result;
	}

/**
 * deletes current item vocabulary entry
 * @return boolean
 */	
	public function RemoveCurrentItem(){
		if (is_null($this->Vocabulary)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$item = $this->CurrentItem();
		if (!$this->CheckPermission($item,TMetaItemPermission::META_RELATE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $this->DataSource->DeleteFromVocabulary($this->Vocabulary,$this->Key,$item);
		//$this->Invoke(
		new TDataReloadEvent($this);
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
		if (is_null($this->Vocabulary)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$ss = $this->DataSource->VocabularyItemsByKey($this->Vocabulary,$c,$this->Key,$this->getFilter(), $this->getSorting());
		if (!is_null($pagesize)){
			TApplicationSession::Set($this->UniqueId()."_items_count",$ss->ItemCount());
			if ($page)
				return new TPagedIterator($ss,$page,$pagesize);
		}
		return $ss;
	}
	
/**
 * creates new item and assigns it a key in vocabulary
 * @see TMetaList::ItemCreation()
 * @return TMetaItem
 */	
	protected function itemCreation(IClass $c, array $parameters = array()){
		$result = null;
		if ($result = parent::itemCreation($c,$parameters)){
			if (is_null($this->Vocabulary)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				$this->DataSource->AddToVocabulary($this->Vocabulary,$$this->Key,$result);
		}
		return $result;
	}
}
?>