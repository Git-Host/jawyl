<?php
/**
 * @package Common
 * @subpackage Storage
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
require_once 'common/iterator.inc';
require_once 'common/data.inc';
require_once 'common/logic.inc';
require_once 'common/acid.inc';

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * class for storage exceptions
 */
class TStorageException extends TException {
/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			default:return "";break;		
		}
	}
}

final class TStorageOptions {
	const FOR_TRANSACTIONS = 0;
	const FOR_FETCHING = 1;
	const OPTIMIZE_SPEED = 2;
	const OPTIMIZE_SIZE = 4;

	public static function CheckOption($option){
		return ($option == self::FOR_TRANSACTIONS) || ($option ==  self::FOR_FETCHING) || ($option ==  self::OPTIMIZE_SIZE) || ($option == self::OPTIMIZE_SPEED);
	}
}

final class TTableReferenceType extends TDataType {
	const REFERENCE = "REFER";
	
	const REL_CASCADE = 0;
	const REL_RESTRICT = 1;
	const REL_SETNULL = 2;
	const REL_NOACTION = 3;
	const REL_SETDEFAULT = 4;

/** 
 * @var TTableField
 */	
	public $MasterField;
	public $OnDelete;
	public $OnUpdate;
	public $ReferenceType;
		
	public function __construct(TTableField $field, TDataType $reftype, $ondelete = TTableReferenceType::REL_CASCADE,$onupdate = TTableReferenceType::REL_RESTRICT){
		$this->MasterField = $field;
		$this->ReferenceType = $reftype;
		$this->TypeCode = self::REFERENCE;
		$this->OnDelete = $ondelete;
		$this->OnUpdate = $onupdate;
	} 
		
	public function __toString(){
		return parent::__toString().";masterfield=".$this->MasterField->TableName.".".$this->MasterField->Name.";datatype=".base64_encode($this->ReferenceType->__toString()).";onupdate=".$this->OnUpdate.";ondelete=".$this->OnDelete;
	} 
}


/**
 * @property string $Name
 * @property string $Alias
 * @property TDataSource $DataSource
 */

interface IDataSourceField {
/**
 * @return string
 */	
	public function Name();
/**
 * @return IDataSet
 */	
	public function DataSource();
/**
 * @return string
 */	
	public function Alias();
	
	public function AttachTo(TDataSource $datasource);
}

/**
 * @property string $Name
 * @property array $Fields
 * @property array $Filter
 * @property array $Join
 * @property array $Sorting
 * @property boolean $IsTarget
 * @property int $Offset
 * @property int $Count
 * @property string $Alias
 * @property array $Targets
 * @property array $UsedFields
 * @property bool $ForceUniqueness
 */
abstract class TDataSource {
	protected $fields = array();
	protected $conditions = array();
	protected $joins = array();
	protected $isTarget = false;
	protected $offset = null;
	protected $count = null;
	protected $alias = null;
	protected $sorting = array();
	protected $forceUniqueness = false;
	
	private $_attached_fields_ = array();
	
	protected $_targets_;
	
	public function ExplicitTargets(){
		$result = array();
		if ($this->isTarget)
			$result[] = $this;
		if (!empty($this->joins))
			foreach ($this->joins as $j)
				$result = array_merge($result,$j->Source->Targets());
		return $result;		
	}
	
	public function Find($property, $value){
		$result = array();
		if ($this->_get($value) === $value)
			$result[] = $this;
			if (!empty($this->joins))
				foreach ($this->joins as $j)
					$result = array_merge($result,$j->Source->Find($property,$value));
		return $result;	
	}
	
	public function AttachField(IDataSourceField $fld){
		$id = $fld->Alias?$fld->Alias:$fld->Name;
		if (!key_exists($id,$this->_attached_fields_)){
			$this->_attached_fields_[$id] = $fld;
			if ($fld->DataSource !== $this)
				$fld->AttachTo($this);
		}
	}
	
/**
 * @return string
 */	
	public abstract function Name();
	
	public function __construct($alias = null, array $fields = array(), array $filter = array(), array $join = array(), array $sorting = array(), $istarget = false,$offset = null,$count = null,$force_unique = false){
		$this->alias = $alias;
		$this->Fetch($fields)->Filter($filter)->Join($join)->Sort($sorting)->SetOffset($offset)->SetCount($count);
		$this->isTarget = TConvertions::ConvertToBoolean($istarget);
		$this->forceUniqueness = $force_unique;
	}
	
	public function Add(TDataSource $datasource){
		return new TDataSet(array($this,$datasource));
	}
	
	public final function Fetch($f){
		$flds = func_get_args();
		foreach ($flds as $fld){
			if (is_array($fld)){
				foreach ($fld as $f1) $this->Fetch($f1);
			} else if ($fld instanceof IDataSourceField){
				if (is_null($fld->DataSource))
					$fld->AttachTo($this);
				$this->fields[] = $fld;
			} else if ($fld instanceof TOperation){
				$fld->AttachFields($this);
				$this->fields[] = $fld;
			}
		}
		return $this;
	}
	
	public final function Filter($c){
		$conds = func_get_args();
		foreach ($conds as $cond){
			if (is_array($cond)){
				foreach ($cond as $c1) $this->Filter($c1);
			} else if ($cond instanceof TCondition){
				$cond->attachFields($this);
				$this->conditions[] = $cond;
			}
		}
		return $this;
	}
	
	private function _join_array(array $j){
		$n = count($j);
		$i = 0;
		while ($i < $n){
			if (is_array($j[$i])) $this->_join_array($j[$i]);
			else if ($j[$i] instanceof TJoin) $this->joins[] = $j[$i];
			else if (($j[$i] instanceof TDataSource) || (is_string($j[$i]))){
				if ($i + 2 < $n){
					$c = $j[$i + 2];
					if (is_array($c))
						if (count($c) == 0)
							$c = null;	
					if ($c)	
						$this->joins[] = new TJoin($j[$i], $j[$i + 1], $j[$i + 2]);
					else throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
					$i += 2;
				} else throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
			}
			$i++;
		}
	} 
	
	public final function Join($j){
		unset($this->targetsCache);
		$this->_join_array(func_get_args());
		return $this;
	}
	
	public function Sort($s){
		$sorts = func_get_args();
		foreach ($sorts as $sort){
			if (is_array($sort)){
				foreach ($sort as $s1) $this->Sort($s1);
			} else if ($sort instanceof TSorting)
				$this->sorting[] = $sort;
		}
		return $this;
	}	
	
	public final function SetOffset($offset){
		if (TConvertions::CheckInteger($offset) || is_null($offset))
			$this->offset = $offset;
		else throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		return $this;	
	}
	
	public final function SetCount($count){
		if (TConvertions::CheckInteger($count) || is_null($count))
			$this->count = $count;
		else throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		return $this;	
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Fields":$this->Fetch($value);break;
			case "Filter":$this->Filter($value);break;
			case "Join":$this->Join($value);break;
			case "Sorting":$this->Sort($value);break;
			case "IsTarget":$this->isTarget = TConvertions::ConvertToBoolean($value);break;
			case "Offset":$this->SetOffset($value);break;
			case "Count":$this->SetCount($value);break;
			case "ForceUniqueness":$this->forceUniqueness = $value;break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case "Name":return $this->Name();
			case "Alias":return $this->alias;
			case "Fields":return $this->fields;break;
			case "Filter":return $this->conditions;break;
			case "Join":return $this->joins;break;
			case "Sorting":return $this->sorting;break;
			case "IsTarget":return $this->isTarget;break;
			case "Offset":return $this->offset;break;
			case "Count":return $this->count;break;
			case "ForceUniqueness":return $this->forceUniqueness;break;
			case "Targets":{
				if (!is_array($this->targetsCache))
					$this->_targets_ = $this->ExplicitTargets();
				return $this->_targets_;
			}break;
			case "UsedFields":return $this->_attached_fields_;break;
		}
	}
}

/**
 * @property string $Name
 * @property TDataSource $DataSource
 * @property string $Alias
 */
class TDataSourceField implements IDataSourceField {
	protected $name;
	protected $datasource = null;
	protected $alias;
	
	public function __construct($name,$alias = null,$datasource = null){
		$this->name = $name;
		$this->alias = $alias;
		if ($datasource instanceof TDataSource)
			$this->AttachTo($datasource);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Name":return $this->Name();break;
			case "DataSource":return $this->DataSource();break;
			case "Alias":return $this->Alias();break;
		}
	}
	
	public function AttachTo(TDataSource $datasource){
		if ($this->datasource !== $datasource){
			$this->datasource = $datasource;
			$this->datasource->AttachField($this);
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "DataSource":$this->AttachTo($value);break;
		}
	}
	
	public function Name(){return $this->name;}
	public function DataSource(){return $this->datasource;}
	public function Alias(){return $this->alias;}
}


/**
 *
 */
class TTable extends TDataSource {
	protected $name;

	public function __construct($name, $alias = null, array $fields = array(), array $filter = array(), array $join = array(), array $sorting = array(), $istarget = false, $offset = null, $count = null, $force_unique = false){
		$this->name = $name;
		parent::__construct($alias,$fields,$filter,$join,$sorting,$istarget,$offset,$count,$force_unique);
	}	
	
	public function Name(){
		return $this->name;
	}
	
	public function AttachField(IDataSourceField $fld){
		if (!(($fld instanceof TTableField) || ($fld instanceof TExpressionField) || ($fld instanceof TAllField)))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		parent::AttachField($fld);
	}
	
	public function __toString(){return (string)$this->Name();}
}

/**
 *
 */
class TTableField extends TDataSourceField {
	public function AttachTo(TDataSource $datasource){
		if (!($datasource instanceof TTable))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
		parent::AttachTo($datasource);
	}	
}

/**
 * 
 * @property int $Direction
 * @property int $Depth
 * @property boolean $IncludeBases
 */

class TNestedSets extends TTable {
	const DIR_DOWN = 0;
	const DIR_UP = 1;
		
	protected $direction = self::DIR_DOWN;
	protected $depth = null;
	protected $includeBases = false;
	protected $basesNeeded = false;

	protected function setDirection($dir){
		if (($dir == self::DIR_DOWN) || ($dir == self::DIR_UP))
			$this->direction = $dir;	
	}
	
	public function AttachField(IDataSourceField $fld){
		if (!(($fld instanceof TTableField) || ($fld instanceof TExpressionField) || ($fld instanceof TAllField)))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		if ($fld instanceof TNestedSetsField)	
			if ($fld->Type == TNestedSetsField::BASE_FIELD)
				$this->basesNeeded = true;
		parent::AttachField($fld);
	}
	
	public function __construct($name, $direction = null, $depth = null, $includebases = null, $alias = null, array $fields = array(), array $filter = array(), array $join = array(),array $sorting = array(), $istarget = false, $offset = null, $count = null, $force_unique = false){
		parent::__construct($name, $alias,$fields,$filter,$join,$sorting,$istarget,$offset,$count,$force_unique);
		if (!is_null($direction))
			$this->setDirection($direction);
		if (is_int($depth))
			$this->depth = $depth;
		if (is_bool($includebases))
			$this->includeBases = $includebases; 		
	}
	
	public function __get($nm){
		switch ($nm){
			case "Direction":return $this->direction;break;
			case "Depth":return $this->depth;break;
			case "IncludeBases":return $this->includeBases;break;
			case "BasesNeeded":return $this->basesNeeded;break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Direction":$this->setDirection($value);break;
			case "Depth":if (TConvertions::CheckInteger($value)) $this->depth = $value;break;
			case "IncludeBases":$this->includeBases = TConvertions::ConvertToBoolean($value);break;
			default:parent::__set($nm,$value);break;
		}
	}
}

/**
 * @property int $Type
 */

class TNestedSetsField extends TTableField {
	const SUBSET_FIELD = 0;
	const BASE_FIELD = 1;
	
	private $_type_ = TNestedSetsField::SUBSET_FIELD;
	
	public function AttachTo(TDataSource $datasource){
		if (!($datasource instanceof TNestedSets))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
		parent::AttachTo($datasource);
	}

	public function __construct($name,$type = null, $alias = null, $datasource = null){
		if (($type == self::BASE_FIELD) || ($type == self::SUBSET_FIELD))
			$this->_type_ = $type;
		parent::__construct($name,$alias,$datasource);	
	}	
	
	public function __get($nm){
		switch ($nm){
			case "Type":return $this->_type_;break;
			default:return parent::__get($nm);break;
		}
	}
}

final class TNestedSetsAllField extends TNestedSetsField {
	public function __construct($datasource = null, $type = null){
		parent::__construct("all", $type, null, $datasource);	
	}	
}

/**
 * @property array $Sources 
 */

final class TDataSet extends TDataSource {
	protected $source = array();
	
	public function __construct($source, $alias = null, array $fields = array(),array $filter = array(), array $join = array(), array $sorting = array(), $istarget = false,$offset = null,$count = null, $force_unique = false){
		parent::__construct($alias,$fields,$filter,$join,$sorting,$istarget,$offset,$count,$force_unique);
		$this->AddSource($source);
	}
	
	public function Name(){
		return $this->Alias;
	}

	public function ExplicitTargets(){
		$result = parent::Targets();
		if (!empty($this->source))
			foreach ($this->source as $s)
				$result = array_merge($result,$s->Targets());
		return $result;			
	}
	
	public function Find($property, $value){
		$result = parent::Find($property, $value);
		if (!empty($this->source))
			foreach ($this->source as $s)
				$result = array_merge($result,$s->Find($property,$value));
		return $result;	
	}
	
	public function Add(TDataSource $datasource){
		$this->AddSource($datasource);
		return $this;
	}
	
	public function AddSource($s){
		unset($this->targetsCache);
		$sources = func_get_args();
		foreach ($sources as $src){
			if (is_array($src)){
				foreach ($src as $s1) $this->AddSource($s1);
			} else if ($src instanceof TDataSource)
				$this->source[] = $src;
		}
		return $this;		
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Sources":$this->AddSource($value);break;
			default:parent::__set($nm, $value);break;
		}
	}	
	
	public function __get($nm){
		switch ($nm){
			case "Sources":return $this->source;break;
			default:return parent::__get($nm);break;
		}
	}
}

/** 
 * @property TOperation $Expression
 */
final class TExpressionField extends TDataSourceField {
	private $_expression_;
	
	public function __construct($alias, TOperation $expr, $datasource = null){
		$this->_expression_ = $expr;
		parent::__construct($alias,$alias,$datasource);
	}	
	
	public function __get($nm){
		switch ($nm){
			case "Expression":return $this->_expression_;break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function AttachTo(TDataSource $datasource){
		parent::AttachTo($datasource);
		$this->_expression_->AttachFields($datasource);
	}	
}

final class TAllField extends TDataSourceField {
	public function __construct($datasource = null){
		parent::__construct("all",null,$datasource);
	}
}

/**
 *
 * @property TTableField $Field
 * @property mixed $Value
 *
 */


final class TTableFieldValue {
/**
 * @var TTableField
 */	
	private $_field_;
	private $_value_;
	
	public function __construct(TTableField $field, $value){
		$this->_field_ = $field;
		$this->_value_ = $value;
	}
	
	public function __get($nm){
		switch ($nm){
			case "Field":return $this->_field_;break;
			case "Value":return $this->_value_;break;
		}
	}
}

/**
 * @property IDataSourceField $Field
 * @property int $Sort
 *
 */

class TSorting {
/**
 * @var TTableField
 */	
	private $_field_;
	private $_sort_;
	
	public function __construct(IDataSourceField $field, $sort = TSortingType::SORT_ASC){
		$this->_field_ = $field;
		$this->_sort_ = new TSortingType($sort);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Field":return $this->_field_;break;
			case "Sort":return (int)$this->_sort_->__toString();break;
		}
	}
}

final class TDBActionParameter {
	private $_name_ = "";
	private $_value_ = null;
	
	public function __construct($name,$value = null){
		$this->_name_ = $name;
		$this->_value_ = $value;
	}
	
	public function __get($nm){
		switch ($nm){
			case "Name":return $this->_name_;break;
			case "Value":return $this->_value_;break;
		}
	}
}

abstract class TNestedSetElement {
	protected $row;
	protected $parent;
	
	public function __construct(array $row, TNestedSetElement $parent = null){
		$this->row = $row;
		$this->parent = $parent;
	}	
	
	public function __get($nm){
		if (key_exists($nm,$this->row))
			return $this->row[$nm];
		return null;
	}
	
	public function NestingParent(){
		return $this->parent;
	}
}

/**
 * @property int $Type
 * @property array $Operands
 * @author dakrasilnikov
 *
 */

class TOperation {	
	/**
 * @var int
 */	
	protected $type;
/**
 * @var array
 */	
	protected $operands = array();
	
	protected function checkType($type){
		return new TOperationType($type);
	}
	
/**
 * constructor
 * @param int $conditiontype one of TCondition constants
 * @param array $operands condition operands, array which can contain php values, IAttributeDefinition or TSysField
 */	
	public function __construct($type, array $operands){
		$this->type = (string)$this->checkType($type);
		$this->type = (int)$this->type;
		foreach ($operands as $o)
			$this->addOperand($o);
	}
	
	protected function addOperand($o){
		if (is_string($o))
			if (preg_match('/^\:(\w+)$/',$o,$matches))
				$o = new TDBActionParameter($matches[1]);
		$this->operands[] = $o;
	}
	
/**
 * gets condition operands
 * @return array
 */	
	public function Operands(){return $this->operands;}
/**
 * gets condition type
 * @return int
 */	
	public function Type(){return $this->type;}
	
	public function __get($nm){
		switch ($nm){
			case "Type":return $this->Type();break;
			case "Operands":return $this->Operands();break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Operands":$this->addOperand($value);break;
		}
	}
	
	public function AttachFields(TDataSource $datasource){
		foreach ($this->operands as $o){
			if ($o instanceof IDataSourceField){
				if (is_null($o->DataSource))
					$o->AttachTo($datasource);
			} else if ($o instanceof TOperation)
				$o->AttachFields($datasource);
		}
	}	
}

/**
 * @package Common
 * @subpackage Storage
 * @category System
 * class for storage query conditions
 * @property int $Type  
 * @property array $Operands
 */
class TCondition extends TOperation {
	protected function checkType($type){
		return new TConditionType($type);
	}
}

/**
 * @property int $Type
 * @property array $Conditions
 * @property TDataSource $Source
 */

final class TJoin {
	const JOIN_INNER = 0;
	const JOIN_LEFT = 1;
	const JOIN_RIGHT = 2;
	const JOIN_FULL = 3;
	
	private $_type_;
	private $_conditions_;
	private $_source_;
	
	private function _add_condition($c){
		if ($c instanceof TCondition){
			$c->AttachFields($this->_source_);
			$this->_conditions_[] = $c;
		}
		else if (is_array($c))
			foreach ($c as $c1) $this->_add_condition($c1);
		else throw new TCoreException(TCoreException::ERR_BAD_VALUE);		 
	}
	
	public function __construct($src, $type, $conditions){
		if (is_string($src))
			$src = new TTable($src);
		if (!($src instanceof TDataSource))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$this->_source_ = $src;
		if (!in_array($type,array(self::JOIN_INNER,self::JOIN_LEFT,self::JOIN_FULL,self::JOIN_RIGHT)))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$this->_type_ = $type;
		$this->_add_condition($conditions);
	}
	
	public function __get($nm){
		switch ($nm){
			case "Type":return $this->_type_;break;
			case "Conditions":return $this->_conditions_;break;
			case "Source":return $this->_source_;break;
		}
		return null;
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "Conditions":$this->_add_condition($value);break;
		}
	}
}

/**
 * @property string $Name
 * @property TDataType $Type
 * @property boolean $PrimaryKey
 * @property boolean $Unique
 * @property boolean $AutoIncrement
 * @property boolean $Nullable
 * @property mixed $Default
 */
class TFieldDefinition {
	protected $name;
	protected $type;
	protected $primaryKey;
	protected $unique;
	protected $autoAssigned;
	protected $nullable;
	protected $default;
	
	public function __construct($name, TDataType $type, $nullable = true, $default = null, $pk = false, $unique = false, $autoassigned = false){
		$this->name = $name;
		$this->type = $type;
		$this->primaryKey = $pk;
		$this->unique = $unique;
		$this->autoAssigned = $autoassigned;
		$this->nullable = $nullable;
		$this->default = $default;
	}
	
	public function __get($nm){
		switch ($nm){
			case "Name":return $this->name;break;	
			case "Type":return $this->type;break;	
			case "PrimaryKey":return $this->primaryKey;break;	
			case "Unique":return $this->unique;break;	
			case "AutoAssigned":return $this->autoAssigned;break;
			case "Nullable":return $this->nullable;break;	
			case "Default":return $this->default;break;
		}
	}
}

/**
 * 
 * @property string $Name
 * @property array $Fields
 * @property boolean $Unique
 * @property boolean $Primary
 *
 */

class TIndexDefinition {
	protected $name;
	protected $fieldNames;
	protected $unique;
	protected $primary;
	
	public function __construct($name,array $fields,$unique = false,$primary = false){
		$this->name = $name;
		$this->fieldNames = $fields;
		$this->unique = $unique;
		$this->primary = $primary;
	}
	
	public function __get($nm){
		switch ($nm){
			case "Name":return $this->name;break;	
			case "Fields":return $this->fieldNames;break;	
			case "Unique":return $this->unique;break;	
			case "Primary":return $this->primary;break;	
		}
	}
}

/**
 * 
 * @property string $Name
 * @property TTableField $MasterField
 * @property int $OnDelete
 * @property int $OnUpdate
 */

class TForeignKeyDefinition {
	protected $name;
	protected $masterField;
	protected $onDelete;
	protected $onUpdate;
	
	public function __construct($name, TTableField $field,$ondelete = TTableReferenceType::REL_CASCADE, $onupdate = TTableReferenceType::REL_RESTRICT){
		$this->name = $name;
		$this->masterField = $field;
		$this->onDelete = $ondelete;
		$this->onUpdate = $onupdate;
	}

	public function __get($nm){
		switch ($nm){
			case "Name":return $this->name;break;	
			case "MasterField":return $this->masterField;break;	
			case "OnDelete":return $this->onDelete;break;	
			case "OnUpdate":return $this->onUpdate;break;	
		}
	}
}

interface IAnsiSQLDBDriver {
/**
 * @param string $q
 * @param array $parameters
 * @return int
 */	
	public function Execute($q,array $parameters = array());

/**
 * @param string $q
 * @param array $parameters
 * @return IIterator<array>
 */	
	public function Fetch($q, array $parameters = array());
	
/**
 * @param string $q
 * @param array $parameters
 * @return mixed
 */		
	public function Scalar($q, array $parameters = array());
}


interface IDBUser {
	public function Uid();
	public function Login();
	public function Password();
}

interface IDBACLManager {
/**
 * @return IDBUser
 */	
	public function CurrentDBUser();
}


/**
 * @property int $AuthType
 * @property IDBACLManager $Acl
 * @property boolean $QueryDebug
 */
interface IDBDriver extends ITransactionProvider {	
	const AUTH_DEFAULT = 0;
	const AUTH_IMPLICIT_ACL = 1;
	const AUTH_EXPLICIT_ACL = 2;
/**
 * gets driver current connection parameters
 * @return array
 */		
	public function ConnectionParameters();	
/**
 * connects to datasource. returns true on success, false otherwise. 
 * @return boolean
 */				
	public function Connect();
		
	public function IsConnected();
/**
 * disconnects from current database
 */		
	public function DisConnect();

/**
 * @param string $tablename
 * @return boolean
 */		
	public function TableExists($tablename);
/**
 * 
 * @param string $tablename
 * @return string
 */	
	public function RealTableName($tablename);
/**
 * @param string $tablename
 * @param array $definitions
 * @return boolean
 */		
	public function DefineTable($tablename, array $definitions, array $options = array());
/**
 * @param string $tablename
 * @return boolean
 */		
	public function UndefineTable($tablename);
	
/**
 * @param string $tablename
 * @param TFieldDefinition[] $definitions
 * @return boolean
 */		
	public function DefineFields($tablename, array $definitions);
	
/**
 * @param string $tablename
 * @param string[] $fields
 * @return boolean
 */		
	public function UndefineFields($tablename, array $fields);
/**
 * @param string $tablename
 * @return boolean
 */		
	public function DefineIndex($tablename, TIndexDefinition $definition);
/**
 * @param string $tablename
 * @return boolean
 */		
	public function UndefineIndex($tablename, $indexname);
/**
 * @param string $tablename
 * @return boolean
 */		
	public function UndefineForeignKey($tablename, $fieldname);
/**
 * @param string $tablename
 * @return boolean
 */		
	public function DefineForeignKey($tablename, TForeignKeyDefinition $definition);
/**
 * @param string $name
 * @return boolean
 */		
	public function DefineNestedSets($name, array $definitions, array $options = array());
/**
 * @param string $name
 * @return boolean
 */		
	public function UndefineNestedSets($name);
/**
 * @param string $name
 * @return boolean
 */		
	public function NestedSetsDefined($name);
/**
 * fetches nestings
 * @return IIterator<TNestedSetElement>
 */		
	public function NestedSetFetch(TDataSource $records);
/**
 * @return boolean
 */		
	public function NestedSetAddRecord(TDataSource $parents, array $record,&$primary_key = null);
/**
 * @return boolean
 */		
	public function NestedSetBulkInsert(TDataSource $parent, array &$sets, array $fields);
/**
 * @return boolean
 */		
	public function NestedSetBranchesDelete(TDataSource $branches);
/**
 * @return boolean
 */	
	public function NestedSetBranchesMove(TDataSource $branches, TDataSource $destination);	
/**
 * string[] $fields
 * @return boolean
 */	
	public function InsertRecords(TTable $table,array $records, array $fields = array(),&$primary_key = null);
/**
 * @param int
 */	
	public function DeleteRecords(TDataSource $records);
/**
 * @param IDataSourceField[] $fields
 * @return int 
 */	
	public function UpdateRecords(TDataSource $records, array $fields, array $values);

/**
 * @return IIterator
 */	
	public function FetchRecords(TDataSource $records);

}

/**
 * @property int $AuthType
 * @property IDBACLManager $Acl
 */
abstract class TDBDriver extends TConfigurable implements IDBDriver {
	private $_auth_type_ = IDBDriver::AUTH_DEFAULT;
/**
 * @var IDBACLManager
 */	
	protected $_ioc_acl_;
	
	public function __construct($name, IConfigurable $container = null){
		if ($container)
			parent::__construct($name,$container);
		else
			parent::__construct($name);
	}

	public function __get($nm){
		switch ($nm){
			case "AuthType":return $this->_auth_type_;break;
			case "Acl":{
				$result = parent::__get($nm);
				if ($this->_auth_type_ == IDBDriver::AUTH_EXPLICIT_ACL)
					return $result;
				if ($this->_auth_type_ == IDBDriver::AUTH_IMPLICIT_ACL){
					if (!$result){
						$service = $this->Application()->CurrentService();
						if ($service instanceof ISecuredService){
							$result = $service->ACL();
							if (!($result instanceof IDBACLManager))
								$result = null;
						}
					}
				}	
				return $result;
			}break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "AuthType":{
				if (($value == IDBDriver::AUTH_DEFAULT) || ($value == IDBDriver::AUTH_EXPLICIT_ACL) || ($value == IDBDriver::AUTH_IMPLICIT_ACL))
					$this->_auth_type_ = $value;
				else throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
}

?>