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
TApplication::import("common::ajax");
/**
 * @ignore
 */
TApplication::import("widgets::areas::TAreasNestedTable");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * nested table widget
 */
class TAreasPagedTable extends TAreasNestedTable {
	
	public function __construct ($name,TResponse $response, array $settings = array()){
		parent::__construct($name,$response,$settings);
		$this->allow_open = false;
	}
	
	protected function checkSource($src){
		return ($src instanceof IPagedItems) && parent::checkSource($src); 
	}

	protected function isSublevel(){
		return false;
	}
}
?>