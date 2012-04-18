<?php
TApplication::Import('common::storage');
TApplication::Import('common::item');

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
			return $this->_type_;
	}
}

class TOrmCondition {
	private $_type_;
	private $_operands_;

	public function __construct(TConditionType $type,array $operands = array()){
		$this->_type_ = $type;
		$this->_operands_ = $operands;
	}
	
	public function __get($nm){
		switch ($nm){
			case 'Type':return $this->_type_;break;
			case 'Operands':return $this->_operands_;break;
		}
	}
}

/**
 * @author dkrasilnikov@gmail.com
 * @property bool $Nullable
 * @property bool $ReadOnly
 * @property int $Type
 */
class TOrmPropertyMeta extends TPropertyMeta {
	public function __set($nm,$value){
		switch ($nm){
			case 'Nullable':$this->nullable = $value;break;
			case 'ReadOnly':$this->readonly = $value;break;
			case 'Type':$this->type = new TItemPropertyType($value);break;
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

class TOrmCollectionPropertyMeta extends TPropertyMeta {
	public function __construct($name,$caption,$readonly = false){
		parent::__construct($name, $caption, TItemPropertyType::PT_COLLECTION, array(),true,$readonly);
	}
} 

interface IORMStorage {
/**
 * Fills an object with missing data
 * @param object $object
 * @return object
 */	
	public function Load($object);
/** 
 * Saves object to storage, returns saved object
 * @param object $object
 * @return object
 */	
	public function Save($object);
	
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
	public function Fetch($dummy,array $greedy = array(),array $conditions = array(),array $sorting = array(),$offset = null, $count = null);
/**
 * @param object|ReflectionClass|string $object
 * @return TOrmPropertyMeta[]
 */
	public function Properties($object);
}

interface IORMHierarchyStorage {
	public function Container($object);
	public function Contents();
	public function FetchContents($dummy,$direct = true, array $greedy = array(),array $conditions = array(),array $sorting = array(),$offset = null, $count = null);
}