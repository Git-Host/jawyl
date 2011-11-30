<?php
/**
 * @package Common
 * @subpackage Cache
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("core::TConfigurable");

/**
 * @package Common
 * @subpackage Cache
 * @category System
 * interface for implementing caches
*/
interface ICache {
/**
 * gets cache unit by id
 * @param string $cacheid id of cache unit
 * @return ICacheUnit
 * @see ICacheUnit
 */	
	public function GetCacheUnit($cacheid);
}

/**
 * @package Common
 * @subpackage Cache
 * @category System
 * interface for implementing cache units
*/
interface ICacheUnit {
/**
 * constructor
 * @param string $cacheid cache unit id
 */	
	public function __construct($cacheid);
/**
 * stores data in cache unit (appends data to something already stored)
 * @param mixed $data
 */	
	public function CacheData($data);
/**
 * gets cache unit data. If data is not yet stored should return null.
 * @return mixed
 */	
	public function Data();
/**
 * set cache unit data size
 * @param int $size
 */	
	public function SetSize($size);
/**
 * set cache unit life time in seconds
 * @param int $lifetime
 */	
	public function SetLifeTime($lifetime);
/**
 * gets time when data was cached
 * @return TDate
 */	
	public function GetCacheDateTime();
}

/**
 * @package Common
 * @subpackage Cache
 * @category System
 * abstract class for implementing cache units
 * @see ICacheUnit
*/
abstract class TCacheUnit implements ICacheUnit {
/**
 * @var string unit id 
 */	
	protected $id;
/**
 * @var int timestamp of cache unit creation 
 */	
	protected $created;
/**
 * @var int 
 */	
	protected $size = null;
/**
 * @var int 
 */	
	protected $lifeTime = null;

/**
 * constructor
 * @param string $id
 * @see ICacheUnit
 */	
	public function __construct($id){
		$this->id = $id;
		$this->created = time();
	}

/**
 * @param int $size
 * @see ICacheUnit 
 */	
	public function SetSize($size){
		$this->size = $size;
		$this->created = time();
	}
	
/**
 * @param int $lifetime
 * @see ICacheUnit 
 */	
	public function SetLifeTime($lifetime){
		$this->lifeTime = $lifetime;
	}
/**
 * gets size of unit data. Should be overriden in descendants.
 * @return int
 */
	protected abstract function getDataSize();
	
	public function GetCacheDateTime(){
		return new TDate($this->created);
	}
}	

/**
 * @package Common
 * @subpackage Cache
 * @category System
 * Cache unit that stores data as an array.
 * @see TCacheUnit
 */
class TArrayCacheUnit extends TCacheUnit {
/**
 * @var array unit data
 */	
	protected $data = array();
/**
 * gets size of unit data array
 * @return int
 */	
	protected function getDataSize(){
		return count($this->data);
	}
	
	public function SetSize($size){
		$this->data = array();
		parent::SetSize($size);
	}
	
/**
 * gets data stored by cache unit. If unit is expired data is cleared and null returned as if data was not stored yet.
 * @return mixed
 * @see ICacheUnit
 */	
	public function Data(){
		if (!is_null($this->lifeTime)){
			if ($this->created + $this->lifeTime < time()){
				$this->size = null;
				$this->data = array();
			}
		}
		if (!is_null($this->size))
			if ($this->size == $this->getDataSize())
				return $this->data;
		return null;
	}	
	
/**
 * appends specified data to unit data array
 * @param mixed $data data to append
 */	
	public function CacheData($data){
		$this->data[] = $data;
	}
}

/**
 * @package Common
 * @subpackage Cache
 * @category System
 * abstract class to implement configurable caches
 * @see TConfigurable
 */
abstract class TCache extends TConfigurable implements ICache {
	public $UnitClass;
/**
 * constructor
 * @param string $name
 */	
	public function __construct($name){
		parent::__construct($name);
	}
	
/** 
 * loads cached data into an array
 * @return array
 */	
	protected abstract function &loadStorage();
/**
 * saves cached data from an array
 * @param array $storage
 * @return boolean
 */	
	protected abstract function saveStorage(array &$storage);

	public function GetCacheUnit($cacheid){
		$c = $this->loadStorage();
		if (!is_array($c)) $c = array();
		if (!key_exists($cacheid,$c)){
			$uc = $this->UnitClass?$this->UnitClass:'TArrayCacheUnit';
			$c[$cacheid] = new $uc($cacheid);
			$this->saveStorage($c);
		}
		return $c[$cacheid];
	}	
}

?>