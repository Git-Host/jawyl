<?php
TApplication::Import('common::soap');

/**
 * 
 * Soap gateway class
 * @author dakrasilnikov
 * @property array $LastCallResults
 * @property string|TSoapClient $Endpoint
 */

final class TSoapGateway extends TSoapServer {
	private $_client_counter_ = 0;
	
	private $_endpoints_ = array();
	
	private $_last_call_results_ = array(); 
	
	public function __get($nm){
		switch ($nm){
			case 'LastCallResults':return $this->_last_call_results_;break;
			default:return parent::__get($nm);break;
		}
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'Endpoint':{
				if (is_string($value)){
					$com = TApplication::Instance($value);
					if ($com instanceof TSoapClient)
						$value = $com;
					else {
						$this->_client_counter_++;
						$com = new TSoapClient($this->name.'_client'.$this->_client_counter_,$this);
						$com->WSDL = $value;
						$value = $com;
					} 	
				}
/*
 * @todo check WSDL compatibility 
 */				
				if ($value instanceof TSoapClient)
					$this->endpoint[] = $value;
				else throw new TCoreException(TCoreException::ERR_BAD_VALUE);	
			}break;
			default:parent::__set($nm, $value);break;
		}
	}
	
	protected function compiledWsdlPath(){
/*
 * @todo Implement WSDL substitution
 **/		
		
	}
	
	public function __call($name,$arguments){
		$this->_last_call_results_ = array();
		foreach ($this->_endpoints_ as $c){
			$result = $c->Call($name,$arguments);
			$this->_last_call_results_[] = $result;
		}
		return $result;
	}
}