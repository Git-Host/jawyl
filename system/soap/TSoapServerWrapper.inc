<?php
require_once 'common/soap.inc';

/** 
 * Soap component wrapper
 * @author dakrasilnikov
 * @property TConfigurable $Component
 */

class TSoapServerWrapper extends TSoapServer {
/**
 * @var TConfigurable
 */	
	protected $_ioc_component_;
	
	protected function getHandler(){
		return $this->_ioc_component_;
	}
}