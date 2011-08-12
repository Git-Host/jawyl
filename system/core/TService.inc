<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Service
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * interface for implementing services. 
 * Service is a top level handler of http requests in application.
 * After request is parsed by TApplication and THttpRequest instance is created, application creates a service object.
 * This object is created according to configuration file, where there is a list of named application services.
 * If request contains service name, then that service is instanciated, else application tries to get default service.
 * When service is instantiated application calls its Handle method and passes request object to it.
 * If no service is marked as default in configuration file, then application instantiates each service until request is handled.  
 * @package CoreRuntimeClasses
 * @subpackage Service
 * @category Core
 * @see TApplication
 * @see THttpRequest
 * @see IService::Handle()
 */
interface IService {
	/**
	 * gets service name
	 * @return string
	 */
	public function Name();
	/**
	 * service handle method. used for request processing.
	 * @param THttpRequest $request request object to process.
	 * @return boolean should return true when request is handled by service and false otherwise. 
	 */
	public function Handle(THttpRequest $request);
}

class TServiceRole extends TPolicy {
	const ADMIN = "serv.admin";
	const USER = "serv.user";
	const ANONYMOUS = "serv.anonym";
	
	protected function checkType($type){
		return in_array($type, array(self::ADMIN,self::USER,self::ANONYMOUS));
	}
	
	public function __construct($type){
		parent::__construct($type);
		if (TApplication::CurrentService() instanceof ISecurityObject)
			$this->sobj = TApplication::CurrentService();
	}
	
	public function Descenders(){
		$result = array();
		switch ($this->type){
			case self::USER:$result[] = new TServiceRole(self::ADMIN);break;
			case self::ANONYMOUS:$result[] = new TServiceRole(self::USER);break;
		} 
		return $result;
	}	
}

/**
 * Interface for implementing services that use application security objects.
 * When implemented should provide access to service ACL and policy components.   
 * @package CoreRuntimeClasses
 * @subpackage Service
 * @category Core
 * @see TApplication
 * @see IACLManager
 * @see IPolicyManager
 */
interface ISecuredService extends IService {
/**
 * gets service ACL component
 * @return IACLManager
 */
		public function ACL();
/**
 * gets service policy component
 * @return IPolicyManager
 */
		public function Policy();
}

/**
 * Abstract service class. Can be used to derive custom service classes.
 * In addition to basic TConfigurable settings logic, TService derives settings from application object.
 * So if you define some setting for the application, you can read it as a property of service.
 * For example you can specify Policy property setting for application, and all services would use the same Policy component. 
 * Note that setting service property has no effect on application properties, 
 * as in this case property value is set for service object.   
 * 
 * @property string $Policy policy component name
 * @property string $ACL acl component name
 *    
 * @package CoreRuntimeClasses
 * @subpackage Service
 * @category Core
 * @uses TConfigurable
 * @uses ISecuredService
 */
abstract class TService extends TConfigurable implements ISecuredService, ISecurityObject {
	protected $request;
	
/**
 * gets service ACL component, which name is determined by service ACL property
 * @see ISecuredService::ACL()
 * @return IACLManager
 */	
	public function ACL(){
		$a = TApplication::Instance($this->ACL);
		if (!($a instanceof IACLManager))
			throw new TCoreException(TCoreException::ERR_BAD_TYPE);
		return $a;
	}
/**
 * gets service policy component, which name is determined by service Policy property
 * @see ISecuredService::Policy()
 */	
	public function Policy(){
		$p = TApplication::Instance($this->Policy);
		if (!($p instanceof IPolicyManager))
			throw new TCoreException(TCoreException::ERR_BAD_TYPE);
		return $p;
	}
	
	public function Soid(){
		return TApplication::$Application->Name()."::".$this->Name();
	}
/**
 * @ignore
 */	
	public function __set($nm,$value){
		switch ($nm){
			case "Language":{TI18n::SetLanguage($value);parent::__set($nm,$value);}break;
			case "Locale":{TI18n::SetLocale($value);parent::__set($nm,$value);}break;
			case "Translator":{TI18n::SetTranslator($value);parent::__set($nm,$value);}break;
			default:parent::__set($nm,$value);
		}
	}
/**
 * @ignore
 */	
	public function __get($nm){
		if ($nm == "Request")
			return clone $this->request;
		$result = parent::__get($nm);
		if (!$result)
			$result = TApplication::$Application->__get($nm);	
		return $result;
	}
	
/**
 * @ignore
 */	
	public function __isset($nm){
		$result = parent::__isset($nm);
		if (!$result) $result = TApplication::$Application->__isset($this->Name().".".$nm);
		if (!$result) $result = TApplication::$Application->__isset($nm);
		return $result; 
	}
/**
 * gets service name
 * @return string
 * @see IService::Name()
 */	
	public function Name(){return $this->name;}

/**
 * public constructor
 */
	public function __construct($name){
		parent::__construct($name);
	}
	
	public function Handle(THttpRequest $request){
		$this->request = $request;
	}
}
?>