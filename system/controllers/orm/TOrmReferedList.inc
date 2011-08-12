<?php
/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("controllers::TOrmList");

/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 * Orm item associated items
 */
class TOrmReferedList extends TOrmList implements IItemsCollection {
	const AN_ADD = "AddItem";
	const AN_REMOVE = "RemoveItem";
	
/**
 * Adding and removing action is added to TOrmList actions
 * @see IActionList::Actions()
 * @return array 
 */	
	protected function actionsList(){
		$result = parent::actionsList();
		$addaction = new TControllerAction($this,TOrmReferedList::AN_ADD,"AddItem","Add");
		$result[$addaction->LocalName()] = $addaction;
		$removeaction = new TControllerAction($this,TOrmReferedList::AN_REMOVE,"RemoveCurrentItem","Remove");
		$result[$removeaction->LocalName()] = $removeaction;
		return $result;
	}
	
	protected function setupAction(TControllerAction $action){
		switch ($action->LocalName()){
			case TOrmReferedList::AN_ADD:{
				$action->ClearParameters();
				if ($this->DataSource && $this->BaseClass){
					$c = $this->DataSource->GetClass($this->BaseClass);
					if (!is_null($c)){
						$ai = $this->DataSource->GetItems($c);
						$selection = array();
						while ($ai->Next())
							$selection[$ai->Item()->Id()] = $ai->Item()->__toString();
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
			case TOrmReferedList::AN_ADD:return !is_null($this->BaseClass);
			case TOrmReferedList::AN_REMOVE:return !is_null($this->currentItem);break;
		}
		return parent::ActionAvailable($action);
	}	

	public function __set($nm, $value){
		switch ($nm){
			case "BaseClass":{
				parent::__set($nm,$value);
				if ($this->DataSource && key_exists(TOrmReferedList::AN_ADD,$this->_actions_))
					$this->setupAction($this->_actions_[TOrmReferedList::AN_ADD]);
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __unset($nm){
		switch ($nm){
			case "BaseClass":{
				parent::__unset($nm);
				if ($this->DataSource && key_exists(TOrmReferedList::AN_ADD,$this->_actions_))
					$this->setupAction($this->_actions_[TOrmReferedList::AN_ADD]);
			}break;
			default:parent::__unset($nm);break;
		}
	}	
}
?>