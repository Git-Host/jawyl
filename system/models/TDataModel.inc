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
				$this->setIocMember($this->dbDriver, $val, 'IDBDriver');	
				if ($this->dbDriver instanceof IDBDriver)
					$this->dbDriver->Connect();
			};break;
			default:parent::__set($nm,$val);break;
		}
	}		
}
