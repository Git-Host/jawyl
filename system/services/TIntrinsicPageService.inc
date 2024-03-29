<?php
/**
 * @package Services
 * @subpackage PageServices
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
require_once 'services/TPageService.inc';
/**
 * @ignore
 */
require_once 'core/utils/TConvertions.inc';

/**
 * @package Services
 * @subpackage PageServices
 * @category System
 * class of page configured with xml file
 */
class TIntrinsicPage extends TPage {
	protected $caching;	
	protected $itemData = array();
	private $_config_file_;
	private $_lazy_load_ = false;
	
/**
 * gets xml dom of page element specified by name
 * @return DOMDocument
 */	
	public function XmlData($name){
		if (isset($this->itemData[$name]))
			if ($this->itemData[$name] instanceof DOMDocument)
				return $this->itemData[$name];
		return null;
	}
	
	public function __get($nm){
		if ($this->_lazy_load_){
			$this->_lazy_load_ = false;
			$loadcomponents = true;  
			$service = $this->service;
			include $this->_config_file_;
		}
		return parent::__get($nm);
	}

	private static function _compile_page_element_settings(DOMNode $node, DOMXPath $xpath){
		$result = array();
		$config = $xpath->query("setting",$node);
		foreach ($config as $setting){
			if (key_exists($setting->getAttribute("name"),$result)){
				if (!is_array($result[$setting->getAttribute("name")]))
					$result[$setting->getAttribute("name")] = array($result[$setting->getAttribute("name")]);
				$result[$setting->getAttribute("name")][] = $setting->nodeValue;	
			} else
			$result[$setting->getAttribute("name")] = $setting->nodeValue;
		}
		return var_export($result,true);
	}

	private static function _compile_page_element(DOMNode $node, DOMXPath $xpath, $container_class, $compiled_container, &$result,&$counter,&$classes, TService $service){
		if ($node->tagName == "setting"){ 
			if (!is_subclass_of($container_class,"TResponseElement"))
				$result .= $compiled_container.'->'.$node->getAttribute("name").' = "'.addcslashes($node->nodeValue,'"').'";'."\n";
		}
		else {
			$result .= " if (\$loadcomponents) {\n";
			$extension = false;
			
			if (isset($classes[$node->getAttribute("name")]) || $inst = TApplication::$Application->Instance($node->getAttribute("name"))){
				$extension = true;
				if ($node->getAttribute("classname")){
					if (isset($classes[$node->getAttribute("name")]))
						$extension = $classes[$node->getAttribute("name")][0] == $node->getAttribute("classname");
				}	 
			}	
			
			if ($extension){
				if (isset($classes[$node->getAttribute("name")]))
					$varname = $classes[$node->getAttribute("name")][1];
				else {
					$varname = '$obj'.$counter;
					$counter++;
					$result .= $varname.' = $service->Application()->Instance("'.$node->getAttribute("name").'");';
					$classes[$node->getAttribute("name")] = array('',$varname,get_class($inst));
				}
			} else {
				$varname = '$obj'.$counter;
				$counter++;
				$c = $service->Application()->GetClass($node->getAttribute("classname"));
				if (!$c) throw new TCoreException(TCoreException::ERR_NOT_CLASS,array('class'=>$node->getAttribute("classname")));
				$classes[$node->getAttribute("name")] = array($node->getAttribute("classname"),$varname,$c->getName());
				if (!$c->implementsInterface("IConfigurable")) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				if (!(is_subclass_of($c->getName(),"TConfigurable") || is_subclass_of($c->getName(),"TMVPObject"))) throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				$result .= '$params = array("name"=>"'.$node->getAttribute("name").'");'."\n";
				if (is_subclass_of($c->getName(),"TResponseElement")){
					$result .= '$params["response"] = $this;'."\n";
					$result .= '$params["settings"] = '.self::_compile_page_element_settings($node,$xpath).";\n";
				}
				
				$datas = $xpath->query("data",$node);
				foreach ($datas as $data){
					$result .= '$dom = new DOMDocument();$dom->loadXML("'.addslashes($node->owner_document()->saveXML($data)).'");'."\n";
					$result .= '$this->itemData["'.$node->getAttribute("name").'"] = $dom;'."\n";
				}
				
				$_container = false;
				$_container_class = "";
				if ($node->getAttribute("parent")){
					if (isset($classes[$node->getAttribute("parent")])){
						$_container = $classes[$node->getAttribute("parent")][1];
						$_container_class = $classes[$node->getAttribute("parent")][3];
					}
				} else /*if ($compiled_container != '$this')*/{
					$_container = $compiled_container;
					$_container_class = $container_class;
				};
				if ($_container)
					$result .= '$params["container"] = '.$_container.';'."\n";	
				$result .= $varname.' = $service->Application()->Instantiate("'.$node->getAttribute("classname").'",$params);'."\n";	
				if (is_subclass_of($c->getName(),"TWidget")){
					$result .= '$this->AddWidget('.$varname.');'."\n";
					if ($_container)
						if (is_subclass_of($_container_class,'TWidget'))
							$result .= $_container.'->AddWidget('.$varname.');'."\n";
				}
					
			}	
			
			if ($node->getAttribute("membername"))
				$result .= $compiled_container.'->'.$node->getAttribute("membername").' = '.$varname.';'."\n";
			self::_compile_elements($node,$classes[$node->getAttribute("name")][2],$varname,$xpath,$result,$counter,$classes,$service);
			$result .= " }\n";
		}
	}
 	
	private static function _compile_elements(DOMNode $node, $container_class, $compiled_container, DOMXPath $xpath, &$result, &$counter,&$classes, TService $service){
		$nodes = $xpath->query("setting|component",$node);
		foreach ($nodes as $n)
			self::_compile_page_element($n,$xpath,$container_class, $compiled_container, $result,$counter,$classes,$service);
	}
	
	private static function _compile_page_plugin($filename,$real_classname,TIntrinsicPageService $service, &$counter, &$classes, &$text){
		$dom = new DOMDocument();
		$dom->preserveWhiteSpace = false;
		$dom->Load($filename);
		self::_compile_elements($dom->firstChild, $real_classname, '$this', new DOMXPath($dom), $text, $counter,$classes,$service);
	}
	
	private static function _compile_page_contents($filenames,TIntrinsicPageService $service, &$counter, &$classes, &$text){
		if (file_exists($filenames[0])){
			$dom = new DOMDocument();
			$dom->preserveWhiteSpace = false;
			$dom->Load($filenames[0]);
			if (TConvertions::ConvertToBoolean($service->ValidateConfig))
				if (!$dom->schemaValidate($service->Application()->SysPath()."/schemas/page.xsd"))
					throw new TCoreException(TCoreException::ERR_CONF_INCORRECT);
		}
		
		$override = false;
		if (file_exists($filenames[2])){
			$override = true;
			$odom = new DOMDocument();
			$odom->preserveWhiteSpace = false;
			$odom->Load($filenames[2]);
			if (TConvertions::ConvertToBoolean($service->ValidateConfig))
				if (!$odom->schemaValidate($service->Application()->SysPath()."/schemas/page.xsd"))
					throw new TCoreException(TCoreException::ERR_CONF_INCORRECT);
		}
		
		if (!(isset($dom) || $override))
			throw new TCoreException(TCoreException::ERR_APPCONF_INCORRECT);
		
		if (isset($dom))
			$classname = trim($dom->firstChild->getAttribute("classname"));
		
		if ($override)
			$classname = trim($odom->firstChild->getAttribute("classname"));
		
		if (!isset($classname) || !$classname){ 
			$classname = "TIntrinsicPage";
			$real_classname = $classname;
		} else {
			$c = $service->Application()->GetClass($classname);
			if (is_null($c))
				throw new TCoreException(TCoreException::ERR_NOT_CLASS, array('class'=>$classname));	
			$real_classname = $c->getName();
		}	
		
		$parent = null;
		
		if ($override){
			if ($odom->firstChild->getAttribute("parent"))
				$parent = $odom->firstChild->getAttribute("parent");
		}
		
		if (!$parent && isset($dom))
			if ($dom->firstChild->getAttribute("parent"))
				$parent = $dom->firstChild->getAttribute("parent");
		
		if ($parent){
			$pfilenames = $service->PageConfigFiles($parent);
			if (is_file($pfilenames[0]) || is_file($pfilenames[2])){
				self::_compile_page_contents($pfilenames,$service,$counter,$classes,$text);
			}
		}
		
		if ($v = $override?($odom->firstChild->getAttribute("cache")?$odom->firstChild->getAttribute("cache"):(isset($dom)?$dom->firstChild->getAttribute("cache"):null)):$dom->firstChild->getAttribute("cache"))
			$text .= '$this->caching = "'.$v.'";'."\n";
		if ($v = $override?($odom->firstChild->getAttribute("title")?$odom->firstChild->getAttribute("title"):(isset($dom)?$dom->firstChild->getAttribute("title"):null)):$dom->firstChild->getAttribute("title"))
			$text .= '$this->title = "'.$v.'";'."\n";
		if ($v = $override?($odom->firstChild->getAttribute("template")?$odom->firstChild->getAttribute("template"):(isset($dom)?$dom->firstChild->getAttribute("template"):null)):$dom->firstChild->getAttribute("template"))
			$text .= '$this->templateName = "'.$v.'";'."\n";
		
		if ($override)
			self::_compile_elements($odom->firstChild, $real_classname, '$this', new DOMXPath($odom), $text, $counter,$classes,$service);
		else
			self::_compile_elements($dom->firstChild, $real_classname, '$this', new DOMXPath($dom), $text, $counter,$classes,$service);
		
		if (file_exists($filenames[3])){
			$files = TFileSystem::ListFiles($filenames[3]);
			foreach ($files as $f)
				if (isset($f['extension']) && $f['extension'] == 'xml')
					self::_compile_page_plugin($f['dirname'].'/'.$f['basename'],$real_classname,$service, $counter, $classes, $text);
		}
		return $classname;
	}
	
	private static function _create_path($dir){
		TFileSystem::ForceDir($dir);
	}
	
	private static function _compile_page(array $filenames,TIntrinsicPageService $service){
		$text = "<?php\n";
		$counter = 1;	
		$classes = array();
		$classname = self::_compile_page_contents($filenames,$service,$counter, $classes, $text);		
		$text .= "?>";
		self::_create_path(dirname($filenames[1].".php"));
		file_put_contents($filenames[1].".php", $text);
		$text = "<?php\n";
		if ($classname == "TIntrinsicPage")
			$text .= '$result = new TIntrinsicPage($path,"'.$filenames[1].'.php",$service);'."\n";
		else 
			$text .= 'if (is_subclass_of($service->Application()->GetClass("'.$classname.'")->getName(),"TIntrinsicPage")) $result = $service->Application()->Instantiate("'.$classname.'",array("path"=>$path,"filename"=>"'.$filenames[1].'.php","service"=>$service)); else throw new TCoreException(TCoreException::ERR_BAD_TYPE);'."\n";
		$text .= "?>";	
		file_put_contents($filenames[1]."_loader.php", $text);
	} 
	
	private static function _check_need_compile(array $config,$compiled,TIntrinsicPageService $service){
		return true;
		if (!is_file($compiled.".php")) return true;
		if (file_exists($config[0]))
			if (filemtime($config[0]) > filemtime($compiled.".php")) return true;
		if (file_exists($config[2]))
			if (filemtime($config[2]) > filemtime($compiled.".php")) return true;

		
		if (file_exists($config[2])){
			$dom = new DOMDocument();
			$dom->preserveWhiteSpace = false;
			$dom->Load($config[2]);
			
			if (TConvertions::ConvertToBoolean($service->ValidateConfig))
				if (!$dom->schemaValidate($service->Application()->SysPath()."/schemas/page.xsd"))
				throw new TCoreException(TCoreException::ERR_CONF_INCORRECT);

			if ($dom->firstChild->getAttribute("parent")){
				$pfilenames = $service->PageConfigFiles($dom->firstChild->getAttribute("parent"));
				if (!empty($pfilenames))
					return self::_check_need_compile($pfilenames,$compiled,$service);
				return false;
			}
		}
		
		
		$dom = new DOMDocument();
		$dom->preserveWhiteSpace = false;
		$dom->Load($config[0]);
		
		if (TConvertions::ConvertToBoolean($service->ValidateConfig))
			if (!$dom->schemaValidate($service->Application()->SysPath()."/schemas/page.xsd"))
				throw new TCoreException(TCoreException::ERR_CONF_INCORRECT);
				
		if ($dom->firstChild->getAttribute("parent")){
			$pfilenames = $service->PageConfigFiles($dom->firstChild->getAttribute("parent"));
			if (!empty($pfilenames))
				return self::_check_need_compile($pfilenames, $compiled, $service); 
		}	
		return false;
	}
	
/**
 * instanciates an xml based page
 * @return TIntrinsicPage
 */	
	public static function Instanciate($path,TIntrinsicPageService $service){
		$result = null;
		$conffiles = $service->PageConfigFiles($path);
		if ($service->CompilePages){
			if (is_file($conffiles[0]) || is_file($conffiles[2])){
				if (self::_check_need_compile($conffiles,$conffiles[1],$service)){
					self::_compile_page($conffiles,$service);
				}
			}
		}
		
		if (is_file($conffiles[1]."_loader.php"))
			include $conffiles[1]."_loader.php";
		
		return $result;
	}
	

/**
 * constructor
 */	
	public function __construct($path, $filename, TIntrinsicPageService $service){
		if (!$path) $path = 'default';		
		parent::__construct($service,$path);
		$loadcomponents = is_null($service->CurrentPage());
		$this->_lazy_load_ = !$loadcomponents;
		$this->_config_file_ = $filename;  
		include $filename;
	}
		
/**
 * gest page cache life time
 * @return int
 */	
	public function Caching(){
		if (!is_null($this->caching)) return $this->caching;
		return $this->Service()->OutPutCaching;
	}

	protected function beforeRespond($handle_result, THttpRequest $request){
		$pth = $request->path_remainder;
		if (!$handle_result && !empty($pth))
			throw new TCoreException(TCoreException::ERR_RESOURCE_NOT_FOUND);
	}	

	public function Handle(THttpRequest $request){
		if (is_array($request->path_remainder)){
			$cnt = count($request->path_remainder);
			if ($cnt > 0){
				$ptr = 0;
				if (!isset($request->controller)){
					$controller = $request->path_remainder[$ptr];
					if ($ctr = $this->Item($controller)){
						$request->controller = $controller;
						$ptr++;
					}
					else if ($this->DefaultPresenter){
						if ($ctr = $this->Item($this->DefaultPresenter))
							$request->controller = $controller;
					}
					else $ctr = $this;
				}
				
				if (!isset($request->method))
					if ($ptr < count($request->path_remainder)){
						$method = $request->path_remainder[$ptr];
						if (is_callable(array(&$ctr,$method))){
							$request->method = $method;
							$ptr++;
						} 
					}
				
				$tmp = array_slice($request->path_remainder,$ptr,$cnt - $ptr);
				$cnt = count($tmp);				
				for ($i = $ptr; $i < $cnt; $i++)
					$request->SetGetParameter($i - $ptr,$tmp[$i]);
			} 
		}
		parent::Handle($request);
	}
}

class TIntrinsicAccessDeniedPage extends TIntrinsicPage {
	private $_worker_;
	
	public function __construct($path, $filename, TIntrinsicPageService $service){		
		parent::__construct($path, $filename, $service);
		$this->_worker_ = new TAccessDeniedPage($service,$path,"","");
	}
	
	public function SendHeaders(){
		$this->_worker_->SendHeaders();
		parent::SendHeaders();
	}
}

class TIntrinsicNotFoundPage extends TIntrinsicPage {
	private $_worker_;
	
	public function __construct($path, $filename, TIntrinsicPageService $service){		
		parent::__construct($path, $filename, $service);
		$this->_worker_ = new TNotFoundPage($service,$path,"","");
	}
	
	public function SendHeaders(){
		$this->_worker_->SendHeaders();
		parent::SendHeaders();
	}
} 

/**
/**
 * @package Services
 * @subpackage PageServices
 * @category System
 * class of page service configured by xml file
 */
abstract class TIntrinsicPageService extends TPageService {
	private $PAGES = array();
	
	private $__default__ = null;
	private $__notfound__ = null;
	private $__accessdenied__ = null;
	
	private $_compile_pages_ = false;
	private $_validate_config_ = false; 
	
	protected $_bool_sef_rpc_ = false;
	
	public function __set($nm,$value){
		switch ($nm){
			case "DefaultPath":$this->__default__ = $value;break;
			case "NotFoundPath":$this->__notfound__ = $value;break;
			case "AccessDeniedPath":$this->__accessdenied__ = $value;break;
			case "CompilePages":$this->_compile_pages_ = TConvertions::ConvertToBoolean($value);break;
			case "ValidateConfig":$this->_validate_config_ = TConvertions::ConvertToBoolean($value);break;
			default:parent::__set($nm,$value);
		}
	}

	public function __get($nm){
		switch ($nm){
			case "CompilePages":return $this->_compile_pages_;break;
			case "ValidateConfig":return $this->_validate_config_;break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function PageIdToFileName($id){
		if (is_dir($this->Application()->PrivateDir()."/".$this->Name().($id?"/":"").$id))
			$filename = $id.($id?"/":"")."default";
		else 	
			$filename = $id;
		return $filename;
	}
	
/**
 * @see TPageService::getPage()
 * @param mixed $id
 * @return TIntrinsicPage
 */	
	protected function getPage($id){
		if (!$id) return $this->getDefaultPage();
		if (isset($this->PAGES[$id])) return $this->PAGES[$id];
		$conffiles =  $this->PageConfigFiles($id);
		if (is_file($conffiles[0]) || is_file($conffiles[1].'.php') || is_file($conffiles[2])){
			$this->PAGES[$id] = TIntrinsicPage::Instanciate($id, $this);
			return $this->PAGES[$id];
		}
		return null;
	}
	
	public function PageConfigFiles($id){
		$filename = $this->PageIdToFileName($id);
		return array(
					$this->Application()->PrivateDir()."/".$this->Name()."/".$filename.".xml",
					$this->Application()->ConfCompileDir()."/".$this->Name()."/".$filename,
					$this->Application()->OverrideDir()."/".$this->Name()."/".$filename.".xml",
					$this->Application()->ExtensionsDir()."/modules/".$this->Name()."/".$filename
					);
	}
	
/**
 * @see TPageService::getDefaultPage()
 * @return TIntrinsicPage
 */	
	protected function getDefaultPage(){
		return $this->getPage($this->__default__?$this->__default__:'default');
	}

/**
 * @see TPageService::getNotFoundPage()
 * @return TIntrinsicPage
 */	
	protected function getNotFoundPage(){
		return $this->getPage($this->__notfound__);
	}

/**
 * @see TPageService::getAccessDeniedPage()
 * @return TIntrinsicPage
 */	
	protected function getAccessDeniedPage(){
		return $this->getPage($this->__accessdenied__);
	}
	
	private function _check_page($path){		
		return  is_file($this->Application()->PrivateDir()."/".$this->Name()."/".$path.".xml")
		|| is_file($this->Application()->OverrideDir()."/".$this->Name()."/".$path.".xml")  
		|| is_file($this->Application()->ConfCompileDir()."/".$this->Name()."/".$path."_loader.php") 
		|| is_dir($this->Application()->PrivateDir()."/".$this->Name()."/".$path);   
	}
	
	protected function handleRequest(TRequest $request){
		$request->IsAjax = $request->IsAjax || ($request->ajaxcall == "on");
		unset($request->ajaxcall);
		if (preg_replace('/^\\//', '',$request->path)){
			$path_parts = array_filter(preg_split('/[\\\\\\/]/',$request->path));
			$cnt = count($path_parts);
			$ptr = $cnt;
			while (($ptr > 0) && !$this->_check_page(join("/",array_slice($path_parts,0,$ptr))))
				$ptr--;	
			if (!empty($path_parts)){
				$request->page = join("/",array_slice($path_parts,0,$ptr));			
				$request->path_remainder = array_slice($path_parts,$ptr,$cnt - $ptr);
			}
		}
		parent::handleRequest($request); 
	}
	
	protected function formAppUrlParameters($page,$controller = null,$method = null,array $parameters = array()){
		if (TConvertions::ConvertToBoolean($this->Application()->Sef())){
			$path = $page;
			if ($this->SefRpc){
				if ($controller)
					$path .= "/".$controller;
				if ($method)
					$path .= "/".$method;
				$params = array();	
				foreach ($parameters as $key=>$value){
					if (is_string($key))
						$params[$key] = $value;
					else if (is_numeric($key))
						$path .= "/".$value;
				}
				return array("path"=>$path) + $params;
			}
			
			return parent::formAppUrlParameters(null, $controller, $method, array('path'=>$path) + $parameters);
		}
		return parent::formAppUrlParameters($page, $controller, $method, $parameters); 
	}	
}
?>