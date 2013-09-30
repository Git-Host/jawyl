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
 * @property string $Service Name of service to which request was made
 * @property string $RawPostData
 * 
 * @see TApplication
 */
	final class THttpRequest extends TRequest {
		const PM_GET = 'get';
		const PM_POST = 'post';
		const PM_COOKIE = 'cookie';
		
		private $_get_parameters_ = array();
		private $_post_parameters_ = array();
		private $_file_parameters_ = array();
		private $_raw_post_data_ = null;
		
		protected $origin = '';
/**
 * @var bool $IsXmlHttpRequest stores value indicating whether current request was sent by XmlHttpRequest object.
 */		
		public $IsXmlHttpRequest = false;
/**
 * @var bool $IsAjax stores value indicating whether current request is an ajax request.
 */		
		public $IsAjax = false;
		
		public $JsonObject = null;
		
		private function _accept_query_params($params,$urldecode,&$member,&$orig){
			foreach ($params as $key=>$value)
			  if ($key != "service"){
			  		$v = $this->_accept_value($value,$urldecode);
			  		if (is_array($member))
						$member[$key] = $v;
					if (!is_null($v))	
						$orig[$key] = $value;
			  }
		}
/**
 * @ignore
 */		
		public function __construct($service = null, IFileStorage $file_storage = null){
			parent::__construct($service,$file_storage);
			$this->IsXmlHttpRequest = strtolower($this->Header("X_REQUESTED_WITH")) == "xmlhttprequest";
			$this->IsAjax = $this->IsXmlHttpRequest;
			
			if (strtolower($this->Header('Content-Type')) == 'application/json')
				$this->JsonObject = json_decode($this->RawPostData());
			
			$orig = array();
			
			$this->_accept_query_params($_GET, true, $this->_get_parameters_, $orig);
			$this->_accept_query_params($_POST, false, $this->_post_parameters_, $orig);
				
			if ($this->fileStorage instanceof IFileStorage)
			{
				foreach ($_FILES as $key=>$file){
					if (is_array($file)/* && (($file['error'] == 0) || is_array($file['error']))*/){
						if (is_array($file["name"])){
							$files = array();
							$orig[$key] = '';
							foreach ($file["name"] as $i => $fname){
								$orig[$key] .= $fname.';';
								if ($file["error"][$i] == UPLOAD_ERR_OK)
									$files[$i] = new TUploadedFile($this->fileStorage,array(
										"name"=>$fname,
										"tmp_name"=>$file["tmp_name"][$i],
										"type"=>$file["type"][$i],
										"size"=>$file["size"][$i],
										"error"=>UPLOAD_ERR_OK
									));
								else 
									$files[$i] = new TUploadedFile($this->fileStorage,array(
										"error"=>$file["error"][$i]
									));
							}
							$this->_file_parameters_[$key] = $files;
						} else {
							$this->_file_parameters_[$key] = new TUploadedFile($this->fileStorage, $file);
							$orig[$key] = $file["name"];
						}
					}
				}
			}

			$t = null;
			$this->_accept_query_params($_COOKIE, false, $t, $orig);

			ksort($orig,SORT_STRING);	
			
			$this->origin = base64_encode(serialize($orig));
		}

/**
 * Gets current request URI, which is base64 encoded list of request url and parameters.
 */
		public function RequestURI(){
			return $this->origin;
		}
		
		private function _accept_value($value,$urldecode = false){
			if (get_magic_quotes_gpc()){
				if (is_array($value))
					array_walk_recursive($value, create_function('&$item,$key,$urldecode', '$item = stripslashes($urldecode?rawurldecode($item):$item);'), $urldecode);
				else 
        					$value = stripslashes($urldecode?rawurldecode($value):$value);
			}
			if (!is_null($value))		
				return $value;
			return null;	
		}
		
		public function ParameterMethod($name){
			if (isset($this->_get_parameters_[$name]))
				return self::PM_GET;
			if (isset($this->_post_parameters_[$name]) || isset($this->_file_parameters_[$name]))
				return self::PM_POST;
			if ($this->Cookie($name))
				return self::PM_COOKIE;
			return false; 
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
		
		public function RawPostData(){
			global $HTTP_RAW_POST_DATA;
			if (isset($HTTP_RAW_POST_DATA))
				return $HTTP_RAW_POST_DATA;
			if (is_null($this->_raw_post_data_))
				$this->_raw_post_data_ = file_get_contents("php://input");
			return $this->_raw_post_data_;	
		}
				
/**
 * gets request header
 * @param string $nm header name
 * @return string 
 */
		public function Header($nm){
			if (isset($_SERVER['HTTP_'.strtoupper($nm)]))
				return $_SERVER['HTTP_'.strtoupper($nm)];
			return null;
		}
		
/**
 * @ignore
 */		
		public function __set($nm,$value){
			if (strtolower($nm) == "service") return;
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
			if (isset($_COOKIE[$nm])){
				$v = $this->_accept_value($_COOKIE[$nm]);
				if ($v) return $v;
			}
			return null;
		}
/**
 * @ignore
 */		
		
		public function __get($nm){
			if ($nm == 'RawPostData')
				return $this->RawPostData();
			if (key_exists($nm, $this->_get_parameters_))
				return $this->_get_parameters_[$nm];
			if (key_exists($nm, $this->_post_parameters_))
				return $this->_post_parameters_[$nm];
			if (key_exists($nm, $this->_file_parameters_))
				return $this->_file_parameters_[$nm];
			if (key_exists($nm, $_COOKIE))
				return $this->_accept_value($_COOKIE[$nm]);	
			return parent::__get($nm);
		}	

/**
 * @ignore
 */		
		public function __isset($nm){
			return key_exists($nm, $this->_get_parameters_) || key_exists($nm, $this->_post_parameters_) || key_exists($nm, $this->_file_parameters_) || key_exists($nm, $_COOKIE) || parent::__isset($nm);
		}
/**
 * @ignore
 */		
		public function __unset($nm){
			if ($nm != "Service") {
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