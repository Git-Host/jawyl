<?php
require_once 'common/storage.inc';
require_once 'common/item.inc';

/**
 * @author dkrasilnikov@gmail.com
 * @property int $Type
 */
class TOrmSorting {
/**
 * @var string
 */	
	public $Attribute;
	
	private $_type_ = TSortingType::SORT_ASC;
	
/**
 * @param string $attr
 * @param int $type
 */	
	public function __construct($attr,TSortingType $type = null){
		$this->Attribute = $attr;
		if (!is_null($type))
			$this->_type_ = $type;
	}
	
	public function __get($nm){
		if ($nm == 'Type')
			return (int)$this->_type_->__toString();
	}
}

/**
 * @author dkrasilnikov@gmail.com
 * @property int $Type
 * @property array $Operands
 */
class TOrmCondition {
	private $_type_;
	private $_operands_;

	public function __construct(TConditionType $type,array $operands = array()){
		$this->_type_ = $type;
		$this->_operands_ = $operands;
	}
	
	public function __get($nm){
		switch ($nm){
			case 'Type':return (int)$this->_type_->__toString();break;
			case 'Operands':return $this->_operands_;break;
		}
	}
}

/**
 * @author dkrasilnikov@gmail.com
 * @property bool $Nullable
 * @property bool $ReadOnly
 * @property bool $Required
 * @property int $Type
 */
class TOrmPropertyMeta extends TPropertyMeta {
	public function __set($nm,$value){
		switch ($nm){
			case 'Nullable':$this->nullable = $value;break;
			case 'ReadOnly':$this->readonly = $value;break;
			case 'Type':$this->type = new TItemPropertyType($value);break;
			case 'Required':$this->required = $value;break;
		}
	}
}

class TOrmReferencePropertyMeta extends TOrmPropertyMeta implements IReferencePropertyMeta {
	protected $className;
	
	public function __construct($name,$caption,$classname,$nullable = true,$readonly = false){
		parent::__construct($name, $caption, TItemPropertyType::PT_REFERENCE, array(),$nullable,$readonly);
		$this->className = $classname;
	}
	
	public function ClassName(){
		return $this->className;
	}
}

class TOrmCollectionPropertyMeta extends TOrmReferencePropertyMeta {
	public function __construct($name,$caption,$classname,$readonly = false){
		parent::__construct($name, $caption, $classname,true,$readonly);
		$this->Type = TItemPropertyType::PT_COLLECTION;
	}
} 

interface IORMStorage {
/**
 * Fills an object with missing data
 * @param object $object
 * @return object
 */	
	public function Load($object, array $assume_descendants = array());
/** 
 * Saves object to storage, returns saved object
 * @param object $object
 * @return object
 */	
	public function Save($object, array $propeties_mask = array());
	
/** 
 * Deleted object from storage, returns true on success
 * @param object $object
 * @return boolean
 */	
	public function Delete($object);
	
/**
 * Fetches a collection of objects from storage
 * @param object|ReflectionClass|string $dummy
 * @param string[] $greedy
 * @param TOrmCondition[] $conditions
 * @param TOrmSorting[] $sorting
 */	
	public function Fetch($dummy,array $greedy = array(),array $conditions = array(),array $sorting = array(), array $assume_descendants = array(), $offset = null, $count = null, &$total = null);
/**
 * @param object|ReflectionClass|string $object
 * @return TOrmPropertyMeta[]
 */
	public function Properties($object);
	
/**
 * @param object|ReflectionClass|string $object
 * @return TOrmPropertyMeta[]
 */	
	public function Keys($object);
	
	
/**
 * @param object|ReflectionClass|string $object
 * @return string[]
 */	
	public function Tables($object);
}

interface IORMHierarchyStorage {
	public function Put($object,$container);
	public function Container($object);
	public function Contents($object);
	public function FetchContents($dummy,$direct = true, array $greedy = array(),array $conditions = array(),array $sorting = array(), array $assume_descendants = array(), $offset = null, $count = null, &$total = null);
	public function ParentProperty($object);
	public function ChildrenProperty($object);
}