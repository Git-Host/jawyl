<?php
/**
 * @package View
 * @subpackage Widgets
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
TApplication::import("common::mvcEvents");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * abstract class for implementing widgets interacting with controller, which is treated as datasource.
 * @property TController $Source a controller to interact with. Can be specified by assigning controller name or controller itself. 
 */
abstract class TSourcedWidget extends TWidget  {
/**
 * @ignore
 */
	private $_source_ = array();

	protected function checkSource($src){
		return $src instanceof TMVCObject;
	}
	
	protected function checkEvent(TEvent $event){
		return in_array($event->Sender,$this->_source_);
	}

/**
 * @ignore
 */
	public function __get($nm){
		switch ($nm){
			case "Source":return $this->getIocArrayMember($this->_source_, array(&$this,"checkSource"), null,true);break;
			case "Sources":return $this->getIocArrayMember($this->_source_, array(&$this,"checkSource"));break;
			default:return parent::__get($nm);break;
		}
	}

/**
 * @ignore
 */
	public function __set($nm,$val){
		switch ($nm){
			case "Source":$this->setIocArrayMember($this->_source_, $val, array(&$this,"checkSource"));break;
			default:parent::__set($nm,$val);break;
		}
	}	
}
?>