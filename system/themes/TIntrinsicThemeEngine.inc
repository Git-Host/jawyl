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

require_once 'core/TConfigurable.inc';
require_once 'common/theme/engine.inc';
require_once 'core/utils/processing.inc';
require_once 'services/TPageService.inc';

function capitalize($str){
	return mb_strtoupper(mb_substr($str,0,1)).mb_strtolower(mb_substr($str, 1));
}

function html_obfuscate($str){
	/* @TODO obfuscation logic */
	return $str;
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
	
	private $_themes_dir_;
	
	private $_context_stack_ = array();
	
	private $_widget_stack_ = array();
	
	private $_cycle_counter_ = 0;
	
	private $_cycleodd_ = 0;
	
	private $_cleaner_;
	
	private $_global_vars_ = array();
	
	protected $_ioc_output_cache_;

/**
 * @var IDataFilter[]
 */	
	protected $_ioc_content_filter_ = array();
	
	private $_filter_hash_ = null;
	
/**
 * @var TResponse
 */	
	protected $response;
	
	private $_current_cache_name_ = array();
	private $_current_cache_lifetime_ = array();
	
	public function Soid(){
		return $this->Application()->Name()."::".$this->Name();
	}
		
	protected function processTag($name){
		$v = $name; 
		if (is_string($name)){
			$cc = $this->currentContext();
			if ($name == "context")
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
	
	protected function evalContextTag($name,$get_self = false,$bubble = true){
		$cc = $this->currentContext();		
		
		$n = trim($name);
		if ($n == "context")
			return $cc;

		if (is_object($cc)){
			if (method_exists($cc,$n))
				return call_user_func(array($cc,$n));
			if (method_exists($cc, '__get') || property_exists($cc, $n)){
				$v = $cc->$n;
				if (!is_null($v))
					return $v;
			}
		}
		
		if (isset($this->_global_vars_[$n]))
			return $this->_global_vars_[$n];
		
		if (defined($n))
			return constant($n);		
		
		if ($bubble){
			if ($this->response){	
				if (isset($this->response->$n))
					return $this->response->$n;
				if (method_exists($this->response,$n))
					return $this->response->$n();
			}				
			if (isset($this->Application()->CurrentService()->$n))
				return $this->Application()->CurrentService()->$n;
			if (method_exists($this->Application()->CurrentService(),$n))
				return $this->Application()->CurrentService()->$n();			
			if (isset($this->Application()->$n))
				return $this->Application()->$n;
			if (method_exists($this->Application(),$n))
				return $this->Application()->$n();			
		}
								
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
	
	private function _compile_expression($expr,$get_self = false,$bubble = true){
		$expr = trim($expr);
		if (is_numeric($expr))
			return $expr;
		if ($expr[0] == '`')
			return str_replace('`','"',addslashes($expr));

		if ($expr[0] == '$'){
			$brpos = strpos($expr, '[');
			$result = '$this->_global_vars_["'.substr($expr,1,$brpos?($brpos-1):(strlen($expr)-1)).'"]';
			if ($brpos !== false){
				$expr = substr($expr,$brpos);
				$result .= preg_replace_callback('/\[([^\]]*)\]/', array($this,'_compile_arr_index'), $expr);
				$result = 'isset('.$result.') && '.$result;
			}
			return $result;
		}
		
		if (strpos($expr,'::') !== false)
			return $expr;
		
		return '$this->evalContextTag("'.$expr.'",'.($get_self?'true':'false').((!$bubble)?',false':'').')';
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
		return '<?php $this->_out_widget($this->evalContextTag("'.$nm.'",true),'.$tmpl.'); ?>';
	}
	
	protected function callMethod($method,$var){
		if (method_exists($this->currentContext(), $method))
			return $this->currentContext()->$method($var);
		if ($this->currentWidget())
			if (method_exists($this->currentWidget(), $method))
				return $this->currentWidget()->$method($var);
		if ($this->response){
			if (method_exists($this->response,$method))
				return $this->response->$method($var);
		}	
		if (method_exists($this->Application()->CurrentService(),$method))
			return $this->Application()->CurrentService()->$method($var);
		if (function_exists($method))
			return  $method($var);
		return null;
	}
	
	private function _get_content_filter($nm){
		if (is_null($this->_filter_hash_)){
			$this->_filter_hash_ = array();
			foreach ($this->ContentFilter as $f)
				$this->_filter_hash_[$f->CallName()] = $f;
		}
			
		if (isset($this->_filter_hash_[$nm]))
			return $this->_filter_hash_[$nm];
		return null;
	}
	
	private function _compile_filters_open(array $filters){
		$result ='';
		foreach ($filters as $f){
			$fn = trim($f);
			$v = strtolower($fn);
			if ($v){
				switch ($v){
					case 'c':
					case 'd':
					case 'dt':
					case 't':
					case 'n':
					case 'i':$result .= '_'.$v.'(';break;
					case 'u':$result .= 'mb_strtoupper(';break;
					case 'l':$result .= 'mb_strtolower(';break;
					case 'cap':$result .= 'capitalize(';break;
					case 'obf':$result .= 'html_obfuscate(';break;
					case 'esc':$result .= 'addslashes(';break;
					case 'jsl':$result .= 'addcslashes(';break;
					case 'trm':$result .= 'trim(';break;
					case 'q':$result .= 'htmlentities(';break;
					default:{
						if ($this->_get_content_filter($fn))
							$result .= '$this->_get_content_filter("'.$fn.'")->Filter(';
						else if (function_exists($fn))
							$result .= $fn.'(';
						else
							$result .= '$this->callMethod("'.$fn.'",'; 	
					}break;
				}
			}
		}
		return $result;
	}
	
	private function _compile_filters_close(array $filters){
		$result ='';
		foreach (array_reverse($filters) as $f){
			$v = strtolower(trim($f));
			if ($v){
				switch ($v){
					case 'q':$result .= ',ENT_QUOTES,"UTF-8",false)';break;
					case 'jsl':$result .= ',"\\0\\n\\v\\f\\r\\\'\\"\\\\")';break;
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
	
	private function _arith_compile($matches){
		$result = '<?php $this->_global_vars_["'.$matches[1].'"] = ';
		$result .= $this->_compile_expression(trim($matches[2]),true);
		$result .= ' '.$matches[3].' ';
		$result .= $this->_compile_expression(trim($matches[4]),true);
		return $result.'; ?>';
	}	
	
	private function _varout_compile($matches){
		$result = '<?php echo ';
		$filters = explode('<', $matches[2]);
		$result .= $this->_compile_filters_open($filters);
		$val = trim($matches[3]); 
		$result .= $this->_compile_expression($val,false,$matches[1] != "@"); 
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
			return $result.'$'.$matches[2].'=>$'.$matches[3].'){ $cyclecounter'.$this->_cycle_counter_.'++;$this->_global_vars_["counter"] = $cyclecounter'.$this->_cycle_counter_.'; $this->_cycleodd_ = ($cyclecounter'.$this->_cycle_counter_.' % 2 <> 0)?true:false; $this->pushContext($'.$matches[3].'); $this->_global_vars_[\''.$matches[2].'\'] = $'.$matches[2].';$this->_global_vars_[\''.$matches[3].'\'] = $'.$matches[3].'; ?>';
		return $result.'$'.$matches[2].'){ $cyclecounter'.$this->_cycle_counter_.'++;$this->_global_vars_["counter"] = $cyclecounter'.$this->_cycle_counter_.'; $this->_cycleodd_ = ($cyclecounter'.$this->_cycle_counter_.' % 2 <> 0)?true:false; $this->pushContext($'.$matches[2].'); $this->_global_vars_[\''.$matches[2].'\'] = $'.$matches[2].'; ?>';
	}
	
	private function _counter_iter_compile($matches){
		$this->_cycle_counter_++;
		$result = '<?php $this->_global_vars_["iterlevel"] = (!isset($this->_global_vars_["iterlevel"]))?0:$this->_global_vars_["iterlevel"]+1; $cyclecounter'.$this->_cycle_counter_.' = '.$this->_compile_expression($matches[1],true).' - 1;$cyclebound'.$this->_cycle_counter_.' = '.$this->_compile_expression($matches[2],true).'; while ($cyclecounter'.$this->_cycle_counter_.' + 1 <= $cyclebound'.$this->_cycle_counter_.'){$cyclecounter'.$this->_cycle_counter_.'++;';
		return $result.' $this->_global_vars_["counter"] = $cyclecounter'.$this->_cycle_counter_.'; $this->_cycleodd_ = ($cyclecounter'.$this->_cycle_counter_.' % 2 <> 0)?true:false; $this->pushContext($this->currentContext()); ?>';
	}
	
	protected function preCompile($text){
		return $text;
	}
	
	private function _compile_oper($matches){
		switch ($matches[1]){
			case 'if':
			case 'ifnot':{
				$n = count($matches);
				$not = $matches[1] == 'ifnot'?'!':''; 
				if ($n > 4){
					$oper = trim($matches[3]);
					if (!$oper)
						$oper = $not?('!='):'==';
					
					$result = '('.$this->_compile_expression($matches[2]).' '.$oper.' '.$this->_compile_expression($matches[4],true).')';
					
					if ($not && $oper != '==' && $oper != '!=')
						$result = '(!'.$result.')';				
					return '<?php if '.$result.' { ?>'; 
				} /*else if ($n > 3)
					return '<?php if ('.$this->_compile_expression($matches[2]).' '.$not.'= '.$this->_compile_expression($matches[3],true).') { ?>';*/ 
				else
					return '<?php if ('.$not.$this->_compile_expression($matches[2]).') { ?>';
			}break;
			case 'is':return '<?php $__c__n__ = '.$this->_compile_expression($matches[2],true).'; if ($this->CurrentContext() instanceof $__c__n__) { ?> ';break;
			case 'switch':return '<?php switch ('.$this->_compile_expression($matches[2]).') {';break;
			case 'case':return ' case '.$this->_compile_expression($matches[2],true).':{ ?>';break;
			case 'with':return '<?php $this->pushContext('.$this->_compile_expression($matches[2]).'); ?>';break;
		}
	}
	
	
	private function _compile($filename,$preprocessor = null){
		if (basename($filename) == $filename)
			$filename = $this->TemplateDir."/".$filename;
		$text = file_get_contents($filename);
		if ($preprocessor instanceof ITemplatePreProcessor)
			$text = $preprocessor->PreProcess($text);
			
		$expr = '[_\w][_\w:]*|\$[_\w]+(?:\[[^\]]*\])*|`[^`]*`|\d+(?:\.\d+)?';
						
		$text = $this->preCompile($text);
		$text = preg_replace('/{%startcache\s+([_\w]+)\s+(\d+)\s*%}/','<?php if ($this->startCache("$1",$2)) { ?>',$text);
		$text = preg_replace('/{%stopcache%}/','<?php $this->saveCache(); } ?>',$text);		
		$text = preg_replace_callback('/{%insert\s+([^\s]*?|`[^`]*`)(?:\s+context\s*=\s*([^\s]*?|`[^`]*`))?\s*%}/',array(&$this,"_repl_compile"),$text);
		$text = preg_replace_callback('/{%widget\s+([^\s]*?|`[^`]*`)(?:\s+skin\s*=\s*([^\s]*?|`[^`]*`))?\s*%}/',array(&$this,"_widg_compile"),$text);
		$text = preg_replace('/<\?(?!php)/','<?php ',$text);
			
		$text = preg_replace_callback('/{%iterate\s+('.$expr.')\s+as\s+([_\w]+)(?:\s*>\s*([_\w]+))?\s*%}/',array(&$this,"_iter_compile"),$text);
		$text = preg_replace_callback('/{%iterate\s+('.$expr.')\s+>\s+('.$expr.')\s*%}/',array(&$this,"_counter_iter_compile"),$text);
		
		$text = preg_replace('/{%iterate%}/','<?php $this->popContext(); } $this->_global_vars_["iterlevel"]--; ?>',$text);

		$text = preg_replace_callback('/{%(if(?:not)?|is)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(if(?:not)?)\s+('.$expr.')(?:\s+(<=?|>=?)\s+|\s+)?('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
	/*	$text = preg_replace_callback('/{%(ifnot)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(ifnot)\s+('.$expr.')\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(is)\s+('.$expr.')\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
	*/	
		$text = preg_replace('/{%odd%}/','<?php if ($this->_cycleodd_) { ?>',$text);
		
		$text = preg_replace_callback('/{%(with)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(switch)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		$text = preg_replace_callback('/{%(case)\s+('.$expr.')\s*%}/',array($this,'_compile_oper'),$text);
		
		$text = preg_replace('/{%default%}/',' default:{ ?>',$text);		
		$text = preg_replace('/{%else%}/','<?php } else { ?>',$text);
		$text = preg_replace('/{%endis%}/','<?php } ?>',$text);
		$text = preg_replace('/{%endif%}/','<?php } ?>',$text);
		$text = preg_replace('/{%endodd%}/','<?php } ?>',$text);
		$text = preg_replace('/{%endcase%}/','<?php }break; ',$text);
		$text = preg_replace('/{%endswitch%}/',' } ?>',$text);
		$text = preg_replace('/{%endwith%}/','<?php $this->popContext(); ?>',$text);
		
		$text = preg_replace_callback('/{%assign\\s+([_\w]+)(\s+(?:(?:[_\w][_\w]*\s*\<\s*))+)?\s+('.$expr.')\s*%}/',array(&$this,"_assign_compile"),$text);
		$text = preg_replace_callback('/{%assign\\s+([_\w]+)\s+('.$expr.')\s*(\+|-|\*|\/|%)\s*('.$expr.')\s*%}/',array(&$this,"_arith_compile"),$text);
		$text = preg_replace_callback('/{(@?)%\s*(\s+(?:(?:[_\w][_\w]*\s*\<\s*))+)?('.$expr.')\s*%}/',array(&$this,"_varout_compile"),$text);
		$text = preg_replace_callback('/{%\s*(\s+(?:(?:[_\w][_\w]*\s*\<\s*))+)(.*?)\s*%}/',array(&$this,"_rawout_compile"),$text);
		$text = preg_replace('/{%\s*(.*?)\s*%}/','<?php echo $1; ?>',$text);
		/* $text = preg_replace('/\?>\s*<\?php/m','',$text); */
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
			
		$this->_global_vars_['SERVICE'] = $this->Application()->CurrentService();
		$this->_global_vars_['PAGE'] = $this->response;
		$this->_global_vars_['THEME_URL'] = $this->ThemeUrl();
		$this->_global_vars_['APP_URL'] = $this->Application()->Url();
		$this->_global_vars_['APPLICATION'] = $this->Application();
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
			$cf = str_ireplace($this->TemplateDir,$this->CompileDir,$filename);
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
	
	public function __set($nm,$value){
		switch ($nm){
			case 'ThemesDir':{
				$this->_themes_dir_ = $this->Application()->AdjustPath($value);
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case 'ThemesDir':return $this->_themes_dir_;break;
			case "TemplateDir":return $this->ThemeDir().'/templates';break;
			case "CompileDir":return $this->ThemeDir().'/compile';break;
			default:{
				if (isset($this->_global_vars_[$nm]))
					return $this->_global_vars_[$nm];
				return parent::__get($nm);
			}break;
		}
	}
	
	public function ThemeUrl(){
		$theme_dir = $this->ThemeDir();
		$theme_dir = preg_replace('/[\\\\\\/]/','|',$theme_dir);
		$docroot = preg_replace('/[\\\\\\/]/','|',realpath($_SERVER['DOCUMENT_ROOT']));
		return str_replace('|','/',str_ireplace($docroot,'',$theme_dir));
	}
	
	private function _theme_dir($nm){
		$themes_dir = $this->Application()->AppPath().'/themes';
		if ($this->ThemesDir)
			$themes_dir = $this->ThemesDir;
		if (!is_dir($themes_dir))
			mkdir($themes_dir);
		return $themes_dir."/".$nm;
	}
	
	public function ThemeDir(){
		return $this->_theme_dir($this->ThemeName);		
	}	

	/**
	 * @param string $themename
	 * @return bool
	 */
	public function ThemeExists($themename){
		return file_exists($this->_theme_dir($themename));
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