<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Configuration
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

interface IConfigurable {
	public function Name();
}

/**
 * A static pattern class which implements basic configuration logic, leting association of named values with application objects.
 * In configuration file &quot;application.xml&quot; you can specify any named settings for any named application object.
 * When application loads configuration file, it assigns specified values to object's attributes. 
 * If a configurable object has public attribute with the same name as a configuration setting then it's value is assigned to that attribute.
 * This pattern class implements error safe value assigning to object's attributes. 
 * @package CoreRuntimeClasses
 * @subpackage Configuration
 * @category Core
 */
 final class TConfigurablePattern {
/**
  * @var array associative array of object settings not having corresponding public attributes 
*/	
 	private static $_config_ = array();
/**
  * @var array associative array of named configurable objects 
*/	 	
 	private static $_instances_ = array();
/**
 * factory method. Registers configurable object in global configuration. Should be used in constructors of configurable objects. 
 * @param IConfigurable $name name of configurable object
 * @see IConfigurable
 */ 	
 	public static function Instanciate(IConfigurable $instance){
 		self::$_instances_[$instance->Name()] = $instance; 		
 	}
 	
 /**
 * factory method. Returns configurable object with specified name, if it was already created. 
 * @param string $name name of configurable object
 * @returns TConfigurable
 * @see TConfigurable
 */ 	
 	public static function Instance($name){
 		if (key_exists($name,self::$_instances_))
 			return self::$_instances_[$name];
 		return null;
 	} 	
 	
/**
  * @ignore 
*/	
 	private static function _get_property($instance, $name){
 		$c = new ReflectionClass(get_class($instance));
 		return self::_get_cp($c,$name);
 	}
 	
/**
  * @ignore 
*/	
 	private static function _get_cp($class,$name){
  		$prop = null;
  		$prop = $class->getProperty($name);
  		if (!is_null($prop)) return $prop;
  		if ($class->getParentClass())
  			return self::_get_cp($class->getParentClass(),$name);
  		return null;
  	}
  	
  	private static function _eval_ioc_array_members(array &$member, $type, $callback){
  		foreach ($member as &$m){
  			$obj = self::GetIocMember($m, $type);
  			if (($obj) && is_callable($callback)) call_user_func($callback, $obj);
  		}	
  	}

/**
 * Assignes multiple IOC dependency injection. Dependencies are evaluated where possible.
 * @param array $member reference to an object member holding a dependency
 * @param mixed $value assigned value
 * @param string|array|callback $type an expression for type checks, can be a class or interface name, an array of names or a callback function
 * @param callback $callback handler which is called for evaluated dependency
 */  	
  	public static function SetIocArrayMember(array &$member,$value,$type,$callback = null){
  		self::_eval_ioc_array_members($member, $type, $callback);
  		$m = null;
  		self::SetIocMember($m, $value, $type);
  		$member[] = $m;
  		if (($m) && is_callable($callback,false)) call_user_func($callback, $m);
  	}

/**
 * Gets value of multiple IOC dependency injection. Dependencies are evaluated where nescessary.
 * @param array $member reference to an object member holding a dependency
 * @param string|array|callback $type an expression for type checks, can be a class or interface name, an array of names or a callback function
 * @param callback $callback handler which is called for evaluated dependency
 * @param boolean $last
 */  	
  	public static function GetIocArrayMember(array &$member,$type,$callback = null,$last = false){
  		self::_eval_ioc_array_members($member, $type, $callback);
  		if ($last)
  			if (!empty($member))
  				return $member[count($member) - 1];
  			else return null;	
  		return $member;
  	}
  	
  	private static function _check_ioc_type($value,$type){
  		if (is_callable($type,false))
  			return call_user_func($type,$value);
  		else if (is_array($type)){
  			$result = true;
			foreach ($type as $t){
				$result = self::_check_ioc_type($value, $t);
				if (!$result) break;
			}	
			return $result;
  		}
  		if (is_object($value))  		
  			return $value instanceof $type;
  		return gettype($value) == $type; 
  	}

/**
 * 
 * Assignes IOC dependency injection, which is evaluated if possible.
 * @param mixed $member reference to an object member holding a dependency
 * @param mixed $value assigned value 
 * @param string|array|callback $type an expression for type checks, can be a class or interface name, an array of names or a callback function
 * @throws TCoreException
 */  	
  	public static function SetIocMember(&$member,$value,$type){
		if (is_string($value)){
			$member = $value;
			$value = TConfigurable::Instance($value);
		}
		if (self::_check_ioc_type($value, $type))
			$member = $value;
		else if ($value) {
			throw new TCoreException(TCoreException::ERR_BAD_TYPE);
		}
	}
	
/**
 * 
 * Gets value of IOC dependency injection, which is evaluated if nescessary.
 * @param mixed $member reference to an object member holding a dependency
 * @param string|array|callback $type an expression for type checks, can be a class or interface name, an array of names or a callback function
 * @throws TCoreException
 */	
	
	public static function GetIocMember(&$member,$type){
		if (is_string($member)){
			$member = TConfigurable::Instance($member);
			if (($member) && (!self::_check_ioc_type($value, $type)))
				throw new TCoreException(TCoreException::ERR_BAD_TYPE);
		}
		return $member;	
	}
  	
 /**
  * sets object attribute to a configuration value
  * @param object $instance an object which attribute should be set
  * @param string $name name of the setting (attribute)
  * @param mixed $value value to be assigned 
  */ 	
  	
  	public static function set(IConfigurable $instance, $name, $value){
  		$prop = self::_get_property($instance,$name);
  		if (isset($prop)){
		  if ($prop->IsPublic()){	
  			if ($prop->isStatic())
  				return $prop->setValue(null,$value);
  			else
  				return $prop->setValue($instance,$value);
		  }
  		}  else {
  			if (!key_exists($instance->Name(),self::$_config_)) self::$_config_[$instance->Name()] = array();
  			self::$_config_[$instance->Name()][$name] = $value;
  		}
  		return false;
  	}
  	
 /**
  * gets object setting value
  * @param object $instance an object which setting to get
  * @param string $name name of the setting (attribute)
  * @returns mixed
  */ 	
  	
  	public static function get(IConfigurable $instance, $name){
  		$prop = self::_get_property($instance,$name);
  		$result = null;
		if (isset($prop)){
		  if ($prop->IsPublic()){	
  			if ($prop->isStatic())
  				$result = $prop->getValue(null);
  			else
  				$result = $prop->getValue($instance);
		  }
		} else if ($instance instanceof IConfigurable){
  			if (key_exists($instance->Name(),self::$_config_))
				if (key_exists($name,self::$_config_[$instance->Name()]))
					$result = self::$_config_[$instance->Name()][$name];
		}
		return $result; 
  	}
  	
 /**
  * checks if object setting is set
  * @param object $instance an object which setting is tested
  * @param string $name name of the setting (attribute)
  * @returns boolean
  */ 	
  	public static function is_set(IConfigurable $instance, $name){
  		$prop = self::_get_property($instance,$name);
		if (isset($prop))
			return !is_null($property->getValue($instance));
		else if ($instance instanceof IConfigurable)
			if (key_exists($instance->Name(),self::$_config_))
				return isset(self::$_config_[$instance->Name()][$name]);
		return false;
  	}
  	
 /**
  * clears object setting
  * @param object $instance an object which setting to clear
  * @param string $name name of the setting (attribute)
  */ 	
  	public static function un_set(IConfigurable $instance, $name){
  		$prop = self::_get_property($instance,$name);
		if (isset($prop))
		 return $prop->setValue($instance,null);
		 else if ($instance instanceof IConfigurable)
			if (key_exists($instance->Name(),self::$_config_))
		 		if (key_exists($name,self::$_config_[$instance->Name()])){
		 			unset(self::$_config_[$instance->Name()][$name]);
		 			$result = true;
				}		 	
  		return false;
  	}
 } 
 
/**
 * A base abstract class for all named application runtime objects. It uses TConfigurablePattern to set its settings, 
 * but in addition provides opportunity to set configuration settings that don&apos;t have corresponding public attributes.
 * In this case setting value is stored in internal settings array, and can be accessed by magic __get and __set.
 * Also TConfigurable works as a factory, providing access by name to all configurable objects objects.
 * @package CoreRuntimeClasses
 * @subpackage Configuration
 * @category Core
 *  
 * @see TConfigurablePattern
 */ 

 abstract class TConfigurable implements IConfigurable {
/**
  * @var string name of configurable object 
*/	
 	protected $name;
/**
  * @var IConfigurable configurable container of object 
*/	
 	protected $container;
/**
 * class constructor. You need to pass it a unique name of configurable object.
 * It is declared as protected to make TApplication constructor protected. 
 * Descendants can make (and mostly do so) their constructors public, 
 * so they can be instantiated by application object at runtime. 
 * @param string $name name of object being created
 */ 	
	function __construct($name, IConfigurable $container = null){
 		$this->name = $name;
 		$this->container = $container;
 		TConfigurablePattern::Instanciate($this);
 	}
 	
 	public function Name(){
 		return $this->name;
 	}
 	
/**
 * factory method. Returns configurable object with specified name, if it was already created. 
 * @param string $name name of configurable object
 * @returns TConfigurable
 * @see TConfigurable
 */ 	
 	public static function Instance($name){
 		return TConfigurablePattern::Instance($name);
 	}
 	/**
 	 * @ignore
 	 */
	public function __set($nm,$value){
  		TConfigurablePattern::set($this,$nm,$value); 
	}
	
 	/**
 	 * @ignore
 	 */
	public function __get($nm){
		$result = TConfigurablePattern::get($this,$nm);
		return $result;
	}
	
 	/**
 	 * @ignore
 	 */
	public function __isset($nm){
		return TConfigurablePattern::is_set($this,$nm);
	}
	
 	/**
 	 * @ignore
 	 */
	public function __unset($nm){
		return TConfigurablePattern::un_set($this,$nm);
	}	
 }
?>