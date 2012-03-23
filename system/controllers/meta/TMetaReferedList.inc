<?php
/**
 * @package Controllers
 * @subpackage Meta
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("controllers::TMetaList");

/**
 * @package Controllers
 * @subpackage Meta
 * @category System
 * Meta item associated items
 */
class TMetaReferedList extends TMetaList implements IItemsCollection {
	const AN_ADD = "AddItem";
	const AN_REMOVE = "RemoveItem";
	
/**
 * Adding and removing action is added to TMetaList actions
 * @see IActionList::Actions()
 * @return array 
 */	
	protected function actionsList(){
		$result = parent::actionsList();
		$addaction = new TControllerAction($this,TMetaReferedList::AN_ADD,"AddItem","Add");
		$result[$addaction->LocalName()] = $addaction;
		$removeaction = new TControllerAction($this,TMetaReferedList::AN_REMOVE,"RemoveCurrentItem","Remove");
		$result[$removeaction->LocalName()] = $removeaction;
		return $result;
	}
	
	protected function setupAction(TControllerAction $action){
		switch ($action->LocalName()){
			case TMetaReferedList::AN_ADD:{
				$action->ClearParameters();
				if ($this->DataSource && $this->BaseClass){
					$c = $this->DataSource->GetClass($this->BaseClass);
					if (!is_null($c)){
						$ai = $this->DataSource->GetItems($c);
						$selection = array();
						foreach ($ai as $i)
							$selection[$i->Id()] = $i->__toString();
						$action->AddParemeter(new TActionParameter("id","Add",TActionParameterType::TYPE_SELECT,null,$selection));
					} 
				}
			}break;
			default:parent::setupAction($action);break;
		}
	}
	
/**
 * @see IActionList::ActionAvailable()
 * @return boolean
 */	
	public function ActionAvailable(IAction $action){
		switch ($action->LocalName()){
			case TMetaReferedList::AN_ADD:return !is_null($this->BaseClass);
			case TMetaReferedList::AN_REMOVE:return !is_null($this->currentItem);break;
		}
		return parent::ActionAvailable($action);
	}	

	public function __set($nm, $value){
		switch ($nm){
			case "BaseClass":{
				parent::__set($nm,$value);
				if ($this->DataSource && key_exists(TMetaReferedList::AN_ADD,$this->_actions_))
					$this->setupAction($this->_actions_[TMetaReferedList::AN_ADD]);
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __unset($nm){
		switch ($nm){
			case "BaseClass":{
				parent::__unset($nm);
				if ($this->DataSource && key_exists(TMetaReferedList::AN_ADD,$this->_actions_))
					$this->setupAction($this->_actions_[TMetaReferedList::AN_ADD]);
			}break;
			default:parent::__unset($nm);break;
		}
	}	
}
?>