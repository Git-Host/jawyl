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
 * A base abstract class for all named application runtime objects.
 * Also TConfigurable works as a factory, providing access by name to all configurable objects objects.
 * @package CoreRuntimeClasses
 * @subpackage Configuration
 * @category Core
 *  
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
 	
 	private static $_instances_ = array();
 	
 	private static $_ioc_types_ = array();
 	
 	public static $CompileIocTypes = false;
 	
/**
 * class constructor. You need to pass it a unique name of configurable object.
 * It is declared as protected to make TApplication constructor protected. 
 * Descendants can make (and mostly do so) their constructors public, 
 * so they can be instantiated by application object at runtime. 
 * @param string $name name of object being created
 */ 	
	public function __construct($name, IConfigurable $container = null){
 		$this->name = $name;
 		$this->container = $container;
 		self::$_instances_[$name] = $this;
 		if (!isset(self::$_ioc_types_[get_class($this)])){
			$filename = TApplication::$Application->IocTypesCompileDir.'/'.get_class($this).'.ioc.php'; 			
 			if (self::$CompileIocTypes)
 				self::$CompileIocTypes(get_class($this),$filename,true);
 			if (file_exists($filename)){
 				require_once($filename);
 				self::$_ioc_types_[get_class($this)] = $ioctypes;
 			}
 		}
 	}
 	
 	public static function CompileIocTypes($classname,$filename,$checkmtime = true){
 		$cft = file_exists($filename)?filemtime($filename):0;
 		$needcompile = (($cft == 0) || (!$checkmtime))?true:false;
 					
 		$classes = array();				
 		$rc = new ReflectionClass($classname);
 		while ($rc instanceof ReflectionClass){
 			$classes[] = $rc;
 			$needcompile = (filemtime($rc->getFileName()) > $cft)?true:$needcompile;
 			$rc = $rc->getParentClass();
 		}
 		if ($needcompile){
 			$needsave = false;
 			$types = array();
 			foreach ($classes as $rc){
 				$prop = $rc->getProperties(ReflectionProperty::IS_PROTECTED & (~ReflectionProperty::IS_STATIC));
 				foreach ($prop as $p){
					$iocname = $p->getName();
 					if (preg_match('/^_ioc_[a-z_]*[a-z]_$/', $iocname)){
						if ($dc = $p->getDocComment()){
							preg_match('/@var\s+(\w*)(\[\s*\])?/',$dc,$matches);
							if (isset($matches[1])){
								$needsave = true;
								$types[] = '"'.$iocname.'"=>array("type"=>"'.$matches[1].'","is_array"=>'.(isset($matches[2])?'true':'false').')';
							}
						}	
 					}
 				}
 			}
 			if ($needsave)	
				file_put_contents($filename, '<?php $ioctypes = array('.join(',', $types).'); ?>');
		} 		
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
 		if (key_exists($name,self::$_instances_))
 			return self::$_instances_[$name];
 		return null;
 	}
 	
  	private function _eval_ioc_array_members(array &$member, $type, $callback){
  		foreach ($member as &$m){
  			$obj = $this->GetIocMember($m, $type);
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
  	protected function setIocArrayMember(&$member,$value,$type,$callback = null){
  		$this->_eval_ioc_array_members($member, $type, $callback);
  		$m = null;
  		$this->setIocMember($m, $value, $type);
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
  	protected function getIocArrayMember(array &$member,$type,$callback = null,$last = false){
  		$this->_eval_ioc_array_members($member, $type, $callback);
  		if ($last)
  			if (!empty($member))
  				return $member[count($member) - 1];
  			else return null;	
  		return $this->$nm;
  	}
  	
  	private function _check_ioc_type($value,$type){
  		if (is_object($value) && is_string($type)){
  			return $value instanceof $type;
  		}
  		  		
  		if (($type == 'boolean') || ($type == 'integer') || ($type == 'float') || ($type == 'string') || ($type == 'array') || ($type == 'object'))
  			if (gettype($value) == $type)
  				return true;
  						
  		if (is_callable($type,false))
  			return call_user_func($type,$value);
  			
  		if (is_string($type))	
  			if (is_callable(array($this,$type)))
  				return call_user_func(array($this,$type),$value);
  				
  		if (is_array($type)){
  			$result = true;
			foreach ($type as $t){
				$result = $this->_check_ioc_type($value, $t);
				if (!$result) break;
			}	
			return $result;
  		}
  		
  		return false; 
  	}

/**
 * 
 * Assignes IOC dependency injection, which is evaluated if possible.
 * @param mixed $member reference to an object member holding a dependency
 * @param mixed $value assigned value 
 * @param string|array|callback $type an expression for type checks, can be a class or interface name, an array of names or a callback function
 * @throws TCoreException
 */  	
  	protected function setIocMember(&$member,$value,$type){
		if (is_string($value)){
			$member = $value;
			$value = self::Instance($value);
		}
		if ($value){
			if ($this->_check_ioc_type($value, $type))
				$member = $value;
			else  
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
	
	protected function getIocMember(&$member,$type){
		if (is_string($member)){
			$member = self::Instance($member);
			if (($member) && (!$this->_check_ioc_type($member, $type)))
				throw new TCoreException(TCoreException::ERR_BAD_TYPE);
		}
		return $member;	
	}
	
	private function _check_property($nm){
		$rc = new ReflectionClass(get_class($this));
		if ($rc->hasProperty($nm))
			return $rc->getProperty($nm);
		return false;
	}
 	
 	private function _check_ioc(&$nm,&$type,&$is_ioc_array){
 		$type = null;
 		$is_ioc_array = false;
		$iocname = '_ioc'.preg_replace_callback('/([A-Z][a-z\d]*)/',create_function('$matches','return "_".strtolower($matches[0]);'),$nm).'_';
 		if ($p = $this->_check_property($iocname)){
			$type = 'IConfigurable';
			if (isset(self::$_ioc_types_[get_class($this)]))
				if (isset(self::$_ioc_types_[get_class($this)][$iocname])){
					$type = self::$_ioc_types_[get_class($this)][$iocname]['type'];
					$is_ioc_array = self::$_ioc_types_[get_class($this)][$iocname]['is_array'];
				}
			$nm = $iocname;
			return true;
		}
		return false;		
 	}
	
 	/**
 	 * @ignore
 	 */
	public function __set($nm,$value){
		if ($this->_check_property($nm))
			$this->$nm = $value;
		else {	
			if ($this->_check_ioc($nm, $type, $is_ioc_array)){
				if ($is_ioc_array)
					$this->setIocArrayMember($this->$nm, $value, $type);
				else 		
					$this->setIocMember($this->$nm, $value, $type);
			} 
		}
	}
	
 	/**
 	 * @ignore
 	 */
	public function __get($nm){
		if ($this->_check_property($nm))
			return $this->$nm;
		if ($this->_check_ioc($nm, $type, $is_ioc_array)){
			if ($is_ioc_array)
				return $this->getIocArrayMember($this->$nm, $type);
			return $this->getIocMember($this->$nm, $type);	
		}
		return null;
	}
	
 	/**
 	 * @ignore
 	 */
	public function __isset($nm){
		if ($this->_check_property($nm))
			return isset($this->$nm);
		if ($this->_check_ioc($nm, $type, $is_ioc_array))
			return isset($this->$nm);
		return false;
	}
	
 	/**
 	 * @ignore
 	 */
	public function __unset($nm){
		if ($this->_check_property($nm))
			unset($this->$nm);
		else	
			if ($this->_check_ioc($nm, $type, $is_ioc_array))
				unset($this->$nm);
	}	
 }
?>