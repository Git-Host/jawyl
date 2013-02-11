<?php
require_once 'connectivity/MySQL/TMySqlDBDriver.inc';
require_once 'common/metastorage.inc';

/**
 * 
 * @author dakrasilnikov
 * @property string $MainTable
 * @property string $RestFrom
 * @property string $From
 * @property string $Where
 * @property string $Order
 * @property string $Limit 
 */

class TMySqlMetaDataSetProcessor extends TMySqlDataSetProcessor {
	private function _subclass_alias(TMetaClassDataSource $ds, IClass $class){
		return $ds->Name."_e".$class->Id();
	}
	
	protected function getFieldPrefix(IDataSourceField $fld){
		if ($fld instanceof TMetaClassDataSourceAttribute)
			return $this->_subclass_alias($fld->DataSource, $fld->Attribute->Class);
		else if ($fld instanceof TMetaSysField){
			switch ($fld->SysField){
				case TSysField::CLASS_ID:return $fld->DataSource->Name."_e";break;
				case TSysField::CLASS_NAME:return $fld->DataSource->Name."_c";break;
				case TSysField::INSTANCE_ID:return $this->_subclass_alias($fld->DataSource, $fld->DataSource->Class);break;
			}
		}	
		return parent::getFieldPrefix($fld);
	}
	
	protected function getFieldName(IDataSourceField $fld){
		if ($fld instanceof TMetaSysField){
			switch ($fld->SysField){
				case TMetaSysField::CLASS_ID:return TMySqlMetaDriver::CLASS_PK_FIELD;break;
				case TMetaSysField::CLASS_NAME:return TMySqlMetaDriver::CLASS_NAME_FIELD;break;
				case TMetaSysField::INSTANCE_ID:return TMySqlMetaDriver::ENTITY_PK_FIELD;break;
			}
		}
		return parent::getFieldName($fld);
	}	
	
	protected function dataSourceTargetName(TDataSource $ds){
		if ($ds instanceof TMetaClassDataSource)
			return $ds->Name."_e";
		return parent::dataSourceTargetName($ds);
	}
	
	private function _anc_srcs(TDataSource $ds, IClass $class, array &$result){
		$anc = $class->Ancestor();
		if ($anc instanceof IClass){
			$result[] = $this->_subclass_alias($ds,$anc).".*";
			$this->_anc_srcs($ds, $anc, $result);  
		}
	}
	
	private function _dsc_srcs(TDataSource $ds, IClass $class, array &$result){
		$dsc = $class->Descendants();
		foreach ($dsc as $d){
			$result[] = $this->_subclass_alias($ds,$d).".*";
			$this->_dsc_srcs($ds, $d, $result);
		}
	}
	
	protected function parseAllField(IDataSourceField $fld){
		if ($fld->DataSource instanceof TMetaClassDataSource){
			$result = array($this->_subclass_alias($fld->DataSource,$fld->DataSource->Class).".*",$fld->DataSource->Name."_c.*");
			$this->_anc_srcs($fld->DataSource,$fld->DataSource->Class,$result);
			$this->_dsc_srcs($fld->DataSource,$fld->DataSource->Class,$result);
			return join(",",$result);
		}
		return parent::parseAllField($fld); 
	} 
	
	private function _inner_joins(TDataSource $ds, IClass $class, array $classfilter){
		$anc = $class->Ancestor();
		if ($anc instanceof IClass)
			if ($ds->FullFetch || key_exists($anc->Id(),$classfilter)){
				$alias = $this->_subclass_alias($ds, $class);
				$palias = $this->_subclass_alias($ds, $anc);
				return " inner join [".TMySqlMetaDriver::ENTITIES_TABLE."_".$anc->Id()."] as ".$palias." on ".$palias.".".TMySqlMetaDriver::ENTITY_PK_FIELD." = ".$alias.".".TMySqlMetaDriver::ENTITY_PK_FIELD.$this->_inner_joins($ds, $anc, $classfilter);
			}
		return "";
	}
	
	private function _left_joins(TDataSource $ds, IClass $class, array $classfilter){
		$descs = $class->Descendants();
		if (!empty($descs)){
			$result = "";
			foreach ($descs as $d)
				if (($ds->FullFetch) || (key_exists($d->Id(),$classfilter))) {
				$alias = $this->_subclass_alias($ds, $class);
				$dalias = $this->_subclass_alias($ds, $d);
				$result .= " left join [".TMySqlMetaDriver::ENTITIES_TABLE."_".$anc->Id()."] as ".$dalias." on ".$dalias.".".TMySqlMetaDriver::ENTITY_PK_FIELD." = ".$alias.".".TMySqlMetaDriver::ENTITY_PK_FIELD.$this->_left_joins($ds, $d, $classfilter);
				}
			return $result;	
		}
		return "";
	}
	
	private function _class_filter(TDataSource $ds){
		$result = array();
		if (!$ds->FullFetch)
			foreach ($ds->UsedFields as $fld)
				if (!key_exists($fld->Attribute->Class->Id(),$result))
					$result[$fld->Attribute->Class->Id()] = 1;
		return $result;
	}
	
	protected function parseDataSource(TDataSource $ds, array $joinconditions){
		if ($ds instanceof TMetaClassDataSourceAttribute){
			$on = ((!empty($joinconditions))?(" on ".$this->parseOperations($joinconditions)):"");
			$alias = $this->_subclass_alias($ds, $ds->Class); 
			$result = "[".TMySqlMetaDriver::ENTITIES_TABLE."_".$ds->Class->Id()."] as ".$alias.$on;

			$malias = $ds->Name."_e";
			$calias = $ds->Name."_c";
			
			$classfilter = $this->_class_filter();
			
			$result .= $this->_inner_joins($ds, $ds->Class, $classfilter);
			$result .= " inner join [".TMySqlMetaDriver::ENTITIES_TABLE."] ".$malias." on ".$malias.".".TMySqlMetaDriver::ENTITY_PK_FIELD." = ".$alias.".".TMySqlMetaDriver::ENTITY_PK_FIELD;
			$result .= " inner join [".TMySqlMetaDriver::CLASSES_TABLE."] ".$calias." on ".$calias.".".TMySqlMetaDriver::CLASS_PK_FIELD." = ".$malias.".".TMySqlMetaDriver::CLASS_PK_FIELD;
			$result .= $this->_left_joins($ds, $ds->Class, $classfilter);
			return $result;			
		}
		return parent::parseDataSource($ds, $joinconditions);
	}
}

class TMySqlMetaDataSetUpdateProcessor extends TMySqlDataSetUpdateProcessor {
	protected function createInternalProcessor(TDataSource $dataset, TMySqlDBDriver $driver){
		return new TMySqlMetaDataSetProcessor($dataset, $driver);
	}	
} 

class TMySqlMetaDriver extends TMySqlDBDriver implements IMetaDriver {	
	private $_check_db_ = false;
	private $_class_pool_start_ = 1;
	private $_class_pool_size_ = null;	
	
	const CLASSES_TABLE = "db_classes";
	const CLASSES_STATICS = "db_statics";
	const ATTRDEFINITIONS_TABLE = "db_class_attributes";
	const ENTITIES_TABLE = "db_entities";
	const SNAPSHOTS_PREFIX = "_snapshots";
		
	const CLASS_PK_FIELD = "cid";
	const CLASS_NAME_FIELD = "classname";
	const CLASS_LKEY_FIELD = "lkey";
	const CLASS_RKEY_FIELD = "rkey";
	const CLASS_ANCESTOR_FIELD = "ancestor";

	const CLASS_ATTR_PK_FIELD = "caid";
	const CLASS_ATTR_NAME_FIELD = "attributename";
	const CLASS_ATTR_FIELD_FIELD = "fieldname";
	const CLASS_ATTR_TYPE_FIELD = "attrtype";
	const CLASS_ATTR_OPTIONS_FIELD = "options";
	const CLASS_ATTR_REFCLASS_FIELD = "refclass";
	const CLASS_ATTR_STATIC_FIELD = "isstatic";
			
	const ENTITY_PK_FIELD = "entityid";
	
	private $_classes_loaded_ = false;
		
	public function __set($nm,$value){
		switch ($nm){
			case "CheckDB":$this->_check_db_ = TConvertions::ConvertToBoolean($value);break;
			case "ClassPoolStart":{
				if (is_numeric($value)){
					if ($value > 0)
						$this->_class_pool_start_ = $value;
				} else $this->_class_pool_start_ = 1;
			}break;
			case "ClassPoolSize":{
				if (is_numeric($value)){
					if ($value > 0)
						$this->_class_pool_size_ = $value;
				} else $this->_class_pool_size_ = null;
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
		
	public function __get($nm){
		switch ($nm){
			case "CheckDB":return $this->_check_db_;break;
			case "ClassPoolStart":return $this->_class_pool_start_;break;
			case "ClassPoolSize":return $this->_class_pool_size_;break;
			default:return parent::__get($nm);break;
		}
	}
							
	private function _check_database_structure(){
		$classes = false;
		$statics = false;
		$pstatics = false;
		$classattributes = false;
		$entities = false;
		$tables = $this->query("show tables");
		
		$checkprot = "";
			
		while ($table = $tables->FetchIndexed())
			switch ($table[0]){
				case $this->RealTableName(self::CLASSES_STATICS).self::SNAPSHOTS_PREFIX:{
					$cid = false;
					$ds = false;
					$dt = false;
					$columns = $this->query("show columns from ".$this->RealTableName(self::CLASSES_STATICS).self::SNAPSHOTS_PREFIX);
					while ($column = $columns->FetchAssoc())
						switch ($column["Field"]){
							case self::CLASS_PK_FIELD:$cid = ($column["Type"] == "bigint(20)");break;
							case "date_since":$ds = ($column["Type"] == "datetime");break;
							case "date_till":$dt = ($column["Type"] == "datetime");break;
						}
					$pstatics = $cid && $ds && $dt;
					if (!$pstatics) $this->query("drop table ".$this->RealTableName(self::CLASSES_STATICS).self::SNAPSHOTS_PREFIX);
				}break;
				case $this->RealTableName(self::CLASSES_STATICS):{
					$cid = false;
					$columns = $this->query("show columns from ".$this->RealTableName(self::CLASSES_STATICS));
					while ($column = $columns->FetchAssoc())
						switch ($column["Field"]){
							case self::CLASS_PK_FIELD:$cid = ($column["Type"] == "bigint(20)") && ($column["Key"] == "PRI");break;
						}
					$statics = $cid;
					if (!$statics) $this->query("drop table ".$this->RealTableName(self::CLASSES_STATICS));
				}break;
				case $this->RealTableName(self::CLASSES_TABLE):{
					$cid = false;
					$name = false;
					//$alias = false;
					$rkey = false;
					$lkey = false;
					$ancestor = false;						
					$columns = $this->query("show columns from ".$this->RealTableName(self::CLASSES_TABLE));
					while ($column = $columns->FetchAssoc())
						switch ($column["Field"]){
							case self::CLASS_PK_FIELD:$cid = ($column["Type"] == "bigint(20)") && ($column["Key"] == "PRI") && ($column["Extra"] == "auto_increment");break;
							case self::CLASS_NAME_FIELD:$name = ($column["Type"] == "varchar(200)") && ($column["Key"] == "UNI") && ($column["Null"] == "NO");break;
						//	case self::CLASS_ALIAS_FIELD:$alias = ($column["Type"] == "varchar(200)") && ($column["Key"] == "UNI") && ($column["Null"] == "NO");break;
							case self::CLASS_LKEY_FIELD:$lkey = ($column["Type"] == "bigint(20)") && ($column["Key"] == "MUL") && ($column["Null"] == "NO");break;
							case self::CLASS_RKEY_FIELD:$rkey = ($column["Type"] == "bigint(20)") && ($column["Key"] == "MUL") && ($column["Null"] == "NO");break;
							case self::CLASS_ANCESTOR_FIELD:$ancestor = ($column["Type"] == "bigint(20)") && ($column["Key"] == "MUL");break;
						}
					$classes = $cid && $name /*&& $alias */&& $rkey && $lkey && $ancestor;
					if (!$classes) $this->query("drop table ".$this->RealTableName(self::CLASSES_TABLE));
				}break;
				case $this->RealTableName(self::ATTRDEFINITIONS_TABLE):{
					$caid = false;
					$cid = false;
					$name = false;
					$field = false;
					$type = false;
					$options = false;
					$refclass = false;
					$isstat = false;
					$columns = $this->query("show columns from ".$this->RealTableName(self::ATTRDEFINITIONS_TABLE));
					while ($column = $columns->FetchAssoc())
						switch ($column["Field"]){
							case self::CLASS_ATTR_PK_FIELD:$caid = ($column["Type"] == "bigint(20)") && ($column["Key"] == "PRI") && ($column["Extra"] == "auto_increment");break;
							case self::CLASS_PK_FIELD:$cid = ($column["Type"] == "bigint(20)") && ($column["Key"] == "MUL") && ($column["Null"] == "NO");break;
							case self::CLASS_ATTR_NAME_FIELD:$name = ($column["Type"] == "varchar(200)") && ($column["Null"] == "NO");break;
							case self::CLASS_ATTR_FIELD_FIELD:$field = ($column["Type"] == "varchar(200)") && ($column["Null"] == "YES");break;
							case self::CLASS_ATTR_TYPE_FIELD:$type = ($column["Type"] == "char(6)") && ($column["Null"] == "NO");break;
							case self::CLASS_ATTR_OPTIONS_FIELD:$options = ($column["Type"] == "text") && ($column["Null"] == "YES");break;
							case self::CLASS_ATTR_REFCLASS_FIELD:$refclass = ($column["Type"] == "bigint(20)") && ($column["Key"] == "MUL");break;
							case self::CLASS_ATTR_STATIC_FIELD:$isstat = ($column["Type"] == "bit(1)");break;
						}
						
					$classattributes = $caid && $cid && $name && $field && $type && $options && $refclass && $isstat;
					if (!$classattributes) $this->query("drop table ".$this->RealTableName(self::ATTRDEFINITIONS_TABLE));
				}break;
				case $this->RealTableName(self::ENTITIES_TABLE):{
					$check = array();
					$check["eid"] = false;
					$check["cid"] = false;
					$columns = $this->query("show columns from ".$this->RealTableName(self::ENTITIES_TABLE));
					while ($column = $columns->FetchAssoc())
						switch ($column["Field"]){
							case self::ENTITY_PK_FIELD:$check["eid"] = ($column["Type"] == "bigint(20)") && ($column["Key"] == "PRI") && ($column["Extra"] == "auto_increment");break;
							case self::CLASS_PK_FIELD:$check["cid"] = ($column["Type"] == "bigint(20)") && ($column["Key"] == "MUL") && ($column["Null"] == "NO");break;
						}
					$entities = $check["eid"] && $check["cid"];
					if (!$entities) $this->query("drop table ".$this->RealTableName(self::ENTITIES_TABLE));
				}break;
			}
			
			//`".self::CLASS_ALIAS_FIELD."` varchar(200) NOT NULL,
			//UNIQUE KEY `".self::CLASS_ALIAS_FIELD."` (`".self::CLASS_ALIAS_FIELD."`),
			
			if (!$classes)
				$this->query("CREATE TABLE `".$this->RealTableName(self::CLASSES_TABLE)."` (
						  `".self::CLASS_PK_FIELD."` bigint(20) NOT NULL auto_increment,
						  `".self::CLASS_NAME_FIELD."` varchar(200) NOT NULL,
						  `".self::CLASS_LKEY_FIELD."` bigint(20) NOT NULL,
						  `".self::CLASS_RKEY_FIELD."` bigint(20) NOT NULL,
						  `".self::CLASS_ANCESTOR_FIELD."` bigint(20) NULL,
						  PRIMARY KEY  (`".self::CLASS_PK_FIELD."`),
						  UNIQUE KEY `".self::CLASS_PK_FIELD."` (`".self::CLASS_PK_FIELD."`),
						  UNIQUE KEY `".self::CLASS_NAME_FIELD."` (`".self::CLASS_NAME_FIELD."`),
						  KEY `".self::CLASS_LKEY_FIELD."` (`".self::CLASS_LKEY_FIELD."`),
						  KEY `".self::CLASS_RKEY_FIELD."` (`".self::CLASS_RKEY_FIELD."`),
						  KEY `".self::CLASS_ANCESTOR_FIELD."` (`".self::CLASS_ANCESTOR_FIELD."`)".
						  ((!is_null($this->ClassPoolSize))?", CHECK(".self::CLASS_PK_FIELD." < ".($this->ClassPoolStart+$this->ClassPoolSize).")":"")."
						) ENGINE=InnoDB AUTO_INCREMENT=".$this->ClassPoolStart." DEFAULT CHARSET=utf8");
			if (!$statics)				
				$this->query("CREATE TABLE `".$this->RealTableName(self::CLASSES_STATICS)."` (
							`".self::CLASS_PK_FIELD."` bigint(20) NOT NULL,
							PRIMARY KEY  (`".self::CLASS_PK_FIELD."`),
							UNIQUE KEY `".self::CLASS_PK_FIELD."` (`".self::CLASS_PK_FIELD."`),
							CONSTRAINT `".$this->staticsTable()."_fk` FOREIGN KEY (`".self::CLASS_PK_FIELD."`) REFERENCES `".$this->classesTable()."` (`".self::CLASS_PK_FIELD."`) ON DELETE CASCADE
							) ENGINE=InnoDB DEFAULT CHARSET=utf8");
			if (!$pstatics)
				$this->query("CREATE TABLE `".$this->RealTableName(self::CLASSES_STATICS).self::SNAPSHOTS_PREFIX."` (
							`".self::CLASS_PK_FIELD."` bigint(20) NOT NULL, 
							`date_since` datetime NOT NULL, 
							`date_till` datetime, 
							constraint `".$this->staticsTable()."_ss_fk` FOREIGN KEY (".self::CLASS_PK_FIELD.") references ".$this->classesTable()." (".self::CLASS_PK_FIELD.") on delete cascade,
							primary key (`".self::CLASS_PK_FIELD."`,`date_since`)
							) ENGINE=InnoDB CHARSET=utf8");				
			if (!$classattributes)
				$this->query("CREATE TABLE `".$this->RealTableName(self::ATTRDEFINITIONS_TABLE)."` (
							  `".self::CLASS_ATTR_PK_FIELD."` bigint(20) NOT NULL auto_increment,
							  `".self::CLASS_PK_FIELD."` bigint(20) NOT NULL,
							  `".self::CLASS_ATTR_NAME_FIELD."` varchar(200) NOT NULL,
							  `".self::CLASS_ATTR_FIELD_FIELD."` varchar(200) NULL,
							  `".self::CLASS_ATTR_TYPE_FIELD."` char(6) NOT NULL,
							  `".self::CLASS_ATTR_OPTIONS_FIELD."` text NULL,
							  `".self::CLASS_ATTR_REFCLASS_FIELD."` bigint(20) NULL,
							  `".self::CLASS_ATTR_STATIC_FIELD."` bit NOT NULL DEFAULT 0,
							  PRIMARY KEY  (`".self::CLASS_ATTR_PK_FIELD."`),
							  UNIQUE KEY `".self::CLASS_ATTR_PK_FIELD."` (`".self::CLASS_ATTR_PK_FIELD."`),
							  UNIQUE KEY `i".self::CLASS_ATTR_NAME_FIELD."` (`".self::CLASS_PK_FIELD."`,`".self::CLASS_ATTR_NAME_FIELD."`),
							  UNIQUE KEY `i".self::CLASS_ATTR_FIELD_FIELD."` (`".self::CLASS_PK_FIELD."`,`".self::CLASS_ATTR_FIELD_FIELD."`),
							  KEY `".self::CLASS_PK_FIELD."` (`".self::CLASS_PK_FIELD."`),
							  KEY `".self::CLASS_ATTR_REFCLASS_FIELD."` (`".self::CLASS_ATTR_REFCLASS_FIELD."`),
							  CONSTRAINT `".$this->attributesTable()."_fk` FOREIGN KEY (`".self::CLASS_PK_FIELD."`) REFERENCES `".$this->classesTable()."` (`".self::CLASS_PK_FIELD."`) ON DELETE CASCADE,
							  CONSTRAINT `".$this->attributesTable()."_fk1` FOREIGN KEY (`".self::CLASS_ATTR_REFCLASS_FIELD."`) REFERENCES `".$this->classesTable()."` (`".self::CLASS_PK_FIELD."`) ON DELETE RESTRICT
							) ENGINE=InnoDB DEFAULT CHARSET=utf8");
			if (!$entities)
				$this->query("CREATE TABLE `".$this->RealTableName(self::ENTITIES_TABLE)."` (
							  `".self::ENTITY_PK_FIELD."` bigint(20) NOT NULL auto_increment,
							  `".self::CLASS_PK_FIELD."` bigint(20) NOT NULL,
							  PRIMARY KEY (`".self::ENTITY_PK_FIELD."`),
							  UNIQUE KEY `".self::ENTITY_PK_FIELD."` (`".self::ENTITY_PK_FIELD."`),
							  KEY `".self::CLASS_PK_FIELD."` (`".self::CLASS_PK_FIELD."`),
							  CONSTRAINT `".$this->entitiesTable()."_class_fk` FOREIGN KEY (`".self::CLASS_PK_FIELD."`) REFERENCES `".$this->classesTable()."` (`".self::CLASS_PK_FIELD."`) ON DELETE CASCADE
							) ENGINE=InnoDB DEFAULT CHARSET=utf8");
	}
			
	public function ConnectionParameters(){
		return array_merge(parent::ConnectionParameters(),array("CheckDB" => TItemPropertyType::PT_BOOL,"ClassPoolStart"=>TItemPropertyType::PT_INT,"ClassPoolSize"=>TItemPropertyType::PT_INT)); 	
	}
		 
	public function Connect(){
		$this->CHECK_REFLECTION = $this->CheckDB;
		$result = parent::Connect();
		if ($result && $this->CheckDB)
			$this->_check_database_structure();
		return $result;
	}

	private function _parse_meta(array $meta){
		$attrs = array_keys($meta);
		if (count(array_diff(array("name","attributes"),$attrs)) == 0){
			$anc = false;
			$attributes = array();
			if (is_array($meta["attributes"]))
				foreach ($meta["attributes"] as $a){
					if (is_array($a))
						$a = $this->_parse_meta($a);
					if ($a instanceof TDBAttributeDefinition)
						$attributes[] = $a;		
				}
			if (key_exists("ancestor")){
				if ($meta["ancestor"] instanceof IClass)
					$anc = $meta["ancestor"];
				else if (is_string($meta["ancestor"])){
					$c = $this->GetClass($meta["ancestor"]);
					if ($c) $anc = $c;
				} 	
			}
			return new TDBClass($meta["name"],$anc,$attributes);
		}
		else if (count(array_diff(array("name","type"),$attrs)) == 0){
			if ($meta["type"] instanceof TDataType){
				if (key_exists("class",$meta)){
					if ($meta["class"] instanceof IClass)
						$c = $meta["class"];
					else if (is_string($meta["class"]))
						$c = $this->GetClass($meta["class"]);	
				}
				
				$static = (key_exists("static",$meta))?$meta["static"]:false; 
				$unique = (key_exists("unique",$meta))?$meta["unique"]:false;
				$indexed = (key_exists("indexed",$meta))?$meta["indexed"]:false; 
				$nullable = (key_exists("nullable",$meta))?$meta["nullable"]:true; 
				$default = (key_exists("default",$meta))?$meta["default"]:null;
				$periodic = (key_exists("periodic",$meta))?$meta["periodic"]:false;				
				
				if ($c)
					return new TDBAttributeDefinition($meta["name"], $meta["type"],$static,$unique,$indexed,$nullable,$default,$periodic,$c);
				else
					return new TDBAttributeDefinition($meta["name"], $meta["type"],$static,$unique,$indexed,$nullable,$default,$periodic);
			}
		}
		return null;
	}
	
	private function _define_class(TDBClass $c){
		$prk = null;
		$parentid = null;
			
		$table = $this->RealTableName(self::CLASSES_TABLE);
		$enttable = $this->RealTableName(self::ENTITIES_TABLE);
			
		$this->query("lock tables ".$table." write");	
		$anc = $c->Ancestor();
			
		if (!$anc) {
			$q = "select max(".self::CLASS_RKEY_FIELD.") + 1 as prk from ".$table;
			$prk = $this->scalarValue($q);
			if (!$prk) $prk = 1;
		}
		else 
		if ($temp = $this->Scalar("select ".self::CLASS_RKEY_FIELD." from ".$table." where ".self::CLASS_NAME_FIELD." = :cname",array("cname"=>$anc->Name()))){
			$prk = $temp[self::CLASS_RKEY_FIELD];
			if (!($anc instanceof TDBDefinedClass))
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			$parentid = $anc->Id();
		}	
			
		if (isset($prk)) {
			$this->BeginTransaction();
			try {
				$this->Execute("update ".$table." set ".self::CLASS_RKEY_FIELD." = ".self::CLASS_RKEY_FIELD." + 2, ".self::CLASS_LKEY_FIELD." = if(".self::CLASS_LKEY_FIELD." >= :prk,".self::CLASS_LKEY_FIELD." + 2,".self::CLASS_LKEY_FIELD.") where ".self::CLASS_RKEY_FIELD." >= :prk", array("prk"=>$prk));
				$this->Execute("insert ".$table." (".self::CLASS_NAME_FIELD.",".self::CLASS_LKEY_FIELD.",".self::CLASS_RKEY_FIELD.",".self::CLASS_ANCESTOR_FIELD.") values (:name,:lkey,:rkey,:ancestor)",array("name"=>$c->Name(),"lkey"=>$prk,"rkey"=>$prk+1,"ancestor"=>$parentid));
				$cid = $this->lastInsertId();
				$this->query("CREATE TABLE `".$enttable."_".$cid."` (`".self::ENTITY_PK_FIELD."` bigint(20) NOT NULL PRIMARY KEY, constraint `fk_".$enttable."_".$cid."` FOREIGN KEY (".self::ENTITY_PK_FIELD.") references ".$enttable." (".self::ENTITY_PK_FIELD.") on delete cascade) ENGINE=InnoDB DEFAULT CHARSET=utf8");
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
		return false;
	}
	
	protected static function transformColType(TDataType $type){
		switch ($type->TypeCode){
			case TMetaReferenceType::META_REFERENCE:return parent::transformColType(new TLongType(20));break;
			default:return parent::transformColType($type);break;
		}
	}	
	
	protected function quoteValue($value){
		if ($value instanceof IInstancet) 
			$value = $value->Id();
		return parent::quoteValue($value);
	}
	
	private function _define_attribute(TDBAttributeDefinition $a){
		if ($a->Class instanceof TDBDefinedClass){
			$coltype = self::TransformColType($a->Type);
			$refclass = null;
			if ($a->Type instanceof TMetaReferenceType)
				$refclass = $attrtype->Class->Id(); 
				
			$table = $this->RealTableName(self::ATTRDEFINITIONS_TABLE);				
			$this->BeginTransaction();
			try {
				$this->Execute("insert into ".$table." (".self::CLASS_PK_FIELD.",".self::CLASS_ATTR_NAME_FIELD.",".self::CLASS_ATTR_TYPE_FIELD.",".self::CLASS_ATTR_OPTIONS_FIELD.",".self::CLASS_ATTR_REFCLASS_FIELD.",".self::CLASS_ATTR_STATIC_FIELD.") values (:class,:name,:type,:opt,:refclass,:static)", array("class"=>$a->Class->Id(),"name"=>$a->Name(),"type"=>$a->Type->TypeCode,"opt"=>$a->__toString(),"refclass"=>$refclass,"static"=>$a->Static));
				$caid = $this->lastInsertId();
				$fieldname = "field".$caid;
				$this->Execute("update ".$table." set ".self::CLASS_ATTR_FIELD_FIELD." = :fld where ".self::CLASS_ATTR_PK_FIELD." = :id",array("fld"=>"field".$caid,"id"=>$caid));
					
				$_table_ = $a->Static?$this->RealTableName(self::CLASSES_STATICS):$this->RealTableName(self::ENTITIES_TABLE)."_".$a->Class->Id();
				if (($a->Periodic) && (!$a->Static)) 
					$this->query("CREATE TABLE IF NOT EXISTS `".$_table_.self::SNAPSHOTS_PREFIX."` (
							`".self::ENTITY_PK_FIELD."` bigint(20) NOT NULL, 
							`date_since` datetime NOT NULL, 
							`date_till` datetime, 
							constraint `fk_".$this->RealTableName(self::ENTITIES_TABLE)."_".$a->Class->Id()."_ss` FOREIGN KEY (".self::ENTITY_PK_FIELD.") references ".$this->RealTableName(self::ENTITIES_TABLE)."_".$a->Class->Id()." (".self::ENTITY_PK_FIELD.") on delete cascade,
							primary key (`".self::ENTITY_PK_FIELD."`,`date_since`)
							) ENGINE=InnoDB CHARSET=utf8");
					
				if ($a->Static) 
					$q = "ALTER TABLE ".$_table_." ADD COLUMN `".$fieldname."` ".$coltype." NULL ";
				else 
					$q = "ALTER TABLE ".$_table_." ADD COLUMN `".$fieldname."` ".$coltype." ".($a->Nullable?"NULL":"NOT NULL")." ".(isset($a->Default)?(sprintf("DEFAULT %s ",$this->quoteValue($a->Default))):" ");
					
				if ($a->Type instanceof TMetaReferenceType)
					$q = $q." , ADD INDEX `i".$fieldname."` (`".$fieldname."`), ADD CONSTRAINT fk_".$_table_."_".$fieldname." FOREIGN KEY `".$fieldname."` (`".$fieldname."`) REFERENCES `".$this->RealTableName(self::ENTITIES_TABLE).(is_null($a->ReferenceClass)?"":"_".$a->Type->ReferenceClass->Id())."` (`".self::ENTITY_PK_FIELD."`) ON DELETE ".($a->Type->IntergrityDelete?"CASCADE":($a->Nullable?"SET NULL":"RESTRICT"))." ON UPDATE RESTRICT";
				else if ($a->Type instanceof TTableReferenceType)
					$q = $q." , ADD INDEX `i".$fieldname."` (`".$fieldname."`), ADD CONSTRAINT fk_".$_table_."_".$fieldname." FOREIGN KEY `".$fieldname."` (`".$fieldname."`) REFERENCES `".$a->Type->MasterField->Table->Name."` (`".$a->MasterField->Name."`) ON DELETE ".self::transformRefIntegrity($a->Type->OnDelete)." ON UPDATE ".self::transformRefIntegrity($a->Type->OnUpdate);
				else 
					{
					if ($a->Unique && (!$a->Periodic) && (!$a->Static))
						$q = $q.", ADD UNIQUE `i".$fieldname."` (`".$fieldname."`)";
					else if ($a->Indexed)
						$q = $q.", ADD INDEX `i".$fieldname."` (`".$fieldname."`)";
					}
							
				$this->query($q);
					
				if ($a->Static && $a->Default){
					if ($a->Periodic)
						$this->SaveStatic($a->Class, $a, $a->Default,new TDate());
					else
						$this->SaveStatic($a->Class, $a, $a->Default);
				}
				$this->CommitTransaction();
			} catch (Exception $e) {
				$this->RollbackTransaction();
				throw $e;
				return false;
			}
			return true;
		} 
		return false;			
	}
	
	private function _undefine_class(TDBClass $c){
		$table = $this->RealTableName(self::CLASSES_TABLE);
		$res = $this->query("select ".self::CLASS_PK_FIELD.",".self::CLASS_LKEY_FIELD.",".self::CLASS_RKEY_FIELD." from ".$table." where ".self::CLASS_NAME_FIELD." = ".$this->quoteValue($c->Name()));
  		if ($temp = $res->FetchAssoc()){
  			if ($temp[self::CLASS_RKEY_FIELD] == $temp[self::CLASS_LKEY_FIELD] + 1){   
  					$this->BeginTransaction();
  					$this->query("lock tables ".$table." write");
  					try {
    					$this->Execute("delete from ".$table." where (".self::CLASS_LKEY_FIELD." >= :lkey) and (".self::CLASS_RKEY_FIELD." <= :rkey)",array("lkey"=>$temp[self::CLASS_LKEY_FIELD],"rkey"=>$temp[self::CLASS_RKEY_FIELD]));
			    		$this->Execute("update ".$table." set ".self::CLASS_RKEY_FIELD." = ".self::CLASS_RKEY_FIELD." - (:rkey - :lkey + 1), ".self::CLASS_LKEY_FIELD." =  if(".self::CLASS_LKEY_FIELD." > :rkey,".self::CLASS_LKEY_FIELD." - (:rkey - :lkey + 1),".self::CLASS_LKEY_FIELD.") where ".self::CLASS_RKEY_FIELD." > :rkey",array("lkey"=>$temp[self::CLASS_LKEY_FIELD],"rkey"=>$temp[self::CLASS_RKEY_FIELD]));
			    		$this->query("DROP TABLE `".$this->RealTableName(self::ENTITIES_TABLE)."_".$temp[self::CLASS_PK_FIELD]."`");
			    		$this->CommitTransaction();
			    		$this->query("unlock tables");
  					} catch (Exception $e){
  						$this->RollbackTransaction();
  						$this->query("unlock tables");
  						throw $e;
  						return false;
  					}
			    	return true;
  			} else throw new TMySqlException($this,TMySqlException::ERR_STORE_CLASS_NO_UNDEFINE,array("class"=>$c->Name()));
  		}
  		return false; 
	}
	
	private function _undefine_attribute(TDBAttributeDefinition $a){
		if ($a->Class instanceof TDBDefinedClass){	
			$this->BeginTransaction();
			try {
				$table = $this->RealTableName(self::ATTRDEFINITIONS_TABLE);
				if (!($a instanceof TDBDefinedAttribute)){
					if ($temp = $this->query("select * from ".self::ATTRDEFINITIONS_TABLE." where (".self::CLASS_PK_FIELD." = ".$this->quoteValue($a->Id()).") and (".self::CLASS_ATTR_NAME_FIELD." = ".$this->quoteValue($a->Name()).")")->FetchAssoc())
						$a = TMySqlRowConverter::ToAttrDefinition($temp, $this);
				}
				if (!($a instanceof TDBDefinedAttribute))
					throw new TMetaStorageException(TMetaStorageException::ERR_META_ATTR_NOT_DEFINED, array("attr"=>$a->Name,"class"=>$a->Class->Name()));
				$_table_ = 	$a->Static?($this->RealTableName(self::CLASSES_STATICS).($a->Periodic?self::SNAPSHOTS_PREFIX:"")):$this->RealTableName(self::ENTITIES_TABLE)."_".$a->Class->Id();
				$this->query("ALTER TABLE ".$_table_." DROP COLUMN ".$a->Field);
				$this->Execute("delete from ".$table." where (".self::CLASS_ATTR_NAME_FIELD." = :name) and (".self::CLASS_PK_FIELD." = :cid)",array("name"=>$a->Name(),"cid"=>$a->Class->Id()));
				$this->CommitTransaction();
			} catch (Exception $e){
				$this->RollbackTransaction();
				throw $e;
				return false;
			}
			return true;
		} 
		return false;				
	}
	
	private function _class_defined(TDBClass $c){
		return !is_null($this->Scalar("select ".self::CLASS_PK_FIELD." from ".$this->RealTableName(self::CLASSES_TABLE)." where ".self::CLASS_NAME_FIELD." = :cname",array("cname"=>$c->Name())));
	}
	
	private function _attribute_defined(TDBAttributeDefinition $a){
		return !is_null($this->Scalar("select a.".self::CLASS_ATTR_PK_FIELD." from ".$this->RealTableName(self::ATTRDEFINITIONS_TABLE)." a inner join ".$this->RealTableName(self::CLASSES_TABLE)." c on a.".self::CLASS_PK_FIELD." = c.".self::CLASS_PK_FIELD." where (c.".self::CLASS_PK_FIELD." = :cid) and (a.".self::CLASS_ATTR_NAME_FIELD." = :aname)",array("cid"=>$a->Class->Id(),"aname"=>$a->Name)));
	}
	
	private function _load_classes(){
		if (!$this->_classes_loaded_){
			$result = $this->query("select c.*, a.* 
				from ".$this->RealTableName(self::CLASSES_TABLE)." c 
					inner join ".$this->RealTableName(self::ATTRDEFINITIONS_TABLE)." a on c.".self::CLASS_PK_FIELD." = a.".self::CLASS_PK_FIELD." order by c.".self::CLASS_LKEY_FIELD." asc");
			
			while ($row = $result->FetchAssoc()){
				$c = TMySqlDBClass::Obtain($row[self::CLASS_PK_FIELD], $this);
				if (!$c){
					$anc = null;
					if (!is_null($row[self::CLASS_ANCESTOR_FIELD]))
						$anc = $this->GetClass($row[self::CLASS_ANCESTOR_FIELD]);
					$c = TMySqlDBClass::Create($row[self::CLASS_PK_FIELD], $row[self::CLASS_NAME_FIELD], $this,$anc);
				}
				if ($ca = TMySqlRowConverter::ToAttrDefinition($row, $this))
					$c->AttributeDefinitions = $ca;
			}
			$this->_classes_loaded_ = true;
		}
	}

	private function _get_attr_value(TInstanceAttribute $a){			
		$v = $a->getValue();
		switch ($a->Definition()->Type()->TypeCode){ 
			case TDataType::BOOLEAN:$v = ($v)?1:0;break;
			case TDataType::DATETIME:$v = $v?$v->ToString("Y-m-d"):null;break;
			case TDataType::FILE:
			case TDataType::FILELINK:
			case TDataType::IMAGE:
			case TDataType::IMAGELINK:$v = $v?$v->__toString():null;break;
		}
		return $v;			
	}
	
/**
 * defines class or attribute
 * @param IClass|IAttributeDefinition|array $meta
 * @return boolean
 */		
	public function Define($meta){
		if (is_array($meta))
			$meta = $this->_parse_meta($meta);
		if ($meta instanceof IClass)
			return $this->_define_class($meta);
		else if ($meta instanceof TDBAttributeDefinition)
			return $this->_define_attribute($meta);
		return false;
	}
/**
 * undefines class or attribute
 * @param mixed $meta
 * @return boolean
 */		
	public function Undefine($meta){
		if (is_array($meta)){
			$c = null;
			if (key_exists("class",$meta)){
				$name = null;
				if (is_string($meta["class"]))
					$meta["class"] = new TDBClass($meta["class"]);
				if (($meta["class"] instanceof IClass) && (key_exists("name",$meta)))
					$meta = new TDBAttributeDefinition($meta["name"], new TStringType(),false,false,false,true,null,false,$meta["class"]); 
			} else if (key_exists("name",$meta)) $meta = new TDBClass($meta["name"]);
		}
		if (is_string($meta))
			return $this->_undefine_class($this->GetClass($meta));
		if ($meta instanceof TDBClass)
			return $this->_undefine_class($meta);
		if ($meta instanceof TDBAttributeDefinition)
			return $this->_undefine_attribute($meta);
		return false;
	}
/**
 * checks if class is defined
 * @param mixed $meta
 * @return boolean
 */		
	public function IsDefined($meta){
		if (is_array($meta))
			$meta = $this->_parse_meta($meta);
		if ($meta instanceof IClass)
			return $this->_class_defined($meta);
		else if ($meta instanceof TDBAttributeDefinition)
			return $this->_attribute_defined($meta);
		return false;
	}
/**
 * gets class
 * @param string $meta
 * @return IClass
 * @see IClass
 */		
	public function GetClass($name){
		$this->_load_classes();
		$result = TMySqlDBClass::Obtain($name, $this);
		if (is_null($result))
			throw new TMetaStorageException(TMetaStorageException::ERR_STORE_CLASS_NOT_FOUND, array("class"=>$name));
		return $result;
	}
	
	
	private function _prepare_snapshot_record(array $record, array $values){
		$result = array();
		$result["flds"] = array_unique(array_merge(array_keys($record),array_keys($values)));
		$result["params"] = array_map(create_function('$key','return ":".$key;'),$result["flds"]);
		$result["values"] = array_merge($record,$values);
		return $result;
	}
	
	
	private function _set_snapshot_values($tablename, TDate $date, $idfield, $id, array $values){
		if (!empty($values)){
			$interval = $this->Fetch("select * from ".$tablename." where (".$idfield." = :ss_id) and (`date_since` <= :d) and ((`date_till` > :d) or (`date_till` is null))",array("ss_id"=>$id,"d"=>$date->ToString("Y-m-d")));
			foreach ($interval as $int){
				$date_since = new TDate($int["date_since"]);
				$record = $this->_prepare_snapshot_record($int, $values);
				$record["values"]["date_since"] = $date->ToString("Y-m-d");		
				$this->Execute("update ".$tablename." set `date_till` = :ds where (".$idfield." = :ss_id) and (`date_since` = :ods)",array("ss_id"=>$id,"ds"=>$date->ToString("Y-m-d"),"ods"=>$date_since->ToString("Y-m-d")));
				return $this->Execute("insert ".$tablename." (".join(",",$record["flds"]).") values (".join(",",$record["params"]).")",$record["values"]);
			} 
			
			$min_date = $this->Scalar("select min(`date_since`) from ".$tablename." where (".$idfield." = :ss_id)",array("ss_id"=>$id));
			$record = $this->_prepare_snapshot_record(array($idfield=>$id,"date_since"=>$date->ToString("Y-m-d"),"date_till"=>$min_date), $values);
			return $this->Execute("insert ".$tablename." (".join(",",array_keys($record["flds"])).") values (".join(",",$record["params"]).")",$record["values"]);
		}
		return 0;
	}
	
	private function _get_snapshot_values($tablename, TDate $date, mixed $field, $idfield, $id){
		if (is_string($field))
			$field = explode(",", $field);
		if (is_array($field) && (!empty($field))){
			if (count($field) == 1)
				return $this->Scalar("select ".$field[0]." from ".$tablename." where ".$idfield." = :ss_id and (`date_since` <= :dt) and ((`date_till` > :dt) or (`date_till` is NULL))",array(":ss_id"=>$id,"dt"=>$date->ToString("Y-m-d")));
			$result =  $this->Fetch("select ".join(", ", $field)." from ".$tablename." where ".$idfield." = :ss_id and (`date_since` <= :dt) and ((`date_till` > :dt) or (`date_till` is NULL))",array(":ss_id"=>$id,"dt"=>$date->ToString("Y-m-d")));
			foreach ($result as $r)
				return $r;	 
		}	
		return null;
	}
	
/**
 * 
 * gets value of class static attribute
 * @return mixed
 */		
	public function StaticValue(IClass $class, IAttributeDefinition $adef, TDate $date = null){
		if ($adef->IsStatic()){
			$_table_ = $this->RealTableName(self::CLASSES_STATICS);
			if ($adef->IsPeriodic()){
				if ($date instanceof TDate)
					return $this-> _get_snapshot_values($_table_.self::SNAPSHOTS_PREFIX, $date, $adef->Field, self::CLASS_PK_FIELD, $class->Id());
				else
					return $this->Scalar("select ".$adef->Field." from ".$_table_.self::SNAPSHOTS_PREFIX." where ".self::CLASS_PK_FIELD." = :cid and (`date_till` is NULL)",array("cid"=>$class->Id()));
			} else
				return $this->Scalar("select ".$adef->Field." from ".$_table_." where ".self::CLASS_PK_FIELD." = :cid",array("cid"=>$class->Id()));	
		}
		return null;
	}
/**
 * saves class static sttribute value
 * @param mixed $value
 * @return boolean
 */		
	public function SaveStatic(IClass $class, IAttributeDefinition $adef, $value, TDate $date = null){
		$result = 0;
		if ($adef->IsStatic()){
			$this->BeginTransaction();
			try {
				if ($adef->IsPeriodic()){
					if (is_null($date)) $date = new TDate();
					$result = $this->_set_snapshot_values($this->RealTableName(self::CLASSES_STATICS).self::SNAPSHOTS_PREFIX, $date, self::CLASS_PK_FIELD, $class->Id(), array($adef->Field=>$value));	
				} else {
					$result = $this->Execute("update ".$this->RealTableName(self::CLASSES_STATICS)." set ".$adef->Field." = :value where ".self::CLASS_PK_FIELD." = :cid",array("value"=>$value,"cid"=>$class->Id()));
					if ($result == 0)
						$result = $this->Execute("insert ".$this->RealTableName(self::CLASSES_STATICS)." (".self::CLASS_PK_FIELD.",".$adef->Field.") values (:cid,:value)",array("value"=>$value,"cid"=>$class->Id()));
				}
				$this->CommitTransaction();
			} catch (Exception $e) {
				$this->RollbackTransaction();
				throw $e;
			}
		}
		return $result > 0;
	}		
	

	private function _create_class_section($id,IInstance $instance, TDate $date = null, IClass $class){
		$aiv = create_function('$field, $value, &$flds,&$params,&$values', '$flds[] = $field;$params[] = ":".$field;$values[$field] = $v;');
		if ($class->Ancestor() instanceof IClass)
			$this->_create_class_section($id,$instance,$date,$class->Ancestor());
		$ads = $class->AttributeDefinitions();
		$q = "insert into ".$this->RealTableName(self::ENTITIES_TABLE)."_".$class->Id()." (".self::ENTITY_PK_FIELD;
		$pq = "insert into ".$this->RealTableName(self::ENTITIES_TABLE)."_".$class->Id().self::SNAPSHOTS_PREFIX." (".self::ENTITY_PK_FIELD.",`date_since`,`date_till`";
			
		$flds = array();
		$params = array();
		$vls = array("id"=>$id);
		$pflds = array();
		$pvls = array("id"=>$id,"date_since"=>$date->ToString("Y-m-d"));
		$pparams = array();
		
		foreach ($ads as $ad){
			$v = $instance->GetAttributeValue($ad->Name());
			if (!$ad->IsPeriodic())
				$aiv($ad->Field(),$v,$flds,$params,$vls);
			 else 
			 	$aiv($ad->Field(),$v,$pflds,$pparams,$pvls);
		}
		$this->Execute($q.((count($flds) > 0)?(",".join(",",$flds)):("")).") values (:id,".((count($params) > 0)?(",".join(",",$params)):("")).")",$vls);
		if (count($pflds) > 0)
			$this->Execute($pq.((count($pflds) > 0)?(",".join(",",$pflds)):("")).") values (:id,:date_since,null,".((count($pparams) > 0)?(",".join(",",$pparams)):("")).")",$pvls);
	}
				
	private function _create_instance(IInstance $instance, TDate $date = null){
		$this->BeginTransaction();
		try {
			$this->Execute("insert into ".$this->RealTableName(self::ENTITIES_TABLE)." (".self::CLASS_PK_FIELD.") values (:cid)",array("cid"=>$instance->InstanceClass()->Id()));
			$id = $this->lastInsertId();
			$this->_create_class_section($id,$instance,$date,$instance->InstanceClass());
			$this->CommitTransaction();
			$instance = new TMySqlDBInstance($id,$instance->InstanceClass());
			$this->LoadInstance($instance,$date);
		} catch (Exception $e) {
			$this->RollbackTransaction();
			throw $e;
		}
		return $instance;	
	}	

/**
 * saves instance to storage
 * @return boolean
 */		
	public function SaveInstance(IInstance $instance, TDate $date = null){	
		if (!is_null($instance->Id()))
		{
			$this->BeginTransaction();
			try {
				$attributes = $instance->Attributes();
				$updates = array();
				$values = array();
				$periodic_updates = array();						
					
				foreach ($attributes as $a)
			 		if ($a->NeedSave()){ 
			 			if ($a->Definition()->IsPeriodic()){
			 				$classname = $a->Definition()->AttributeClass()->Name();
							if (!key_exists($classname,$periodic_updates)){
								$periodic_updates[$classname] = array();
				  				$periodic_updates[$classname]["table"] = $this->RealTableName(self::ENTITIES_TABLE)."_".$a->Definition()->AttributeClass()->Id().self::SNAPSHOTS_PREFIX;
				  				$periodic_updates[$classname]["values"] = array();
							}
							$periodic_updates[$classname]["values"][$a->Definition()->Field] = $this->_get_attr_value($a);
			 			} 
			 			else {
							$updates[] = "e".$a->Definition()->AttributeClass()->Id().".".$a->Definition()->Field." = :".$a->Definition()->Field;
							$values[$a->Definition()->Field] = $this->_get_attr_value($a);
			 			}
			 		}
				 	
				$q = "update ".$this->RealTableName(self::ENTITIES_TABLE)."_".$instance->InstanceClass()->Id()." e".$instance->InstanceClass()->Id();
				$c = $instance->InstanceClass();
				$a = $instance->InstanceClass()->Ancestor();
				while ($a instanceof IClass){
					$q .= " inner join ".$this->RealTableName(self::ENTITIES_TABLE)."_".$a->Id()." e".$a->Id()." on e".$a->Id().".".self::ENTITY_PK_FIELD." = e".$c->Id().".".self::ENTITY_PK_FIELD;
					$c = $a;
					$a = $a->Ancestor();
				}
				$q .= " set ".join(",",$updates)." where e".$instance->InstanceClass()->Id().".".self::ENTITY_PK_FIELD." = :id"; 
				$values["id"] = $instance->Id();	
				$this->Execute($q, $values);	

				if (!($date instanceof TDate))
					$date = new TDate();
				foreach ($periodic_updates as $pu)
					$this->_set_snapshot_values($pu["table"], $date, self::ENTITY_PK_FIELD, $instance->Id(), $pu["values"]);
						
				$this->CommitTransaction();
			} catch (Exception $e) {
				$this->RollbackTransaction();
				throw $e;
			}
			//$instance = $this->FillInstance(new TMySqlDBInstance($instance->Id(),$instance->InstanceClass()),$date);
			return $instance;	
		} 
		else 
			return $this->_create_instance($instance,$date);
	}
/**
 * sets attributes of the set of instances according to specified mask
 * only non-periodic attributes are affected
 * @return boolean  
 */		
	public function InstancesBulkEdit(IInstance $mask, TDataSource $instances){
		$attributes = $mask->Attributes();
		$fields = array();
		$values = array();
		$search = $instances->Find("Class",$mask->InstanceClass());
		
		if (empty($search))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);

		$ds = $search[0]; 
		if (count($search) > 1)
			foreach ($search as $s)
			 if ($s->IsTarget){
			 	$ds = $s;
			 	break;
			 }
		
		foreach ($attributes as $a){
			$fields[] = new TMetaClassDataSourceAttribute($a->Definition(),null,$ds);
			$values[] = $a->getValue();
		}
		$p = new TMySqlMetaDataSetUpdateProcessor($instances, $this, $fields, $values);
		return $this->Execute($p->UpdateQuery(), $p->Parameters);
	}
/**
 * gets instance from storage
 * @param mixed $id instance unique id
 * @return IInstance
 * @see IInstance
 */		
	public function Instanciate($id){
		$result = null;
		$q = "select ".self::ENTITY_PK_FIELD.",".self::CLASS_PK_FIELD." from ".$this->RealTableName(self::ENTITIES_TABLE)." ";
		if (isset($id))
		  $q .= " where ".self::ENTITY_PK_FIELD." = :id";
		else {
			throw new TMySqlException($this,TMySqlException::ERR_STORE_NO_ENTITY_ID);
			return false;
		}  
		if ($res = new TMySqlDBInstanceIteratorAdapter($this->Fetch($q,array("id"=>$id)),$this))
			foreach ($res as $r){
				$result = $r->Item;
				break;
			}
		return $result;
	}
/**
 * sets instance attributes to values from storage
 * @return boolean
 * @see IInstance
 */		
	public function LoadInstance(IInstance $instance, TDate $date = null){
		$flds = array();
		$from = "";
		$inner = "";
		$left = "";
		$c = $instance->InstanceClass();  
		while ($c instanceof IClass){
			$flds[] = "e_".$c->Id().".*";

			if ($c === $instance->InstanceClass())
				$from = $this->RealTableName(self::ENTITIES_TABLE)."_".$c->Id()." e_".$c->Id();
			else
				$inner .= " inner join ".$this->RealTableName(self::ENTITIES_TABLE)."_".$c->Id()." e_".$c->Id()." on e_".$c->Id().".".self::ENTITY_PK_FIELD." = e_".$d->Id().".".self::ENTITY_PK_FIELD;

			if ($c->HasPeriodicAttributes()){
				$flds[] = "e_".$c->Id()."_ss.*";
				$left .= " left join ".$this->RealTableName(self::ENTITIES_TABLE)."_".$c->Id().self::SNAPSHOTS_PREFIX." e_".$c->Id()."_ss on (e_".$c->Id()."_ss.".self::ENTITY_PK_FIELD." = e_".$d->Id().".".self::ENTITY_PK_FIELD.") and (e_".$c->Id()."_ss.`date_since` <= :dt) and ((e_".$c->Id()."_ss.`date_till` > :dt) or (e_".$c->Id()."_ss.`date_till` is null))"; 
			}	
			$d = $c;
			$c = $c->Ancestor();
		}
		
		$inner .= " inner join ".$this->RealTableName(self::ENTITIES_TABLE)." e on e.".self::ENTITY_PK_FIELD." = e_".$d->Id().".".self::ENTITY_PK_FIELD;
		$inner .= " inner join ".$this->RealTableName(self::CLASSES_TABLE)." c on c.".self::CLASS_PK_FIELD." = e.".self::CLASS_PK_FIELD;
		
		$result = $this->Fetch("select ".join(",",$flds)." from ".$from.$inner.$left." wnere e_".$instance->InstanceClass()->Id().".".self::ENTITY_PK_FIELD." = :id", array("id"=>$instance->Id(),"dt"=>$date->ToString("Y-m-d")));
		foreach ($result as $row){
			foreach ($row as $key=>$value)
			 if (!is_numeric($key))
			 	if ($instance->InstanceClass()->GetAttributeDefinition($key))
			 		$instance->SetAttributeValue($key,$value,false);
			 return true;
		}
		return false;
	}
	
/**
 * gets instance snapshots for specified period 
 * @param IInstance $instance
 * @param TDate $since
 * @param TDate $till
 * @return IIterator<IInstance>
 */	
	public function SnapShots(IInstance $instance,TDate $since = null, TDate $till = null){
		$j = "";
		$base = array();
		$flds = array();
		$c = $instance->InstanceClass();
		while ($c instanceof IClass){
			if ($c->HasPeriodicAttributes()){
				$j .= " left join ".$this->RealTableName(self::ENTITIES_TABLE)."_".$c->Id().self::SNAPSHOTS_PREFIX." e_".$c->Id()." on (e_".$c->Id().".".self::ENTITY_PK_FIELD." = e.".self::ENTITY_PK_FIELD.") and (e_".$c->Id().".`date_since` = e.`date_since`)";
				$q = "select ".self::ENTITY_PK_FIELD.", `date_since` from ".$this->RealTableName(self::ENTITIES_TABLE)."_".$c->Id().self::SNAPSHOTS_PREFIX." where (".self::ENTITY_PK_FIELD." = :id)";
				if (!is_null($since)){
					$q .= " and (`date_since` >= :since)";
					$base[] = "select ".self::ENTITY_PK_FIELD.", max(`date_since`) as `date_since` from ".$this->RealTableName(self::ENTITIES_TABLE)."_".$c->Id().self::SNAPSHOTS_PREFIX." where (".self::ENTITY_PK_FIELD." = :id) and (`date_since` < :since)";
				}
				if (!is_null($till))
					$q .= " and (`date_since` <= :till)";
				$base[] = $q;
				
				$flds[] = "e_".$c->Id().".*"; 	
			}
			$c = $c->Ancestor();
		}
		$q = "select e.`date_since` as snapshot_date,".join(",",$flds)." from (".join(" union distinct ",$base).") e".$j." order by e.`date_since` asc";
		$parameters = array("id"=>$instance->Id());
		if (!is_null($since))
			$parameters = array("since"=>$since->ToString("Y-m-d"));
		if (!is_null($till))
			$parameters = array("till"=>$till->ToString("Y-m-d"));
			
		$result = new TMySqlDBSnapshotIterator($this->Fetch($q, $parameters),$this,$instance,$since);
		return $result;
	}	

/**
 * deletes instance from storage
 * @return boolean
 * @see IInstance
 */		
	public function DeleteInstance(IInstance $instance){
		$this->BeginTransaction();
		try {			
			$this->Execute("delete from ".$this->RealTableName(self::ENTITIES_TABLE)." where ".self::ENTITY_PK_FIELD." = :id",array("id"=>$instance->Id()));
			$this->CommitTransaction();
		} catch (Exception $e){
			$this->RollbackTransaction();
			throw $e;
		}
		return true;
	}
/**
 * deletes instances of specified class according to conditions
 * @return boolean
 */		
	public function DeleteInstances(TDataSource $instances){
		$p = new TMySqlMetaDataSetProcessor($instances, $this);
		return $this->Execute($p->DeleteQuery(), $p->Parameters);
	}		
	
	public function FetchInstances(TDataSource $instances){
		$p = new TMySqlMetaDataSetProcessor($instances, $this);
		return new TMySqlDBInstanceIteratorAdapter($this->Fetch($p->SelectQuery(), $p->Parameters));
	}							
}
