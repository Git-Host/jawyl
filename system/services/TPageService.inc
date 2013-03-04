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
	public function __construct($message,$msgcode = 0){
		$this->code = $msgcode;
		$this->message = $message;
	}
	
	protected function getMessageText($msgcode){
		return $this->message;
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
 * event dispatcher proxy
 */	
	public function Dispath(TEvent $event){}
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
			$w->LoadResources();
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
			
		try {	
			$this->exception = null;
			$result = parent::Handle($request);
		} catch (TExposableException $e){
			$this->exception = $e;
			if ($this->isAjax)
				$this->AjaxEngine->Error($this,$e->getMessage());
			$result = false;	
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
		header('HTTP/1.0 403 Access denied!',true,403);
		parent::SendHeaders();
	}
}

class TNotFoundPage extends TUtilPage {
	public function SendHeaders(){
		header('HTTP/1.0 404 Page not found!',true,404);
		parent::SendHeaders();
	}
}


/**
 * @package Services
 * @subpackage PageServices
 * @category System
 * abstract page service class
 * @property IThemeEngine $ThemeEngine theme engine to use for output, can be set by name
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
	public final function Url($page,$controller = null,$method = null, array $parameters = array(),$absolute = true){
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
		
		if (!($page instanceof TPage))
			$page = $this->getPage($page);
		
		if (!($page instanceof TPage))
			return false;
		
		if ($acl instanceof IACLManager && $policy instanceof IPolicyChecker && $page instanceof TPage){
			$u = $acl->CurrentUser();
			if (!$u instanceof ISecuritySubject)
				$u = TAnonymousUser::Instance();
						
			return $policy->CheckPolicy($u,new TPagePolicy(TPagePolicy::PAGE_ACCESS,$page));
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
		if (!$request->page)
			$this->page = $this->getDefaultPage();
		 else 
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
		return $this->page->Handle($this->Request);	
	}
	
	public function CurrentPage(){
		return $this->page;
	}	
}
?>