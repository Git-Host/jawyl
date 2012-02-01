<?php
/**
 * @package Common
 * @subpackage Cache
 * @category System
 * interface for implementing change logs
*/
interface IChangeLog {
/**
 * gets object modification time 
 * @param string $objectid
 * @return TDate
 */	
	public function GetModDateTime($objectid);
/**
 * checks whether specified objects were chaged since specified time  
 * @param string[]|string $objects
 * @return boolean
 */	
	public function CheckModified($objects,TDate $changetime);
/**
 * logs objects change
 * @param string[]|string $objects
 * @return boolean
 */	
	public function LogChange($objects);
}

/**
 * @package Common
 * @subpackage Cache
 * @category System
 * abstract class to implement configurable change logs
 * @see TConfigurable
 */
abstract class TChangeLog extends TConfigurable implements IChangeLog {
/**
 * constructor
 * @param string $name
 */	
	public function __construct($name){
		parent::__construct($name);
	}	
} 