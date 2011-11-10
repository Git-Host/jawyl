<?php
/**
 * @package View
 * @subpackage Theming
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("core::TConfigurable");
/**
 * @ignore
 */
TApplication::import("common::theme::interfaces");
/**
 * @ignore
 */
TApplication::import("common::orm");
/**
 * @ignore
 */
TApplication::import("services::TPageService");

/**
 * @package View
 * @subpackage Theming
 * @category System
 * Native theme engine class
 */
class TIntrinsicThemeEngine extends TThemeEngine implements ISecurityObject {
/**
 * @var string
 */	
	public $ThemeName;
/**
 * @var string
 */	
	public $ThemesDir;
	
	private $_context_stack_ = array();
	
	private $_widget_stack_ = array();
	
	private $_cycle_counter_ = 0;
	
	private $_cycleodd_ = 0;
	
	private $_cleaner_;
	
	protected $_ioc_output_cache_; 
	
/**
 * @var TResponse
 */	
	protected $response;
	
	private $_current_cache_name_ = array();
	private $_current_cache_lifetime_ = array();
	
	public function Soid(){
		return TApplication::$Application->Name()."::".$this->Name();
	}
		
	protected function processTag($name){
		$cc = $this->currentContext();
		if (strtolower($name) == "context"){
			echo $cc;
			return;
		}	
		if (method_exists($cc,$name)){
			echo call_user_func(array($cc,$name));
			return;
		}
		if (property_exists($cc,$name)){
			echo $cc->$name;
			return;
		}
		echo $name;	
		return;
	}	
	
	protected function evalContextTag($name){
		$cc = $this->currentContext();
		if (strtolower($name) == "context")
			return $cc;
		if (method_exists($cc,$name))
			return call_user_method($name,$cc);
		if (property_exists($cc,$name))
			return $cc->$name;
		if (method_exists($cc,"__get"))	
			return $cc->__get($name);	
		return null;
	}
	
	protected function getContextIterator($name){
		$iter = $this->evalContextTag($name);
		if (is_array($iter)) $iter = new TArrayIterator($iter);
		if (!($iter instanceof IIterator)) throw new TCoreException(TCoreException::ERR_BAD_TYPE);
		return $iter;
	}
	
	private function _out_widget($name,$templatefile){
		$widget = $this->response->Item(trim($name));
		if (!is_null($widget)){
			$cw = $this->currentWidget();
			if ($cw !== $widget)
				array_push($this->_widget_stack_,$widget);
			$widget->Output($this,$templatefile);
			if ($cw !== $widget)
				array_pop($this->_widget_stack_);
		}
	}
	
	private function _widg_compile($matches){
		$nm = "";$tmpl = "";
		if (count($matches) > 1)
			$nm = str_replace('"','',$matches[1]);
		if (count($matches) > 2){
			$tmpl = str_replace('"','',$matches[2]);
			$tmpl = '"'.$tmpl.'""';
		}
		$nm = ($nm == "")?"null":$nm;
		$tmpl = ($tmpl == "")?"null":$tmpl;
		return '<?php $this->_out_widget("'.$nm.'",'.$tmpl.'); ?>';
	}

	private function _repl_compile($matches){
		$context = "null";
		if (count($matches) > 1)
			$nm = str_replace('"','',$matches[1]);
		if (count($matches) > 2){
			$context = str_replace('"','',$matches[2]);
			if ($context == "") $context = "null"; else $context = "$".$context;
		}
		return '<?php $this->ProcessTemplate('.$context.',"'.$nm.'"); ?>';
	}
	
	private function _iter_compile($matches){
		$this->_cycle_counter_++;
		return '<?php $cyclebase'.$this->_cycle_counter_.'=$this->getContextIterator("'.$matches[1].'"); $cyclecounter'.$this->_cycle_counter_.' = 0; foreach ($cyclebase'.$this->_cycle_counter_.' as $'.$matches[2].'){  $cyclecounter'.$this->_cycle_counter_.'++; $this->_cycleodd_ = ($cyclecounter'.$this->_cycle_counter_.' % 2 <> 0)?true:false; ?>';
	}
	
	protected function preCompile($text){
		return $text;
	}
	
	private function _compile($filename,$preprocessor = null){
		if (basename($filename) == $filename)
			$filename = $this->TemplateDir."/".$filename;
		$text = file_get_contents($filename);
		if ($preprocessor instanceof ITemplatePreProcessor)
			$text = $preprocessor->PreProcess($text);
		
		$text = $this->preCompile($text);
		$text = preg_replace('/<%startcache\\s+([_\\w]+)\\s+(\\d+)%>/','<?php if ($this->startCache("$1",$2)) { ?>',$text);
		$text = preg_replace('/<%stopcache%>/','<?php $this->saveCache(); } ?>',$text);		
		$text = preg_replace_callback('/<%insert\\s+([^\\s]*|"[^"]*")(?:\\s+context\\s*=\\s*([^\\s]*|"[^"]*"))?\\s*%>/',array(&$this,"_repl_compile"),$text);
		$text = preg_replace_callback('/<%widget\\s+([^\\s]*|"[^"]*")(?:\\s+skin\\s*=\\s*([^\\s]*|"[^"]*"))?\\s*%>/',array(&$this,"_widg_compile"),$text);
		$text = preg_replace('/<\\?(?!php)/','<?php ',$text);	
		$text = preg_replace_callback('/<%iterate\\s+([_\\w]+)\\s+as\\s+([_\\w]+)\\s*%>/',array(&$this,"_iter_compile"),$text);
		$text = preg_replace('/<%iterate%>/','<?php } ?>',$text);
		$text = preg_replace('/<%if\\s+([_\\w]+)\\s*%>/','<?php if ($this->evalContextTag("$1")) { ?>',$text);
		$text = preg_replace('/<%if\\s+([_\\w]+)\\s+([_\\w]+|"[^"]*")\\s*%>/','<?php if ($this->evalContextTag("$1") == $2) { ?>',$text);
		$text = preg_replace('/<%odd%>/','<?php if ($this->_cycleodd_) { ?>',$text);
		$text = preg_replace('/<%else%>/','<?php } else { ?>',$text);
		$text = preg_replace('/<%endif%>/','<?php } ?>',$text);
		$text = preg_replace('/<%endodd%>/','<?php } ?>',$text);
		$text = preg_replace('/<%\\s*([_\\w]*?)\\s*%>/','<?php $this->processTag("$1"); ?>',$text);
		$text = preg_replace('/<%\\s*(.*?)\\s*%>/','<?php echo $1; ?>',$text);
		return $text;
	}
	
	private function _need_compile($filename){
		if (basename($filename) == $filename)
			$filename = $this->TemplateDir."/".$filename;
		$cf = basename($filename);
		if (file_exists($this->CompileDir."/".$cf)){
			if (filemtime($this->CompileDir."/".$cf) < filemtime($filename))
				return true;
			return false;
		}
		return true;
	}
	
	public function OutputCache(){
		return $this->OutputCache;
	}
	
	protected function startCache($cachename,$lifetime){
		if ($this->OutputCache){
			if (!$this->OutputCache->CachedOutput($cachename,$lifetime)){
				array_push($this->_current_cache_name_, $cachename);
				array_push($this->_current_cache_lifetime_, $lifetime);
				ob_start();
				return true;
			}
			return false;
		}
		return true;
	}
	
/**
 * saves cached output to file, returns full path of created file
 * @param string $filename template file name
 * @param string $text cached output
 * @return string 
 */	
	protected function stopCache(){
		if ($this->OutputCache){
			$cn = array_pop($this->_current_cache_name_);
			$clt = array_pop($this->_current_cache_lifetime_);
			$this->OutputCache->SaveCache($cn,ob_get_contents(),$clt);
			ob_end_flush();
		}
	}
/**
 * gets path to include for a specified template
 * @param string $filename template file name
 * @return string 
 */	
	protected function outputTemplate($context, $filename, $cachelifetime = 0, $preprocessor = null){
		if ($cachelifetime > 0)
			if (!$this->startCache($filename,$cachelifetime)) return;
			
		$SERVICE = TApplication::$Application->CurrentService();
		$PAGE = $this->response;
		$THEME_URL = $this->ThemeUrl();
		$APPLICATION = TApplication::$Application;
		$CONTEXT = $context;
		$WIDGET = $this->CurrentWidget();
		include $this->getCompiledFile($filename,$preprocessor);
		if ($cachelifetime > 0)
			$this->stopCache(); 
	}

/**
 * gets compiled file for a template file
 * @param string $filename template file name
 * @param $cache optional cache life time, when not specified caching is disabled
 * @return string 
 */	
	protected function getCompiledFile($filename, $cachelifetime = 0, $preprocessor = null){
		$cf = $filename;
		if ($this->_need_compile($filename)){
			$compiletext = $this->_compile($filename,$preprocessor);
			file_put_contents($this->CompileDir."/".basename($cf),$compiletext);
		}
		return $this->CompileDir."/".basename($cf);
	}
	
/**
 * @see IThemeEngine::TemplateDir()
 * @return string
 */	
	public function TemplateDir(){
		return $this->TemplateDir;
	}
	
	public function __get($nm){
		switch ($nm){
			case "TemplateDir":return $this->ThemeDir().'/templates';break;
			case "CompileDir":return $this->ThemeDir().'/compile';break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function ThemeUrl(){
		$theme_dir = $this->ThemeDir();
		$theme_dir = preg_replace('/[\\\\\\/]/','|',$theme_dir);
		$apppath = preg_replace('/[\\\\\\/]/','|',TApplication::$Application->AppPath);
		$theme_url = str_replace('|','/',str_ireplace($apppath,'',$theme_dir));
		return TApplication::$Application->Root.$theme_url;
	}
	
	public function ThemeDir(){
		$themes_dir = TApplication::$Application->AppPath.'/themes';
		if (isset($this->ThemesDir))
			$themes_dir = $this->ThemesDir; 
		if (!is_dir($themes_dir))
			mkdir($themes_dir);	
		return $themes_dir."/".$this->ThemeName;		
	}		
	
/**
 * @see IThemeEngine::TemplateExtension()
 * @return string
 */	
	public function TemplateExtension(){
		return "tpl";
	}	

/**
 * @see IThemeEngine::ProcessTemplate()
 * @param mixed $context
 * @param string $filename
 */	
		
	public function ProcessTemplate($context, $filename,  $cachelifetime = 0, IPreProcessor $preprocessor = null){
		$cc = $this->CurrentContext();
		if (($context !== $cc) && (!is_null($context)))
			array_push($this->_context_stack_,$context);
		$this->outputTemplate($cc, $filename,$cachelifetime,$preprocessor);
		if (($context !== $cc) && (!is_null($context)))
			array_pop($this->_context_stack_);
	}

	protected function currentContext(){
		$sc = count($this->_context_stack_);
		if ($sc > 0)
			return $this->_context_stack_[$sc - 1];
		return null;
	}
	
	protected function currentWidget(){
		$sc = count($this->_widget_stack_);
		if ($sc > 0)
			return $this->_widget_stack_[$sc - 1];
		return null;
	}
	
/**
 * @see IThemeEngine::RenderResponse()
 * @param string $templatename
 */	
	public function RenderResponse(TResponse $response,$templatename){
		$td = $this->TemplateDir;
		$cd = $this->CompileDir;
		if (!file_exists($td)) mkdir($td);
		if (!file_exists($cd)) mkdir($cd);
		$this->response = $response;
		$this->ProcessTemplate($this->response,$templatename.'.'.$this->TemplateExtension());
	}	
}
?>