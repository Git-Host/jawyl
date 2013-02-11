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
 * navigation structure obtained from navigation configuration file  
 */
class TXmlWizardProvider extends TPresenter implements IWizardProvider {
	private $_dom_;
	
	private $_current_step_;
	private $_step_count_ = null;
	private $_finished_ = false;
	
	public function __construct($name,TIntrinsicPage $response, array $settings = array()){
		$this->_dom_ = $response->XmlData($name);
		parent::__construct($name,$response,$settings);
		$this->_current_step_ = $this->loadStateParameter('current_step');
	}
	
/** 
 * @return boolean
 */		
	public function StepBack(){
		if ($this->_current_step_ > 0){
			$this->_current_step_--;
			$this->saveStateParameter('current_step', $this->_current_step_);
		}	
	}
	
	private function _steps_count(){
		if (is_null($this->_step_count_)){
			$xpath = new DOMXPath($this->_dom_);
			$steps = $xpath->query("step");
			$this->_step_count_ = $steps->length;
		}
		return $this->_step_count_;
	}
	
/**
 * @param array $parameters
 * @return boolean
 */		
	public function StepNext(array $parameters = array()){
		$xpath = new DOMXPath($this->_dom_);
		$params = $xpath->query("step[position()=$this->_current_step_]/parameter");
		
		$stored = $this->Application()->Session()->Get($this->Name().'_wizparams');
		$needperform = false;
		
		foreach ($params as $p)
			if ($stored[$p->getAttribute('action')][$p->getAttribute('name')] != $parameters[$p->getAttribute('action').'_'.$p->getAttribute('name')]){
				$needperform = true;
				$stored[$p->getAttribute('action')][$p->getAttribute('name')] = $parameters[$p->getAttribute('action').'_'.$p->getAttribute('name')];
			}
			
		$this->Application()->Session()->Set($this->Name().'_wizparams',$stored);
		
		if ($needperform){
			$perform = $xpath->query("step[position()=$this->_current_step_]");
			foreach ($perform as $p){
				$a = $this->response->Instance($p->getAttribute('action'));
				if ($a instanceof IAction)
					$a->Execute($stored[$p->getAttribute('action')]);
			}
		}
		
		if ($this->IsLastStep())
			$this->_finished_ = true;
		else {
			$this->_current_step_++;
			$this->saveStateParameter('current_step', $this->_current_step_);
		}
	}
/**
 * @return IActionParameter[]
 */		
	public function StepParameters(){
		$xpath = new DOMXPath($this->_dom_);
		$params = $xpath->query("step[position()=$this->_current_step_]/parameter");
		$stored = $this->Application()->Session()->Get($this->Name().'_wizparams');
		$result = array();
		foreach ($params as $p){
			$action = $this->response->Instance($p->getAttribute('action'));
			$v = null;
			if (isset($stored[$p->getAttribute('action')]))
				if (isset($stored[$p->getAttribute('action')][$p->getAttribute('name')]))
					$v = $stored[$p->getAttribute('action')][$p->getAttribute('name')];
			if ($action instanceof IAction){
				$p = new TActionParameter($p->getAttribute('action').'_'.$p->getAttribute('name'), $p->getAttribute('title'), $p->getAttribute('type'),$v);
				$action->AddParameter($p);
				$result[] = $p;
			}
		}
		return $result;
	}
/**
 * @return boolean
 */		
	public function BackAllowed(){
		$xpath = new DOMXPath($this->_dom_);
		$step = $xpath->query("step[position()=$this->_current_step_]");
		$deprecate_back = false;
		foreach ($step as $s)
			$deprecate_back = $s->getAttribute('nostepback') == "true";
		return ($this->_current_step_ > 0) && !$deprecate_back;
	}
/**
 * 
 * @return boolean
 */		
	public function Finished(){
		return $this->_finished_;
	}		
	
	public function IsLastStep(){
		return $this->_current_step_ >= $this->_steps_count() - 1;
	}
}