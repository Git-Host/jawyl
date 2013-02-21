<?php
interface IMySqlResultSet {
	/**
	 * @return array|false
	 */
	public function FetchAssoc();

	/**
	 * @return array|false
	*/
	public function FetchIndexed();

	/**
	 * @return int
	*/
	public function RowCount();
	/**
	 * @return int
	*/
	public function FieldCount();
	/**
	 * @param int $ind
	 * @return string
	*/
	public function FieldName($ind);

	/**
	 * @param int $position
	 * @return bool
	*/
	public function Seek($position);
}


interface IMySqlQuery {
	/**
	 * @param number $row
	 * @param number $field
	 */
	public function Scalar($row = 0, $field = 0);
	/**
	 * @return IMySqlResultSet
	*/
	public function ResultSet();
	public function Execute();
}

interface IMySqlConnection {
	public function __construct($server,$user,$pwd,$db,$names);
	public function Begin();
	public function Commit();
	public function Rollback();
	public function Close();
	/**
	 * @param string $q
	 * @param array $parameters
	 * @return IMySqlQuery
	*/
	public function Query($q, array $parameters = array());

	/**
	 * @param mixed $value
	 * @return string
	*/
	public function Quote($value);

	/**
	 * @return int
	*/
	public function LastId();
}

abstract class TMySqlAdapter implements IMySqlConnection {
	protected abstract function quoteString($value);
	
	public function Quote($value){
		if (is_null($value)) return 'null';
		if (is_bool($value)) return $value?'1':'0';
		if (is_int($value) || is_float($value)) return $value;
		if ($value instanceof TDate)
			$value = $value->ToString('Y-m-d H:i:s');
		else
			$value = (string)$value;
		
		if (is_string($value)){
			if ($value == '')
				return 'null';
			if (($value == 'true') || ($value == 'false'))
				return $this->Quote(TConvertions::ConvertToBoolean($value));
			return $this->quoteString($value);
		}
		throw new TCoreException(TCoreException::ERR_BAD_VALUE);		
	}
}

class TMySqlResultSet implements IMySqlResultSet {
	private $_result_;

	public function __construct($result){
		$this->_result_ = $result;
	}

	/**
	 * @return bool
	 */
	public function FetchAssoc(){
		return mysql_fetch_assoc($this->_result_);
	}

	/**
	 * @return bool
	 */
	public function FetchIndexed(){
		return mysql_fetch_array($this->_result_,MYSQL_NUM);
	}

	/**
	 * @return int
	 */
	public function RowCount(){
		return mysql_num_rows($this->_result_);
	}
	/**
	 * @return int
	 */
	public function FieldCount(){
		return mysql_num_fields($this->_result_);
	}
	/**
	 * @param int $ind
	 * @return string
	 */
	public function FieldName($ind){
		return mysql_field_name($this->_result_, $ind);
	}

	/**
	 * @param int $position
	 * @return bool
	 */
	public function Seek($position){
		return mysql_data_seek($this->_result_, $position);
	}
}

class TMySqlQuery implements IMySqlQuery {
	private $_q_;
	private $_connection_;

	public function __construct($q,$connection){
		$this->_q_ = $q;
		$this->_connection_ = $connection;
	}

	private function _check_error(){
		if (mysql_errno($this->_connection_) != 0){
			throw new TMySqlException(mysql_errno($this->_connection_),array("msgtext"=>mysql_error($this->_connection_),"query"=>$this->_q_));
			return false;
		}
		return true;
	}

	public function Scalar($row = 0, $field = 0){
		$r = mysql_query($this->_q_,$this->_connection_);
		if ($this->_check_error())
			return mysql_result($r, $row, $field);
		return null;
	}

	public function ResultSet(){
		$r = mysql_query($this->_q_,$this->_connection_);
		if ($this->_check_error())
			return new TMySqlResultSet($r);
		return null;
	}

	public function Execute(){
		mysql_query($this->_q_,$this->_connection_);
		if ($this->_check_error())
			return mysql_affected_rows($this->_connection_);
		return null;
	}
}

class TMySqlConnection extends TMySqlAdapter {
	private $_connection_;

	public function __construct($server,$user,$pwd,$db,$names = null){
		if ($this->_connection_ = mysql_pconnect($server,$user,$pwd))
		{
			if (mysql_select_db($db)){
				if ($names)
					mysql_query("SET NAMES '".$names."'",$this->_connection_);
				return true;
			} else
				throw new TMySqlException(TMySqlException::ERR_MYSQL_DBFAIL,array("db"=>$db,"server"=>$server));
		} else
			throw new TMySqlException(TMySqlException::ERR_MYSQL_CONNECTFAIL,array("server"=>$server));
	}

	public function __destruct(){
		$this->Close();
	}

	public function Begin(){
		mysql_query('start transaction',$this->_connection_);
	}
	public function Commit(){
		mysql_query('commit',$this->_connection_);
	}
	public function Rollback(){
		mysql_query('rollback',$this->_connection_);
	}
	public function Close(){
		mysql_close($this->_connection_);
	}

	private function _prepare_query_param_key(&$value){
		$value = ":".$value;
	}
	
	private function _prepare_param(&$value){
		$value = $this->Quote($value);
	}
	
	private function _prepare_statement($q,array $parameters = array()){
		$pnames =  array_keys($parameters);
		$pvalues = array_values($parameters);
		array_walk($pnames,array($this,"_prepare_query_param_key"));
		array_walk($pvalues,array($this,"_prepare_param"));
		return str_replace(array_reverse($pnames),array_reverse($pvalues),$q);
	}

	/**
	 * @param string $q
	 * @param array $parameters
	 * @return IMySqlQuery
	 */
	public function Query($q, array $parameters = array()){
		return new TMySqlQuery($this->_prepare_statement($q,$parameters), $this->_connection_);
	}

	/**
	 * @param mixed $value
	 * @return string
	 */
	protected function quoteString($value){
		return "'" . mysql_real_escape_string($value,$this->_connection_) . "'";
	}

	/**
	 * @return int
	 */
	public function LastId(){
		return mysql_insert_id($this->_connection_);
	}
}

class TMySqliResultSet implements IMySqlResultSet {
/**
 * @var mysqli_result
 */	
	private $_result_;

	public function __construct(mysqli_result $result){
		$this->_result_ = $result;
	}
	
	public function __destruct(){
		$this->_result_->free();
	}

	/**
	 * @return bool
	 */
	public function FetchAssoc(){
		return $this->_result_->fetch_assoc();
	}

	/**
	 * @return bool
	 */
	public function FetchIndexed(){
		return $this->_result_->fetch_array(MYSQLI_NUM);
	}

	/**
	 * @return int
	 */
	public function RowCount(){
		return $this->_result_->num_rows;
	}
	/**
	 * @return int
	 */
	public function FieldCount(){
		return $this->_result_->field_count;
	}
	/**
	 * @param int $ind
	 * @return string
	 */
	public function FieldName($ind){
		if ($f = $this->_result_->fetch_field_direct($ind))
			return $f->name;
		return null;
	}

	/**
	 * @param int $position
	 * @return bool
	 */
	public function Seek($position){
		return $this->_result_->data_seek($position);
	}
}

class TMySqliQuery implements IMySqlQuery {
/**
 * @var mysqli_stmt
 */	
	private $_q_;
	
/**
 * @var mysqli
 */	
	private $_connection_;
	
	private $_param_mask_;

	public function __construct($query, mysqli $connection){
		preg_match_all('/\'(?:(?<=\\\\)\'|[^\'])*\'|:\w+/', $query, $this->_param_mask_, PREG_SET_ORDER);
		array_walk($this->_param_mask_,create_function('&$value','$value = ($value[0][0] == ":")?substr($value[0],1):null;'));
		$this->_param_mask_ = array_filter($this->_param_mask_);
		$this->_q_ = $connection->prepare(preg_replace_callback(
					'/\'(?:(?<=\\\\)\'|[^\'])*\'|:\w+/',
					create_function('$matches','return ($matches[0][0] == ":")?"?":$matches[0];'),
					$query)
		);
		if (!$this->_q_){
			throw new Exception($connection->error);
		};
		$this->_connection_ = $connection;
	}

	private function _check_error(){
		if ($this->_connection_->errno != 0){
			throw new TMySqlException($this->_connection_->errno,array("msgtext"=>$this->_connection_->error));
			return false;
		}
		return true;
	}
	
	public function Bind(array $parameters){
		if (!empty($parameters)){
			$types = '';
			$values = array();
			foreach ($this->_param_mask_ as $nm){
				if (is_int($parameters[$nm]) || is_bool($parameters[$nm]))
					$types .= 'i';
				else if (is_double($parameters[$nm]))
					$types .= 'd';
				else 
					$types .= 's';
				$values[] = &$parameters[$nm];
			}
			array_unshift($values,$types);
			call_user_func_array(array($this->_q_,'bind_param'), $values);
		}
	}

	public function Scalar($row = 0, $field = 0){
		$this->_q_->execute();
		if ($this->_check_error()){
			$r = $this->_q_->get_result();
			$res = null;
			if ($r->data_seek($row))
				if ($row = $r->fetch_array(MYSQLI_NUM))
					if (isset($row[$field]))
						$res = $row[$field];
			$r->free();
			return $res;
		}
		return null;
	}

	public function ResultSet(){
		$this->_q_->execute();
		if ($this->_check_error())
			return new TMySqliResultSet($this->_q_->get_result());
		return null;
	}

	public function Execute(){
		$this->_q_->execute();
		if ($this->_check_error())
			return $this->_q_->affected_rows;
		return null;
	}
}

class TMySqliConnection extends TMySqlAdapter {
	private $_connection_;
	
	private $_queries_ = array();

	public function __construct($server,$user,$pwd,$db,$names = null){
		$this->_connection_ = new mysqli($server,$user,$pwd,$db);
		if ($this->_connection_->connect_errno)
			throw new TMySqlException(TMySqlException::ERR_MYSQL_CONNECTFAIL,array("server"=>$server));
		$this->_connection_->set_charset($names);
	}

	public function __destruct(){
		$this->Close();
	}

	public function Begin(){
		$this->_connection_->autocommit(false);
	}
	public function Commit(){
		$this->_connection_->commit();
	}
	public function Rollback(){
		$this->_connection_->rollback();
	}
	
	public function Close(){
		$this->_connection_->close();
	}

	private function _prepare_param(&$value){
		if ($value instanceof TDate)
			$value = $value->ToString('Y-m-d H:i:s');
	}
		
	/**
	 * @param string $q
	 * @param array $parameters
	 * @return IMySqlQuery
	 */
	public function Query($q, array $parameters = array()){
		if (!isset($this->_queries_[$q]))
			$this->_queries_[$q] = new TMySqliQuery($q,$this->_connection_);
		array_walk($parameters, array($this,'_prepare_param'));
		$this->_queries_[$q]->Bind($parameters);
		return $this->_queries_[$q];
	}

	/**
	 * @param mixed $value
	 * @return string
	 */
	protected function quoteString($value){
		return "'" . $this->_connection_->real_escape_string($value) . "'";
	}

	/**
	 * @return int
	 */
	public function LastId(){
		return $this->_connection_->insert_id;
	}
}