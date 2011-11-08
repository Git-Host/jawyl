<?php

/**
 * @package Common
 * @subpackage Items
 * @category System
 * Orm item property types class.	 
 */	
	final class TItemPropertyType {
		private $_code_;
		
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
/**
 * gets list of orm property types
 * @return array
 */		
		public static function TypeList(){
			return array(
				self::PT_STRING,
				self::PT_INT,
				self::PT_DEC,
				self::PT_DATE,
				self::PT_MULTILINE,
				self::PT_HTML,
				self::PT_FILE,
				self::PT_IMAGE,
				self::PT_BOOL,
				self::PT_PASSWORD,
				self::PT_REFERENCE,
				self::PT_SET
			);
		}

		public function __construct($code){
			if (!in_array($code,self::TypeList()))
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			$this->_code_ = $code;
		}
		
		public static function GetType($code){return new TItemPropertyType($code);}
		
		public function __toString(){return $this->_code_;}
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
 * @param array $selection
 */		
		public function SetSelection(array $selection);
		
		public function ReadOnly();
		
		public function Nullable();		
	}

/**
 * @package Common
 * @subpackage Items
 * @category System
 * interface for implementing orm item properties
 */	
	interface IProperty extends IPropertyMeta {
/**
 * gets property value
 * @return mixed
 */		
		public function Value();
/**
 * gets property item
 */		
		public function Item();
	}
	
	interface IReferencePropertyMeta extends IPropertyMeta {
/**
 * gets reference class name
 * @return string
 */		
		public function ClassName();
/**
 * gets instance property refers to
 * @return IInstance
 */		
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
 * @return IInstance
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
 * interface for implementing orm items 
 */	
	interface IItem extends IIdentity {
/**
 /**
 * gets item orm class name
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
 * @param string[] $excludedtypes array of property types, which properties should be excluded
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
		public function SetSelection(array $selection){$this->meta->SetSelection($selection);}
		
		public function ReadOnly(){return $this->meta->ReadOnly();}
		
		public function Nullable(){return $this->meta->Nullable();}		
		
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