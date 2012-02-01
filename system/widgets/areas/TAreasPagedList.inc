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
TApplication::import("widgets::areas::TAreasList");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * paged list view widget, needs source to be IPagedItems
 * @see IPagedItems
 */
class TAreasPagedList extends TAreasList {
/**
 * @see TSourcedWidget::linkSource()
 */	
	protected function checkSource($src){
		return ($src instanceof IPagedItems) && parent::checkSource($src); 
	}
	
/**
 * gets list page count
 * @return int
 */	
	public function PageCount(){
		$pc = $this->Source->PageCount();
		if ($pc == 0) $pc = 1;
		return $pc;
	}
	
/**
 * gets current page number
 * @return int
 */	
	public function CurrentPage(){
		return $this->Source->CurrentPageNumber() + 1;
	}
/**
 * gets list orm items
 * @return IIterator
 * @see TAjaxList::getItems()
 */	
	protected function getItems(){
		if (!($this->Source instanceof IPagedItems))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		return $this->Source->CurrentPage();
	}	
}
?>