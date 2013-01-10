<?php
/**
 * @package Caching
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

require_once 'common/cache.inc';

/**
 * @package Caching
 * @subpackage Session
 * @category System
 * Text file based changelog
 * @see TChangeLog
 */
class TTextChangeLog extends TChangeLog {
	private $records  = null;
/**
 * @var string
 */	
	public $LogFile;
/**
 * @var boolean;
 */	
	private $_store_log_ = true;
	
	public function __set($nm,$value){
		if ($nm == "StoreLog") $this->_store_log_ = TConvertions::ConvertToBoolean($value);
		else
		parent::__set($nm,$value);
	}
	
	public function __get($nm){
		if ($nm == "StoreLog") return $this->_store_log_;
		return parent::__get($nm); 
	}
	
	private function getlog(){
		if (is_null($this->records) || (!$this->_store_log_)){
			$this->records = array();
			$fn = $this->LogFile;
			if (is_null($fn)) $fn = $this->Application()->AppPath()."/changelog.txt";
			if (!file_exists($fn)) return $this->records;
			$recs = explode("\n",file_get_contents($fn));
			foreach ($recs as $rec)
				if ($rec != ""){
					$temp = explode(":",$rec);
					$this->records[$temp[0]] = new TDate((int)$temp[1]);	
				}	
		}
		return $this->records;
	}
	
/**
 * gets object modification time 
 * @param string $objectid
 * @return TDate
 */	
	public function GetModDateTime($objectid){
		$recs = $this->getlog();
		if (!key_exists($objectid,$recs)) return null;
		return $recs[$objectid];
	}
/**
 * checks whether specified objects were chaged since specified time  
 * @param string[]|string $objects
 * @return boolean
 */	
	public function CheckModified($objects,TDate $checktime){
		$recs = $this->getlog();
		foreach ($objects as $obj)
			if (key_exists($obj,$recs))
				if ($checktime->TimeStamp() < $recs[$obj]->TimeStamp()) return true;
		return false;
	}
/**
 * logs objects change
 * @param string[]|string $objects
 * @return boolean
 */	
	public function LogChange($objects){
		$this->records = null;
		$recs = $this->getlog();
		$cnt = "";
		if (is_array($objects)){
		foreach ($objects as $obj)
			$this->records[$obj] = new TDate();
		} else 
			$this->records[$objects] = new TDate();
		
		foreach ($this->records as $obj=>$dt)
			$cnt .= $obj.":".$dt->TimeStamp()."\n";
		$fn = $this->LogFile;
		if (is_null($fn)) $fn = $this->Application()->AppPath()."/changelog.txt";
		file_put_contents($fn,$cnt);	
	}		
}
?>