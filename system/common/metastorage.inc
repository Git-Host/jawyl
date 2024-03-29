<?php
require_once 'common/meta.inc';
require_once 'common/storage.inc';

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * base class for meta storage exceptions
 */
class TMetaStorageException extends TMetaException {
	const ERR_QUERY_INCORRECT = 501;
	const ERR_QUERY_NOT_LOADED = 502;
	const ERR_STORE_CLASS_NOT_FOUND = 503;
	const ERR_STORE_CLASS_NO_UNDEFINE = 504;
	const ERR_STORE_BAD_ATTR_TYPE = 505;
	const ERR_STORE_NO_ENTITY_ID = 507;

/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_QUERY_INCORRECT:return 'Incorrect xml query!';break;
			case self::ERR_QUERY_NOT_LOADED:return 'Xml query not loaded!';break;
			case self::ERR_STORE_CLASS_NOT_FOUND:return 'Class "%class" not found!';break;
			case self::ERR_STORE_CLASS_NO_UNDEFINE:return 'Class "%class" can not be undefined!';break;
			case self::ERR_STORE_BAD_ATTR_TYPE:return 'Inapropriate attribute type specified!';break;
			case self::ERR_STORE_STRUCTURE_NOT_FOUND:return 'Structure "%struct" not found!';break;
			case self::ERR_STORE_NO_ENTITY_ID:return 'No entity identification information specified!';break;
			default:return parent::getMessageText($msgcode);break;		
		}
	}
}

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * interface for implementing database engine meta drivers  
 */
interface IMetaDriver {
/**
 * defines class or attribute
 * @param IClass|IAttributeDefinition|array $meta
 * @return boolean
 */		
	public function Define($meta);
/**
 * undefines class or attribute
 * @param mixed $meta
 * @return boolean
 */		
	public function Undefine($meta);
/**
 * checks if class is defined
 * @param mixed $meta
 * @return boolean
 */		
	public function IsDefined($meta);
/**
 * gets class
 * @param string $name
 * @return IClass
 * @see IClass
 */		
	public function GetClass($name);
/**
 * 
 * gets value of class static attribute
 * @return mixed
 */		
	public function StaticValue(IClass $class, IAttributeDefinition $adef, TDate $date = null);
/**
 * saves class static sttribute value
 * @param mixed $value
 * @return boolean
 */		
	public function SaveStatic(IClass $class, IAttributeDefinition $adef, $value, TDate $date = null);		
/**
 * gets class ancestors
 * @param string $classname class name
 * @param boolean $onlydirect optional, defaults to false, when set to true only direct ancestor is fetched
 * @return IIterator iterator of IClass 
 * @see IIterator
 * @see IClass 
 *  */		
//	public function ClassAncestors(IClass $class, $onlydirect = false);
/**
 * gets class descendants
 * @param string $classname class name
 * @param boolean $onlydirect optional, defaults to false, when set to true only direct descendants are fetched
 * @return IIterator iterator of IClass
 * @see IIterator
 * @see IClass 
 */		
//	public function ClassDescendants(IClass $class, $onlydirect = false);
/**
 * saves instance to storage
 * @return boolean
 */		
	public function SaveInstance(IInstance $instance, TDate $date = null);
/**
 * sets attributes of the set of instances according to conditions and specified mask
 * only non-periodic attributes are affected
 * @param IInstance $mask mask instance for attribute values and class specification
 * @param TCondition[] $conditions array of conditions
 * @return boolean  
 */		
	public function InstancesBulkEdit(IInstance $mask, TDataSource $instances);
/**
 * gets instance from storage
 * @param mixed $id instance unique id
 * @return IInstance
 * @see IInstance
 */		
	public function Instanciate($id);
/**
 * sets instance attributes to values from storage
 * @return boolean
 * @see IInstance
 */		
	public function LoadInstance(IInstance $instance, TDate $date = null);
/**
 * gets instance snapshots for specified period 
 * @param IInstance $instance
 * @param TDate $since
 * @param TDate $till
 * @return IIterator<IInstance>
 */	
	public function SnapShots(IInstance $instance,TDate $since = null, TDate $till = null);
/**
 * gets instances of class
 * @param string $classname class name
 * @param boolean $exact optional,defaults to false, when set to true instances of class descendants are not fetched
 * @param TMetaFetchOptions $options optional fetch options
 * @return IIterator iterator of IInstance
 * @see IIterator
 * @see IInstance
 * @see TMetaFetchOptions
 */		
	public function FetchInstances(TDataSource $instances);
/**
 * deletes instance from storage
 * @return boolean
 * @see IInstance
 */		
	public function DeleteInstance(IInstance $instance);
/**
 * deletes instances of specified class according to conditions
 * @return boolean
 * @param string $classname name of class
 * @param TCondition[] $conditions array of conditions
 */		
	public function DeleteInstances(TDataSource $instances);
}

interface IReflectionDriver {
/**
 * gets meta class for php class. 
 * If CHECK_REFLECTION is set to true then storage class is checked to fit php class, 
 * which means performing all necessary data definitions in database.    
 * @return IClass
 */		
	public function ReflectClass(ReflectionClass $class);
}

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * class for storage system fields
 * @property int $SysField type of system field, returns one of TMetaSysField constants
 * @property TMetaClassSource $Source  
 */
final class TMetaSysField extends TDataSourceField {
	const INSTANCE_ID = 0;
	const CLASS_ID  = 1;
	const CLASS_NAME = 2;
	
	private $_sysfield_;

/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "SysField":return $this->_sysfield_;break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function Name(){
		switch ($this->_sysfield_){
			case self::INSTANCE_ID:return "id";break;
			case self::CLASS_ID:return "cid";break;
			case self::CLASS_NAME:return "classname";break;
		}
	}
/**
 * constructor
 * @param int $field one of TMetaSysField constants
 */	
	public function __construct($field, $alias = null, $dataset = null){
		if (!in_array($field,array(self::INSTANCE_ID,self::CLASS_ID,self::CLASS_NAME)))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		if (!is_null($dataset))
			if (!($dataset instanceof TMetaClassDataSource))
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
		parent::__construct($this->Name(),$alias,$dataset);	
	}  
}

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * base class for database meta drivers alowing reflection of php classes to orm database  
 */
class TReflectionDriver implements IReflectionDriver {
/**
@var IMetaDriver
 */	
	private $_meta_driver_; 
	
/**
 * @var boolean indicates whether to check class storage meta and tables when it is reflected
 */	
	private $_check_reflection_ = false;
	
	public function __construct(IMetaDriver $driver){
		$this->_meta_driver_ = $driver;
	}

/**
 * @param boolean $value
 */	
	public function SetCheckReflection($value){
		$this->_check_reflection_ = $value;
	}
				
/**
 * gets attribute type that fits to object property
 * @return TDataType
 */		
	protected function propType(ReflectionProperty $p){
		$attrtype = null;
		if ($p->isPrivate() && !$p->isStatic() && $p->isDefault())
		{
			$v = $p->getValue();
			if (is_bool($v))
				$attrtype = new TBooleanType();
			else if (is_double($v))
				$attrtype = new TFloatType();
			else if (is_int($v))
				$attrtype = new TLongType();
			else if (is_string($v))
				$attrtype = new TTextType();
			else if (is_object($v)){
				if ($v instanceof DateTime)
					$attrtype = new TDateTimeType();
				else if ($v instanceof TDataType)
					$attrtype = $v;
			}
		}
		return $attrtype;
	}

/**
 * gets meta class for php class. 
 * If CHECK_REFLECTION is set to true then storage class is checked to fit php class, 
 * which means performing all necessary data definitions in database.    
 * @return IClass
 */		
	public function ReflectClass(ReflectionClass $class){
		if (!$class->isSubclassOf("TDBReflectedInstance")){ 
			throw new TCoreException(TCoreException::ERR_NOT_CLASS,null,null,array("class"=>"TDBReflectedInstance"));
			return null;
		}
		$c = $this->_meta_driver_->GetClass($class->getName());	
		if ($this->_check_reflection_){
			if (!$c){
				$parent = $class->getParentClass();
				$dbpc = null;
				if ($parent->getName() != "TDBReflectedInstance"){
					$dbpc = $this->ReflectClass($parent);
					if ($dbpc) $dbpc = $dbpc->Name();
				}
					
				$this->_meta_driver_->Define(new TDBClass($class->getName(),$dbpc));
				$c = $this->_meta_driver_->GetClass($class->getName());	
			}

			$classattrs = $c->AttributeDefinitions();
				
			$eca = array();
				
			foreach ($classattrs as $ca){
				$prop = $class->getProperty($ca->Name());
				$at = $this->propType($prop);
				
				$delete = !is_null($prop);
				if (!$delete){
					$at = $this->propType($prop);
					$delete = $at != $ca->Type();
				}
					
				if ($delete)
					$this->_meta_driver_->Undefine($ca);
				else
					$eca[$ca->Name()] = 1;
			}
				
			$props = $class->getProperties();
			foreach ($props as $property){
				$at = $this->propType($property);
				if (!is_null($at) && !isset($eca[$property->getName()]))
					$this->_meta_driver_->Define(new TAttributeDefinition($property->getName(), $at, $c));
			}
			$c = $this->_meta_driver_->GetClass($class->getName());
		}
		return $c;
	}
}

/**
 * @package Common
 * @subpackage Meta
 * @category System 
 * class of meta class attribute definition 
 * @property string $Name
 * @property TDataType $Type
 * @property TDBClass $Class
 * @property boolean Static
 * @property boolean $Unique
 * @property boolean $Indexed
 * @property boolean $Nullable
 * @property mixed $Default
 * @property boolean $Periodic
 * @property boolean $AutoGenerate
 */
class TAttributeDefinition implements IAttributeDefinition {
/**
 * @var string 
 */		
	protected $name;
/**
 * @var string
 */		
	protected $type;
/**
 * @var IClass
 */		
	protected $class;
/**
 * @var boolean
 */		
	protected $static = false;
/**
 * @var boolean
 */				
	protected $unique = false;
/**
 * @var boolean should attribute be indexed in storage
 */		
	protected $indexed = false;
/**
 * @var boolean can attribute contain null value
 */		
	protected $nullable = true;
/**
 * @var mixed default attribute value
 */		
	protected $default = null;
/**
 * determines whether attribute is periodic
 * @var boolean
 */		
	protected $periodic = false;
	
/**
 * determines whether attribute value is generated automatically
 * @var boolean
 */	
	protected $autogenerate = false;
		
/**
 * constructor
 * @param string $name
 * @param boolean $static
 * @param boolean $unique
 * @param boolean $indexed
 * @param boolean $nullable
 * @param mixed $default
 * @param boolean $periodic
 * @param boolean $autogenerate
 */		
	public function __construct($name, TDataType $type, TDBClass $class = null, $static = false, $unique = false, $indexed = false, $nullable = true, $default = null, $periodic = false, $autogenerate = false){
		$this->name = $name;
		$this->static = $static;
		$this->type = $type;
		$this->class = $class;
		$this->unique = $unique;
		$this->indexed = $indexed;
		$this->nullable = $nullable;
		$this->default = $default;
		$this->periodic = $periodic;
		$this->autogenerate = $autogenerate;
	}

/**
 * @return string
 * @see IAttributeDefinition::Name()
 */		
	public function Name(){return $this->name;}
	
/**
 * @return TDataType
 * @see IAttributeDefinition::Type()
 */		
	public function Type(){return $this->type;}	
/**
 * @return IClass
 * @see IAttributeDefinition::AttributeClass()
 */		
	public function AttributeClass(){return $this->class;}
	
	public function SetAttributeClass(IClass $class){$this->class = $class;}
	
	public function IsStatic(){return $this->static;}
		
	public function IsPeriodic(){return $this->periodic;}
		
	public function __get($nm){
		switch ($nm){
			case "Name":return $this->Name();break;
			case "Type":return $this->type;break;
			case "Class":return $this->AttributeClass();break;
			case "Static":return $this->static;break;
			case "Unique":return $this->unique;break;
			case "Indexed":return $this->indexed;break;
			case "Nullable":return $this->nullable;break;
			case "Default":return $this->default;break;
			case "Periodic":return $this->periodic;break;
			case "AutoGenerate":return $this->autogenerate;break;
			default:return null;break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Class":{
				if ($this->class !== $value){
					$this->SetAttributeClass($value);	
					$this->class->AttributeDefinitions = $this;
				}
			}break;
		}
	}
	
	public function __toString(){
		return $this->type.";".($this->unique?"unique;":"").($this->indexed?"indexed;":"").($this->default?("default=".$this->default.";"):"").($this->periodic?"periodic;":"").($this->autogenerate?"autogenerated;":"");
	}
}

/**
 * @package Common
 * @subpackage Meta
 * @category System 
 * class of meta class attribute definition 
 * @property mixed $Id
 * @property string $Name
 * @property string $Field
 * @property TDataType $Type
 * @property TDBClass $Class
 * @property boolean Static
 * @property boolean $Unique
 * @property boolean $Indexed
 * @property boolean $Nullable
 * @property mixed $Default
 * @property boolean $Periodic
 */	
class TDBDefinedAttribute extends TAttributeDefinition {
/**
 * @var mixed
 */	
	protected $id;
/**
 * @var string
 */		
	protected $field;
/**
 * constructor
 * @param mixed $id
 * @param string $name
 * @param string $field
 * @param boolean $static
 * @param boolean $unique
 * @param boolean $indexed
 * @param boolean $nullable
 * @param mixed $default
 * @param boolean $periodic
 */		
	public function __construct($id,$name,$field,TDataType $type, TDBClass $class, $static = false, $unique = false, $indexed = false, $nullable = true, $default = null, $periodic = false){
		$this->id = $id;
		$this->field = $field;
		parent::__construct($name, $type, $static, $unique, $indexed, $nullable, $default, $periodic, $class);
		$this->class->AttributeDefinitions = $this;
	}
	
/**
 * @return mixed
 * @see IAttributeDefinition::Id()
 */		
	public function Id(){return $this->id;}
/**
 * @return string
 * @see IAttributeDefinition::Field()
 */		
	public function Field(){return $this->field;}
		
	public function __get($nm){
		switch ($nm){
			case "Id":return $this->id;break;
			case "Field":return $this->field;break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Class":{
				if (!($value instanceof TDBClass))
					throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				parent::__set($nm,$value);
			}break;
		}
	}	
}

class TDBClass extends TClass {
/**
 * @var IClass
 */	
	protected $ancestor = false;
		
	public function __construct($name,$ancestor = false, $attributes = null){
		parent::__construct($name,$attributes);
		if (($ancestor instanceof IClass) || ($ancestor === false))
			$this->ancestor = $ancestor;
		else throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
	}
		
	public function Ancestor(){return $this->ancestor;}
	public function Instances(){return new TArrayIterator(array());}
	public function Descendants(){return new TArrayIterator(array());}
	protected function loadStatic(IAttributeDefinition $def, TDate $date = null){return null;}
	protected function saveStatic(IAttributeDefinition $def, $value, TDate $date = null){return false;}
}

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * Base class for meta classes using meta database drivers  
 * @property TDBDefinedClass[] $Descendants 
 */
class TDBDefinedClass extends TDBClass {
/**
 * @var IMetaDriver
 */	
	protected $driver;
/**
 * @var mixed
 */		
	protected $id;

	protected $descendants = array();
	
/**
 * constructor
 * @param mixed $id class unique id
 * @param string $name class unique name
 * @param IMetaDriver $driver driver to use
 * @param array $attributes 
 */	
	public function __construct($id,$name,IMetaDriver $driver, TDBDefinedClass $ancestor = null, $attributes = null){
		$this->id = $id;
		$this->driver = $driver;
		if ($ancestor){
			parent::__construct($name,$ancestor,$attributes);
			$ancestor->Descendants = $this;
		}
		else
			parent::__construct($name);
	}
	
	protected function addAttributeDefinition(IAttributeDefinition $def){
		if (!($def instanceof TAttributeDefinition))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		if ($def->Class !== $this)	
			$def->Class = $this;
		parent::addAttributeDefinition($def);
	}
	
	protected function addDescendant(TDBDefinedClass $c){
		$this->descendants[] = $c;
	}

	public function Ancestor(){
		return $this->ancestor;
	}	
	
	public function Descendants(){
		return $this->descendants;
	}
	
/**
 * gets used driver
 * @return IMetaDriver
 */	
	public function Driver(){return $this->driver;}
/**
 * gets class instances
 * @return IIterator
 * @see IClass::Instances()
 */	
	public function Instances(){
		return $this->driver->GetClassInstances($this->Name());
	}
	
	protected function loadStatic(IAttributeDefinition $def, TDate $date = null){
		return $this->driver->StaticValue($this,$date);
	}
	
	protected function saveStatic(IAttributeDefinition $def, $value, TDate $date= null){
		return $this->driver->SaveStatic($this,$def,$value,$date);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Descendants":return $this->Descendants();break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Descendants":$this->addDescendant($value);break;
			default:parent::__set($nm, $value);break;
		}
	}
}

/**
 * 
 * @property TDBDefinedClass $Class
 * @property boolean $FullFetch
 *
 */

final class TMetaClassDataSource extends TDataSource {
	private $_class_;
	private $_full_fetch_ = false;
	
	public function __construct(TDBDefinedClass $class, $alias, array $fields = array(),array $filter = array(),array $join = array(), array $sorting = array(), $istarget = false,$offset = null,$count = null){
		if (is_null($alias))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$flds = array();
		foreach ($fields as $f)
			if ($f instanceof TMetaClassDataSourceAttribute)
				$flds[] = $f;
		parent::__construct($alias, $flds, $filter,$join,$sorting,$istarget,$offset,$count);
		$this->_class_ = $class;
	}
	
/**
 * @return string
 */	
	public function Name(){
		return $this->Alias();
	}

	public function __get($nm){
		switch ($nm){
			case "Class":return $this->_class_;break;
			case "FullFetch":return $this->_full_fetch_;break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function AttachField(IDataSourceField $fld){
		if (!(($fld instanceof TMetaClassDataSourceAttribute) || ($fld instanceof TExpressionField) || ($fld instanceof TAllField)))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		if ($fld instanceof TAllField)
			$this->_full_fetch_ = true;	
		parent::AttachField($fld);
	}	
}

/**
 * @property TDBDefinedAttribute $Attribute
 *
 */

final class TMetaClassDataSourceAttribute extends TDataSourceField {
	private $_attr_;
	private $_alias_;
	
	public function __construct(TDBDefinedAttribute $attr, $alias = null, $dataset = null){
		$this->_attr_ = $attr;
		parent::__construct($attr->Name,$alias,$dataset);
	}
	
	public function AttachTo(TDataSource $datasource){
		if (!($datasource instanceof TMetaClassDataSource))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		parent::AttachTo($datasource);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Attribute":return $this->_attr_;break;
			default:return parent::__get($nm);break;
		}
	}
}
	
/**
 * @package Common
 * @subpackage Storage
 * @category System
 * Base class for instances using meta database drivers  
 */	
class TDBInstance extends TInstance {
/**
 * constructor
 * @param mixed $id instance unique id
 * @param TDBClass $class instance class
 */	
	public function __construct($id,TDBClass $class){parent::__construct($id,$class);}
/**
 * @see IInstance::Save()
 */	
	public function Save(TDate $date = null){return $this->class->Driver()->SaveInstance($this,$date);}
/**
 * @see IInstance::Refresh()
 */	
	public function Refresh(TDate $date = null){$this->class->Driver()->FillInstance($this,$date);}
/**
 * @see IInstance::Delete()
 */	
	public function Delete(){return $this->class->Driver()->DeleteInstance($this);}
	public function SnapShots(TDate $since = null, TDate $till = null){
		return $this->class->Driver()->SnapShots($this,$since,$till);
	}
}

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * Base class for reflected classes.    
 */	
abstract class TDBReflectedInstance extends TDBInstance {
/**
 * constructor. Internally performs class reflection. 
 * @param mixed $id unique instance id
 * @param TReflectionDBDriver $driver reflection driver to use
 */	
	public function __construct($id, TReflectionDBDriver $driver){
		parent::__construct($id,$driver->ReflectClass(new ReflectionClass(get_class($this))));
	}
/**
 * @ignore
 */	
	public function __set($nm,$val){
		if ($this->class->GetAttributeDefinition($nm)){
			$prop = new ReflectionProperty(get_class($this),$nm);
			if ($prop->isPrivate()){
				parent::__set($nm,$val);
				return;
			} else throw new TMetaException(TMetaException::ERR_META_NO_PROPERTY,array("property"=>$nm, "class"=>get_class($this)));
		}
		parent::__set($nm,$val);
	}
	
/**
 * @ignore
 */	
	public function __get($nm){
		if ($this->class->GetAttributeDefinition($nm)){
			$prop = new ReflectionProperty(get_class($this),$nm);
			if ($prop->isPrivate())
				return parent::__get($nm);
			else throw new TMetaException(TMetaException::ERR_META_NO_PROPERTY,array("property"=>$nm, "class"=>get_class($this))); 
		}
		return parent::__get($nm);
	}
	
/**
 * @ignore
 */	
	public function __isset($nm){
		if ($this->class->GetAttributeDefinition($nm)){
			$prop = new ReflectionProperty(get_class($this),$nm);
			if ($prop->isPrivate())
				return !is_null(parent::__get($nm));
			else throw new TMetaException(TMetaException::ERR_META_NO_PROPERTY,array("property"=>$nm, "class"=>get_class($this)));
		}
		return parent::__isset($nm);
	}
	
/**
 * @ignore
 */	
	public function __unset($nm){
		if ($this->class->GetAttributeDefinition($nm)){
			$prop = new ReflectionProperty(get_class($this),$nm);
			if ($prop->isPrivate())
				parent::__unset($nm);
			else throw new TMetaException(TMetaException::ERR_META_NO_PROPERTY,array("property"=>$nm, "class"=>get_class($this))); 
		}
		parent::__unset($nm);
	}
} 

	
/**
 * @package Common
 * @subpackage Meta
 * @category System 
 * abstract converter from DT_RAW iterator to DT_INSTANCE iterator 
 */	
abstract class TRawToInstanceIterator extends TIteratorAdapter {
/**
 * protected convertion method, used internally in TRawToInstanceIterator::Item() to convert from raw data to instance
 */		
	protected abstract function Convert(array &$instanceinfo);
		
/**
 * @see IIterator::Item()
 */		
	public function Item(){return $this->Convert($this->iterator->Item());}
/**
 * gets iterator data type, for TRawToInstanceIterator always return DT_INSTANCE
 * @return int
 * @see IIterator::DataType()
 */		
	public function DataType(){return IIterator::DT_INSTANCE;}		
}

