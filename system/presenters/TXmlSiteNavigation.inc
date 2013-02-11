<?php
/**
 * @package Presenters
 * @subpackage Xml
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


/**
 * @ignore
 */
require_once 'presenters/TXmlMenuSource.inc';


/**
 * @package Presenters
 * @subpackage Xml
 * @category System
 * class of navigation link
 */
class TXmlStrictLinkMenuItem extends TXmlLinkMenuItem  {
	public function Url(){
		$page = $this->node->getAttribute("page");
		if ($page)
			return $this->source->Application()->CurrentService()->Url($page,$this->node->getAttribute("controller"),$this->node->getAttribute("method"),$this->Parameters());
		return $this->node->getAttribute("url");
	}
	
	public function Page(){
		return $this->node->getAttribute("page");
	}
	
/**
 * @see ILink::Parameters()
 * @return array
 */	
	public function Parameters(){
		$result = array();
		$attributes = $this->node->attributes;
		$check_array = array("service","title","url","page","controller","method","check_access");
		foreach ($attributes as $name=>$node)
			if (!in_array($name,$check_array))
				$result[$name] = html_entity_decode($node->nodeValue);		
		return $result;
	}

	public function __toString(){
		return $this->Title();
	}
	
	public function Soid(){
		return $this->Url();
	}
}


/**
 * @package Presenters
 * @subpackage Xml
 * @category System
 * navigation structure obtained from navigation configuration file  
 */
class TXmlSiteNavigation extends TXmlMenuSource {
/**
 * gets top level of links
 */	
	public function Roots(){
		$result = array();
		if ($this->response->Service() instanceof TIntrinsicPageService){
			$file = $this->Application()->PrivateDir()."/".$this->response->Service()->Name()."/navigation.xml";
			if (file_exists($file)){
				$dom = new DOMDocument();
        		$dom->Load($file);
	        	//if ($dom->schemaValidate($this->Application()->SysPath()."/schemas/navigation.xsd")){
    	        $xpath = new DOMXPath($dom);
				$links = $xpath->query("/root/link|group");
				foreach ($links as $link)
					$result[] = $this->CreateItem($link, $xpath); 
				//}            		
			}	
		}
		return $result;
	}	
	
	public function CreateItem(DOMNode $node, DOMXPath $xpath){
		switch ($node->tagName){
			case "link":return new TXmlStrictLinkMenuItem($node,$xpath,$this);break;
			default:return parent::CreateItem($node, $xpath);
		} 
	}
}
?>