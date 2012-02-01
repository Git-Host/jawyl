<?php
/**
 * @package Controllers
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

	TApplication::import("common::mvc");
	TApplication::import("common::controller::events");
	
	interface IItemAccessor {
/**
 * gets list item by id
 * @param mixed $id item id
 * @return IItem
 */	
		public function Item($id);		
	}
	
/**
 * @package Controllers
 * @category System
 * interface for implementing item lists 
 */		
	interface IItemsList {
/**
 * gets controller items list. If page is specified then a portion of list is returned
 * @param int $page page number
 * @return IIterator iterator of IItem
 * @see IIterator
 * @see IItem
 */		
		public function Items($page = null);
/**
 * makes an item with specified $id selected
 * @param mixed $id item id 
 */		
		public function SelectItem($id);
/**
 * return current selected item
 * @return IItem
 * @see IItem
 */
		public function CurrentItem();	
	}
	
	interface IItemsCollection {
		public function AddItem($id);
		public function RemoveCurrentItem();
	}
	
	
/**
 * @package Controllers
 * @category System
 * interface for implementing item management controllers
 */	
	interface IItemsSource extends IItemsList {
/**
 * Get current item property value.
 * @param string $name property name
 * @return mixed
 * @see IItem
 */		
		public function CurrentItemPropertyValue($name);
/**
 * Create new item. Return created item.
 * @param TCreateInfo $createinfo 
 * @return IItem
 * @see TCreateInfo
 * @see IItem 
 */		
		public function CreateItem(TCreateInfo $createinfo = null);
/**
 * Deletes current item. Return true on success.
 * @return boolean
 */		
		public function DeleteCurrentItem();
/**
 * Edit current item. Returns true on success.
 * @param string $attrname name of property to edit
 * @param mixed $value new value of property
 * @return boolean
 */
		public function EditCurrentItem($attrname,$value);
		public function EditTheItem($id,$attrname,$value);
	}	 
	
/**
 * @package Controllers
 * @category System
 * interface for implementing item structures
 */	
	interface IItemsStructure extends IItemsSource {
/**
 * gets parent item subitems list. If page and pagesize are specified then a portion of list is returned
 * @param mixed $parent parent item id
 * @param int $page page number
 * @param int $pagesize page size 
 * @return IIterator iterator of IItem
 * @see IIterator
 * @see IItem 
 */		
		public function SubItems($parent,$page = null, $pagesize = null);
/**
 * Make item be subitem of another item (parent). Return true on success
 * @param mixed $id item id
 * @param mixed $parent parent item id
 * @return boolean 
 */
		public function MoveItem($id,$parent);
/**
 * Get item parent. If item has no parent return null
 * @param mixed $id item id
 * @return IItem
 * @see IItem
 */
		public function ItemParent($id); 
	}

/**
 * @package Controllers
 * @category System
 * interface for implementing sortable item lists and structures 
 */	
	interface ISortableItems {
/**
 * gets controller sorting action or null if controller has no sorting. 
 * @return TControllerAction
 * @see TControllerAction
 */		
		public function SortingAction();
	}

/**
 * @package Controllers
 * @category System
 * interface for implementing filterable item lists and structures 
 */	
	interface IFilterableItems {
/**
 * gets controller filtering action or null if controller has no filtering. 
 * @return TControllerAction
 * @see TControllerAction
 */		
		public function FilterAction();
	}
	
/**
 * @package Controllers
 * @category System
 * interface for implementing paged list and structure controllers
 */	
	interface IPagedItems {
/**
 * Open a specified page
 * @param int $page page number
 */		
		public function OpenPage($page);
/**
 * Get controller pages count
 * @param int $pagesize size of page
 * @return int
 */		
		public function PageCount();
/**
 * Get current page items
 * @param int $pagesize size of page
 * @return IIterator iterator of IItem
 * @see IIterator
 * @see IItem 
 */		
		public function CurrentPage();
/**
 * Get current page number
 * @return int
 */		
		public function CurrentPageNumber();
		
		public function PageSize();
	}

/**
 * @package Controllers
 * @category System
 * interface for implementing nested item lists
 */	
	interface INestedItems extends IItemsSource {
/**
 * Open item. Makes controller list limited to children of specified item.
 * Created items also become children of specified item. Return true on success.
 * @param mixed $id item id
 * @return boolean
 */		
		public function OpenItem($id);
/**
 * Open parent of current opened item. Return true on success.
 * @return boolean. 
 */		
		public function LevelUp();
/**
 * Get current opened item
 * @return IItem
 */		
		public function CurrentContainer();
		
		public function ItemContainers(IItem $item);
	}

/**
 * @package Controllers
 * @subpackage Items
 * @category System
 * interface for implementing link items
 */	
	interface ILink {
/**
 * get link title
 * @return string
 */		
		public function Title();
/**
 * get link service name
 * @return string 
 */		
		public function Service();
/**
 * get link parameters
 * @return array associative parameters array
 */		
		public function Parameters();
		public function Url();
	}
	
	interface IParent {
		public function Children();
	}
	
/**
 * @package Controllers
 * @subpackage Items
 * @category System
 * interface for implementing option items
 */	
	interface IOption {
/**
 * get option id
 * @return mixed
 */		
		public function Id();
/**
 * get option name
 * @return string 
 */		
		public function Name();
/**
 * get whether option is checked
 * @return boolean
 */		
		public function Checked();
	}
	
/**
 * @package Controllers
 * @category System
 * interface for implementing options lists
 */	
	interface IOptionsList {
/**
 * get options list
 * @return IOption[]
 */		
		public function Options();
/**
 * check option
 * @param mixed $id option id
 */		
		public function CheckOption($id);
/**
 * get whether option is checked
 * @param mixed $id option id
 * @return boolean
 */		
		public function IsChecked($id);
/**
 * get whether list can be accessed
 * @return boolean
 */		
		public function Enabled();
	}
	
	interface IMenuStructure {
		public function Roots();
	/*	public function Children($id);*/
	}
	
	interface IWizardProvider {
/** 
 * @return boolean
 */		
		public function StepBack();
/**
 * @param array $parameters
 * @return boolean
 */		
		public function StepNext(array $parameters = array());
/**
 * @return IActionParameter[]
 */		
		public function StepParameters();
/**
 * @return boolean
 */		
		public function BackAllowed();
/**
 * @return boolean
 */		
		public function Finished();
/**
 * @return boolean
 */		
		public function IsLastStep();
	}
?>