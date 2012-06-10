<?php
TApplication::Import('common::soap');

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
	
	public function __construct($name, IConfigurable $container = null){
		parent::__construct($name,$container);
		$this->options = new TSoapOptions();
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'WSDL':$this->wsdl = TApplication::$Application->AdjustPath($value);break;
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