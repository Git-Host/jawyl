<?php
class TEnumeration {
	private $_type_;
	
	public function __construct($type){
		$rc = new ReflectionClass(get_class($this));
		$types = array_values($rc->getConstants());
		if (!in_array($type,$this->_types_))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$this->_type_ = $type;
	}
	
	public function __toString(){
		return $this->_type_;
	}
}