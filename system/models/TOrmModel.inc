<?php
/**
 * @package Components
 * @subpackage Orm
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


TApplication::import("common::orm");
TApplication::import("common::item"); 
TApplication::import("models::TDataModel");

class TOrmClassPermission extends TPolicy {
	const ORM_INSTANCIATE = "orm.instant";
	const ORM_INSTLIST = "orm.instlist";
	
	protected function checkType($type){
		return ($this->sobj instanceof TClass) && (in_array($type, array(self::ORM_INSTANCIATE,self::ORM_INSTLIST)));
	}
	
	public function Descenders(){
		switch ($this->type){
			case self::ORM_INSTANCIATE:
			case self::ORM_INSTLIST:return array(new TServiceRole(TServiceRole::USER));break;
		}
		return array();
	}	
}

class TOrmItemRole extends TPolicy {
	const ORM_OWNER = "orm.owner";
	
	protected function checkType($type){
		return ($this->sobj instanceof TOrmItem) && ($type == self::ORM_OWNER);
	}
	
	public function Descenders(){
		switch ($this->type){
			case self::ORM_OWNER:return array(new TServiceRole(TServiceRole::ADMIN));break;
		}
		return array();
	}
/**
 * checks whether specified policy is role  
 * @param string $type
 * @return boolean
 */
	public static function IsRole($policy){
		return $policy == self::ORM_OWNER;
	}
}

class TOrmItemPermission extends TPolicy {
	const ORM_DELETE = "orm.delete";
	const ORM_RELATE = "orm.relate";
	const ORM_LIST = "orm.list";
	const ORM_VIEW = "orm.view";
	const ORM_MODIFY = "orm.modify";
	const ORM_ACL = "orm.acl";

	protected function checkType($type){
		return ($this->sobj instanceof TOrmItem) && (in_array($type, array(self::ORM_ACL,self::ORM_DELETE,self::ORM_LIST,self::ORM_MODIFY,self::ORM_RELATE,self::ORM_VIEW)));
	}
	
	public function Descenders(){
		switch ($this->type){
			case self::ORM_ACL:
			case self::ORM_DELETE:
			case self::ORM_MODIFY:return array(new TServiceRole(TServiceRole::ADMIN),new TOrmItemRole(TOrmItemRole::ORM_OWNER,$this->sobj));break;
			case self::ORM_RELATE:return array(new TServiceRole(TServiceRole::USER),new TOrmItemRole(TOrmItemRole::ORM_OWNER,$this->sobj));break;
			case self::ORM_VIEW:
			case self::ORM_LIST:return array(new TServiceRole(TServiceRole::ANONYMOUS),new TOrmItemRole(TOrmItemRole::ORM_OWNER,$this->sobj));break;
		}
		return array();
	}	
}

class TOrmPropertyPermission extends TPolicy {
	const ORM_READ = "orm.read";
	const ORM_WRITE = "orm.write";
	
	protected function checkType($type){
		return ($this->sobj instanceof TOrmProperty) && (in_array($type, array(self::ORM_READ,self::ORM_WRITE)));
	}
	
	public function Descenders(){
		$result = array(new TServiceRole(TServiceRole::USER));
		$item = $this->sobj->Item();
		switch ($this->type){
			case self::ORM_READ:$result[] = new TOrmItemPermission(TOrmItemPermission::ORM_VIEW,$item);break;
			case self::ORM_WRITE:$result[] = new TOrmItemPermission(TOrmItemPermission::ORM_MODIFY,$item);break;
		}
		return $result;
	}
}

/**
 * @package Components
 * @subpackage Orm
 * @category System
 * Orm component exception class
 */
class TOrmModelException extends TOrmException {
	const ERR_ORMMETA_INCORRECT = 801;
/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_ORMMETA_INCORRECT:return 'Orm model meta file incorrect!';break;
			default:return parent::getMessageText($msgcode);break;
		}
	}
}

class TOrmPropertyMeta implements IPropertyMeta {
/**
 * @var array
 */	
	protected $selection;
/**
 * @var string
 */	
	protected $caption;
/**
 * @var string
 */	
	protected $name;
/**
 * @var string
 */	
	protected $type;
/**
 * @var boolean
 */	
	protected $nullable;
/**
 * @var boolean;
 */	
	protected $readonly;
	
/**
 * constructor
 * @param IInstance $i instance of wrapped attribute
 * @param string $name property name
 * @param TItemPropertyType $type property type
 * @param boolean $nullable is property nullable
 * @param array $sellist optional values selection list, associative array of values and titles
 * @see IProperty  
 */	
	public function __construct($name, TItemPropertyType $type, $nullable, array $sellist = null,$readonly = false){
		$this->name = $name;
		$this->type = $type;
		$this->nullable = $nullable;
		$this->caption = $this->Name();
		if (is_array($sellist))
			$this->selection = $sellist;
		$this->readonly = $readonly;
	}
/**
 * @see IPropertyMeta::Name()
 * @return string
 */	
	public function Name(){
		return $this->name;
	}
/**
 * @see IPropertyMeta::Type()
 * @return TItemPropertyType
 */	
	public function Type(){
		return $this->type;
	}
/**
 * @see IPropertyMeta::Caption()
 * @return string
 */	
	public function Caption(){return $this->caption;}
/**
 * @see IPropertyMeta::SetSelection()
 */		
	public function SetSelection(array $selection){
		if (is_array($selection))
			$this->selection = $selection;
	}
/**
 * @see IPropertyMeta::Selection()
 * @return array
 */	
	public function Selection(){
		if (is_array($this->selection))
			return $this->selection;
		return array();
	}
/**
 * sets property caption
 * @param string $caption
 */	
	public function SetCaption($caption){
		$this->caption = $caption;
	}

	public function ReadOnly(){
		return $this->readonly;
	}
	
	public function Nullable(){
		return $this->nullable;
	}
}

/**
 * @package Components
 * @subpackage Orm
 * @category System
 * Orm item property class
 */
class TOrmProperty extends TProperty {
/**
 * @var TOrmItem  
 */	
	protected $item;	
	
/**
 * constructor
 * @param IInstance $i instance of wrapped attribute
 * @param string $name property name
 * @param string $type property type
 * @param boolean $nullable is property nullable
 * @param array $sellist optional values selection list, associative array of values and titles
 * @see IProperty  
 */	
	public function __construct(TOrmItem $item, TOrmPropertyMeta $meta){
		$this->item = $item;
		parent::__construct($meta);
	}
/**
 * @see IProperty::Value()
 * @return mixed
 */	
	public function Value(){
		return $this->item->Instance()->__get($this->Name());
	}
	
	public function Item(){
		return $this->item;
	}
}

class TOrmAttributePropertyMeta extends TOrmPropertyMeta {
/**
 * @var IAttributeDefinition
 */	
	protected $attrDefinition;

/**
 * constructor
 * @param IInstance $i instance wrapped attribute belongs to
 * @param IAttributeDefinition $ca definition of wrapped attribute
 * @param array $sellist optional values selection list, associative array of values and titles
 * @see IProperty  
 */	
	public function __construct(IAttributeDefinition $ca, array $sellist = null){
		$this->attrDefinition = $ca;
		parent::__construct($ca->Name(),self::TypeFromDefinition($ca),$ca->Type()->Nullable,$sellist,false);
	}
	
/**
 * gets property type for specified attribute definition
 * @return TItemPropertyType
 */	
	public static function TypeFromDefinition(IAttributeDefinition $ca){
		switch ($ca->Type()->TypeCode){
			case TAttributeType::BIGINT:return TItemPropertyType::GetType(TItemPropertyType::PT_INT);break;
			case TAttributeType::BOOLEAN:return  TItemPropertyType::GetType(TItemPropertyType::PT_BOOL);break;
			case TAttributeType::DATETIME:return  TItemPropertyType::GetType(TItemPropertyType::PT_DATE);break;
			case TAttributeType::DECIMAL:return  TItemPropertyType::GetType(TItemPropertyType::PT_DEC);break;
			case TAttributeType::FILE:return  TItemPropertyType::GetType(TItemPropertyType::PT_FILE);break;
			case TAttributeType::FILELINK:return  TItemPropertyType::GetType(TItemPropertyType::PT_FILE);break;
			case TAttributeType::FLOAT:return  TItemPropertyType::GetType(TItemPropertyType::PT_DEC);break;
			case TAttributeType::HTML:return  TItemPropertyType::GetType(TItemPropertyType::PT_HTML);break;
			case TAttributeType::IMAGE:return  TItemPropertyType::GetType(TItemPropertyType::PT_IMAGE);break;
			case TAttributeType::IMAGELINK:return  TItemPropertyType::GetType(TItemPropertyType::PT_IMAGE);break;
			case TAttributeType::INTEGER:return  TItemPropertyType::GetType(TItemPropertyType::PT_INT);break;
			case TAttributeType::PASSWORD:return  TItemPropertyType::GetType(TItemPropertyType::PT_PASSWORD);break;
			case TAttributeType::REFERENCE:return  TItemPropertyType::GetType(TItemPropertyType::PT_REFERENCE);break;
			case TAttributeType::STRING:return  TItemPropertyType::GetType(TItemPropertyType::PT_STRING);break;
			case TAttributeType::TEXT:return  TItemPropertyType::GetType(TItemPropertyType::PT_MULTILINE);break;
			case TAttributeType::GUID:return TItemPropertyType::GetType(TItemPropertyType::PT_STRING);break;
			case TAttributeType::SET:return TItemPropertyType::GetType(TItemPropertyType::PT_SET);break;
		}
	} 
	
	public function AttributeDefinition(){
		return $this->attrDefinition;
	}	
}


/**
 * @package Components
 * @subpackage Orm
 * @category System
 * Orm item property class
 */
class TOrmAttributeProperty extends TOrmProperty {
/**
 * constructor
 * @param TOrmItem $i instance wrapped attribute belongs to
 * @param IAttributeDefinition $ca definition of wrapped attribute
 * @param array $sellist optional values selection list, associative array of values and titles
 * @see IProperty  
 */	
	public function __construct(TOrmItem $item, TOrmAttributePropertyMeta $meta){
		parent::__construct($item,$meta);
	}
	
	public function AttributeDefinition(){
		return $this->meta->AttributeDefinition();
	}
}

class TOrmReferencePropertyMeta extends TOrmAttributePropertyMeta implements IReferencePropertyMeta {
/**
 * @var array
 * stores fetched reference class items
 */	
	protected static $selections = array();
/**
 * constructor 
 */	
	public function __construct(IAttributeDefinition $ca){
		parent::__construct($ca,self::SelectionFromDefinition($ca));
	}
/**
 * gets a selection list from attribute definition, also stores it in selection array
 * @return array
 */	
	public static function SelectionFromDefinition(IAttributeDefinition $ca){
		if (!key_exists($ca->Type()->Class->Id(),self::$selections)){
			self::$selections[$ca->Type()->Class->Id()] = array();
			$ci = $ca->Type()->Class->ClassInstances();
			while ($ci->Next()){
				$wi = TOrmModel::GlobalWrap($ci->Item());
				if (!is_null($wi)) 
					self::$selections[$ca->Type()->Class->Id()][$wi->Instance()->Id()] = $wi->__toString();
			}
		}
		return self::$selections[$ca->Type()->Class->Id()];
	}	
/**
 * @see IReferenceProperty::ClassName()
 * @return string
 */	
	public function ClassName(){
		return $this->attrDefinition->Class->Name();
	}	
	
}

/**
 * @package Components
 * @subpackage Orm
 * @category System
 * orm item reference property class
 */
class TOrmReferenceProperty extends TOrmAttributeProperty implements IReferenceProperty {
/**
 * constructor 
 */	
	public function __construct(TOrmItem $item, TOrmReferencePropertyMeta $meta){
		parent::__construct($item,$meta);
	}
/**
 * @see IProperty::Value()
 * @return TOrmItem
 * @see TOrmItem
 */	
	public function ReferedInstance(){
		$dbi = new TDBInstance($this->Value(),$this->attrDefinition->Type()->Class);
		$dbi->Refresh();
		return TOrmModel::GlobalWrap($dbi);	
	}
} 

/**
 * @package Components
 * @subpackage Orm
 * @category System
 * Orm item class
 */
class TOrmItem extends TInstanceWrapper implements IItem {
	protected static $propertyMetas = array();
/**
 * @var array
 */	
	protected $properties;
/**
 * @var TOrmModel
 */	
	protected $component;

/**
 * gets item wrapped orm instance
 * @return IInstance
 */	
	public function Instance(){
		return $this->INSTANCE;
	}
		
/**
 * constructor
 * @param IInstance $i instance to wrap
 * @param TOrmModel $com orm component to interact with
 */	
	public function __construct(IInstance $i, TOrmModel $com){
		parent::__construct($i);
		$this->properties = array();
		$this->component = $com;
		if (!key_exists($this->ClassName(),self::$propertyMetas)){
			self::$propertyMetas[$this->ClassName()] = array();
			$ca = TOrmModel::ClassAttributeDefinitions($this->INSTANCE->InstanceClass());
			foreach ($ca as $a){
				if ($a->Type()->TypeCode == TAttributeType::REFERENCE)
					self::$propertyMetas[$this->ClassName()][$a->Name()] = new TOrmReferencePropertyMeta($a);
				else
					self::$propertyMetas[$this->ClassName()][$a->Name()] = new TOrmAttributePropertyMeta($a);
			}
		}
		
		foreach (self::$propertyMetas[$this->ClassName()] as $pm){
			if ($pm instanceof TOrmReferencePropertyMeta)
				$this->properties[$pm->Name()] = new TOrmReferenceProperty($this,$pm);
			else
				$this->properties[$pm->Name()] = new TOrmAttributeProperty($this,$pm);
		}
	}
/**
 * @see IItem::ItemId()
 * @return mixed
 */	
	public function ItemId(){return $this->Id();}
/**
 * @see IItem::ClassName()
 * @return string
 */	
	public function ClassName(){return $this->INSTANCE->InstanceClass()->Name();}
/**
 * @see IItem::__toString()
 * @return string
 */	 
	public function __toString(){return $this->INSTANCE->__toString();}
/**
 * @see IItem::Properties()
 * @return array
 */	
	public function Properties($includetypes = array(),$excludetypes = array()){
		$result = array();
		$itc = count($includetypes);
		foreach ($this->properties as $name=>$prop)
		   if ((($itc == 0) || (in_array($prop->Type(),$includetypes))) && (!in_array($prop->Type(),$excludetypes)))
		   	$result[$name] = $prop;
		return $result; 
	}
/**
 * @see IItem::Property()
 * @return TOrmProperty
 */	
	public function Property($name){
		if (key_exists($name,$this->properties))
			return $this->properties[$name];
		return null; 
	}	
/**
 * @see IItem::Delete()
 */	
	public function Delete(){
		if (parent::Delete()){
			$this->component->UnCache($this);
			return true;
		}
		return false;
	}
}

/**
 * @package Components
 * @subpackage Orm
 * @category System
 * iterator adapter to wrap orm instances with orm items  
 */
class TOrmIteratorAdapter extends TIteratorAdapter {
/**
 * @var TOrmModel 
 */	
	protected $component;
/**
 * constructor
 */	
	public function __construct(IIterator $iterator, TOrmModel $com){
		parent::__construct($iterator);
		$this->component = $com;
	}
/**
 * @see IIterator::Item()
 * @return TOrmItem
 * @see TOrmItem
 */	
	public function Item(){
		$result = null;
		$item = $this->component->WrapItem($this->iterator->Item);
		if ($item)
			$result = $this->component->Cache($item);
		return $result;
	}
}

/**
 * @property IAttributeDefinition $Attribute
 * @property mixed $Value
 * @property int $Operation
 */
class TOrmItemFilter {
	private $_attribute_definition_ = null;
	private $_value_ = null;
	private $_oper_ = TCondition::C_EQUAL;
	
	public function __construct(IAttributeDefinition $attribute, $value, $oper = TCondition::C_EQUAL){
		$this->_attribute_definition_ = $attribute;
		$this->_value_ = $value;
		$this->_oper_ = $oper;
	}
	
	public function __get($nm){
		switch ($nm){
			case "Attribute":return $this->_attribute_definition_;break;
			case "Value":return $this->_value_;break; 
			case "Operation":return $this->_oper_;break;
		}
	}
}

/**
 * @property IAttributeDefinition $Attribute
 * @property int $SortType
 */
class TOrmItemSorting {
	private $_attribute_definition_ = null;
	private $_sort_type_ = TSorting::SORT_ASC;
	
	public function __construct(IAttributeDefinition $attribute, $sorttype = TSorting::SORT_ASC){
		$this->_attribute_definition_ = $attribute;
		if (($sorttype == TSorting::SORT_ASC) || ($sorttype == TSorting::SORT_DESC))
			$this->_sort_type_ = $sorttype;
		else throw new TCoreException(TCoreException::ERR_BAD_VALUE);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Attribute":return $this->_attribute_definition_;break;
			case "SortType":return $this->_sort_type_;break; 
		}		
	}
}

/**
 * @package Components
 * @subpackage Orm
 * @category System
 * basic component to interact with orm storage
 * @property string $MetaFile name of file to use for installation. File must be located in application folder. 
 */
class TOrmModel extends TDataModel implements IPolicyProvider {
	protected static $ormModels = array();
/**
 * @var DOMDocument
 */	
	private $_dom_;

/**
 * @var array
 * orm items cache
 */	
	protected $loadedItems = array();
	
	protected function securityItem(TOrmItem $item){return $item;}
	
	protected function securityProperty(TOrmProperty $property){return $property;}
	
	protected function securityClass(TClass $class){return $class;}
		
	public function GetPolicy(ISecurityObject $object, $policycode){
		if ($object instanceof TOrmItem){
			if (TOrmItemRole::IsRole($policycode))
				return new TOrmItemRole($policycode,$this->securityItem($object));
			else	
				return new TOrmItemPermission($policycode,$this->securityItem($object));				
		} else if ($object instanceof TOrmProperty)
			return new TOrmPropertyPermission($policycode,$this->securityProperty($object));
		else if ($object instanceof TClass)
			return new TOrmClassPermission($policycode,$this->securityClass($object));
		return null;
	}

	public function SecurityWrap($object){
		if ($object instanceof TOrmItem)
			return $this->securityItem($object);
		else if ($object instanceof TOrmProperty)
			return $this->securityProperty($object);
		else if ($object instanceof TClass)
			return $this->securityClass($object);
		return null;
	}	
	
/**
 * constructor
 * @param string $name
 */	
	public function __construct($name){
		self::$ormModels[$name] = $this;
		parent::__construct($name);
	} 
	
	private function _get_meta(){
		$metapath = TApplication::$Application->PrivateDir."/orm-metas/";
		$filename = $this->MetaFile;
		if (!file_exists($filename))
			$filename = $metapath.$filename;
		if (!file_exists($filename))
			$filename = $metapath.$this->Name().".xml";	
		if (is_file($filename)){
			$this->_dom_ = new DOMDocument();
			$this->_dom_->preserveWhiteSpace = false;
			$this->_dom_->Load($filename);
			if (!$this->_dom_->schemaValidate(self::$SystemPath."/schemas/ormmeta.xsd"))
				throw new TOrmModelException(TOrmModelException::ERR_ORMMETA_INCORRECT);
			return new DOMXPath($this->_dom_);
		}	
		return null;
	}
/**
 * loads component xml metafile, parses it and performs installation 
 * @see IInstallable::Install()
 */	
	public function Install(){
		$meta = $this->_get_meta();
		if (is_null($meta)) return true;
		 
		$this->begin();	
	  	$metaitems = $meta->query("/meta/*");
	  	foreach ($metaitems as $mi){
	  		switch ($mi->tagName){
	  			case "class":{
	  				$parent = null;
	  				if (!is_null($mi->getAttribute("parent")))
	  					if ($mi->getAttribute("parent") != "")
	  						$parent = $mi->getAttribute("parent");
	  				if (!$this->dbDriver->IsClass($mi->getAttribute("name")))		
	  					$this->dbDriver->DefineClass($mi->getAttribute("name"),$mi->getAttribute("alias"),$parent);
	  				$classname = $mi->getAttribute("name");
	  				$attrdefs = $meta->query("attribute",$mi);
	  				foreach ($attrdefs as $ad){
	  					$type = TAttributeType::getType(strtoupper($ad->getAttribute("type")),$ad->getAttribute("size"),$ad->getAttribute("decimals"),$ad->getAttribute("unique"),$ad->getAttribute("index"),$ad->getAttribute("nullable"),$ad->getAttribute("default"),$ad->getAttribute("referto"),$ad->getAttribute("autogenerate"));
						$this->dbDriver->DefineAttribute($classname,$ad->getAttribute("name"),$ad->getAttribute("alias"), $type,$ad->getAttribute("static"));	  					
	  				}
	  			}break;
/*	  			case "association":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::ASSOCIATION)) $this->dbDriver->DefineStructure($mi->getAttribute("name"),$mi->getAttribute("alias"),TStructureType::ASSOCIATION,$mi->getAttribute("description"));break;
	  			case "agregation":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::AGREGATION))$this->dbDriver->DefineStructure($mi->getAttribute("name"),$mi->getAttribute("alias"),TStructureType::AGREGATION,$mi->getAttribute("description"));break;
	  			case "collection":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::COLLECTION))$this->dbDriver->DefineStructure($mi->getAttribute("name"),$mi->getAttribute("alias"),TStructureType::COLLECTION,$mi->getAttribute("description"));break;
	  			case "vocabulary":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::VOCABULARY))$this->dbDriver->DefineStructure($mi->getAttribute("name"),$mi->getAttribute("alias"),TStructureType::VOCABULARY,$mi->getAttribute("description"));break;
*/	  			
	  		}
	  	}
		$this->commit();
	  return true;
	}

/**
 * @see IInstallable::UnInstall()
 */	
	public function UnInstall(){
		$meta = $this->_get_meta();
		if (is_null($meta)) return true;
		$this->begin();	
	  	$metaitems = $meta->query("/meta/*");
	  	$classes = array();
	  	$structures = array();
	  	foreach ($metaitems as $mi){
	  		switch ($mi->tagName){
	  			case "class":$classes[] = $mi->getAttribute("name");break;
/*	  				case "association":
	  				case "agregation":
	  				case "collection":
	  				case "vocabulary":$structures[] = $mi->getAttribute("name");break;
*/	  				
	  		}
	  	}
	  		
/*	  		for ($i = count($structures); $i > 0; $i--)
	  			$this->dbDriver->UnDefineStructure($structures[$i]);*/
	  	for ($i = count($classes); $i > 0; $i--)
	  		$this->dbDriver->UnDefineClass($classes[$i]);
	  			
		$this->commit();
	  	return true;
	} 

/**
 * @see IInstallable::IsInstalled()
 * @return boolean
 */	
	public function IsInstalled(){
		$meta = $this->_get_meta();
		if (is_null($meta)) return true;
	  	$metaitems = $meta->query("/meta/*");
	  	foreach ($metaitems as $mi){
	  		switch ($mi->tagName){
	  			case "class":{
	  				$c = $this->dbDriver->getClass($mi->getAttribute("name"));
	  				if (is_null($c)) return false;
	  				$attrdefs = $meta->query("attribute",$mi);
	  				foreach ($attrdefs as $ad){
	  					$ca = $c->GetAttributeDefinition($ad->getAttribute("name"));
	  					if (is_null($ca)) return false;
	  					if ($ca->IsStatic() != $ad->getAttribute("static")) return false;
	  					$type = TAttributeType::getType(strtoupper($ad->getAttribute("type")),$ad->getAttribute("size"),$ad->getAttribute("decimals"),$ad->getAttribute("unique"),$ad->getAttribute("index"),$ad->getAttribute("nullable"),$ad->getAttribute("default"),$ad->getAttribute("referto"),$ad->getAttribute("autogenerate"));
	  					if ($ca->Type()->__toString() != $type->__toString()) return false;
	  				}
	  			}break;
	  		/*	
	  			case "association":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::ASSOCIATION)) return false;break;
	  			case "agregation":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::AGREGATION)) return false;break;
	  			case "collection":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::COLLECTION)) return false;break;
	  			case "vocabulary":if (!$this->dbDriver->IsStructureDefined($mi->getAttribute("name"),TStructureType::VOCABULARY)) return false;break;
	  		*/	
	  		}
	  	}
	  return true;	
	}
	
	private static function _class_attrs(IClass $c){
		$result = array_values($c->AttributeDefinitions());
		$p = $c->Ancestor();
		if (!is_null($p))
			$result = array_merge(self::_class_attrs($p),$result);
		return $result;
	}
	
	public static function ClassAttributeDefinitions(IClass $c){
		return self::_class_attrs($c);
	}
	
	public static function PropertiesByClass(IClass $class){
		$attributes = self::ClassAttributeDefinitions($class);
		$properties = array();
		foreach ($attributes as $a){
			if ($a->Type()->TypeCode == TAttributeType::REFERENCE)
				$properties[$a->Name()] = new TOrmReferencePropertyMeta($a);
			else
				$properties[$a->Name()] = new TOrmAttributePropertyMeta($a);
		}
		return $properties;
	}	
	
/**
 * @ignore
 */	
	public function __set($nm,$val){
		switch ($nm){
			case "DataSource":{
				TConfigurablePattern::SetIocMember($this->dbDriver, $val, 'IORMDriver');
				if ($this->dbDriver instanceof IORMDBDriver){
					if ($this->CheckInstall) {
						if (!$this->IsInstalled())
						throw new TInstallationException(TInstallationException::ERR_NOT_INSTALLED);
					}			
				}
			};break;
			default:parent::__set($nm,$val);break;
		}
	}
	
/**
 * gets orm class object by class name
 * @param string $className
 * @return IClass
 * @uses IORMDBDriver::getClass()
 */	
	public function GetClass($className){
		return $this->dbDriver->getClass($className);
	}
	
/**
 * gets orm class descenders by class name
 * @param string $className
 * @return IIterator iterator of IClass
 * @uses IORMDBDriver::ClassDescenders()
 */	
	public function ClassDescenders($className){
		return $this->dbDriver->ClassDescenders($className);
	}

/**
 * static function for wrapping orm instances with orm items globally.
 * it passes orm instance to WrapItem method of each existing orm component and returns first non null result.
 * Note that result depends on order of component loading. 
 * So you should load components that wrap items in any case (such as TOrmModel, because its WrapItem method always returns TOrmItem object) as late as possible.
 * @return TOrmItem
 * @uses TOrmModel::WrapItem()
 */	
	public static function GlobalWrap(IInstance $item){
		foreach (self::$ormModels as $name=>$ia)
			foreach ($ia as $com)
				if ($com instanceof TOrmModel)
				{
					$wi = $com->WrapItem($item);
					if (!is_null($wi)) return $wi;
				}
		return null; 
	} 

/**
 * function for wrapping orm instance with orm item.
 * @return TOrmItem
 */	
	public function WrapItem(IInstance $item){
		return new TOrmItem($item,$this);
	}
	
/**
 * caches orm item in component cache, to prevent redundant storage queries
 * @param TOrmItem $item item to cache
 * @param boolean $forcerewrite optional,defaults to false. When set to true rewrites already cached item with passes one.
 * @return TOrmItem returns cached item
 */	
	public function Cache(TOrmItem $item, $forcerewrite = false){
		if ($forcerewrite || !isset($this->loadedItems[$item->ItemId()]))
			$this->loadedItems[$item->ItemId()] = $item;
		return $this->loadedItems[$item->ItemId()];
	}
/**
 * wraps instance and cache resulting orm item
 * @param IInstance $item instance to wrap and cache
 * @param boolean $forcerewrite optional,defaults to false. When set to true rewrites already cached item with new one.
 * @return TOrmItem returns cached item
 */	
	protected function WrapNCache(IInstance $item, $forcerewrite = false){
		if ($i = $this->WrapItem($item))
			return $this->Cache($i,$forcerewrite);
		else
			return null;
	}
/**
 * deletes item from component cache
 */	
	public function UnCache(TOrmItem $item){
		unset($this->loadedItems[$item->ItemId()]);
	}
/**
 * creates orm item
 * @param IClass $c orm class of new item
 * @param array $parameters associative array of attribute values, where keys are attribute names
 * @return TOrmItem
 * @uses TOrmModel::WrapNCache
 */	
	public function CreateItem(IClass $c, array $parameters){
		$dbi = new TDBInstance(null,$c);
		foreach ($parameters as $key=>$value)
			$dbi->SetAttributeValue($key,$value,true);
		$dbi = $this->WrapNCache($dbi->Save(),true);
		return $dbi;
	}
/**
 * gets orm item by its id or code
 * @param mixed|IIdentity $identity item id
 * @return TOrmItem 
 */	
	public function GetItem($identity){
		$id = $identity;
		if ($identity instanceof IIdentity)
			$id = $identity->ItemId();
		$result = null;
		if (!key_exists($id,$this->loadedItems)){
			if ($instance = $this->dbDriver->Instanciate($id)){
				$result = $this->WrapNCache($instance);
				return $result;
			}
		}
		return $this->loadedItems[$id];
	}
/**
 * edits orm item
 * @param mixed $id orm item id
 * @param array $values associative array of new attribute values, keys are treated as attribute names
 * @return TOrmItem returns edited orm item 
 */	
	public function EditItem(IIdentity $item,array $values){
		if (!($item instanceof TOrmItem))
			$item = $this->GetItem($item);
		if ($item instanceof TOrmItem){	
			foreach ($values as $key=>$value)
				$item->__set($key,$value);
			return $this->WrapNCache($item->Instance()->Save(),true);
		}
		return false;
	}
/**
 * deleted orm item
 * @param mixed $id orm item id
 * @return boolean 
 */	
	public function DeleteItem(IIdentity $item){
		if (!($item instanceof TOrmItem))
			$item = $this->GetItem($item);
		if ($item instanceof TOrmItem)
			return $item->Delete();
		return false;
	}

/**
 * creates datasource according to specified options
 * @param IClass $class orm class
 * @param TOrmItemFilter[] $filter filtering conditions 
 * @param TOrmItemSorting[] $sorting array of sorting options
 * @return TOrmClassDataSource
 */	
	protected function makeDataSource(IClass $class, array $filters, array $sorting){
		$result = new TOrmClassDataSource($class, "c".$class->Id());
		$result->Fields = new TAllField($result);
		foreach ($filters as $f){
			$result->Filter = new TCondition(TCondition::C_EQUAL, array(new TOrmClassDataSourceAttribute($f->Attribute,null,$result),$f->Value));
		}
		foreach ($sorting as $s)
			$result->Sorting = new TSorting(new TOrmClassDataSourceAttribute($s->Attribute,null,$result),$s->SortType);
		return $result;		 
	}
	
/**
 * gets orm class items
 * @param IClass $class orm class
 * @param TOrmItemFilter[] $filter array of TCondition filtering conditions 
 * @param TOrmItemSorting[] $sorting array of TSorting results sorting
 * @return TOrmIteratorAdapter iterator of TOrmItem
 * @uses IORMDBDriver::FetchInstances()
 */	
	public function GetItems(IClass $class, array $filters, array $sorting){
		return new TOrmIteratorAdapter($this->dbDriver->FetchInstances($this->makeDataSource($class, $filters, $sorting)),$this);
	}

/**
 * starts transaction
 */	
	protected function begin(){
		$this->dbDriver->BeginTransaction();
		$this->startCapture('$this->rollback();');
	}
/**
 * commits transaction
 */	
	protected function commit(){
		$this->dbDriver->CommitTransaction();
		$this->stopCapture();
	}
/**
 * rollbacks transaction
 */	
	protected function rollback(){
		$this->dbDriver->RollbackTransaction();
	}
	
	public function FilePropertyContent(IIdentity $item,$name){
		if (!($item instanceof TOrmItem))
			$item = $this->GetItem($item);
		if ($item instanceof TOrmItem){
			$p = $item->Property($name);
			$dom = new DOMDocument();
			$dom->loadXML($p->Value());
			$xpath = new DOMXPath($dom);
			$fn = $xpath->query("/file/header/name");
			$mime = "";
			if ($fn->length > 0){
				if ($fn->item(0)->nodeValue != "") 
					$mime = mime_content_type($fn->item(0)->nodeValue);
			}
			if ($mime == ""){
				$mt = $xpath->query("/file/header/mime");
				if ($mt->length > 0) 
					$mime = $mt->item(0)->nodeValue;
			}
			header("Content-Type:".$mime);
			$content = $xpath->query("/file/content");
			if ($content->length > 0) {
				echo $content->item(0)->nodeValue;
			}
		}
		die;
	}	
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
 * @param TOrmItemFilter[] $filter
 * @param TOrmItemSorting[] $sorting
 * @return TOrmIteratorAdapter
 */	
	public function CollectionItems($collection, IClass $class, array $filter = array(), array $sorting = array());
}

interface IVocabulariesManager {
/**
 * Adds an association of orm item to specified key to vocabulary
 * @param string $vocabulary
 * @param mixed $key
 * @param IIdentity $item
 * @return boolean
 */	
	public function AddToVocabulary($vocabulary, $key, IIdentity $item);
/** 
 * Removes orm item association from vocabulary
 * @param string $vocabulary
 * @param mixed $key
 * @param IIdentity $item
 * @return boolean
 */	
	public function DeleteFromVocabulary($vocabulary, $key, IIdentity $item);
/** 
 * Fetches orm items associated with a specified key from a vocabulary
 * @param string $vocabulary
 * @param IClass $class
 * @param mixed $key
 * @param TOrmItemFilter[] $filter
 * @param TOrmItemSorting[] $sorting
 * @return TOrmIteratorAdapter
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
 * @param TOrmItemFilter[] $filter
 * @param TOrmItemSorting[] $sorting
 * @param int $direction relations direction set by FETCH_OBJECTS or FETCH_SUBJECTS constant
 * @return TOrmIteratorAdapter
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
 * @param TOrmItemFilter[] $filter
 * @param TOrmItemSorting[] $sorting
 * @return TOrmIteratorAdapter
 */
	public function FetchChildren($structure, IClass $class, IIdentity $parent = null, $only_direct = false, array $filter = array(), array $sorting = array());
/** 
 * Gets item direct container
 * @param string $structure
 * @param IIdentity $item
 * @return TOrmItem
 */	
	public function ItemContainer($structure, IIdentity $item);
/**
 * 
 * Gets item containers list
 * @param string $structure
 * @param IIdentity $item
 * @return TOrmIteratorAdapter
 */	
	public function ItemContainers($structure, IIdentity $item);
}
?>