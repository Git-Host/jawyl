<?php
require_once 'core.inc';

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
 	
 	private $_application_ = null;
 	
 	private $_components_ = array();
 	
 	private static $_ioc_types_ = array();
 	
 	public static $CompileIocTypes = false;
 	
/**
 * class constructor. You need to pass it a unique name of configurable object.
 * @param string $name name of object being created
 */ 	
	public function __construct($name, IConfigurable $container = null, IApplication $application = null){
 		$this->name = $name;
 		$this->_application_ = $application;
 		$this->container = $container;
 		if ($this->container)
 			$this->container->RegisterComponent($this);
 		if (!isset(self::$_ioc_types_[get_class($this)])){
			$filename = $this->Application()->IocTypesCompileDir().'/'.get_class($this).'.ioc.php';
 			if (self::$CompileIocTypes)
 				self::CompileIocTypes(get_class($this),$filename,true);
 			if (file_exists($filename)){
 				require_once($filename);
 				//self::$_ioc_types_[get_class($this)] = $ioctypes;
 			}
 		}
 	}
 	
 	public function __destruct(){
 		$this->container = null;
 		foreach ($this->_components_ as $nm => $c) 
 			unset($this->_components_[$nm]);
 		unset($c);
 		unset($this->_components_);
 	}

 /**
  * @return TApplication
  * @see IConfigurable::Application()
  */	
 	public function Application(){
 		if ($this->_application_ instanceof IApplication)
 			return $this->_application_;
 		if ($this->container instanceof IConfigurable)
 			return $this->container->Application();
 		return null;
 	}
 	
 	public function RegisterComponent(IConfigurable $component){
 		$this->_components_[$component->Name()] = $component;
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
							preg_match('/@var\s+([\w_]*)(\[\s*\])?/',$dc,$matches);
							if (isset($matches[1])){
								$needsave = true;
								$types[$iocname] = '"'.$iocname.'"=>array("type"=>"'.$matches[1].'","is_array"=>'.(isset($matches[2])?'true':'false').')';
							}
						}	
 					}
 				}
 			}
 			if ($needsave){
 				TFileSystem::ForceDir(dirname($filename));	
				file_put_contents($filename, '<?php self::$_ioc_types_[get_class($this)] = array('.join(',', array_values($types)).'); ?>');
 			}
		} 		
 	}
 	
 	public function Name(){
 		return $this->name;
 	}
 	
 	protected function component($name,IConfigurable $exclude = null){
 		if (isset($this->_components_[$name]))	
 			return $this->_components_[$name];
 		$inst = null;	
 		foreach ($this->_components_ as $com)
 			if ($com !== $exclude)
 				if ($inst = $com->component($name,$exclude))
 					return $inst;
 		return $inst;		
 	}
 	
 	public function Locate($name,IConfigurable $exclude = null){
 		if ($name == $this->name)
 			return $this;
 		$inst = null;	
 		if ($inst = $this->component($name,$exclude))
 			return $inst;	
 		if ($this->container instanceof IConfigurable)
 			return $this->container->Locate($name,$this); 	
 		return null;
 	}
 	
 	
/**
 * @param string $name name of configurable object
 * @returns TConfigurable
 * @see TConfigurable
 */ 	
 	public function Instance($name){
 		return $this->Locate($name);
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
  		if (!is_array($member)) $member = array();
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
  		return $member;
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
			$value = $this->Instance($value);
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
			$member = $this->Instance($member);
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
 	
 	private function _check_ioc($nm,&$type,&$is_ioc_array){
 		$type = null;
 		$is_ioc_array = false;
		$iocname = '_ioc'.strtolower(preg_replace('/([A-Z][a-z\d]*)/','_$1',$nm)).'_';
 		if ($p = $this->_check_property($iocname)){
			$type = 'IConfigurable';
			if (isset(self::$_ioc_types_[get_class($this)]))
				if (isset(self::$_ioc_types_[get_class($this)][$iocname])){
					$type = self::$_ioc_types_[get_class($this)][$iocname]['type'];
					$is_ioc_array = self::$_ioc_types_[get_class($this)][$iocname]['is_array'];
				}
			return $iocname;
		}
		return false;		
 	}
 	
 	private function _check_suffix($nm,$suffix){
		$name = '_'.$suffix.strtolower(preg_replace('/([A-Z][a-z\d]*)/','_$1',$nm)).'_';
		if ($p = $this->_check_property($name))
			return $name;
		return false;
 	}
	
 	/**
 	 * @ignore
 	 */
	public function __set($nm,$value){
		if ($n = $this->_check_ioc($nm, $type, $is_ioc_array)){
			if ($is_ioc_array){
				if (is_array($value)){
					foreach ($value as $v)
						$this->setIocArrayMember($this->$n, $v, $type);
				} else
					$this->setIocArrayMember($this->$n, $value, $type);
			} else 		
				$this->setIocMember($this->$n, $value, $type);
		} else if ($n = $this->_check_suffix($nm,'json')){
			$this->$n = is_object($value)?$value:json_decode($value);	
		} else if ($n = $this->_check_suffix($nm,'bool')){
			$this->$n = TConvertions::ConvertToBoolean($value);
		} else if ($n = $this->_check_suffix($nm,'array')){
			$this->$n = TConvertions::ConvertToArray($value);
		} else if ($n = $this->_check_suffix($nm,'int')){
			$this->$n = (int)$value;
		} else if ($n = $this->_check_suffix($nm,'float')){
			$this->$n = (float)$value;
		} else if ($n = $this->_check_suffix($nm,'conf')){
			$this->$n = $value;
		}
	}
	
 	/**
 	 * @ignore
 	 */
	public function __get($nm){
		if ($n = $this->_check_ioc($nm, $type, $is_ioc_array)){
			if ($is_ioc_array)
				return $this->getIocArrayMember($this->$n, $type);
			return $this->getIocMember($this->$n, $type);	
		}
		if (($n = $this->_check_suffix($nm, 'json'))
			|| ($n = $this->_check_suffix($nm, 'bool'))
			|| ($n = $this->_check_suffix($nm, 'array'))
			|| ($n = $this->_check_suffix($nm, 'float'))
			|| ($n = $this->_check_suffix($nm, 'int'))
			|| ($n = $this->_check_suffix($nm, 'conf')))
			return $this->$n;
		if (isset($this->_components_[$nm]))
			return $this->_components_[$nm];
		return null;
	}
	
 	/**
 	 * @ignore
 	 */
	public function __isset($nm){
		if ($n = $this->_check_ioc($nm, $type, $is_ioc_array))
			return isset($this->$n);
		if ($n = $this->_check_suffix($nm, 'json'))
			return isset($this->$n);
		if ($n = $this->_check_suffix($nm, 'bool'))
			return isset($this->$n);
		return false;
	}
	
 	/**
 	 * @ignore
 	 */
	public function __unset($nm){
		if ($n = $this->_check_ioc($nm, $type, $is_ioc_array))
				unset($this->$n);
		else 
			if ($n = $this->_check_suffix($nm, 'json'))
				unset($this->$n);
		else
			if ($n = $this->_check_suffix($nm, 'bool'))
				unset($this->$n);
	}	
 }
?>