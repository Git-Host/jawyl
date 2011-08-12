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
	TApplication::import("widgets::TJQueryWidget");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * 
 * widget containing other widgets used by TIntrinsicPageService
 * @see TIntrinsicPageService
 */	
	abstract class TXmlWidgetContainer extends TJQueryWidget {
		private $_node_;
/**
 * constructor
 * @param string $name
 */		
		public function __construct($name,TResponse $response, array $settings = array()){
			parent::__construct($name,$response,$settings);
			if (!($response instanceof TIntrinsicPage))
				throw new TCoreException(TCoreException::ERR_NOT_CLASS,array("class"=>"TIntrinsicPage"));
			$this->_node_ = $response->getNode($name);
		}		

/**
 * get title for widget with specified name
 * @param string $name
 * @return string
 */		
		public function Title($name){
			$xpath = new DOMXPath($this->_node_->ownerDocument);
			$titles = $xpath->query("data/titles/title[@widget='".$name."']",$this->_node_);
			if ($titles->length > 0)
				return html_entity_decode($titles->item(0)->nodeValue); 
			return "";
		}

/**
 * @see TJQueryWidget::linkCustomResources()
 */		
		protected function linkCustomResources(){}		
	}
?>