<?php
TApplication::Import('common::storage');

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
} 