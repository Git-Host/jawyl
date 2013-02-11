<?php
/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore 
 */
require_once 'common/eventsystem.inc';
/**
 * @ignore 
 */
require_once 'common/iterator.inc';
/**
 * @ignore 
 */
require_once 'core/TConfigurable.inc';
/**
 * @ignore 
 */
require_once 'core/utils/TConvertions.inc';
/**
 * @ignore 
 */
require_once 'core/utils/THttpMeta.inc';
/**
 * @ignore 
 */
require_once 'common/theme/engine.inc';

require_once 'common/enum.inc';

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * basic class for all MVP objects. 
 * Enables event processing for MVP objects, and allows them to be configurable.
 * @uses TConfigurablePattern 
 */
abstract class TMVPObject extends TConfigurable implements IEventListener, IEventSender {
	
/**
 * @var IEventBroadCaster[]
 */	
	protected $_ioc_broad_casters_ = array();
/**
 * @var IEventHandler
 */	
	protected $_ioc_delegation_provider_ = null; 
	
	private $_explicit_handlers_ = array();
	
	public $DefaultMethod;	
	
	public function __toString(){
		return $this->Name();
	}
	
/**
 * @ignore
 */		
	public function __set($nm,$value){
		switch ($nm){
			default:{
				if (preg_match('/^On([\w_]*)Event$/',$nm,$matches) != 0){
					if (!is_array($this->_explicit_handlers_[$matches[1]]))
						$this->_explicit_handlers_[$matches[1]] = array();
					$this->_explicit_handlers_[$matches[1]][] = $value;
				}
				parent::__set($nm, $value);
			}break;
		}
	}
	
/**
 * calls http method
 * @param string $method method name
 * $param array $arguments array of arguments, which can be an associative array. In this case method argument values are set according to argument name but not order.
 * @return mixed return value of method called. If method can not be called returns false.
 */	
	public function __call($method,array $arguments){
		$c = new ReflectionClass(get_class($this));
		$mn = $this->methodHttpName($method);
		if ($c->hasMethod($mn)){
			$m = $c->getMethod($mn);
			$args = array();
			$margs = $m->getParameters();
			$assignedkeys = array();
			
			foreach ($margs as $marg){
				$v = null;
				if (key_exists($marg->getName(),$arguments)){
					$args[] = $arguments[$marg->getName()];
					$assignedkeys[] = $marg->getName();
				}
				else if (key_exists($marg->getPosition(),$arguments)){
					$args[] = $arguments[$marg->getPosition()];
					$assignedkeys[] = $marg->getPosition();
				}
				else
					$args[] = $marg->getDefaultValue();
			}
			
			$unassignedkeys = array_diff($assignedkeys, array_keys($arguments));
			foreach ($unassignedkeys as $key)
				$args[] = $arguments[$key]; 			
			
			if ($m->isStatic())
				return $m->invokeArgs(null,$args);
			else
				return $m->invokeArgs($this,$args);	
		}
		return false;
	}
	
	protected function methodHttpName($method){
		return "http_".$method;
	}
	
	public function HttpMethodCall($method, THttpRequest $request){
		$args = $request->Parameters();
		if (!is_null($request->JsonObject))
			$args[] = $request->JsonObject;
		$this->__call($method, $args);
	}

/**
 * @see IEventSender::Invoke()
 */	
	public function Invoke(TEvent $event){
		foreach ($this->_ioc_broad_casters_ as $bc)
			$bc->BroadCastEvent($event);
	}

	protected function checkEvent(TEvent $event){
		return true;
	}
	
	public function Dispatch(TEvent $event){
		if ($event->StopHandling) return;
		if (!$this->checkEvent($event)) return;
		$rc = new ReflectionClass(get_class($this));
		if ($rc->hasMethod("onEvent")){
			$m = $rc->getMethod("onEvent");
			if ($m->getNumberOfRequiredParameters() >= 1){
				$params = $m->getParameters();
				if ($pc = $params[0]->getClass()){
					$pc = $pc->getName();
					if ($pc == "TEvent")
						if ($event instanceof TEvent)
							$this->onEvent($event);
				}
			}
		}
		
		$mn = "on".$event->Name."Event";
		if ($rc->hasMethod($mn)){
			$m = $rc->getMethod($mn);
			if ($m->getNumberOfRequiredParameters() >= 1){
				$params = $m->getParameters();
				if ($pc = $params[0]->getClass()){
					$pc = $pc->getName();
					if (($pc == "TEvent") || (is_subclass_of($pc,"TEvent")))
					if ($event instanceof $pc){
						$this->$mn($event);
						if ($event->StopHandling) return;
					}
				}
			}
		}
		
		$explicit = $this->__get("On".$event->Name."Event");
		if (is_array($explicit)){
			foreach ($explicit as $handler){
				$h = $handler;
				if (!is_callable($h,false))
					if ($this->DelegationProvider)
						$h = array($this->DelegationProvider,$h);
				if (is_callable($h))
					call_user_func_array($h,array($event,$this));
				if ($event->StopHandling) return;	
			}
		}
		
		if ($this->DelegationProvider){
			$implicit = $this->DelegationProvider->EventHandlingDelegates($this,$event);
			foreach ($implicit as $handler){
				call_user_func_array($handler,array($event,$this));
				if ($event->StopHandling) return;
			}	
		}
	}
	
	public function AddBroadCaster(IEventBroadCaster $broadcaster){
		$this->BroadCasters = $broadcaster;
	}
		
	public function IsHttpCallable($method){
		$m = $this->methodHttpName($method);
		if (method_exists($this, $m)){
			$rm = new ReflectionMethod($this,$m);
			return $rm->isPublic() && (!$rm->isStatic());
		}
		return false;
	}

/**
 * @ignore
 */	
	public function __get($nm){
		$result = parent::__get($nm);
		if (is_null($result) && (preg_match('/^On([\w_]*)Event$/',$nm,$matches) != 0)){
			if (key_exists($matches[1], $this->_explicit_handlers_))
				return $this->_explicit_handlers_[$matches[1]];
			else 
				return null;
		}	
		return $result;
	}
	
/**
 * @ignore
 */	
	public function __isset($nm){
		switch ($nm){
			default:{
				if (preg_match('/^On([\w_]*)Event$/',$nm,$matches) != 0)
					return !empty($this->_explicit_handlers_[$matches[1]]);
				return parent::__isset($nm);
			}break;
		}
	}
	
/**
 * @ignore
 */		
	public function __unset($nm){
		switch ($nm){
			default:{
				if (preg_match('/^On([\w_]*)Event$/',$nm,$matches) != 0)
					$this->_explicit_handlers_[$matches[1]] = array();
				parent::__unset($nm);
			}break;
		}
	} 
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * basic class for all MVP models
 * @property-read string $Name component name
 * TModel derives settings from current service (and through service from application).
 * @see TService
 */
abstract class TModel extends TConfigurable {
	
/**
 * @ignore
 */	
	public function __get($nm){
		if ($nm == "Name") return $this->name;
		$result = parent::__get($nm);
		if (!$result && !is_null($this->Application()->CurrentService()))
			$result = $this->Application()->CurrentService()->__get($nm);
		if (!$result)
			$result = $this->Application()->$nm;
		return $result;
	}
/**
 * @ignore
 */	
 	public function __isset($nm){
		$result = parent::__isset($nm);
		// if (!$result) $result = $this->Application()->__isset(get_class($this).".".$nm);
		if (!$result) {
			$service = $this->Application()->CurrentService();
			$result = isset($service->$nm);
		}
		if (!$result) $result = isset($this->Application()->$nm);
		return $result; 
	}

	public function __toString(){
		return $this->Name();
	}
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * interface for implementing objects that are part of response
 * @see TResponse
 */
interface IResponseElement {
/**
 * gets element unique identifier
 * @return string
 */	
	public function UniqueId();
	public function Response();
} 

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * basic class for all mvp objects that are also reponse elements
 * @property string $SaveState sets up where element should store its state. Should be one of TResponseElement::SS_SESSION, TResponseElement::SS_COOKIE constants. If not explicitly specified for element then derived from response object.  
 * @see TResponse
 */
abstract class TResponseElement extends TMVPObject implements IResponseElement {
/**
 * @var TResponse response object element belogs to
 */	
	protected $response;

/**
 * @ignore
 */	
	private $_save_state_ = null;
/**
 * @ignore
 */	
	const SESS_STATE_SAVER = "mvp_state";
	
	const SS_SESSION = "session";
	const SS_COOKIE = "cookie";

/**
 * proxy method for setting up element before constructor settings are done. 
 */	
	protected function preSetting(){
		
	} 
/**
 * proxy method for setting up element after constructor settings are done. 
 */	
	protected function postSetting(){
		
	}
/**
 * constructor.
 * @param string $name element name
 * @param TResponse $response response object element belongs to
 * @param array $settings associative array of element settings
 */	
	public function __construct ($name,TResponse $response, array $settings = array(), IConfigurable $container = null){
		if (is_null($container))
			$container = $response;
		parent::__construct($name,$container);
		$this->response = $response;
		$this->BroadCasters = $this->response;
		$this->DelegationProvider = $this->response;
		$this->response->AddEventListener($this);
		$this->preSetting();
		foreach ($settings as $key=>$value)
			if (is_array($value)){
				foreach ($value as $v)
					$this->__set($key,$v);
			} else $this->__set($key,$value);
		$this->postSetting();
	}
	
	public function Response(){
		return $this->response;
	}

/**
 * gets element unique identifier
 * @return string
 */	
	public function UniqueId(){
		return md5($this->response->UniqueId()."::".$this->Name());	
	}

/**
 * @ignore 
 */	
	public function __set($nm,$value){
		switch ($nm){ 
			case "SaveState":$this->_save_state_ = $value;break;
			default:parent::__set($nm,$value);break;
		}
	}
	
/**
 * @ignore 
 */	
	public function __get($nm){
		switch ($nm) { 
			case "SaveState":{
				if (is_null($this->_save_state_)){
					$result = $this->response->SaveState;
					if (is_null($result)) 
						$result = self::SS_COOKIE;
					return $result; 
				}
				return $this->_save_state_;
			}break;
			default:return parent::__get($nm);break;
		}
	}
	
/**
 * gets element state parameter value according to element SaveState property
 */	
	protected function loadStateParameter($name){
		$value = null;
		switch ($this->SaveState){
			case self::SS_SESSION:{
				$ss = $this->Application()->Session()->Get(self::SESS_STATE_SAVER);
				if (!is_null($ss))
					if (key_exists($this->UniqueId(),$ss))
						if (key_exists($name,$ss[$this->UniqueId()]))
							$value = $ss[$this->UniqueId()][$name];
			}break;
			case self::SS_COOKIE:$value = $this->Application()->Request()->Cookie(urlencode($this->UniqueId().$name));break;
		}
		if ($value == "none") $value = null;
		if ($value == "") $value = null;
		return $value;
	}
/**
 * saves element state parameter according to element SaveState property
 */	
	protected function saveStateParameter($name,$value){
		switch ($this->SaveState){
			case self::SS_SESSION:{
				$ss = $this->Application()->Session()->Get(self::SESS_STATE_SAVER);
				if (is_null($ss)) $ss = array();
				if (!key_exists($this->UniqueId(),$ss)) $ss[$this->UniqueId()] = array();
				
				if (!is_null($value))
					$ss[$this->UniqueId()][$name] = $value;
				else if (isset($ss[$this->UniqueId()][$name]))
					unset($ss[$this->UniqueId()][$name]);	
				$this->Application()->Session()->Set(self::SESS_STATE_SAVER,$ss);
			}break;
			case self::SS_COOKIE:$this->response->SetCookie(urlencode($this->UniqueId().$name), $value);break;
		}
	}
} 

interface IActionParameter {
	public function Name();
	public function Type();
	public function Title();
	public function Value();
	public function SetValue($value);
	public function Selectors();
	public function SetSelectors(array $selectors);
}
	
interface IAction extends IConfigurable {
	public function Title();
	public function Parameters();
	public function AddParameter(IActionParameter $param);
	public function GetParameter($name);
	public function DeleteParameter($name);
	public function ClearParameters();
	public function ParametersArray();
	public function Execute(array $parameters);
	public function Perform();
	public function Available();
	public function CallLink();
}
	
/**
 * @package Presenters
 * @category System
 * interface for implementing presenters with actions
 */	
interface IActionsList {
/**
 * gets presenter actions list
 * @return TPresenterAction[]
 * @see TPresenterAction
 */		
	public function Actions();
/**
 * checks whether specified action can be executed
 * @param TPresenterAction $action
 * @return boolean
 */
	public function ActionAvailable(IAction $action);
/**
 * gets name of action group
 * @return string
 */
	public function ActionGroup(IAction $action);
}


abstract class TAction extends TResponseElement implements IAction {
	protected $title;
	private $_parameters_;
	
	public function __construct($name, TResponse $response, array $settings = array(), IConfigurable $container = null, $title = "", TActionParameterIterator $parameters = null){
		$this->title = $title;
		$this->_parameters_ = $parameters;
		parent::__construct($name,$response,$settings,$container);
	}
	
/**
 * gets action title
 * @return string
 */	
	public function Title(){return $this->title;}
	
/**
 * gets action parameters iterator
 * @return TActionParameterIterator
 */	
	public function Parameters(){
		return $this->_parameters_;
	}
	
	protected function onAddParameter(IActionParameter $param){}
	
	public function AddParameter(IActionParameter $param){
		$params = $this->ParametersArray();
		$this->onAddParameter($param);
		$params[] = $param;
		$this->_parameters_ = new TActionParameterIterator($params);
	}
	
	public function GetParameter($name){
		return $this->_parameters_->ValueByKey($name);
	}
	
	public function DeleteParameter($name){
		$result = array();
		foreach ($this->_parameters_ as $key=>$p)
			if ($key != $name)
				$result[] = $p;
		$this->_parameters_ = new TActionParameterIterator($result);
	}
	
	public function ClearParameters(){
		$this->_parameters_ = new TActionParameterIterator(array());
	}
	
/**
 * gets action parameters as an array
 * @return TActionParameter[] array of TActionParameter
 */	
	public function ParametersArray(){
		$result = array();
		if (isset($this->_parameters_)){
			foreach ($this->_parameters_ as $key=>$p)
				$result[$p->Name()] = $p;
		} 
		return $result;
	}
	
	protected function checkEvent(TEvent $event){
		if ($event->Name == TMvpEvent::ACTION_EVENT_NAME)
			if ($event->ActionName == $this->name)
				return true;
		return false;
	}
	
	protected function onActionEvent(TActionCallEvent $event){
		$this->Perform();
	}
	
	public function Perform(){
		if (count($this->_parameters_) > 0){
			$pe = new TActionParametersEvent($this,$this);
			//$this->Invoke(
			new TActionParametersEvent($this,$this);
			//);
		} else $this->Execute();
	}
	
	public function CallLink(){
		return $this->response->Url($this);
	}
	
	public function IsHttpCallable($method){
		return ($method == "Perform") || ($method == "Execute");
	}
	
	protected function methodHttpName($method){
		return $method;
	}

	public function HttpMethodCall($method, THttpRequest $request){
		if ($method == 'Execute')
			$this->Execute($request->Parameters());
		else	
			$this->Perform();
	}
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * basic class for all presenters.
 * @property IEventSender $Reloaders list of event senders causing presenter reloading
 * @see TResponseElement
 */
abstract class TPresenter extends TResponseElement implements IActionsList {
/**
 * @var IEventSender
 */
	protected $_ioc_reloaders_ = array();
	
	protected $actions = array();
	
	public function __construct($name, TResponse $response, array $settings = array()){
		parent::__construct($name,$response,$settings);
		$this->actions = $this->actionsList();		
		foreach ($this->actions as $a)
			$this->setupAction($a);
	}
	
	protected function onEvent(TEvent $event){
		if (in_array($event->Sender,$this->_ioc_reloaders_))
			if ($event instanceof TDataReloadEvent)
				$event->ReSend($this);
			else
				new TDataReloadEvent($this);
	}
	
	protected function setupAction(TPresenterAction $action){}
	
	protected function actionsList(){return array();}
	
	public function Actions(){return $this->actions;}
/**
 * checks whether specified action can be executed
 * @param TPresenterAction $action
 * @return boolean
 */
	public function ActionAvailable(IAction $action){return false;}
/**
 * gets name of action group
 * @return string
 */
	public function ActionGroup(IAction $action){return $this->Name();}	
}
/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * basic class for all widgets.
 * @see TResponseElement
 */
abstract class TWidget extends TResponseElement {
/**
 * @var TWidget[] array of subwidgets
 */	
	protected $widgets = array();
/**
 * @var IThemeEngine
 */
	protected $currentEngine;
/**
 * adds a widget as a subwidget
 */	
	public function AddWidget(TWidget $widget){
		$this->widgets[$widget->Name()] = $widget;	
	}

/**
 * gets subwidgets
 * @return TWidget[]
 */	
	public function Widgets(){
		return $this->widgets;
	}
	
/**
 * @ignore
 */	
	protected function getTemplateByClass($classname,$path,$extension, $subname = null){
		$tfilename = $classname;
		if ($subname) $tfilename .= "_".$subname;
		$tfilename .= ".".$extension;
		
    	if (file_exists($path."/".$tfilename))	
			return $path."/".$tfilename;
		$parent_class = get_parent_class($classname);
		if (isset($parent_class))
			if ($parent_class != "TWidget")
				return $this->getTemplateByClass($parent_class,$path,$extension,$subname);
		return false;
	}

/**
 * gets widget skin file
 * @return string
 */	
	public function Skin(IThemeEngine $engine, $subname = null){
		$templatefile = false;
		$tfilename = $this->SkinName;
		if (!$tfilename)
			$tfilename = $this->Name();
		if ($subname) $tfilename .= "_".$subname;
		$tfilename .= ".".$engine->TemplateExtension();
		if (file_exists($engine->TemplateDir()."/".$tfilename))
			$templatefile = $engine->TemplateDir()."/".$tfilename;	
		if (!$templatefile)
			$templatefile = $this->getTemplateByClass(get_class($this),$engine->TemplateDir(),$engine->TemplateExtension(),$subname);
		if (!$templatefile) return null;	
		return $templatefile;
	}

	protected function startCache(IThemeEngine $engine, $cachename, $lifetime){
		if ($engine->OutputCache){
			if (!$engine->OutputCache->CachedOutput($cachename,$lifetime)){
				$this->Application()->Buffer()->StartRecording();
				return true;
			}
			return false;
		}
		return true;
	}
	
	protected function stopCache(IThemeEngine $engine,$cachename,$lifetime){
		if ($engine->OutputCache){
			$contents = $this->Application()->Buffer()->StopRecording();
			$engine->OutputCache->SaveCache($cachename,$contents,$lifetime);
			echo $contents;
		}		
	}
	
	
/**
 * outputs widget, using specified theme engine
 */	
	public function Output(IThemeEngine $engine,$skinfile = null){
		$this->currentEngine = $engine;
		$skin = $skinfile;
		if (!$skin)
			$skin = $this->SkinFile;
		if (!$skin)
			$skin = $this->Skin($engine);
		if ($skin) 
			$engine->ProcessTemplate($this,$skin,$this->CacheLifeTime);
		else {
			if ($this->CacheLifeTime > 0)
				if (!$this->startCache($engine,$this->Name(),$this->CacheLifeTime)) return; 
			$this->defaultOutput($this->currentEngine);
			if ($this->CacheLifeTime > 0)
				$this->stopCache($engine,$this->Name(),$this->CacheLifeTime);
		}
	}

/**
 * loads resources needed for widget functionality
 */	
	public abstract function LoadResources();

/**
 * default widget output
 */	
	protected abstract function defaultOutput(IThemeEngine $engine);
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * basic class for all views.
 * @property string $ContentType response content type
 * @property array $Headers response additional headers. Can be set both as an array and as multiline text with 'name:value' pairs in each line
 * @property boolean $DenyCallClean when set to true denies request parameter cleaning. Can be set by string that can be parsed by TConvertions::ConvertToBoolean
 * @property string $SaveState specifies response elements SaveSate option default value. If not explicitly set then is derived from current service. 
 * @see TConvertions::ConvertToBoolean()
 * @see TMVPObject
 */
abstract class TResponse extends TMVPObject implements IEventBroadCaster, IEventHandler {
	public $CacheLifeTime = 0;
	
	public $DefaultPresenter;
	
/**
 * @var string
 * stores value of SaveState property
 */	
	protected $stateSave = null;
/**
 * @var boolean
 * stores value of DenyCallClean property 
 */	
	protected $denyCallClean = false;
/**
 * @var TService
 * service response belongs to
 */	
	protected $service;
	
/**	
 * @var THttpRequest
 */
	protected $request;
	
	private $_event_listeners_ = array();
	
/**
 * @var THttpMeta
 */	
	private $_http_meta_;
	
	public function BroadCastEvent(TEvent $event){
		foreach ($this->_event_listeners_ as $l)
			$l->Dispatch($event);
	}
	
	public function AddEventListener(IEventListener $listener){
		$this->_event_listeners_[] = $listener;
	}
	
	public function EventHandlingDelegates(IEventListener $listener, TEvent $event){
		$rc = new ReflectionClass(get_class($this));
		$methods = $rc->getMethods();
		$result = array();
		foreach ($methods as $m){
			if ($m->IsPublic()){
				$checkname = "";
				if ($listener instanceof TResponseElement)
					$checkname .= $listener->Name();
				$checkname .= "handlerof";
				if ($event->Sender instanceof TResponseElement)
					 $checkname .= $event->Sender->Name();
				$checkname .= $event->Name."event";	 
				if (strtolower($m->getName()) == strtolower($checkname))
					$result[] = array($this,$m->getName());
			}
		}
		return $result;
	}
/**
 * @ignore
 */	
	public function __set($nm,$value){
		switch ($nm){
			case "ContentType":$this->_http_meta_->ContentType = $value;break;
			case "Headers":$this->_http_meta_->Headers = $value;break;
			case "DenyCallClean":$this->denyCallClean = TConvertions::ConvertToBoolean($value);
			case "SaveState":$this->stateSave = $value;break;
			default:parent::__set($nm,$value);
		}
	}
/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "ContentType":return $this->CurrentContentType();break;
			case "Headers":return $this->_http_meta_->Headers;break;
			case "DenyCallClean":return $this->denyCallClean;break;
			case "SaveState":{if (is_null($this->stateSave)) return $this->service->SaveState;return $this->stateSave;}break;
			case "Service":return $this->Service();break;
			case "Request":return $this->Request();break;
			default:{
					$item = $this->Item($nm);
					if ($item) return $item;
					$v = $this->service->__get($nm);
					if ($v) return $v;
					return parent::__get($nm);
			}break;
		}
	}
/**
 * sends response additional headers
 */
	public function SendHeaders(){
		$this->_http_meta_->SendHeaders();
	}
/**
 * Sets a cookie
 * @param string $nm
 * @param mixed $value
 */	
	public function SetCookie($name,$value,$expire = null, $path = null){
		if (is_null($expire)) 
			$expire = time()+86400;
			
		if (is_null($value))
			$expire = time()-86400;
			
		$oldval = $this->Application()->Request()->Cookie($name);
		if (is_array($oldval) || is_array($value)){
			if (empty($value))
				$unset = array_keys($oldval);
			else if (empty($oldval))
				$unset = array();	
			else
				$unset = array_diff(array_keys($oldval), array_keys($value));

			if (is_array($value))	
				foreach ($value as $key=>$v)
					if (!in_array($key, $unset))
						$this->SetCookie($name."[$key]", $v, $expire, $path);

			foreach ($unset as $u)
				$this->SetCookie($name."[$u]", null, $expire, $path);
		} else 
			@setcookie($name,$value,$expire,$path);
	}
	
/**
 * gets current content type
 * @return string
 */
	public function CurrentContentType(){
		return $this->_http_meta_->ContentType;
	}
		
/**
 * returns content type header
 * @param string $contentType optional content type. When not specified current content type is used.
 */	
	public function ContentTypeHeader($contentType = null){
		return $this->_http_meta_->ContentTypeHeader($contentType);
	}
/**
 * gets service response belongs to
 * @return TService
 */	
	public function Service(){return $this->service;}
	
/**
 * 
 * @return THttpRequest
 */	
	public function Request(){return $this->request;}

/**
 * constructor
 * @param TService $service service response belongs to 
 */	
	public function __construct(TService $service, $name = null){
		$this->_http_meta_ = new THttpMeta();
		if (!$name)
			$name = $service->Name().'.response';
		parent::__construct($name,$service);
		$this->service = $service;
		$this->DelegationProvider = $this;
		$this->BroadCasters = $this;
		$this->AddEventListener($this);
	}

/**
 * cleans request parameters by redirecting to response url.
 * if call cleaning is denied does nothing.
 * @see TResponse::$DenyCallClean
 */	
	public function CleanCall(){
		if (!$this->denyCallClean)
			$this->Application()->Redirect($this->Url());
	}
	
	protected function PreHandle(THttpRequest $request){return true;}
	protected function PostHandle(THttpRequest $request,array $caller){return true;}
/**
 * handles request. 
 * If request 'controller' and 'method' parameters are set, 
 * it tries to call specified method of specified response element, passing it other request parameters as arguments.
 * The request object itself is also passes to method as the last argument.
 * After method is called CleanCall method is called.
 * @see THttpRequest
 * @see TResponse::CleanCall()
 */	
	public function Handle(THttpRequest $request){
		$this->request = $request;
		if (!$this->PreHandle($request)) return false;
		$controllers = array();
		if (isset($request->controller)) 
			$controllers[] = $this->Item($request->controller);
		if ($this->DefaultController && ($this->DefaultController != $request->controller))
			$controllers[] = $this->Item($this->DefaultController);	
		$controllers[] = $this;
		$controllers = array_filter($controllers);
		$controller = null;
		$method = null;
				
		foreach ($controllers as $c){
			$methods = array($request->method);
			if ($c->DefaultMethod)
				$methods[] = $c->DefaultMethod;
			if ($c instanceof TAction)
				$methods[] = "Perform";
			$methods = array_filter($methods);
			foreach ($methods as $m){
				if ($c->IsHttpCallable($m)){
					$controller = $c;
					$method = $m;
					break;
				}
			}	
			if ($method) break;
		}
		
		if (!$method) return false;
		
		$controller->HttpMethodCall($method,$request);
		$result = $this->PostHandle($request,array($controller,$method));
		
		if ($result)
			if (!(($controller->Name() == $this->DefaultController) && ($method == $controller->DefaultMethod)))
				$this->CleanCall();
		return $result;
	}

/**
 * constructs an url refering to specified response element of the response.
 */	
	public abstract function Url($controller = null,$method = null,array $parameters = array());
	
	public function UniqueId(){
		return $this->Url();
	}
	
/**
 * gets response element by name
 * @param string $name element name
 * @return TResponseElement
 */	
	public function Item($name){
		return $this->Instance($name);
	}	
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * class representing presenter action parameter types.
 */
final class TActionParameterType extends TEnumeration {
	const TYPE_BOOLEAN = "bool";
	const TYPE_INTEGER = "int";
	const TYPE_REAL = "real";
	const TYPE_STRING = "str";
	const TYPE_DATE = "date";
	const TYPE_PICTURE = "pic";
	const TYPE_FILE = "file";
	const TYPE_SELECT = "select";
	const TYPE_SET = "set";
	const TYPE_PASSWORD = "pwd";
	const TYPE_TEXT = "text";
	const TYPE_HTML = "html";
	const TYPE_UNSUPPORTED = "unknown";
	
	public static function GetType($code){
		return new TActionParameterType($code);
	}
}
	
/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * class for holding item creation information
 */
final class TCreateInfo {
	private $_item_type_;
	private $_attributes_ = array();
/**
 * gets type of item
 * @return mixed
 */		
	public function ItemType(){return $this->_item_type_;}
/**
 * constructor
 * @param mixed $itemtype type of item
 * @param array $attributes associatiove array of item attribute values
 */	
	public function __construct($itemtype,array $attributes = array()){
		$this->_item_type_ = $itemtype;
		$this->_attributes_ = $attributes;
	}
/**
 * gets creation parameter value
 * @param string $name
 * @return mixed
 */
	public function Parameter($name){
		return $this->_attributes_[$name];
	}
/**
 * gets array of creation parameters
 * @return array
 */	
	public function Parameters(){return $this->_attributes_;}
/**
 * method for serialization
 * @return string
 */	
	public function __toString(){
		$result = "itemtype:".$this->ItemType();
		foreach ($this->_attributes_ as $key=>$value)
			$result = $result."|".$key.":".$value;
		return $result;
	}
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * class for holding presenter action parameters
 */
class TActionParameter implements IActionParameter {	
	private $_name_;
	private $_type_;
	private $_value_;
	private $_title_;
	private $_group_;
	private $_optional_ = false;
	private $_selectors_ = array();
/**
 * gets parameter name
 * @return string
 */		
	public function Name(){return $this->_name_;}
/**
 * gets parameter type. Return one of TActionParameterType constants
 * @return TActionParameterType
 */	
	public function Type(){return $this->_type_;}
/**
 * gets parameter title
 * @return string 
 */	
	public function Title(){return $this->_title_;}
	
	public function Optional(){return $this->_optional_;}
	
	public function Group(){return $this->_group_;}
/**
 * gets parameter value
 * @return mixed
 */	
	public function Value(){
		if ($this->_type_ == TActionParameterType::TYPE_BOOLEAN)
			return ($this->_value_ == true)?"on":"off";
		return $this->_value_;
	}
		
/**
 * sets parameter value
 * @param mixed $value
 */	
	public function SetValue($value){
		$this->_value_ = $value;
	}

/**
 * gets parameter value selection list
 * @return array
 */	
	public function Selectors(){return $this->_selectors_;}
/**
 * constructor.
 * @param string $name parameter name
 * @param string $title parameter title
 * @param mixed $type parameter type. must be one of TActionParameterType constants
 * @param mixed $value optional parameter value
 * @param array $selectors optional parameter value selection list. Selection list should be an array. which keys are treated as values for selection and values as value titles.
 */	 
	public function __construct($name, $title, $type, $value = null,$selectors = null, $group = null, $optional = false){
		$this->_name_ = $name;
		$this->_title_ = $title;
		$this->_value_ = $value;
		if ($type instanceof TActionParameterType)
			$this->_type_ = $type;
		else 
			$this->_type_ = TActionParameterType::GetType($type);
		if ($this->_type_ == TActionParameterType::TYPE_UNSUPPORTED)
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		if (is_array($selectors))
			$this->_selectors_ = $selectors;
		$this->_group_ = $group;
		$this->_optional_ = $optional;	
	}
	public function SetSelectors(array $selectors){
		$this->_selectors_ = $selectors;
	}
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * iterator of TActionParameter
 */
class TActionParameterIterator extends TArrayIterator {
/**
 * adds parameter to iterator
 */	
	public function addParameter(TActionParameter $param){
		$this->array[] = $param;
	}
/**
 * constructor. constructs an iterator from an array of TActionParameter
 * @param TActionParameter[] $array array of TActionParameter
 */		
	public function __construct(array $array = array()){
		$temp = array();
		foreach ($array as $value)
		  if ($value instanceof TActionParameter)
		  	$temp[$value->Name()] = $value;
		parent::__construct($temp);
	}
}

/**
 * @package Common
 * @subpackage Mvp
 * @category System
 * class for holding presenter actions
 */
final class TPresenterAction extends TAction {
	private $_presenter_;
	private $_method_;
	
	private $_local_name_;

	public function Presenter(){
		return $this->_presenter_;
	}
/**
 * gets name of method action should call
 * @return string
 */	
	public function Method(){return $this->_method_;}
	
	public function Execute(array $parameters){
		$this->_presenter_->__call($this->_method_,$parameters);
	}
	
	public function LocalName(){
		return $this->_local_name_;
	}
	
	public function Available(){
		return $this->_presenter_->ActionAvailable($this);
	}
	
/**
 * invokes state invalidation event on action presenters events
 * @see TAction::Dispatch()
 */	
	public function Dispatch(TEvent $event){
		if ($event->Sender === $this->_presenter_)
			//$this->Invoke(
			new TActionStateEvent($this,$event->Sender);
			//);
		else parent::Dispatch($event);
	}
	
/**
 * constructor
 * @param string $name action name
 * @param string $method name of method action should call
 * @param string $title action title
 * @param TActionParameterIterator $parameters iterator of action parameters 
 */	
	public function __construct(TPresenter $presenter, $name, $method, $title,TActionParameterIterator $parameters = null){
			$this->_local_name_ = $name;
			$this->_presenter_ = $presenter;
			$this->_method_ = $method;
			parent::__construct($presenter->Name().".".$name,$presenter->Response(),array(),$presenter,$title,$parameters);
	}
}


?>