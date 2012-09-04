<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage SEF
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package CoreRuntimeClasses
 * @subpackage SEF
 * @category Core
 * A request rout class, used internally to parse request URL into a THttpRequest object.
 * Also it is used to convert request parameters to URL.
 * @see TApplication
 * @see TServiceMap
 * @see THttpRequest
 */
	final class TRequestRoute {
		private $_regexp_;
		private $_request_;
		private $_request_regexp_;
		private $_request_params_ = array();
		private $_service_;
		private $_defaults_ = array();
		
/**
 * class constructor
 * @param string $regexp regular expression for request parsing
 * @param string $request request pattern
 * @param string $service service name to wich route refers by default
 * @param array $defaults optional array of request parameters default values
*/
		
		public function __construct($regexp,$request,$service,array $defaults = array()){
			$this->_regexp_ = '/'.$regexp.'/';
			$this->_request_ = $request;
			$this->_service_ = $service;
			$this->_defaults_ = $defaults;
			
			preg_match_all("/\{:([^\}]*)\}/",$this->_request_,$prms);
			foreach ($prms[1] as $p)
				$this->_request_params_[] = $p;

			preg_match_all("/\[:([^\]]*)\]/",$this->_request_,$prms);
			foreach ($prms[1] as $p)
				$this->_request_params_[] = $p;
		}

/**
 * Constructs an URL from parameters. If count of passed parameters less then needed to construct an URL null is returned.
 * @param string $service name of service to which URL should refer
 * @param array $parameters array of request parameters that should be put into resulting URL
 * @param array $notapplied optional output array of parameters not processed by method
 * @return string|null 
*/
		public function MakeUri($service,array $parameters = array(), array &$notapplied  = array()){
			if (!in_array("service",$this->_request_params_)) 
				if ($service != $this->_service_) {
					return null;
				}
			
			if (count(array_diff($this->_request_params_,array_keys($parameters))) > 0)
				return null;

			$def = array_intersect(array_keys($this->_defaults_),array_keys($parameters));
			if (count($def) > 0){
				foreach ($def as $d)
					if ($this->_defaults_[$d] != $parameters[$d])
						return null;
			}
				
			$notapplied = array_diff(array_keys($parameters),array_merge($this->_request_params_,array_keys($this->_defaults_)));
			
			$result = $this->_request_;
			$result = preg_replace('/\{:service\}/',$service,$result);
			
			foreach ($parameters as $key=>$value){
				$result = preg_replace('/\{:\s*'.$key.'\s*\}/',$value,$result);
				$result = preg_replace('/\[:\s*'.$key.'\s*\]/',rawurlencode($value),$result);
			}
			return $result;
		}
		
/**
 * Parses URL into a THttpRequest object.
 * If URL can not be parsed into THttpRequest using route patterns method returns null.
 * @return THttpRequest
 * @see THttpRequest
*/			
		public function GetRequest($uri){
			preg_match($this->_regexp_,$uri,$params);
			if (count($params) > 0){
				$service = $this->_service_;
				$qp = array();
				
				for ($i = 1; $i < count($params); $i++)
				 	if ($this->_request_params_[$i-1] == "service")
				 		$service = $params[$i];
				 	else
				 		$qp[$this->_request_params_[$i-1]] = $params[$i];

				$request = new THttpRequest($service);
				foreach ($this->_defaults_ as $key=>$value)
					if (!$request->__isset($key))
						$request->setGetParameter($key,$value);
				foreach ($qp as $key=>$value)	
					$request->setGetParameter($key,urldecode($value));
				return $request;
			}
			return null;
		}
	}
	
/**
 * @package CoreRuntimeClasses
 * @subpackage SEF
 * @category Core
 * Final class that is used internally by application.
 * Implements SEF routing logic. Application loads sef routs from configuration file,
 * creates and adds TRequestRoute objects to TServiceMap.
 * @see TRequestRoute
 * @see TApplication  
 */
	final class TServiceMap {
		private $_routs_ = array();
/**
 * add a request route to service map.
 * @param string $regexp regular expression for request parsing
 * @param string $request request pattern
 * @param string $service service name to wich route refers by default
 * @param array $params optional array of request parameters default values
 * @see TRequestRoute::__construct()
 */		
		public function AppendRoute($regexp,$request,$service,array $params = array()){
			$this->_routs_[] = new TRequestRoute($regexp,$request,$service,$params);
		}
		
/**
 * Parses URL into a THttpRequest object.
 * If URL can not be parsed into THttpRequest using route patterns returns null.
 * @return THttpRequest
 * @see THttpRequest
*/			
		public function ParseUri($uri){
			$request = null;
			foreach ($this->_routs_ as $route){
				$request = $route->GetRequest($uri);
				if (!is_null($request)) break; 
			}
			return $request;	
		}
		
/**
 * Constructs an URL from parameters. If URL can not be constructed using routing patterns returns null. 
 * @param string $service name of service to which URL should refer
 * @param array $parameters array of request parameters that should be put into resulting URL
 * @param array $notapplied optional output array of parameters not processed by method
 * @return string|null 
*/		
		public function MakeUri($service,array $parameters = array(), array &$notapplied = array()){
			foreach ($this->_routs_ as $route){
				$uri = $route->MakeUri($service,$parameters,$notapplied);
				if (!is_null($uri)) return $uri;
			}
			return null;
		}
		
	}
?>