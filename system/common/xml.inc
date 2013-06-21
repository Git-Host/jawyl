<?php

interface IXSLTransformer {
/**
 * @param string $xml
 * @return string
 */	
	public function TransformXML($xml);
/**
 * @param DOMDocument $xml
 * @return DOMDocument 
 */	
	public function TransformDOM(DOMDocument $xml);
/**
 * @param DOMDocument $xml
 * @return string 
 */	
	public function STransformDOM(DOMDocument $xml);
}

class TXSLTransformer extends TConfigurable implements IXSLTransformer {
/**
 * @var DOMDocument
 */		
	private $_xsl_;
/**
 * @var XSLTProcessor
 */	
	private $_processor_;
	private $_use_php_functions_ = false;
	private $_parameters_ = array();
	
	public function __construct($name, IConfigurable $container = null, IApplication $application = null){
		parent::__construct($name,$container,$application);
		$this->_xsl_ = new DOMDocument();
		$this->_processor_ = new XSLTProcessor();	
	}
	
	
	public function __set($nm,$value){
		switch ($nm){
			case 'XSL':{  
				$this->_xsl_->load($this->Application()->AdjustPath($value));
			}break;
			case 'Parameters':{
				if (is_string($value))
					$value = TConvertions::ConvertToArray($value,array(';','='),array(false,true));
				if (is_array($value))
					$this->_parameters_ = $value + $this->_parameters_;
			}break;
			case 'UsePHP':$this->_use_php_functions_ = TConvertions::ConvertToBoolean($value);break;
			default:parent::__set($nm, $value);break;
		}
	}
	
/**
 * @param string $xml
 * @return string
 */	
	public function TransformXML($xml){
		$temp = new DOMDocument();
		$temp->loadXML($xml);
		return $this->STransformDOM($temp);
	}
	
	private function _setup_transform(){
		$this->_processor_->importStyleSheet($this->_xsl_);
		$this->_processor_->setParameter('', $this->_parameters_);
		if ($this->_use_php_functions_)
			$this->_processor_->registerPHPFunctions();
	}
	
/**
 * @param DOMDocument $xml
 * @return DOMDocument 
 */	
	public function TransformDOM(DOMDocument $xml){
		$this->_setup_transform();
		return $this->_processor_->transformToDoc($xml);
	}
/**
 * @param DOMDocument $xml
 * @return string 
 */	
	public function STransformDOM(DOMDocument $xml){
		$this->_setup_transform();
		return $this->_processor_->transformToXml($xml);			
	}
}