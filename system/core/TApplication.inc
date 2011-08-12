<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright &copy; 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
	require_once "TException.inc";
	require_once "utils/TFile.inc";
	require_once "utils/TDate.inc";
	require_once "utils/TOutputBuffer.inc";
	require_once "utils/TConvertions.inc";
	require_once "security/acl.inc";
	require_once "security/policy.inc";
	require_once "TConfigurable.inc";
	require_once "TServiceMap.inc";
	require_once "THttpRequest.inc";
	require_once "TService.inc";
	require_once "TI18n.inc";
	require_once "TApplicationSession.inc";
	
/**
 * Jawyl application is an instance of TApplication class.
 * This class provides an application skeleton which is used through all the runtime. 
 * Application serves for setting global system variables and paths, parsing Http requests to THttpRequest objects, using request routs when SEF enabled, 
 * and passing requests to instantiated services. TApplication inherits from TConfigurable.
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * @see TConfigurable
 * @see TApplication::Run()  
 * @see TApplication::import()
 * @see TService
 * @see THttpRequest
 * @see TServiceMap
 *    
 * @property-read string $RequestPath gets a local path to which current request refers 
 * @property-read string $AppPath gets a local path of application
 * @property-read string $URI gets a request URI
 * @property-read string $Root gets application path relative to host root
 * @property-read string $RelativePath gets a request path relative to TApplication::$Root
 * @property-read string $Host gets a request host name
 * 
 * @property-write string $Language sets current i18n and l10n language name;
 * @see TI18n::SetLanguage()
 * @see ILocale::SetLanguage()
 * @property-write string $Locale sets current l10n locale instance name 
 * @see ILocale
 * @see TLocale
 * @see TI18n::SetLocale()
 * 
 * @property-write string $Translator sets current i18n translator instance name
 * @see ITranslator
 * @see TTranslator
 * @see TI18n::SetTranslator()
 *  
 * @property-write string $SessionHandler sets current session handler by its name, when session is open setting this property leads to exception.
 * @see TApplicationSession
 * 
 * @property string $PrivateDir gets and sets application private directory relative to TApplication::$AppPath. defaults to "private". 
 * @property string $Shared gets and sets application public shared directory relative to TApplication::$Root, used to store client resources such as javascript and css. defaults to "shared".
 * @property string $Path gets and sets application import paths array in the form of semicolon delimited list
 * 
*/	
	final class TApplication extends TConfigurable {
		private $_request_path_;
/**
 * @var string stores local path where Jawyl framework is placed
*/
		public static $SystemPath;
		private $_app_path_;
		private $_private_dir_ = "private";
		private $_shared_ = "/shared";
		private $_uri_;
		private $_root_;
		private $_relative_path_;
		private $_ajax_engine_;
		private $_default_service_name_;
		private $_sef_;
		
		public static $CompileAppConfig = false;
		
/**
 * @var TApplication is a reference to TApplication skeleton
*/
		public static $Application;
		
		private $_service_map_;
		private $_current_service_;
		
		private static $_import_paths_ = array();
		
		private static $_request_;
		private static $_classes_ = array();
		private static $_services_ = array(); 
		
/**
* @ignore
* */		
		public function __get($nm){
			switch ($nm){
				case "RequestPath":return $this->_request_path_;break;
				case "AppPath":return $this->_app_path_;break;
				case "PrivateDir":return $this->_private_dir_;break;
				case "Shared":return $this->_shared_;break;
				case "URI":return $this->_uri_;break;
				case "Root":return $this->_root_;break;
				case "RelativePath":return $this->_relative_path_;break;
				case "Path":return join(';',self::$_import_paths_);break;
				case "Host":return $_SERVER["HTTP_HOST"];break;
				case "ConfigDir":return $this->_request_path_."/".$this->_private_dir_;break;
				case "ConfigFile":return $this->_request_path_."/".$this->_private_dir_."/application.xml";break;
				case "Sef":return $this->_sef_;break;
			}
			return parent::__get($nm);
		}
		
/**
* Application constructor. Called internally. Sets application paths and opens "application.xml" configuration file.
* @param string $requestPath
* @internal
* */		
		
		public final function __construct($requestPath){
			self::$Application = $this;
			self::$SystemPath = realpath(dirname(__FILE__)."/../");
			
			$this->_request_path_ = str_ireplace("\\","/",$requestPath);
			$this->_app_path_ = $this->_request_path_;
			$docroot = str_ireplace("\\","/",realpath($_SERVER["DOCUMENT_ROOT"]));
			$this->_root_ = str_ireplace($docroot,"",$this->_app_path_);
			
			
			if (!isset($_SERVER["REQUEST_URI"])){
				if (isset($_SERVER['HTTP_X_ORIGINAL_URL'])) 
					$this->_uri_ = $_SERVER['HTTP_X_ORIGINAL_URL'];
				// IIS Isapi_Rewrite
				else if (isset($_SERVER['HTTP_X_REWRITE_URL'])) 
					$this->_uri_ = $_SERVER['HTTP_X_REWRITE_URL'];
				else
				{
					// Use ORIG_PATH_INFO if there is no PATH_INFO
					if ( !isset($_SERVER['PATH_INFO']) && isset($_SERVER['ORIG_PATH_INFO']) )
						$_SERVER['PATH_INFO'] = $_SERVER['ORIG_PATH_INFO'];

					// Some IIS + PHP configurations puts the script-name in the path-info (No need to append it twice)
					if ( isset($_SERVER['PATH_INFO']) ) {
						if ( $_SERVER['PATH_INFO'] == $_SERVER['SCRIPT_NAME'] )
							$this->_uri_ = $_SERVER['PATH_INFO'];
						else
						$this->_uri_ = $_SERVER['SCRIPT_NAME'] . $_SERVER['PATH_INFO'];
					}

					// Append the query string if it exists and isn't null
					if (isset($_SERVER['QUERY_STRING']) && !empty($_SERVER['QUERY_STRING'])) {
						$this->_uri_ .= '?' . $_SERVER['QUERY_STRING'];
					}
				}
			}
			else
				$this->_uri_ = $_SERVER["REQUEST_URI"];				

			//$this->_uri_ = strtolower($this->_uri_);	
				
			$this->_relative_path_ = preg_replace('/\?.*$/','',str_ireplace($this->_root_,"",$this->_uri_));
					
			$config_file = $this->ConfigDir."/application.config";	
			if (self::$CompileAppConfig){
				$config = file_exists($config_file);
				$src = file_exists($this->ConfigFile);
			
				if (!$config){
					if ($src)
						$this->_compile_config();
					else throw new TCoreException(TCoreException::ERR_NO_APPCONF);	
				} else {
				 	if ($src)
				 		if (filemtime($config_file) < filemtime($this->ConfigFile))
				 			$this->_compile_config();
				}
			}	
		}
		
		
		private static function _parse_contents(DOMXPath $xpath, DOMNode $n, $container, &$result, &$counter){
			$nodes = $xpath->query("component|setting",$n);
			foreach ($nodes as $node){
				switch (strtolower($node->tagName)){
					case "component":{
							$counter++;
							if ($container)
								$result .= '$obj'.$counter.' = self::Instantiate("'.$node->getAttribute("classname").'",array("name"=>"'.$node->getAttribute("name").'","container"=>'.$container.'));'."\n";
							else 
								$result .= '$obj'.$counter.' = self::Instantiate("'.$node->getAttribute("classname").'",array("name"=>"'.$node->getAttribute("name").'"));'."\n";
							if ($node->getAttribute("membername"))	
								$result .= $container.'->__set("'.$node->getAttribute("membername").'",$obj'.$counter.');'."\n";
							self::_parse_contents($xpath,$node,'$obj'.$counter,$result,$counter);
					}break;
					case "setting":$result .= $container.'->'.$node->getAttribute("name").' = "'.$node->nodeValue.'";'."\n";break;
				}
			}
		}		
		
		private function _compile_config(){
			if (file_exists($this->ConfigFile)){
				$configXml = new DOMDocument();
				$configXml->preserveWhiteSpace = false;
				$configXml->Load($this->ConfigFile);
				if (!$configXml->schemaValidate(self::$SystemPath."/schemas/application.xsd"))
					throw new TCoreException(TCoreException::ERR_APPCONF_INCORRECT);
				parent::__construct($configXml->firstChild->getAttribute("name"));

				$app_config = '<?php'."\n";
				$xpath = new DOMXPath($configXml);
				$objid = 0;
				self::_parse_contents($xpath,$configXml->firstChild,'$this',$app_config,$objid);
				
				
				$default_service = false;
				$servicenodes = $xpath->query("/application/service");
				if ($servicenodes->length == 0) throw new TCoreException(TCoreException::ERR_SERVICE_NOT_FOUND);
				$one_service =  $servicenodes->length == 1;
				
				foreach ($servicenodes as $servicenode){
					$app_config .= 'self::$_services_["'.$servicenode->getAttribute("name").'"] = self::Instantiate("'.$servicenode->getAttribute("classname").'",array("name"=>"'.$servicenode->getAttribute("name").'"));'."\n";
					if (TConvertions::ConvertToBoolean($servicenode->getAttribute("default")) || $one_service)
						$default_service = $servicenode->getAttribute("name");
					$service_config = '<?php'."\n";
					self::_parse_contents($xpath,$servicenode,'self::$_services_["'.$servicenode->getAttribute("name").'"]',$service_config,$objid);
					$service_config .= '?>';
					file_put_contents($this->ConfigDir."/service.".$servicenode->getAttribute("name").".config",$service_config);
				}
				
				if ($default_service)
					$app_config .= '$this->_default_service_name_ = "'.$default_service.'";'."\n";
				$app_config .= '?>';	
					 
				file_put_contents($this->ConfigDir."/application.config",$app_config);
				
				$routs_config = '<?php $this->_service_map_ = new TServiceMap();'."\n";
				$routs = $xpath->query("/application/requestmap/route");
				foreach ($routs as $route){
					$regexp = $xpath->query("regexp",$route);
					if ($regexp->length > 0)
						$regexp = $regexp->item(0)->nodeValue;
					else
						$regexp = null;
					$request = $xpath->query("request",$route);
					if ($request->length > 0)
						$request = $request->item(0)->nodeValue;
					else
						$resquest = null;
					$service = $xpath->query("service",$route);
					if ($service->length > 0)
						$service = $service->item(0)->nodeValue;
					else
						$service = null;
					$routs_config .= '$this->_service_map_->AppendRoute("'.$regexp.'","'.$request.'","'.$service.'",array(';
					$parameters = $xpath->query("parameter",$route);
					$first = true;
					foreach ($parameters as $p){
						if (!$first) $routs_config .= ','; else $first = false; 
						$routs_config .= '"'.$p->getAttribute("name").'"=>"'.$p->nodeValue.'"';
					} 
					$routs_config .= '));'."\n";
				}
				$routs_config .= '?>';
				file_put_contents($this->ConfigDir."/url.config",$routs_config);
			}
		}
		
		private static function _check_path(&$path){
				$filename = null;
				if (is_file($path.".inc"))
					$filename = $path.".inc";
				else if (is_file($path.".php"))
					$filename = $path.".php";
				else if (is_dir($path))
					$filename = $path;
				if (!is_null($filename)) $path = $filename;
				return !is_null($filename);
		}

/**
 * Adds a path to application import paths
 * @param string $path a path to be added
 * @see TApplication::import()
*/					
		public static function AddImportPath($path){
			self::$_import_paths_[] = $path;
		}
		
/**
 * Imports a class or a library. Search is made relative to passed $root, then application import paths, then application private dir, then framework root dir.
 * @param string $classname a valid class or library name (with namespaces) to be imported 
 * @param string $root optional additional path relative to which class or library should be searched  
 * @see TApplication::AddImportPath()
*/					
		public static function import($classname,$root = null){
			$path = str_replace("::","/",$classname);
			$located = false;
			if (!is_null($root)){
				$filename = $root."/".$path;
				$located = self::_check_path($filename);
			}		

			if (!$located){
				$filename = self::$SystemPath."/".$path;
				$located = self::_check_path($filename);
			}			
			
			if (!$located){
				$filename = self::$Application->AppPath."/".self::$Application->PrivateDir."/code/".$path;
				$located = self::_check_path($filename);
			}
			
			if (!$located){
				foreach (self::$_import_paths_ as $ipath){
					$filename = $ipath."/".$path;	
					$located = self::_check_path($filename);
					if ($located) break;
				}
			}
			
			if (!$located) return;
			
			if (is_file($filename)){
				require_once $filename;
				$parts = explode("::",$classname);
				if (class_exists($parts[count($parts)-1]))
					self::$_classes_[$classname] = $parts[count($parts)-1];
			}
			else if (is_dir($filename)){
				$files = scandir($filename);
				foreach ($files as $file){
					if (is_file($file)){
						$parts = explode(".",$file);
						require_once $path."/".$file;
						if (class_exists($parts[0]))
							self::$_classes_[$classname."::".$parts[0]] = $parts[0];
					}
				}
			}
		}
/**
 * Gets a specified class Reflection object, if class does not exist tries to import it
 * @param string $classname a valid class name (with namespaces) 
 * @returns ReflectionClass   
 * @see TApplication::import()
*/					
		public static function GetClass($classname){
			$result = null;
			if (!key_exists($classname,self::$_classes_))
				self::import($classname);
			if (!key_exists($classname,self::$_classes_))
				return $result;
			$result = new ReflectionClass(self::$_classes_[$classname]);
			return $result;
		}

/**
 * Instantiates a specified class, if class does not exist tries to import it
 * @param string $classname a valid class name (with namespaces) 
 * @param array $arguments optional an array of arguments that should be passed to constructor 
 * @returns object  
 * @see TApplication::import()
 * @see TApplication::getClass()
*/					
		public static function Instantiate($classname,array $arguments = array()){
			$i = null;
			if (class_exists($classname))
				$c = new ReflectionClass($classname);
			else 
				$c = self::GetClass($classname);
			if ($c){	
				if (count($arguments) == 0)	
					$i = $c->newInstance();
				else {
					$i = $c->newInstanceArgs($arguments);
				}
			}
			return $i;
		}
		
/**
 * Gets a service with a specified name
 * @param string $name service name 
 * @returns TService  
 * @see TApplication::getService()
*/					
		public static function Service($name){
			return self::$Application->GetService($name);
		}
/**
 * Gets a service with a specified name
 * @param string $name service name 
 * @returns TService  
 * @see TApplication::Service()
*/					
		public function GetService($name){
			return self::$_services_[$name];
		}

/**
 * Makes a 301 redirect to a specified URL
 * @param string $url url where to redirect 
 */						
		public static function Redirect($url){
			TApplicationSession::Close();
			header("Location:".$url);
			die;
		} 
		
		private function _app_run(){ 
			include_once $this->ConfigDir."/application.config";
			$servicename = "";
			if (isset($_REQUEST["service"])) $servicename = $_REQUEST["service"]; 
			
			if ($this->Sef){
				include_once $this->ConfigDir."/url.config";
				self::$_request_ = $this->_service_map_->parseUri($this->RelativePath);
			}
			else 
				self::$_request_ = new THttpRequest($servicename);

			$servicename = self::$_request_->getService();
			if (!$servicename)
				$servicename = $this->_default_service_name_;
			if ($servicename){
				$this->_current_service_ = self::Service($servicename);
				if ($this->_current_service_){
					$service_config = $this->ConfigDir."/service.".$this->_current_service_->Name().".config";
					if (file_exists($service_config)) include_once $service_config; 
					$this->_current_service_->Handle(self::Request());	
				}
			} else {
				foreach (self::$_services_ as $service){
					$this->_current_service_ = self::Service($servicename);
					$service_config = $this->ConfigDir."/service.".$this->_current_service_->Name().".config";
					if (file_exists($service_config)) include_once $service_config; 
					if ($this->_current_service_->Handle(self::Request())) break;	
				}
			}	
			TApplicationSession::Close();
		}
		
/**
 * Gets current request URI, which is a base64 encoded serialization result of THttpRequest object 
 * @returns string
 * @see THttpRequest 
 */						
		public static function RequestURI(){
			if (self::$_request_)
				return self::$_request_->RequestURI();
			return false;
		}

/**
 * Get current request object copy
 * @return THttpRequest
 */		
		public static function Request(){
			return clone self::$_request_; 
		} 
		
/**
 * Gets current service 
 * @returns TService
 * @see TService 
 */						
		public static function CurrentService(){return self::$Application->_current_service_;}
		
/**
 * Constructs an url to request a specified service
 * @param string $service optional service name, if not specified current service is used 
 * @param array $parameters optional request parameters array  
 * @returns string
 * @see TService 
 */						
		public function MakeUrl($service = null,array $parameters = array()){
			$dr = preg_replace("/[\\/\\\\]/","|",realpath($_SERVER['DOCUMENT_ROOT']));
			$rp = preg_replace("/[\\/\\\\]/","|",realpath($this->RequestPath));		
			
			$pth = str_ireplace($dr,"",$rp);	
			$pth = str_replace("|","/",$pth);
			$url = "";
			if ($this->Sef){
				$notapplied = array();
				$url = $this->serviceMap->makeUri($service,$parameters,$notapplied);
				if (!is_null($url)){
					if (count($notapplied) > 0){
						$url .= "?";
						foreach ($notapplied as $key)
						 if ($parameters[$key])
							$url = $url."&".$key."=".urlencode($parameters[$key]);
					}
					return $pth.$url;
				}
			}
			
			$url = $pth;
			
			if ((!is_null($service)) || (count(array_values($parameters)) > 0))
				$url .= "/?";
			if (!is_null($service))
				$url .= "service=".urlencode($service);
				
			foreach ($parameters as $key=>$value)
				if ($value)
					$url = $url."&".$key."=".urlencode($value);
			return $url;
		}
		
/**
 * Static alias for TApplication::MakeUrl()
 * @param string $service service name 
 * @param array $parameters request parameters array  
 * @returns string
 * @see TApplication::MakeUrl() 
 */						
		public static function Url($service = null,array $parameters = array()){
			return self::$Application->MakeUrl($service,$parameters);
		}
		
		public static function FileStoragePath(){
   			$path = self::$Application->FileStoragePath;
   			if (!$path)
   				$path = TApplication::$Application->AppPath."/files";
   			if (!is_dir($path))
   				mkdir($path);
   			return $path;
		}
/**
* @ignore
* */		
		public function __set($nm,$value){
			switch ($nm){
				case "Shared":$this->_shared_ = preg_replace('/\A[\\\\\/]?(.*)[\\\\\/]?\z/','/$1',$value);break;
				case "Language":{TI18n::SetLanguage($value);parent::__set($nm,$value);}break;
				case "Locale":{TI18n::SetLocale($value);parent::__set($nm,$value);}break;
				case "Translator":{TI18n::SetTranslator($value);parent::__set($nm,$value);}break;
				case "SessionHandler":{TApplicationSession::SetHandler(TConfigurable::Instance($value));parent::__set($nm,$value);}break;
				case "Path":self::$_import_paths_ = explode(";",$value);break;
				case "PrivateDir":$this->_private_dir_ = $value;break;
				case "ImportPath":self::AddImportPath($value);break;
				case "Sef":$this->_sef_ = TConvertions::ConvertToBoolean($value);break;
				default:parent::__set($nm,$value);
			}
		}
		
/**
* Application main method, which launches runtime. Called once in the initial script. 
*/		
		public static function Run($callerPath){
			global $Application;
			$Application = new TApplication(realpath(dirname($callerPath)));
			$Application->_app_run();
		}
	}
?>