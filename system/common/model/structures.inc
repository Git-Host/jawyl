<?php
TApplication::import('common::item');
TApplication::import('common::logic');

class TClassPermission extends TPolicy {
	const INSTANCIATE = "item.instant";
	const INSTLIST = "item.instlist";
	
	protected function checkType($type){
		return ($this->sobj instanceof IClass) && (in_array($type, array(self::INSTANCIATE,self::INSTLIST)));
	}
	
	public function Descendants(){
		switch ($this->type){
			case self::INSTANCIATE:
			case self::INSTLIST:return array(new TServiceRole(TServiceRole::USER));break;
		}
		return array();
	}	
}

class TItemRole extends TPolicy {
	const OWNER = "item.owner";
	
	protected function checkType($type){
		return ($this->sobj instanceof TMetaItem) && ($type == self::OWNER);
	}
	
	public function Descendants(){
		switch ($this->type){
			case self::OWNER:return array(new TServiceRole(TServiceRole::ADMIN));break;
		}
		return array();
	}
/**
 * checks whether specified policy is role  
 * @param string $type
 * @return boolean
 */
	public static function IsRole($policy){
		return $policy == self::OWNER;
	}
}

class TItemPermission extends TPolicy {
	const DELETE = "item.delete";
	const RELATE = "item.relate";
	const LST = "item.list";
	const VIEW = "item.view";
	const MODIFY = "item.modify";
	const ACL = "item.acl";

	protected function checkType($type){
		return ($this->sobj instanceof IItem) && (in_array($type, array(self::ACL,self::DELETE,self::LST,self::MODIFY,self::RELATE,self::VIEW)));
	}
	
	public function Descendants(){
		switch ($this->type){
			case self::ACL:
			case self::DELETE:
			case self::MODIFY:return array(new TServiceRole(TServiceRole::ADMIN),new TMetaItemRole(TMetaItemRole::OWNER,$this->sobj));break;
			case self::RELATE:return array(new TServiceRole(TServiceRole::USER),new TMetaItemRole(TMetaItemRole::OWNER,$this->sobj));break;
			case self::VIEW:
			case self::LST:return array(new TServiceRole(TServiceRole::ANONYMOUS),new TMetaItemRole(TMetaItemRole::OWNER,$this->sobj));break;
		}
		return array();
	}	
}

class TPropertyPermission extends TPolicy {
	const READ = "item.read";
	const WRITE = "item.write";
	
	protected function checkType($type){
		return ($this->sobj instanceof IProperty) && (in_array($type, array(self::READ,self::WRITE)));
	}
	
	public function Descendants(){
		$result = array(new TServiceRole(TServiceRole::USER));
		$item = $this->sobj->Item();
		switch ($this->type){
			case self::READ:$result[] = new TMetaItemPermission(TMetaItemPermission::VIEW,$item);break;
			case self::WRITE:$result[] = new TMetaItemPermission(TMetaItemPermission::MODIFY,$item);break;
		}
		return $result;
	}
}

/**
 * @property IPropertyMeta $Property
 * @property mixed $Value
 * @property int $Operation
 */
class TItemFilter {
	private $_property_ = null;
	private $_value_ = null;
	private $_oper_ = TConditionType::C_EQUAL;
	
	public function __construct(IPropertyMeta $property, $value, $oper = TConditionType::C_EQUAL){
		$this->_property_ = $property;
		$this->_value_ = $value;
		$this->_oper_ = new TConditionType($oper);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Property":return $this->_property_;break;
			case "Value":return $this->_value_;break; 
			case "Operation":return $this->_oper_;break;
		}
	}
}

/**
 * @property IPropertyMeta $Property
 * @property int $SortType
 */
class TItemSorting {
	private $_property_ = null;
	private $_sort_type_ = TSorting::SORT_ASC;
	
	public function __construct(IPropertyMeta $property, $sorttype = TSorting::SORT_ASC){
		$this->_property_ = $property;
		$this->_sort_type_ = TSortingType($sorttype);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Property":return $this->_property_;break;
			case "SortType":return $this->_sort_type_;break; 
		}		
	}
}

interface IItemsManager {
/**
 * gets class object by class name
 * @param string $className
 * @return IClass
 */	
	public function GetClass($className);
/**
 * creates item
 * @param IClass $c class of new item
 * @param array $parameters associative array of attribute values, where keys are property names
 * @return IItem
 */	
	public function CreateItem(IClass $c, array $parameters);
/**
 * gets item by its id or code
 * @param mixed|IIdentity $identity item id
 * @return IItem 
 */	
	public function GetItem($identity);
/**
 * edits item
 * @param mixed $id item id
 * @param array $values associative array of new attribute values, keys are treated as attribute names
 * @return ШItem returns edited item 
 */	
	public function EditItem(IIdentity $item,array $values);	
/**
 * deleted item
 * @param mixed $id item id
 * @return boolean 
 */	
	public function DeleteItem(IIdentity $item);
/**
 * gets class items
 * @param IClass $class class
 * @param TItemFilter[] $filter array of TCondition filtering conditions 
 * @param TItemSorting[] $sorting array of TSorting results sorting
 * @return IIterator iterator of IItem
 */	
	public function GetItems(IClass $class, array $filters, array $sorting);	
}

interface ICollectionsManager {
/**
 * 
 * Adds item to collection
 * @param string $collection
 * @param IIdentity $item
 * @return boolean
 */	
	public function AddToCollection($collection, IIdentity $item);
/**
 * Deletes item from collection
 * @param string $collection
 * @param IIdentity $item
 * @return boolean
 */	
	public function DeleteFromCollection($collection, IIdentity $item);
/**
 * 
 * Gets collection items
 * @param string $collection
 * @param IClass $class
 * @param TItemFilter[] $filter
 * @param TItemSorting[] $sorting
 * @return IIterator
 */	
	public function CollectionItems($collection, IClass $class, array $filter = array(), array $sorting = array());
}

interface IVocabulariesManager {
/**
 * Adds an association of item to specified key to vocabulary
 * @param string $vocabulary
 * @param mixed $key
 * @param IIdentity $item
 * @return boolean
 */	
	public function AddToVocabulary($vocabulary, $key, IIdentity $item);
/** 
 * Removes item association from vocabulary
 * @param string $vocabulary
 * @param mixed $key
 * @param IIdentity $item
 * @return boolean
 */	
	public function DeleteFromVocabulary($vocabulary, $key, IIdentity $item);
/** 
 * Fetches items associated with a specified key from a vocabulary
 * @param string $vocabulary
 * @param IClass $class
 * @param mixed $key
 * @param TItemFilter[] $filter
 * @param TItemSorting[] $sorting
 * @return IIterator
 */	
	public function VocabularyItemsByKey($vocabulary, IClass $class, $key, array $filter = array(), array $sorting = array());
}

interface IAssociationsManager {
	const FETCH_OBJECTS = 0;
	const FETCH_SUBJECTS = 1;
	
/** 
 * Association one item to another
 * @param string $association
 * @param IIdentity $subject
 * @param IIdentity $object
 * @return boolean
 */	
	public function Associate($association, IIdentity $subject, IIdentity $object);
/** 
 * Deassociates one item from another
 * @param string $association
 * @param IIdentity $subject
 * @param IIdentity $object
 * @return boolean
 */	
	public function DeassociateItems($association, IIdentity $subject, IIdentity $object);
/** 
 * Fetches item relatives
 * @param string $association
 * @param IClass $class
 * @param IIdentity $item
 * @param TItemFilter[] $filter
 * @param TItemSorting[] $sorting
 * @param int $direction relations direction set by FETCH_OBJECTS or FETCH_SUBJECTS constant
 * @return IIterator
 */	
	public function Relatives($association, IClass $class, IIdentity $item, array $filter = array(), array $sorting = array(), $direction = IAssociationsManager::FETCH_OBJECTS);
}

interface IStructuresManager {
/** 
 * Moves item within or out of structure
 * @param string $structure
 * @param IIdentity $item
 * @param IIdentity $parent
 * @return boolean
 */	
	public function MoveItem($structure, IIdentity $item, IIdentity $parent = null);
/** 
 * Fetches item children
 * @param string $structure
 * @param IClass $class
 * @param IIdentity $parent
 * @param boolean $only_direct
 * @param TItemFilter[] $filter
 * @param TItemSorting[] $sorting
 * @return IIterator
 */
	public function FetchChildren($structure, IClass $class, IIdentity $parent = null, $only_direct = false, array $filter = array(), array $sorting = array());
/** 
 * Gets item direct container
 * @param string $structure
 * @param IIdentity $item
 * @return IItem
 */	
	public function ItemContainer($structure, IIdentity $item);
/**
 * 
 * Gets item containers list
 * @param string $structure
 * @param IIdentity $item
 * @return IIterator
 */	
	public function ItemContainers($structure, IIdentity $item);
}

