<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Exceptions
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


/**
 * @package CoreRuntimeClasses
 * @subpackage Exceptions
 * @category Core
 * Basic exception class. Extends php exception functionality.
 */
	abstract class TException extends Exception {		
/**
 * @var boolean specifies whether to include stack trace in exception output 
 */		
		public static $EnableStackTrace = false;
/**
 * gets error message text by error code
 * @param int $msgcode
 */		
		protected abstract function getMessageText($msgcode);
		
/**
 * exception constructor
 * @param int $msgcode
 * @param string $context
 */		
		public final function __construct($msgcode,array $parameters = array()){
			$msg = _i($this->getMessageText($msgcode));
			foreach ($parameters as $key=>$value)
				$msg = str_ireplace("%".$key,$value,$msg);
			parent::__construct($msg,$msgcode);
		}

/**
 * gets string representation of exception
 * @return string
 */		
		public function __toString(){
			$text = _i("Exception %code of class ".get_class($this)." at line %line in %file:\n%message");
			$text = str_ireplace('%code',$this->getCode(),$text);
			$text = str_ireplace('%line',$this->getLine(),$text);
			$text = str_ireplace('%file',$this->getFile(),$text);
			$text = str_ireplace('%message',$this->getMessage(),$text);
			if (self::$EnableStackTrace)
				$text .= "\n".$this->getTraceAsString();
			return $text;
		}
	}
	
/**
 * @package CoreRuntimeClasses
 * @subpackage Exceptions
 * @category Core
 * 
 * core exceptions
 */	
	final class TCoreException extends TException {
		const ERR_NO_APPCONF = 100;
		const ERR_APPCONF_INCORRECT = 101;
		const ERR_SERVICE_NOT_FOUND = 102;
		const ERR_BAD_VALUE = 103;
		const ERR_BAD_TYPE = 104;
		const ERR_ACCESS_DENIED = 105;
		const ERR_CONF_INCORRECT = 106;
		const ERR_NO_INTERFACE = 107;
		const ERR_SESS_OPEN = 108;
		const ERR_SESS_CLOSED = 109;
		const ERR_NOT_CLASS = 110;
		const ERR_FILE_MISSING = 111;
		const ERR_RESOURCE_NOT_FOUND = 112;

/**
 * @param int $msgcode
 * @return string
 */		
		protected function getMessageText($msgcode){
			switch ($msgcode){
				case self::ERR_NO_APPCONF:return 'Application configuration file not found!';break;
				case self::ERR_APPCONF_INCORRECT:return 'Application configuration file incorrect!';break;
				case self::ERR_SERVICE_NOT_FOUND:return 'Service not found!';break;
				case self::ERR_BAD_VALUE:return 'Inapropriate value specified!';break;
				case self::ERR_BAD_TYPE:return 'Value of inapropriate type specified!';break;
				case self::ERR_ACCESS_DENIED:return 'Access denied!';break;
				case self::ERR_CONF_INCORRECT:return 'Configuration file incorrect!';break;
				case self::ERR_NO_INTERFACE:return 'Object does not implement needed interface!';break;
				case self::ERR_SESS_CLOSED:return 'Can not operate on closed session!';break;
				case self::ERR_SESS_OPEN:return 'Can not set session handler on opened session!';break;
				case self::ERR_NOT_CLASS:return 'Object must be of %class class!';break;
				case self::ERR_FILE_MISSING:return 'File %file missing!';break;
				case self::ERR_RESOURCE_NOT_FOUND: return 'Resource not found!';break;
			}
		}
	}
	
?>