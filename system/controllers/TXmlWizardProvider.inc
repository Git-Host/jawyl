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
class TXmlWizardProvider extends TController implements IWizardProvider {
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
		
		$stored = TApplicationSession::Get($this->Name().'_wizparams');
		$needperform = false;
		
		foreach ($params as $p)
			if ($stored[$p->getAttribute('action')][$p->getAttribute('name')] != $parameters[$p->getAttribute('action').'_'.$p->getAttribute('name')]){
				$needperform = true;
				$stored[$p->getAttribute('action')][$p->getAttribute('name')] = $parameters[$p->getAttribute('action').'_'.$p->getAttribute('name')];
			}
			
		TApplicationSession::Set($this->Name().'_wizparams',$stored);
		
		if ($needperform){
			$performed = TApplicationSession::Get($this->Name().'_wizperformed');
			
			if (isset($performed[$this->_current_step_])){
				$rollback = $xpath->query("step[position()=$this->_current_step_]/rollback/actions");
				if ($rollback->length == 0)
					throw new Exception('Action was already performed and can not be rollbacked!');

				foreach ($rollback as $rb){
					$a = TConfigurable::Instance($rb->getAttribute('name'));
					if ($a instanceof IAction)
						$a->Execute();
				}
			}
			
			$perform = $xpath->query("step[position()=$this->_current_step_]/perform/actions");
			foreach ($perform as $p){
				$a = TConfigurable::Instance($p->getAttribute('name'));
				if ($a instanceof IAction)
					$a->Execute($stored[$p->getAttribute('name')]);
			}
			
			$performed[$this->_current_step_] = true;
			TApplicationSession::Set($this->Name().'_wizperformed', $performed);
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
		$stored = TApplicationSession::Get($this->Name().'_wizparams');
		$result = array();
		foreach ($params as $p){
			$action = TConfigurable::Instance($p->getAttribute('action'));
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
		return $this->_current_step_ > 0;
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