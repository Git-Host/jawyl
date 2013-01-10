<?php
require_once 'common/metastorage.inc';

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 * mysql exception
 */
class TMySqlException extends TStorageException {
	const ERR_MYSQL_DBFAIL = 1001;
	const ERR_MYSQL_CONNECTFAIL = 1002;
	
/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_MYSQL_CONNECTFAIL:return "Cannot connect to server %server!";break;
			case self::ERR_MYSQL_DBFAIL:return "Cannot select database %db on server %server!";break;
			default:{
				$msgtext = parent::getMessageText($msgcode);
				if ($msgtext != "") return $msgtext;
				return "%msgtext\nQuery:\n%query";
			}break;
		}
	}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 * 
 * iterator through MySql result set
 * @see IIterator
 */
class TMySqlDBRawDataIterator extends TIterator {
	protected $resultSet;
	protected $currentRow;
	protected $fields = array();
	protected $pointer;
/**
 * @var TMySqlDBDriver
 */	
	protected $driver;
	
	public function __construct($resultset, TMySqlDBDriver $driver){
		$this->resultSet = $resultset;
		$this->driver = $driver;
		$fn = mysql_num_fields($this->resultSet);
		for ($i = 0; $i < $fn; $i++){
			$nm = mysql_field_name($this->resultSet,$i);
			if (in_array($nm,$this->fields)) $nm .= $i;
			$this->fields[] = $nm;
		}
		$this->pointer = -1;
	}
	
	public function Next(){
		$this->currentRow = null;
		if ($row = mysql_fetch_array($this->resultSet,MYSQL_NUM)){
			$this->currentRow = array_merge($row,array_combine($this->fields,$row));
			$this->pointer++;
			return true;
		}
		return false;
	}	
	
	public function Seek($position){
		if ($position == 0 && mysql_num_rows($this->resultSet) == 0)
			return true;
		if (mysql_data_seek($this->resultSet, $position)){
			if ($row = mysql_fetch_array($this->resultSet,MYSQL_NUM)){
				$this->currentRow = array_merge($row,array_combine($this->fields,$row));
				$this->pointer = $position;
				return true;
			}
		}
		return false;
	}
	
	public function Rewind() {
		if ($this->position == -1)
			return $this->Next();
		return $this->Seek(0); 	
    }

    public function Key() {
        return $this->pointer;
    }
	
	public function Item(){
		return $this->currentRow;
	}
		
	public function ItemCount(){
		return mysql_num_rows($this->resultSet);
	}
	public function DataType(){return IIterator::DT_RAW;}
}

class TMySqlNestedSetElement extends TNestedSetElement {
	
}

class TMySqlNestingsIterator extends TIteratorAdapter {
	private $_item_;
	private $_current_parent_;
	private $_parents_stack_ = array();
	private $_prev_item_ = null;
	
	public function __construct(IIterator $base){
		parent::__construct($base);
	}	
	
	public function Next(){
		$this->_prev_item_ = $this->_item_; 
		unset($this->_item_);
		return parent::Next(); 
	}

	public function Seek($count){
		$this->Next();
	}
	
	public function Item(){
		if (!isset($this->_item_)){
			$row = $this->iterator->Item();
			
			$lkey = TMySqlDBDriver::NS_LKEY_FIELD;
			$rkey = TMySqlDBDriver::NS_RKEY_FIELD;
			
			if (!empty($this->_prev_item_)){
				if ($this->_prev_item_->$rkey > $row[$rkey])
					array_push($this->_parents_stack_, $this->_prev_item_);
			}
			
			$hindex = count($this->_parents_stack_) - 1;
			$parent = null;
			if (!empty($this->_parents_stack_))
				$parent = $this->_parents_stack_[$hindex];
				
			while ($parent && $row[$lkey] > $parent->$rkey) {
				array_pop($this->_parents_stack_);
				$hindex--;
				$parent = null;
				if (!empty($this->_parents_stack_))
					$parent = $this->_parents_stack_[$hindex];
			} 
			
			$this->_item_ = new TMySqlNestedSetElement($row,$parent);
		}
		return $this->_item_;
	}

	public function DataType(){return IIterator::DT_RAW;}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 * stored meta class
 * @property TAttributeDefinition[] $AttributeDefinitions
 */
class TMySqlDBClass extends  TDBDefinedClass {
	private static $_loaded_classes_ = array();
	private static $_classes_by_id_ = array();
/**
 * constructor
 * @param int $id
 * @param string $name
 */	
	public function __construct($id,$name,TMySqlMetaDriver $driver, TDBDefinedClass $ancestor = null,$attributes = null){
		if (!key_exists($driver->Name(),self::$_loaded_classes_))
			self::$_loaded_classes_[$driver->Name()] = array();
		if (!key_exists($driver->Name(),self::$_classes_by_id_))
			self::$_classes_by_id_[$driver->Name()] = array();
		self::$_loaded_classes_[$driver->Name()][$name] = $this;
		self::$_classes_by_id_[$driver->Name()][$id] = $this;
		parent::__construct($id,$name,$driver,$ancestor,$attributes);
	}
	
	public static function Create($id,$name,TMySqlMetaDriver $driver,TDBDefinedClass $ancestor = null, $attributes = null){
		if ($c = self::Obtain($id, $driver))
			return $c;
		if ($c = self::Obtain($name, $driver))
			return $c;
		return new TMySqlDBClass($id,$name,$driver,$ancestor,$attributes);
	}  
	
	public static function Obtain($id,TMySqlMetaDriver $driver){
		if (is_numeric($id)){
			if (key_exists($driver->Name(),self::$_classes_by_id_))
				if (key_exists($name,self::$_classes_by_id_[$driver->Name()]))
					return self::$_classes_by_id_[$driver->Name()][$id];
		}
		else	
		if (key_exists($driver->Name(),self::$_loaded_classes_))
			if (key_exists($name,self::$_loaded_classes_[$driver->Name()]))
				return self::$_loaded_classes_[$driver->Name()][$name];
		return null;		
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'AttributeDefinitions':{
				if ($value instanceof TAttributeDefinition)
					$this->addAttributeDefinition($value);
			}break;
			default:parent::__set($nm, $value);break;
		}
	}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 * stored instance
 */
class TMySqlDBInstance extends TDBInstance {
/**
 * constructor
 * @param int $id
 */	
	public function __construct($id,TMySqlDBClass $class){
		parent::__construct($id,$class);
	}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 * 
 * converts dataset row to object
 */
class TMySqlRowConverter {
/**
 * converts row to class
 * @return IClass
 */	
	public static function ToClass(array $row, TMySqlMetaDriver $driver){
		if (key_exists(TMySqlDBDriver::CLASS_NAME_FIELD,$row))
			return  $driver->GetClass($row[TMySqlDBDriver::CLASS_NAME_FIELD]);
		else if (key_exists(TMySqlDBDriver::CLASS_PK_FIELD,$row))
			return  $driver->GetClass($row[TMySqlDBDriver::CLASS_PK_FIELD]);	
		return null;
	}
	
	protected static function getDataType($options, $typecode = null){
		$result = array();
		if (is_null($typecode)){
			preg_match('/type=([^;]*)/', $options,$result);
			$typecode = $result[1];
		}
		switch ($typecode){
			case TTableReferenceType::REFERENCE:{
				preg_match('/masterfield=(\w*)\.(\w*)/', $options,$result);
				$field = new TDBTableField(new TDBTable($result[1]), $result[2]);
				preg_match('/datatype=([^;]*)/', $options,$result);
				$reftype = $this->getDataType(base64_decode($result[1]));
				preg_match('/ondelete=([^;]*)/', $options,$result);
				$ondelete = $result[1];
				preg_match('/onupdate=([^;]*)/', $options,$result);
				$onupdate = $result[1];
				return new TTableReferenceType($field, $reftype, $ondelete, $onupdate);
			}break;
			case TMetaReferenceType::META_REFERENCE:{
				$refclass = null;
				preg_match('/refclass=([^;]*)/', $options,$result);
				$refclass = $result[1];
				if (!is_null($refclass))
					$refclass = $driver->GetClass($refclass);
				return new TMetaReferenceType($refclass,	strpos($options, "integritydelete") !== false);
			}break;
			default:{
				preg_match('/size=([^;]*)/', $options,$result);
				$size = $result[1];
				preg_match('/decimals=([^;]*)/', $options,$result);
				$decimals = $result[1];
				preg_match('/autogenerate=([^;]*)/', $options,$result);
				$autogenerate = $result[1];
				return TDataType::GetType($typecode,$size,$decimals,$autogenerate);
			}break;
		}
		return null;
	}
	
/**
 * converts row to attribute definition
 * @return IAttributeDefinition
 */
	public static function ToAttrDefinition(array $row, TMySqlMetaDriver $driver){
		if (!(
			(key_exists(TMySqlDBDriver::CLASS_ATTR_PK_FIELD,$row))
			&& (key_exists(TMySqlDBDriver::CLASS_ATTR_NAME_FIELD,$row))
			&& (key_exists(TMySqlDBDriver::CLASS_ATTR_FIELD_FIELD,$row))
			&& (key_exists(TMySqlDBDriver::CLASS_ATTR_OPTIONS_FIELD,$row))
			&& (key_exists(TMySqlDBDriver::CLASS_ATTR_TYPE_FIELD,$row))
			&& (key_exists(TMySqlDBDriver::CLASS_ATTR_REFCLASS_FIELD,$row))
		)) return null;
		
		$type = $this->getDataType($row[TMySqlDBDriver::CLASS_ATTR_OPTIONS_FIELD],$row[TMySqlDBDriver::CLASS_ATTR_TYPE_FIELD]);
		$indexed = strpos($row[TMySqlDBDriver::CLASS_ATTR_OPTIONS_FIELD], "indexed") !== false;
		$unique = strpos($row[TMySqlDBDriver::CLASS_ATTR_OPTIONS_FIELD], "unique") !== false;
		$nullable = strpos($row[TMySqlDBDriver::CLASS_ATTR_OPTIONS_FIELD], "nullable") !== false;
		$periodic = strpos($row[TMySqlDBDriver::CLASS_ATTR_OPTIONS_FIELD], "periodic") !== false;
		$result = array();
		preg_match('/default=([^;]*)/',$row[TMySqlDBDriver::CLASS_ATTR_OPTIONS_FIELD],$result);
		$default = $result[1];

		$c = null;
		if (key_exists(TMySqlMetaDriver::CLASS_PK_FIELD,$row))
			$c = $driver->GetClass($row[TMySqlMetaDriver::CLASS_PK_FIELD]);
		else if (key_exists(TMySqlMetaDriver::CLASS_NAME_FIELD,$row))	
			$c = $driver->GetClass($row[TMySqlMetaDriver::CLASS_NAME_FIELD]);
		
		if (!is_null($type) && (key_exists(TMySqlMetaDriver::CLASS_ATTR_PK_FIELD,$row)) && (key_exists(TMySqlMetaDriver::CLASS_ATTR_NAME_FIELD,$row)) && (key_exists(TMySqlMetaDriver::CLASS_ATTR_FIELD_FIELD,$row)))	
			return new TDBDefinedAttribute($row[TMySqlMetaDriver::CLASS_ATTR_PK_FIELD],$row[TMySqlMetaDriver::CLASS_ATTR_NAME_FIELD],$row[TMySqlMetaDriver::CLASS_ATTR_FIELD_FIELD],$type,$c,$row[TMySqlMetaDriver::CLASS_ATTR_STATIC_FIELD] == 1,$unique, $indexed, $nullable, $default, $periodic);
		return null;
	}
/**
 * converts to instance
 * @return IInstance
 */	
	public static function ToInstance(array $row, TMySqlMetaDriver $driver){
		if (!((key_exists(TMySqlMetaDriver::CLASS_NAME_FIELD,$row) || key_exists(TMySqlMetaDriver::CLASS_PK_FIELD,$row))
			&& (key_exists(TMySqlMetaDriver::ENTITY_PK_FIELD,$row))
			)) return null;
		if (key_exists(TMySqlMetaDriver::CLASS_NAME_FIELD,$row))	
			$c = $driver->GetClass($row[TMySqlMetaDriver::CLASS_NAME_FIELD]);
		else	
			$c = $driver->GetClass($row[TMySqlMetaDriver::CLASS_PK_FIELD]);
		$instance = new TMySqlDBInstance($row[TMySqlMetaDriver::ENTITY_PK_FIELD],$c);
			
		foreach ($row as $key=>$value)
			if (is_string($key)){
				$ca = $c->GetAttributeDefinition($key);
				if (!is_null($ca))
					$instance->SetAttributeValue($key,$value,false);
			}
		return $instance;
	}
}

class TMySqlDBSnapshotIterator extends TIteratorAdapter {
/**
 * @var TMySqlDBDriver
 */	
	private $_driver_;
/**
 * @var TInstanceSnapshot
 */	
	private $_item_ = null;
	
	private $_since_;
	
/**
 * @var TMySqlDBInstance
 */	
	private $_instance_;
	
	public function __construct(IIterator $iterator, TMySqlDBDriver $driver, TMySqlDBInstance $instance, $since){
		parent::__construct($array);
		$this->_driver_ = $driver;
		$this->_instance_ = $instance;
		$this->_since_ = $since;
	}
	
	private function _load_item(){
		$old = $this->_item_;
		$temp = parent::Item();
		$ss_date = new TDate($temp["snapshot_date"]);
		$this->_item_ = new TInstanceSnapshot($ss_date,$this->_instance_);
		foreach ($temp as $key=>$value)
			if ($ca = $this->_instance_->InstanceClass()->GetAttributeDefinition($key)){
				if (is_null($value))
					if ($old instanceof IInstance)
						$value = $old->GetAttributeValue($key);
				$this->_item_->SetAttributeValue($key,$value,false);
			}
		if ($this->_since_ instanceof TDate)
			if ($this->_since_->TimeStamp() > $ss_date->TimeStamp())
				$this->Next();		
	}
	
	public function Item(){
		return $this->_item_;
	}
	
	public function Next(){
/*
 * @todo probably bug risk
 */		
		$this->_load_item();
		return parent::Next();
	}
	
	public function Seek($position){
/*
 * @todo probably bug risk
 */		
		$this->_load_item();
		return parent::Seek($position);
	}	
}


/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 */
final class TMySqlDBClassIterator extends TMySqlDBRawDataIterator {
	private $_item_ = null;
	
	public function Next(){
		unset($this->_item_);
		return parent::Next();
	}	
	public function Item(){
		if (!isset($this->_item_))
			$this->_item_ = TMySqlRowConverter::ToClass($this->currentRow,$this->driver); 
		return $this->_item_;
	}
	public function DataType(){return IIterator::DT_CLASS;}
}

final class TMySqlDBMixedIterator extends TMySqlDBRawDataIterator {
	public function Item(){
		$_ITEM = array();
		$_ITEM["instance"] = TMySqlRowConverter::ToInstance($this->currentRow,$this->driver);
		$_ITEM["class"] = TMySqlRowConverter::ToClass($this->currentRow,$this->driver);
		$_ITEM["attrdef"] = TMySqlRowConverter::ToAttrDefinition($this->currentRow,$this->driver);
		$_ITEM["row"] = $this->currentRow;
		return $_ITEM;
	}
	public function DataType(){return IIterator::DT_MIXED;}	
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 */
final class TMySqlDBClassAttrIterator extends TMySqlDBRawDataIterator {
	private $_item_ = null;
			
	public function Next(){
		unset($this->_item_);
		return parent::Next();
	}	
	
	public function Seek($count){
		unset($this->_item_);
		return parent::Seek($count);
	} 
	
	public function Item(){
		if (!isset($this->_item_))
			$this->_item_ = TMySqlRowConverter::ToAttrDefinition($this->currentRow,$this->driver);
		return $this->_item_;
	}
	
	public function DataType(){return IIterator::DT_ATTRDEFINITION;}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 */
final class TMySqlDBInstanceIterator extends TMySqlDBRawDataIterator {
	private $_instance_ = null;
			
	public function Next(){
		unset($this->_instance_);
		return parent::Next();
	}	

	public function Seek($count){
		unset($this->_item_);
		return parent::Seek($count);
	}	
	
	public function Item(){
		if (!isset($this->_instance_))
			$this->_instance_ = TMySqlRowConverter::ToInstance($this->currentRow,$this->driver);
		return $this->_instance_;
	}
	public function DataType(){return IIterator::DT_INSTANCE;}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 */
class TMySqlDBClassIteratorAdapter extends TIteratorAdapter {
	private $_item_ = null;
	private $_driver_;
	
	public function __construct(IIterator $iterator, TMySqlDBDriver $driver){
		$this->_driver_ = $driver;
		parent::__construct($iterator);
	}
	
	public function Next(){
		unset($this->_item_);
		return parent::Next();
	}	
	
	public function Seek($count){
		unset($this->_item_);
		return parent::Seek($count);
	}	
	
	public function Item(){
		if (!isset($this->_item_))
			$this->_item_ = TMySqlRowConverter::ToClass($this->iterator->Item(),$this->_driver_); 
		return $this->_ITEM;
	}
	public function DataType(){return IIterator::DT_CLASS;}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 */
class TMySqlDBClassAttrIteratorAdapter extends TIteratorAdapter {
	private $_item_ = null;
	private $_driver_;
	
	public function __construct(IIterator $iterator, TMySqlDBDriver $driver){
		$this->_driver_ = $driver;
		parent::__construct($iterator);
	}
				
	public function Next(){
		unset($this->_item_);
		return parent::Next();
	}

	public function Seek($count){
		unset($this->_item_);
		return parent::Seek($count);
	}	
	
	public function Item(){
		if (!isset($this->_item_))
			$this->_item_ = TMySqlRowConverter::ToAttrDefinition($this->iterator->Item(),$this->_driver_); 
		return $this->_item_;
	}
	public function DataType(){return IIterator::DT_ATTRDEFINITION;}
}

/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 */
class TMySqlDBInstanceIteratorAdapter extends TIteratorAdapter {
	private $_instance_ = null;
	private $_driver_;
	
	public function __construct(IIterator $iterator, TMySqlDBDriver $driver){
		$this->_driver_ = $driver;
		parent::__construct($iterator);
	}
				
	public function Next(){
		unset($this->instance_);
		return parent::Next();
	}	
	
	public function Seek($count){
		unset($this->_instance_);
		return parent::Seek($count);
	}	
	
	public function Item(){
		if (!isset($this->_instance_))
			$this->_instance_ = TMySqlRowConverter::ToInstance($this->iterator->Item(),$this->_driver_);
		return $this->_instance_;
	}
	public function DataType(){return IIterator::DT_INSTANCE;}
}

?>