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
	require_once 'core.inc';
	require_once 'TException.inc';
	require_once 'utils/TFileSystem.inc';	
	require_once 'utils/TFile.inc';
	require_once 'utils/TDate.inc';
	require_once 'utils/TOutputBuffer.inc';
	require_once 'utils/TConvertions.inc';
	require_once 'security/acl.inc';
	require_once 'security/policy.inc';
	require_once 'TConfigurable.inc';
	require_once 'TBuiltinExceptionHandler.inc';
	require_once 'TServiceMap.inc';
	require_once 'TRequest.inc';
	require_once 'TConsoleCall.inc';
	require_once 'THttpRequest.inc';
	require_once 'TService.inc';
	require_once 'TI18n.inc';
	require_once 'session.inc';
	
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
 * @property-read string $ConfCompileDir gets directory where application compiled configuration files are stored
 * @property-read string $IocTypesCompileDir gets directory where application compiled configuration files are stored
 * @property-read int $CallType 
 * 
 * @property string $Language sets current i18n and l10n language name;
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
 * 
 * @property string $PrivateDir gets and sets application private directory relative to TApplication::$AppPath. defaults to "private". 
 * @property string $Shared gets and sets application public shared directory relative to TApplication::$Root, used to store client resources such as javascript and css. defaults to "shared".
 * @property string $Path gets and sets application import paths array in the form of semicolon delimited list
 * @property string $ConfigDir
 * @property string $OverrideDir
 * 
*/	
	final class TApplication extends TConfigurable implements IApplication, IFileStorage {
		const ENV_PRODUCTION = 0;
		const ENV_TESTING = 1;
		const ENV_DEVELOPING = 2;
		
		const CALL_HTTP = 0;
		const CALL_CONSOLE = 1;
				
		private $_request_path_;
/**
 * @var string stores local path where Jawyl framework is placed
 */
		public static $SystemPath;

/**
 * @var string
 */		
		private $_app_path_;
		
/**
 * @var string
 */		
		private $_prev_private_dir_;
/**
 * @var string
 */		
		private $_prev_override_dir_;

/**
 * @var string
 */
		private $_prev_ext_dir_;
		
/**
 * @var string
 */		
		private $_private_dir_;
/**
 * @var string
 */		
		private $_override_dir_;

/**
 * @var string
 */
		private $_ext_dir_;		
		
/**
 * @var string
 */		
		private $_prev_configcompile_dir_;
/**
 * @var string
 */		
		private $_prev_ioccompile_dir_;
		
/**
 * @var string
 */		
		private $_configcompile_dir_;
/**
 * @var string
 */		
		private $_ioccompile_dir_;
		
/**
 * @var string
 */		
		private $_shared_ = '/shared';
/**
 * @var string
 */		
		private $_uri_;
/**
 * @var string
 */		
		private $_root_;
/**
 * @var string
 */		
		private $_relative_path_;

/**
 * @var string
 */		
		private $_default_service_name_;
/**
 * @var bool
 */		
		private $_sef_;
/**
 * @var string
 */		
		private $_string_encoding_ = 'UTF-8';
/**
 * @var int
 */		
		private $_call_type_;
/**
 * @var string
 */		
		private $_file_storage_path_;
		
		private static $_runtime_successfull_ = false;
		
/**
 * @var IExceptionHandler
 */		
		protected $_ioc_exception_handler_;
		
/**
 * @var IClassLoader[]
 */		
		protected $_ioc_class_loader_ = array();
		
/**
 * @var bool
 */		
		public static $CompileAppConfig = false;
/**
 * @var bool
 */		
		public static $ErrorsCapture = true;
/**
 * @var string
 */		
		public static $ConfigCompileDirectory = 'config';
/**
 * @var string
 */		
		public static $IocTypesCompileDirectory = 'private/ioc';
/**
 * @var string
 */		
		public static $PrivateDirectory = 'private';
/**
 * @var string
 */		
		public static $OverrideDirectory = 'override';
		
/**
 * @var string
 */
		public static $ExtensionsDirectory = 'private';
		
/**
 * @var int
 */		
		public static $Environment = self::ENV_PRODUCTION;
		
/**
 * @var TApplication is a reference to TApplication skeleton
 */
		public static $Application;
		
/**
 * @var TServiceMap
 */		
		private $_service_map_;
		
/**
 * @var IService
 */		
		private $_current_service_;

/**
 * @var ISessionHandler
 */		
		private $_session_;
/**
 * @var IOutputBuffer
 */		
		private $_buffer_;
		
/**
 * @var TRequest
 */
		private static $_request_;
		private static $_services_ = array();
		
		private $_lang_;
		
/**
* @ignore
* */		
		public function __get($nm){
			switch ($nm){
				case 'Language':return $this->_lang_;break;
				case 'SessionHandler':return $this->Session();break;
				case 'SysPath':return $this->SysPath();break;
				case 'RequestPath':return $this->_request_path_;break;
				case 'AppPath':return $this->_app_path_;break;
				case 'PrivateDir':{
					if ($this->_prev_private_dir_ != self::$PrivateDirectory){
						$this->_private_dir_ = $this->AdjustPath(self::$PrivateDirectory);
						$this->_prev_private_dir_ = self::$PrivateDirectory;
					}
					return $this->_private_dir_;
				}break;
				case 'OverrideDir':{
					if ($this->_prev_override_dir_ != self::$OverrideDirectory){
						$this->_override_dir_ = $this->AdjustPath(self::$OverrideDirectory);
						$this->_prev_override_dir_ = self::$OverrideDirectory;
					}
						
					return $this->_override_dir_;
				}break;
				case 'ExtensionsDir':{
					if ($this->_prev_ext_dir_ != self::$ExtensionsDirectory){
						$this->_ext_dir_ = $this->AdjustPath(self::$ExtensionsDirectory);
						$this->_prev_ext_dir_ = self::$ExtensionsDirectory;
					}				
					return $this->_ext_dir_;
				}break;
				case 'FileStoragePath':return $this->_file_storage_path_;break;
				case 'Shared':return $this->_shared_;break;
				case 'URI':return $this->_uri_;break;
				case 'Root':return $this->_root_;break;
				case 'RelativePath':return $this->_relative_path_;break;
				case 'Path':return get_include_path();break;
				case 'Host':return $_SERVER['HTTP_HOST'];break;
				case 'Port':return $_SERVER['SERVER_PORT'];break;
				case 'Domain':return $this->Host.($this->Port?(($this->Port != '80')?':'.$this->Port:''):'');
				case 'ConfCompileDir':{
					if ($this->_prev_configcompile_dir_ != self::$ConfigCompileDirectory){
						$this->_configcompile_dir_ = $this->AdjustPath(self::$ConfigCompileDirectory);
						$this->_prev_configcompile_dir_ = self::$ConfigCompileDirectory;
					}
					return $this->_configcompile_dir_;
				}break;
				case 'IocTypesCompileDir':{
					if ($this->_prev_ioccompile_dir_ != self::$IocTypesCompileDirectory){
						$this->_ioccompile_dir_ = $this->AdjustPath(self::$IocTypesCompileDirectory);
						$this->_prev_ioccompile_dir_ = self::$IocTypesCompileDirectory;
					}
					return $this->_ioccompile_dir_;
				}break;
				case 'ConfigFile':return $this->PrivateDir.'/application.xml';break;
				case 'Sef':return $this->_sef_;break;
				case 'Environment':return self::$Environment;break;
				case 'StringEncoding':return $this->_string_encoding_;break;
				case 'CallType':return $this->_call_type_;break;
			}
			return parent::__get($nm);
		}
				
/**
* Application constructor. Called internally. Sets application paths and opens "application.xml" configuration file.
* @param string $requestPath
* @internal
* */		
		
		public final function __construct($requestPath){
			mb_internal_encoding($this->_string_encoding_);
			self::$Application = $this;
			self::$SystemPath = str_replace('\\','/',realpath(dirname(__FILE__).'/../'));
			$this->_ioc_exception_handler_ = new TBuiltinExceptionHandler($this);
			$this->_buffer_ = new TOutputBuffer();
			$this->_request_path_ = str_replace('\\','/',$requestPath);
			$this->_app_path_ = $this->_request_path_;
			$docroot = str_ireplace('\\','/',realpath($_SERVER['DOCUMENT_ROOT']));
			$this->_root_ = str_ireplace($docroot,'',$this->_app_path_);			
			
			if (!isset($_SERVER['REQUEST_URI'])){
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
				$this->_uri_ = $_SERVER['REQUEST_URI'];

			$this->_relative_path_ = preg_replace('/\?.*$/','',str_ireplace($this->_root_,'',$this->_uri_));
			if (!$this->_root_) $this->_root_ = '/';

			parent::__construct('');
		}
		
/**
 * @return string
 */		
		public function Uri(){
			return $this->_uri_;
		}
		
		public function Host(){return $this->Host;}
		
		public function AppPath(){return $this->AppPath;}
		
		public function PrivateDir(){return $this->PrivateDir;}
		
		public function ExtensionsDir(){return $this->ExtensionsDir;}
		
		public function IocTypesCompileDir(){return $this->IocTypesCompileDir;}
		
		public function ConfCompileDir(){return $this->ConfCompileDir;}
		
		public function OverrideDir(){return $this->OverrideDir;}
		
		public function Sef(){return $this->Sef;}
		
		public function Shared(){return $this->Shared;}
		
		public function Root(){return $this->Root;}

/**
 * @return IOutputBuffer
 */		
		public function Buffer(){
			return $this->_buffer_;
		}

/**
 * @return ISessionHandler
 */
		public function Session(){
			if (!$this->_session_ instanceof ISessionHandler)
				$this->_session_ = new TSessionHandler('default_session_handler',$this,$this);
			return $this->_session_;	
		}
		
		public function SysPath(){
			return self::$SystemPath;
		}
		
		public function Application(){
			return $this;
		}
		
		private static function _parse_contents(DOMXPath $xpath, DOMNode $n, $container, &$result, &$counter){
			$nodes = $xpath->query('component|setting',$n);
			foreach ($nodes as $node){
				switch (strtolower($node->tagName)){
					case 'component':{
							$counter++;
							if ($node->getAttribute('classname')){
								if ($container)
									$result .= '$obj'.$counter.' = $this->Instantiate("'.$node->getAttribute('classname').'",array("name"=>"'.$node->getAttribute('name').'","container"=>'.$container.'));'."\n";
								else 
									$result .= '$obj'.$counter.' = $this->Instantiate("'.$node->getAttribute('classname').'",array("name"=>"'.$node->getAttribute('name').'"));'."\n";
							} else {
								$result .= '$obj'.$counter.' = $this->Instance("'.$node->getAttribute('name').'");'."\n";
							}
							if ($node->getAttribute('membername'))	
								$result .= $container.'->__set("'.$node->getAttribute('membername').'",$obj'.$counter.');'."\n";
							self::_parse_contents($xpath,$node,'$obj'.$counter,$result,$counter);
					}break;
					case 'setting':$result .= $container.'->'.$node->getAttribute('name').' = "'.addcslashes($node->nodeValue,'"').'";'."\n";break;
				}
			}
		}
		
		private function _compile_init_str($inputfile, &$objid){
			$app_config = '';
			if (file_exists($inputfile)){
				$configXml = new DOMDocument();
				$configXml->preserveWhiteSpace = false;
				$configXml->Load($inputfile);
				if (!$configXml->schemaValidate(self::$SystemPath.'/schemas/init.xsd'))
					throw new TCoreException(TCoreException::ERR_APPCONF_INCORRECT);
				$xpath = new DOMXPath($configXml);
				if (!$objid) $objid = 0;
				self::_parse_contents($xpath,$configXml->firstChild,'$this',$app_config,$objid);
			}
			return $app_config;
		}

		private function _compile_init($inputfile,$outputfile){
			if (file_exists($inputfile)){
				$app_config = '<?php'."\n";
				$app_config .= $this->_compile_init_str($inputfile,$objid);
				if (file_exists($this->OverrideDir.'/'.basename($inputfile)))
					$app_config .= $this->_compile_init_str($this->OverrideDir.'/'.basename($inputfile),$objid);
				$app_config .= '?>';
				file_put_contents($outputfile,$app_config);
			}
		}
		
		
		private function _check_need_plugin_compile($indir,$outdir){
			$files = TFileSystem::ListFiles($indir);
			$need_compile = false;
			
			foreach ($files as $f){
				if (is_dir($f['dirname'].'/'.$f['basename'])){
					$need_compile = $this->_check_need_plugin_compile($f['dirname'].'/'.$f['basename'], $outdir);
					if ($need_compile) break;
				} else if (isset($f['extension']) && $f['extension'] == 'xml'){
					$restime = 0;
					if (file_exists($outdir.'/'.$f['filename'].'.php'))
						$restime = filemtime($outdir.'/'.$f['filename'].'.php');
					$need_compile = filemtime($f['dirname'].'/'.$f['basename']) > $restime;
					if ($need_compile) break;
				}
			}
			
			return $need_compile;	
		}
		
		
		private function _compile_plugin($indir,array &$confs){
			if (file_exists($indir)){
				$files = TFileSystem::ListFiles($indir);
				foreach ($files as $f){
					if (isset($f['extension']) && $f['extension'] == 'xml'){
						if (!isset($confs[$f['filename'].'.php']))
							$confs[$f['filename'].'.php'] = '<?php'."\n";
						$confs[$f['filename'].'.php'] .= $this->_compile_init_str($f['dirname'].'/'.$f['basename'],$objid);
					}		
				}		
			}			
		}
		
		private function _compile_plugins($indir,$outdir,$service = null){
			if (file_exists($indir) && file_exists($outdir) && $this->_check_need_plugin_compile($indir, $outdir)){				
				$files = TFileSystem::ListFiles($indir);
				$confs = array();
				foreach ($files as $f)
					if (is_dir($f['dirname'].'/'.$f['basename']))
						$this->_compile_plugin($f['dirname'].'/'.$f['basename'],$confs);
				foreach ($confs as $fn => $text)
					file_put_contents($outdir.'/'.$fn, $text);
			}
		}
		
		
		private function _compile_routes($file){
			if (file_exists($this->OverrideDir.'/'.basename($file)))
				$file = $this->OverrideDir.'/'.basename($file);
			
			if (file_exists($file)){
				$configXml = new DOMDocument();
				$configXml->preserveWhiteSpace = false;
				$configXml->Load($file);
			
				if (!$configXml->schemaValidate(self::$SystemPath.'/schemas/routes.xsd'))
					throw new TCoreException(TCoreException::ERR_APPCONF_INCORRECT);
					
				$xpath = new DOMXPath($configXml);
				$routs_config = '<?php $this->_service_map_ = new TServiceMap();'."\n";
				$routs = $xpath->query('/requestmap/route');
				foreach ($routs as $route){
					$regexp = $xpath->query('regexp',$route);
					if ($regexp->length > 0)
						$regexp = $regexp->item(0)->nodeValue;
					else
						$regexp = null;
					$request = $xpath->query('request',$route);
					if ($request->length > 0)
						$request = $request->item(0)->nodeValue;
					else
						$resquest = null;
					$service = $xpath->query('service',$route);
					if ($service->length > 0)
						$service = $service->item(0)->nodeValue;
					else
						$service = null;
					$routs_config .= '$this->_service_map_->AppendRoute("'.$regexp.'","'.$request.'","'.$service.'",array(';
					$parameters = $xpath->query('parameter',$route);
					$first = true;
					foreach ($parameters as $p){
						if (!$first) $routs_config .= ','; else $first = false;
						$routs_config .= '"'.$p->getAttribute('name').'"=>"'.$p->nodeValue.'"';
					}
					$routs_config .= '));'."\n";
				}
				$routs_config .= '?>';
				file_put_contents($this->ConfCompileDir.'/url.php',$routs_config);
			}
		}
		
		private function _compile_config($file){
			if (file_exists($file)){
				$configXml = new DOMDocument();
				$configXml->preserveWhiteSpace = false;
				$configXml->Load($file);
				
				if (!$configXml->schemaValidate(self::$SystemPath.'/schemas/application.xsd'))
					throw new TCoreException(TCoreException::ERR_APPCONF_INCORRECT);
					
				$app_config = '<?php'."\n";
				$app_config .= '$this->name = "'.$configXml->firstChild->getAttribute('name').'";';
				$xpath = new DOMXPath($configXml);
				$objid = 0;
				self::_parse_contents($xpath,$configXml->firstChild,'$this',$app_config,$objid);
				
				
				$default_service = false;
				$servicenodes = $xpath->query('/application/service');
				if ($servicenodes->length == 0) throw new TCoreException(TCoreException::ERR_SERVICE_NOT_FOUND);
				$one_service =  $servicenodes->length == 1;
				$sconfigXml = new DOMDocument();
				$sconfigXml->preserveWhiteSpace = false;
				
				foreach ($servicenodes as $servicenode){
					$app_config .= 'self::$_services_["'.$servicenode->getAttribute('name').'"] = "'.$servicenode->getAttribute('classname').'";'."\n";
					if (TConvertions::ConvertToBoolean($servicenode->getAttribute('default')) || $one_service)
						$default_service = $servicenode->getAttribute('name');
					$service_config = '<?php'."\n";
					self::_parse_contents($xpath,$servicenode,'self::$_services_["'.$servicenode->getAttribute('name').'"]',$service_config,$objid);
					$serv_override_file = $this->OverrideDir.'/'.$servicenode->getAttribute('name').'.service.xml';
					if (file_exists($serv_override_file)){
						$sconfigXml->Load($serv_override_file);
						$sxpath = new DOMXPath($sconfigXml);
						self::_parse_contents($sxpath,$sconfigXml->firstChild,'self::$_services_["'.$servicenode->getAttribute('name').'"]',$service_config,$objid);
					}
					$service_config .= '?>';
					file_put_contents($this->ConfCompileDir.'/service.'.$servicenode->getAttribute('name').'.php',$service_config);
				}
				
				if ($default_service)
					$app_config .= '$this->_default_service_name_ = "'.$default_service.'";'."\n";
				$app_config .= '?>';	
					 
				file_put_contents($this->ConfCompileDir.'/application.php',$app_config);				
			}
		}
		
		private static function _check_path(&$path){
				$filename = null;
				if (file_exists($path.'.inc'))
					$filename = $path.'.inc';
				else if (file_exists($path.'.php'))
					$filename = $path.'.php';
				else if (is_dir($path) && file_exists($path))
					$filename = $path;
				if (!is_null($filename)) $path = $filename;
				return !is_null($filename);
		}

/**
 * Adds a path to application import paths
 * @param string $path a path to be added
 * @see TApplication::Import()
*/					
		public function AddImportPath($path){
			set_include_path(get_include_path().PATH_SEPARATOR.$this->AdjustPath($path));
		}
		
		private function _canonical_path($path){
			$path = preg_replace('/[\\\\\\/]\.[\\\\\\/]/','/',$path);
			$path = preg_replace('/[\\\\\\/][\\\\\\/]+/','/',$path);
			while (preg_match('/[\\\\\\/]\.\.[\\\\\\/]/',$path) > 0)
				$path = preg_replace('/[\\\\\\/][^\\\\\\/]+[\\\\\\/]\.\.[\\\\\\/]/','/',$path);
			return $path;
		}
		
		public function AdjustPath($path){
			$trimmed = trim($path);
			if (preg_match('/^(https?:\/\/|\/|[a-zA-Z]:\\\\).*$/',$trimmed))
				return $this->_canonical_path($trimmed);
			return $this->_canonical_path($this->_app_path_.'/'.$trimmed);
		}
		
/**
 * Gets a specified class Reflection object, if class does not exist tries to import it
 * @param string $classname a valid class name (with namespaces) 
 * @returns ReflectionClass   
 * @see TApplication::import()
*/					
		public function GetClass($classname){
			$result = null;	
			if (!self::_check_class($classname))	
				self::_autoloader($classname);
			$result = new ReflectionClass(self::_check_class($classname));
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
		public function Instantiate($classname,array $arguments = array()){
			$i = null;
				
			if ($c = $this->GetClass($classname)){
				if (count($arguments) == 0)	
					$i = $c->newInstance();
				else {
					$i = $c->newInstanceArgs($arguments);
				}				
				
			} else throw new TCoreException(TCoreException::ERR_NOT_CLASS, array('class'=>$classname));
			return $i;
		}
		
/**
 * Gets a service with a specified name
 * @param string $name service name 
 * @returns TService  
 */					
		public function GetService($name){
			if (is_string(self::$_services_[$name]))
				self::$_services_[$name] = self::Instantiate(self::$_services_[$name],array("name"=>$name,"container"=>$this));
			if (self::$_services_[$name] instanceof IService)
				$this->_service_setup(self::$_services_[$name]);
			return self::$_services_[$name];
		}

/**
 * Makes a 302 redirect to a specified URL
 * @param string $url url where to redirect 
 */						
		public function Redirect($url){
			$this->Session()->Close();
			if (!headers_sent())
				header('Location:'.$url);
			die;
		} 
		
		private function _service_setup(IService $service){
			$service_config = $this->ConfCompileDir.'/service.'.$service->Name().'.php';
			if (file_exists($service_config)) include_once $service_config;
			$service_config = $this->ConfCompileDir.'/plugins/service.'.$service->Name().'.php';
			if (file_exists($service_config)) include_once $service_config;
		}
		
		private function _app_run(){
			if (self::$CompileAppConfig){
				TFileSystem::ForceDir($this->ConfCompileDir);
				for ($i = 0; $i < 2; $i++){
					if (file_exists($this->PrivateDir."/init.$i.xml") || file_exists($this->OverrideDir."/init.$i.xml")){
						if (!file_exists($this->ConfCompileDir."/init.$i.php"))
							$this->_compile_init($this->PrivateDir."/init.$i.xml",$this->ConfCompileDir."/init.$i.php");
						else
							if (
									(file_exists($this->PrivateDir."/init.$i.xml") && (filemtime($this->ConfCompileDir."/init.$i.php") < filemtime($this->PrivateDir."/init.$i.xml")))
									||
									(file_exists($this->OverrideDir."/init.$i.xml") && (filemtime($this->ConfCompileDir."/init.$i.php") < filemtime($this->OverrideDir."/init.$i.xml")))
							)
							$this->_compile_init($this->PrivateDir."/init.$i.xml",$this->ConfCompileDir."/init.$i.php");
					}
				}
			
				$this->_compile_plugins($this->ExtensionsDir.'/plugins', $this->ConfCompileDir.'/plugins');
					
				$config_file = $this->ConfCompileDir.'/application.php';
			
				$config = file_exists($config_file);
				$src = file_exists($this->ConfigFile);
				if (!$config){
					if ($src){
						$this->_compile_config($this->ConfigFile);
					} else throw new TCoreException(TCoreException::ERR_NO_APPCONF);
				} else {
					if ($src){
						if (filemtime($config_file) < filemtime($this->ConfigFile))
							$this->_compile_config($this->ConfigFile);
					}
				}
			
				if (file_exists($this->PrivateDir.'/routes.xml') || file_exists($this->OverrideDir.'/routes.xml')){
					if (
							(!file_exists($this->ConfCompileDir.'/url.php'))
							||	
							(file_exists($this->PrivateDir.'/routes.xml') && (filemtime($this->ConfCompileDir.'/url.php') < filemtime($this->PrivateDir.'/routes.xml')))
							||
							(file_exists($this->OverrideDir.'/routes.xml') && (filemtime($this->ConfCompileDir.'/url.php') < filemtime($this->OverrideDir.'/routes.xml')))
					)
						$this->_compile_routes($this->PrivateDir.'/routes.xml');
				}
			}			
			
			
			if (file_exists($this->ConfCompileDir.'/init.0.php'))
				include_once $this->ConfCompileDir.'/init.0.php';
			include_once $this->ConfCompileDir.'/application.php';
			if (file_exists($this->ConfCompileDir.'/plugins/application.php'))
				include_once $this->ConfCompileDir.'/plugins/application.php';			
			if (file_exists($this->ConfCompileDir.'/init.1.php'))
				include_once $this->ConfCompileDir.'/init.1.php';
			$servicename = '';
			$this->_call_type_ = isset($_SERVER['HTTP_HOST'])?self::CALL_HTTP:self::CALL_CONSOLE;
			if ($this->_call_type_ == self::CALL_HTTP) {
				if (isset($_REQUEST['service'])) $servicename = $_REQUEST['service'];
				if ($this->Sef){
					include_once $this->ConfCompileDir.'/url.php';
					self::$_request_ = $this->_service_map_->ParseUri($this->RelativePath,$this,$servicename);
				}
				if (!self::$_request_) 
					self::$_request_ = new THttpRequest($servicename,$this);				
			} else {
				self::$_request_ = new TConsoleCall();
			}
						
			$servicename = self::$_request_->GetService();
			if (!$servicename)
				$servicename = $this->_default_service_name_;
				
			$handled = false;
			
			if ($servicename){
				$this->_current_service_ = $this->GetService($servicename);
				if ($this->_current_service_){
					$handled = $this->_current_service_->Handle(self::Request());	
				} else throw new TCoreException(TCoreException::ERR_SERVICE_NOT_FOUND, array('service'=>$servicename));
			} else {
				foreach (self::$_services_ as $servicename=>$serviceclass)
					if ($this->_current_service_ = $this->GetService($servicename)){
						if ($handled = $this->_current_service_->Handle(self::Request())) break;	
					}
				if (!$handled)
					throw new TCoreException(TCoreException::ERR_SERVICE_NOT_FOUND, array('service'=>'default instance'));
			}
			$this->Session()->Close();	
		}
		
/**
 * Gets current request URI, which is a base64 encoded serialization result of THttpRequest object 
 * @returns string
 * @see THttpRequest 
 */						
		public function RequestURI(){
			if (self::$_request_)
				return self::$_request_->RequestURI();
			return false;
		}

/**
 * Get current request object copy
 * @return THttpRequest
 */		
		public function Request(){
			return clone self::$_request_; 
		} 
		
/**
 * Gets current service 
 * @returns TService
 * @see TService 
 */						
		public function CurrentService(){return $this->_current_service_;}
		
/**
 * Constructs an url to request a specified service
 * @param string $service optional service name, if not specified current service is used 
 * @param array $parameters optional request parameters array  
 * @returns string
 * @see TService 
 */						
		public function Url($service = null,array $parameters = array(),$base = null, $force_absolute = false){
			$dr = preg_replace('/[\\/\\\\]/','|',realpath($_SERVER['DOCUMENT_ROOT']));
			
			if ($base)
				$rp = preg_replace('/[\\/\\\\]/','|',realpath($this->AdjustPath($base)));
			else	
				$rp = preg_replace('/[\\/\\\\]/','|',realpath($this->RequestPath));		
			
			$pth = str_ireplace($dr,'',$rp);	
			$pth = str_replace('|','/',$pth);
			
			if (!$pth || $pth[0] != '/')
				$pth = '/'.$pth;
			
			if (!preg_match('/.*[\/\\\\]$/',$pth))
				$pth .= '/'; 				
				
			$url = $pth;
			$sef_url = null;
			
			if ($this->Sef){
				$notapplied = array();
				$sef_url = $this->_service_map_->MakeUri($service,$parameters,$notapplied);
			}
			
			if (!is_null($sef_url)){
				if (count($notapplied) > 0){
					$sef_url .= '?';
					foreach ($notapplied as $key)
						if (!is_null($parameters[$key]))
						$sef_url = $sef_url.'&'.$key.'='.rawurlencode($parameters[$key]);
				}
				$url = $url.$sef_url;
			} else {
				if ((!is_null($service)) || (count(array_values($parameters)) > 0))
					$url .= '?';
				if (!is_null($service))
					$url .= '&service='.rawurlencode($service);
				
				foreach ($parameters as $key=>$value)
					if ($value)
						$url = $url.'&'.$key.'='.rawurlencode($value);
			}
			
			if ($force_absolute)
				$url = 'http://'.$this->Domain.$url;
			return $url;
		}
				
		public function FileStoragePath(){
   			$path = $this->FileStoragePath;
   			if (!$path)
   				$path = $this->AppPath.'/files';
   			if (!is_dir($path))
   				TFileSystem::ForceDir($path);	
   			return $path;
		}
		
		private static function _adjust_environment(){
			switch (self::$Environment){
				case self::ENV_PRODUCTION:{
					error_reporting(E_ERROR);
				}break;
				case self::ENV_TESTING:{
					error_reporting(E_ERROR | E_WARNING | E_USER_ERROR | E_USER_WARNING);
				}break;
				case self::ENV_DEVELOPING:{
					error_reporting(E_ALL | E_STRICT);
					register_shutdown_function(array("TApplication","OnShutdown"));
					TConfigurable::$CompileIocTypes = true;
				}break; 
			}
		}
/**
* @ignore
* */		
		public function __set($nm,$value){
			switch ($nm){
				case 'TimeZone':date_default_timezone_set($value);break;
				case 'Shared':$this->_shared_ = preg_replace('/\A[\\\\\/]?(.*)[\\\\\/]?\z/','/$1',$value);break;
				case 'Language':{
					$this->_lang_ = $value;
					setlocale(LC_ALL,$value);
					TI18n::SetLanguage($value);
				}break;
				case 'Locale':TI18n::SetLocale($value);break;
				case 'Translator':TI18n::SetTranslator($value);break;
				case 'SessionHandler':{
					if (is_string($value))
						$this->_session_ = $this->Instance($value);
					else if ($value instanceof ISessionHandler)
						$this->_session_ = $value;
					else throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
				}break;
				case 'Path':set_include_path(str_replace(';', PATH_SEPARATOR, $value));break;
				case 'FileStoragePath':$this->_file_storage_path_ = $this->AdjustPath($value);break;
				case 'ImportPath':$this->AddImportPath($value);break;
				case 'Sef':$this->_sef_ = TConvertions::ConvertToBoolean($value);break;
				case 'Environment':{
					if (!is_numeric($value)){
						$temp = strtolower($value);
						$value = self::ENV_PRODUCTION;
						if (preg_match('/^test.*$/', $temp))
							$value = self::ENV_TESTING;
						if (preg_match('/^develop.*$/', $temp))
							$value = self::ENV_DEVELOPING;
					}
					self::$Environment = $value;
					self::_adjust_environment();
				}break;
				case 'StringEncoding':{
					if (mb_internal_encoding($value))
						$this->_string_encoding_ = $value;
				}break;
				default:parent::__set($nm,$value);
			}
		}
		
		private static function _check_class($classname){
			if (!class_exists($classname,false)){
				$parts = explode('/',$classname);
				if (class_exists($parts[count($parts) - 1],false))
					return $parts[count($parts) - 1]; 
				else
					return null;	
			}
			return $classname;			
		}
		
		private static function _autoloader($class){
			if ($class[0] == '/')
				$class = substr($class, 1);
			if (function_exists('stream_resolve_include_path')){
				if ($inc = stream_resolve_include_path($class.'.inc'))
					include_once $inc;
			} else include $class.'.inc';
			
			if (!self::_check_class($class)){
				if (is_array(self::$Application->ClassLoader)){
					foreach (self::$Application->ClassLoader as $cl)
							if ($cl->LoadClass($class)) break;
				} else if (self::$Application->ClassLoader){
					self::$Application->ClassLoader->LoadClass($class);
				}
			}			
		}
		
		public static function OnShutdown(){
			if ((!self::$_runtime_successfull_) && $error = error_get_last()){
				if ($error["type"] < 8)
					self::AppErrorHandler($error["type"], $error["message"], $error["file"], $error["line"]);
			}
		}
		
/**
* Application main method, which launches runtime. Called once in the initial script. 
*/		
		public static function Run($callerPath){
			if (!ini_get('date.timezone'))
				date_default_timezone_set('Asia/Vladivostok');	
			self::$_runtime_successfull_ = false;
			global $Application;
			spl_autoload_register(array("TApplication","_autoloader"));
			self::_adjust_environment();
			if (self::$ErrorsCapture){
				set_error_handler(array('TApplication','AppErrorHandler'));
				set_exception_handler(array('TApplication','AppExceptionHandler'));
			}
			$Application = new TApplication(realpath(dirname($callerPath)));
			set_include_path(get_include_path().PATH_SEPARATOR.self::$SystemPath.PATH_SEPARATOR.$Application->PrivateDir.'/code');
			$Application->_app_run();
			self::$_runtime_successfull_ = true;
		}
		
		public static function AppErrorHandler($errno,$errstr,$errfile,$errline){
			if (error_reporting() & $errno) {
				$e = new ErrorException($errstr, $errno, $errno, $errfile,$errline);
				TApplication::AppExceptionHandler($e);
				throw $e;
				//throw new ErrorException($errstr, $errno, $errno, $errfile,$errline);
				return false;
			} 
			return false;
		}
		
		public static function AppExceptionHandler(Exception $e){
			if (self::$Application && self::$Application->_ioc_exception_handler_ instanceof IExceptionHandler){
				return self::$Application->_ioc_exception_handler_->HandleException($e);
			} else {
				echo $e->getCode().':'.$e->getMessage().' on line '.$e->getLine().' in '.$e->getFile();
				die;
			}
			return false;	
		}
	}
?>