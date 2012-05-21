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
TApplication::import("common::theme::engine");
/**
 * @ignore
 */
TApplication::import("services::TPageService");

function capitalize($str){
	return strtoupper($str[0]).strtolower(substr($str, 1));
}

/**
 * @package View
 * @subpackage Theming
 * @category System
 * 
 * @property string $TemplateDir
 * @property string $CompileDir
 * 
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
	
	private $_global_vars_ = array();
	
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
		$v = $name; 
		if (is_string($name)){
			$cc = $this->currentContext();
			if (strtolower($name) == "context")
				$v = $cc;
			else if (method_exists($cc,$name))
				$v = call_user_func(array($cc,$name));
			else if (isset($cc->$name))
				$v = $cc->$name;
			else if (method_exists($cc,"__get")){
				$t = $cc->__get($name);
				if (!is_null($t))
					$v = $t;
				else if (isset($this->_global_vars_[$name]))
					$v = $this->_global_vars_[$name];
				else if (defined($name))
					$v = constant($name);		
			}
		}
		echo $v;
	}

	protected function pushContext($context){
		array_push($this->_context_stack_,$context);
	}
	
	protected function popContext(){
		array_pop($this->_context_stack_);
	}
	
	protected function evalContextTag($name,$get_self = false){
		$cc = $this->currentContext();
		$n = trim($name);
		if (strtolower($n) == "context")
			return $cc;
		if (method_exists($cc,$n))
			return call_user_func(array($cc,$n));
		if (isset($cc->$n))
			return $cc->$n;
		if (isset($this->_global_vars_[$n]))
			return $this->_global_vars_[$n];
		if (defined($n))
			return constant($n);		
		$v = @$cc->$n;
		if (!is_null($v))
			return $v;
		if (isset($this->response->$n))
			return $this->response->$n;
		if (isset($this->response->Service()->$n))
			return $this->response->Service()->$n;
		if (isset(TApplication::$Application->$n))
			return TApplication::$Application->$n;
		if (class_exists($n))
			return $n;			
		if ($get_self)	
			return $name;
		return null;
	}
	
	private function _compile_arr_index($matches){
		$expr = trim($matches[1]);
		if ($expr[0] == '`')
			return '['.str_replace('`','"',addslashes($expr)).']';
		if ($expr[0] == '$'){
			return '[$this->_global_vars_["'.substr($expr,1).'"]]';
		}
		return '[$this->evalContextTag(\''.$expr.'\',true)]';		
	}
	
	private function _compile_expression($expr,$get_self = false){
		$expr = trim($expr);
		if ($expr[0] == '`')
			return str_replace('`','"',addslashes($expr));

		if ($expr[0] == '$'){
			$brpos = strpos($expr, '[');
			$result = '$this->_global_vars_["'.substr($expr,1,$brpos?($brpos-1):(strlen($expr)-1)).'"]';
			if ($brpos !== false){
				$expr = substr($expr,$brpos);
				$result .= preg_replace_callback('/\[([^\]]*)\]/', array($this,'_compile_arr_index'), $expr);
			}
			return $result;
		}
		
		if (strpos($expr,'::') !== false)
			return $expr;
		
		return '$this->evalContextTag("'.$expr.'"'.($get_self?',true':'').')';
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
	
	protected function callMethod($method,$var){
		if (method_exists($this->currentContext(), $method))
			return $this->currentContext()->$method($var);
		if ($this->currentWidget())
			if (method_exists($this->currentWidget(), $method))
				return $this->currentWidget()->$method($var);
		if (method_exists($this->response,$method))
			return $this->response->$method($var);	
		if (method_exists($this->response->Service(),$method))
			return $this->response->Service()->$method($var);
		if (function_exists($method))
			return  $method($var);
		return null;
	}
	
	private function _compile_filters_open(array $filters){
		$result ='';
		foreach ($filters as $f){
			$v = strtolower(trim($f));
			if ($v){
				switch ($v){
					case 'c':
					case 'd':
					case 'dt':
					case 't':
					case 'n':
					case 'i':$result .= '_'.$v.'(';break;
					case 'u':$result .= 'strtoupper(';break;
					case 'l':$result .= 'strtolower(';break;
					case 'cap':$result .= 'capitalize(';break;
					case 'q':$result .= 'htmlentities(';break;
					default:{
						if (function_exists($v))
							$result .= $v.'(';
						else
							$result .= '$this->callMethod("'.trim($f).'",'; 	
					}break;
				}
			}
		}
		return $result;
	}
	
	private function _compile_filters_close(array $filters){
		$result ='';
		foreach ($filters as $f){
			$v = strtolower(trim($f));
			if ($v){
				switch ($v){
					case 'q':$result .= ',ENT_QUOTES,"UTF-8")';break;
					default:$result .= ')';break;
				}
			}
		}
		return $result;
	}
	
	private function _assign_compile($matches){
		$result = '<?php $this->_global_vars_["'.$matches[1].'"] = ';
		$filters = explode('<', $matches[2]);
		$result .= $this->_compile_filters_open($filters);
		$val = trim($matches[3]); 
		$result .= $this->_compile_expression($val,true); 
		$result .= $this->_compile_filters_close($filters);
		return $result.'; ?>';
	}
	
	private function _varout_compile($matches){
		$result = '<?php echo ';
		$filters = explode('<', $matches[1]);
		$result .= $this->_compile_filters_open($filters);
		$val = trim($matches[2]); 
		$result .= $this->_compile_expression($val,true); 
		$result .= $this->_compile_filters_close($filters);
		return $result.'; ?>';
	}
	
	private function _rawout_compile($matches){
		$result = '<?php echo ';
		$filters = explode('<', $matches[1]);
		$result .= $this->_compile_filters_open($filters);
		$result .= trim($matches[2]);
		$result .= $this->_compile_filters_close($filters);
		return $result.'; ?>';
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
		$result = '<?php $this->_global_vars_["iterlevel"] = (!isset($this->_global_vars_["iterlevel"]))?0:$this->_global_vars_["iterlevel"]+1; $cyclebase'.$this->_cycle_counter_.' = '.$this->_compile_expression($matches[1]).'; if (is_array($cyclebase'.$this->_cycle_counter_.')) $cyclebase'.$this->_cycle_counter_.' = new TArrayIterator($cyclebase'.$this->_cycle_counter_.'); $cyclecounter'.$this->_cycle_counter_.' = 0; foreach ($cyclebase'.$this->_cycle_counter_.' as ';
		if (count($matches) == 4)
			return $result.'$'.$matches[2].'=>$'.$matches[3].'){  $cyclecounter'.$this->_cycle_counter_.'++; $this->_cycleodd_ = ($cyclecounter'.$this->_cycle_counter_.' % 2 <> 0)?true:false; $this->pushContext($'.$matches[3].'); $this->_global_vars_[\''.$matches[2].'\'] = $'.$matches[2].';$this->_global_vars_[\''.$matches[3].'\'] = $'.$matches[3].'; ?>';
		return $result.'$'.$matches[2].'){  $cyclecounter'.$this->_cycle_counter_.'++; $this->_cycleodd_ = ($cyclecounter'.$this->_cycle_counter_.' % 2 <> 0)?true:false; $this->pushContext($'.$matches[2].'); $this->_global_vars_[\''.$matches[2].'\'] = $'.$matches[2].'; ?>';
	}
	
	protected function preCompile($text){
		return $text;
	}
	
	private function _compile_oper($matches){
		switch ($matches[1]){
			case 'if':{
				if (count($matches) > 3)
					return '<?php if ('.$this->_compile_expression($matches[2]).' == '.$this->_compile_expression($matches[3],true).') { ?>';
				else
					return '<?php if (@'.$this->_compile_expression($matches[2]).') { ?>';
			}break;
			case 'ifnot':{
				if (count($matches) > 3)
					return '<?php if ('.$this->_compile_expression($matches[2]).' != '.$this->_compile_expression($matches[3],true).') { ?>'; 
				else
					return '<?php if (!@'.$this->_compile_expression($matches[2]).') { ?>';
			}break;
			case 'is':return '<?php $__c__n__ = '.$this->_compile_expression($matches[2],true).'; if ($this->CurrentContext() instanceof $__c__n__) { ?>';break;
			case 'switch':return '<?php switch ('.$this->_compile_expression($matches[2]).') { ?>';break;
			case 'case':return '<?php case '.$this->_compile_expression($matches[2],true).':{ ?>';break;
			case 'with':return '<?php $this->pushContext('.$this->_compile_expression($matches[2]).'); ?>';break;
		}
	}
	
	
	private function _compile($filename,$preprocessor = null){
		if (basename($filename) == $filename)
			$filename = $this->TemplateDir."/".$filename;
		$text = file_get_contents($filename);
		if ($preprocessor instanceof ITemplatePreProcessor)
			$text = $preprocessor->PreProcess($text);
			
		$expr = '[_\w][_\w:]*|\$[_\w]+(?:\[[^\]]*\])*|`[^`]*`';
						
		$text = $this->preCompile($text);
		$text = preg_replace('/{%startcache\s+([_\w]+)\s+(\d+)\s*%}/','<?php if ($this->startCache("$1",$2)) { ?>',$text);
		$text = preg_replace('/{%stopcache%}/','<?php $this->saveCache(); } ?>',$text);		
		$text = preg_replace_callback('/{%insert\s+([^\s]*?|`[^`]*`)(?:\s+context\s*=\s*([^\s]*?|`[^`]*`))?\s*%}/',array(&$this,"_repl_compile"),$text);
		$text = preg_replace_callback('/{%widget\s+([^\s]*?|`[^`]*`)(?:\s+skin\s*=\s*([^\s]*?|`[^`]*`))?\s*%}/',array(&$this,"_widg_compile"),$text);
		$text = preg_replace('/<\?(?!php)/','<?php ',$text);
			
		$text = preg_replace_callback('/{%iterate\s+('.$expr.')\s+as\s+([_\w]+)(?:\s*>\s*([_\w]+))?\s*%}/',array(&$this,"_iter_compile"),$text);
		$text = preg_replace('/{%iterate%}/','<?php $this->popContext(); } $this->_global_vars_["iterlevel"]--; ?>',$text);

		$text = preg_replace_callback('/{%(if(?:not)?|is)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(if(?:not)?)\s+('.$expr.')\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
	/*	$text = preg_replace_callback('/{%(ifnot)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(ifnot)\s+('.$expr.')\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(is)\s+('.$expr.')\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
	*/	
		$text = preg_replace('/{%odd%}/','<?php if ($this->_cycleodd_) { ?>',$text);
		
		$text = preg_replace_callback('/{%(with)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(switch)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(case)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		
		$text = preg_replace('/{%default%}/','<?php default:{ ?>',$text);		
		$text = preg_replace('/{%else%}/','<?php } else { ?>',$text);
		$text = preg_replace('/{%endis%}/','<?php } ?>',$text);
		$text = preg_replace('/{%endif%}/','<?php } ?>',$text);
		$text = preg_replace('/{%endodd%}/','<?php } ?>',$text);
		$text = preg_replace('/{%endcase%}/','<?php }break; ?>',$text);
		$text = preg_replace('/{%endswitch%}/','<?php } ?>',$text);
		$text = preg_replace('/{%endwith%}/','<?php $this->popContext(); ?>',$text);
		
		$text = preg_replace_callback('/{%assign\\s+([_\w]+)(\s+(?:(?:[_\w][_\w]*\s*\<\s*))+)?\s+('.$expr.')\s*%}/',array(&$this,"_assign_compile"),$text);
		$text = preg_replace_callback('/{%\s*(\s+(?:(?:[_\w][_\w]*\s*\<\s*))+)?('.$expr.')\s*%}/',array(&$this,"_varout_compile"),$text);
		$text = preg_replace_callback('/{%\s*(\s+(?:(?:[_\w][_\w]*\s*\<\s*))+)(.*?)\s*%}/',array(&$this,"_rawout_compile"),$text);
		$text = preg_replace('/{%\s*(.*?)\s*%}/','<?php echo $1; ?>',$text);
		$text = preg_replace('/\?>\s*<\?php/m','',$text);
		return $text;
	}
	
	private function _need_compile($filename,$compiled){
		if (file_exists($compiled)){
			if (filemtime($compiled) < filemtime($filename))
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
			
		$this->_global_vars_['SERVICE'] = TApplication::$Application->CurrentService();
		$this->_global_vars_['PAGE'] = $this->response;
		$this->_global_vars_['THEME_URL'] = $this->ThemeUrl();
		$this->_global_vars_['APPLICATION'] = TApplication::$Application;
		$this->_global_vars_['APPLICATION_SHARED'] = TApplication::$Application->Shared;
/*		
		$this->_global_vars_['CONTEXT'] = $context;
		$this->_global_vars_['WIDGET'] = $this->CurrentWidget();
*/
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
		if (!TFileSystem::IsAbsolute($filename)){
			$cf = $this->CompileDir."/".$filename;
			$filename = $this->TemplateDir."/".$filename;
		} else {
			$cf = str_ireplace($this->TemplateDir,'', $filename);
		}
		if ($this->_need_compile($filename,$cf)){
			$compiletext = $this->_compile($filename,$preprocessor);
			TFileSystem::ForceDir(dirname($cf));
			file_put_contents($cf,$compiletext);
		}
		return $cf;
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