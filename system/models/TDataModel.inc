<?php
TApplication::import("core::TService");
TApplication::import("common::mvc");
TApplication::import("common::storage");
TApplication::import("common::model::interfaces");


abstract class TDataModel extends TModel implements IInstallable {
/**
 * @var IDBDriver
 */	
	protected $dbDriver;
	
	public function __set($nm,$val){
		switch ($nm){
			case "DataSource":{
				TConfigurablePattern::SetIocMember($this->dbDriver, $val, 'IDBDriver');	
				if ($this->dbDriver instanceof IDBDriver)
					$this->dbDriver->Connect();
			};break;
		/*	case "DBCache":{
				if (is_string($val))
					$val = TConfigurable::Instance($val);
				if ($val instanceof ICache)
					$this->dbDriver->SetCache($val);
			}break;
			case "DBChangeLog":{
				if (is_string($val))
					$val = TConfigurable::Instance($val);
				if ($val instanceof IChangeLog)
					$this->dbDriver->SetChangeLog($val);
			}break;
			case "DBCachingLifeTime":{$this->dbDriver->CacheLifeTime($val);}break;*/
			default:parent::__set($nm,$val);break;
		}
	}		
}
