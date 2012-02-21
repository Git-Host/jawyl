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
				$xpath = new DOMXPath($this->domains[$domain]);
				$msg = $xpath->query("/domain/msg[@id='".$text."']");
				if ($msg->length > 0){
					$count = $plural;
					if (is_null($count)) $count = 1;
					if ($count == 1)
						$trans = $xpath->query("default",$msg->item(0));
					else 
						$trans = $xpath->query("plural[@from <= ".$plural." and @to >= ".$plural."]",$msg->item(0));
						
					if ($trans->length > 0)
						return $trans->item(0)->nodeValue;
				}
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

/**
 * @see ITranslator::SetLanguage()
 * @param string $language
 */			
		public function SetLanguage($language){
			$dirs = array();
			if (is_dir(TApplication::$SystemPath."/languages/".$language))
				$dirs[] = TApplication::$SystemPath."/languages/".$language;
			if (is_dir(TApplication::$Application->PrivateDir."/languages/".$language))
				$dirs[] = TApplication::$Application->PrivateDir."/languages/".$language;
			$this->domains = array();
			foreach ($dirs as $dir)
			if (is_dir($dir)) {
    			if ($dh = opendir($dir)) {
        			while ($file = readdir($dh)){
            			if (is_file($dir."/".$file) && ($file != "..") && ($file != ".")){
            				$pi = pathinfo($dir."/".$file);
            				if ($pi["extension"] == "xml"){
            					$domainname = basename($file, ".xml"); 
            					$this->domains[$domainname] = new DOMDocument();
            					$this->domains[$domainname]->Load($dir."/".$file);
            				}
        				}
        			}
        			closedir($dh);
    			}
			}		
		}
	}

?>