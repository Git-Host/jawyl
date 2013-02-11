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
require_once 'common/mvp.inc';
/**
 * @ignore
 */
require_once 'common/presenter/basic.inc';
/**
 * @ignore
 */
require_once 'services/TIntrinsicPageService.inc';

/**
 * @package Presenters
 * @subpackage Xml
 * @category System
 * Xml link class
 */
class TXmlLink implements ILink {
	private $_node_;
/**
 * constructor
 */	
	public function __construct(DOMNode $node){
		$this->_node_ = $node;
	}
/**
 * @see ILink::Title()
 * @return string
 */	
	public function Title(){
		return $this->_node_->nodeValue;
	}
/**
 * @see ILink::Service()
 * @return string
 */
	public function Service(){
		return $this->_node_->getAttribute("service");
	}
/**
 * @see ILink:;Parameters()
 * @return string
 */	
	public function Parameters(){
		$result = array();
		$attributes = $this->_node_->attributes;
		foreach ($attributes as $name=>$node)
			if ($name != "service")
				$result[$name] = $node->nodeValue;		
		return $result;
	}
}

/**
 * @package Presenters
 * @subpackage Xml
 * @category System
 * List of links obtained from presenter content in page configuration file
 */
class TXmlLinks extends TPresenter {
	private $_dom_;
/**
 * constructor
 * @param string $name
 */	
	public function __construct($name,TIntrinsicPage $response, array $settings = array()){
		parent::__construct($name,$response,$settings);
		$this->_dom_ = $response->XmlData($name);
	}

/**
 * gets links
 * @return array array of ILink
 */	
	public function Items(){
		$result = array();
		if ($this->_dom_ instanceof DOMDocument){
			$xpath = new DOMXPath($this->_dom_);
			$links = $xpath->query("link");
			foreach ($links as $link)
				$result[] = new TXmlLink($link);
		} 
		return $result;
	}
}
?>