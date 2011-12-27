<?php
/**
 * Класс исключения Oracle
 * @author dakrasilnikov
 *
 */
class TOracleException extends TStorageException {
	private $_msg_;
/**
 * Конструктор
 * @param array $error - массив содержащий информацию об ошибке.
 * @param string $prefix - необязательный префикс сообщения об ошибке
 */	
	public function __construct($code, array $error){
		if (is_null($code)){
			if (isset($error['code']))
				$code = $error['code'];
			$this->_msg_ = '';
			if (isset($error['message']))
				$this->_msg_ .= $error['message'];
			if (isset($error['offset']))
				$this->_msg_ .= ' on line %offset';
			if (isset($error['sqltext']))	
				$this->_msg_ .= ' in sql: %sqltext';
		} 
		parent::__construct($code,$error);
	}
	
/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			default:{
				$msgtext = parent::getMessageText($msgcode);
				if ($msgtext != "") return $msgtext;
				return $this->_msg_;
			}break;
		}
	}
}


/**
 * Класс итератора возвращаемого методом выборки. 
 * Используется неявно.
 * @author dakrasilnikov
 *
 */

class TOracleRawDataIterator extends TIterator {
	private $_recordset_;
	private $_exec_flag_;
	
	protected $currentRow;
	protected $fields = array();
	protected $pointer;
	
	
/** 
 * Конструктор
 * @param resource $recordset
 * @param int $flag
 */	
	public function __construct($recordset,$flag){
		$this->_recordset_ = $recordset;
		$this->_exec_flag_ = $flag;
		$fn = oci_num_fields($this->_recordset_);
		for ($i = 0; $i < $fn; $i++){
			$nm = oci_field_name($this->_recordset_,$i);
			if (in_array($nm,$this->fields)) $nm .= $i;
			$this->fields[] = $nm;
		}
		$this->pointer = -1;
	}
	
	public function Rewind() {	
		$this->pointer = 0;
		$this->currentRow = false;
		//@oci_free_cursor($this->_recordset_);	
/*
 * @todo php-bug: Исключения не подхватываются при неявных вызовах методов итератора в foreach 
 */				
		if (!oci_execute($this->_recordset_,$this->_exec_flag_))
			throw new TOracleException(null,oci_error($this->_recordset_)); 
		return $this->next();
    }

    public function Valid() {
    	return is_array($this->currentRow);
    }
    	
	public function Next(){
		if ($row = oci_fetch_array($this->_recordset_,OCI_NUM)){
			$this->currentRow = array_merge($row,array_combine($this->fields,$row));
			$this->pointer++;
			return true;
		}
		return false;
	}	
	
	public function Seek($position){
		$result = false;
		if ($position < $this->pointer)
				$result = $this->Rewind();		
		while ($this->pointer < $position){
			$result = $this->Next();
		}		
		return $result !== false;
	}

    public function Key() {
        return $this->pointer;
    }
	
	public function Item(){
		return $this->currentRow;
	}
		
	public function ItemCount(){
		return oci_num_rows($this->_recordset_);
	}
	
	public function DataType(){return IIterator::DT_RAW;}
}