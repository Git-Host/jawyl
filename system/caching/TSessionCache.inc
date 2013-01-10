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
 * Session cache. Holds cache values for current application session.
 * @see TCache
 */
class TSessionCache extends TCache {
	
	protected function &loadStorage(){
		return $this->Application()->Session()->Get($this->name);
	}
	
	protected function saveStorage(array &$storage){
		$this->Application()->Session()->Set($this->name,$storage);
	}	
}
?>