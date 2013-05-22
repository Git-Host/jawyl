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
	require_once 'widgets/TJQueryWidget.inc';

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * 
 * widget containing other widgets used by TIntrinsicPageService
 * @see TIntrinsicPageService
 */	
	abstract class TWidgetContainer extends TJQueryWidget {
		
		private $_titles_ = array();
		
		public function __set($nm, $val){
			switch($nm){
				case 'Titles':
					$this->_titles_ = TConvertions::ConvertToArray($val,array(',','='),array(false, true));
				break;
				
				default: parent::__set($nm, $val);	break;
			}
			
		}
/**
 * get title for widget with specified name
 * @param string $name
 * @return string
 */		
		public function Title($name){
			return (isset($this->_titles_[$name]))? $this->_titles_[$name]: 'Tab';
		}
	}