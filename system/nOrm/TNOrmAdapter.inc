<?php
TApplication::Import('common::orm');
TApplication::Import('common::acid');

class TClassOrmConfig {
	private $_classname_;
	private $_tables_ = array();
	private $_nested_sets_ = array();
	private $_keys_ = array();
	private $_properties_ = array();
	private $_identity_;
	private $_table_classes_ = array();
	
	
	public function __construct($classname){
		$this->_classname_ = $classname;
	}
	
	private function _parse_refprop($propname,$type,$propval,$sorting,$default_tbl){
		preg_match_all('/([_\w]+)\s*\(([^\)]*)\)/', $propval, $props);
			$n2 = count($props[2]);
			for ($j = 0; $j < $n2; $j++){
				$flds = explode(',',$props[2][$j]);
				foreach ($flds as $fld){
					$fldnm = explode('.',$fld);
					$this->_properties_[$propname]['fields'][] = array('name'=>trim(count($fldnm) > 1?$fldnm[1]:$fldnm[0]),'table'=>count($fldnm) > 1?trim($fldnm[0]):$default_tbl,'property'=>$props[1][$j]);
				}
			}
			if ($type == 'collection'){
				preg_match_all('/([_\w]+)\s*\(\s*(asc|desc)\s*\)/', $sorting, $sorts);
				$n2 = count($sorts[1]);
				$this->_properties_[$propname]['sorting'] = array();
				for ($j = 0; $j < $n2; $j++)
					$this->_properties_[$propname]['sorting'][] = array('property'=>$sorts[1][$j],'type'=>$sorts[2][$j] == 'asc'?TSortingType::SORT_ASC:TSortingType::SORT_DESC);
			}
		
	}
	
	private function _override_scalar_property($name,array $settings, $class){
			if (!isset($this->_properties_[$name]))
				$this->_properties_[$name] = array();
			$this->_properties_[$name]['type'] = 'scalar';
			$this->_properties_[$name]['datatype'] = $settings['datatype']; 
			$this->_properties_[$name]['field'] = $settings['field'];
			$this->_properties_[$name]['defclass'] = $class;
	}
	
	private function _override_ref_property($name,array $settings,$default_tbl,$class){
			if (!isset($this->_properties_[$name]))
				$this->_properties_[$name] = array('fields'=>array());
			$this->_properties_[$name]['type'] = $settings['type'];
			$this->_properties_[$name]['classname'] = $settings['classname'];
			$this->_properties_[$name]['defclass'] = $class;
			$this->_properties_[$name]['mtm'] = explode('.',trim(@$settings['mtm']));
			$this->_parse_refprop($name,$settings['type'],$settings['fields'],@$settings['sorting'],$default_tbl);
	}
	
	public function ParseClass(ReflectionClass $rc){
		$doc = $rc->getDocComment();
		$lineend = '(?:\s+[^\r\n]*|\s*)$/m';
		preg_match_all('/@orm.table\s+([_\w]+)\s*\(((?:\s*[_\w]+\s*,?)+)\)'.$lineend,$doc,$matches);
		$n = count($matches[1]);
		for ($i = 0; $i < $n; $i++){
			$this->_tables_[$matches[1][$i]] = array();
			$this->_table_classes_[$matches[1][$i]] = $rc->getName();
			$keyfields = explode(',', $matches[2][$i]);
			foreach ($keyfields as $fld)
				$this->_tables_[$matches[1][$i]][] = trim($fld);
		}
		
		$tbls = array_keys($this->_tables_);
		if (!empty($this->_tables_)){
			$default_tbl = array_keys($this->_tables_);
			$default_tbl = $default_tbl[0];
		}
		
		preg_match_all('/@orm.key\s+([_\w]+)'.$lineend,$doc,$matches);
		if (!empty($matches[1])){
			$this->_keys_ = array();
			foreach ($matches[1] as $key)
				$this->_keys_[] = $key;
		}
		
		$this->_identity_ = (count($this->_keys_) > 1)?1:0;
		
		preg_match_all('/@orm.identity\s+(explicit|implicit)'.$lineend,$doc,$matches);
		if (!empty($matches[1])){
			$this->_identity_ = 0;
			foreach ($matches[1] as $idtype)
				$this->_identity_ = ($idtype == 'explicit')?1:0;
		}
		
		preg_match_all('/@orm.nestedset\s+([_\w]+)\s+([_\w]+)'.$lineend,$doc,$matches);
		$this->_nested_sets_ = array();
		if (!empty($matches[1])){
			$this->_nested_sets_ = array('parent'=>$matches[1][0],'children'=>$matches[2][0]);
		}
		
		$props = $rc->getProperties(ReflectionProperty::IS_PUBLIC & ~ReflectionProperty::IS_STATIC);
		foreach ($props as $p)
			if ($p->getDeclaringClass()->getName() == $rc->getName()){
			$pdoc = $p->getDocComment();
			$datatype = null;
			if (preg_match('/@var\s+([_\w]+)/',$pdoc,$matches2))
				$datatype = $matches2[1];
		
			if (preg_match('/@orm.scalar\s+(\w[_\w\.]*)\s*$/m', $pdoc,$matches)){
				$fld = explode('.',$matches[1]);
				$this->_override_scalar_property($p->getName(), array('datatype'=>is_null($datatype)?'string':$datatype,'field'=>array('name'=>(count($fld) > 1)?$fld[1]:$fld[0],'table'=>count($fld) > 1?$fld[0]:$default_tbl)),$rc->getName());
			} elseif ($datatype && preg_match('/@orm.(reference|collection)\s+((?:[_\w]+\.[_\w]+\s+)?)\s*((?:(?:[_\w]+)\s*\((?:(?:\s*\w[_\w\.]*\s*,?)+)\)\s*,?\s*)+)(?:\s+-s\s+((?:[_\w]+\((?:asc|desc)\))+))?\s*$/m', $doc,$matches)){
				$this->_override_ref_property($p->getName(), array('type'=>$matches[1],'classname'=>$datatype,'mtm'=>$matches[2][$i],'fields'=>$matches[3],'sorting'=>$matches[4]),$default_tbl,$rc->getName());
			}
		}
		
		preg_match_all('/@property\s+([_\w]+)\s+\$([_\w]+)\s*\{@orm.scalar\s+(\w[_\w\.]*)\s*\}'.$lineend,$doc,$matches);
		$n = count($matches[2]);
		for ($i = 0; $i < $n; $i++){
			$fld = explode('.',$matches[3][$i]);
			$this->_override_scalar_property($matches[2][$i], array('datatype'=>$matches[1][$i],'field'=>array('name'=>count($fld) > 1?$fld[1]:$fld[0],'table'=>count($fld) > 1?$fld[0]:$default_tbl)),$rc->getName());
		}
		
		preg_match_all('/@property\s+([_\w]+)\s+\$([_\w]+)\s*\{@orm.(reference|collection)\s+((?:[_\w]+\.[_\w]+\s+)?)\s*((?:(?:[_\w]+)\s*\((?:(?:\s*\w[_\w\.]*\s*,?)+)\)\s*,?\s*)+)((?:\s+-s\s+[^\}]*)?)\s*\}'.$lineend,$doc,$matches);
		$n = count($matches[2]);
		for ($i = 0; $i < $n; $i++){
			$this->_override_ref_property($matches[2][$i], array('type'=>$matches[3][$i],'classname'=>$matches[1][$i],'mtm'=>$matches[4][$i],'fields'=>$matches[5][$i],'sorting'=>$matches[6][$i]),$default_tbl,$rc->getName());
		}
	}

	public function ParseXml($filename){
		$cname = basename($filename,'.orm.xml');
		$xml = new DOMDocument();
		$xml->preserveWhiteSpace = false;
		$xml->Load($filename);
		if (!$xml->schemaValidate(TApplication::$SystemPath."/schemas/norm.xsd"))
			throw new TNOrmException(TNOrmException::ERR_INVALID_ORM_CONFIG);
		$xpath = new DOMXPath($xml);
		
		$keys = $xpath->query('/class/key');
		if ($keys->length > 0){
			$this->_keys_ = array();
			foreach ($keys as $node)
				$this->_keys_[] = $node->nodeValue;	
		}
		
		$this->_identity_ = (count($this->_keys_) > 1)?1:0;
		
		if ($xml->firstChild->getAttribute('identity') != 'auto')
			$this->_identity_ = ($xml->firstChild->getAttribute('identity') == 'explicit')?1:0;
		
		$tables = $xpath->query('/class/table');
		foreach ($tables as $node){
			$this->_tables_[$node->getAttribute("name")] = array();
			$this->_table_classes_[$matches[1][$i]] = $cname;
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
			$this->_override_scalar_property($node->getAttribute("name"), array('datatype'=>$node->getAttribute("type"),'field'=>array("name"=>$node->getAttribute("field"),"table"=>$node->getAttribute("table")?$node->getAttribute("table"):$default_tbl)),$cname);
		}
		
		$links = $xpath->query('/class/reference|/class/collection');
		foreach ($links as $node){
			if (!isset($this->_properties_[$node->getAttribute("name")]))
				$this->_properties_[$node->getAttribute("name")] = array('fields'=>array());
			$this->_properties_[$node->getAttribute("name")]["type"] = $node->tagName;
			$this->_properties_[$node->getAttribute("name")]["classname"] = $node->getAttribute("classname");
			$this->_properties_[$node->getAttribute("name")]["defclass"] = $cname;
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
	
	public function IsOrmClass(){
		return !empty($this->_tables_);
	}
	
	public function __toString(){
		$text ='<?php $ormconfig=array(';
		$text .= '"classname"=>"'.$this->_classname_.'",';
		$text .= '"identity"=>'.$this->_identity_.',';
		$text .= '"tables"=>array(';
		$temp = array();
		foreach ($this->_tables_ as $table=>$keys){
			$temp[] = '"'.$table.'"=>array("'.join('","',$keys).'")';
		}
		$text .= join(',',$temp).')';
		$text .= ',"tdefclasses"=>array(';
		$temp = array();
		foreach ($this->_table_classes_ as $table=>$class){
			$temp[] = '"'.$table.'"=>"'.$class.'"';
		}
		$text .= join(',',$temp).')';
		$text .= ',"keys"=>array("'.join('","',$this->_keys_).'")';
		$text .= ',"nestedsets"=>array('.((!empty($this->_nested_sets_))?('"parent"=>"'.$this->_nested_sets_['parent'].'","children"=>"'.$this->_nested_sets_['children'].'"'):'').')';
		$text .= ',"properties"=>array(';
		
		$temp = array();
		foreach ($this->_properties_ as $prop=>$data){
			$pt = '"'.$prop.'"=>array("type"=>"'.$data['type'].'","defclass"=>"'.$data['defclass'].'",';
			switch ($data['type']){
				case 'scalar':$pt .= '"datatype"=>"'.$data['datatype'].'","field"=>array("name"=>"'.$data['field']['name'].'","table"=>"'.$data['field']['table'].'")';break;
				case 'reference':
				case 'collection':{
					$pt .= '"classname"=>"'.$data['classname'].'",';
					if ($data['type'] == 'collection'){
						if (count($data['mtm']) > 1)
						$pt .= '"mtm"=>array("class"=>"'.$data['mtm'][0].'","property"=>"'.$data['mtm'][1].'"),';
						else
						$pt .= '"mtm"=>false,';
					}
					$pt .= '"fields"=>array(';
					$temp2 = array();
					foreach ($data['fields'] as $field){
						$temp2[] = 'array("name"=>"'.$field['name'].'","table"=>"'.$field['table'].'","property"=>"'.$field['property'].'")';
					}
					$pt .= join(',',$temp2).')'; 
					if ($data['type'] == 'collection'){
						$pt .= ',"sorting"=>array(';
						$temp2 = array();
						foreach ($data['sorting'] as $sort){
							$temp2[] = 'array("property"=>"'.$sort['property'].'","type"=>"'.$sort['type'].'")';
						}
						$pt .= ')';
					}
				}break;
			}
			$pt .= ')';
			$temp[] = $pt;
		}
		$text .= join(',',$temp).'));';
		return $text;
	}
}

class TNOrmIterator extends TIteratorAdapter {
	private $_classname_;
/**
 * @var TNOrmAdapter
 */	
	private $_adapter_;

	private $_assumed_descendants_ = array();
	
	private $_assumed_classes_ = array();
	
	public function __construct(IIterator $base, $classname, $adapter, array $assumed_descendants = array(), array $assumed_classes = array()){
		parent::__construct($base);
		$this->_classname_ = $classname;
		$this->_adapter_ = $adapter;
		if (!empty($assumed_descendants))
			$this->_assumed_descendants_ = $assumed_descendants;
		if (!empty($assumed_classes))
			$this->_assumed_classes_ = $assumed_classes;
	}
	
	public function Item() {
		$row = parent::Item();
		if (!is_null($row))
			return $this->_adapter_->Create(parent::Item(), $this->_classname_, $this->_assumed_descendants_, $this->_assumed_classes_);
		return null;
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
	protected $assumedDescendants = array();
	
	protected $value = false;
	
	public function __construct($object,TNOrmAdapter $adapter, array $assumed_descendants = array()){
		$this->object = $object;
		$this->adapter = $adapter;
		$this->assumedDescendants = $assumed_descendants;
	}
	
	public function Value(){
		if ($this->value === false)
			$this->value = $this->adapter->FetchContainers($this->object,$this->assumedDescendants);
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
	private $_empty_key_ = false;
	protected $assumeDescendants = array();
	
	protected function createDummy($meta){
		return new $meta['classname']();
	}
	
	public function __construct(&$row, &$meta, TNOrmAdapter $adapter, array $assume_descendants = array()){
		$this->adapter = $adapter;
		$this->assumeDescendants = $assume_descendants; 
		$this->dummy = $this->createDummy($meta);
		$keys = array();
		$props = array();
		foreach ($meta['fields'] as $field){
			$nm = $field['name'];
			$keys[] = $row->$nm;
			if (is_null($row->$nm))
				$this->_empty_key_ = true;
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
		if (!isset($this->value)){
			if (!$this->_empty_key_)
				$this->load();
		}
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
		$this->value = $this->adapter->Load($this->dummy,$this->assumeDescendants);
	}
}

class TNOrmMtmCollectionIterator extends TIteratorAdapter {
	private $_ref_prop_;
	
	public function __construct(IIterator $base,$refproperty){
		parent::__construct($base);
		$this->_ref_prop_ = $refproperty;
	}
	
	public function Item() {
		$item = parent::Item();
		$nm = $this->_ref_prop_;
		return $item->$nm;
	}
}

class TNOrmCollectionLoader extends TNOrmLoader {
	protected $sorting;
	private $_mtm_ref_prop_;
	
	public function __construct(&$row, &$meta, TNOrmAdapter $adapter, array $assume_descendants = array()){
		parent::__construct($row, $meta, $adapter,$assume_descendants);
		$this->sorting = array();
		foreach ($meta['sorting'] as $prop=>$type)
			$this->sorting[] = new TOrmSorting($prop,new TSortingType($type));
	}
	
	protected function createDummy($meta){
		if (count($meta['mtm']) > 1){
			$this->_mtm_ref_prop_ = $meta['mtm']['property']; 
			return new $meta['mtm']['class']();
		}
		return new $meta['classname']();
	}
	
	protected function load(){
		$this->value = $this->adapter->Fetch($this->dummy,array(),array(),$this->sorting,$this->assumeDescendants);
		if ($this->_mtm_ref_prop_)
			$this->value = new TNOrmMtmCollectionIterator($this->value,$this->_mtm_ref_prop_);
	}
}

interface INOrmObject {
/**
 * @return bool
 */	
	public function WasMoved();
}

class TNOrmObject implements INOrmObject {
	//private $_origins_ = array();
	
	private $_loaders_ = array();
	
	private $_was_moved_ = false;
	
	private function _check_prefixed_property($nm,$prefix){
		$rc = new ReflectionClass(get_class($this));
		$prop = '_'.$prefix.preg_replace_callback('/([A-Z][a-z\d]*)/',create_function('$matches','return "_".strtolower($matches[0]);'),$nm).'_';
		if ($rc->hasProperty($prop)){
			$p = $rc->getProperty($prop);
			if (!$p->IsPublic() && !$p->IsStatic())
				return $prop;
		}			
		return false;
	}
		
	public function __set($nm,$value){
		if ($n = $this->_check_prefixed_property($nm,'orm')){
			$this->$n = $value;
			if ($value instanceof TNOrmLoader){
				$this->_loaders_[$n] = $value;
				$this->$n = null;
			}
			/*if (!isset($this->_origins_[$n]))
				$this->_origins_[$n] = ($value instanceof TNOrmLoader)?$value->Key():$value;*/
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
		return null;
	}
	
	public function WasMoved(){
		return $this->_was_moved_;	
	}
} 

/**
 * @author dkrasilnikov@gmail.com
 * @property IDBDriver $Driver
 */

class TNOrmAdapter extends TConfigurable implements IORMStorage, IORMHierarchyStorage, ITransactionProvider {
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

	
	private function _form_ds_field($orm,$meta,$tables,&$fields){
		switch ($meta['type']){ 
			case 'reference':{
				foreach ($meta['fields'] as $field)
					$fields[$field['table'].'.'.$field['name']] = $this->_create_field($field['name'], $tables[$field['table']], empty($orm['nestedsets'])?0:1);
			}break;
			case 'scalar':$fields[$meta['field']['table'].'.'.$meta['field']['name']] = $this->_create_field($meta['field']['name'], $tables[$meta['field']['table']], empty($orm['nestedsets'])?0:1);break;
		}
	}
	
	private function _form_ds_join($type,$ds,$prevkeys,$keys,$table,&$tables,&$join){
		$conditions = array();
		$n = count($keys);
		$tables[$table] = new TTable($table);
		for ($i = 0; $i < $n; $i++)
			$conditions[] = new TCondition(TConditionType::C_EQUAL, array(new TTableField($keys[$i],null,$tables[$table]),new TTableField($prevkeys[$i],null,$ds)));
		$join[] = new TJoin($tables[$table], $type, $conditions); 
	}
	
	private function _form_assumed_descender($ds,$baseorm,$classname,$basekeys,&$join,&$tables,&$fields,&$keyfields){
		$orm = $this->_get_orm_config($classname);
		$keys_set = false;
		$keyfields = array();
		foreach ($orm['tables'] as $table=>$keys)
			if (!isset($tables[$table])){
				$this->_form_ds_join(TJoin::JOIN_LEFT, $ds, $basekeys, $keys, $table, $tables, $join);
				if (!$keys_set && ($classname == $orm['tdefclasses'][$table])){
					foreach ($keys as $keyfield){
						$alias = strtolower($classname).'_'.$keyfield;
						$keyfields[] = $keyfield;
						$fields[$table.'.'.$keyfield] = new TTableField($keyfield,$alias,$tables[$table]);
					}
					$keys_set = true;	
				}
			}
				
		foreach ($orm['properties'] as $name=>$meta)
			if ($meta['defclass'] == $orm['classname'])
				$this->_form_ds_field($orm,$meta,$tables,$fields);
	}
	
	private function _form_ds($orm,&$join,&$tables,&$fields,&$assumed_descendants){
		$prevkeys = null;
		
		foreach ($orm['tables'] as $table=>$keys){
			if (!isset($ds)){
				if (empty($orm['nestedsets']))
					$tables[$table] = new TTable($table,null,array(),array(),array(),array(), true, null, null, true);
				else	
					$tables[$table] = new TNestedSets($table,null,null,null,null,array(),array(),array(),array(), true, null, null, true);
				$ds = $tables[$table];	
				$prevkeys = $keys;
			} else 
				$this->_form_ds_join(TJoin::JOIN_INNER, $ds, $prevkeys, $keys, $table, $tables, $join);
		}
		
		foreach ($orm['properties'] as $name=>$meta){
			$this->_form_ds_field($orm,$meta,$tables,$fields);
		}
		
		if (!empty($assumed_descendants)){
			usort($assumed_descendants, create_function('$a,$b','return (is_subclass_of($a,$b))?1:0;'));
			$real_descendants = array();
			foreach ($assumed_descendants as $cname)
				if (is_subclass_of($cname, $orm['classname'])){
					$this->_form_assumed_descender($ds,$orm, $cname,  $prevkeys, $join, $tables, $fields,$kv);
					$real_descendants[$cname] = $kv;
				}	
			$assumed_descendants = array_reverse($real_descendants,true);
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
		$result = array();
		if ($orm = $this->_get_orm_config($object)){
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
		}
		return $result;
	}
	
	private function _create_field($name,$table,$type){
		switch ($type){
			case 1:return new TNestedSetsField($name,TNestedSetsField::SUBSET_FIELD,null,$table);
			case 2:return new TNestedSetsField($name,TNestedSetsField::BASE_FIELD,null,$table);
		}
		return new TTableField($name,null,$table);
	}
	
	private function _form_filter_condition(array &$filter, $object, $prop, array $meta, array $tables, $fieldtype){
			if (!is_null($object->$prop)){
				switch ($meta['type']){
					case 'reference':{
						$v = $this->_reference_value($object->$prop,$meta);
						$n = count($meta['fields']);
						for ($i = 0; $i < $n; $i++){
							$operands = array($this->_create_field($meta['fields'][$i]['name'], $tables[$meta['fields'][$i]['table']], ($tables[$meta['fields'][$i]['table']] instanceof TNestedSets)?$fieldtype:0),$v[$i]);
							$filter[] = new TCondition(TConditionType::C_EQUAL, $operands);
						}
					}break;
					case 'collection':break;
					case 'scalar':
					default:{
						$operands = array($this->_create_field($meta['field']['name'], $tables[$meta['field']['table']], ($tables[$meta['field']['table']] instanceof TNestedSets)?$fieldtype:0),$this->_get_scalar_prop($object,$prop,$meta['datatype']));
						$filter[] = new TCondition(TConditionType::C_EQUAL, $operands);
					}break;
				}
			}
	}
	
	private function _filter_from_object($orm,$object,&$tables,$fieldtype = 1){
		$filter = array();
		foreach ($orm['keys'] as $prop)
			$this->_form_filter_condition($filter, $object, $prop, $orm['properties'][$prop], $tables, $fieldtype);
			
		if (count($filter) == count($orm['keys']))
			return $filter;	
		
		foreach ($orm['properties'] as $prop=>$meta)
			if (!in_array($prop,$orm['keys']))
				$this->_form_filter_condition($filter, $object, $prop, $meta, $tables, $fieldtype);
				
		return $filter;
	}
	
/**
 * starts transaction
 * @return bool
 */			
	public function BeginTransaction(){
		return $this->_ioc_driver_->BeginTransaction();
	}
/**
 * commits transaction
 * @return bool
 */		
	public function CommitTransaction(){
		return $this->_ioc_driver_->CommitTransaction();
	}
/**
 * rollbacks transaction
 * @return bool
 */		
	public function RollbackTransaction(){
		return $this->_ioc_driver_->RollbackTransaction();
	}	
	
	
/**
 * Fills an object with missing data
 * @param object $object
 * @return object
 */	
	public function Load($object, array $assume_descendants = array()){
		if ($orm = $this->_get_orm_config($object)){
			$join = array(); 
			$tables = array();
			$fields = array();
			$assume_classes = $assume_descendants;
			$ds = $this->_form_ds($orm, $join, $tables, $fields,$assume_descendants);
			$ds->Fields = $fields;
			$ds->Filter = $this->_filter_from_object($orm, $object,$tables);
			$ds->Join = $join;
			$result = $this->_ioc_driver_->FetchRecords($ds);
			foreach ($result as $item){
				return $this->Create($item,$object,$assume_descendants,$assume_classes);
			}
		}
		return null;
	}
	
	
	private function _check_save_tables($table,$orm,&$tables,$keyvalues){
		if (!isset($tables[$table])){
			$tables[$table] = array(
				'keys' => array(),
				'values' => array()
			);
			$n = count($orm['tables'][$table]);
			if (!empty($keyvalues))
				for ($i = 0; $i < $n; $i++)
					$tables[$table]['keys'][$orm['tables'][$table][$i]] = @$keyvalues[$i];
		}
	}
	
	
	private function _entity_keys($object,$orm,&$emptyid){
		$keyvalues = $this->_get_object_key_values($object);
		$emptyid = false;
		$n = count($orm['keys']);
		if (count($keyvalues) != $n)
			$emptyid = true;
		return $keyvalues;
	}
	
/** 
 * Saves object to storage, returns saved object
 * @param object $object
 * @return object
 */	
	public function Save($object, array $propeties_mask = array()){
		if ($orm = $this->_get_orm_config($object)){
			$keyvalues = $this->_entity_keys($object, $orm, $emptyid);
			$tables = array();
		
			foreach ($orm['properties'] as $prop => $meta){
				if (($meta['type'] != 'collection') && (empty($propeties_mask) || isset($properties_mask[$prop])))
				{
					switch ($meta['type']){
						case 'reference':{
							$v = $this->_reference_value($object->$prop,$meta);
							$n = count($meta['fields']);
							for ($i = 0; $i < $n; $i++){
								$this->_check_save_tables($meta['fields'][$i]['table'], $orm, $tables,$keyvalues);
								if (!isset($tables[$meta['fields'][$i]['table']]['keys'][$meta['fields'][$i]['name']]))
									$tables[$meta['fields'][$i]['table']]['values'][$meta['fields'][$i]['name']] = $v[$i];
							}
						}break;
						case 'scalar':
						default:{
							$this->_check_save_tables($meta['field']['table'], $orm, $tables,$keyvalues);
							if (!isset($tables[$meta['field']['table']]['keys'][$meta['field']['name']]))
								$tables[$meta['field']['table']]['values'][$meta['field']['name']] = $this->_get_scalar_prop($object,$prop,$meta['datatype']);
						}break; 
					}	
				}
			}	
		
			$this->_ioc_driver_->BeginTransaction();
			try {
				$maintable = true;
				foreach ($orm['tables'] as $t=>$keyfields){
					$table = $tables[$t];
					$filter = array();
					foreach ($table['keys'] as $fld=>$value)
						$filter[] = new TCondition(TConditionType::C_EQUAL, array(new TTableField($fld),$value));
					$fields = array();
					$values = array();
					foreach ($table['values'] as $fld=>$value){
						$fields[] = new TTableField($fld);
						$values[] = $value;
					}
				
					$apcent = false;
					if (!$emptyid){
						if ($orm['identity'] == 1){
							$apcent = true;
							$kn = array_keys($table['keys']);
							$check_exists = new TTable($t,null,array(new TTableField($kn[0])), $filter);
							$check = $this->_ioc_driver_->FetchRecords($check_exists);
							foreach ($check as $chk){
								$apcent = false;
								break;
							}
						}
					}
				
					$ds = new TTable($t, null, array(), $filter, array(), array());
					
					if ($emptyid || $apcent){
						$merged = $table['keys'] + $table['values'];
						if ($maintable && !empty($orm['nestedsets'])){
							$parentprop = $orm['nestedsets']['parent'];
							$filter = array();
							if (is_object($object->$parentprop)){
								$pkeyvalues = $this->_entity_keys($object->$parentprop, $orm,$pemptyid);
								foreach ($keyfields as $i => $fld)
									$filter[] = new TCondition(TConditionType::C_EQUAL, array(new TNestedSetsField($fld),@$pkeyvalues[$i]));
							}
							$ds = new TNestedSets($t, null, null, null, null, array(), $filter, array(), array());
							$this->_ioc_driver_->NestedSetAddRecord($ds, $merged,$pk);
						} else
							$this->_ioc_driver_->InsertRecords($ds, array(array_values($merged)), array_keys($merged),$pk);
						
						if ($emptyid && $orm['identity'] == 0 && $maintable){
							foreach ($tables as $tn=>&$tbl)
								if ($tn != $t)
									$tbl['keys'][$orm['tables'][$tn][0]] = $pk;
							
							$c = get_class($object);
							$object = new $c(); 
							foreach ($orm['keys'] as $key) 
								$object->$key = $pk;
						}
					} else if (!empty($fields)){
						$this->_ioc_driver_->UpdateRecords($ds, $fields, $values);
						if ($object instanceof INOrmObject){
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
		return null;
	}
	
	public function Delete($object){
		if ($orm = $this->_get_orm_config($object)){
			$keyvalues = $this->_entity_keys($object, $orm, $emptyid);
			if ($emptyid){
				throw new TNOrmException(TNOrmException::ERR_MISSING_KEY);
			}
		
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
					if ($maintable && !empty($orm['nestedsets'])){
						for ($i = 0; $i < $n; $i++)
							$filter[] = new TCondition(TConditionType::C_EQUAL, array(new TNestedSetsField($keys[$i]),$keyvalues[$i]));	
						$ds = new TNestedSets($table, null, null, null, null, array(), $filter, array(), array());
						$result = $result && $this->_ioc_driver_->NestedSetBranchesDelete($ds);
					} else {
						for ($i = 0; $i < $n; $i++)
							$filter[] = new TCondition(TConditionType::C_EQUAL, array(new TTableField($keys[$i]),$keyvalues[$i]));	
						$ds = new TTable($table, null, array(), $filter, array(), array(),true);
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
		return false;
	}

	public function FetchContainers($object, array $assume_descendants = array()){
		if ($orm = $this->_get_orm_config($object)){
			$join = array(); 
			$tables = array();
			$fields = array();
			$assume_classes = $assume_descendants;
			$ds = $this->_form_ds($orm, $join, $tables, $fields, $assume_descendants);
			$ds->Filter = $this->_filter_from_object($orm, $object, $tables, 2);
			$ds->Direction = TNestedSets::DIR_UP;
			$ds->IncludeBases = false;
			$ds->Fields = $fields;
			$ds->Join = $join;
			$ns = $this->_ioc_driver_->NestedSetFetch($ds);
			$parentprop = $orm['nestedsets']['parent'];
			$item = false;
			foreach ($ns as $nse){
				$item = $this->Create($nse,$orm['classname'],$assume_descendants,$assume_classes);
				if (isset($p))
					$item->$parentprop = $p;
				else 
					$item->$parentprop = false;
				$p = $item;	
			}
			return $item;
		}
		return null;
	}
	
	
	private function _filter_from_conditions(array &$filter,$orm,&$tables,$conditions, &$join){
		foreach ($conditions as $condition)
			if ($condition instanceof TOrmCondition){
				$operands = array();
				foreach ($condition->Operands as $o)
					if (is_string($o) && isset($orm['properties'][$o])){
						switch ($orm['properties'][$o]['type']){
							case 'collection':{
								if (count($orm['properties'][$o]['mtm']) > 1){
									$colorm = $this->_get_orm_config($orm['properties'][$o]['mtm']['class']);
									$fp = $colorm['properties'][$orm['properties'][$o]['mtm']['property']];
								} else {
									$colorm = $this->_get_orm_config($orm['properties'][$o]['classname']);
									if (count($colorm['keys']) > 1)
										throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_FILTER);
									$fp = $colorm['properties'][$colorm['keys'][0]];
								}
								
								if (count($fp['fields']) > 1)
									throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_FILTER);
								
								$coljoin = array();
								$fields = array();
								$ad = array(); 
								$colds = $this->_form_ds($colorm, $coljoin, $tables, $fields, $ad);
								$conditions = array();
								foreach ($orm['properties'][$o]['fields'] as $fld){
									$jp = $colorm['properties'][$fld['property']];
									if (count($jp['fields']) > 1)
										throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_FILTER);
									$jfld = $jp['fields'][0];	
									$conditions[] = new TCondition(TConditionType::C_EQUAL, array(
										$this->_create_field($fld['name'], $tables[$fld['table']], ($tables[$fld['table']] instanceof TNestedSets)?1:0),
										$this->_create_field($jfld['name'], $tables[$jfld['table']], ($tables[$jfld['table']] instanceof TNestedSets)?1:0)
									));
								}
								
								$join[] = new TJoin($colds, TJoin::JOIN_INNER, $conditions); 	
									
								foreach ($fp['fields'] as $field)
									$operands[] = $this->_create_field($field['name'], $tables[$field['table']], ($tables[$field['table']] instanceof TNestedSets)?1:0); 
							}break;
							case 'reference':{
								if (count($orm['properties'][$o]['fields']) > 1)
									throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_FILTER);
								foreach ($orm['properties'][$o]['fields'] as $field)
									$operands[] = $this->_create_field($field['name'], $tables[$field['table']], ($tables[$field['table']] instanceof TNestedSets)?1:0); 
							}break;
							case 'scalar':
							default: $operands[] = $this->_create_field($orm['properties'][$o]['field']['name'], $tables[$orm['properties'][$o]['field']['table']], ($tables[$orm['properties'][$o]['field']['table']] instanceof TNestedSets)?1:0);
						}
					} else 	
						$operands[] = $o;
								
				$filter[] = new TCondition($condition->Type, $operands);
			} else if ($condition instanceof TOperation){
				$temp = array();
				$this->_filter_from_conditions($temp, $orm, $tables, $condition->Operands, $join);
				$filter[] = ($condition instanceof TCondition)?new TCondition($condition->Type,$temp):new TOperation($condition->Type,$temp);
			}
	}
	
	private function _form_sorting($orm,$tables,$sorting){
		$sort = array();
		foreach ($sorting as $s){
			if (isset($orm['properties'][$s->Attribute]))
				switch ($orm['properties'][$s->Attribute]['type']){
					case 'collection':break;
					case 'reference':{
						foreach ($orm['properties'][$s->Attribute]['fields'] as $field){
							$sort[] = new TSorting(($s->Type == TSortingType::SORT_SHUFFLE)?new TTableField(''):$this->_create_field($field['name'], $tables[$field['table']], ($tables[$field['table']] instanceof TNestedSets)?1:0), $s->Type);
						}
					}break;
					default:$sort[] = new TSorting(($s->Type == TSortingType::SORT_SHUFFLE)?new TTableField(''):$this->_create_field($orm['properties'][$s->Attribute]['field']['name'], $tables[$orm['properties'][$s->Attribute]['field']['table']], ($tables[$orm['properties'][$s->Attribute]['field']['table']] instanceof TNestedSets)?1:0), $s->Type);break;	
				}
		}
		return $sort;
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
	public function Fetch($dummy,array $greedy = array(),array $conditions = array(),array $sorting = array(), array $assume_descendants = array(), $offset = null, $count = null, &$total = null){
		if ($orm = $this->_get_orm_config($dummy)){			
			$join = array(); 
			$tables = array();
			$fields = array();	
			
			$break = !empty($assume_descendants);
			$assume_classes = $assume_descendants;
			$ds = $this->_form_ds($orm, $join, $tables, $fields, $assume_descendants);
			
			if (is_object($dummy)){
				$filter = $this->_filter_from_object($orm, $dummy, $tables);
			}
								
			$this->_filter_from_conditions($filter, $orm, $tables, $conditions, $join);
			
			$ds->Filter = $filter;
			$ds->Join = $join;
						
			if (!is_null($total)){
				$cds = clone $ds;
				$cfields = array(); 
				foreach ($orm['keys'] as $keyprop)
					if (isset($orm['properties'][$keyprop])){
						switch ($orm['properties'][$keyprop]['type']){
							case 'scalar':$cfields[] = $this->_create_field($orm['properties'][$keyprop]['field']['name'], $orm['properties'][$keyprop]['field']['table'], 0);break;
							case 'reference':{
								foreach ($orm['properties'][$keyprop]['fields'] as $cfld)
									$cfields[] = $this->_create_field($cfld['name'], $cfld['table'], 0);break;
							}break;
						}
					}
				$cds->Fields = $cfields;
				$cds = new TDataSet(new TDataSet($cds,'cnt_base'),null,array(new TExpressionField('cnt', new TOperation(TOperationType::O_COUNT, array()))));
				$cntr = $this->_ioc_driver_->FetchRecords($cds);
				foreach ($cntr as $cnt){
					$total = $cnt['cnt'];
					break;
				}
			}			
			
			$ds->Fields = $fields;
			$ds->Sorting = $this->_form_sorting($orm, $tables, $sorting);
			$ds->Offset = $offset;
			$ds->Count = $count;	
			return new TNOrmIterator($this->_ioc_driver_->FetchRecords($ds),$orm['classname'],$this,$assume_descendants,$assume_classes);
		}
		return false;
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
	
	private function _assign_property($dummy,$property,$meta,$row, array $assume_descendants = array()){
		switch ($meta['type']){
			case 'scalar':{
				$nm = $meta['field']['name'];
				$v = $row->$nm;
				switch ($meta['datatype']){
					case 'bool':$v = TConvertions::ConvertToBoolean($v);break;
					case 'date':
					case 'TDate':{
						if (!is_null($v)){
							if ($this->_date_format_){
								$v = date_create_from_format($this->_date_format_,$v);
								$v = $v->getTimestamp();
							}
							$v = new TDate($v);
						}
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
					$dummy->$property = new TNOrmObjectLoader($row,$meta,$this,$assume_descendants);
			}break;
			case 'collection':{
				if (!isset($dummy->$property)) 
					$dummy->$property = new TNOrmCollectionLoader($row,$meta,$this,$assume_descendants);	
			}break;
		}
	}

	public function Properties($dummy){
		if ($orm = $this->_get_orm_config($dummy)){
			$result = array();
			foreach ($orm['properties'] as $key=>$prop){
				if ($prop['type'] == 'scalar'){
					$type = TItemPropertyType::PT_STRING;
					switch ($prop['datatype']){
						case 'string':break;
						case 'int':$type = TItemPropertyType::PT_INT;break;
						case 'float':$type = TItemPropertyType::PT_DEC;break;	
						case 'date': 
						case 'TDate':$type = TItemPropertyType::PT_DATE;break;
						case 'bool':$type = TItemPropertyType::PT_BOOL;break;
					}
					$result[$key] = new TOrmPropertyMeta($key, $key, $type); 
				} else {
					switch ($prop['type']){
						case 'collection':$result[$key] = new TOrmCollectionPropertyMeta($key, $key, $prop['classname']);break;
						case 'reference':$result[$key] = new TOrmReferencePropertyMeta($key, $key, $prop['classname']);break; 
					} 
				}
			}
			return $result;
		}
		return false;
	}
	
	private function _get_ns_property_name($object,$nm){
		if ($orm = $this->_get_orm_config($object))
			if (!empty($orm['nestedsets']))
				return $orm['nestedsets'][$nm];
		throw new TCoreException(TCoreException::ERR_BAD_TYPE);
	}
	
	private function _get_ns_property($object,$nm){
		$p = $this->_get_ns_property_name($object, $nm);
		return $object->$p;
	}
	
	public function Put($object,$container){
		$p = $this->_get_ns_property_name($object, 'parent');
		$object->$p = $container;
	}
	
	public function Container($object){
		return $this->_get_ns_property($object, 'parent');
	}
	
	public function Contents($object){
		return $this->_get_ns_property($object, 'children');
	}
	
	private function _sort_nested_sets(array &$items,$childprop,$f){
		usort($items, $f);	
		foreach ($items as $item)
			$this->_sort_nested_sets($item->$childprop, $childprop,$f);
	}
	
	public function FetchContents($dummy,$direct = true, array $greedy = array(),array $conditions = array(),array $sorting = array(), array $assume_descendants = array(), $offset = null, $count = null,  &$total = null){
		if ($orm = $this->_get_orm_config($dummy)){
			if (empty($orm['nestedsets']))
				throw new TCoreException(TCoreException::ERR_BAD_TYPE);
			$join = array(); 
			$tables = array();
			$fields = array();
			$assume_classes = $assume_descendants;
			$ds = $this->_form_ds($orm, $join, $tables, $fields, $assume_descendants);
			if (is_object($dummy))
				$filter = $this->_filter_from_object($orm, $dummy, $tables, 2);
		
			$this->_filter_from_conditions($filter, $orm, $tables, $conditions, $join);
			$ds->Fields = $fields;
			$ds->Join = $join;
			$ds->Filter = $filter;
			$ds->Direction = TNestedSets::DIR_DOWN;
			$ds->IncludeBases = false;
			$ds->Offset = $offset;
			$ds->Count = $count;
			if ($direct){
				$ds->Depth = 0;
				$ds->Sorting = $this->_form_sorting($orm, $tables, $sorting);
				return new TNOrmIterator($this->_ioc_driver_->NestedSetFetch($ds), $orm['classname'], $this, $assume_descendants,$assume_classes);
			} 
			$ns = $this->_ioc_driver_->NestedSetFetch($ds);
			$result = array();
			$items = array();
			$childprop = $orm['nestedsets']['children'];
			foreach ($ns as $nse){
				$item = $this->Create($nse,$orm['classname'],$assume_descendants,$assume_classes);
				$item->$childprop = array();
				$items[join('|',$this->_entity_keys($item, $orm, $remptyid))] = $item;
				if (is_null($nse->NestingParent())){
					$result[] = $item;
				} else {
					$p = $this->Create($nse->NestingParent(), $orm['classname'],$assume_descendants,$assume_classes);
					$p = $items[join('|',$this->_entity_keys($p,$orm, $remptyid))];					
					array_push($p->$childprop,$item); 
				}
			}
			if (!empty($sorting)){
				$f = '$r = 0;';
				foreach ($sorting as $s){
					if ($orm['properties'][$s->Attribute]['type'] == 'scalar'){
						switch ($orm['properties'][$s->Attribute]['datatype']){
							case 'date':
							case 'TDate':$f .= '$r = $a->'.$s->Attribute.'->TimeStamp() - $b->'.$s->Attribute.'->TimeStamp();';break;
							case 'int':$f .= '$r = $a->'.$s->Attribute.' - $b->'.$s->Attribute.';';break;
							case 'float':$f .= '$r = $a->'.$s->Attribute.' - $b->'.$s->Attribute.';$r = ($r <> 0)?($r/abs($r)):$r;';break;
							case 'bool':$f .= '$r = (int)$a->'.$s->Attribute.' - (int)$b->'.$s->Attribute.';';break;
							default:$f .= '$r = strcmp($a->'.$s->Attribute.',$b->'.$s->Attribute.');';break;
						}
					}
					if ($s->Type == TSortingType::SORT_DESC)
						$f .= '$r = -$r;';
					$f .= 'if ($r <> 0) return $r;';
				} 	
				$f .= 'return 0;';	
				$this->_sort_nested_sets($result, $childprop, create_function('$a,$b',$f));
			}
			return new TArrayIterator($result);
		}
		return false;
	}
	
/**
 * Instantiates or loads an object
 * @param mixed $row
 * @param object|string $dummy
 */	
	public function Create($row,$dummy, array $assumed_descendants = array(), array $assumed_classes = array()){
		if (is_array($row))
			$row = (object)$row;
		if ($orm = $this->_get_orm_config($dummy)){
			$cn = $orm['classname'];
			foreach ($assumed_descendants as $c=>$keyfields){
				$isit = true;
				foreach ($keyfields as $kf){
					$kf = strtolower($c.'_'.$kf);
					if (!isset($row->$kf))
						$isit = false;
				}
				if ($isit){
					$cn = $c;
					break;
				}
			}
			if ($cn != $orm['classname']){
				$orm = $this->_get_orm_config($cn);
				if (!$orm)
					throw TCoreException(TCoreException::ERR_BAD_VALUE); 
			}	
			$dummy = new $cn();
	
			foreach ($orm['keys'] as $key)
				$this->_assign_property($dummy, $key, $orm['properties'][$key], $row, $assumed_classes);
			
			$dummy = $this->_check_loaded($dummy, $orm);	

			if (!empty($orm['nestedsets'])){
				$parentprop = $orm['nestedsets']['parent'];
				$dummy->$parentprop = new TNOrmBranchLoader($dummy,$this,$assumed_classes);
			}		
			
			foreach ($orm['properties'] as $name=>$prop){
				if (!in_array($name,$orm['keys']))
					$this->_assign_property($dummy, $name, $prop, $row, $assumed_classes);
			}		
			return $dummy;
		}
		return null;
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
		else if ($object instanceof ReflectionClass){
			$cn = $object->getName();
			$rc = $object;
		} else if (is_object($object)) 
			$cn = get_class($object);
		else throw new TCoreException(TCoreException::ERR_BAD_VALUE);

		if (!isset($this->_loaded_orms_[$cn])){
			if (!$this->_script_config_path_)
				$this->_script_config_path_ = TApplication::$Application->AdjustPath('private/orm');
				
			TFileSystem::ForceDir($this->_script_config_path_);	
				
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
					if ($this->_use_config_ & self::CONF_XML){
						$fn = $this->_xml_config_path_.'/'.$rc->getName().'.orm.xml';
						if (file_exists($fn) && filemtime($fn) > $src_time)
							$src_time = filemtime($fn);
					}		
					array_unshift($classes, $rc);
					$rc = $rc->getParentClass();
				}
						
						
				if (!file_exists($filename) || (filemtime($filename) < $src_time)){
					$orm = new TClassOrmConfig($cn);

					if ($this->_use_config_ & self::CONF_XML){
						foreach ($classes as $rc)
							if (file_exists($this->_xml_config_path_.'/'.$rc->getName().'.orm.xml'))	
								$orm->ParseXml($this->_xml_config_path_.'/'.$rc->getName().'.orm.xml');
					}
					
					if ($this->_use_config_ & self::CONF_PHPDOC)
						foreach ($classes as $rc)
							$orm->ParseClass($rc);
					if ($orm->IsOrmClass())	
						file_put_contents($filename, $orm);
				}	
				$ormconfig = false;
				if (file_exists($filename))
					include $filename;
				$this->_loaded_orms_[$cn] = $ormconfig;
			}
		} 
		return $this->_loaded_orms_[$cn];	
	}
}