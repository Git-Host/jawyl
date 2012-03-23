<?php
/**
 * @package Common
 * @subpackage Data
 * @category System
 * abstract base class for data types
 */
	abstract class TDataType {
		const STRING = "STRING";
		const DATETIME = "DATETM";
		const BOOLEAN = "BOOL";
		const INTEGER = "INT";
		const BIGINT = "BIGINT";
		const FLOAT = "FLOAT";
		const DECIMAL = "DEC";
		const TEXT = "TEXT";
		const HTML = "HTML";
		const IMAGE = "IMAGE";
		const FILE = "FILE";
		const FILELINK = "FILLNK";
		const IMAGELINK = "IMGLNK";
		const PASSWORD = "PWD";
		const GUID = "GUID";
		const SET = "SET";
		const TIMESTAMP = "TIME";
		
/**
 * @var string unique string code of type, should be one of TDataType constants
 */		
		protected $typeCode;
		
		public function __get($nm){
			switch ($nm){
				case "TypeCode":return $this->typeCode;break;
				default:return null;break;
			}
		} 

/**
 * string representation of type
 * @return string
 */		
		public function __toString(){
			return "type=".$this->typeCode;
		} 
		
		public static function GetType($type, $size = null, $decimals = null, $autogenerate = false){
			switch ($type) {
				case self::STRING:return new TStringType($size);break;
				case self::DATETIME:return new TDateTimeType();break;
				case self::BOOLEAN:return new TBooleanType();break;
				case self::INTEGER:return new TIntegerType($size);break;
				case self::BIGINT:return new TLongType($size);break;
				case self::FLOAT:return new TFloatType($size,$decimals);break;
				case self::DECIMAL:return new TDecimalType($size,$decimals);break;
				case self::TEXT:return new TTextType();break;
				case self::HTML:return new THTMLType();break;
				case self::IMAGE:return new TImageType();break;
				case self::FILE:return new TFileType();break;
				case self::FILELINK:return new TFileLinkType($size);break;
				case self::IMAGELINK:return new TImageLinkType($size);break;
				case self::PASSWORD:return new TPasswordType($size);break;
				case self::GUID:return new TGuidType($size,$autogenerate);break;
				case self::SET:return new TSetType();break;
				case self::TIMESTAMP:return new TTimeStampType();break;
			}
		} 		
	}
	
	class TTimeStampType extends TDataType {
		public function __construct(){
			$this->typeCode = TDataType::TIMESTAMP;
		}
	} 

	class TStringType extends TDataType {
		public $Length = null;
		public $VarLength = true;
		public function __construct($l = null, $varlength = true){
			$this->VarLength = $varlength;
			$this->typeCode = TDataType::STRING;
			if (isset($l))
				$this->Length = $l;
		}
		
		public function __toString(){
			return parent::__toString().";size=".$this->Length.";varlength=".($this->VarLength?"yes":"no");
		} 
	}
	
	class TIntegerType extends TDataType {
		public $Size = null;
		public function __construct($s = null){
			$this->typeCode = TDataType::INTEGER;
			if (isset($s))
				$this->Size = $s;
		}

/**
 * @return string
 */		
		public function __toString(){
			return parent::__toString().";size=".$this->Size;
		} 
	}

	final class TLongType extends TIntegerType {
		public function __construct($s = null){
			parent::__construct($s);
			$this->typeCode = TDataType::BIGINT;
		}
	}
	
	class TDecimalType extends TDataType {
		public $Size = null;
		public $Decimals = null;
		public function __construct($s = null, $d = null){
			$this->typeCode = TDataType::DECIMAL;
			if (isset($s))
				$this->Size = $s;
			if (isset($d))
				$this->Decimals = $d;
		}

/**
 * @return string
 */		
		public function __toString(){
			return parent::__toString().";size=".$this->Size.";decimals=".$this->Decimals;
		} 
	}
	
	final class TFloatType extends TDecimalType {
		public function __construct($s = null, $d = null){
			parent::__construct($s,$d);
			$this->typeCode = TDataType::FLOAT;
		}
	}
	
	final class TBooleanType extends TDataType {
		public function __construct(){
			$this->typeCode = TDataType::BOOLEAN;
		} 
	}
	
	class TTextType extends TDataType {
		public function __construct(){
			$this->typeCode = TDataType::TEXT;
		}
	}
	
	final class THTMLType extends TTextType {
		public function __construct(){
			$this->typeCode = TDataType::HTML;
		}
	}
	
	class TFileType extends TDataType {
		public function __construct(){
			$this->typeCode = TDataType::FILE;
		}
	}
	
	final class TImageType extends TFileType {
		public function __construct(){
			$this->typeCode = TDataType::IMAGE;
		}
	}
	
	class TFileLinkType extends TStringType {
		public function __construct($l = null){
			$this->Length = 200;
			parent::__construct($l,true);
			$this->typeCode = TDataType::FILELINK;
		}
	}
	
	final class TImageLinkType extends TFileLinkType {
		public function __construct($l = null){
			parent::__construct($l);
			$this->typeCode = TDataType::IMAGELINK;
		}
	}
	
	final class TPasswordType extends TStringType {
		public function __construct($l = null){
			$this->Length = 10;
			parent::__construct($l,true);
			$this->typeCode = TDataType::PASSWORD;
		}
	}
	
	final class TDateTimeType extends TDataType {
		public function __construct(){
			$this->typeCode = TDataType::DATETIME;
		} 
	}
	
	final class TGuidType extends TDataType {
		const GUID32 = 32;
		const GUID23 = 23;
		const GUID13 = 13;
		
		private $_guid_length_;
		
		public function __get($nm){
			switch ($nm){
				case "Length":return $this->_guid_length_;break;
			}
		} 
		
		public function __construct($autogenerate = false, $length = TGuidType::GUID32){
			if (!in_array($length,array(self::GUID13,self::GUID23,self::GUID32)))
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			$this->_guid_length_ = $length;
			$this->typeCode = TDataType::GUID;
		} 
		
		public function Generate(){
			switch ($this->_guid_length_){
				case self::GUID13:return uniqid();break;
				case self::GUID23:return uniqid(rand(), true);break;
				case self::GUID32:return md5(uniqid(rand(), true));break;
			}
		}
		
		public function __toString(){
			return parent::__toString().";size=".$this->_guid_length_;
		} 		
	}
	
	final class TSetType extends TDataType {
		public function __construct(){
			$this->typeCode = TDataType::SET;
		} 
	}
