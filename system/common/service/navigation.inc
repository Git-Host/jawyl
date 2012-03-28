<?php
/**
 * @package Services
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/	

/**
 * interface for implementing services that can navigate throw unique resources
 */
	interface IItemNavigation {
/**
 * gets specified resource url
 * @return string
 */		
		public function ItemNavigationUrl(IItem $item = null, $controller = null, $method = null, array $parameters = array());
	}
?>