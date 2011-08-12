<?php
/**
 * @package Caching
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


TApplication::import("common::cache");

/**
 * @package Caching
 * @subpackage RunTime
 * @category System
 * Runtime cache. Holds cache values for current application response.
 * @see TCache
 */
class TRunTimeCache extends TCache {
	public $UnitClass;
/**
 * @var array
 */	
	protected static $cache;
/**
 * @return TArrayCacheUnit
 * @see ICache::GetCacheUnit()
 */	
	public function GetCacheUnit($cacheid){
		if (!isset(self::$cache))  self::$cache = array();
		if (!isset(self::$cache[$cacheid])){
			if (isset($this->UnitClass)){
				$c = new ReflectionClass($this->UnitClass);
				self::$cache[$cacheid] = $c->newInstance($cacheid);
			}
			else
			self::$cache[$cacheid] = new TArrayCacheUnit($cacheid);
		}
		return self::$cache[$cacheid];
	}	
}
?>