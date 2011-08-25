<?php
/**
 * @package Controllers
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
TApplication::import("common::mvc");
/**
 * @ignore
 */
TApplication::import("common::controller::interfaces");
/**
 * @ignore
 */
TApplication::import("services::TIntrinsicPageService");

/**
 * @package Controllers
 * @subpackage Xml
 * @category System
 * navigation structure obtained from navigation configuration file  
 */
class TXmlMenuSource extends TController implements IWizardProvider {
	private $_dom_;
	
	private $_current_step_;
	
	public function __construct($name,TIntrinsicPage $response, array $settings = array()){
		$this->_dom_ = $response->XmlData($name);
		parent::__construct($name,$response,$settings);
		$this->_current_step_ = $this->loadStateParameter('current_step');
    	$xpath = new DOMXPath($this->_dom_);
	}
	
/** 
 * @return boolean
 */		
	public function StepBack(){
			
	}
/**
 * @param array $parameters
 * @return boolean
 */		
	public function StepNext(array $parameters = array()){
	}
/**
 * @return IActionParameter[]
 */		
	public function StepParameters(){
		$xpath = new DOMXPath($this->_dom_);
		$params = $xpath->query("step");
		$result = array();
		foreach ($params as $p)
			$result[] = new TActionParameter($p->getAttribute('name'), $p->getAttribute('title'), $p->getAttribute('type'));
	}
/**
 * @return boolean
 */		
	public function BackAllowed(){
		return $this->_current_step_ > 0;
	}
/**
 * 
 * @return boolean
 */		
	public function Finished(){}		
}