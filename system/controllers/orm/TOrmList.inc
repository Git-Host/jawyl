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
TApplication::import("common::mvc");
/**
 * @ignore
 */
TApplication::import("common::controller::interfaces");
/**
 * @ignore
 */
TApplication::import("common::controller::TItemsSource");

TApplication::import("models::TOrmModel");

/**
 * @package Controllers
 * @subpackage Orm
 * @category System
 * Items list interacting with TOrmModel
 * @property TOrmModel $DataSource orm component to interact with, can be set by component name
 * @property array $CreationAttributes array of names of attributes used for item creation, can be set as semicolon separated string
 * @property array $FilterAttributes array of names of attributes used for filtering, can be set as semicolon separated string
 * @property array $SortingAttributes array of names of attributes used for sorting, can be set as semicolon separated string
 * @see TOrmModel 
 */
class TOrmList extends TItemsSource implements ISortableItems, IFilterableItems, IPagedItems {
	const AN_CREATE = "CreateItem";
	const AN_DELETE = "DeleteItem";
	const AN_FILTER = "SetFilter";
	const AN_SORT =	"SetSorting";	
	
/**
 * @var string 
 * name of class which items to operate with
 */	
	protected $_baseClass_;
	public $AbstractBaseClass;
	public $MasterItemsCreation;
	public $CreationAttributesCaptions;

/**
 * @var array
 */	
	protected $create_params = array();
/**
 * @var TOrmModel
 */	
	protected $__datasource__;
	
	private $filter_attributes = array();
	private $sorting_attributes = array();

/**
 * @var TControllerAction
 */	
	private $sorting_action;
/**
 * @var TControllerAction
 */	
	private $filter_action;
	
/**
 * @var array
 */	
	protected $SEARCH_CONDITIONS = array();
/**
 * @var array
 */	
	protected $SORTING = array();
/**
 * @var int
 */	
	protected $PAGE;
	
	protected $page_size = null;

	protected function setupAction(TControllerAction $action){
		switch ($action->LocalName()){
			case TOrmList::AN_CREATE:{
					if (($this->DataSource) && ($this->BaseClass)){
						$action->ClearParameters();
						$c = $this->DataSource->GetClass($this->BaseClass);
						$classes = array();
						$descs = $this->DataSource->ClassDescenders($this->BaseClass);
						$abstractbc = TConvertions::ConvertToBoolean($this->AbstractBaseClass);
						while ($descs->Next())
							if ((!$abstractbc) || ($descs->Item()->Id() != $c->Id()))
								$classes[$descs->Item()->Name()] = $descs->Item()->Name();

							$action->AddParameter(new TActionParameter("ormclass","Class",TActionParameterType::TYPE_SELECT,null,$classes));
		
							if ($this->CreationAttributes){
								$ca = $this->CreationAttributes;
								$captions = array();
								if ($this->CreationAttributesCaptions)
									$captions = explode(";",$this->CreationAttributesCaptions);
								$i = 0;
								foreach ($ca as $a){
									$caption = null;
									if (key_exists($i,$captions)) $caption = $captions[$i];
									$ad = $c->GetAttributeDefinition($a);
									if ($ad)
										$action->AddParameter($this->actionParam($c->GetAttributeDefinition($a),null,$caption));
								}
							$i++;
						}
					}
			}break;
			case TOrmList::AN_FILTER:{
				if ($this->DataSource && $this->BaseClass){
					$action->ClearParameters();
					$params = array();
					$c = $this->DataSource->GetClass($this->BaseClass);
					foreach ($this->filter_attributes as $aname){
						$ca = $c->GetAttributeDefinition($aname);
						if ($ca)
							$action->AddParameter($this->actionParam($ca,key_exists($aname,$this->SEARCH_CONDITIONS)?$this->SEARCH_CONDITIONS[$aname]:null));
					}
				}
			}break;
			case TOrmList::AN_SORT:{
				if ($this->DataSource && $this->BaseClass){
					$action->ClearParameters();
					$params = array();
					$c = $this->DataSource->GetClass($this->BaseClass);
					$sellist = array("null"=>"none",TSorting::SORT_ASC=>"asc",TSorting::SORT_DESC=>"desc");
					foreach ($this->sorting_attributes as $aname){
						$ca = $c->GetAttributeDefinition($aname);
						if ($ca)
							$action->AddParameter(new TActionParameter(preg_replace('/\s/','_',$ca->Name()),$ca->Name(),TActionParameterType::GetType(TActionParameterType::TYPE_SELECT),key_exists($aname,$this->SORTING)?$this->SORTING[$aname]:null,$sellist));
					}
				}
			}break;
			default:parent::setupAction($action);break;
		}	
	}
	
	public function CheckDataSource($src){
		return $src instanceof TOrmModel;
	}
	
	
/**
 * @ignore
 */	
	public function __set($nm,$value){
		switch ($nm){
			case "DataSource":TConfigurablePattern::SetIocMember($this->__datasource__, $value, array(&$this,"CheckDataSource"));break;
			case "BaseClass":{
				$this->_baseClass_ = $value;
				if ($this->BaseClass && $this->DataSource && (key_exists(TOrmList::AN_CREATE,$this->_actions_)))
					$this->setupAction($this->_actions_[TOrmList::AN_CREATE]);
			}break;	
			case "CreationAttributes":{
				if (is_string($value))
					$this->create_params = explode(";",$value);
				else if (is_array($value))
					$this->create_params = $value;
				else throw new TCoreException(TCoreException::ERR_BAD_VALUE,array());
				if ($this->BaseClass && $this->DataSource && (key_exists(TOrmList::AN_CREATE,$this->_actions_)))
					$this->setupAction($this->_actions_[TOrmList::AN_CREATE]);
			}break;
			case "FilterAttributes":{
				$this->filter_attributes = explode(";",$value);
				if ($this->DataSource && key_exists(TOrmList::AN_FILTER,$this->_actions_))
					$this->setupAction($this->_actions_[TOrmList::AN_FILTER]);
			}break;			
			case "SortAttributes":{
				$this->sorting_attributes = explode(";",$value);
				if ($this->DataSource && key_exists(TOrmList::AN_SORT,$this->_actions_))
					$this->setupAction($this->_actions_[TOrmList::AN_SORT]);
			}break;			
			//case "ExcludeClasses":{$this->class_filter = explode(";",$value);TApplicationSession::Set($this->UniqueId()."_items_count",null);}break;
			case "PageSize":$this->page_size = $value;break;
			default: parent::__set($nm,$value);break;
		}
	}

/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "DataSource":return TConfigurablePattern::GetIocMember($this->__datasource__,array(&$this,"CheckDataSource"));break;
			case "BaseClass":return $this->_baseClass_;break;
			case "CreationAttributes":return $this->create_params;break; 
			case "FilterAttributes":return $this->filter_attributes;break;			
			case "SortAttributes":return $this->sorting_attributes;break;			
			//case "ExcludeClasses":return $this->class_filter;break;
			case "PageSize":return $this->page_size;break;
			default:return parent::__get($nm);break;
		}
	}	

/**
 * @ignore
 */	
	public function __isset($nm){
		switch ($nm){
			case "DataSource":return isset($this->__datasource__);break;
			case "BaseClass":return isset($this->_baseClass_);break;
			case "CreationAttributes":return isset($this->create_params);break;
			case "FilterAttributes":return isset($this->filter_attributes);break;			
			case "SortAttributes":return isset($this->sorting_attributes);break;
			case "PageSize":return isset($this->page_size);break;			
			default:return parent::__isset($nm);break;
		}
	}	

/**
 * @ignore
 */	
	public function __unset($nm){
		switch ($nm){
			case "DataSource":unset($this->__datasource__);break;
			case "BaseClass":{unset($this->_baseClass_);$this->setupAction(TOrmList::AN_CREATE);}break;
			case "CreationAttributes":{unset($this->create_params);$this->setupAction(TOrmList::AN_CREATE);}break;
			case "FilterAttributes":{unset($this->filter_attributes);$this->setupAction(TOrmList::AN_FILTER);}break;			
			case "SortAttributes":{unset($this->sorting_attributes);$this->setupAction(TOrmList::AN_SORT);}break;
			case "PageSize":unset($this->page_size);break;			
			default:parent::__unset($nm);break;
		}
	}

/**
 * constructor
 * @param string $name
 * @see TResponseElement::__construct()
 */	
	public function __construct($name, TResponse $response, array $settings = array()){
		parent::__construct($name,$response,$settings);
		$this->PAGE = $this->loadStateParameter("page");
		if (is_null($this->PAGE)) $this->PAGE = 0;
		
		if (key_exists(TOrmList::AN_FILTER,$this->_actions_))
			$this->filter_action = $this->_actions_[TOrmList::AN_FILTER];
		if (key_exists(TOrmList::AN_SORT,$this->_actions_))	
			$this->sorting_action = $this->_actions_[TOrmList::AN_SORT];
		
		$fs = $this->loadStateParameter("filter");
		if (!is_null($fs)){
			$filter = explode("|",$fs);
			$this->SEARCH_CONDITIONS = array();
			foreach ($filter as $f){
				$temp = explode(':',$f);
				if (count($temp) > 1)
					$this->SEARCH_CONDITIONS[$temp[0]] = $temp[1];
			}
		}
		$fs = $this->loadStateParameter("sorting");
		if (!is_null($fs)){
			$sorting = explode("|",$fs);
			$this->SORTING = array();
			foreach ($sorting as $s){
				$temp = explode(':',$s);
				if (count($temp) > 1)
					$this->SORTING[$temp[0]] = $temp[1];
			}
		}
	}
	
/**
 * gets action parameter for specified attribute definition
 * @param string $prefix optional action parameter name prefix, defaults to "cp_"
 * @param mixed $value optional action parameter value
 * @return TActionParameter
 */	
	protected function actionParam(IAttributeDefinition $ca,$value = null,$caption = null){
		$sellist = array();
		$type = TActionParameterType::TYPE_UNSUPPORTED;
		switch ($ca->Type()->TypeCode){
			case TAttributeType::BIGINT:$type = TActionParameterType::TYPE_INTEGER;break;
			case TAttributeType::BOOLEAN:$type = TActionParameterType::TYPE_BOOLEAN;break;
			case TAttributeType::DATETIME:$type = TActionParameterType::TYPE_DATE;break;
			case TAttributeType::DECIMAL:$type = TActionParameterType::TYPE_REAL;break;
			case TAttributeType::FLOAT:$type = TActionParameterType::TYPE_REAL;break;
			case TAttributeType::INTEGER:$type = TActionParameterType::TYPE_INTEGER;break;
			case TAttributeType::PASSWORD:$type = TActionParameterType::TYPE_PASSWORD;break;
			case TAttributeType::HTML:$type = TActionParameterType::TYPE_HTML;break;
			case TAttributeType::TEXT:$type = TActionParameterType::TYPE_TEXT;break;
			case TAttributeType::FILE:
			case TAttributeType::FILELINK:
			case TAttributeType::IMAGE:
			case TAttributeType::IMAGELINK:$type = TActionParameterType::TYPE_FILE;break;
			case TAttributeType::REFERENCE:{
					$type = TActionParameterType::TYPE_SELECT;
					$ci = $ca->Type()->Class->ClassInstances();
					$sellist["null"]="";
					while ($ci->Next()){
						$wi = TOrmModel::GlobalWrap($ci->Item());
						if (!is_null($wi)) 
							$sellist[$wi->Instance()->Id()] = $wi->__toString();
					}
			}break;
			case TAttributeType::STRING:$type = TActionParameterType::TYPE_STRING;break;
		}
		if ($type != TActionParameterType::TYPE_UNSUPPORTED)
			return new TActionParameter(preg_replace('/\s/','_',$ca->Name()),is_null($caption)?$ca->Name():$caption,$type,$value,$sellist);
		return null;
	}

/**
 * returns create and delete controller actions. 
 * Create action parameters are created according to controller $CreationAttributes property. 
 * @see IActionList::Actions()
 * @return array array of TControllerAction
 */	
	protected function actionsList(){
		$ucreateaction = new TControllerAction($this,TOrmList::AN_CREATE,"CreateOrmItem","New Item");
		$deleteaction = new TControllerAction($this,TOrmList::AN_DELETE,"DeleteCurrentItem","Delete");
		$filteraction = new TControllerAction($this,TOrmList::AN_FILTER,"SetFilter","Filter setup");
		$sortingaction = new TControllerAction($this,TOrmList::AN_SORT,"SetSorting","Sorting setup");
		$result = array(
			$ucreateaction->LocalName()=>$ucreateaction,
			$deleteaction->LocalName()=>$deleteaction,
			$filteraction->LocalName()=>$filteraction,
			$sortingaction->LocalName()=>$sortingaction
		);
		return $result;
	}
	

/**
 * @see IActionList::ActionAvailable()
 * @return boolean
 */	
	public function ActionAvailable(IAction $action){
		switch ($action->LocalName()){
			case TOrmList::AN_CREATE:return !is_null($this->BaseClass);
			case TOrmList::AN_DELETE:return !is_null($this->currentItem);break;
			case TOrmList::AN_FILTER:
			case TOrmList::AN_SORT:return (count($action->ParametersArray()) > 0);break;
		}
		return true;
	}
	
/**
 * @see IFilterable::FilterAction()
 * @return TControllerAction
 */	
	public function FilterAction(){
		if ($this->filter_action)
			if ($this->filter_action->Available())
				return $this->filter_action;
		return null;
	}
	
/**
 * @see ISortable::SortingAction()
 * @return TControllerAction
 */	
	public function SortingAction(){
		if ($this->sorting_action)
			if ($this->sorting_action->Available())
				return $this->sorting_action;
		return null;
	}

/**
 * creates array of TCondition for filtering according to current controller state
 * @return array array of TCondition
 * @see TCondition
 */	
	protected function getFilter(){
		$result = array();
		$c = $this->DataSource->GetClass($this->BaseClass);
		foreach ($this->FilterAttributes as $aname)
			if (key_exists($aname,$this->SEARCH_CONDITIONS)){
				$ca = $c->GetAttributeDefinition($aname);
				switch ($ca->Type()->TypeCode){
					case TAttributeType::HTML:
					case TAttributeType::STRING:
					case TAttributeType::TEXT:$result[] = new TOrmItemFilter($ca, $this->SEARCH_CONDITIONS[$aname],TCondition::C_LIKE);break;
					default:$result[] = new TOrmItemFilter($ca, $this->SEARCH_CONDITIONS[$aname]);break;
				}
			}
		return $result;
	}

/**
 * creates array of TSorting for list sorting according to current controller state
 * @return array array of TSorting
 * @see TSorting
 */	
	protected function getSorting(){
		$result = array();
		$c = $this->DataSource->GetClass($this->BaseClass);
		foreach ($this->SortAttributes as $aname)
			if (key_exists($aname,$this->SORTING)){
				$ca = $c->GetAttributeDefinition($aname);
				$result[] =new TOrmItemSorting($ca,$this->SORTING[$aname]);
			}
		return $result;
	}	
	
/**
 * resets bage to be the first and sets items total count to null
 */	
	protected function resetPageInfo(){
		$this->PAGE = 0;
		TApplicationSession::Set($this->UniqueId()."_items_count",null);
	}

/**
 * set filtering parameters (called by filter action)
 */	
	public function SetFilter(){
		$this->SEARCH_CONDITIONS = array();
		$fs = array();
		if (func_num_args() > 0){
			$request = func_get_arg(0);
			$c = $this->DataSource->GetClass($this->BaseClass);
			$ca = $this->FilterAttributes;
			foreach ($ca as $a){
				$p = $this->actionParam($c->GetAttributeDefinition($a),"fp_");
				if ($p){
					if (!is_null($request->__get($p->Name())) && ($request->__get($p->Name()) != "") && ($request->__get($p->Name()) != "null")){
						$this->SEARCH_CONDITIONS[$a] = $request->__get($p->Name());
						$fs[] = $a.":".$this->SEARCH_CONDITIONS[$a];
					}
				}
			}
		}
		$this->saveStateParameter("filter",join("|",$fs));
		$this->ResetPageInfo();	
		//$this->Invoke(
		new TDataReloadEvent($this);
		//);
	}
	
/**
 * set sorting mode (colled by sorting action)
 */	
	public function SetSorting(){
		$this->SORTING = array();
		$fs = array();
			
		if (func_num_args() > 0){
			$request = func_get_arg(0);
			$c = $this->DataSource->GetClass($this->BaseClass);
			$ca = $this->SortAttributes;
			foreach ($ca as $a){
				$p = $this->actionParam($c->GetAttributeDefinition($a),"sp_");
				if ($p){
					if (!is_null($request->__get($p->Name())) && ($request->__get($p->Name()) != "") && ($request->__get($p->Name()) != "null"))
					$this->SORTING[$a] = $request->__get($p->Name());
					$fs[] = $a.":".$this->SORTING[$a];
				}
			}
		}
			
		$this->saveStateParameter("sorting",join("|",$fs));	
		//$this->Invoke(
		new TDataReloadEvent($this);
		//);
	}
	
/**
 * create new orm item
 * @param string $ormclass orm class name
 * @return TOrmItem
 */	
	public function CreateOrmItem($ormclass){
		$params = array();
		if (func_num_args() > 1){
			$request = func_get_arg(1);
			$c = $this->DataSource->GetClass($ormclass);
			$ca = $this->CreationAttributes;
			foreach ($ca as $a){
				$ad = $c->GetAttributeDefinition($a);
				if ($ad){
					$p = $this->actionParam($c->GetAttributeDefinition($a));
					if ($p)
						$params[$a] = $request->__get($p->Name());
				}
			}
		}
		$ci = new TCreateInfo($ormclass,$params);
		return $this->CreateItem($ci);
	}	

/**
 * @see IPagedItems::OpenPage()
 * @param int $page
 */	
	public function OpenPage($page){
		$this->PAGE = $page;
		//$this->Invoke(
		new TDataReloadEvent($this);
		//);
		$this->saveStateParameter("page",$this->PAGE);
	}

/**
 * @see IPagedItems::CurrentPageNumber()
 * @return int
 */	
	public function CurrentPageNumber(){
		return $this->PAGE;
	}
	
	public function PageSize(){
		return $this->page_size;
	}
	
/**
 * gets list items total count
 * @return int 
 */	
	protected function getItemsCount(){
		return TApplicationSession::Get($this->UniqueId()."_items_count");
	}
	
/**
 * @see IPagedItems::PageCount()
 * @param int $pagesize
 * @return int
 */	
	public function PageCount(){
		$ic = $this->getItemsCount();
		$result = $ic/$this->page_size;
		if (floor($result) < $result) $result = floor($result) + 1;
		return $result;
	}

/**
 * @see IPagedItems::CurrentPage()
 * @return IIterator iterator of TOrmItem
 */	
	public function CurrentPage(){
		$check = false;
		if ($this->CURRENT_CONTAINER)
			$check = $this->CheckPermission($this->CurrentContainer(),TOrmItemPermission::ORM_LIST);
		else  
			$check = $this->CheckPermission($this->DataSource->GetClass($this->BaseClass),TOrmClassPermission::ORM_INSTLIST);
		
		if (!$check){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result; 
		}
		$items = $this->getItems($this->PAGE,$this->page_size);
		return $items; 
			
	}

/**
 * gets list items on specified page. 
 * if page or page size not specified returns all items, matching current filter
 * Can be overriden in descenders.
 * @param int $page
 * @param int $pagesize
 * @return IIterator iterator of TOrmItem 
 */	
	protected function getItems($page = null, $pagesize = null){
		$result = array();
		$c = $this->DataSource->GetClass($this->BaseClass);
		$ss = $this->DataSource->GetItems($c,$this->getFilter(),$this->getSorting());
		if (!is_null($pagesize)){
			TApplicationSession::Set($this->UniqueId()."_items_count",$ss->ItemCount());
			if ($page)
				return new TPagedIterator($ss,$page,$pagesize);
		}	
		return $ss;
	}

/**
 * gets list items on specified page with permission check. 
 * if page or page size not specified returns all items, matching current filter.
 * @param int $page
 * @return IIterator iterator of TOrmItem 
 * @uses TOrmList::getItems()
 */	
	public function Items($page = null){
		$result = array();
		$c = $this->DataSource->GetClass($this->BaseClass);
		if (!$this->CheckPermission($c,TOrmClassPermission::ORM_INSTLIST)){
		  throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
		  return $result;
		}
		$result = $this->getItems($page,$this->page_size);
		return $result;
	}

/**
 * gets orm item property by item id and property name
 * @param mixed $id item id
 * @param string $name property name
 * @return IProperty
 */	
	protected function itemProperty($id,$name){
		$result = false;
		$item = $this->DataSource->GetItem($id);
		$result = $item->Property($name);
		if (!$this->CheckPermission($result,TOrmPropertyPermission::ORM_READ)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $item->Property($name); 
		return $result;
	}
	
/**
 * edits orm item
 * @param mixed $id item id
 * @param string $attrname attribute name
 * @param mixed $value new attribute value
 * @return boolean
 */	
	protected function editItem($id,array $attrvalues){
		$result = false;
		$item = $this->DataSource->GetItem($id);
		foreach ($attrvalues as $attrname=>$value){
			$prop = $item->Property($attrname);
			if (!$this->CheckPermission($prop,TOrmPropertyPermission::ORM_WRITE)){
				throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
				return $result;
			}
		}
		return $this->DataSource->EditItem($item,$attrvalues);
	}
	
/**
 * gets orm item by item id with permission check
 * @param mixed $id item id
 * @return TOrmItem 
 */	
	public function Item($id){
		$result = null;
		$item = $this->DataSource->GetItem($id);
		if (!$this->CheckPermission($item,TOrmItemPermission::ORM_VIEW)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		return $item;
	}

/**
 * gets orm item properties
 * @param mixed $id item id
 * @return array array of IProperty
 */	
	protected function itemProperties($id){
		$result = false;
		$item = $this->DataSource->GetItem($id);
		if (!$this->CheckPermission($item,TOrmItemPermission::ORM_VIEW)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $item->Properties(); 
		return $result;
	}
/**
 * checks specified orm class to be base class, if success creates new orm item and resets page information
 * @return TOrmItem
 */	
	protected function itemCreation(IClass $c, array $parameters = array()){
		$result = null;
		if ($c->IsClass($this->BaseClass)){
			$result = $this->DataSource->CreateItem($c,$parameters);
			$this->ResetPageInfo();	
		}
		else throw new TCoreException(TCoreException::ERR_NOT_CLASS,null,null,array("class"=>$this->BaseClass));
		return $result;
	}
	
/**
 * @return IPolicyManager
 * @see TSecuredController::getPolicyManager()
 */
	protected function getPolicyManager(){
		if ($this->__datasource__ instanceof IPolicyManager)
			return $this->__datasource__;
		return parent::getPolicyManager();
	}
	
/**
 * @return IPolicyProvider
 * @see TSecuredController::getPolicyManager()
 */
	protected function getPolicyProvider(){
		if ($this->__datasource__ instanceof IPolicyProvider)
			return $this->__datasource;
		$provider = TApplication::Instance($this->PolicyProvider);
		if ($provider instanceof IPolicyProvider)
			return $provider;
		return null;
	}
	

/**
 * creates orm item according to creation information in $createinfo.
 * Performs permission check.
 * @see TItemsSource::Create()
 * @return TOrmItem
 */	
	protected function create(TCreateInfo $createinfo = null){
		$result = array();
		$cc = $this->DataSource->GetClass($createinfo->ItemType());
		if ($this->CheckPermission($cc,TOrmClassPermission::ORM_INSTANCIATE))
			$result = $this->ItemCreation($cc, $createinfo->Parameters()); 
		else  throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
		return $result;
	}

/**
 * deletes item by id
 * @param mixed $id item id
 * @return boolean
 */	
	protected function deleteItem($id){
		$result = false;
		$item = new TIdentity($id);
		if (!$this->CheckPermission($item,TOrmItemPermission::ORM_DELETE)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return $result;
		}
		$result = $this->DataSource->DeleteItem($id);
		$this->ResetPageInfo();	 
		return $result;
	}	
	
	protected function onRefresh(TEvent $event){
		$this->ResetPageInfo();
		parent::onRefresh($event);
	}
}
?>