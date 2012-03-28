<?php
/**
 * @package Presenters
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("common::presenter::basic");
/**
 * @ignore
 */
TApplication::import("common::presenter::TItemsList");

/**
 * @package Presenters
 * @category System
 * abstract class for holding and managing collections.
 * @see TItemsList
 * @see IItemsSource
 */	
abstract class TItemsSource extends TItemsList implements IItemsSource {		
/**
 * gets presenter actions list, containing create and delete actions.
 * @return TPresenterAction[]
 * @see IActionsList
 */
		protected function actionsList(){
			$result = array(
				"Create"=>new TPresenterAction($this,"CreateItem","CreateItem","New"),
				"Delete"=>new TPresenterAction($this,"Delete","DeleteCurrentItem","Delete")
			);
			return $result;
		}
		
/**
 * always returns true for create action, for delete action returns true if current collection item is selected
 * @return boolean
 * @see IActionsList
 */		
		public function ActionAvailable(IAction $action){
			if ($action->LocalName() == "Delete")
				return !is_null($this->currentItem);
			return true;
		}
/**
 * Method for getting item property value by item id. Should be overriden in descendants.
 * @param mixed $id item id
 * @param string $name property name
 * @return mixed
 */
		protected abstract function itemProperty($id,$name);
/**
 * Method for getting item properties list by item id. Should be overriden in descendants.
 * @param mixed $id item id
 * @return IItemProperty[] associative array of IItemProperty instances where keys are property names.
 * @see IItemProperty
 */
		protected abstract function itemProperties($id);
/**
 * Method for creating item. Should be overriden in descendants. Returns true if succeed.
 * @param TCreateInfo $createinfo item creation information
 * @return boolean
 * @see TCreateInfo
 */		
		protected abstract function create(TCreateInfo $createinfo = null);
/**
 * Method for deleting item. Should be overriden in descendants. Returns true if succeed.
 * @param mixed $id item id
 * @return boolean
 */		
		protected abstract function deleteItem($id);
/**
 * item creation method. Called by create action.
 * Wraps presenter Create method. If items creation succeeds invokes TItemCreateEvent.  
 * @return boolean
 * @see IItemsSource
 * @see TItemCreateEvent
 */		
		public function CreateItem(TCreateInfo $createinfo = null){
			$result = null;
			if ($result = $this->Create($createinfo)){
				//$this->Invoke(
				new TItemCreateEvent($result->ItemId(),null,$this);
				//);
				$this->SelectItem($result->ItemId());
			}
			return $result;
		}

/**
 * gets presenter current item property value. If current item is not selected returns null.
 * @param string $name property name
 * @return mixed 
 * @see IItemsSource
 */		
		public function CurrentItemPropertyValue($name){
			if (!is_null($this->CurrentItem())){
				$prop = $this->ItemProperty($this->CurrentItemId(),$name);
				return $prop->Value();
			}
			return null;
		} 
		
/**
 * Deletes presenter current item. If deletion succeeds invokes delete event. Wraps presenter DeleteItem method.
 * @see IItemsSource
 * @see TItemDeleteEvent  
 */		
		public function DeleteCurrentItem(){
			if (!is_null($this->CurrentItem())){
				if ($this->DeleteItem($this->CurrentItemId())){
					//$this->Invoke(
					new TItemDeleteEvent($this->CurrentItemId(),$this);
					//);
					$this->SelectItem(null);
				}
			}
		}
		
		public function DeleteTheItem($id){
			if ($this->DeleteItem($id))
				$this->Invoke(new TItemDeleteEvent($id,$this));
		}
/**
 * Method for setting item property value. Should be overriden in descendants. Returns true if succeed.
 * @param mixed $id item id
 * @param string $name property name
 * @param mixed $value property value
 * @return boolean
 */		
		protected abstract function editItem($id,array $attrvalues);
/**
 * Sets presenter current item property value. Wraps presenter EditItem method. Invokes edit event on success.
 * @see IItemsSource
 * @see TItemEditEvent  
 */		
		public function EditCurrentItem($attrname,$value){
			if (!is_null($this->CurrentItem())){
				$OLD_ID = $this->CurrentItemId();
				if ($item = $this->EditItem($this->CurrentItemId(),array($attrname=>$value)))
					//$this->Invoke(
					new TItemEditEvent($this->CurrentItemId(),$attrname,$item->__get($attrname),$this, $OLD_ID);
					//);
			}
		}
		
		public function ModifyCurrentItem(array $attrvalues){
			if (!is_null($this->CurrentItem())){
				$OLD_ID = $this->CurrentItemId();
				if ($item = $this->EditItem($this->CurrentItemId(),$attrvalues))
					//$this->Invoke(
					new TItemEditEvent($this->CurrentItemId(),array_keys($attrvalues),array_values($attrvalues),$this, $OLD_ID);
					//);
			}
		}

		public function EditTheItem($id,$attrname,$value){
			if ($this->EditItem($id,$attrname,$value))
				//$this->Invoke(
				new TItemEditEvent($id,$attrname,$value,$this,$id);
				//);
		}

		public function ModifyTheItem($id,array $attrvalues){
			if ($this->EditItem($id,$attrvalues))
				//$this->Invoke(
				new TItemEditEvent($id,array_keys($attrvalues),array_values($attrvalues),$this,$id);
				//);
		}
}
?>