<?php
/**
 * @package I18n
 * @subpackage Xml
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package I18n
 * @subpackage Xml
 * @category System
 * translator using xml files to store message domains
 */
	class TXmlTranslator extends TTranslator {
/**
 * @var array loaded xml message domains
 */		
		protected $domains;
/**
 * searches for message translation in loaded text domains
 * @param string $domain
 * @param string $text
 * @param int $plural
 * @return string
 */
		protected function domain_search($domain,$text,$plural = null){
			if (key_exists($domain,$this->domains)){
				$v = include $this->domains[$domain];
				if (is_string($v)) return $v;
			}
			return $text;
		}
		
/**
 * @see ITranslator::Translate()
 * @param string $domain
 * @param string $text
 * @param int $plural
 * @return string
 */		
		public function Translate($domain,$text, $plural = null){
			if (is_object($text))
				$text = (string)$text;
			if (!is_string($text)) return $text;
			if (is_null($domain)){
				$tr = $text;
				$names = array_keys($this->domains);
				foreach ($names as $d) {
					$tr = $this->domain_search($d,$text,$plural);
					if ($tr != $text) break;
				}
				return $tr;
			}
			return $this->domain_search($domain,$text,$plural); 
		}
		
		private function _compile_domain($domain,$xml){
			$fn = $this->Application()->ConfCompileDir().'/languages/'.$domain.'.php';
			
			if (!file_exists($fn) || filemtime($fn) < filemtime($xml)){
				$dom = new DOMDocument();
				$dom->Load($xml);
				TFileSystem::ForceDir($this->Application()->ConfCompileDir().'/languages');
				$xpath = new DOMXPath($dom);
				$msgs = $xpath->query('/domain/msg');
				$script = "<?php\n";
				foreach ($msgs as $msg){
					$trans = $xpath->query("default",$msg);
					foreach ($trans as $tr)
						$script	.= 'if (($text === "'.addslashes(html_entity_decode($msg->getAttribute("id"))).'") && ($plural < 2 || is_null($plural))) return "'.addslashes($tr->nodeValue)."\";\n";	 
					$trans = $xpath->query("plural",$msg);
					foreach ($trans as $tr)
						$script	.= 'if (($text === "'.addslashes(html_entity_decode($msg->getAttribute("id"))).'") && ($plural >= '.$tr->getAttribute("from").') && ($plural <= '.$tr->getAttribute("to").')) return "'.addslashes($tr->nodeValue)."\";\n";	 
				}
				file_put_contents($fn, $script);
			}
			return $fn;
		}
		

/**
 * @see ITranslator::SetLanguage()
 * @param string $language
 */			
		public function SetLanguage($language){
			$dir = null;
			if (is_dir($this->Application()->PrivateDir()."/languages/".$language))
				$dir = $this->Application()->PrivateDir()."/languages/".$language;
			$this->domains = array();
			if (is_dir($dir)) {
    			if ($dh = opendir($dir)) {
        			while ($file = readdir($dh)){
            			if (is_file($dir."/".$file) && ($file != "..") && ($file != ".")){
            				$pi = pathinfo($dir."/".$file);
            				if ($pi["extension"] == "xml"){
            					$domainname = basename($file, ".xml");
            					$this->domains[$domainname] = $this->_compile_domain($domainname,$dir."/".$file);
            				}
        				}
        			}
        			closedir($dh);
    			}
			}		
		}
	}

?>