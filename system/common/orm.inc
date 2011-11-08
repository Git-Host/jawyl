<?php
/**
 * @package Common
 * @subpackage Orm
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

TApplication::import("common::iterator");
TApplication::import("common::data");

/**
 * @package Common
 * @subpackage Orm
 * @category System
 * orm exception class
 */
class TOrmException extends TException {
	const ERR_ORM_ATTR_NOT_DEFINED = 401;
	const ERR_ORM_NO_PROPERTY = 402;	

/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_ORM_ATTR_NOT_DEFINED:return 'Attribute %attr not defined in orm class %class!';break;
			case self::ERR_ORM_NO_PROPERTY:return 'Property %property not found in php class %class!';break;
		}
		return "";
	}
}

/**
 * @package Common
 * @subpackage Orm
 * @category System
 * class of reference attribute type
 * @property IClass $ReferenceClass
 * @property boolean $IntegrityDelete
 * @uses TDataType
 */	
	final class TOrmReferenceType extends TDataType {
		const ORM_REFERENCE = "ORMREF";
/**
 * @var IClass - orm class to which attribute refer
 */		
		private $_class_;
		
		private $_integrity_delete_ = false;
		
		public function __construct(IClass $c, $integritydelete = false){
			$this->typeCode = self::ORM_REFERENCE;
			$this->_integrity_delete_ = $integritydelete;
			$this->_class_ = $c;
		}
		
		public function __get($nm){
			switch ($nm){
				case "ReferenceClass":return $this->_class_;break;
				case "IntegrityDelete":return $this->_integrity_delete_;break;
			}
		}

		
/**
 * @return string
 */		
		public function __toString(){
			return parent::__toString().";refclass=".$this->Class->Name().";".($this->IntegrityDelete?"integritydelete":"");
		} 
	}	


/**
 * @package Common
 * @subpackage Orm
 * @category System
 * interface for implementing orm classes
 */	
	interface IClass {
/**
 * gets class name
 * @return string
 */		
		public function Name();
/**
 * gets class direct ancestor or false for root classes
 * @return IClass|false
 */		
		public function Ancestor();
		
/**
 * 
 * @return IIterator<IClass>
 */		
		public function Descenders();
/**
 * gets class attribute definition
 * @param string $name - attribute name or attribute storage field name
 * @return IAttributeDefinition
 * @see IAttributeDefinition
 */		
		public function GetAttributeDefinition($name);
/**
 * gets class attribute definitions
 * @return IAttributeDefinition[]
 * @see IAttributeDefinition
 */		
		public function AttributeDefinitions();
/**
 * gets whether class is a descender of another class, or is this class
 * @param string $name - name of class to test against
 * @return boolean 
 */		
		public function IsClass($name);
/**
 * gets all instances of class
 * @return IIterator iterator of class instances
 * @see IIterator
 * @see IInstance
 */		
		public function Instances();

/**
 * @param string $attrname
 * @return mixed
 */		
		public function GetStatic($name, TDate $date = null);

/**
 * @param string $attrname
 * @param mixed $value
 * @return boolean
 */		
		public function SetStatic($name, $value, TDate $date = null);
		
		public function HasPeriodicAttributes();
	}

/**
 * @package Common
 * @subpackage Orm
 * @category System
 * interface for implementing orm class attribute definitions
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
	interface IAttributeDefinition {
/**
 * gets attribute name
 * @return string
 */		
		public function Name();
/**
 * gets attribute type
 * @return TDataType
 * @see TDataType
 */		
		public function Type();
/**
 * gets class where attribute is defined
 * @return IClass
 * @see IClass
 */		
		public function AttributeClass();
		
		public function SetAttributeClass(IClass $class);

/**
 * @return boolean
 */		
		public function IsStatic();
/**
 * @return boolean
 */		
		public function IsPeriodic();
	}
	
/**
 * @package Common
 * @subpackage Orm
 * @category System
 * interface for implementing orm instance attributes
 */	
	interface IInstanceAttribute {
/**
 * gets attribute definition
 * @return IAttributeDefinition
 * @see IAttributeDefinition
 */		
		public function Definition();
/**
 * gets attribute value
 * @return mixed
 */		
		public function GetValue();
/**
 * sets attribute value
 * @param mixed $value value to set
 * @param boolean $needsave should new value be saved to storage, optional, defaults to true
 */		
		public function SetValue($value,$needsave = true);
/**
 * gets whether current attribute value should be saved to storage
 * @return boolean
 */		
		public function NeedSave();
	}

/**
 * @package Common
 * @subpackage Orm
 * @category System
 * interface for implementing orm instances
 */	
	interface IInstance {
/**
 * gets instance unique id
 * @return mixed
 */		
		public function Id();
/**
 * gets instance orm class
 * @return IClass
 * @see IClass
 */		
		public function InstanceClass();
/**
 * gets string representation of instance
 * @return string
 */	
		public function __toString();
/**
 * sets instance attribute value
 * @param string $name attribute name
 * @param mixed $value new attribute value
 * @param boolean $needsave should new value be saved to storage, optional, defaults to true
 * @return boolean
 */		
		public function SetAttributeValue($name,$value,$needsave = true);
/**
 * gets instance attribute value
 * @param string $name attribute name
 * @return mixed
 */		
		public function GetAttributeValue($name);
/**
 * gets instance attributes array
 * @return IInstanceAttribute[]
 */		
		public function Attributes();
/**
 * saves instance to storage, if instance id is set to null new instance is created
 * @param TDate $date used for instances with periodic attributes to save a snapshot. when not specified current date is used
 */		
		public function Save(TDate $date = null);
/**
 * reloads instance attribute values from storage
 */		
/**
 * updates instance attributes from storage
 * @param TDate $date used for instances with periodic attributes to load snapshot values. when not specified current date is used
 */		
		public function Refresh(TDate $date = null);
/**
 * deletes instance from storage
 */		
		public function Delete();
/**
 * gets instance snapshots within specified period
 * @param TDate $since when specified snapshots are returned inclusively since this date
 * @param TDate $till when specified snapshots are inclusively constrained by this date
 * @return IIterator iterator of TInstanceSnapshot  
 */
		public function SnapShots(TDate $since = null, TDate $till = null);		
	}

		
/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * class of orm instance attribute 
 */		
	class TInstanceAttribute implements IInstanceAttribute {
/**
 * @var IInstance
 */		
		protected $instance;
/**
 * @var IAttributeDefinition
 */		
		protected $definition;
/**
 * @var boolean
 */		
		protected $needSave = false;
/**
 * @var mixed
 */		
		protected $value;
		
		public static function InstanceAttribute(IInstance $instance,IAttributeDefinition $definition){
			switch ($definition->Type()->TypeCode){
				case TDataType::DATETIME:return new TInstanceDateTimeAttribute($instance,$definition);break;
				case TDataType::IMAGE:
				case TDataType::FILE:return new TInstanceFileAttribute($instance,$definition);
				case TDataType::IMAGELINK:
				case TDataType::FILELINK:return new TInstanceFileLinkAttribute($instance,$definition);break;
				case TDataType::BOOLEAN:return new TInstanceBooleanAttribute($instance,$definition);break;
				case TDataType::GUID:return new TInstanceGuidAttribute($instance,$definition);break;
				case TDataType::SET:return new TInstanceSetAttribute($instance,$definition);break;
			}
			return new TInstanceAttribute($instance,$definition);			
		}
/**
 * constructor
 */		
		public function __construct(IInstance $instance,IAttributeDefinition $definition){
			$this->instance = $instance;
			$this->definition = $definition;
		}
/**
 * @see IInstanceAttribute::Definition()
 */		
		public function Definition(){return $this->definition;}
/**
 * @see IInstanceAttribute::GetValue()
 */		
		public function GetValue(){
			if ((is_null($this->value)) && ($this->definition->AutoGenerate)){
				$this->value = $this->generateValue();
				$this->needSave = true;
			}
			return $this->value;
		}
/**
 * @see IInstanceAttribute::SetValue()
 */		
		public function SetValue($value,$needsave = true){
			if ($value instanceof IInstance)
				$value = $value->Id();	
			$this->value = $value;
			$this->needSave = $needsave;
		}
/**
 * @see IInstanceAttribute::NeedSave()
 */		
		public function NeedSave(){
			if ((is_null($this->value)) && ($this->definition->AutoGenerate))
				$this->needSave = true;
			return $this->needSave;
		}
		
		protected function generateValue(){
			return $this->definition->Default;
		}
	}
	
/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * class of orm instance boolean attribute 
 */	
	class TInstanceBooleanAttribute extends TInstanceAttribute {
/**
 * @see IInstanceAttribute::SetValue()
 */		
		public function SetValue($value,$needsave = true){
			parent::SetValue(TConvertions::ConvertToBoolean($value),$needsave);	
		}		
	}	
	
	class TInstanceGuidAttribute extends TInstanceAttribute {
		protected function autoGeneration(){
			return $this->definition->Type()->Generate();
		}
	}
	
	class TInstanceSetAttribute extends TInstanceAttribute {
		public function SetValue($value,$needsave = true){
			if (is_array($value)){
				$temp = $value;
				$value = 0;
				foreach ($temp as $v)
					$value = $value | $v;	
			}
			parent::SetValue($value,$needsave);	
		}			
	}

/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * class of orm instance datetime attribute 
 */	
	class TInstanceDateTimeAttribute extends TInstanceAttribute {
/**
 * @see IInstanceAttribute::SetValue()
 */		
		public function SetValue($value,$needsave = true){
			if ($value instanceof TDate)
				$d = $value;
			else if (!is_null($value)) 
				$d = new TDate($value);
			else 
				$d = $value;	
			parent::SetValue($d,$needsave);	
		}		
	} 

/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * class of orm instance file attribute 
 */	
	class TInstanceFileAttribute extends TInstanceAttribute {
/**
 * @see IInstanceAttribute::SetValue()
 */		
		public function SetValue($value,$needsave = true){
			if ($value instanceof TFile){
				$v = new TDBStoredFile('<file><header><name>'.$value->Name.'</name><mime>'.$value->MimeType.'</mime></header><content><![CDATA['.$value->getContents().']]></content></file>');
				parent::SetValue($v,$needsave);
			}
			else if (is_string($value)){
				$v = null;
				if ($value != "")
					$v = new TDBStoredFile($value);
				parent::SetValue($v,$needsave);
			}
			else
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
		}		
	}

/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * class of orm instance file link attribute 
 */	
	class TInstanceFileLinkAttribute extends TInstanceAttribute {
/**
 * @see IInstanceAttribute::SetValue()
 */		
		public function SetValue($value,$needsave = true){
			if (!is_null($value)){
				if ($value instanceof TFile)
					parent::SetValue($value,$needsave);
				else if (is_string($value) /*&& file_exists($value)*/){
					if (file_exists($value)){
						$finfo = pathinfo($value);
						$f = new TFile($finfo["basename"],$value);
						parent::SetValue($f,$needsave);
					} else parent::SetValue(null,$needsave);
				}
				else if ($value == "")
					parent::SetValue(null,$needsave);
				else
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			}
			else
			parent::SetValue($value,$needsave);	
		}		
	}

/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * abstract base class for orm instances. Provides ability to refer to instance attributes through properties. 
 */	
	abstract class TInstance implements IInstance {
/**
 * @var mixed
 */		
		protected $id;
/**
 * @var IClass
 */		
		protected $class;
/**
 * @var array
 */		
		protected $attrValues = array();
/**
 * constructor
 */		
		public function __construct($id,IClass $class){
			$this->id = $id;
			$this->class = $class;
		}
/**
 * @see IInstance::Id()
 */		
		public function Id(){return $this->id;}
 /**
 * @see IInstance::InstanceClass()
 */		
		public function InstanceClass(){return $this->class;}
/**
 * @see IInstance::Soid()
 */		
		public function Soid(){return "I::".$this->Id();}
		
		protected function checkAttrValue($id){
			return key_exists($id,$this->attrValues);
		}
		
/**
 * @see IInstance::SetAttributeValue()
 */		
		public function SetAttributeValue($name,$value,$needsave = true){
			if ($attr = $this->class->GetAttributeDefinition($name)){
				if ($attr->IsStatic())
					return $this->class->SetStatic($name, $value);
				if (!$this->checkAttrValue($attr->Id()))
					$this->attrValues[$attr->Id()] = TInstanceAttribute::InstanceAttribute($this,$attr);
				if (!$this->attrValues[$attr->Id()]->NeedSave())	
					$this->attrValues[$attr->Id()]->SetValue($value,$needsave);
				return true;
			}
			throw new TOrmException(TOrmException::ERR_ORM_ATTR_NOT_DEFINED,array("attr"=>$name,"class"=>$this->class->Name()));
			return false;
		}
	
/**
 * @see IInstance::GetAttributeValue()
 */		
		public function GetAttributeValue($name){
			if ($attr = $this->class->GetAttributeDefinition($name)){
				if ($attr->IsStatic())
					return $this->class->GetStatic($name);
				if (!$this->checkAttrValue($attr->Id())) 
					$this->Refresh();
				if ($this->checkAttrValue($attr->Id()))
					return $this->attrValues[$attr->Id()]->GetValue();
				return null;	
			}
			throw new TOrmException(TOrmException::ERR_ORM_ATTR_NOT_DEFINED,array("attr"=>$name,"class"=>$this->class->Name()));
			return false;
		}
	
/**
 * @see IInstance::Attributes()
 */		
		public function Attributes(){
			return $this->attrValues;
		}
		
/**
 * @ignore
 */
		public function __set($nm,$val){
			$this->SetAttributeValue($nm,$val,true);
		}
		
/**
 * @ignore
 */
		public function __get($nm){
			return $this->GetAttributeValue($nm);
		}
		
/**
 * @ignore
 */
		public function __isset($nm){
			$v = $this->GetAttributeValue($nm);
			return $v !== false;
		}
		
/**
 * @ignore
 */
		public function __unset($nm){
			unset($this->attrvalues[$this->class->GetAttributeDefinition($name)->Id()]);
		}

/**
 * string representation of instance 
 * @return string
 */
		public function __toString(){
			return $this->Id();
		}
	}
	
/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * abstract base class for orm classes. 
 */		
	abstract class TClass implements IClass {
/**
 * @var array
 */		
		protected $attrByName = null;
		
		protected $staticValues = array();
		
		protected $hasPeriodic = false;
		
		protected function addAttributeDefinition(IAttributeDefinition $def){
			if ($def->IsPeriodic())
				$this->hasPeriodic = true;
			$this->attrByName[$def->Name()] = $def;
			$def->SetAttributeClass($this);
		}
		
/**
 * constructor
 */		
		public function __construct($name,$attributes = null){
			$this->name = $name;
			if (is_array($attributes))
				if (!empty($attributes))
					foreach ($attributes as $ad)
						$this->addAttributeDefinition($ad);
		}
/**
 * @see IClass::Name()
 */		
		public function Name(){return $this->name;}
/**
 * @see IClass::IsClass()
 */		
		public function IsClass($name){
			if ($this->name == $name)
				return true;
			else {
				$parent = $this->Ancestor();
				if ($parent instanceof IClass)
					return $parent->IsClass($name);
			}
			return false;
		}
			
/**
 * @ignore
 */		
		private function _get_attr($name){
			if (key_exists($name,$this->attrByName))
				return $this->attrByName[$name];
			$parent = $this->Ancestor();
			if ($parent instanceof IClass)
				return $parent->GetAttributeDefinition($name);
			return null;
		}
		
/**
 * @return IAttributeDefinition
 * @see IClass::GetAttributeDefinition
 */		
		public function GetAttributeDefinition($name){
			$result = $this->_get_attr($name);
			if (!$result)
				throw new TOrmException(TOrmException::ERR_ORM_ATTR_NOT_DEFINED, array("attr"=>$name,"class"=>$this->Name()));
		}
		
/**
 * @return array
 * @see IClass::AttributeDefinitions
 */		
		public function AttributeDefinitions(){
			return $this->attrByName;
		}

/**
 * gets class root ancestor
 * @return IClass
 */		
		public function GetOrigin(){
			$p = $this->Ancestor();
			if ($p instanceof IClass)
				return $p->GetOrigin();
			return $this;	
		}
		
/**
 * gets string representation of class
 * @return string
 */		
		public function __toString(){return $this->name;}
		
		protected abstract function loadStatic(IAttributeDefinition $def, TDate $date = null);
		
/**
 * @param string $attrname
 * @return mixed
 */		
		public function GetStatic($name, TDate $date = null){
			$result = null;
			$ca = $this->GetAttributeDefinition($name);
			if ($ca)
				if ($ca->IsStatic()){
					if ($ca->IsPeriodic()){
						if (is_null($date)) $date = new TDate();
						$result = $this->loadStatic($ca,$date);
					}
					else
						if (!key_exists($ca->Name(), $this->staticValues)){
							$result = $this->loadStatic($ca);
							if ($result)
								$this->staticValues[$ca->Name()] = $result;
						} else
							$result = $this->staticValues[$ca->Name()];

					if (is_null($result)){
						$parent = $this->Ancestor();
						if ($parent instanceof IClass)
							return $parent->GetStatic($name,$date);
					}	
				}
			return null;
		}

		protected abstract function saveStatic(IAttributeDefinition $def, $value, TDate $date = null);
		
/**
 * @param string $attrname
 * @param mixed $value
 * @return boolean
 */		
		public function SetStatic($name, $value, TDate $date = null){
			$ca = $this->GetAttributeDefinition($name);
			if ($ca)
				if ($ca->IsStatic()){
					if ($ca->IsPeriodic()){
						if (is_null($date)) $date = new TDate();
						return $this->saveStatic($ca, $value, $date);
					}
					else {
						$this->staticValues[$ca->Name()] = $value;
						$this->saveStatic($ca, $value);
					}
				}
			return false;
		}
		
		public function __set($nm,$value){
			switch ($nm){
				case "AttributeDefinitions":$this->addAttributeDefinition($value);break;
				default:$this->SetStatic($name, $value);break;
			}
		}

		public function __get($nm){
			switch ($nm){
				case "HasPeriodicAttributes":return $this->HasPeriodicAttributes();break;
				default:return $this->GetStatic($nm);break;
			}
		}
		
		public function HasPeriodicAttributes(){
			return $this->hasPeriodic;
		}
	}	
	
/**
 * @package Common
 * @subpackage Orm
 * @category System 
 * base class for instance wrappers.
 */	
	class TInstanceWrapper implements IInstance {
/**
 * @var IInstance
 */		
		protected $instance;
/**
 * constructor
 */		
		public function __construct(IInstance $instance){$this->instance = $instance;}		
		public function Id(){return $this->instance->Id();}
		public function Code(){return $this->instance->Code();}
		public function InstanceClass(){return $this->instance->InstanceClass();}
		public function Soid(){return $this->instance->Soid();}
		public function SetAttributeValue($name,$value,$needsave = true){return $this->instance->SetAttributeValue($name,$value,$needsave);}
		public function GetAttributeValue($name){return $this->instance->GetAttributeValue($name);}
		public function Attributes(){return $this->instance->Attributes();}
		public function Save(TDate $date = null){return $this->instance->Save($date);}
		public function Refresh(TDate $date = null){return $this->instance->Refresh($date);}
		public function Delete(){return $this->instance->Delete();}
		/**
		 * @ignore
		 */
		public function __set($nm,$val){
			$this->SetAttributeValue($nm,$val,true);/*$this->INSTANCE->__set($nm,$val);*/
		}
		/**
		 * @ignore
		 */
		public function __get($nm){
			return $this->GetAttributeValue($nm);/*$this->INSTANCE->__get($nm);*/
		}
		/**
		 * @ignore
		 */
		public function __isset($nm){return $this->instance->__isset($nm);}
		/**
		 * @ignore
		 */
		public function __unset($nm){$this->instance->__unset($nm);}
/**
 * @return string
 */		
		public function __toString(){return $this->instance->__toString();}
/**
 * @see IIterator::SnapShots()
 * @param TDate $since
 * @param TDate $till
 * @return TSnapshotIterator
 */		
		public function SnapShots(TDate $since = null, TDate $till = null){return $this->instance->SnapShots($since,$till);}
	}
	
	class TInstanceSnapshot extends TInstanceWrapper {
		protected $snapshotDate;
		protected $snapshotAttributes = array();
		
		public function SnapShotDate(){return $this->snapshotDate;}
		public function __construct(TDate $date,IInstance $instance){
			$this->snapshotDate = $date;
			parent::__construct($instance);
		}
/**
 * @see IInstance::SetAttributeValue()
 */		
		public function SetAttributeValue($name,$value,$needsave = true){
			if ($attr = $this->InstanceClass()->GetAttributeDefinition($name)){
				if ($attr->Type()->Periodic){
					if ($attr->IsStatic())
						return $this->InstanceClass()->SetStatic($name,$value,$this->snapshotDate);
					$this->snapshotAttributes[$name] = TInstanceAttribute::InstanceAttribute($this,$attr);
					$this->snapshotAttributes[$name]->SetValue($value,$needsave);
					return true;
				} 
				return parent::SetAttributeValue($name,$value,$needsave);
			}
			throw new TOrmException(TOrmException::ERR_ORM_ATTR_NOT_DEFINED,array("attr"=>$name,"class"=>$this->class->Name()));
			return false;
		}
	
/**
 * @see IInstance::GetAttributeValue()
 */		
		public function GetAttributeValue($name){
			if ($attr = $this->InstanceClass()->GetAttributeDefinition($name)){
				if ($attr->Type()->Periodic){
					if ($attr->IsStatic())
						return $this->InstanceClass()->GetStatic($name,$this->snapshotDate);
					if (key_exists($name,$this->snapshotAttributes))
						return $this->snapshotAttributes[$name]->GetValue();
					return null;
				} 
				return parent::GetAttributeValue($name);
			}
			throw new TOrmException(TOrmException::ERR_ORM_ATTR_NOT_DEFINED,array("attr"=>$name,"class"=>$this->InstanceClass()->Name()));
			return false;
		}
	
/**
 * @see IInstance::Attributes()
 */		
		public function Attributes(){
			$attrs = parent::Attributes();
			foreach ($this->snapshotAttributes as $key=>$attr)
			$attrs[$key] = $attr;
			return $attrs;
		}
	}
?>