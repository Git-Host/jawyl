<?php
/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("common::item");
TApplication::import("common::storage");
TApplication::import("connectivity::MySQL::util");

/**
 * @property string $From
 * @property string $Where
 * @property string $Fields
 * @property string $Targets
 * @property string $Order
 * @property string $Limit 
 * @property array $Parameters;
 * @property array $TargetSources;
 */

class TMySqlDataSetProcessor {	
	private $_fields_ = array();
	private $_where_ = array();
	private $_targets_ = array();
	private $_targets_ds_ = array();
	private $_from_ = array();
	private $_order_ = array();
	private $_limit_ = null;
	private $_param_prefix_ = TMySqlDataSetProcessor::PARAM_PREFIX;
	private $_subqueries_ = 0;
	protected $parameters = array();
	
	private $_param_counter_ = 1;
	
	const NS_BASE_SUFFIX = "_base";
	const PARAM_PREFIX = "param";

/**
 * @var TMySqlDBDriver
 */	
	protected $driver;
	
	protected function getFieldPrefix(IDataSourceField $fld){
		if ($fld->DataSource){
			if ($fld instanceof TNestedSetsField)
				switch ($fld->Type){
					case TNestedSetsField::BASE_FIELD:return ($fld->DataSource->Alias?$fld->DataSource->Alias:$fld->DataSource->Name).self::NS_BASE_SUFFIX;break;
					case TNestedSetsField::SUBSET_FIELD:return ($fld->DataSource->Alias?$fld->DataSource->Alias:('['.$fld->DataSource->Name.']'));break;
				}	
			return 	$fld->DataSource->Alias?$fld->DataSource->Alias:('['.$fld->DataSource->Name.']');
		}
		return '';	
	}
	
	protected function parseAllField(IDataSourceField $fld){
		$prefix = $this->getFieldPrefix($fld);
		if ($prefix)
			$prefix .= ".";
		return $prefix."*"; 
	} 
	
	protected function getFieldName(IDataSourceField $fld){
		return $fld->Name();
	}
	
	protected final function parseField(IDataSourceField $fld,$forfetch = false){
		if (($fld instanceof TAllField) || ($fld instanceof TNestedSetsAllField))
			return $this->parseAllField($fld);
		if ($fld instanceof TExpressionField)
			return 	"(".$this->parseOperation($fld->Expression).")".($forfetch?($fld->Alias?(" as ".$fld->Alias):""):"");
		$prefix = $this->getFieldPrefix($fld);
		if ($prefix)
			$prefix .= ".";
		return $prefix.$this->getFieldName($fld).($forfetch?($fld->Alias?(" as ".$fld->Alias):""):""); 
	}
	
	protected final function parseOperand($operand){
		if ($operand instanceof IDataSourceField)
			return $this->parseField($operand);
		else if ($operand instanceof TOperation)
			return $this->parseOperation($operand);
		else if ($operand instanceof TDBActionParameter){
			if (!key_exists($operand->Name,$this->parameters));
				$this->parameters[$operand->Name] = $operand->Value;
			return ":".$operand->Name;	
		} else {
			$name = $this->_param_prefix_."_".$this->_param_counter_;
			$this->parameters[$name] = $operand;
			$this->_param_counter_++;
			return ":".$name;
		}		
	}
	
	protected function parseOperation(TOperation $o){
			$type = "binar";
			$oper = "";
			$unarseparator = ", ";
			switch ($o->Type){
    			case TConditionType::C_AND:$oper = "and";break;
    			case TConditionType::C_OR:$oper = "or";break;
    			case TConditionType::C_NOT:{$oper = "not ";$type="unar";$unarseparator=" and ";}break;
    			case TConditionType::C_IS:$oper = "is";break;
    			case TConditionType::C_REGEXP:{$oper="regexp";$type="unar";}break;
    			case TConditionType::C_LIKE:$oper="like";break;
    			case TConditionType::C_NOT_LIKE:$oper="not like";break;
    			case TConditionType::C_EQUAL:$oper = "<=>";break;
    			case TConditionType::C_NOT_EQUAL:$oper = "<>";break;
    			case TConditionType::C_MORE:$oper = ">";break;
    			case TConditionType::C_LESS:$oper = "<";break;
    			case TConditionType::C_MORE_OR_EQUAL:$oper = ">=";break;
    			case TConditionType::C_LESS_OR_EQUAL:$oper = "<=";break;
   				case TOperationType::O_ADD:$oper = "+";break;
   				case TOperationType::O_SUBTRACT:$oper = "-";break;
   				case TOperationType::O_MUL:$oper = "*";break;
   				case TOperationType::O_DIV:$oper = "/";break;
   				case TOperationType::O_AND:$oper = "&";break;
   				case TOperationType::O_OR:$oper = "|";break;
   				case TConditionType::C_IN:{$oper = "in";}break;
/*   				case "gsum":{$oper = "sum";$type="unar";}break;
   				case "gavg":{$oper = "avg";$type="unar";}break;
   				case "gcount":{$oper = "count";$type="unar";}break;
*/   				
   				case TOperationType::O_IFELSE:{$oper = "if";$type="unar";}break;
   				case TOperationType::O_CASE:$oper = "case";break;
   				case TOperationType::O_STRLEN:{$oper = "length";$type="unar";}break;
   				case TOperationType::O_STRPOS:{$oper = "locate";$type="unar";}break;
   				case TOperationType::O_STRCONCAT:{$oper = "concat";$type="unar";}break;
   				case TOperationType::O_STRLOWER:{$oper = "lower";$type="unar";}break;
   				case TOperationType::O_STRUPPER:{$oper = "upper";$type="unar";}break;
   				case TOperationType::O_SUBSTR:{$oper = "substr";$type="unar";}break;
   				case TOperationType::O_STRREPLACE:{$oper = "replace";$type="unar";}break;
   				case TOperationType::O_STRLTRIM:{$oper = "ltrim";$type="unar";}break;
   				case TOperationType::O_STRRTRIM:{$oper = "rtrim";$type="unar";}break;
   				case TOperationType::O_STRTRIM:{$oper = "trim";$type="unar";}break;
   				case TOperationType::O_MOD:$oper = "%";break;
   				case TOperationType::O_ABS:{$oper = "abs";$type="unar";}break;
   				case TOperationType::O_ACOS:{$oper = "acos";$type="unar";}break;
   				case TOperationType::O_ASIN:{$oper = "asin";$type="unar";}break;
   				case TOperationType::O_ATAN:{$oper = "atan";$type="unar";}break;
   				case TOperationType::O_COS:{$oper = "cos";$type="unar";}break;
   				case TOperationType::O_SIN:{$oper = "sin";$type="unar";}break;
   				case TOperationType::O_TAN:{$oper = "tan";$type="unar";}break;
   				case TOperationType::O_CEIL:{$oper = "ceil";$type="unar";}break;
   				case TOperationType::O_FLOOR:{$oper = "floor";$type="unar";}break;
   				case TOperationType::O_ROUND:{$oper = "round";$type="unar";}break;
   				case TOperationType::O_POW:{$oper = "pow";$type="unar";}break;
   				case TOperationType::O_EXP:{$oper = "exp";$type="unar";}break;
   				case TOperationType::O_LN:{$oper = "ln";$type="unar";}break;
   				case TOperationType::O_LOG:{$oper = "log";$type="unar";}break;
   				case TOperationType::O_SQRT:{$oper = "sqrt";$type="unar";}break;
   				case TOperationType::O_RAND:{$oper = "rand";$type="unar";}break;
   				case TOperationType::O_DATE_CUR:{$oper = "curdate";$type="unar";}break;
   				case TOperationType::O_DATE_ADD:{$oper = "adddate";$type="unar";}break;
   				case TOperationType::O_DATE_SUBT:{$oper = "subdate";$type="unar";}break;
   				case TOperationType::O_DATE_DIFF:{$oper = "datediff";$type="unar";}break;
   				case TOperationType::O_DATE_MAKE:{$oper = "makedate";$type="unar";}break;
   				case TOperationType::O_DATE_DAY:{$oper = "day";$type="unar";}break;
   				case TOperationType::O_DATE_MONTH:{$oper = "month";$type="unar";}break;
   				case TOperationType::O_DATE_YEAR:{$oper = "year";$type="unar";}break;
   				case TOperationType::O_DATE_WEEK:{$oper = "week";$type="unar";}break;
   				case TOperationType::O_DATE_WKDAY:{$oper = "dayofweek";$type="unar";}break;
   				case TOperationType::O_TIME_CUR:{$oper = "curtime";$type="unar";}break;
   				case TOperationType::O_TIME_ADD:{$oper = "subdate";$type="unar";}break;
   				case TOperationType::O_TIME_SUBT:{$oper = "subtime";$type="unar";}break;
   				case TOperationType::O_TIME_DIFF:{$oper = "timediff";$type="unar";}break;
   				case TOperationType::O_TIME_MAKE:{$oper = "maketime";$type="unar";}break;
   				case TOperationType::O_TIME_HOUR:{$oper = "hour";$type="unar";}break;
   				case TOperationType::O_TIME_MIN:{$oper = "minute";$type="unar";}break;
   				case TOperationType::O_TIME_SEC:{$oper = "second";$type="unar";}break;
   				case TOperationType::O_TIME_MKSEC:{$oper = "microsecond";$type="unar";}break;
   				case TOperationType::O_COUNT:{$oper = "count";$type="unar";}break;			
   				case TOperationType::O_SUM:{$oper = "sum";$type="unar";}break;
   				case TOperationType::O_AVG:{$oper = "avg";$type="unar";}break;
			}
			
			if (in_array($oper,array("adddate","subdate"))){
				$op1 = "";$op2 = "";
				if (count($o->Operands) > 0)
					$op1 = $this->parseOperand($o->Operands[0]);
				if (count($o->Operands) > 1)	
					$op2 = $this->parseOperand($o->Operands[1]);
				$type = "day";
				if (count($o->Operands) > 2)
					$type = $this->parseOperand($o->Operands[2]);	
				return $oper."(".$op1.", interval ".$op2." ".$type.")";
			} elseif ($oper == "case"){
				$result = "case";
				$i = 0;
				$n = count($o->Operands);
				if ($n < 2)
					throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				while ($i < $n){
					if ($i % 2 == 0){
						if ($i + 1 ==  $n)
							$result .= " else ".$this->parseOperand($o->Operands[$i]);
						else
							$result .= " when ".$this->parseOperand($o->Operands[$i]);	
					} else 
						$result .= " then ".$this->parseOperand($o->Operands[$i]);
					$i++;
				}
				$result .= " end case";
				return $result; 
			} elseif ($oper == "in"){
				$result = $this->parseOperand($o->Operands[0])." in (";
				$temp = array();
				$n = count($o->Operands);
				for ($i = 1; $i < $n; $i++)
					$temp[] = $this->parseOperand($o->Operands[$i]);
				return $result.join(",",$temp).")";
			} else {
				$arguments = array();
				foreach ($o->Operands as $o)
			 		$arguments[] = $this->parseOperand($o);	
			}

			if ($type=="binar")
				return join(" ".$oper." ",$arguments);
			else {
				if ($oper == 'count' && empty($arguments))
					$arguments[] = '*';
				return $oper."(".join($unarseparator,$arguments).")";
			}
			return null;
	}

	protected final function parseOperations(array $opers){
		$result = array();
		foreach ($opers as $op)
			$result[] = $this->parseOperation($op);
		return join(" and ",$result);	
	} 
	
	protected final function parseSort(TSorting $s){
		switch ($s->Sort){
			case TSortingType::SORT_ASC:return $this->parseField($s->Field)." asc";break;
			case TSortingType::SORT_DESC:return $this->parseField($s->Field)." desc";break;
			case TSortingType::SORT_SHUFFLE:return "rand()";break;
		}
	}
	
	protected function dataSourceTargetName(TDataSource $ds){
		return $ds->Alias?$ds->Alias:$ds->Name;
	}
	
	private function _parse_ds(TDataSource $ds, array $joinconditions){
		if ($ds->IsTarget){
			$this->_targets_[] = $this->dataSourceTargetName($ds);
			$this->_targets_ds_[] = $ds;
		}		
		return $this->parseDataSource($ds, $joinconditions);
	}
		
	protected function parseDataSource(TDataSource $ds, array $joinconditions){
		$on = ((!empty($joinconditions))?(" on ".$this->parseOperations($joinconditions)):"");
		if ($ds instanceof TNestedSets){
			$result = "[".$ds->Name."]".($ds->Alias?" as ".$ds->Alias:"").$on;
			$src = $ds->Alias?$ds->Alias:('['.$ds->Name.']');
			if ($ds->BasesNeeded){
				$base = ($ds->Alias?$ds->Alias:$ds->Name).self::NS_BASE_SUFFIX;
				$result .= " inner join [".$ds->Name."] as ".$base." on ";
				$d = $ds->IncludeBases?"=":"";
				switch ($ds->Direction){
					case TNestedSets::DIR_DOWN:{
						$result .= "(".$src.".".TMySqlDBDriver::NS_LKEY_FIELD." >".$d." ".$base.".".TMySqlDBDriver::NS_LKEY_FIELD.") and (".$src.".".TMySqlDBDriver::NS_RKEY_FIELD." <".$d." ".$base.".".TMySqlDBDriver::NS_RKEY_FIELD.")";
						if (!is_null($ds->Depth))
							$result .= " and (".$src.".".TMySqlDBDriver::NS_LEVEL_FIELD." <= ".$base.".".TMySqlDBDriver::NS_LEVEL_FIELD." + ".($ds->Depth + 1).")";
					}break;
					case TNestedSets::DIR_UP:{
						$result .= "(".$src.".".TMySqlDBDriver::NS_LKEY_FIELD." <".$d." ".$base.".".TMySqlDBDriver::NS_LKEY_FIELD.") and (".$src.".".TMySqlDBDriver::NS_RKEY_FIELD." >".$d." ".$base.".".TMySqlDBDriver::NS_RKEY_FIELD.")";
						if (!is_null($ds->Depth))
							$result .= " and (".$base.".".TMySqlDBDriver::NS_LEVEL_FIELD." <= ".$src.".".TMySqlDBDriver::NS_LEVEL_FIELD." + ".($ds->Depth + 1).")";
					}break;
				}
			} else if (!is_null($ds->Depth))
				$this->_where_[] = "(".$src.".".TMySqlDBDriver::NS_LEVEL_FIELD." <= ".$ds->Depth.")";
			if ($ds->IsTarget)	
				if (empty($this->_order_))
					$this->_order_ = array($src.".".TMySqlDBDriver::NS_LKEY_FIELD." asc");
			$this->_fields_[] = $src.".".TMySqlDBDriver::NS_LKEY_FIELD;		 	
			$this->_fields_[] = $src.".".TMySqlDBDriver::NS_RKEY_FIELD;		 	
			$this->_fields_[] = $src.".".TMySqlDBDriver::NS_LEVEL_FIELD;
			return $result;
		} else if ($ds instanceof TTable){
			return "[".$ds->Name."]".($ds->Alias?" as ".$ds->Alias:"").$on;
		} else if ($ds instanceof TDataSet){
			$this->_subqueries_++;
			$p = new TMySqlDataSetProcessor($ds, $this->driver,$this->_param_prefix_."_".$this->_subqueries_);
			$this->parameters = array_merge($this->parameters,$p->Parameters);
			return "(".$p->SelectQuery().") as ".$ds->Name.$on;
		} else
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
	}
	
	private function _parse_join(TJoin $j){
		$result = "";
		switch ($j->Type){
			case TJoin::JOIN_INNER:$result .= " inner join ";break;
			case TJoin::JOIN_LEFT:$result .= " left join ";break;
			case TJoin::JOIN_RIGHT:$result .= " right join ";break;
			case TJoin::JOIN_FULL:$result .= "  full join ";break;
		} 
		$result .= $this->_parse_ds($j->Source, $j->Conditions);
		return $result;
	}
	
	private function _parse(TDataSource $ds, $root, array $joinconditions){			
		foreach ($ds->Fields as $fld)
			if ($fld instanceof IDataSourceField)
				$this->_fields_[] = $this->parseField($fld,true);
			else if ($fld instanceof TOperation)
				$this->_fields_[] = $this->parseOperation($fld); 
					 
		
		foreach ($ds->Filter as $f)
			$this->_where_[] = $this->parseOperation($f);
			
			
		foreach ($ds->Sorting as $s)
			$this->_order_[] = $this->parseSort($s);

		if ($ds->Count)
			$this->_limit_ = ($ds->Offset?$ds->Offset.",":"").$ds->Count;
			
		if (($ds instanceof TDataSet) && $root){
			foreach ($ds->Sources as $s)
				$this->_parse($s,$false);
		} else {
			$from = $this->_parse_ds($ds,$joinconditions);
			foreach ($ds->Join as $j)
				$from .= $this->_parse_join($j);
		}

		$this->_from_[] = $from;	
	}
	
	public function __construct(TDataSource $dataset, TMySqlDBDriver $driver, $paramprefix = TMySqlDataSetProcessor::PARAM_PREFIX){
		$this->_param_prefix_ = $paramprefix;
		$this->driver = $driver;
		$this->_parse($dataset, true, array());
		$this->_fields_ = join(", ",$this->_fields_);
		$this->_from_ = join(", ",$this->_from_);
		$this->_where_ = join(" and ",$this->_where_);
		$this->_order_ = join(", ",$this->_order_);
		$this->_targets_ = join(", ",$this->_targets_);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Fields":return $this->_fields_;break;
			case "Where":return $this->_where_;break;
			case "Targets":return $this->_targets_;break;
			case "From":return $this->_from_;break;
			case "Order":return $this->_order_;break;
			case "Limit":return $this->_limit_;break;
			case "Parameters":return $this->parameters;break;
			case "TargetSources":return $this->_targets_ds_;break;
		}
	}
	
	public function SelectQuery(){
		$result = "select ";
		if ($this->Fields)
			$result .= $this->Fields;
		else
		  	$result .= "*";	
		$result .= " from ".$this->From;
		if ($this->Where)
			$result .= " where ".$this->Where;
		if ($this->Order)
			$result .= " order by ".$this->Order;
		if ($this->Limit)
			$result .= " limit ".$this->Limit;
		return $result;	
	}
	
	public function DeleteQuery(){
		$result = "delete ".$this->Targets." from ".$this->From;
		if ($this->Where)
			$result .= " where ".$this->Where;
		return $result;		
	}
}

/**
 * @property string $Updates
 */
class TMySqlDataSetUpdateProcessor extends TMySqlDataSetProcessor {
	private $_updates_ = array();
	private $_internal_;
	
	protected function createInternalProcessor(TDataSource $dataset, TMySqlDBDriver $driver){
		return new TMySqlDataSetProcessor($dataset, $driver);
	}
	
	public function __construct(TDataSource $dataset, TMySqlDBDriver $driver, array $fields, array $values){
		//parent::__construct($dataset, $driver);
		$this->_internal_ = $this->createInternalProcessor($dataset, $driver);
		$n = count($fields);
		$m = count($values);
		for ($i = 0; ($i < $n) && ($i < $m); $i++)
			$this->_updates_[] = $this->_internal_->parseField(($fields[$i] instanceof IDataSourceField)?$fields[$i]:(is_string($fields[$i])?new TTableField($fields[$i]):null))." = ".$this->_internal_->parseOperand($values[$i]);
		$this->_updates_ = join(",",$this->_updates_);
	}
	
	public function UpdateQuery(){
		$result = "update ".$this->From." set ".$this->Updates;
		if ($this->Where)
			$result .= " where ".$this->Where;
		return $result; 
	}
	
	public function __get($nm){
		switch ($nm){
			case "Updates":return $this->_updates_;break;
			default:return $this->_internal_->__get($nm);break;
		}
	}
} 


/**
 * @package Connectivity
 * @subpackage MySql
 * @category System
 * 
 * MySql database driver. 
 * @see IDBDriver
 */

class TMySqlDBDriver extends TDBDriver implements IAnsiSQLDBDriver {	
	public $Server = "localhost";
	public $User = "root";
	public $Password = "";
	public $DataBase;
	public $Names = "utf8";
	public $BulkInsertAmount = 100;
	
	protected $tablePrefix = "";
			
	const NS_LKEY_FIELD = "ns_lkey";
	const NS_RKEY_FIELD = "ns_rkey";
	const NS_LEVEL_FIELD = "ns_level";
	const NS_DELETED_FIELD = "ns_deleted";
	
	const AUTH_CONNECTION = 0;
	const AUTH_APP = 1;
	const AUTH_EXPLICIT = 2;
			
	protected $connection;
	
	private $_auth_method_ = self::AUTH_CONNECTION;
	//private $_acl_manager_ = null;
	private $_query_debug_ = false;
	
	private $_json_ns_names_ = null;

	public function __set($nm,$value){
		switch ($nm){
			case "DBPrefix":{$this->tablePrefix = $value;}break;
			case "QueryDebug":$this->_query_debug_ = TConvertions::ConvertToBoolean($value);break;
			default:parent::__set($nm,$value);break;
		}
	}
		
	public function __get($nm){
		switch ($nm){
			case "DBPrefix":return $this->tablePrefix;break;
			case "QueryDebug":return $this->_query_debug_;break;
			default:return parent::__get($nm);break;
		}
	}
		
	protected function quoteValue($value){
		if (is_null($value)) return "null";
		if (is_bool($value)) return $value?"1":"0";
		if (is_int($value) || is_float($value)) return $value;
		if ($value instanceof TDate)
			$value = $value->ToString("Y-m-d H:i:s");	  
		if (is_string($value)){
			if ($value == "")
		  		return "Null";
		  	if (($value == "true") || ($value == "false"))
		  		return $this->quoteValue(TConvertions::ConvertToBoolean($value));	
			return "'" . mysql_real_escape_string($value,$this->connection) . "'";
		}
		return $value;
	}
		
	protected function scalarValue($q, $row = 0, $field = 0){
		return mysql_result(mysql_query($q,$this->connection),$row,$field);
	}
		
	protected function lastInsertId(){
		return mysql_insert_id($this->connection);
	}
		
	private function _prepare_query_param_key(&$value){
		$value = ":".$value;
	}
	
	private function _prepare_table_name($matches){
		return "`".$this->RealTableName($matches[1])."`";
	}
		
	private function _prepare_statement($q,array $parameters = array()){
		$pnames =  array_keys($parameters);
		$pvalues = array_values($parameters);
		array_walk($pnames,array($this,"_prepare_query_param_key"));
		$evaluated = $this->_prepare_record_result($pvalues);
		return str_replace(array_reverse($pnames),array_reverse($evaluated),preg_replace_callback('/\[(\w[\w_]*)\]/',array($this,"_prepare_table_name"),$q));
	}
		
	public function Execute($q,array $parameters = array()){
		$this->_auto_connect();
		return $this->query($this->_prepare_statement($q,$parameters));
	}

	public function Fetch($q, array $parameters = array()){
		$this->_auto_connect();
		return $this->iQuery($this->_prepare_statement($q,$parameters));
	}
	
	public function Scalar($q, array $parameters = array()){
		$this->_auto_connect();
		return $this->scalarValue($this->_prepare_statement($q,$parameters));
	}	
	
	public function RealTableName($tablename){
		return $this->tablePrefix.$tablename;
	}
		
	protected function query($q){
		$result = false;
		$select = false;
		$queries = explode(chr(0),$q);
		if (count($queries) > 1){
			foreach ($queries as $q1)
				$result = $this->query($q1,$iteratortype);
			return $result;
		}
		
		if (preg_match('/\\A\\s*select\\s/i',$q)){
			$select = true; 
		}

		if ($this->_query_debug_){
			echo $q."<br/>";
			return;
		}
		
		$result = mysql_query($q,$this->connection);
		if (mysql_errno($this->connection) != 0){
			throw new TMySqlException(mysql_errno($this->connection),array("msgtext"=>mysql_error($this->connection),"query"=>$q));
			return false;
		}

		
		if (!$select)
			$result = mysql_affected_rows($this->connection);
		
		return $result;
	}
	
	protected function iQuery($q){
		return new TMySqlDBRawDataIterator($this->query($q),$this);
	}
		
	public function ConnectionParameters(){
		return array("Server" => TItemPropertyType::PT_STRING,
					"User" => TItemPropertyType::PT_STRING,
					"Password" => TItemPropertyType::PT_PASSWORD,
					"DataBase" => TItemPropertyType::PT_STRING,
					"Names" => TItemPropertyType::PT_STRING,
					"DBPrefix" => TItemPropertyType::PT_STRING);
	}
		 
	public function Connect(){
		$u = null;
		$p = null;
		switch ($this->AuthType){
			case IDBDriver::AUTH_DEFAULT:{
				$u = $this->User;
				$p = $this->Password;
			}break;
			default:
				if ($acl = $this->Acl)
					if ($user = $acl->CurrentDBUser()){
						$u = $user->Login();
						$p = $user->Password();
					}
			break;
		}
		
		if ($this->connection = mysql_pconnect($this->Server,$u,$p))
		{
			if (mysql_select_db($this->DataBase)){
				if ($this->Names)
				   $this->query("SET NAMES '".$this->Names."'");
				return true;
			} else 
			throw new TMySqlException(TMySqlException::ERR_MYSQL_DBFAIL,array("db"=>$this->DataBase,"server"=>$this->Server));
		} else 
			throw new TMySqlException(TMySqlException::ERR_MYSQL_CONNECTFAIL,array("server"=>$this->Server));
		return false;
	}
		
	public function IsConnected(){
		return !is_null($this->connection);
	}
	
	private function _auto_connect(){
		if (!$this->IsConnected()) $this->Connect();
	}
	
	public function DisConnect(){
		//mysql_close($this->connection);
		$this->connection = null;
	}	
		
	public function BeginTransaction(){
		$this->_auto_connect();
		$this->query("start transaction");
	}
		
	public function CommitTransaction(){$this->query("commit");}
		
	public function RollbackTransaction(){$this->query("rollback");}
		
	protected static function transformColType(TDataType $type){
		switch ($type->TypeCode) {
			case TDataType::PASSWORD:
			case TDataType::IMAGELINK:
			case TDataType::FILELINK:
			case TDataType::STRING:return ($type->VarLength?"var":"")."char".($type->Length?("(".$type->Length.")"):"");break;
			case TDataType::GUID:return "char(".($type->Length).")";break;
			case TDataType::BIGINT:return "bigint".($type->Size?("(".$type->Size.")"):"");break;
			case TDataType::BOOLEAN:return "bit";break;
			case TDataType::DATETIME:return "datetime";break;
			case TDataType::DECIMAL:return "decimal".(($type->Size || $type->Decimals)?("(".$type->Size.($type->Decimals?(",".$type->Decimals):"").")"):"");break;
			case TDataType::FLOAT:return "float".(($type->Size || $type->Decimals)?("(".$type->Size.($type->Decimals?(",".$type->Decimals):"").")"):"");break;
			case TDataType::INTEGER:return "integer".($type->Size?("(".$type->Size.")"):"");break;
			case TDataType::SET:
			case TDataType::IMAGE:
			case TDataType::FILE:return "mediumblob";break;
			case TDataType::HTML:
			case TDataType::TEXT:return "text";break;
			case TDataType::TIMESTAMP:return "timestamp";break;
			default:{throw new TCoreException(TCoreException::ERR_BAD_VALUE);return false;}break;
		}
	}

	protected static function transformRefIntegrity($ricode){
		switch ($ricode){
			case TTableReferenceType::REL_CASCADE:return "CASCADE";break;
			case TTableReferenceType::REL_RESTRICT:return "RESTRICT";break;
			case TTableReferenceType::REL_NOACTION:return "NO ACTION";break;
			case TTableReferenceType::REL_SETDEFAULT:return "SET DEFAULT";break;
			case TTableReferenceType::REL_SETNULL:return "SET NULL";break;
		}
		return "RESTRICT";
	}
	
	private function _transform_field_definition($tablename,TFieldDefinition $def){
		if ($def->Type instanceof TTableReferenceType)
			$coltype = self::transformColType($def->Type->ReferenceType);
		else
			$coltype = self::transformColType($def->Type);
		$result = array();
		$result[] = "`".$def->Name."` ".$coltype.($def->Nullable?" NULL":" NOT NULL").(isset($def->Default)?(sprintf(" DEFAULT %s",$this->quote_smart($def->Default))):"").(($def->AutoAssigned && ($def->Type == TDataType::INTEGER || $def->Type == TDataType::BIGINT))?" AUTO_INCREMENT":"").($def->PrimaryKey?" PRIMARY KEY":"");
		if ($def->Type instanceof TTableReferenceType){
			$result[] = "INDEX `i".$def->Name."` (`".$def->Name."`)";
			$result[] = "CONSTRAINT `fk_".$this->RealTableName($tablename)."_".$def->Name."` FOREIGN KEY `".$def->Name."` (`".$def->Name."`) REFERENCES `".$this->RealTableName($def->Type->MasterField->Table)."` (`".$def->Type->MasterField->Name."`) ON DELETE ".self::transformRefIntegrity($def->Type->OnDelete)." ON UPDATE ".self::transformRefIntegrity($def->Type->OnUpdate);
		} else if ($def->Unique)
			$result[] = "UNIQUE `i".$def->Name."` (`".$def->Name."`)";
		return $result;	
	}

	private function _transform_index_definition(TIndexDefinition $def){
		return ($def->Primary?"PRIMARY KEY `pk_":(($def->Unique?"UNIQUE `i":"")."INDEX `i")).$def->Name."` (".join(",",$def->Fields).")";
	}
		
	private function _transform_fk_definition($tablename,TForeignKeyDefinition $def){
		return "CONSTRAINT `fk_".$this->RealTableName($tablename)."_".$def->Name."` FOREIGN KEY `".$def->Name."` (`".$def->Name."`) REFERENCES `".$this->RealTableName($def->MasterField->Table)."` (`".$def->MasterField->Name."`) ON DELETE ".self::transformRefIntegrity($def->OnDelete)." ON UPDATE ".self::transformRefIntegrity($def->OnUpdate);
	}
		
	public function DefineNestedSets($name, array $definitions, array $options = array()){
		return $this->DefineTable($name,array_merge($definitions,array(
			new TFieldDefinition(TMySqlDBDriver::NS_LKEY_FIELD, new TIntegerType(20)),
			new TFieldDefinition(TMySqlDBDriver::NS_RKEY_FIELD, new TIntegerType(20)),
			new TFieldDefinition(TMySqlDBDriver::NS_LEVEL_FIELD, new TIntegerType()),
			new TFieldDefinition(TMySqlDBDriver::NS_DELETED_FIELD, new TBooleanType()),
			new TIndexDefinition(TMySqlDBDriver::NS_LKEY_FIELD, array(TMySqlDBDriver::NS_LKEY_FIELD)),
			new TIndexDefinition(TMySqlDBDriver::NS_RKEY_FIELD, array(TMySqlDBDriver::NS_RKEY_FIELD)),
			new TIndexDefinition(TMySqlDBDriver::NS_LEVEL_FIELD, array(TMySqlDBDriver::NS_LEVEL_FIELD)),
			new TIndexDefinition("nscomplexkey", array(TMySqlDBDriver::NS_LKEY_FIELD,TMySqlDBDriver::NS_RKEY_FIELD,TMySqlDBDriver::NS_LEVEL_FIELD))
		)),$options);
	}
		
	public function UndefineNestedSets($name){
		return $this->UndefineTable($name);
	}
		
	public function NestedSetsDefined($name){
		return $this->TableExists($name);
	}
								
	public function TableExists($tablename){
		$this->_auto_connect();
		return mysql_num_rows(mysql_query(sprintf("show tables like %s",$this->quoteValue($this->RealTableName($tablename))),$this->connection)) > 0;
	}
		
	public function DefineTable($tablename, array $definitions, array $options = array()){
		$q = "create table ".$this->RealTableName($tablename)." (";
		$fd = array();
		$fkd = array();
		$ind = array();
		foreach ($definitions as $def)
			if ($def instanceof TFieldDefinition){
				$fld = $this->_transform_field_definition($tablename,$def);
				$fd[] = $fld[0];
				if (count($fld) > 1) $ind[] = $fld[1];
				if (count($fld) > 2) $fkd[] = $fld[2];
			} else if ($def instanceof TIndexDefinition){
				$ind[] = $this->_transform_index_definition($def);
			} else if ($def instanceof TForeignKeyDefinition){
				$fkd[] = $this->_transform_fk_definition($tablename,$def); 
			}
		$defs = array_merge($fd,$ind,$fkd);
		
		$engine = "InnoDB";
		if (in_array(TStorageOptions::FOR_TRANSACTIONS,$options))
			$engine = "InnoDB";
		else if (in_array(TStorageOptions::FOR_FETCHING,$options))
			$engine = "MyISAM";
			
		$row_format = "";
		if ($engine == "MyISAM"){
			if (in_array(TStorageOptions::OPTIMIZE_SIZE, $options))
				$row_format = "ROW_FORMAT=DYNAMIC";
			else if (in_array(TStorageOptions::OPTIMIZE_SPEED, $options))
				$row_format = "ROW_FORMAT=FIXED";
		}
		
		$q .= join(",",$defs).") ENGINE=$engine $row_format DEFAULT CHARSET=utf8";
		return $this->query($q);
	}
		
	public function UndefineTable($tablename){
		$this->query("drop table if exists `".$this->RealTableName($tablename)."`");
	}
	
	public function DefineField($tablename, TFieldDefinition $definition){
		$q = "alter table `".$this->RealTableName($tablename)."`";
		$add = array();
		$fld = $this->_transform_field_definition($tablename,$definition);
		$add[] = "add ".$fld[0];
		if (count($fld) > 1) $add[] = "add ".$fld[1];
		if (count($fld) > 2) $add[] = "add ".$fld[2];
		return $this->query($q.join(",",$add)); 
	}
		
	public function UndefineField($tablename, $fieldname){
		return $this->query("alter table `".$this->RealTableName($tablename)."` drop column `".$fieldname."`");
	}
		
	public function DefineIndex($tablename, TIndexDefinition $definition){
		return $this->query("alter table `".$this->RealTableName($tablename)."` add ".$this->_transform_index_definition($definition)); 
	}
		
	public function UndefineIndex($tablename, $indexname){
		return $this->query("alter table `".$this->RealTableName($tablename)."` drop index `".$indexname."`");
	}
		
	public function DefineForeignKey($tablename, TForeignKeyDefinition $definition){
		return $this->query("alter table `".$this->RealTableName($tablename)."` add ".$this->_transform_fk_definition($tablename,$definition)); 
	}
	
	private function _prepare_record_value(&$value){
		$value = $this->quoteValue($value);
	}
		
	private function _prepare_record(&$record){
		array_walk($record,array(&$this,"_prepare_record_value"));
		$record = "(".join(",",$record).")";
	}
		
	private function _prepare_record_result($record){
		array_walk($record,array(&$this,"_prepare_record_value"));
		return $record;
	}
	
	public function InsertRecords(TTable $table,array $records, array $fields = array(),&$primary_key = null){
		$this->_auto_connect();
		$q1 = "insert ".$this->RealTableName($table->Name)." ";
		if (count($fields))
			$q1 .= "(".join(",",$fields).") ";
		array_walk($records,array(&$this,"_prepare_record"));
		$oldid = $this->lastInsertId();
		$this->BeginTransaction();
		try {
			$i = 0;
			$n = count($records);
			while ($i < $n){
				$this->query($q1."values ".join(",",array_slice($records,$i,$this->BulkInsertAmount)));
				$i = $i + $this->BulkInsertAmount;
			}
		} catch (Exception $e) {
			$this->RollbackTransaction();
			throw $e;
			return false;
		}
		$primary_key = $this->lastInsertId();
		$this->CommitTransaction();
		$primary_key = ($primary_key === $oldid)?null:$primary_key;
		return true;
	}
	
	public function DeleteRecords(TDataSource $records){
		$p = new TMySqlDataSetProcessor($records, $this);
		/*foreach ($p->TargetSources as $ds)
			if (!($ds instanceof TTable)){
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				return 0;
			}*/
		return $this->Execute($p->DeleteQuery(),$p->Parameters);
	}
	
	public function UpdateRecords(TDataSource $records,  array $fields, array $values){
		$p = new TMySqlDataSetUpdateProcessor($records, $this, $fields, $values);
		return $this->Execute($p->UpdateQuery(),$p->Parameters);
	}
/**
 * @return IIterator<array>
 */	
	public function FetchRecords(TDataSource $records){
		$p = new TMySqlDataSetProcessor($records, $this);
		return $this->Fetch($p->SelectQuery(),$p->Parameters);
	}

	private function _ns_findparent(TNestedSets $target, TDataSource $parents){		
		$result = array(null,null,-1);
		$f= $parents->Filter;
		$j = $parents->Join;
		if (!empty($f) || !empty($j)){
			$target->Fetch(array(new TNestedSetsField(TMySqlDBDriver::NS_LKEY_FIELD),new TNestedSetsField(TMySqlDBDriver::NS_RKEY_FIELD),new TNestedSetsField(TMySqlDBDriver::NS_LEVEL_FIELD)));
			$p = new TMySqlDataSetProcessor($parents, $this);
			if ($res = $this->Fetch($p->SelectQuery(),$p->Parameters))
				foreach ($res as $item){
					$result[0] = $item[TMySqlDBDriver::NS_LKEY_FIELD];
					$result[1] = $item[TMySqlDBDriver::NS_RKEY_FIELD];
					$result[2] = $item[TMySqlDBDriver::NS_LEVEL_FIELD];
					break;
				}
		}
		if (is_null($result[1])) {
			$q = "select max(".TMySqlDBDriver::NS_RKEY_FIELD.") + 1 as prk from ".$this->RealTableName($target->Name);
			$result[1] = $this->scalarValue($q);				
			if (is_null($result[1])) $result[1] = 1;
		}
		return $result;
	}
				
	private function _prepare_ns_bulkkeys(array &$item, &$lkey, $level){
		$item["lkey"] = $lkey;
		$item["level"] = $level;
		
		if (count($item[1]) == 0){
			$item["rkey"] = $lkey + 1;
			$lkey = $lkey + 2;
		} else {
			$lkey++;
			foreach ($item[1] as &$ch)
				$this->_prepare_ns_bulkkeys($ch,$lkey,$item["level"] + 1);
			$item["rkey"] = $lkey;
			$lkey++;	
		}
	}

	private function _prepare_ns_bulkrows(array &$item,array &$rows){
		$row = array_merge($item[0],array($item["lkey"],$item["rkey"],$item["level"]));
		$this->_prepare_record($row);
		$rows[] = $row;
		foreach ($item[1] as $ch)
			$this->_prepare_ns_bulkrows($ch,$rows);
	}
		
	public function NestedSetBulkInsert(TDataSource $parent, array &$sets, array $fields){
		$this->_auto_connect();
		$target = $this->_get_nstarget($parent);
		$tname = $this->RealTableName($target->Name);
		$this->query("lock tables ".$tname." write");
		list($plk,$prk,$plevel) = $this->_ns_findparent($parent, $parent);
		
		$tprk = $prk;
		
		foreach ($sets as &$s)
			$this->_prepare_ns_bulkkeys($s,$tprk,$plevel + 1);	
		
		$delta = $tprk - $prk;
		
		$rows = array();
		foreach ($sets as &$s)
			$this->_prepare_ns_bulkrows($s,$rows);

		$this->BeginTransaction();
		try {
			$this->Execute("update ".$tname." set ".TMySqlDBDriver::NS_RKEY_FIELD." = :rk + ".$delta.", ".TMySqlDBDriver::NS_LKEY_FIELD." = if(".TMySqlDBDriver::NS_LKEY_FIELD." >= :rk,".TMySqlDBDriver::NS_LKEY_FIELD." + ".$delta.",".TMySqlDBDriver::NS_LKEY_FIELD.") where ".TMySqlDBDriver::NS_RKEY_FIELD." >= :rk",array("rk"=>$prk));
			
			$i = 0;
			$n = count($rows);
			while ($i < $n){
				$q = "insert ".$tname." (".join(",",$fields).",".TMySqlDBDriver::NS_LKEY_FIELD.",".TMySqlDBDriver::NS_RKEY_FIELD.",".TMySqlDBDriver::NS_LEVEL_FIELD.") values ".join(",",array_slice($rows,$i,$this->BulkInsertAmount));
				$this->query($q);
				$i = $i+$this->BulkInsertAmount;
			}
			$this->CommitTransaction();
			$this->Execute("unlock tables");
		} catch (Exception $e){
			$this->RollbackTransaction();
			$this->Execute("unlock tables");
			throw $e;
			return false;
		}
		return true;
	}
	
	private function _get_nstarget(TDataSource $ds){
		$targets = $ds->Targets;
		$result = null;
		foreach ($targets as $t)
			if (($t->IsTarget) && ($t instanceof TNestedSets)){
				$result = $t;
				break;
			}
		if (is_null($result))	
			if ($ds instanceof TNestedSets)
				$result = $ds;	
		if (is_null($result)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
		return $result;			
	}
	
	public function NestedSetsSysField($name,$field){
		if ($this->_json_ns_names_)
			if (isset($this->_json_ns_names_->$name))
				if (isset($this->_json_ns_names_->$name->$field))
					return $this->_json_ns_names_->$name->$field;
		return constant('TMySqlDBDriver::'.$field); 			
	}
		
	public function NestedSetAddRecord(TDataSource $parents, array $record,&$primary_key = null){
		$this->_auto_connect();
		$target = $this->_get_nstarget($parents);
		$tname = $this->RealTableName($target->Name);
		$this->query("lock tables ".$tname." write");
		list($plk,$prk,$plevel) = $this->_ns_findparent($target, $parents);
		$result = false;
		$this->BeginTransaction();
		try {
		$this->Execute("update ".$tname." set ".TMySqlDBDriver::NS_RKEY_FIELD." = ".TMySqlDBDriver::NS_RKEY_FIELD." + 2, ".TMySqlDBDriver::NS_LKEY_FIELD." = if(".TMySqlDBDriver::NS_LKEY_FIELD." >= :rk,".TMySqlDBDriver::NS_LKEY_FIELD." + 2,".TMySqlDBDriver::NS_LKEY_FIELD.") where ".TMySqlDBDriver::NS_RKEY_FIELD." >= :rk", array("rk"=>$prk));
		$oldid = $this->lastInsertId();
		$this->Execute("insert ".$tname." (".join(",",array_keys($record)).",".TMySqlDBDriver::NS_LKEY_FIELD.",".TMySqlDBDriver::NS_RKEY_FIELD.",".TMySqlDBDriver::NS_LEVEL_FIELD.") values (".join(",",$this->_prepare_record_result(array_values($record))).",:lk,:rk,:pl)",array("lk"=>$prk,"rk"=>$prk+1,"pl"=>$plevel+1));
		$primary_key = $this->lastInsertId();
		if ($primary_key == $oldid) $primary_key = null; 
		$this->CommitTransaction();
		$this->query("unlock tables");
		$result = true;
		} catch (Exception $e){
			$this->RollbackTransaction();
			$this->query("unlock tables");
			throw $e;
			$result = false;
		}
		return $result;
	}
	
	private function _ns_eject_branches($target, TDataSource $branches){
			$p = new TMySqlDataSetUpdateProcessor($branches, $this,array(TMySqlDBDriver::NS_DELETED_FIELD),array(1));
			$this->Execute($p->UpdateQuery(),$p->Parameters);
			
			$roots = $this->Fetch("select b.".TMySqlDBDriver::NS_LKEY_FIELD.",b.".TMySqlDBDriver::NS_RKEY_FIELD.",b.".TMySqlDBDriver::NS_LEVEL_FIELD." from ".$target." b left join ".$target." b1 on (b1.".TMySqlDBDriver::NS_DELETED_FIELD." = 1) and (b.".TMySqlDBDriver::NS_LKEY_FIELD." > b1.".TMySqlDBDriver::NS_LKEY_FIELD.") and (b.".TMySqlDBDriver::NS_RKEY_FIELD." < b1.".TMySqlDBDriver::NS_RKEY_FIELD.")  where (b.".TMySqlDBDriver::NS_DELETED_FIELD." = 1) and (b1.".TMySqlDBDriver::NS_LKEY_FIELD." is null) order by b.".TMySqlDBDriver::NS_RKEY_FIELD." desc");
			$ranges = array();
			$prev_range = -1;
			foreach ($roots as $root){
				if ($prev_range > -1){
					if ($root[TMySqlDBDriver::NS_RKEY_FIELD] == $ranges[$prev_range][0] - 1){
						$ranges[$prev_range][0] = $root[TMySqlDBDriver::NS_LKEY_FIELD];
						continue;	
					}
				} 
				$ranges[] = array($root[TMySqlDBDriver::NS_LKEY_FIELD],$root[TMySqlDBDriver::NS_RKEY_FIELD],$root[TMySqlDBDriver::NS_LEVEL_FIELD]);
				$prev_range++;
			}
			
			foreach ($ranges as $range){
				$this->Execute("update ".$target." set ".TMySqlDBDriver::NS_DELETED_FIELD." = 1 where (".TMySqlDBDriver::NS_LKEY_FIELD." >= :lk) and (".TMySqlDBDriver::NS_RKEY_FIELD." <= :rk)", array("lk"=>$range[0],"rk"=>$range[1]));
				$this->Execute("update ".$target." set ".TMySqlDBDriver::NS_LKEY_FIELD." = if(".TMySqlDBDriver::NS_LKEY_FIELD." > :rk,".TMySqlDBDriver::NS_LKEY_FIELD." - :rk + :lk - 1,".TMySqlDBDriver::NS_LKEY_FIELD."),".TMySqlDBDriver::NS_RKEY_FIELD." = ".TMySqlDBDriver::NS_RKEY_FIELD." - :rk + :lk - 1 where (".TMySqlDBDriver::NS_RKEY_FIELD." > :rk) and (".TMySqlDBDriver::NS_DELETED_FIELD." = 0)", array("lk"=>$range[0],"rk"=>$range[1]));
			}
			return $ranges;
	}
	
	public function NestedSetBranchesDelete(TDataSource $branches){
		$this->_auto_connect();
		$result = 0;
		$target = $this->_get_nstarget($branches);
		$tname = $this->RealTableName($target->Name);
		$this->query("lock tables ".$tname." write");
		$this->BeginTransaction();
		try {
			$this->_ns_eject_branches($tname, $branches);
			$result = $this->Execute("delete from ".$target->Name." where ".TMySqlDBDriver::NS_DELETED_FIELD." = 1");
			$this->CommitTransaction();
			$this->query("unlock tables");
			$result = true;
		} catch (Exception $e){
			$this->RollbackTransaction();
			$this->query("unlock tables");
			throw $e;
			$result = false; 
		}
		return $result;		
	}	
	
	public function NestedSetBranchesMove(TDataSource $branches, TDataSource $destination){
		$this->_auto_connect();
		$result = 0;
		$target = $this->_get_nstarget($branches);
		$dest_target = $this->_get_nstarget($destination);
		if ($target->Name != $dest_target->Name)
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		
		$tname = $this->RealTableName($target->Name);
		$this->query("lock tables ".$tname." write");
		$this->BeginTransaction();
		try {
			$ranges = $this->_ns_eject_branches($tname, $branches);
			$destination->Filter(new TCondition(TCondition::C_EQUAL, array(new TNestedSetsField(TMySqlDBDriver::NS_DELETED_FIELD,TNestedSetsField::BASE_FIELD,null,$dest_target),1)));
			list($plk,$prk,$plevel) = $this->_ns_findparent($dest_target, $destination);
			
			if (is_null($plk)) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			
			$delta = 0;
			$n = count($ranges);
			for ($i = $n - 1; $i >= 0; $i--){
				$r = $ranges[$i];
				$this->Execute("update ".$tname." set ".TMySqlDBDriver::NS_LKEY_FIELD." = ".TMySqlDBDriver::NS_LKEY_FIELD." - :lk + :delta + 1, ".TMySqlDBDriver::NS_RKEY_FIELD." = ".TMySqlDBDriver::NS_RKEY_FIELD." - :lk + :delta + 1, ".TMySqlDBDriver::NS_LEVEL_FIELD." = ".TMySqlDBDriver::NS_LEVEL_FIELD." - :level + 1 where (".TMySqlDBDriver::NS_DELETED_FIELD." = 1) and (".TMySqlDBDriver::NS_LKEY_FIELD." >= :lk) and (".TMySqlDBDriver::NS_RKEY_FIELD." <= :rk)", array("lk"=>$r[0],"rk"=>$r[1],"level"=>$r[2],"delta"=>$delta)); 
				$delta += $r[1] - $r[0] + 1;
			}
			
			$this->Execute("update ".$tname." set ".TMySqlDBDriver::NS_RKEY_FIELD." = ".TMySqlDBDriver::NS_RKEY_FIELD." + :delta, ".TMySqlDBDriver::NS_LKEY_FIELD." = if(".TMySqlDBDriver::NS_LKEY_FIELD." >= :lk,".TMySqlDBDriver::NS_LKEY_FIELD." + :delta,".TMySqlDBDriver::NS_LKEY_FIELD.") where (".TMySqlDBDriver::NS_RKEY_FIELD." >= :lk) and (".TMySqlDBDriver::NS_DELETED_FIELD." = 0)", array("lk"=>$plk,"delta"=>$delta));
			$this->Execute("update ".$tname." set ".TMySqlDBDriver::NS_RKEY_FIELD." = ".TMySqlDBDriver::NS_RKEY_FIELD." + :lk, ".TMySqlDBDriver::NS_LKEY_FIELD." = ".TMySqlDBDriver::NS_LKEY_FIELD." + :lk, ".TMySqlDBDriver::NS_LEVEL_FIELD." = ".TMySqlDBDriver::NS_LEVEL_FIELD." + :level, ".TMySqlDBDriver::NS_DELETED_FIELD." = 0 where (".TMySqlDBDriver::NS_DELETED_FIELD." = 1)", array("lk"=>$plk,"level"=>$plevel));
			
			$this->CommitTransaction();
			$this->query("unlock tables");
			$result = true;
		} catch (Exception $e){
			$this->RollbackTransaction();
			$this->query("unlock tables");
			throw $e;
			$result = false;
		}
		return $result;		
	}
/**
 * fetches nestings
 * @return IIterator<TNestedSetElement>
 */			
	public function NestedSetFetch(TDataSource $records){
		$p = new TMySqlDataSetProcessor($records, $this);
		return new TMySqlNestingsIterator($this->Fetch($p->SelectQuery(),$p->Parameters));			
	}	
}
?>