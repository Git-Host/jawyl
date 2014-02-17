<?php
require_once 'common/orm.inc';
require_once 'common/acid.inc';
require_once 'core/utils/repository.inc';
require_once 'core/utils/TSortUtils.inc';

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
			$this->_properties_[$name]['greedy'] = isset($settings['greedy'])?$settings['greedy']:false;
			$this->_properties_[$name]['mtm'] = explode('.',trim($settings['mtm']));
			$this->_parse_refprop($name,$settings['type'],$settings['fields'],isset($settings['sorting'])?$settings['sorting']:array(),$default_tbl);
	}
	
	public function ParseClass(ReflectionClass $rc){
		$doc = $rc->getDocComment();
		$lineend = '.*$/m';
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
				}/* elseif ($datatype && preg_match('/@orm.(reference|collection)\s+((?:[_\w]+\.[_\w]+\s+)?)\s*((?:(?:[_\w]+)\s*\((?:(?:\s*\w[_\w\.]*\s*,?)+)\)\s*,?\s*)+)(?:\s+-s\s+((?:[_\w]+\((?:asc|desc)\))+))?\s*$/m', $doc,$matches)){
				$this->_override_ref_property($p->getName(), array('type'=>$matches[1],'classname'=>$datatype,'mtm'=>$matches[2][$i],'fields'=>$matches[3],'sorting'=>$matches[4]),$default_tbl,$rc->getName());
			}*/
		}
		
		preg_match_all('/@property\s+([_\w]+)\s+\$([_\w]+)\s*\{@orm.scalar\s+(\w[_\w\.]*)\s*\}'.$lineend,$doc,$matches);
		$n = count($matches[2]);
		for ($i = 0; $i < $n; $i++){
			$fld = explode('.',$matches[3][$i]);
			$this->_override_scalar_property($matches[2][$i], array('datatype'=>$matches[1][$i],'field'=>array('name'=>count($fld) > 1?$fld[1]:$fld[0],'table'=>count($fld) > 1?$fld[0]:$default_tbl)),$rc->getName());
		}
		
		preg_match_all('/@property\s+([_\w]+)\s+\$([_\w]+)\s*\{@orm.(reference|collection)\s+((?:[_\w]+\.[_\w]+\s+)?)\s*((?:(?:[_\w]+)\s*\((?:(?:\s*\w[_\w\.]*\s*,?)+)\)\s*,?\s*)+)((?:\s+-s\s+[^\}]*)?)(\s+greedy|\s*)\s*\}'.$lineend,$doc,$matches);
		$n = count($matches[2]);
		for ($i = 0; $i < $n; $i++){
			$this->_override_ref_property($matches[2][$i], array('type'=>$matches[3][$i],'classname'=>$matches[1][$i],'mtm'=>$matches[4][$i],'fields'=>$matches[5][$i],'sorting'=>$matches[6][$i],'greedy'=>trim($matches[7][$i]) != ''),$default_tbl,$rc->getName());
		}
	}

	public function ParseXml($filename){
		$cname = basename($filename,'.orm.xml');
		$xml = new DOMDocument();
		$xml->preserveWhiteSpace = false;
		$xml->Load($filename);
		if (!$xml->schemaValidate($this->Application()->SysPath()."/schemas/norm.xsd"))
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
		$prop_index = 0;
		
		$greedy = array();
		
		foreach ($this->_properties_ as $prop=>$data){
			$pt = '"'.$prop.'"=>array("index"=>'.$prop_index.',"type"=>"'.$data['type'].'","defclass"=>"'.$data['defclass'].'",';
			switch ($data['type']){
				case 'scalar':$pt .= '"datatype"=>"'.$data['datatype'].'","field"=>array("name"=>"'.$data['field']['name'].'","table"=>"'.$data['field']['table'].'")';break;
				case 'reference':
				case 'collection':{
					if ($data['greedy'])
						$greedy[] = '"'.$prop.'"';
					
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
						$pt .= join(',',$temp2).')';
					}
				}break;
			}
			$pt .= ')';
			$temp[] = $pt;
			$prop_index++;
		}
		$text .= join(',',$temp).'),"greedy"=>array('.join(',',$greedy).'));';
		return $text;
	}
}

class TNOrmIterator extends TIteratorAdapter {
	private $_classname_;
/**
 * @var TNOrmAdapter
 */	
	protected $adapter;

	private $_assumed_descendants_ = array();
	
	private $_assumed_classes_ = array();
	
	public function __construct(IIterator $base, $classname, $adapter, array $assumed_descendants = array(), array $assumed_classes = array()){
		parent::__construct($base);
		$this->_classname_ = $classname;
		$this->adapter = $adapter;
		if (!empty($assumed_descendants))
			$this->_assumed_descendants_ = $assumed_descendants;
		if (!empty($assumed_classes))
			$this->_assumed_classes_ = $assumed_classes;
	}
	
	public function Item() {
		$row = parent::Item();
		if (!is_null($row))
			return $this->adapter->Create(parent::Item(), $this->_classname_, $this->_assumed_descendants_, $this->_assumed_classes_);
		return null;
	}
}

class TNOrmException extends TStorageException {
	const ERR_OBJECT_NOT_FOUND = 30001;
	const ERR_COMPOSITE_KEY_FILTER = 30002;
	const ERR_INVALID_ORM_CONFIG = 30003;
	const ERR_COMPOSITE_KEY_INCOMPLETE = 30004;
	const ERR_MISSING_KEY = 30005;
	const ERR_INVALID_PROPERTY = 30006;
	const ERR_NOT_REFERENCE = 30007;
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
			case self::ERR_INVALID_PROPERTY:return "Invalid property name %property specified!";break;
			case self::ERR_NOT_REFERENCE:return "Property %property specified for dereferencing is not a reference property!";break;
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
		if ($this->value === false){
			if ($this->value = $this->adapter->FetchContainers($this->object,$this->assumedDescendants))
				$this->adapter->ProvideUniqueness($this->value);
		}
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
	
	public function __sleep(){
		return array("key","value");
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
		if ($this->adapter instanceof IORMStorage){
			$this->adapter->LazyLoading = true;
			if ($this->value = $this->adapter->Load($this->dummy,$this->assumeDescendants))
				$this->adapter->ProvideUniqueness($this->value);
			$this->adapter->LazyLoading = false;
		}
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
		foreach ($meta['sorting'] as $sort){
			$this->sorting[] = new TOrmSorting(($this->_mtm_ref_prop_?$this->_mtm_ref_prop_.'.':'').$sort['property'],new TSortingType($sort['type']));
		}
	}
	
	protected function createDummy($meta){
		if (count($meta['mtm']) > 1){
			$this->_mtm_ref_prop_ = $meta['mtm']['property']; 
			return new $meta['mtm']['class']();
		}
		return new $meta['classname']();
	}
	
	protected function load(){
		$this->value = $this->adapter->Fetch($this->dummy,$this->_mtm_ref_prop_?array($this->_mtm_ref_prop_):array(),array(),$this->sorting,$this->assumeDescendants);
		if ($this->_mtm_ref_prop_)
			$this->value = new TNOrmMtmCollectionIterator($this->value,$this->_mtm_ref_prop_);
	}
	
	public function __sleep(){
		return array();
	}
}

class TNOrmObject {
	
	protected $loaders = array();
	
	private function _property_name_replace($matches){
		return "_".strtolower($matches[0]);
	}
	
	private function _check_prefixed_property($nm,$prefix){
		$rc = new ReflectionClass(get_class($this));
		$prop = '_'.$prefix.strtolower(preg_replace('/([A-Z][a-z\d]*)/','_$1',$nm)).'_';
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
			if (is_null($value))
				unset($this->loaders[$n]);
			if ($value instanceof TNOrmLoader){
				$this->loaders[$n] = $value;
				$this->$n = null;
			}
		} else if ($n = $this->_check_prefixed_property($nm,'ns')){
			if ($value instanceof TNOrmBranchLoader){
				$this->loaders[$n] = $value;
				$this->$n = null;
			} else if (($value === false) || (is_object($value) && (get_class($this) == get_class($value))))			
				$this->$n = $value;
			else throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		} 
	}
	
	public function __get($nm){		
		if (($n = $this->_check_prefixed_property($nm,'orm')) || ($n = $this->_check_prefixed_property($nm,'ns'))){
			if (is_null($this->$n)){					
				if (isset($this->loaders[$n])){
					$this->$n = $this->loaders[$n]->Value();
					unset($this->loaders[$n]);
				}
			}
			return $this->$n;
		}
		return null;
	}
	
	public function __sleep(){
		foreach ($this->loaders as $nm=>$loader){
			if ($loader instanceof TNOrmObjectLoader){
				if (is_null($this->$nm))
					$this->$nm = $loader->Value();
				unset($this->loaders[$nm]);
			}
		}
		return array_keys(get_object_vars($this));
	}
} 

/**
 * @author dkrasilnikov@gmail.com
 * @property IDBDriver $Driver
 * @property IKeyValueRepository $OrmSettings
 */

class TNOrmAdapter extends TConfigurable implements IORMStorage, IORMHierarchyStorage, ITransactionProvider {
	const CONF_SCRIPT = 0;
	const CONF_XML = 1;
	const CONF_PHPDOC = 2;
/**
 * @var IDBDriver
 */	
	protected $_ioc_driver_;
	
/**
 * @var IKeyValueRepository
 */	
	protected $_ioc_orm_settings_;
	
	private $_use_config_ = 3;
	
	protected $_bool_emulate_integrity_ = false;
	
	private $_xml_config_path_;
	private $_script_config_path_;
	private $_date_format_;
	
	protected $_bool_unique_objects_;
	
	protected $_array_class_uniqueness_ = array();
	
	private $_loaded_orms_ = array();
	
	private $_loaded_objects_ = array();
	
	private $_lazy_loading_ = false;

	
	private function _enrich_descendants($class, array $assumed_descendants){
		if ($this->OrmSettings){
			$osad = $this->OrmSettings->GetValue($class.".AssumedDescendants");
			if (is_array($osad))
				return array_unique(array_merge($assumed_descendants,$osad));
		}
		return $assumed_descendants;
	}
	
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
			array_filter($assumed_descendants,create_function('$a','return class_exists($a,false);'));
			TSortUtils::FastSort($assumed_descendants,create_function('$a,$b','return is_subclass_of($a,$b)?1:(is_subclass_of($b,$a)?-1:0);'));
			$real_descendants = array();
			foreach ($assumed_descendants as $cname)
				if (is_subclass_of($cname, $orm['classname'])){
					if ($pcname = get_parent_class($cname)){
						while ($pcname != $orm['classname']){
							if (!isset($real_descendants[$pcname])){
								$this->_form_assumed_descender($ds,$orm, $pcname,  $prevkeys, $join, $tables, $fields,$kv);
								//$real_descendants[$pcname] = $kv;
							}
							$pcname = get_parent_class($pcname);
						}
					}
				
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
			$refvalues[] = $value;	
	}
	
	private function _get_key_type($classname){
		if ($orm = $this->_get_orm_config($classname)){
			if (count($orm['keys']) > 1)
				throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_FILTER);
			switch ($orm['properties'][$orm['keys'][0]]['type']){
				case 'reference':return $this->_get_key_type($orm['properties'][$orm['keys'][0]]['classname']);break;
				case 'scalar':return $orm['properties'][$orm['keys'][0]]['datatype'];break;
				default:throw new TCoreException(TCoreException::ERR_BAD_VALUE);break;
			}
		}
		throw new TCoreException(TCoreException::ERR_BAD_VALUE);
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
		
		if ($value === ''){
			if ($this->_get_key_type($meta['classname']) != 'string')
				$value = null;
		}
		return is_array($value)?$value:array($value);
	}
	
	private function _get_scalar_prop($object,$prop,$datatype){
		if ($datatype != 'string' && $object->$prop === '')
			return null;
		switch ($datatype){
			case 'int':return (int)$object->$prop;break;
			case 'float':
			case 'double':return (float)$object->$prop;break;
			case 'bool':return is_null($object->$prop)?null:TConvertions::ConvertToBoolean($object->$prop);break;
			case 'date':
			case 'TDate':{
				$v = $object->$prop;
				if (!is_null($v)){
					if (!($v instanceof TDate))
						$v = new TDate($v);
					if ($this->_date_format_)
						return $v->ToString($this->_date_format_);
				}
				return $v;
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
							$result[] = $val;
					}break;
					case 'scalar':
					default:$result[] = $this->_get_scalar_prop($object,$propname,$orm['properties'][$propname]['datatype']);break;
				}
			}
		}
		return $result;
	}
	
	private function _create_field($name,$table,$type = 0, $alias = null){
		switch ($type){
			case 1:return new TNestedSetsField($name,TNestedSetsField::SUBSET_FIELD,$alias,$table);
			case 2:return new TNestedSetsField($name,TNestedSetsField::BASE_FIELD,$alias,$table);
		}
		return new TTableField($name,$alias,$table);
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
	public function Load($object, array $assume_descendants = array(), array $greedy = array()){
		if ($orm = $this->_get_orm_config($object)){	
			if ($this->_lazy_loading_ && ($this->_bool_unique_objects_ || isset($this->_array_class_uniqueness_[$orm['classname']]))){
				if ($result = $this->_loaded_object($object, $orm))
					return clone $result;
			}
			$join = array(); 
			$tables = array();
			$fields = array();
			$assume_descendants = $this->_enrich_descendants($orm["classname"],$assume_descendants);
			$assume_classes = $assume_descendants;			
			$ds = $this->_form_ds($orm, $join, $tables, $fields,$assume_descendants);
			$this->_form_greedy_properties($greedy, $orm, $fields, $tables, $join);						
			$ds->Fields = $fields;
			$f = $this->_filter_from_object($orm, $object,$tables);
			if (empty($f) == 0)
				return  null;
			$ds->Filter = $f;
			$ds->Join = $join;
			$result = $this->_ioc_driver_->FetchRecords($ds);
			foreach ($result as $item){
				$res = $this->Create($item,$object,$assume_descendants,$assume_classes);
				$this->ProvideUniqueness($res);
				return $res;
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
			for ($i = 0; $i < $n; $i++)
				if (isset($keyvalues[$i]))
					$tables[$table]['keys'][$orm['tables'][$table][$i]] = $keyvalues[$i];
		}
	}
	
	
	private function _entity_keys($object,$orm,&$emptyid){
		$keyvalues = $this->_get_object_key_values($object);
		$temp = array_filter($keyvalues);
		$emptyid = empty($temp);
		return $keyvalues;
	}
	
/** 
 * Saves object to storage, returns saved object
 * @param object $object
 * @return object
 */	
	public function Save($object, array $propeties_mask = array(), $origin = null){	
		if ($orm = $this->_get_orm_config($object)){
			if ($origin)
				$keyvalues = $this->_entity_keys($origin, $orm, $emptyid);
			else
				$keyvalues = $this->_entity_keys($object, $orm, $emptyid);
						
			$tables = array();
			
			foreach ($orm['tables'] as $t=>$keyfields)
				$this->_check_save_tables($t, $orm, $tables,$keyvalues);
			
			$sets = array();
			
			foreach ($orm['properties'] as $prop => $meta){
				if ($meta['type'] != 'collection')
				{
					switch ($meta['type']){
						case 'reference':{
							$v = $this->_reference_value($object->$prop,$meta);
							$n = count($meta['fields']);
							for ($i = 0; $i < $n; $i++)
							if (!(in_array($prop,$orm['keys']) && $orm['identity'] == 0) 
									&& (empty($propeties_mask) || isset($properties_mask[$prop]))){
								if (isset($v[$i]))	
									$tables[$meta['fields'][$i]['table']]['values'][$meta['fields'][$i]['name']] = $v[$i];
								else if (!isset($tables[$meta['fields'][$i]['table']]['values'][$meta['fields'][$i]['name']]))
									$tables[$meta['fields'][$i]['table']]['values'][$meta['fields'][$i]['name']] = null;
							}
						}break;
						case 'scalar':
						default:{
							if (!(in_array($prop,$orm['keys']) && $orm['identity'] == 0) && (empty($propeties_mask) || isset($properties_mask[$prop])))	
								$tables[$meta['field']['table']]['values'][$meta['field']['name']] = $this->_get_scalar_prop($object,$prop,$meta['datatype']);
						}break; 
					}	
				} else {
					$v = $object->$prop;
					if (!$emptyid && !empty($v) && is_array($v) && $meta['mtm']){
						$sets[$prop] = array();
						$o = new $meta['mtm']['class']();
						$n = count($keyvalues);
						for ($i = 0; $i < $n; $i++)
							$o->$meta['fields'][$i]['property'] = $keyvalues[$i];
							 
						foreach ($v as $id){
							$o->$meta['mtm']['property'] = $id;
							$sets[$prop][] = clone $o;
						}							
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
				
					$abcent = false;
					if (!$emptyid){
						if ($orm['identity'] == 1){
							$abcent = true;
							$kn = array_keys($table['keys']);
							$check_exists = new TTable($t,null,array(new TTableField($kn[0])), $filter);
							$check = $this->_ioc_driver_->FetchRecords($check_exists);
							foreach ($check as $chk){
								$abcent = false;
								break;
							}
						}
					}
				
					$ds = new TTable($t, null, array(), $filter, array(), array());
										
					if ($emptyid || $abcent){
						$merged = $table['keys'] + $table['values'];
						if ($maintable && !empty($orm['nestedsets'])){
							$parentprop = $orm['nestedsets']['parent'];
							$filter = array();
							if (is_object($object->$parentprop)){
								$pkeyvalues = $this->_entity_keys($object->$parentprop, $orm,$pemptyid);
								foreach ($keyfields as $i => $fld)
									$filter[] = new TCondition(TConditionType::C_EQUAL, array(new TNestedSetsField($fld),$pkeyvalues[$i]));
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
						if (!empty($orm['nestedsets']) && $maintable && $origin){	
							$parentprop = $orm['nestedsets']['parent'];
							$opp = $object->$parentprop;
							$orpp = $origin->$parentprop;
							$move = false;
							if (($opp && !$orpp) || (!$opp && $orpp)) 
								$move = true;
							else if ($opp && $orpp)
								$move = $this->_entity_keys($opp,$orm,$em1) != $this->_entity_keys($orpp, $orm, $em2);
							if ($move){
								$src = new TNestedSets($t, null, null, null, null, array(), $filter, array(), array());
								$filter = array();
								if (is_object($object->$parentprop))
									$filter = $this->_filter_from_object($orm, $object->$parentprop, $tables);
								$dest = new TNestedSets($t, null, null, null, null, array(), $filter, array(), array());
								$this->_ioc_driver_->NestedSetBranchesMove($src,$dest);
							}
						}
					}
					$maintable = false;
				}				
				$this->_ioc_driver_->CommitTransaction();
				if (!empty($sets)){
					foreach ($sets as $p => $objs)
						foreach ($objs as $obj)
							$this->Save($obj);
				}
			} catch (Excetion $e){
				$this->_ioc_driver_->RollbackTransaction();
				throw $e;
				return false;
			}
			$this->_clean_loaded_object($object, $orm);
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
			$assume_descendants = $this->_enrich_descendants($orm["classname"], $assume_descendants);
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
				if (isset($p)){
					$item->$parentprop = $p;
					$this->ProvideUniqueness($p);
				} else 
					$item->$parentprop = false;
				$p = $item;	
			}
			return $item;
		}
		return null;
	}
	
	
	private function _parse_property($ptr,array $name,$orm,array &$operands, array &$tables,array &$join,$greedy_fetch_alias = null){
		if (!isset($orm['properties'][$name[$ptr]])){
			if ($ptr > 0)
				throw new TNOrmException(TNOrmException::ERR_INVALID_PROPERTY,array('property'=>$name[$ptr]));
			return false;
		}
		
		$pfld = ($orm['properties'][$name[$ptr]]["type"] == "scalar")?$orm['properties'][$name[$ptr]]['field']:$orm['properties'][$name[$ptr]]['fields'][0];
		$mtable = ($ptr == 0)?$pfld['table']:($name[$ptr - 1].($ptr - 1));
		
		if (($ptr + 1) == count($name)){	
			switch ($orm['properties'][$name[$ptr]]['type']){
				case 'collection':{
					if (count($orm['properties'][$name[$ptr]]['mtm']) > 1){
						$colorm = $this->_get_orm_config($orm['properties'][$name[$ptr]]['mtm']['class']);
						$fp = $colorm['properties'][$orm['properties'][$name[$ptr]]['mtm']['property']];
					} else {
						$colorm = $this->_get_orm_config($orm['properties'][$name[$ptr]]['classname']);
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
					foreach ($orm['properties'][$name[$ptr]]['fields'] as $fld){
						$jp = $colorm['properties'][$fld['property']];
						if (count($jp['fields']) > 1)
							throw new TNOrmException(TNOrmException::ERR_COMPOSITE_KEY_FILTER);
						$jfld = $jp['fields'][0];
						$conditions[] = new TCondition(TConditionType::C_EQUAL, array(
								$this->_create_field($fld['name'], $tables[$mtable], ($tables[$mtable] instanceof TNestedSets)?1:0),
								$this->_create_field($jfld['name'], $tables[$jfld['table']], ($tables[$jfld['table']] instanceof TNestedSets)?1:0)
						));
					}
			
					$join[] = new TJoin($colds, TJoin::JOIN_INNER, $conditions);
						
					$field = &$fp['fields'][0];
					$p_fld = $this->_create_field($field['name'], $tables[$field['table']], ($tables[$field['table']] instanceof TNestedSets)?1:0);
				}break;
				case 'reference':{
					if (count($orm['properties'][$name[$ptr]]['fields']) > 1){
						$result = array();
						foreach ($orm['properties'][$name[$ptr]]['fields'] as &$field)
							$result[] = $this->_create_field($field['name'], $tables[$mtable], ($tables[$mtable] instanceof TNestedSets)?1:0,$greedy_fetch_alias.'_'.$field['name']);
						return $result;
					} else {
						$field = &$orm['properties'][$name[$ptr]]['fields'][0];
						$p_fld = $this->_create_field($field['name'], $tables[$mtable], ($tables[$mtable] instanceof TNestedSets)?1:0,$greedy_fetch_alias.'_'.$field['name']);
					}
				}break;
				case 'scalar':
				default:$p_fld = $this->_create_field($orm['properties'][$name[$ptr]]['field']['name'], $tables[$mtable], ($tables[$mtable] instanceof TNestedSets)?1:0, $greedy_fetch_alias .'_'.$orm['properties'][$name[$ptr]]['field']['name']);break;
			}	
			$operands[] = $p_fld;
			return $p_fld;		
		} 
			
		if ($orm['properties'][$name[$ptr]]['type'] != 'reference'){
			throw new TNOrmException(TNOrmException::ERR_NOT_REFERENCE,array('property'=>$name[$ptr]));
			return false;
		}
		
		if ($greedy_fetch_alias)
			$greedy_fetch_alias .= '_'.$orm['properties'][$name[$ptr]]['index'];
		
		$ref_orm = $this->_get_orm_config($orm['properties'][$name[$ptr]]['classname']);
		$np = &$ref_orm['properties'][$name[$ptr + 1]];
		if (is_null($np)){
			$tad = array();
			$tad = $this->_enrich_descendants($orm['properties'][$name[$ptr]]['classname'], $tad);
			foreach ($tad as $ad){
				$ref_orm = $this->_get_orm_config($ad);
				$np = &$ref_orm['properties'][$name[$ptr + 1]];
				if ($np) break;
			}
			if (is_null($np))
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		}
		
		if (($np["type"] == "reference") || ($np["type"] == "collection"))
			$rtn = $np["fields"][0]["table"];
		else
			$rtn = $np["field"]["table"];
		
		$alias = $name[$ptr].$ptr;
		
		if (!isset($tables[$alias])){			
			$tables[$alias] = new TTable($rtn,$alias);
			$jcond = array();
			$key_index = 0;
			foreach ($orm['properties'][$name[$ptr]]['fields'] as $rfld){
				$jcond[] = new TCondition(TConditionType::C_EQUAL, array(
							$this->_create_field($ref_orm['tables'][$rtn][$key_index], $tables[$alias],0),
							$this->_create_field($rfld['name'], $tables[$mtable],0)
				));
				$key_index++; 
			}
			$join[] = new TJoin($tables[$alias], TJoin::JOIN_LEFT, $jcond);
		}
		return $this->_parse_property($ptr + 1, $name, $ref_orm, $operands, $tables, $join, $greedy_fetch_alias);
	}
	
	private function _parse_filter_operand($operand, &$filter, $orm, &$tables, &$join, $ind = null, $rorm = null, array &$rjoin = array(), TOperation $context = null){
		if (is_string($operand)){
			$ptables = $tables;
			if ($rorm && ($ind > 0))
				$pjoin = $rjoin;
			else
				$pjoin = $join;
			
			if ($fld = $this->_parse_property(0, explode('.',$operand), ($rorm && ($ind > 0))?$rorm:$orm, $filter, $ptables, $pjoin)){
				if ($context){
					if (is_array($fld)){
						$obj = $context->Operands[$ind?0:1];						
						if (!($obj instanceof TNOrmObject))
							throw new TCoreException(TCoreException::ERR_BAD_VALUE);
						
						$kv = $this->_get_object_key_values($obj);
						$i = 0;
						foreach ($fld as $f){
							$ops = array($f,$kv[$i]);
							$filter[] = ($context instanceof TCondition)?new TCondition($context->Type, $ops):new TOperation($context->Type,$ops);
							$i++;
						}
						return false;
					}
				}
				$tables = $ptables;
				if ($rorm && ($ind > 0))
					$rjoin = $pjoin;
				else
					$join = $pjoin;
			} else
				$filter[] = $operand;
		}
		else if ($operand instanceof IIdentity)
			$filter[] = $operand->ItemId();
		else if ($operand instanceof TNOrmObject)
			$filter = array_merge($filter,$this->_get_object_key_values($operand));
		else if ($operand instanceof TOperation){
			$operands = array();	
			$i = 0;
			$default = true;		
			foreach ($operand->Operands as $o){
				$default = $this->_parse_filter_operand($o, $operands, $orm, $tables, $join, $i, $rorm, $rjoin, $operand);
				if (!$default) break;
				$i++;
			}
			if ($default)
				$filter[] = ($operand instanceof TCondition)?new TCondition($operand->Type, $operands):new TOperation($operand->Type,$operands);
			else {
				foreach ($operands as $o)
					$filter[] = $o;
			}
		} else
			$filter[] = $operand;
		return true;
	}
	
	private function _filter_from_conditions(array &$filter,$orm,&$tables,$conditions, &$join, $rorm = null, array &$rjoin = array()){		
		$i = 0;
		foreach ($conditions as $condition){
			$this->_parse_filter_operand($condition, $filter,$orm, $tables,$join,$i,$rorm,$rjoin);
			$i++;
		}
	}
	
	private function _form_sorting($orm,&$tables,$sorting,&$join){
		$sort = array();
		$flds = array();
		foreach ($sorting as $s){
			if ($s->Type == TSortingType::SORT_SHUFFLE)
				$sort[] = new TSorting(new TTableField(''),$s->Type);	
			else if (isset($orm['properties'][$s->Attribute])){
				switch ($orm['properties'][$s->Attribute]['type']){
					case 'collection':break;
					case 'reference':{
						foreach ($orm['properties'][$s->Attribute]['fields'] as $field){
							$sort[] = new TSorting($this->_create_field($field['name'], $tables[$field['table']], ($tables[$field['table']] instanceof TNestedSets)?1:0), $s->Type);
						}
					}break;
					default:$sort[] = new TSorting($this->_create_field($orm['properties'][$s->Attribute]['field']['name'], $tables[$orm['properties'][$s->Attribute]['field']['table']], ($tables[$orm['properties'][$s->Attribute]['field']['table']] instanceof TNestedSets)?1:0), $s->Type);break;	
				}
			} else if ($gsf = $this->_form_greedy_property($s->Attribute, $orm, $flds, $tables, $join))
					$sort[] = new TSorting($gsf, $s->Type);
		}
		return $sort;
	}
	
	private function _form_greedy_properties(array $greedy, array $orm, array &$fields, array &$tables, array &$join){
		$greedy = array_unique(array_merge($orm["greedy"],$greedy));
		foreach ($greedy as $prop)
			$this->_form_greedy_property($prop,$orm,$fields,$tables, $join);		
	}
	
	private function _form_greedy_property($name,$orm, &$fields, &$tables, &$join){
		$path =  explode('.',$name);
		$porm = $orm;
		$result = false;
		$n = count($path);
		for ($i = 0; $i < $n; $i++){
			if (isset($porm['properties'][$path[$i]]) && $porm['properties'][$path[$i]]['type'] == 'reference'){
				$norm = $this->_get_orm_config($porm['properties'][$path[$i]]['classname']);
				$nm = array_slice($path, 0, $i + 1);
				foreach ($norm['properties'] as $pn=>$p)
					if ($p['type'] != 'collection'){
						$r = $this->_parse_property(0, array_merge($nm,array($pn)), $porm, $fields, $tables, $join,'gp');
						if (($i == $n - 2) && ($pn == $path[$i + 1]))
							$result = $r;
					}
				$porm = $norm;
			} else break;
		}
		return $result;
	}
	
	protected function getTotalsDatasource($orm,$ds){
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
		return new TDataSet(new TDataSet($cds,'cnt_base'),null,array(new TExpressionField('cnt', new TOperation(TOperationType::O_COUNT, array()))));
	}
	
	protected function parseFetchOptions($dummy, array $orm,array $greedy = array(),array $conditions = array(), array &$fields, array &$tables, array &$assume_classes, array &$assume_descendants = array()){
		$join = array();
			
		$assume_descendants = $this->_enrich_descendants($orm["classname"],$assume_descendants);
		$assume_classes = $assume_descendants;
			
		$break = !empty($assume_classes);
		$ds = $this->_form_ds($orm, $join, $tables, $fields, $assume_descendants);

		$this->_form_greedy_properties($greedy, $orm, $fields, $tables, $join);
			
		$filter = array();
			
		if (is_object($dummy))
			$filter = $this->_filter_from_object($orm, $dummy, $tables);
		
		$this->_filter_from_conditions($filter, $orm, $tables, $conditions, $join);
			
		$ds->Filter = $filter;
		$ds->Join = $join;
		return $ds;		
	}
	
	
	private function _get_data_source($dummy, &$tables, &$fields, &$orm, &$assume_classes, array $conditions = array(), array $greedy = array(), array &$assume_descendants = array()){
		$fields = array();
		$tables = array();
		$assume_classes = array();
		$ds = false;
		if ($dummy instanceof TOrmGlueSpec){
			$orm = $this->_get_orm_config($dummy->LeftSide);
			$orm2 = $this->_get_orm_config($dummy->RightSide);
			if ($orm && $orm2){
				$f2 = array();
				$ac2 = array();
				$ds = $this->parseFetchOptions($dummy->LeftSide,$orm,$greedy,$conditions,$fields,$tables,$assume_classes,$assume_descendants);
				$jds = $this->parseFetchOptions($dummy->RightSide,$orm2,array(),array(),$f2,$tables,$ac2);
				$jcnd = array();
				$this->_filter_from_conditions($jcnd, $orm, $tables, $dummy->Conditions, $ds->Joins(), $orm2,$jds->Joins());
				$ds->Join(new TJoin($jds, TJoin::JOIN_INNER, $jcnd));
			}
		} else {
			if ($orm = $this->_get_orm_config($dummy))
				$ds = $this->parseFetchOptions($dummy,$orm,$greedy,$conditions,$fields,$tables,$assume_classes,$assume_descendants);
		}
		return $ds;		
	}
	
	
/**
 * Counts the number of specified objects
 * @param object|ReflectionClass|string|TOrmGlueSpec $dummy
 * @param TOrmCondition[] $conditions
 * @return int
 */
	public function Count($dummy,array $conditions = array()){
		$fields = false;
		$tables = false;
		$orm = false;
		
		if ($ds = $this->_get_data_source($dummy, $tables, $fields, $orm, $assume_classes, $conditions)){
			$ds = $this->getTotalsDatasource($orm, $ds);
			$cntr = $this->_ioc_driver_->FetchRecords($ds);
			foreach ($cntr as $cnt) return $cnt['cnt'];
		}
		return false;		
	}	
	
	
/**
 * 
 * Fetches a collection of objects from storage
 * @param object|ReflectionClass|string|TOrmGlueSpec $dummy
 * @param TOrmCondition[] $conditions
 * @param TOrmSorting[] $sorting
 * @param int $offset
 * @param int $count
 */	
	public function Fetch($dummy,array $greedy = array(),array $conditions = array(),array $sorting = array(), array $assume_descendants = array(), $offset = null, $count = null, &$total = null){		
		$fields = false;
		$tables = false;
		$orm = false;
		
		if ($ds = $this->_get_data_source($dummy, $tables, $fields, $orm, $assume_classes, $conditions, $greedy, $assume_descendants)){
			if (!is_null($total)){
				$cds = $this->getTotalsDatasource($orm, $ds);
				$cntr = $this->_ioc_driver_->FetchRecords($cds);
				foreach ($cntr as $cnt){
					$total = $cnt['cnt'];
					break;
				}
			}				
			$ds->Fields = $fields;
			
			$sjoin = array();
			$ds->Sorting = $this->_form_sorting($orm, $tables, $sorting, $sjoin);
			if (!empty($sjoin))
				$ds->Join = $sjoin;
			$ds->Offset = $offset;
			$ds->Count = $count;
			return new TNOrmIterator($this->_ioc_driver_->FetchRecords($ds),$orm['classname'],$this,$assume_descendants,$assume_classes);
		}
		return false;
	}

	public function ProvideUniqueness($object){
		if ($this->_bool_unique_objects_ || isset($this->_array_class_uniqueness_[get_class($object)])){
			if ($orm = $this->_get_orm_config($object)){
				$id = join('|',$this->_get_object_key_values($object));
				if (!isset($this->_loaded_objects_[$orm['classname']]))
					$this->_loaded_objects_[$orm['classname']] = array();
				if (!isset($this->_loaded_objects_[$orm['classname']][$id]))
					$this->_loaded_objects_[$orm['classname']][$id] = $object;
			}
		}
	}
	
	public function ForceReload($object){
		if ($orm = $this->_get_orm_config($object))
			$this->_clean_loaded_object($object, $orm);
	}
	
	private function _loaded_object($dummy, &$orm){
		if (isset($this->_loaded_objects_[$orm['classname']])){
			$id = $this->_get_object_key_values($dummy);
			if (!empty($id)){
				$id = join('|',$id);
				if (isset($this->_loaded_objects_[$orm['classname']][$id]))
					return $this->_loaded_objects_[$orm['classname']][$id];
			}
		}
		return null;
	}
	
	private function _clean_loaded_object($dummy, &$orm){
		if (isset($this->_loaded_objects_[$orm['classname']])){
			$id = $this->_get_object_key_values($dummy);
			if (!empty($id)){
				$id = join('|',$id);
				if (isset($this->_loaded_objects_[$orm['classname']][$id]))
					$this->_loaded_objects_[$orm['classname']][$id] = null;
			}
		}
	}
	
	private function _check_loaded($dummy,&$orm){
		if ($this->_bool_unique_objects_ || isset($this->_array_class_uniqueness_[$orm["classname"]])){
			if (!isset($this->_loaded_objects_[$orm['classname']]))
				return $dummy;
			$id = $this->_get_object_key_values($dummy);
			if (empty($id))
				return $dummy;	
			$id = join('|',$id);	
			if (isset($this->_loaded_objects_[$orm['classname']][$id]))
				$dummy = $this->_loaded_objects_[$orm['classname']][$id];
			/*
			else	
				$this->_loaded_objects_[$orm['classname']][$id] = $dummy;
				*/
		}
		return $dummy;
	}
	
	private function _assign_property($dummy,$property,$meta,$row, array $assume_descendants = array()){
		switch ($meta['type']){
			case 'scalar':{
				$nm = $meta['field']['name'];
				$v = $row->$nm;
				switch ($meta['datatype']){
					case 'bool':$v = is_null($v)?null:TConvertions::ConvertToBoolean($v);break;
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
					case 'TFile':{
						if (!is_null($v))
							$v = new TFile($this->Application(),basename($v),$v);
					}break;
					case 'TImage':{
						if (!is_null($v))
							$v = new TImage($this->Application(),basename($v),$v);
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
				if ($assign){
					$dummy->$property = new TNOrmObjectLoader($row,$meta,$this,$assume_descendants);
				}
			}break;
			case 'collection':{
				if (!isset($dummy->$property)) 
					$dummy->$property = new TNOrmCollectionLoader($row,$meta,$this,$assume_descendants);	
			}break;
		}
	}
	
	public function ParentProperty($object){
		if ($orm = $this->_get_orm_config($object))
			if (isset($orm['nestedsets']))
				return $orm['nestedsets']['parent'];
		return false;
	}
	
	public function ChildrenProperty($object){
		if ($orm = $this->_get_orm_config($object))
			if (isset($orm['nestedsets']))
				return $orm['nestedsets']['children'];
		return false;
	}
	
	
	protected function toPropertyMeta($nm,$prop){
		if ($prop['type'] == 'scalar'){
			$type = TItemPropertyType::PT_STRING;
			switch ($prop['datatype']){
				case 'string':break;
				case 'int':$type = TItemPropertyType::PT_INT;break;
				case 'float':$type = TItemPropertyType::PT_DEC;break;
				case 'date':
				case 'TDate':$type = TItemPropertyType::PT_DATE;break;
				case 'TFile':$type = TItemPropertyType::PT_FILE;break;
				case 'TImage':$type = TItemPropertyType::PT_IMAGE;break;
				case 'bool':$type = TItemPropertyType::PT_BOOL;break;
			}
			return new TOrmPropertyMeta($nm, $nm, $type);
		} else {
			switch ($prop['type']){
				case 'collection':{
					$fp = array();
					foreach ($prop['fields'] as $f)
						$fp[] = $f['property'];
					return new TOrmCollectionPropertyMeta($nm, $nm, $prop['classname'],false,(count($fp) == 1)?$fp[0]:$fp);break;
				}break;
				case 'reference':return new TOrmReferencePropertyMeta($nm, $nm, $prop['classname']);break;
			}
		}
		return null;
	}
	
	public function Keys($object){
		if ($orm = $this->_get_orm_config($object)){
			$result = array();
			foreach ($orm["keys"] as $key)
				$result[$key] = $this->toPropertyMeta($key, $orm["properties"][$key]);
			return $result;
		}
		return null;
	}	
	
	/**
	 * @param object|ReflectionClass|string $object
	 * @return string[]
	 */
	public function Tables($object){
		if ($orm = $this->_get_orm_config($object))
			return array_keys($orm["tables"]);
		return null;
	}
	
	public function Properties($dummy){
		if ($orm = $this->_get_orm_config($dummy)){
			$result = array();
			foreach ($orm['properties'] as $key=>$prop)
				$result[$key] = $this->toPropertyMeta($key, $prop);
			
			if (!empty($orm['nestedsets']))
				$result[$orm['nestedsets']['parent']] = new TOrmReferencePropertyMeta($orm['nestedsets']['parent'], $orm['nestedsets']['parent'], $orm['classname']);
			
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
		$old = $object->$p;
		$object->$p = $container;
		$this->Save($object);
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
			
			$assume_descendants = $this->_enrich_descendants($orm["classname"],$assume_descendants);
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
				$sjoin = array();
				$ds->Sorting = $this->_form_sorting($orm, $tables, $sorting, $sjoin);
				if (!empty($sjoin))
					$ds->Join = $sjoin;				
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
	public function Create($row, $dummy, array $assumed_descendants = array(), array $assumed_classes = array()){
		if ($orm = $this->_get_orm_config($dummy)){
			if (is_array($row)){
				$greedy = array();	
				foreach ($row as $key=>$value)
					if (preg_match('/gp(_\d+)+_(\w+)/',$key))
						$greedy[$key] = $value;
				
				if (!empty($greedy)){
					$row = array_diff_key($row, $greedy);
					$temp = array();
					foreach ($greedy as $key=>$value){
						$parts = array();
						preg_match('/^((?:\d+_)+)([^\d].*)$/',preg_replace('/^gp_/','',$key),$parts);
						$t = explode('_',$parts[1]);
						array_pop($t);
						array_push($t, $parts[2]);
						$parts = $t;
						$nm = $orm['prop_by_index'][$parts[0]];
						if (!isset($temp[$nm]))
							$temp[$nm] = array();
						if (count($parts) == 2)
							$temp[$nm][$parts[1]] = $value;
						else 
							$temp[$nm]['gp_'.join('_',array_slice($parts, 1))] = $value;
					}
					
					$greedy = array();
					foreach ($temp as $nm=>$r)
						$greedy[$nm] = $this->Create($r, $orm['properties'][$nm]['classname']);
				}
				$row = (object)$row;
			}
		
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
			
			$dummy = clone $this->_check_loaded($dummy, $orm);	

			if (!empty($orm['nestedsets'])){
				$parentprop = $orm['nestedsets']['parent'];
				$dummy->$parentprop = new TNOrmBranchLoader($dummy,$this,$assumed_classes);
			}		
			
			foreach ($orm['properties'] as $name=>$prop){
				if (!in_array($name,$orm['keys']))
					$this->_assign_property($dummy, $name, $prop, $row, $assumed_classes);
			}

			if (!empty($greedy))
				foreach ($greedy as $nm=>$obj)
					$dummy->$nm = $obj;
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
			case 'XmlConfigPath':$this->_xml_config_path_ = $this->Application()->AdjustPath($value);break;
			case 'ScriptConfigPath':$this->_script_config_path_ = $this->Application()->AdjustPath($value);break;
			case 'DateFormat':$this->_date_format_ = $value;break;
			case 'LazyLoading':$this->_lazy_loading_ = TConvertions::ConvertToBoolean($value);break;
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
				$this->_script_config_path_ = $this->Application()->PrivateDir().'/orm';
				
			TFileSystem::ForceDir($this->_script_config_path_);	
				
			if ($this->_use_config_ & (self::CONF_XML | self::CONF_PHPDOC)){
				$filename = $this->_script_config_path_.'/'.$cn.'.orm.php';
				if (!$rc)
					$rc = new ReflectionClass($cn);

				$src_time = 0;	
				
				if (!$this->_xml_config_path_)
					$this->_xml_config_path_ = $this->Application()->PrivateDir().'/orm';
				
					
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
				else
					throw new TCoreException(TCoreException::ERR_BAD_TYPE);
				
				$ormconfig['prop_by_index'] = array();
				foreach ($ormconfig['properties'] as $nm=>$p)
					$ormconfig['prop_by_index'][$p['index']] = $nm;
				$this->_loaded_orms_[$cn] = $ormconfig;
			}
		} 
		return $this->_loaded_orms_[$cn];	
	}
}