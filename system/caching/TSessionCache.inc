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
 * @subpackage Session
 * @category System
 * Session cache. Holds cache values for current application session.
 * @see TCache
 * @see TApplicationSession
 */
class TSessionCache extends TCache {	
/**
 * @return TArrayCacheUnit
 * @see ICache::GetCacheUnit()
 */		
	public function GetCacheUnit($cacheid){
		$c = TApplicationSession::Get($this->name);
		
		if (is_null($c)) $c = array();
		if (!key_exists($cacheid,$c)){
			if (isset($this->UnitClass)){
				$cl = new ReflectionClass($this->UnitClass);
				$c[$cacheid] = $cl->newInstance($cacheid);
			}
			else
			$c[$cacheid] = new TArrayCacheUnit($cacheid);
		}
		TApplicationSession::Set($this->name,$c);
		return $c[$cacheid];
	}	
}
?>