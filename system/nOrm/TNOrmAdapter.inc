<?php
TApplication::Import('common::orm');

class TClassOrmConfig {
	private $_classname_;
	private $_tables_ = array();
	private $_nested_sets_ = array();
	private $_keys_ = array();
	private $_properties_ = array();
	
	public function __construct($classname){
		$this->_classname_ = $classname;
	}
	
	private function _parse_refprop($propname,$type,$propval,$sorting){
		preg_match_all('/(\w*)\s*\(([^\)]*)\)/', $propval, $props);
			$n2 = count($props[1]);
			for ($j = 0; $j < $n2; $j++){
				$flds = explode(',',$props[2][j]);
				foreach ($flds as $fld){
					$fldnm = explode('.',$fld);
					$this->_properties_[$propname]['fields'][] = array('name'=>trim($fldnm[0]),'table'=>count($fldnm) > 1?trim($fldnm[1]):$default_tbl,'property'=>$props[1][j]);
				}
			}
			if ($type == 'collection'){
				preg_match_all('/(\w*)\s*\(\s*(asc|desc)\s*\)/', $sorting, $sorts);
				$n2 = count($sorts[1]);
				$this->_properties_[$propname]['sorting'] = array();
				for ($j = 0; $j < $n2; $j++)
					$this->_properties_[$propname]['sorting'][] = array('property'=>$sorts[1][$j],'type'=>$sorts[2][$j] == 'asc'?TSortingType::SORT_ASC:TSortingType::SORT_DESC);
			}
		
	}
	
	private function _override_scalar_property($name,array $settings){
			if (!isset($this->_properties_[$name]))
				$this->_properties_[$name] = array();
			$this->_properties_[$name]['type'] = 'scalar';
			$this->_properties_[$name]['datatype'] = $settings['datatype']; 
			$this->_properties_[$name]['field'] = $settings['field'];
	}
	
	private function _override_ref_property($name,array $settings){
			if (!isset($this->_properties_[$name]))
				$this->_properties_[$name] = array('fields'=>array());
			$this->_properties_[$name]['type'] = $settings['type'];
			$this->_properties_[$name]['classname'] = $settings['classname'];
			$this->_parse_refprop($name,$settings['type'],$settings['fields'],@$settings['sorting']);
				
				
	}
	
	public function ParseClass(ReflectionClass $rc){
		$lineend = '(?:\s+[^\r\n]*|\s*)$/m';
		$doc = $rc->getDocComment();
		preg_match_all('/@orm.table\s+(\w+)\s*\(((?:\s*\w+\s*,?)+)\)/'.$lineend,$doc,$matches);
		$n = count($matches[1]);
		for ($i = 0; $i < $n; $i++){
			$this->_tables_[$matches[1][$i]] = array();
			$keyfields = explode(',', $matches[2][$i]);
			foreach ($keyfields as $fld)
				$this->_tables_[$matches[1][$i]][] = trim($fld);
		}
		
		$tbls = array_keys($this->_tables_);
		$default_tbl = $tbls[0];
		
		preg_match_all('/@orm.key\s+(\w+)/'.$lineend,$doc,$matches);
		if (!empty($matches[1])){
			$this->_keys_ = array();
			foreach ($matches[1] as $key)
				$this->_keys_[] = $key;
		}
		
		preg_match_all('/@orm.nestedset\s+(\w+)\s+(\w+)/'.$lineend,$doc,$matches);
		$this->_nested_sets_ = array();
		if (!empty($matches[1])){
			$this->_nested_sets_ = array('parent'=>$matches[1],'children'=>$matches[2]);
		}
		
		preg_match_all('/@property\s+(\w+)\s+\$(\w*)\s*\{@orm.scalar\s+(\w[\w\.]*)\s*\}/'.$lineend,$doc,$matches);
		$n = count($matches[2]);
		for ($i = 0; $i < $n; $i++){
			$fld = explode('.',$matches[3][$i]);
			$this->_override_scalar_property($matches[2][$i], array('datatype'=>$matches[1][$i],'field'=>array('name'=>$fld[0],'table'=>count($fld) > 1?$fld[1]:$default_tbl)));
		}
		
		preg_match_all('/@property\s+(\w+)\s+\$(\w*)\s*\{@orm.(reference|collection)\s+((?:(?:\w+)\s*\((?:(?:\s*\w[\w\.]*\s*,?)+)\)\s*,?\s*)+)(?:\s+-s\s+([^\}]*)|\s*)\}/'.$lineend,$doc,$matches);
		$n = count($matches[2]);
		for ($i = 0; $i < $n; $i++){
			$this->_override_ref_property($matches[2][$i], array('type'=>$matches[3][$i],'classname'=>$matches[1][$i],'fields'=>$matches[4][$i],'sorting'=>$matches[5][$i]));
		}
		
		$props = $rc->getProperties(ReflectionProperty::IS_PUBLIC && ~ReflectionProperty::IS_STATIC);
		foreach ($props as $p){
			$doc = $p->getDocComment();
			$datatype = null;
			if (preg_match('/@var\s+(\w+)/',$doc,$matches2))
				$datatype = $matches2[1];
			
			if (preg_match('/@orm.scalar\s+(\w[\w\.]*)\s*$/m', $doc,$matches)){
				$fld = explode('.',$matches[1]);
				$this->_override_scalar_property($p->getName(), array('datatype'=>is_null($datatype)?'string':$datatype,'field'=>array('name'=>$fld[0],'table'=>count($fld) > 1?$fld[1]:$default_tbl)));
			} elseif ($datatype && preg_match('/@orm.(reference|collection)\s+((?:(?:\w+)\s*\((?:(?:\s*\w[\w\.]*\s*,?)+)\)\s*,?\s*)+)(?:\s+-s\s+((?:\w+\((?:asc|desc)\))+))?\s*$/m', $doc,$matches)){
				$this->_override_ref_property($p->getName(), array('type'=>$matches[1],'classname'=>$datatype,'fields'=>$matches[2],'sorting'=>$matches[3]));
			}
		}
	}
	
	public function ParseXml($filename){
		$xml = new DOMDocument();
		$xml->preserveWhiteSpace = false;
		$xml->Load($filename);
		if (!$xml->schemaValidate(self::$SystemPath."/schemas/norm.xsd"))
			throw new TNOrmException(TNOrmException::ERR_INVALID_ORM_CONFIG);
		$xpath = new DOMXPath($xml);
		$keys = $xpath->query('/class/key');
		if ($keys->length > 0){
			$this->_keys_ = array();
			foreach ($keys as $node)
				$this->_keys_[] = $node->nodeValue;	
		}
		$tables = $xpath->query('/class/table');
		foreach ($tables as $node){
			$this->_tables_[$node->getAttribute("name")] = array();
			$keyfields = $xpath->query('keyfield',$node);
			foreach ($keyfields as $fld)
				$this->_tables_[$node->getAttribute("name")][] = $fld;
		}
		
		$tbls = array_keys($this->_tables_);
		$default_tbl = $tbls[0];
		
		$pf = $xpath->query('/class/nestedset/parentfield');
		$cf = $xpath->query('/class/nestedset/childrenfield');
		$this->_nested_sets_ = array();
		if (($pf->length > 0) && ($cf->length > 0)){
			$this->_nested_sets_ = array('parent'=>$pf->item(0)->nodeValue,'children'=>$cf->item(0)->nodeValue);
		}
		
		$scalars = $xpath->query('/class/scalar');
		foreach ($scalars as $node){
			$this->_override_scalar_property($node->getAttribute("name"), array('datatype'=>$node->getAttribute("type"),'field'=>array("name"=>$node->getAttribute("field"),"table"=>$node->getAttribute("table")?$node->getAttribute("table"):$default_tbl)));
		}
		
		$links = $xpath->query('/class/reference|/class/collection');
		foreach ($links as $node){
			if (!isset($this->_properties_[$node->getAttribute("name")]))
				$this->_properties_[$node->getAttribute("name")] = array('fields'=>array());
			$this->_properties_[$node->getAttribute("name")]["type"] = $node->tagName;
			$this->_properties_[$node->getAttribute("name")]["classname"] = $node->getAttribute("classname");
			$properties = $xpath->query('property',$node);
			foreach ($properties as $propnode){
				$fields  = $xpath->query('field',$propnode);
				foreach ($fields as $fldnode)
					$this->_properties_[$node->getAttribute("name")]['fields'][] = array('name'=>$fldnode->getAttribute('name'),'table'=>$fldnode->getAttribute('table')?$fldnode->getAttribute('table'):$default_tbl,'property'=>$propnode->getAttribute('name'));
			}
			if ($node->tagName == 'collection'){
				$this->_properties_[$node->getAttribute("name")]['sorting'] = array();
				$sortings  = $xpath->query('sort',$node);
				foreach ($sortings as $sortnode)
					$this->_properties_[$node->getAttribute("name")]['sorting'][] = array('property'=>$sortnode->getAttribute('property'),'type'=>($fldnode->getAttribute('type') == 'asc')?TSortingType::SORT_ASC:TSortingType::SORT_DESC);
			}
		} 
	}
	
	public function __toString(){
		$text ='<?php $ormconfig=array(';
		$text .= '"classname"=>"'.$this->_classname_.'",';
		$text .= '"tables"=>array(';
		$first = true;
		foreach ($this->_tables_ as $table=>$keys){
			if ($first) $first = false; else $text .= ','; 
			$text .= '"'.$table.'"=>array("'.join('","',$keys).'")';
		}
		$text .= '),"keys"=>array("'.join('","',$this->_keys_).'"),"nestedsets"=>array('.(!empty($this->_nested_sets_))?'"parent"=>"'.$this->_nested_sets_['parent'].'","children"=>"'.$this->_nested_sets_['children'].'"':''.'),"properties"=>array(';
		
		$first = true;
		foreach ($this->_properties_ as $prop=>$data){
			if ($first) $first = false; else $text .= ',';
			$text .= '"'.$prop.'"=>array("type"=>"'.$data['type'].'",';
			switch ($data['type']){
				case 'scalar':$text .= '"datatype"=>"'.$data['datatype'].'","field"=>array("name"=>"'.$data['field']['name'].'","table"=>"'.$data['field']['table'].'")';break;
				case 'reference':
				case 'collection':{
					$text .= '"fields"=>array(';
					$ffirst = true;
					foreach ($data['fields'] as $field){
						if ($ffirst) $ffirst = false; else $text .= ',';
						$text .= 'array("name"=>"'.$field['name'].'","table"=>"'.$field['table'].'","property"=>"'.$field['property'].'")';
					}
					$text .= ')'; 
					if ($data['type'] == 'collection'){
						$text .= ',"sorting"=>array(';
						$sfirst = true;
						foreach ($data['sorting'] as $sort){
							if ($sfirst) $sfirst = false; else $text .= ',';
							$text .= 'array("property"=>"'.$sort['property'].'","type"=>"'.$sort['type'].'")';
						}
						$text .= ')';
					}
				}break;
			}
			$text .= ')';
		}
		$text .= ')';
		$text .= '); ?>';
		return $text;
	}
}

class TNOrmIterator extends TIteratorAdapter {
	private $_classname_;
/**
 * @var TNOrmAdapter
 */	
	private $_adapter_;
	
	public function __construct(IIterator $base, $classname, $adapter){
		parent::__construct($base);
		$this->_classname_ = $classname;
		$this->_adapter_ = $adapter;
	}
	
	public function Item() {
		return $this->_adapter_->Create(parent::Item(), $this->_classname_);
	}
}

class TNOrmException extends TStorageException {
	const ERR_OBJECT_NOT_FOUND = 30001;
	const ERR_COMPOSITE_KEY_FILTER = 30002;
	const ERR_INVALID_ORM_CONFIG = 30003;
	const ERR_COMPOSITE_KEY_INCOMPLETE = 30004;
	const ERR_MISSING_KEY = 30005;
/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_OBJECT_NOT_FOUND:return "Object not found!";break;
			case self::ERR_COMPOSITE_KEY_FILTER:return "Composite references cannot be filtered!";break;
			case self::ERR_INVALID_ORM_CONFIG:return "Invalid orm configuration!";break;
			case self::ERR_COMPOSITE_KEY_INCOMPLETE:return "Can not save entity with incomplete composite key!";break;
			case self::ERR_MISSING_KEY:return "Object identifier is missing!";break;
			default:return "";break;		
		}
	}		
}

class TNOrmBranchLoader {
	protected $object;
	protected $adapter;
	
	protected $value = false;
	
	public function __construct($object,TNOrmAdapter $adapter){
		$this->object = $object;
		$this->adapter = $adapter;
	}
	
	public function Value(){
		if ($this->value === false)
			$this->value = $this->adapter->FetchContainers($this->object);
		return $this->value;
	}
}

/**
 * @author Крас
 * @property mixed $Key
 * @property mixed $Value
 */

abstract class TNOrmLoader {
/**
 * @var TNOrmAdapter
 */	
	protected $adapter;
	protected $key;
	protected $value;
	protected $dummy;
	
	
	public function __construct(&$row, &$meta, TNOrmAdapter $adapter){
		$this->adapter = $adapter; 
		$this->dummy = new $meta['classname']();
		$keys = array();
		$props = array();
		foreach ($meta['fields'] as $field){
			$nm = $field['name'];
			$keys[] = $row->$nm;
			if (!isset($props[$field['property']]))
				$props[$field['property']] = array();
			$props[$field['property']][] = $row->$nm;	
		}	
		foreach ($props as $prop=>$value)
			$this->dummy->$prop = (count($value) == 1)?$value[0]:$value;	
		$this->key = (count($keys) == 1)?$keys[0]:$keys;
	}
	
/**
 * @return mixed
 */	
	public function Key(){
		return $this->key;
	}
	
/**
 * @return mixed
 */	
	public function Value(){
		if (!isset($this->value))
			$this->load();
		return $this->value;
	}
	
	protected abstract function load();
		
	public function __get($nm){
		switch ($nm){
			case 'Key':return $this->Keys();break;
			case 'Value':return $this->Value();break;
		}
	}
}

class TNOrmObjectLoader extends TNOrmLoader {
	protected function load(){
		$this->value = $this->adapter->Load($this->dummy);
	}
}



class TNOrmCollectionLoader extends TNOrmLoader {
	protected $sorting;
	protected $container;
	
	public function __construct(&$row, &$meta, TNOrmAdapter $adapter){
		$this->container = $container;
		parent::__construct($row, $meta, $adapter);
		
		$this->sorting = array();
		foreach ($meta['sorting'] as $prop=>$type)
			$this->sorting[] = new TOrmSorting($prop,new TSortingType($type));
	}
	
	protected function load(){
		$this->value = $this->adapter->Fetch($this->dummy,array(),$this->sorting);
	}
}

interface INOrmObject {
/**
 * @param string $property
 * @return boolean
 */	
	public function WasChanged($property);
	
/**
 * @return bool
 */	
	public function WasMoved();
}

class TNOrmObject implements INOrmObject {
	private $_origins_ = array();
	
	private $_loaders_ = array();
	
	private $_was_moved_ = false;
	
	private function _check_prefixed_property(&$nm,$prefix){
		$rc = new ReflectionClass(get_class($this));
		$prop = '_'.$prefix.preg_replace_callback('/([A-Z][a-z\d]*)/',create_function('$matches','return "_".strtolower($matches[0]);'),$nm).'_';
		if ($p = $rc->getProperty($prop))
			if ($p->IsPrivate() && !$p->IsStatic())
				return $prop;			
		return false;
	}
		
	public function __set($nm,$value){
		if ($n = $this->_check_prefixed_property($nm,'orm')){
			if ($value instanceof TNOrmLoader){
				$this->_loaders_[$n] = $value;
				$this->$n = null;
			}
			if (!isset($this->_origins_[$n]))
				$this->_origins_[$n] = ($value instanceof TNOrmLoader)?$value->Key():$value;
			$this->$n = $value;
		} else if ($n = $this->_check_prefixed_property($nm,'ns')){
			if ($value instanceof TNOrmBranchLoader){
				$this->_loaders_[$n] = $value;
				$this->$n = null;
			} else if (($value === false) || (is_object($value) && (get_class($this) == get_class($value)))){
				if (!is_null($this->$n))
					$this->_was_moved_ = $this->$n != $value;
				$this->$n = $value;
			} else throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		}
	}
	
	public function __get($nm){
		if (($n = $this->_check_prefixed_property($nm,'orm')) || ($n = $this->_check_prefixed_property($nm,'ns'))){
			if (is_null($this->$n)){
				if (isset($this->_loaders_[$n]))
					$this->$n = $this->_loaders_[$n]->Value();
			}
			return $this->$n;
		}
	}
	
	public function WasMoved(){
		return $this->_was_moved_;	
	}
	
	public function WasChanged($property){
		if ($this->_check_orm_property($property)){
			if (isset($this->_origins_[$property]))
				return $this->$property != $this->_origins_[$property];
		}
		return false;
	}
} 

/**
 * @author dkrasilnikov@gmail.com
 * @property IDBDriver $Driver
 */

class TNOrmAdapter extends TConfigurable implements IORMStorage {
	const CONF_SCRIPT = 0;
	const CONF_XML = 1;
	const CONF_PHPDOC = 2;
/**
 * @var IDBDriver
 */	
	protected $_ioc_driver_;
	
	private $_use_config_ = 3;
	
	private $_bool_emulate_integrity_ = false;
	
	private $_xml_config_path_;
	private $_script_config_path_;
	private $_date_format_;
	
	private $_bool_unique_objects_;
	
	private $_loaded_orms_ = array();
	
	private $_loaded_objects_ = array();

	private function _form_ds(&$orm,&$join,&$tables,&$fields){
		$prevkeys = null;
		foreach ($orm['tables'] as $table=>$keys){
			if (!isset($ds)){
				$ds = $table;
				$tables[$table] = new TTable($table);
				$prevkeys = $keys;
			} else {
				$conditions = array();
				$n = count($keys);
				$tables[$table] = new TTable($table);
				for ($i = 0; $i < $n; $i++)
					$conditions[] = new TCondition(TConditionType::C_EQUAL, array(new TTableField($keys[$i],null,$tables[$table]),new TTableField($prevkeys[$i])));
				$join[] = new TJoin($tables[$table], TJoin::JOIN_INNER, $conditions); 
			}	
		}
		
		foreach ($orm['properties'] as $name=>$meta){
			switch ($meta['type']){
				case 'reference':
				case 'collection':{
					foreach ($meta['fields'] as $field)
						$fields[] = new TTableField($field['name'],null,$tables[$field['table']]);break;
				}break;
				case 'scalar':
				default:$fields[] = new TTableField($meta['field']['name'],null,$tables[$meta['field']['table']]);break;
			}
		}
			
		return $ds;	
	}
	
	private function _ref_prop_value($value,$fc,array &$refvalues){
		if (is_array($value)){
			if (count($value) <> $fc)
				throw new TNOrmException(TNOrmException::ERR_INVALID_ORM_CONFIG);
			foreach ($value as $v)
				$refvalues[] = $v;	
		} else if ($value instanceof TNOrmLoader){
			$this->_ref_prop_value($value->Key, $fc, $refvalues);
		} else if (is_object($value)){
			$this->_ref_prop_value($this->_get_object_key_values($value), $fc, $refvalues);
		} else
			$refvalues[] = (string)$value;	
	}
	
	private function _reference_value($value,$meta){
		if ($value instanceof TNOrmObjectLoader){
			$value = $value->Key;
		} elseif (is_object($value)) {
			$propvalues = array();
			foreach ($meta['fields'] as $field){
				if (!isset($propvalues[$field['property']]))
					$propvalues[$field['property']] = array();
				$propvalues[$field['property']][] = $field['name'];
			}
			$temp = array();
			foreach ($propvalues as $propname => $fields){
				$this->_ref_prop_value($value->$propname, count($fields),$temp);
			}	
			$value = $temp;
		}
		return is_array($value)?$value:array($value);
	}
	
	private function _get_scalar_prop($object,$prop,$datatype){
		switch ($datatype){
			case 'date':
			case 'TDate':if ($object->$prop instanceof TDate){
				if ($this->_date_format_)
					return $object->$prop->ToString($this->_date_format_);
				return $object->$prop;
			}break;
		}
		return $object->$prop;
	}
	
	private function _get_object_key_values($object){
		$orm = $this->_get_orm_config($value);
		$result = array();
		foreach ($orm['keys'] as $propname){
			switch ($orm['properties'][$propname]['type']){
				case 'collection':break;
				case 'reference':{
					$v = $this->_reference_value($object->$propname, $orm['properties'][$propname]);
					foreach ($v as $val)
						if (!is_null($val))
							$result[] = $val;
				}break;
				case 'scalar':
				default:if (!is_null($object->$propname)) $result[] = $this->_get_scalar_prop($object,$propname,$orm['properties'][$propname]['datatype']);break;
			}
		}
		return $result;
	}
	
	private function _filter_from_object($orm,$object,&$tables,$ns = false,$ns_parent = false){
		$filter = array();
		foreach ($orm['properties'] as $prop=>$meta)
			if (isset($object->$prop)){
				switch ($meta['type']){
					case 'reference':{
						$v = $this->_reference_value($object->$prop,$meta);
						$n = count($meta['fields']);
						for ($i = 0; $i < $n; $i++){
							$operands = array(new TTableField($meta['fields'][$i]['name'],null,$tables[$meta['fields'][$i]['table']]),$v[$i]);
							$filter[] = new TCondition(TConditionType::C_EQUAL, $operands);
						}
					}break;
					case 'collection':break;
					case 'scalar':
					default:{
						$operands = array(new TTableField($meta['field']['name'],null,$tables[$meta['field']['table']]),$this->_get_scalar_prop($object,$prop,$meta['datatype']));
						$filter[] = new TCondition(TConditionType::C_EQUAL, $operands);
					}break;
				}
			}
		return $filter;
	}
	
/**
 * Fills an object with missing data
 * @param object $object
 * @return object
 */	
	public function Load($object){
		$orm = $this->_get_orm_config($object);
		$join = array(); 
		$tables = array();
		$fields = array();
		
		$ds = $this->_form_ds($orm, $join, $tables, $fields);
		$filter = $this->_filter_from_object($orm, $object,$tables);
		
		$ds = new TTable($ds, $fields, $filter, $join, array(),true);
		$result = $this->_ioc_driver_->FetchRecords($ds);
		foreach ($result as $item){
			return $this->Create($item,$object);
		}
		//throw new TNOrmException(TNOrmException::ERR_OBJECT_NOT_FOUND);
		return null;
	}
	
	
	private function _check_save_tables($object,$table,$orm,&$tables,&$keyvalues){
		if (!isset($tables[$table])){
			$tables[$table] = array(
				'keys' => array(),
				'values' => array()
			);
			$n = count($orm['tables'][$table]);
			for ($i = 0; $i < $n; $i++)
				$tables[$table]['keys'][$orm['tables'][$table][$i]] = $keyvalues[$i];
		}
	}
	
	
	private function _entity_keys($object,$orm,&$emptyid){
		$keyvalues = $this->_get_object_key_values($object);
		$emptyid = false;
		$n = count($orm['keys']);
		if (count($keyvalues) != $n){
			if ($n == 1)
				$emptyid = true;
			else throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_INCOMPLETE);	
		}
		return $keyvalues;
	}
	
/** 
 * Saves object to storage, returns saved object
 * @param object $object
 * @return object
 */	
	public function Save($object){
		$orm = $this->_get_orm_config($object);
		$keyvalues = $this->_entity_keys($object, $orm, $emptyid);
		$tables = array();
		
		foreach ($orm['properties'] as $prop => $meta)
			if (($meta['type'] != 'collection') && (!($object instanceof INOrmObject) || (($object instanceof INOrmObject) && ($object->WasChanged($prop)))))
			{
				switch ($meta['type']){
					case 'reference':{
						$v = $this->_reference_value($object->$prop,$meta);
						$n = count($meta['fields']);
						for ($i = 0; $i < $n; $i++){
							$this->_check_save_tables($object, $meta['fields'][$i]['table'], $orm, $tables,$keyvalues);
							$tables[$meta['fields'][$i]['table']]['values'][$meta['fields'][$i]['name']] = $v[$i];
						}
					}break;
					case 'scalar':
					default:{
						$this->_check_save_tables($object, $meta['field']['table'], $orm, $tables,$keyvalues);
						$tables[$meta['field']['table']]['values'][$meta['field']['name']] = $this->_get_scalar_prop($object,$prop,$meta['datatype']);
					}break; 
				}	
			}
		
		$this->_ioc_driver_->BeginTransaction();
		try {
			$maintable = true;
			foreach ($orm['tables'] as $t=>$keyfields){
				$table = $tables[$t];
				$filter = array();
				foreach ($data['keys'] as $fld=>$value)
					$filter[] = new TCondition(TConditionType::C_EQUAL, array(new TTableField($fld),$value));
				$fields = array();
				$values = array();
				foreach ($data['values'] as $fld=>$value){
					$fields[] = new TTableField($fld);
					$values[] = $value;
				}
				
				$ds = new TTable($table, array(), $filter, array(), array());
				
				if ($emptyid || ($this->_ioc_driver_->UpdateRecords($ds, $fields, $values) == 0)){
					$merged = $data['keys'] + $data['values'];

					if ($maintable && !empty($orm['nestedsets'])){
						$parentprop = $orm['nestedsets']['parent'];
						$filter = array();
						if (is_object($object->$parentprop))
							$filter = $this->_filter_from_object($orm, $object->$parentprop, $tables);
						$ds = new TNestedSets($table, array(), $filter, array(), array());
						$this->_ioc_driver_->NestedSetAddRecord($ds, $merged);
					} else
						$this->_ioc_driver_->InsertRecords($ds, array_values($merged), array_keys($merged),$pk);
					
					if ($emptyid)
						foreach ($orm['keys'] as $key) 
							$object->$key = $pk;
				} else if ($object instanceof INOrmObject){
					if (!empty($orm['nestedsets']) && $object->WasMoved() && $maintable){
						$src = new TNestedSets($table, array(), $filter, array(), array());
						$parentprop = $orm['nestedsets']['parent'];
						$filter = array();
						if (is_object($object->$parentprop))
							$filter = $this->_filter_from_object($orm, $object->$parentprop, $tables);
						$dest = new TNestedSets($table, array(), $filter, array(), array());
						$this->_ioc_driver_->NestedSetBranchesMove($src,$dest);
					}
				}
				$maintable = false;
			}
			$this->_ioc_driver_->CommitTransaction();
		} catch (Excetion $e){
			$this->_ioc_driver_->RollbackTransaction();
			throw $e;
			return false;
		}
		return $this->Load($object);
	}
	
	public function Delete($object){
		$orm = $this->_get_orm_config($object);
		$keyvalues = $this->_entity_keys($object, $orm, $emptyid);
		if ($emptyid)
			throw new TNOrmException(TNOrmException::ERR_MISSING_KEY);
		
/* 
 * @todo Probably implement removing object from loaded objects
 */			
		$this->_ioc_driver_->BeginTransaction();
		try {
			$maintable = true;
			$result = true;
			foreach ($orm['tables'] as $table=>$keys){
				$n = count($keys);
				$filter = array();
				for ($i = 0; $i < $n; $i++)
					$filter[] = new TCondition(TConditionType::C_EQUAL, array(new TTableField($keys[$i]),$keyvalues[$i]));

				if ($maintable && !empty($orm['nestedsets'])){
					$ds = new TNestedSets($table, array(), $filter, array(), array());
					$result = $result && $this->_ioc_driver_->NestedSetBranchesDelete($ds);
				} else {	
					$ds = new TTable($table, array(), $filter, array(), array(),true);
					$result = $result && $this->_ioc_driver_->DeleteRecords($ds);
				}
				$maintable = false;
				if (!$this->_bool_emulate_integrity_) break;
			}			
		} catch (Exception $e){
			$this->_ioc_driver_->RollbackTransaction();
			throw $e;
			return false;
		}
		$this->_ioc_driver_->CommitTransaction();
		return $result;
	}

	public function FetchContainers($object){
		$orm = $this->_get_orm_config($object);
		$join = array(); 
		$tables = array();
		$fields = array();
		
		$ds = $this->_form_ds($orm, $join, $tables, $fields);
		$filter = $this->_filter_from_object($orm, $object, $tables);
		$ds = new TNestedSets($ds,TNestedSets::DIR_UP,null,false,null,$fields,$filter,$join,array(),true);
		$ns = $this->_ioc_driver_->NestedSetFetch($ds);
		$parentprop = $orm['nestedsets']['parent'];
		$item = false;
		foreach ($ns as $nse){
			$item = $this->Create($nse,$orm['classname']);
			if (isset($p))
				$item->$parentprop = $p;
			else 
				$item->$parentprop = false;
			$p = $item;	
		}
		return $item;
	}
	
/**
 * 
 * Fetches a collection of objects from storage
 * @param object|ReflectionClass|string $dummy
 * @param TOrmCondition[] $conditions
 * @param TOrmSorting[] $sorting
 * @param int $offset
 * @param int $count
 */	
	public function Fetch($dummy,array $greedy = array(),array $conditions = array(),array $sorting = array(), $offset = null, $count = null){
		$orm = $this->_get_orm_config($dummy);
		$join = array(); 
		$tables = array();
		$fields = array();
		
/*
 * @todo implement filter by parent property 
 */		
		
		$ds = $this->_form_ds($orm, $join, $tables, $fields);
		if (is_object($dummy))
			$filter = $this->_filter_from_object($orm, $dummy, $tables);
		
		foreach ($conditions as $condition){
			$operands = array();
			foreach ($condition->Operands as $o)
				if (is_string($o) && isset($orm['properties'][$o])){
					switch ($orm['properties'][$o]['type']){
						case 'collection':break;
						case 'reference':{
							if (count($orm['properties'][$o]['fields']) > 1)
								throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_FILTER);
							foreach ($orm['properties'][$o]['fields'] as $field)
								$operands[] = new TTableField($field['name'],null,$tables[$field['table']]);
						}break;
						case 'scalar':
						default: $operands[] = new TTableField($orm['properties'][$o]['field']['name'],null,$tables[$orm['properties'][$o]['field']['table']]);
					}
				} else 	
					$operands[] = $o;
					
			$filter[] = new TCondition($condition->Type, $operands);
		}
		
		$sort = array();
		foreach ($sorting as $s){
			if (isset($orm['properties'][$s->Attribute]))
				switch ($orm['properties'][$s->Attribute]['type']){
					case 'collection':break;
					case 'reference':{
						foreach ($orm['properties'][$s->Attribute]['fields'] as $field)
							$sort[] = new TSorting(new TTableField($field['name'],null,$tables[$field['table']]), $s->Type);break;
					}break;
					default:$sort[] = new TSorting(new TTableField($orm['properties'][$s->Attribute]['field']['name'],null,$tables[$orm['properties'][$s->Attribute]['field']['table']]), $s->Type);break;	
				}
		}
		
		if (!empty($orm['nestedsets'])){
			$ds = new TNestedSets($ds,TNestedSets::DIR_DOWN,null,true,null,$fields,$filter,$join,array(),true, $offset, $count);
			$ns = $this->_ioc_driver_->NestedSetFetch($ds);
			$result = array();
			$childprop = $orm['nestedsets']['children'];
			foreach ($ns as $nse){
				$item = $this->Create($nse,$orm['classname']);
				$item->$children = array();
				if (is_null($nse->NestingParent())){
					$result[] = $item;
				} else {
					$p = $this->Create($nse->NestingParent(), $orm['classname']);					
					$p->$children[] = $item; 
				}
			}
			return $result;
		}
		
		$ds = new TTable($ds, $fields, $filter, $join, $sort,true,$offset,$count);
		return new TNOrmIterator($this->_ioc_driver_->FetchRecords($ds),$orm['classname'],$this);
	} 
	
	private function _check_loaded($dummy,&$orm){
		if ($this->_bool_unique_objects_){
			if (!isset($this->_loaded_objects_[$orm['classname']]))
				$this->_loaded_objects_[$orm['classname']] = array();
			$id = $this->_get_object_key_values($dummy);
			if (empty($id))
				return $dummy;	
			$id = join('|',$id);	
			if (isset($this->_loaded_objects_[$orm['classname']][$id]))
				$dummy = $this->_loaded_objects_[$orm['classname']][$id];
			else	
				$this->_loaded_objects_[$orm['classname']][$id] = $dummy;
		}
		return $dummy;
	}
	
	private function _assign_property($dummy,$property,$meta,$row){
		switch ($meta['type']){
			case 'scalar':{
				$nm = $meta['field']['name'];
				$v = $row->$nm;
				switch ($prop['datatype']){
					case 'bool':$v = TConvertions::ConvertToBoolean($v);break;
					case 'date':
					case 'TDate':{
						if ($this->_date_format_){
							$v = date_create_from_format($this->_date_format_,$v);
							$v = $v->getTimestamp();
						}
						$v = new TDate($v);
					}break;
					case 'int':$v = (int)$v;break;
					case 'float':$v = (float)$v;break;	
				}
				$dummy->$property = $v;
			}break;
			case 'reference':{
				$assign = true;
				if (is_object($dummy->$property) && !($dummy->$property instanceof TNOrmLoader)){
					$new_val = array();
					foreach ($meta['fields'] as $field){
						$nm = $field['name'];
						$new_val[] = $row->$nm;
					}
					$assign = $new_val != $this->_get_object_key_values($dummy->$property);	
				}
				if ($assign)
					$dummy->$property = new TNOrmObjectLoader($row,$meta,$this);
			}break;
			case 'collection': if (!isset($dummy->$property)) $dummy->$property = new TNOrmCollectionLoader($row,$meta,$this);break;
		}
	}

	public function OrmProperties($dummy){
		$orm = $this->_get_orm_config($dummy);
		$result = array();
		foreach ($orm['properties'] as $key=>$prop)
			switch ($prop['type']){
				case 'scalar':$result[$key] = $prop['datatype'];break;
				case 'reference':$result[$key]=$prop['classname'];break;
				case 'collection':$result[$key]='collection';break;
			}
		return $result;
	}
	
	public function OrmNestedSetsOptions($dummy){
		$orm = $this->_get_orm_config($dummy);
		return $orm['nestedsets'];
	}
	
/**
 * Instantiates or loads an object
 * @param mixed $row
 * @param object|string $dummy
 */	
	public function Create($row,$dummy){
		if (is_array($row))
			$row = (object)$row;
		$orm = $this->_get_orm_config($dummy);
		if (!is_object($dummy))
			$dummy = new $orm['classname']();
			
		foreach ($orm['keys'] as $key)
			$this->_assign_property($dummy, $key, $orm['properties']['key'], $row);
			
		$dummy = $this->_check_loaded($dummy, $orm);	

		if (!empty($orm['nestedsets'])){
			$parentprop = $orm['nestedsets']['parent'];
			$dummy->$parentprop = new TNOrmBranchLoader($dummy,$this);
		}		
		
		foreach ($orm['properties'] as $name=>$prop)
			if (!in_array($name,$orm['keys']))
				$this->_assign_property($dummy, $name, $prop, $row);

		return $dummy;
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'UseConfig':{
				if (is_string($value) && !is_numeric($value)){
					$value = strtolower(trim($value));
					switch ($value){
						case 'script':$value = self::CONF_SCRIPT;break;
						case 'xml':$value = self::CONF_XML;break;
						case 'phpdoc':$value = self::CONF_PHPDOC;break;
						default:{
							if (defined('self::'.$value))
								$value = constant('self::'.$value);							
						}break;
					} 	
				}
				if ((int)$value == self::CONF_SCRIPT)
					$this->_use_config_ = self::CONF_SCRIPT;
				else 
					$this->_use_config_ = $this->_use_config_ | (int)$value;
			}break;
			case 'XmlConfigPath':$this->_xml_config_path_ = TApplication::$Application->AdjustPath($value);break;
			case 'ScriptConfigPath':$this->_script_config_path_ = TApplication::$Application->AdjustPath($value);break;
			case 'DateFormat':$this->_date_format_ = $value;break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case 'UseConfig':return $this->_use_config_;break;
			default:return parent::__get($nm);break;
		}
	}
	
	private function _get_orm_config($object){
		$rc = null;
		if (is_string($object))
			$cn = $object;
		if ($object instanceof ReflectionClass){
			$cn = $object->getName();
			$rc = $object;
		} else if (is_object($object)) 
			$cn = get_class($object);
		else throw new TCoreException(TCoreException::ERR_BAD_VALUE);

		if (!isset($this->_loaded_orms_[$cn])){
			if (!$this->_script_config_path_)
				$this->_script_config_path_ = TApplication::$Application->AdjustPath('orm');
			
			if ($this->_use_config_ & (self::CONF_XML | self::CONF_PHPDOC)){
				$filename = $this->_script_config_path_.'/'.$cn.'.orm.php';
				if (!$rc)
					$rc = new ReflectionClass($cn);

				$src_time = 0;	
				
				if (!$this->_xml_config_path_)
					$this->_xml_config_path_ = TApplication::$Application->AdjustPath('private/orm');
				
					
				$classes = array();	
				while ($rc instanceof ReflectionClass){
					if ($this->_use_config_ & self::CONF_PHPDOC)
						if (filemtime($rc->getFileName()) > $src_time)
							$src_time = filemtime($rc->getFileName());
					if ($this->_use_config_ & self::CONF_XML)
						if (filemtime($this->_xml_config_path_.'/'.$rc->getName().'.orm.xml') > $src_time)
							$src_time = filemtime($this->_xml_config_path_.'/'.$rc->getName().'.orm.xml');
					array_unshift($classes, $rc);
					$rc = $rc->getParentClass();
				}
						
						
				if (!file_exists($filename) || (filemtime($filename) < $src_time)){
					$orm = new TClassOrmConfig($cn);

					if ($this->_use_config_ & self::CONF_PHPDOC)
						foreach ($classes as $rc)
							$orm->ParseClass($rc);
						
					if ($this->_use_config_ & self::CONF_XML){
						foreach ($classes as $rc)	
							$orm->ParseXml($this->_xml_config_path_.'/'.$rc->getName().'.orm.xml');
					}
					file_put_contents($filename, $orm);
				}	
				include $filename;
				$this->_loaded_orms_[$cn] = $ormconfig;
			}
		} 
		return $this->_loaded_orms_[$cn];	
	}
}