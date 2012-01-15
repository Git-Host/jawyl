<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * THttpRequest incapsulates data associated with current http request. 
 * The class is final as its is instanciated by application at runtime.
 * Then instance is passed to services for handling. 
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * 
 * @property-read string $service Name of service to which request was made
 * @property mixed $request_param_name You can access request parameters as THttpRequest properties
 * 
 * @see TApplication
 */
	final class THttpRequest {
		private $_get_parameters_ = array();
		private $_post_parameters_ = array();
		private $_file_parameters_ = array();
		private $_service_;
/**
 * @var bool $IsXmlHttpRequest stores value indicating whether current request was sent by XmlHttpRequest object.
 */		
		public $IsXmlHttpRequest = false;
/**
 * @var bool $IsAjax stores value indicating whether current request is an ajax request.
 */		
		public $IsAjax = false;
		
/**
 * @ignore
 */		
		public function __construct($service){
			$this->_service_ = $service;
			$this->IsXmlHttpRequest = strtolower($this->Header("HTTP_X_REQUESTED_WITH")) == "xmlhttprequest";
			$this->IsAjax = $this->IsXmlHttpRequest;
			
			$orig = array();
			
			foreach ($_GET as $key=>$value)
			  if ($key != "service"){
				$this->_get_parameters_[$key] = $this->_accept_value($value,true);
				$orig[$key] = $value;
			  }
				
			foreach ($_POST as $key=>$value)
			  if ($key != "service"){
				$this->_post_parameters_[$key] = $this->_accept_value($value);
				$orig[$key] = $value;
			  }
				
			foreach ($_FILES as $key=>$file){
				$this->$this->_file_parameters_[$key] = new TUploadedFile($file);
				$orig[$key] = $file["name"];
			}
			
			foreach ($_COOKIE as $key=>$value)
				$orig[$key] = $this->_accept_value($value);

			ksort($orig,SORT_STRING);	
			
			$this->origin = base64_encode(serialize($orig));
		}

/**
 * Gets current request URI, which is base64 encoded list of request url and parameters.
 */
		public function RequestURI(){
			return $this->origin;
		}
		
/**
 * Gets name of service to which request is made. 
 */
		public function getService(){return $this->_service_;}
		
		private function _accept_value($value,$urldecode = false){
			if (get_magic_quotes_gpc()){
				if (is_array($value))
					array_walk($value, create_function('&$item,$key,$urldecode', '$item = stripslashes($urldecode?urldecode($item):$item);'), $urldecode);
				else 
        			$value = stripslashes($urldecode?urldecode($value):$value);
			}		
			return $value;
		}
		
/**
 * Sets request GET parameter.
 * @param string $name parameter name
 * @param mixed $value parameter value 
 */
		public function SetGetParameter($name,$value){
			$this->_get_parameters_[$name] = $value;
		}
		
/**
 * sets request POST parameter
 * @param string $name parameter name
 * @param mixed $value parameter value 
 */
		public function SetPostParameter($name,$value){
			$this->_post_parameters_[$name] = $this->$value;
		}
				
/**
 * gets request header
 * @param string $nm header name
 * @return string 
 */
		public function Header($nm){
			if (isset($_SERVER[strtoupper($nm)]))
				return $_SERVER[strtoupper($nm)];
			return null;
		}
		
/**
 * @ignore
 */		
		public function __set($nm,$value){
			if ($nm == "service") return;
			$this->SetGetParameter($nm, $value);
		}
/**
 * gets GET parameters associative array
 */		
		public function GetParameters(){
			return $this->_get_parameters_;
		}
		
/**
 * gets POST parameters associative array
 */		
		public function PostParameters(){
			return $this->_post_parameters_;
		}
		
/**
 * gets FILE parameter values associative array. array members are TUploadedFile instances.
 * @return TUploadedFile[]
 * @see TUploadedFile
 */		
		public function FileParameters(){
			return $this->_file_parameters_;
		}
		
/**
 * gets all parameter values associative array
 */		
		public function Parameters(){
			return array_merge(array_merge($this->_post_parameters_,$this->_get_parameters_),$this->_file_parameters_);
		}
		
		public function Cookie($nm){
			if (isset($_COOKIE[$nm]))
				return $this->_accept_value($_COOKIE[$nm]);
			return null;
		}
/**
 * @ignore
 */		
		
		public function __get($nm){
			if ($nm == "service") return $this->_service_;
			if (isset($this->_get_parameters_[$nm]))
				return $this->_get_parameters_[$nm];
			if (isset($this->_post_parameters_[$nm]))
				return $this->_post_parameters_[$nm];
			if (isset($this->_file_parameters_[$nm]))
				return $this->_file_parameters_[$nm];
			if (isset($_COOKIE[$nm]))
				return $this->_accept_value($_COOKIE[$nm]);	
			return null;
		}	

/**
 * @ignore
 */		
		public function __isset($nm){
			if ($nm == "service") return !is_null($this->_service_);
			else return (isset($this->_get_parameters_[$nm]) || isset($this->_post_parameters_[$nm]) || isset($this->_file_parameters_[$nm]) || isset($_COOKIE[$nm]));
			return false;
		}
/**
 * @ignore
 */		
		public function __unset($nm){
			if ($nm != "service") {
				if (isset($this->_get_parameters_[$nm]))
					unset($this->_get_parameters_[$nm]);
				if (isset($this->_post_parameters_[$nm]))
					unset($this->_post_parameters_[$nm]);
				if (isset($this->_file_parameters_[$nm]))
					unset($this->_file_parameters_[$nm]);
			}
		}
	}
?>