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
/**
 * @var array
 */	
	protected static $cache;

	protected function &loadStorage(){
		return self::$cache;
	}
	protected function saveStorage(array &$storage){
		if ($storage !== self::$cache)
			self::$cache = $storage;
	}	
}
?>