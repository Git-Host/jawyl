<?php
require_once 'common/enum.inc';

/**
 * @package Common
 * @subpackage Items
 * @category System
 * item property types class.	 
 */	
	final class TItemPropertyType extends TEnumeration {
		const PT_STRING = "str";
		const PT_INT = "int";
		const PT_DEC = "dec";
		const PT_MULTILINE = "mline";
		const PT_HTML = "html";
		const PT_DATE = "date";
		const PT_FILE = "file";
		const PT_IMAGE = "image";
		const PT_BOOL = "bool";
		const PT_PASSWORD = "pwd";
		const PT_REFERENCE = "ref";
		const PT_SET = "set";
		const PT_COLLECTION = "collection";
	}
	
	interface IPropertyMeta {
/**
 *  gets property name
 * @return string
 */		
		public function Name();
/**
 * gets property caption
 * @return string
 */		
		public function Caption();
		
		public function SetCaption($caption);
/**
 * gets property type. should return one of TItemPropertyType constants
 * @return TItemPropertyType
 * @see TItemPropertyType
 */		
		public function Type();
/**
 * gets property value selection list
 * @return array
 */		
		public function Selection();
/**
 * sets property value selection list
 * @param array|callable $selection
 */		
		public function SetSelection($selection);
/**
 * @return bool
 */		
		public function ReadOnly();
/**
 * @return bool
 */		
		public function Nullable();
/**
 * @return bool
 */		
		public function Required();
	}

/**
 * @package Common
 * @subpackage Items
 * @category System
 * interface for implementing  item properties
 */	
	interface IProperty extends IPropertyMeta {
/**
 * gets property value
 * @return mixed
 */		
		public function Value();
		
/**
 * gets property display value
 * @return mixed
 */		
		public function DisplayValue();
/**
 * gets property item
 * @return IItem
 */		
		public function Item();
	}
	
	interface IReferencePropertyMeta extends IPropertyMeta {
/**
 * gets reference class name
 * @return string
 */		
		public function ClassName();	
	}	
	
/**
 * @package Common
 * @subpackage Items
 * @category System
 * interface for implementing reference properties
 */	
	interface IReferenceProperty extends IProperty {		
/**
 * gets instance property refers to
 * @return IIdentity
 */		
		public function ReferedInstance();
	}

	interface IIdentity {
/**
 * gets unique item id
 * @return mixed
 */		
		public function ItemId();
	}
	
	class TIdentity implements ISecurityObject {
		private $_item_id_ = null;
		public function __construct($id){
			$this->_item_id_ = $id;
		}
		public function ItemId(){
			return $this->_item_id_;
		}
		public function __toString(){
			return $this->ItemId();
		}
		public function Soid(){
			return $this->ItemId();
		}
	}
	
/**
 * @package Common
 * @subpackage Items
 * @category System
 * interface for implementing items 
 */	
	interface IItem extends IIdentity {
/**
 * gets item class name
 * @return string
 */		
		public function ClassName();
/**
 * gets string representation of item
 * @return string
 */		
		public function __toString();
/**
 * gets item properties
 * @param string[] $includetypes array of property types, which properties should be included
 * @param string[] $excludetypes array of property types, which properties should be excluded
 * @return IProperty[]
 * @see TItemPropertyType
 * @see IProperty
 */		
		public function Properties($includetypes = array(),$excludetypes = array());
/**
 * gets item property
 * @param string $name property name
 * @return IProperty
 * @see IProperty
 */		
		public function Property($name);
	}
	
	class TPropertyMeta implements IPropertyMeta {
		protected $name;
		protected $caption;
		protected $type;
		protected $selection;
		protected $nullable;
		protected $readonly;
		protected $required;
		
		public function __construct($name,$caption,$type,array $selection = array(),$nullable = true,$readonly = false,$required = false){
			$this->name = $name;
			$this->caption = $caption;
			$this->type = new TItemPropertyType($type);
			$this->selection = $selection;
			$this->nullable = $nullable;
			$this->readonly = $readonly;
			$this->required = $required;			
		}
/**
 *  gets property name
 * @return string
 */		
		public function Name(){
			return $this->name;
		}
/**
 * gets property caption
 * @return string
 */		
		public function Caption(){
			return $this->caption;
		}
		
		public function SetCaption($caption){
			$this->caption = $caption;
		}
/**
 * gets property type. should return one of TItemPropertyType constants
 * @return TItemPropertyType
 * @see TItemPropertyType
 */		
		public function Type(){
			return $this->type;
		}
/**
 * gets property value selection list
 * @return array
 */		
		public function Selection(){
			return $this->selection;
		}
/**
 * sets property value selection list
 * @param array $selection
 */		
		public function SetSelection($selection){
			$this->selection = $selection;
		}
		
		public function ReadOnly(){
			return $this->readonly;
		}
		
		public function Nullable(){
			return $this->nullable;
		}		
		
		public function Required(){
			return $this->required;
		}		
	}

	abstract class TProperty implements IProperty {
		protected $meta;
		
		public function __construct(IPropertyMeta $meta){
			$this->meta = $meta;
		}
/**
 *  gets property name
 * @return string
 */		
		public function Name(){return $this->meta->Name();}
		
		public function DisplayValue(){
			if (($s = $this->Selection()) && !is_array($this->Value())){
				if (isset($s[$this->Value()]))
					return $s[$this->Value()];
			}
			return $this->Value();
		}
/**
 * gets property caption
 * @return string
 */		
		public function Caption(){return $this->meta->Caption();}
		
		public function SetCaption($caption){$this->meta->SetCaption($caption);}
/**
 * gets property type. should return one of TItemPropertyType constants
 * @return TItemPropertyType
 * @see TItemPropertyType
 */		
		public function Type(){return $this->meta->Type();}
/**
 * gets property value selection list
 * @return array
 */		
		public function Selection(){return $this->meta->Selection();}
/**
 * sets property value selection list
 * @param array $selection
 */		
		public function SetSelection($selection){$this->meta->SetSelection($selection);}
		
		public function ReadOnly(){return $this->meta->ReadOnly();}
		
		public function Nullable(){return $this->meta->Nullable();}	
			
		public function Required(){return $this->meta->Required();}
	} 
	
	abstract class TReferenceProperty extends TProperty implements IReferenceProperty, IReferencePropertyMeta {
		public function __construct(IReferencePropertyMeta $meta){
			parent::__construct($meta);
		}
		
/**
 * gets reference class name
 * @return string
 */		
		public function ClassName(){
			return $this->meta->ClassName();
		}		
	}
?>