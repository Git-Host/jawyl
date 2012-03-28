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
TApplication::import("common::mvp");
/**
 * @ignore
 */
TApplication::import("common::presenter::basic");
/**
 * @ignore
 */
TApplication::import("services::TIntrinsicPageService");


class TXmlMenuItem implements IParent {
	private $_source_;
	
/**
 * @var DOMNode 
 */	
	protected $node;
/**
 * @var array
 */	
	protected $sublinks = array();
/**
 * constructor
 */	
	public function __construct(DOMNode $node, DOMXPath $xpath, TXmlMenuSource $source){
		$this->node = $node;
		$this->_source_ = $source;
		$links = $xpath->query("link|group|action",$this->node);	
		foreach ($links as $link)
			$this->sublinks[] = $this->createSubItem($link, $xpath);
	}
	
	protected function createSubItem(DOMNode $node, DOMXPath $xpath){
			switch ($node->tagName){
				case "link":return new TXmlLinkMenuItem($node,$xpath,$this->_source_);break;
				case "group":return new TXmlGroupMenuItem($node,$xpath,$this->_source_);break;
				case "action":return TConfigurable::Instance($node->getAttribute("name"));break;
			} 
	}
	
	public function Children(){
		return $this->sublinks;	
	}

	public function Available(){
		if (TConvertions::ConvertToBoolean($this->node->getAttribute("check_access")))
			return $this->_source_->ItemAvailable($this);
		return true;
	}	
}

/**
 * @package Presenters
 * @subpackage Xml
 * @category System
 * class of navigation link
 */
class TXmlLinkMenuItem extends TXmlMenuItem implements ILink, ISecurityObject {	
/**
 * @see ILink::Title()
 * @return string
 */	
	public function Title(){
		return html_entity_decode($this->node->getAttribute("title"),ENT_QUOTES);
	}
/**
 * @see ILink::Service()
 * @return string
 */	
	public function Service(){
		$service = $this->node->getAttribute("service");
		if ((!$service) || ($service == "")) 
			return "self";
		return $service;
	}
	
	public function Url(){
		return $this->node->getAttribute("url");
	}
	
/**
 * @see ILink::Parameters()
 * @return array
 */	
	public function Parameters(){
		$result = array();
		$attributes = $this->node->attributes;
		
		$check_array = array("service","title","url","check_access");
		
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

class TXmlGroupMenuItem extends TXmlMenuItem {
	public function __toString(){
		return $this->node->getAttribute("title");
	}
}

/**
 * @package Presenters
 * @subpackage Xml
 * @category System
 * navigation structure obtained from navigation configuration file  
 */
class TXmlMenuSource extends TPresenter implements IMenuStructure, IActionsList {
	private $_dom_;
	
	public function __construct($name,TIntrinsicPage $response, array $settings = array()){
		$this->_dom_ = $response->XmlData($name);
		parent::__construct($name,$response,$settings);
	}
	
	protected function actionsList(){
		$result = array();
		if ($this->_dom_ instanceof DOMDocument){
    		$xpath = new DOMXPath($this->_dom_);
			$actions = $xpath->query("//action");
			foreach ($actions as $a)
				$result[] = TConfigurable::Instance($a->getAttribute("name"));
		}
		return $result;
	}
	
	public function ItemAvailable(TXmlMenuItem $link){
		return true;
	}
	
/**
 * gets top level of links
 */	
	public function Roots(){
		$result = array();
		if ($this->response->Service() instanceof TIntrinsicPageService){
    		$xpath = new DOMXPath($this->_dom_);
			$links = $xpath->query("link|group|action");
			foreach ($links as $link)
				switch ($link->tagName){
					case "link":$result[] = new TXmlLinkMenuItem($link,$xpath,$this);break;
					case "group":$result[] = new TXmlGroupMenuItem($link,$xpath,$this);break;
					case "action":$result[] = TConfigurable::Instance($link->getAttribute("name"));break;
				} 
		}
		return $result;
	}
	
	public function Actions(){
		return $this->actionsList();
	}
/**
 * checks whether specified action can be executed
 * @param TPresenterAction $action
 * @return boolean
 */
	public function ActionAvailable(IAction $action){
		return $action->Available();
	}
	
/**
 * gets name of action group
 * @return string
 */
	public function ActionGroup(IAction $action){
    	$xpath = new DOMXPath($this->_dom_);
		$a = $xpath->query('//action[@name="'.$action->Name().'"]');
		if ($a->length > 0){
			$g = $a->item(0)->parentNode;
			if ($g)
				if ($g->tagName == "group")
					return $g->getAttribute("title");
		}
		return $this->DefaultGroup;
	}	
}
?>