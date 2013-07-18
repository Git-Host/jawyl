<?php
require_once 'common/soap.inc';

/** 
 * Soap client class
 * @author dakrasilnikov
 * @property string $WSDL
 */

class TSoapClient extends TConfigurable {
/**
 * @var TSoapOptions
 */	
	protected $options;
	
	protected $wsdl;
	
	protected function afterConstruct(){
		$this->options = new TSoapOptions();
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'WSDL':$this->wsdl = $this->Application()->AdjustPath($value);break;
			default:{
				if ($this->options->IsOption($nm))
					$this->options->$nm = $value;
				else
					parent::__set($nm, $value);
			}break;
		}
	}
	
	public function __call($name, array $arguments){
		return $this->Call($name, $arguments);
	}
	
	protected function getSoapHandler(){
		return new SoapClient($this->wsdl,$this->options->ForConstructor(TSoapOptions::TYPE_CLIENT)); 
	}
	
	public function Call($name,array $arguments){
		$c = $this->getSoapHandler();
		return $c->__soapCall($name,$arguments);
	}
}