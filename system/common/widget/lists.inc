<?php
TApplication::import("common::service::navigation");
TApplication::import("common::item");
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
 * @package View
 * @subpackage Widgets
 * @category System
 * interface for implementing widgets with selectable content
 */
interface INavigable {
	public function Link();
}

interface ISelectable {
	public function SelectLink();
}

interface IList {
	public function WrapItem(IItem $item);
	public function WrapId($itemid);
	public function SelectedItem();	
	public function http_SelectItem($id);
	public function SelectionLink(IItem $item);
	public function OpenLink(IItem $item);
}

interface ITree extends IList {
	public function SubItems(TTreeItem $item);
}

interface IFilePropertyPublisher {
	public function http_File($id,$name);
	public function FileLink($base);
}

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * list item
 */
class TListItem implements ISelectable, IItem {
/**
 * @var IItem
 */	
	protected $item;
/**
 * @var IList $list
 */	
	protected $list;
/**
 * constructor
 */		
	public function __construct(IItem $item, IList $list){
		$this->item = $item;
		$this->list = $list;
	}
/**
 * @see ISelectable::SelectLink()
 * @return string
 */	
	public function SelectLink(){
		return $this->list->SelectionLink($this); 
	}
	
	public function OpenLink(){
		return $this->list->OpenLink($this);
	}
/**
 * gets item id
 * @return string
 */	
	public function ItemId(){return $this->list->WrapId($this->item->ItemId());}
/**
 * gets item class name
 * @return string
 */	
	public function ClassName(){
		return $this->item->ClassName();
	}
	
/**
 * @ignore
 */	
	public function __get($nm){
		$p = $this->item->Property($nm);
		if (!is_null($p)) return $p->Value();
		if (method_exists($this->item,"__get"))
			return $this->item->__get($nm);
		return null;
	}
/**
 * checks whether item is selected
 * @return boolean
 */	
	public function IsSelected(){
		if (!is_null($this->list->SelectedItem()))
			return $this->ItemId() == $this->list->SelectedItem()->ItemId();
		return false; 
	} 

/**
 * gets item
 * @return IItem
 */	
	public function Item(){
		return $this->item;
	}
/**
 * gets string representation of item
 * @return string
 */	
	public function __toString(){
		return htmlentities(_i($this->item->__toString()),ENT_QUOTES,"UTF-8");
	}
	
	public function Properties($includetypes = array(),$excludetypes = array()){
		return $this->item->Properties($includetypes,$excludetypes);
	}
	
	public function Property($name){
		return $this->item->Property($name);
	}
}

class TTreeItem extends TListItem {
	public function __construct(IItem $item, ITree $tree){
		parent::__construct($item,$tree);
	}	
	
	public function SubNodes(){
		return $this->list->SubItems($this);
	}
}

interface IMenu {
	public function Wrap($base);
	public function SelectionLink(TMenuItem $item);
}

interface IEditor {
/**
 * @param mixed $value
 */	
	public function SetupValue($value);
/**
 * @return mixed
 */	
	public function Value();
/** 
 * @return string
 */	
	public function FieldName();
}


class TMenuItem implements IIdentity {
	private $_id_;
	private static $_id_counter_ = 0;
	protected $base;
	protected $menu;
	
/**
 * constructor
 */	
	public function __construct($base, IMenu $menu){
		self::$_id_counter_++;
		$this->id = self::$_id_counter_;
		$this->base = $base;
		$this->menu = $menu;
	}
	
	public function ItemId(){
		return "mi".$this->_id_;
	}
	
	public function SubItems(){
		$result = array();
		if ($this->base instanceof IParent)
			foreach ($this->base->Children() as $child)
				$result[] = $this->menu->Wrap($child);
		return $result;
	}
	
	public function Available(){
		if (method_exists($this->base, "Available"))
			return $this->base->Available();
		return true;
	}
	
	public function __toString(){
		return $this->base."";
	}
}

class TLinkMenuItem extends TMenuItem implements INavigable {
	public function __construct(ILink $link, IMenu $menu){
		parent::__construct($link, $menu);
	}
	
	public function __toString(){
		return $this->base->Title();
	}

	public function Link(){
		$service = $this->base->Service();
		if ($service == "self")
			$service = TApplication::$Application->CurrentService()->Name();
		if ($this->base->Url())
			return $this->base->Url();
		return TApplication::$Application->Url($service,$this->base->Parameters()); 
	}
} 

class TActionMenuItem extends TMenuItem implements ISelectable {
	public function __construct(IAction $base, IMenu $menu){
		parent::__construct($base,$menu);
	}	
	
	public function Action(){
		return $this->base;
	}
	
	public function Available(){
		return $this->base->Available();
	}
	
	public function __toString(){
		return $this->base->Title();
	}	
	
	public function SelectLink(){
		return $this->menu->SelectionLink($this); 
	}	
}

class TNavigationMenuItem extends TMenuItem implements INavigable {
	private $_navigator_;
	public function __construct(IItem $item, IMenu $menu, IItemNavigation $navigator){
		parent::__construct($item, $menu);
		$this->_navigator_ = $navigator;
	}
	
	public function Link(){
		return $this->_navigator_->ItemNavigationUrl($this->base); 
	}
}

/**
 * @package View
 * @subpackage Widgets
 * @category System 
 * converts items to list items
 */
class TListIteratorAdapter extends TIteratorAdapter {
/**
 * @var IList
 */	
	protected $list;
/**
 * constructor
 */	
	public function __construct(IIterator $iterator, IList $list){
		parent::__construct($iterator);
		$this->list = $list;
	}
/**
 * @see IIterator::Item()
 */	
	public function Item(){
		$item = $this->iterator->Item();
		$result = null;
		if ($item)
			$result = $this->list->WrapItem($item);
		return $result;
	}
}
?>