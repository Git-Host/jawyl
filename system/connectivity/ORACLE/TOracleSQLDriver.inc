<?php
TApplication::Import('common::storage');
TApplication::Import('connectivity::ORACLE::util');

class TOracleSqlDriver extends TConfigurable implements IAnsiSQLDBDriver {
	private $_user_;
	private $_password_;
	private $_database_;
	private $_connection_;
	
	private $_trancounter_ = 0;
	
	public function __set($nm,$value){
		switch ($nm){
			case 'User':$this->_user_ = $value;break;
			case 'Password':$this->_password_ = $value;break;
			case 'Database':$this->_database_ = $value;break;
			default:parent::__set($nm, $value);break;
		}
	}
	
	public function Connect(){
		if (!$this->_connection_){
			$this->_connection_ = @oci_pconnect($this->_user_, $this->_password_, $this->_database_);
			if (!$this->_connection_){
				throw new TOracleException(null,oci_error());
			}
		}
	}
	
	private function _prepare($sql,&$parameters){
		$stmt = oci_parse($this->_connection_, $sql);
		if (!$stmt)
			throw new TOracleException(null,oci_error($this->_connection_));
		foreach ($parameters as $key=>&$value){
			if ($value instanceof TDBData){
				/*
				switch ($value->Type){
					@todo Правильные операции с BLOB
					case TDBData::TYPE_LARGETEXT:{
						$clob = oci_new_descriptor($this->_connection_, OCI_D_LOB);
						oci_bind_by_name($stmt, $key, $clob, -1, OCI_B_CLOB);	
						$clob->write($value->Value);
					}break;
					case TDBData::TYPE_LARGEBINARY:{
						$blob = oci_new_descriptor($this->_connection_, OCI_D_LOB);
						oci_bind_by_name($stmt, $key, $blob, -1, OCI_B_BLOB);	
						$blob->write($value->Value);
					}break;
					default:oci_bind_by_name($stmt, $key, $value);break;
				}
				*/
				$temp = $value->Value;
				oci_bind_by_name($stmt, $key, $temp);
			} else {
				if (is_array($value) && isset($value[1]))
					oci_bind_by_name($stmt, $key, $value[0],$value[1]);
				else	
					oci_bind_by_name($stmt, $key, $value);
			}
		}
		return $stmt;
	}

/**
 * @param string $sql
 * @param array $parameters
 * @return Iterator
 */	
	protected function fetchRecords($sql,array &$parameters = array()){
		$start = microtime(true);
		$result = new TOracleRecordIterator($this->__prepare($sql, $parameters),$this->_trancounter_ > 0?OCI_DEFAULT :OCI_COMMIT_ON_SUCCESS);
		$this->logSuccess($sql,$parameters,$start);
		return $result;
	}	
	
	
/**
 * @param string $q
 * @param array $parameters
 * @return int
 */	
	public function Execute($q,array $parameters = array()){
		$stmt = $this->__prepare($q, $parameters);			
		if (!oci_execute($stmt,$this->_trancounter_ > 0?OCI_DEFAULT :OCI_COMMIT_ON_SUCCESS)){
			throw new TOracleException(null,oci_error($stmt));
		}
		return oci_num_rows($stmt);
	}

/**
 * @param string $q
 * @param array $parameters
 * @return IIterator<array>
 */	
	public function Fetch($q, array $parameters = array()){
		return new TOracleRecordIterator($this->__prepare($q, $parameters),$this->_trancounter_ > 0?OCI_DEFAULT :OCI_COMMIT_ON_SUCCESS);
	}
	
/**
 * @param string $q
 * @param array $parameters
 * @return mixed
 */		
	public function Scalar($q, array $parameters = array()){
		$stmt = $this->__prepare($q, $parameters);			
		if (!oci_execute($stmt,$this->_trancounter_ > 0?OCI_DEFAULT :OCI_COMMIT_ON_SUCCESS)){
			throw new TOracleException(null,oci_error($stmt));
		}
		if (!oci_fetch($stmt)){
			throw new TOracleException(null,oci_error($stmt));
		}
		return oci_result($stmt, 1);
	}
	
	public function Begin(){
		$this->_trancounter_++;
	}
	
	protected function Commit(){
		if ($this->_trancounter_ > 0) $this->_trancounter_--;
		if (!oci_commit($this->_connection_))
			 throw new TOracleException(null,oci_error($this->_connection_));			
	}
	
	protected function rollbackTransaction(){
		if ($this->_trancounter_ > 0) $this->_trancounter_--;
		if (!oci_rollback($this->_connection_))
			throw new TOracleException(null,oci_error($this->_connection_));
	}	
}