<?php
/**
 * @package Services
 * @subpackage PageServices
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
require_once 'core/TService.inc';
require_once 'common/ajax.inc';
require_once 'common/service/navigation.inc';
require_once 'common/mvp.inc';
require_once 'core/utils/agent.inc';


class TPagePolicy extends TPolicy {
	const PAGE_ACCESS = "page.access";
	
	public function CheckType($type){
		return $type == self::PAGE_ACCESS;
	}
	
	public function Descendants(){
		return array(new TServiceRole(TServiceRole::ANONYMOUS,$this->sobj->Service()));
	}
}

class TExposableException extends TException {
	public function __construct($message,$msgcode = 256){
		$this->code = $msgcode;
		$this->message = $message;
	}
	
	protected function getMessageText($msgcode){
		return $this->message;
	}
}

class TPageSecurityObject implements ISecurityObject, ISecurityChild {
	
	private $_name_;
	
	private $_service_;
	
	public function __construct($name, TPageService $service){
		$this->_name_ = $name;
		$this->_service_ = $service;
	}
	
	/**
	 * gets security object unique id.
	 */
	public function Soid(){
		return $this->_service_->Soid().":".$this->_name_;
	}
	
	/**
	 * @return ISecurityObject[]
	 */
	public function SecurityParents(){
		return array($this->_service_);
	}	
}


/**
 * @package Services
 * @subpackage PageServices
 * @category System
 * abstract response page class
 * @property string $AjaxContentType ContentType that is used for response to ajax requests
 * @property string $ContentType the same as of TResponse, but when request is ajax returns value of $AjaxContentType property
 * @property IAjaxEngine $AjaxEngine component used foe ajax interactions
 * @property IThemeEngine $ThemeEngine
 * @property boolean $IsExceptionThrown
 * @property Exception $Exception
 * @property boolean $BuiltinErrorHandling  
 */
abstract class TPage extends TResponse implements ISecurityObject, ISecurityChild {
/**
 * @var IThemeEngine
 */	
	protected $_ioc_theme_engine_;
/**
 * @var IAjaxEngine
 */	
	protected $_ioc_ajax_engine_;
	
/**
 * @var string
 */	
	protected $name;
/**
 * @var string
 */	
	protected $title;
/**
 * @var string
 */	
	protected $templateName;
/**
 * @var boolean
 */	
	protected $isAjax = false;
/**
 * @var string
 */	
	protected $ajaxContentType = THttpMeta::TYPE_XML;
/**
 * @var array
 * stores response widgets
 */	
	protected $widgets = array();	
	
	protected $exception;
	
/**
 * constructor
 * @param string $name 
 * @param string $title optional
 * @param string $template optional
 */	
	public function __construct(TPageService $service,$name,$title = null,$template = null){
		parent::__construct($service,$name);
		$this->title = $title;
		$this->templateName = $template;
		$this->contentType = THttpMeta::TYPE_HTML;
	}
	
/**
 * @ignore
 */	
	public function __set($nm,$value){
		switch ($nm){
			case 'AjaxContentType':{
				if (!in_array($value,TResponse::ContentTypes()))
					throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				$this->ajaxContentType = $value;
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "AjaxContentType":return $this->ajaxContentType;break;
			case "ContentType":return $this->isAjax?$this->ajaxContentType:parent::__get($nm);break;
			case "AjaxEngine":{
				$result = parent::__get($nm);
				if (!($result instanceof IAjaxEngine))
					$result = $this->service->AjaxEngine;
				return $result;	
			}break;
			case "ThemeEngine":{
				$result = parent::__get($nm);
				if (!($result instanceof IThemeEngine))
					$result = $this->service->ThemeEngine;
				return $result;	
			}break;
			case 'IsExceptionThrown':{
				return $this->exception instanceof Exception;
			}break;
			case 'Exception':{
				return $this->exception;
			}break;
		}	
		return parent::__get($nm);
	}	
/**
 * gets page name
 * @return string
 */	
	public function Name(){return $this->name;}
/**
 * gets page title
 * @return string
 */	
	public function Title(){
		return $this->title;
	}
/**
 * @see ISecurityObject::Soid()
 * @return string
 */	
	public function Soid(){
		return $this->Service()->Soid().":".$this->Name();
	}
	
	public function SecurityParents(){
		return array($this->Service());
	}
/**
 * gets page template name
 * @return string
 */	
	public function TemplateName(){
		if (isset($this->templateName)){
			$v = ltrim(rtrim($this->templateName));
			if ($v == "") $v = null;
			return $v;
		}
		return null;
	}

/**
 * gets current content type of page
 * @return string 
 */	
	public function CurrentContentType(){
		if ($this->isAjax)
			return $this->AjaxContentType;
		return $this->ContentType;
	}
/**
 * loads page resources, call in templates to load widget or any other resources (scripts, css)
 * determined by page class
 */		
	public function LoadResources(){
		$widgets = $this->Widgets();
		foreach ($widgets as $w){
			$w->LoadResources($this->ThemeEngine);
		}
	}
/**
 * sends page output to client
 */	
	protected function respond(THttpRequest $request){
		$this->SendHeaders();
		$this->ThemeEngine->RenderResponse($this,$this->TemplateName());
		return true;
	}
	
	protected function beforeRespond($handle_result, THttpRequest $request){
		
	}
	
	private function _exposable_exception_handling(Exception $e, THttpRequest $request){
		$this->exception = $e;
		$result = false;
		if ($this->isAjax)
			$this->AjaxEngine->Error($this,$e->getMessage());
		else if ($request->ParameterMethod('method') == THttpRequest::PM_GET){
			$this->Application()->Session()->Set('__EXPOSED_ERROR__',array($this->exception->getCode(),$this->exception->getMessage()));
			$this->CleanCall();
		}
	}
	
/**
 * tests request for being ajax request
 * When request is not ajax handles request as TResponse do, then calls Respond method
 * When request is ajax handles request as TResponse do, then sends handling output 
 * @see TResponse::Handle()
 * @return boolean
 */
	public function Handle(THttpRequest $request){
		$this->isAjax = $request->IsAjax;
		$this->denyCallClean = $this->isAjax; 
		
		if ($this->isAjax)
			$this->Application()->Buffer()->StartRecording();
			
		$result = false;
		try {	
			$this->exception = null;
			$result = parent::Handle($request);
			if (!$this->isAjax)
				$this->beforeRespond($result, $request);
		} catch (TExposableException $e){
			$this->_exposable_exception_handling($e,$request);
		} catch (TUploadException $e){
			$this->_exposable_exception_handling($e,$request);
		} catch (Exception $e){
			if ($e->getCode() == E_USER_ERROR || $e->getCode() == E_USER_WARNING || $e->getCode() == E_USER_NOTICE)
				$this->_exposable_exception_handling($e,$request);
			else throw $e;
		}
		
		if ($exception = $this->Application()->Session()->Get('__EXPOSED_ERROR__')){
			$this->exception = new Exception($exception[1],$exception[0]);
			$this->Application()->Session()->Set('__EXPOSED_ERROR__',null);
		}
		
		if (!$this->isAjax)
			return $this->respond($request);
				
					
		$content = $this->Application()->Buffer()->StopRecording();
		$this->SendHeaders();
		$this->AjaxEngine->StartResponse($this,$this->request);
		echo $content;
		$this->AjaxEngine->CommitResponse($this);
		return $result;
	}
	
/**
 * adds an element to response 
 */	
	public function AddWidget(TWidget $widget){
		$this->widgets[$widget->Name()] = $widget;
	}	
	
/**
 * gets response widgets array
 * @return TWidget[] array of TWidget
 */	
	public function Widgets(){
		return $this->widgets;
	}	
	
	protected function processUrlParameters(array $parameters){
		return $parameters;
	}
	
	public function Url($controller = null,$method = null,array $parameters = array()){
		return $this->Service()->Url($this->Name(),$controller,$method,$this->processUrlParameters($parameters)); 
	} 	
}

abstract class TUtilPage extends TPage {
	public $Contents;
}

class TAccessDeniedPage extends TUtilPage {
	public function SendHeaders(){
		header('HTTP/1.0 403 Access denied',true,403);
		parent::SendHeaders();
	}
}

class TNotFoundPage extends TUtilPage {
	public function SendHeaders(){
		header('HTTP/1.0 404 Page not found',true,404);
		parent::SendHeaders();
	}
}


/**
 * @package Services
 * @subpackage PageServices
 * @category System
 * abstract page service class
 * @property IThemeEngine $ThemeEngine theme engine to use for output, can be set by name
 * @property IAjaxEngine $AjaxEngine
 * @property IUserAgentDetector $UADetector
 * @property string $UACookie
 * @property string $MobileRedirect
 * @property string $TabletRedirect
 */
abstract class TPageService extends TService {
	
/**
 * @var TPage
 */	
	protected $page;
	
/**
 * @var IThemeEngine
 */	
	protected $_ioc_theme_engine_;
/**
 * @var IAjaxEngine
 */	
	protected $_ioc_ajax_engine_;

/**
 * @var IUserAgentDetector
 */
	protected $_ioc_u_a_detector_;	
	
	protected $_conf_mobile_redirect_;
	
	protected $_conf_tablet_redirect_;
	
	protected $_conf_u_a_cookie_;
	
/**
 * gets page by page id 
 * @param mixed $id
 * @return TPage
 */	
	protected abstract function getPage($id);
/**
 * gets default page
 * @return TPage
 */	
	protected abstract function getDefaultPage();
/**
 * gets page to show when no page fit request
 * @return TPage
 */	
	protected abstract function getNotFoundPage();
/**
 * gets page to show when user is not allowed to view a requested page
 * @return TPage  
 */	
	protected abstract function getAccessDeniedPage();
	
	protected function formAppUrlParameters($page,$controller = null,$method = null,array $parameters = array()){
		$temp["page"] = $page;
		if (!is_null($controller)) $temp["controller"] = $controller;
		if (!is_null($method)) $temp["method"] = $method;
		return $temp + $parameters;
	}
		
/**
 * gets an url to specified page
 * @param string $page page unique id
 * @param string $controller optional controller name for method calls
 * @param string $method optional method name
 * @param array $parameters associative array of method call and request parameters
 * @return string 
 * @see TApplication::Url()
 */	
	public final function Url($page = null,$controller = null,$method = null, array $parameters = array(),$absolute = true){
		if ($this->page){
			if (is_null($page))
				$page = $this->page->Name();
			
			if ($page != $this->page->Name())
				if (!$this->_check_page_access($page))
					return null;
			
		} else if (!$this->_check_page_access($page))
			return null;	
		
		return $this->Application()->Url($this->name,$this->formAppUrlParameters($page, $controller, $method, $parameters),null,$absolute);
	}
	
	public function RedirectToNotFound(){
		$this->Application()->Redirect($this->Url($this->getNotFoundPage()->Name()));
	}
	
	public function RedirectToAccessDenied(){
		$this->Application()->Redirect($this->Url($this->getAccessDeniedPage()->Name()));
	}
	
	private function _check_page_access($page){
		$policy = $this->Policy();
		$acl = $this->ACL();
		
		if ($acl instanceof IACLManager && $policy instanceof IPolicyChecker){
			$u = $acl->CurrentUser();
			if (!$u instanceof ISecuritySubject)
				$u = TAnonymousUser::Instance();
						
			return $policy->CheckPolicy($u,new TPagePolicy(TPagePolicy::PAGE_ACCESS,($page instanceof TPage)?$page:new TPageSecurityObject($page, $this)));
		}	
		return true;
	}
	
/**
 * checks access to current page
 * @return boolean
 */	
	protected function checkAccess(){
		return $this->_check_page_access($this->page);
	}
	
	protected function checkRequest(TRequest $request){
		return $request instanceof THttpRequest;
	}
		
/**
 * gets page that fits request and calls its Handle method
 * @return boolean
 */		
	protected function handleRequest(TRequest $request){	
		if (!$request->page){
			$ua = $this->UACookie?$request->Cookie($this->UACookie):false;
			if ($nm = $this->UACookie){
				if ($request->$nm){
					setcookie($nm,$request->$nm);
					$ua = $request->$nm;
				}
			}
			
			if ($this->UADetector || $ua){
				if ($this->MobileRedirect && (($this->UADetector->IsMobile() && $ua != 'desktop') || $ua == 'mobile'))
					$this->Application()->Redirect($this->MobileRedirect);
				if ($this->TabletRedirect && (($this->UADetector->IsTablet() && $ua != 'desktop') || $ua == 'tablet'))
					$this->Application()->Redirect($this->TabletRedirect);
			}			
			
			$this->page = $this->getDefaultPage();
		} else 
			$this->page = $this->getPage($request->page);	
		if (is_null($this->page)){
			$this->page = $this->getNotFoundPage();
			if (is_null($this->page)){
				header('HTTP/1.0 404 Page not found!',true,404);
				die;	
			}
		} else if (!$this->checkAccess()){
			$this->page = null;
			$this->page = $this->getAccessDeniedPage();
		}
		if (!($this->page instanceof TPage))
			return false;
		try {	
			return $this->page->Handle($this->Request);
		} catch (TCoreException $e){
			switch ($e->getCode()){
				case TCoreException::ERR_ACCESS_DENIED:{
					if ($page = $this->getAccessDeniedPage())
						$this->Application()->Redirect($page->Url());
				}break;
				case TCoreException::ERR_RESOURCE_NOT_FOUND:{
					if ($page = $this->getNotFoundPage())
						$this->Application()->Redirect($page->Url());
				}break;
			}
			throw $e;
		}
		return false;	
	}
	
	public function CurrentPage(){
		return $this->page;
	}	
}
?>