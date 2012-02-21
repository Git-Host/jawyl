<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * Interface for implementing session handling. Note the magic setter and getter in function list. 
 * Session handler should provide access to session variables through its properties.  
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 */
	interface ISessionHandler {
/**
 * Opens an application session 
 */		
		public function Open();
		
/**
 * Closes an application session 
 */		
		public function Close();
/**
 * Destroys an application session 
 */		
		public function Destroy();
/**
 * Performs garbage collection  
 * @param int $maxlifetime time in seconds after which data is treated as garbage
 */		
		public function GarbageCollection($maxlifetime);
/**
 * get session id, should return null if session is closed
 * @return string  
 */		
		public function Id();
		
/**
 * resets session id, returns true on success
 * @return boolean
 */		
		public function ResetId();
/**
 * gets session variable.
 * @param string $nm name of variable
 * @return mixed  
 */		
		public function Get($nm);
/**
 * sets session variable.
 * @param string $nm name of variable
 * @param mixed $value value of variable  
 */		
		public function Set($nm,$value);
/**
 * Adds handler to session open event.
 * If callback returns false all subsequent handling aborts. To add a handler you need to close session and reopen it.
 * @param callback $callback a function to be executed before session is opened.   
 */		
		public function AddOnOpen($callback);
/**
 * Adds handler to session close event
 * If callback returns false all subsequent handling aborts. To add a handler you need to close session and reopen it.
 * @param callback $callback a function to be executed before session is closed  
 * 
 */		
		public function AddOnClose($callback);
/**
 * Adds handler to session destroy event
 * If callback returns false all subsequent handling aborts. To add a handler you need to close session and reopen it.
 * @param callback $callback a function to be executed before session is destroyed  
 */		
		public function AddOnDestroy($callback);
/**
 * Adds handler to session garbage collection event
 * If callback returns false all subsequent handling aborts. To add a handler you need to close session and reopen it.
 * @param callback $callback a function to be executed before garbage collection starts  
 */		
		public function AddOnGarbageCollection($callback);
	}
	
/**
 * Abstract configurable session handler class. Derives from TConfigurable and implements ISessionHandler.
 * Can be used as base class for special configurable session handlers.
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * @see ISessionHandler
 * @see TConfigurable 
 */	
	abstract class TConfigurableSessionHandler extends TConfigurable implements ISessionHandler {
/**
 * types of handlers
 */		
		const HANDLE_OPEN = "open";
		const HANDLE_CLOSE = "close";
		const HANDLE_DESTROY = "destroy";
		const HANDLE_GC = "gc";
/**
 * @var array associative array of event handlers
 */		
		protected static $handlers = array();
/**
 * @param string $name
 */		
		public function __construct($name){
			parent::__construct($name);
		}
		
/**
 * Adds a handler of specified type to handlers array
 * @param string $type type of handler, can be one of the four constans HANDLE_OPEN,HANDLE_CLOSE,HANDLE_DESTROY,HANDLE_GC
 * @param callback $callback a function to be executed on event 
 */		
		protected function addHandler($type,$callback){
			if (!in_array($type,array(self::HANDLE_OPEN,self::HANDLE_CLOSE,self::HANDLE_DESTROY,self::HANDLE_GC))){
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				return;
			}
				
			if (!key_exists($type,self::$handlers))
				self::$handlers[$type] = array();
			self::$handlers[$type][] = $callback;	
		}

/**
 * Calls handlers of specified type. Can be used in descendants.
 * @param string $type type of handler, can be one of the four constans HANDLE_OPEN,HANDLE_CLOSE,HANDLE_DESTROY,HANDLE_GC
 * @param array $params optional array of parameters passed to handlers 
 */		
		protected function callHandlers($type,$params = array()){
			if (!in_array($type,array(self::HANDLE_OPEN,self::HANDLE_CLOSE,self::HANDLE_DESTROY,self::HANDLE_GC))){
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				return;
			}
			$result = true;
			if (isset($this->HANDLERS[$type])){
				foreach ($this->HANDLERS[$type] as $callback){
					$result = call_user_func_array($callback,$params);
					if (!$result) break;
				}
			}
			return $result;
		}
		
/**
 * implements ISessionHandler::AddOnGarbageCollection
 * @see ISessionHandler
 */		
		public function AddOnOpen($callback){
			$this->addHandler("open",$callback);
		}
		
/**
 * implements ISessionHandler::AddOnGarbageCollection
 * @see ISessionHandler
 */		
		public function AddOnClose($callback){
			$this->addHandler("close",$callback);
		}
		
/**
 * implements ISessionHandler::AddOnGarbageCollection
 * @see ISessionHandler
 */		
		public function AddOnDestroy($callback){
			$this->addHandler("destroy",$callback);
		}
		
/**
 * implements ISessionHandler::AddOnGarbageCollection
 * @see ISessionHandler
 */		
		public function AddOnGarbageCollection($callback){
			$this->addHandler("gc",$callback);
		}	
	}
	
/**
 * Default session handler. Uses native php session handling. It is used to create a default session handler, when no other specified.
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * @see TConfigurableSessionHandler
 * */	

	class TSessionHandler extends TConfigurableSessionHandler implements ISecurityObject {
/**
 * @var string stores php session save path
 */		
		protected $savePath;
		
		protected $isOpen = false;
		
		public function Id(){
			if ($this->isOpen)
				return session_id();
			return null;
		}
		
		public function ResetId(){
			if ($this->isOpen)
				return session_regenerate_id(false);
			return false;
		}
		
		public function Soid(){
			return TApplication::$Application->Name()."::".$this->Name();
		}
		
		public function Open(){
			if (!$this->isOpen){
				session_set_save_handler(array(&$this,"SessOpen"), array(&$this,"SessClose"), array(&$this,"SessRead"), array(&$this,"SessWrite"), array(&$this,"SessDestroy"), array(&$this,"GarbageCollection"));
				session_start();
				$this->isOpen = true;
			}
		}
		
		public function Close(){
			session_commit();
			$this->isOpen = false;	
			return true;
		}
		
		public function Destroy(){
			session_destroy();
		}
		
/**
 * php session open handler
 */		
		
		public function SessOpen($sess_path, $sess_name){
			if (!$this->savePath)
				$this->savePath = realpath($sess_path);
			if (!$this->callHandlers("open",array("sess_path"=>$sess_path,"sess_name"=>$sess_name))) return false;
			return true;
		}
		
/**
 * php session close handler
 */		
		public function SessClose(){
			if (!$this->CallHandlers("close",array())) return false;
			return true;
		}
				
/**
 * php session read handler
 */		
		public function SessRead($id){
			$sess_file = $this->savePath."/sess_$id";
 	 		if (file_exists($sess_file))
    			return file_get_contents($sess_file);
    		return "";
		}
		
/**
 * php session write handler
 */		
		public function SessWrite($id, $sess_data){
 	 		$sess_file = $this->savePath."/sess_$id";
 	 		file_put_contents($sess_file,$sess_data);
 	 		return true;
		}
		
/**
 * php session destroy handler
 */		
		public function SessDestroy($id){
			if (!$this->CallHandlers("destroy",array("id"=>$id))) return false;
			$sess_file = $this->savePath."/sess_$id";
			return(@unlink($sess_file));
		}
		
/**
 * php session garbage collection handler
 */		
		public function GarbageCollection($maxlifetime){
			if (!$this->CallHandlers("gc",array("maxlifetime"=>$maxlifetime))) return false;
			$this->gc($maxlifetime);
  			return true;
		}
		
		protected function gc($maxlifetime){
			$dh = opendir($this->savePath);
			while ($fn = readdir($dh)){
				$pi = pathinfo($fn);
				if (preg_match('/sess_[a-f0-9]{32}/',$fn))
					if (filemtime($this->savePath."/".$fn)+$maxlifetime < time())
						@unlink($this->savePath."/".$fn);
			}
		}
		
		protected function schedule_GarbageCollection($maxlifetime){
			$this->gc($maxlifetime);
		}
	
		public function Get($nm){
			if (isset($_SESSION))
				if (key_exists($nm,$_SESSION))
					return $_SESSION[$nm];
			return null;
		}
	
		public function Set($nm,$value){
			if (isset($_SESSION))
				$_SESSION[$nm] = $value;
		}	

		public function __set($nm,$value){
			switch ($nm){
				case 'SavePath':$this->savePath = TApplication::$Application->AdjustPath($value);break;
				default:parent::__set($nm, $value);break;
			}
		}
	}
	
/**
 * Class providing static access to application session.  
 * @package CoreRuntimeClasses
 * @subpackage Application
 * @category Core
 * @see ISessionHandler
 * @see TConfigurableSessionHandler
 * */	
	
	final class TApplicationSession {
/**
 * @var ISessionHandler current session handler object
 * @see ISessionHandler
 */		
		protected static $session;

/**
 * Gets current session handler object.
 * @returns ISessionHandler
 */		
		public static function Handler(){
			if (self::$session instanceof ISessionHandler)
				return self::$session;
			return null;
		} 
		
/**
 * Gets current session id. Returns null for closed session.
 * @return string
 */		
		public static function SessionId(){
			if (self::Handler())
				return self::Handler()->Id();
			return null; 
		}
/**
 * resets session id to a new generated value
 * @return boolean
 */		
		public static function ResetId(){
			if (!self::SessionId())
				throw new TCoreException(TCoreException::ERR_SESS_CLOSED);
			return self::Handler()->ResetId();
		} 
		
/**
 * Gets session variable by name.
 * @param string $nm name of variable
 */		
		public static function Get($nm){
			if (!self::SessionId()) self::Open();
			/*if (!self::Handler()){
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
				return null;
			}*/	
			return self::Handler()->Get($nm);
		}
		
/**
 * Sets session variable by name.
 * @param string $nm name of variable
 * @param mixed $value value of variable 
 */		
		public static function Set($nm,$value){	
			if (!self::SessionId()) self::Open();
			/*if (!self::Handler())
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);*/
			self::Handler()->Set($nm,$value);
		}
		
/**
 * Sets current session handler. When session is open calling this methods throws exception.
 * Passing null forces setting handler to default. 
 * @param ISessionHandler $handler session handler 
 */		
		public static function SetHandler(ISessionHandler $handler = null){
			if (self::SessionId()){
				throw new TCoreException(TCoreException::ERR_SESS_OPEN);
				return;
			}
			if (!is_null($handler))
				self::$session = $handler;
			else
				self::$session = new TSessionHandler("default_session_handler");
		} 

/**
 * opens session 
 */
		public static function Open(){
			self::Close();
			if (!self::Handler())
				self::SetHandler();
			return self::Handler()->Open();
		}
		 
/**
 * closes session 
 */
		public static function Close(){
			if (self::SessionId())
				return self::Handler()->Close();
		}
		 
/**
 * destroys session 
 */
		public static function Destroy(){
			if (!self::Handler())
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			return self::Handler()->Destroy();
		} 

/**
 * adds handler to session destroy event 
 * @param callback $callback
 * @see ISessionHandler
 */
		public static function AddOnDestroy($callback){
			if (!self::Handler())
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			self::Handler()->AddOnDestroy($callback);
		}
		
/**
 * adds handler to session open event 
 * @param callback $callback
 * @see ISessionHandler
 */
		public static function AddOnOpen($callback){
			if (!self::Handler())
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			self::Handler()->AddOnOpen($callback);
		}
		
/**
 * adds handler to session close event
 * @param callback $callback 
 * @see ISessionHandler
 */
		public static function AddOnClose($callback){
			if (!self::Handler())
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			self::Handler()->AddOnClose($callback);
		}
		
/**
 * adds handler to session garbage collection event 
 * @param callback $callback
 * @see ISessionHandler 
 */
		public static function AddOnGarbageCollection($callback){
			if (!self::Handler())
				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
			self::Handler()->AddOnGarbageCollection($callback);
		}
	}
?>