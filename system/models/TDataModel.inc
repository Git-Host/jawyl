<?php
require_once 'core/TService.inc';
require_once 'common/mvp.inc';
require_once 'common/storage.inc';
require_once 'common/component/installable.inc';


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
