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
TApplication::import("common::widget::interfaces");
/**
 * @ignore
 */
TApplication::import("widgets::areas::TAreasWidget");


/**
 * @package View
 * @subpackage Widgets
 * @category System
 * 
 * @property int $FocusLowBound
 * @property int $FocusHightBound
 * 
 * widget supporting focus order options 
 */
abstract class TAreasFocusedWidget extends TAreasWidget {
/**
 * @var TAreasFocusedWidget
 */	
	protected $_ioc_focus_parent_;
	
	protected $focusOffset = 1;
	
	protected function focusLowBound(){
		$result = $this->focusOffset;
		if (($result >= 0) && ($this->_ioc_focus_parent_ instanceof TAreasFocusedWidget))
			$result += $this->_ioc_focus_parent_->FocusHighBound;
		return $result;
	}
	
	protected function focusHightBound(){
		return $this->focusLowBound();
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case 'FocusOffset':$this->focusOffset = TConvertions::CheckInteger($value)?$value:$this->focusOffset;break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case 'FocusLowBound':return $this->focusLowBound();break;
			case 'FocusHighBound':return $this->focusHighBound();break;
			default:return parent::__get($nm);break;
		}
	}
}