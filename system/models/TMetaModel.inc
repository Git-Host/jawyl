<?php
/**
 * @package Components
 * @subpackage Meta
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


TApplication::import("common::metastorage");
TApplication::import("common::item"); 
TApplication::import("models::TDataModel");

class TMetaClassPermission extends TPolicy {
	const META_INSTANCIATE = "meta.instant";
	const META_INSTLIST = "meta.instlist";
	
	protected function checkType($type){
		return ($this->sobj instanceof TClass) && (in_array($type, array(self::META_INSTANCIATE,self::META_INSTLIST)));
	}
	
	public function Descendants(){
		switch ($this->type){
			case self::META_INSTANCIATE:
			case self::META_INSTLIST:return array(new TServiceRole(TServiceRole::USER));break;
		}
		return array();
	}	
}

class TMetaItemRole extends TPolicy {
	const META_OWNER = "meta.owner";
	
	protected function checkType($type){
		return ($this->sobj instanceof TMetaItem) && ($type == self::META_OWNER);
	}
	
	public function Descendants(){
		switch ($this->type){
			case self::META_OWNER:return array(new TServiceRole(TServiceRole::ADMIN));break;
		}
		return array();
	}
/**
 * checks whether specified policy is role  
 * @param string $type
 * @return boolean
 */
	public static function IsRole($policy){
		return $policy == self::META_OWNER;
	}
}

class TMetaItemPermission extends TPolicy {
	const META_DELETE = "meta.delete";
	const META_RELATE = "meta.relate";
	const META_LIST = "meta.list";
	const META_VIEW = "meta.view";
	const META_MODIFY = "meta.modify";
	const META_ACL = "meta.acl";

	protected function checkType($type){
		return ($this->sobj instanceof TMetaItem) && (in_array($type, array(self::META_ACL,self::META_DELETE,self::META_LIST,self::META_MODIFY,self::META_RELATE,self::META_VIEW)));
	}
	
	public function Descendants(){
		switch ($this->type){
			case self::META_ACL:
			case self::META_DELETE:
			case self::META_MODIFY:return array(new TServiceRole(TServiceRole::ADMIN),new TMetaItemRole(TMetaItemRole::META_OWNER,$this->sobj));break;
			case self::META_RELATE:return array(new TServiceRole(TServiceRole::USER),new TMetaItemRole(TMetaItemRole::META_OWNER,$this->sobj));break;
			case self::META_VIEW:
			case self::META_LIST:return array(new TServiceRole(TServiceRole::ANONYMOUS),new TMetaItemRole(TMetaItemRole::META_OWNER,$this->sobj));break;
		}
		return array();
	}	
}

class TMetaPropertyPermission extends TPolicy {
	const META_READ = "meta.read";
	const META_WRITE = "meta.write";
	
	protected function checkType($type){
		return ($this->sobj instanceof TMetaProperty) && (in_array($type, array(self::META_READ,self::META_WRITE)));
	}
	
	public function Descendants(){
		$result = array(new TServiceRole(TServiceRole::USER));
		$item = $this->sobj->Item();
		switch ($this->type){
			case self::META_READ:$result[] = new TMetaItemPermission(TMetaItemPermission::META_VIEW,$item);break;
			case self::META_WRITE:$result[] = new TMetaItemPermission(TMetaItemPermission::META_MODIFY,$item);break;
		}
		return $result;
	}
}

/**
 * @package Components
 * @subpackage Meta
 * @category System
 * Meta component exception class
 */
class TMetaModelException extends TMetaException {
	const ERR_META_INCORRECT = 801;
/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_META_INCORRECT:return 'Model meta file incorrect!';break;
			default:return parent::getMessageText($msgcode);break;
		}
	}
}

class TMetaPropertyMeta implements IPropertyMeta {
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
			$this->selection = $selection;
	}
/**
 * @see IPropertyMeta::Selection()
 * @return array
 */	
	public function Selection(){
		if (!is_array($this->selection))
			$this->selection = array();
		return $this->selection;
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
 * @subpackage Meta
 * @category System
 * Meta item property class
 */
class TMetaProperty extends TProperty {
/**
 * @var TMetaItem  
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
	public function __construct(TMetaItem $item, TMetaPropertyMeta $meta){
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

class TMetaAttributePropertyMeta extends TMetaPropertyMeta {
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
 * @subpackage Meta
 * @category System
 * Meta item property class
 */
class TMetaAttributeProperty extends TMetaProperty {
/**
 * constructor
 * @param TMetaItem $i instance wrapped attribute belongs to
 * @param IAttributeDefinition $ca definition of wrapped attribute
 * @param array $sellist optional values selection list, associative array of values and titles
 * @see IProperty  
 */	
	public function __construct(TMetaItem $item, TMetaAttributePropertyMeta $meta){
		parent::__construct($item,$meta);
	}
	
	public function AttributeDefinition(){
		return $this->meta->AttributeDefinition();
	}
}

class TMetaReferencePropertyMeta extends TMetaAttributePropertyMeta implements IReferencePropertyMeta {
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
			foreach ($ci as $i){
				$wi = TMetaModel::GlobalWrap($i);
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
 * @subpackage Meta
 * @category System
 * Meta item reference property class
 */
class TMetaReferenceProperty extends TMetaAttributeProperty implements IReferenceProperty {
/**
 * constructor 
 */	
	public function __construct(TMetaItem $item, TMetaReferencePropertyMeta $meta){
		parent::__construct($item,$meta);
	}
/**
 * @see IProperty::Value()
 * @return TMetaItem
 * @see TMetaItem
 */	
	public function ReferedInstance(){
		$dbi = new TDBInstance($this->Value(),$this->attrDefinition->Type()->Class);
		$dbi->Refresh();
		return TMetaModel::GlobalWrap($dbi);	
	}
} 

/**
 * @package Components
 * @subpackage Meta
 * @category System
 * Meta item class
 */
class TMetaItem extends TInstanceWrapper implements IItem {
	protected static $propertyMetas = array();
/**
 * @var array
 */	
	protected $properties;
/**
 * @var TMetaModel
 */	
	protected $component;

/**
 * gets item wrapped Meta instance
 * @return IInstance
 */	
	public function Instance(){
		return $this->INSTANCE;
	}
		
/**
 * constructor
 * @param IInstance $i instance to wrap
 * @param TMetaModel $com Meta component to interact with
 */	
	public function __construct(IInstance $i, TMetaModel $com){
		parent::__construct($i);
		$this->properties = array();
		$this->component = $com;
		if (!key_exists($this->ClassName(),self::$propertyMetas)){
			self::$propertyMetas[$this->ClassName()] = array();
			$ca = TMetaModel::ClassAttributeDefinitions($this->INSTANCE->InstanceClass());
			foreach ($ca as $a){
				if ($a->Type()->TypeCode == TAttributeType::REFERENCE)
					self::$propertyMetas[$this->ClassName()][$a->Name()] = new TMetaReferencePropertyMeta($a);
				else
					self::$propertyMetas[$this->ClassName()][$a->Name()] = new TMetaAttributePropertyMeta($a);
			}
		}
		
		foreach (self::$propertyMetas[$this->ClassName()] as $pm){
			if ($pm instanceof TMetaReferencePropertyMeta)
				$this->properties[$pm->Name()] = new TMetaReferenceProperty($this,$pm);
			else
				$this->properties[$pm->Name()] = new TMetaAttributeProperty($this,$pm);
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
 * @return TMetaProperty
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
 * @subpackage Meta
 * @category System
 * iterator adapter to wrap meta instances with meta items  
 */
class TMetaIteratorAdapter extends TIteratorAdapter {
/**
 * @var TMetaModel 
 */	
	protected $component;
/**
 * constructor
 */	
	public function __construct(IIterator $iterator, TMetaModel $com){
		parent::__construct($iterator);
		$this->component = $com;
	}
/**
 * @see IIterator::Item()
 * @return TMetaItem
 * @see TMetaItem
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
class TMetaItemFilter {
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
class TMetaItemSorting {
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
 * @subpackage Meta
 * @category System
 * basic component to interact with Meta storage
 * @property string $MetaFile name of file to use for installation. File must be located in application folder. 
 */
class TMetaModel extends TDataModel implements IPolicyProvider {
	protected static $metaModels = array();
/**
 * @var DOMDocument
 */	
	private $_dom_;

/**
 * @var array
 * items cache
 */	
	protected $loadedItems = array();
	
	protected function securityItem(TMetaItem $item){return $item;}
	
	protected function securityProperty(TMetaProperty $property){return $property;}
	
	protected function securityClass(TClass $class){return $class;}
		
	public function GetPolicy(ISecurityObject $object, $policycode){
		if ($object instanceof TMetaItem){
			if (TMetaItemRole::IsRole($policycode))
				return new TMetaItemRole($policycode,$this->securityItem($object));
			else	
				return new TMetaItemPermission($policycode,$this->securityItem($object));				
		} else if ($object instanceof TMetaProperty)
			return new TMetaPropertyPermission($policycode,$this->securityProperty($object));
		else if ($object instanceof TClass)
			return new TMetaClassPermission($policycode,$this->securityClass($object));
		return null;
	}

	public function SecurityWrap($object){
		if ($object instanceof TMetaItem)
			return $this->securityItem($object);
		else if ($object instanceof TMetaProperty)
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
		self::$metaModels[$name] = $this;
		parent::__construct($name);
	} 
	
	private function _get_meta(){
		$metapath = TApplication::$Application->PrivateDir."/metas/";
		$filename = $this->MetaFile;
		if (!file_exists($filename))
			$filename = $metapath.$filename;
		if (!file_exists($filename))
			$filename = $metapath.$this->Name().".xml";	
		if (is_file($filename)){
			$this->_dom_ = new DOMDocument();
			$this->_dom_->preserveWhiteSpace = false;
			$this->_dom_->Load($filename);
			if (!$this->_dom_->schemaValidate(self::$SystemPath."/schemas/meta.xsd"))
				throw new TMetaModelException(TMetaModelException::ERR_META_INCORRECT);
			return new DOMXPath($this->_dom_);
		}	
		return null;
	}
/**
 * loads component xml metafile, parses it and performs installation 
 * @see IInstallable::Install()
 */	
	public function Install(){
/*
 * @todo table and nested sets definition
 */		
		$meta = $this->_get_meta();
		if (is_null($meta)) return true;
		 
		$this->begin();
		try {	
	  		$metaitems = $meta->query("/meta/*");
	  		foreach ($metaitems as $mi){
	  			switch ($mi->tagName){
	  				case "class":{
	  					$parent = null;
	  					if (!is_null($mi->getAttribute("parent")))
	  						if ($mi->getAttribute("parent") != "")
	  							$parent = $this->dbDriver->GetClass($mi->getAttribute("parent"));
	  					if (!$c = $this->dbDriver->GetClass($mi->getAttribute("name"))){		
	  						$this->dbDriver->Define(new TDBClass($mi->getAttribute("name"),$parent));
	  						$c = $this->dbDriver->GetClass($mi->getAttribute("name"));
	  					}
	  					$classname = $mi->getAttribute("name");
	  					$attrdefs = $meta->query("attribute",$mi);
	  					foreach ($attrdefs as $ad){
	  						if (strtoupper($ad->getAttribute("type")) != TMetaReferenceType::META_REFERENCE)
	  							$type = TDataType::getType(strtoupper($ad->getAttribute("type")),$ad->getAttribute("size"),$ad->getAttribute("decimals"),TConvertions::ConvertToBoolean($ad->getAttribute("autogenerate")));
	  						else 
	  							$type = new TMetaReferenceType($this->dbDriver->GetClass($ad->getAttribute("refers")),TConvertions::ConvertToBoolean($ad->getAttribute("integritydelete")));	
							$this->dbDriver->Define(new TAttributeDefinition($ad->getAttribute("name"), $type,$c,TConvertions::ConvertToBoolean($ad->getAttribute("static")),TConvertions::ConvertToBoolean($ad->getAttribute("unique")),TConvertions::ConvertToBoolean($ad->getAttribute("index")),TConvertions::ConvertToBoolean($ad->getAttribute("nullable")),$ad->getAttribute("default"),TConvertions::ConvertToBoolean($ad->getAttribute("periodic"))));	  					
	  					}
	  				}break;
		  		}
		  	}
			$this->commit();
		} catch (Exception $e){
			$this->rollback();
			throw $e;
		}
	  return true;
	}

/**
 * @see IInstallable::UnInstall()
 */	
	public function UnInstall(){
		$meta = $this->_get_meta();
		if (is_null($meta)) return true;
		$this->begin();	
		try {
	  		$metaitems = $meta->query("/meta/*");
	  		$classes = array();
	  		$structures = array();
	  		foreach ($metaitems as $mi){
	  			switch ($mi->tagName){
	  				case "class":$classes[] = $mi->getAttribute("name");break;
	  			}
	  		}
	  		
	  		for ($i = count($classes); $i > 0; $i--)
	  			$this->dbDriver->Undefine($classes[$i]);
	  			
			$this->commit();
		} catch (Exception $e){
			$this->rollback();
			throw $e;
		}
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
	  				$c = $this->dbDriver->GetClass($mi->getAttribute("name"));
	  				if (is_null($c)) return false;
	  				$attrdefs = $meta->query("attribute",$mi);
	  				foreach ($attrdefs as $ad){
	  					$ca = $c->GetAttributeDefinition($ad->getAttribute("name"));
	  					if (is_null($ca)) return false;
	  					if ($ca->IsStatic() != $ad->getAttribute("static")) return false;
	  					if ($ca->Type()->__toString() != strtoupper($ad->getAttribute("type"))) return false;
	  				}
	  			}break;
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
				$properties[$a->Name()] = new TMetaReferencePropertyMeta($a);
			else
				$properties[$a->Name()] = new TMetaAttributePropertyMeta($a);
		}
		return $properties;
	}	
	
/**
 * @ignore
 */	
	public function __set($nm,$val){
		switch ($nm){
			case "DataSource":{
				$this->setIocMember($this->dbDriver, $val, 'IMetaDriver');
				if ($this->dbDriver instanceof IMetaDBDriver){
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
 * gets class object by class name
 * @param string $className
 * @return IClass
 * @uses IMetaDBDriver::getClass()
 */	
	public function GetClass($className){
		return $this->dbDriver->getClass($className);
	}
	
/**
 * gets class descendants by class name
 * @param string $className
 * @return IIterator iterator of IClass
 * @uses IMetaDBDriver::ClassDescendants()
 */	
	public function ClassDescendants($className){
		return $this->dbDriver->ClassDescendants($className);
	}

/**
 * static function for wrapping instances with items globally.
 * it passes instance to WrapItem method of each existing meta-component and returns first non null result.
 * Note that result depends on order of component loading. 
 * So you should load components that wrap items in any case (such as TMetaModel, because its WrapItem method always returns TMetaItem object) as late as possible.
 * @return TMetaItem
 * @uses TMetaModel::WrapItem()
 */	
	public static function GlobalWrap(IInstance $item){
		foreach (self::$metaModels as $name=>$ia)
			foreach ($ia as $com)
				if ($com instanceof TMetaModel)
				{
					$wi = $com->WrapItem($item);
					if (!is_null($wi)) return $wi;
				}
		return null; 
	} 

/**
 * function for wrapping instance with item.
 * @return TMetaItem
 */	
	public function WrapItem(IInstance $item){
		return new TMetaItem($item,$this);
	}
	
/**
 * caches item in component cache, to prevent redundant storage queries
 * @param TMetaItem $item item to cache
 * @param boolean $forcerewrite optional,defaults to false. When set to true rewrites already cached item with passes one.
 * @return TMetaItem returns cached item
 */	
	public function Cache(TMetaItem $item, $forcerewrite = false){
		if ($forcerewrite || !isset($this->loadedItems[$item->ItemId()]))
			$this->loadedItems[$item->ItemId()] = $item;
		return $this->loadedItems[$item->ItemId()];
	}
/**
 * wraps instance and cache resulting item
 * @param IInstance $item instance to wrap and cache
 * @param boolean $forcerewrite optional,defaults to false. When set to true rewrites already cached item with new one.
 * @return TMetaItem returns cached item
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
	public function UnCache(TMetaItem $item){
		unset($this->loadedItems[$item->ItemId()]);
	}
/**
 * creates item
 * @param IClass $c class of new item
 * @param array $parameters associative array of attribute values, where keys are attribute names
 * @return TMetaItem
 * @uses TMetaModel::WrapNCache
 */	
	public function CreateItem(IClass $c, array $parameters){
		$dbi = new TDBInstance(null,$c);
		foreach ($parameters as $key=>$value)
			$dbi->SetAttributeValue($key,$value,true);
		$dbi = $this->WrapNCache($dbi->Save(),true);
		return $dbi;
	}
/**
 * gets item by its id or code
 * @param mixed|IIdentity $identity item id
 * @return TMetaItem 
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
 * edits item
 * @param mixed $id item id
 * @param array $values associative array of new attribute values, keys are treated as attribute names
 * @return TMetaItem returns edited item 
 */	
	public function EditItem(IIdentity $item,array $values){
		if (!($item instanceof TMetaItem))
			$item = $this->GetItem($item);
		if ($item instanceof TMetaItem){	
			foreach ($values as $key=>$value)
				$item->__set($key,$value);
			return $this->WrapNCache($item->Instance()->Save(),true);
		}
		return false;
	}
/**
 * deleted item
 * @param mixed $id item id
 * @return boolean 
 */	
	public function DeleteItem(IIdentity $item){
		if (!($item instanceof TMetaItem))
			$item = $this->GetItem($item);
		if ($item instanceof TMetaItem)
			return $item->Delete();
		return false;
	}

/**
 * creates datasource according to specified options
 * @param IClass $class class
 * @param TMetaItemFilter[] $filter filtering conditions 
 * @param TMetaItemSorting[] $sorting array of sorting options
 * @return TMetaClassDataSource
 */	
	protected function makeDataSource(IClass $class, array $filters, array $sorting){
		$result = new TMetaClassDataSource($class, "c".$class->Id());
		$result->Fields = new TAllField($result);
		foreach ($filters as $f){
			$result->Filter = new TCondition(TCondition::C_EQUAL, array(new TMetaClassDataSourceAttribute($f->Attribute,null,$result),$f->Value));
		}
		foreach ($sorting as $s)
			$result->Sorting = new TSorting(new TMetaClassDataSourceAttribute($s->Attribute,null,$result),$s->SortType);
		return $result;		 
	}
	
/**
 * gets class items
 * @param IClass $class class
 * @param TMetaItemFilter[] $filter array of TCondition filtering conditions 
 * @param TMetaItemSorting[] $sorting array of TSorting results sorting
 * @return TMetaIteratorAdapter iterator of TMetaItem
 * @uses IMetaDBDriver::FetchInstances()
 */	
	public function GetItems(IClass $class, array $filters, array $sorting){
		return new TMetaIteratorAdapter($this->dbDriver->FetchInstances($this->makeDataSource($class, $filters, $sorting)),$this);
	}

/**
 * starts transaction
 */	
	protected function begin(){
		$this->dbDriver->BeginTransaction();
		//$this->startCapture('$this->rollback();');
	}
/**
 * commits transaction
 */	
	protected function commit(){
		$this->dbDriver->CommitTransaction();
		//$this->stopCapture();
	}
/**
 * rollbacks transaction
 */	
	protected function rollback(){
		$this->dbDriver->RollbackTransaction();
	}
	
	public function FilePropertyContent(IIdentity $item,$name){
		if (!($item instanceof TMetaItem))
			$item = $this->GetItem($item);
		if ($item instanceof TMetaItem){
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
 * @param TMetaItemFilter[] $filter
 * @param TMetaItemSorting[] $sorting
 * @return TMetaIteratorAdapter
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
 * @param TMetaItemFilter[] $filter
 * @param TMetaItemSorting[] $sorting
 * @return TMetaIteratorAdapter
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
 * @param TMetaItemFilter[] $filter
 * @param TMetaItemSorting[] $sorting
 * @param int $direction relations direction set by FETCH_OBJECTS or FETCH_SUBJECTS constant
 * @return TMetaIteratorAdapter
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
 * @param TMetaItemFilter[] $filter
 * @param TMetaItemSorting[] $sorting
 * @return TMetaIteratorAdapter
 */
	public function FetchChildren($structure, IClass $class, IIdentity $parent = null, $only_direct = false, array $filter = array(), array $sorting = array());
/** 
 * Gets item direct container
 * @param string $structure
 * @param IIdentity $item
 * @return TMetaItem
 */	
	public function ItemContainer($structure, IIdentity $item);
/**
 * 
 * Gets item containers list
 * @param string $structure
 * @param IIdentity $item
 * @return TMetaIteratorAdapter
 */	
	public function ItemContainers($structure, IIdentity $item);
}
?>