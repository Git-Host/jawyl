<?php
interface IFileStorage {
/**
 * @return strings
 */	
	public function FileStoragePath();
}

/**
 * @package CoreRuntimeClasses
 * @subpackage Configuration
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha
 */
interface IConfigurable {
	public function Name();
	public function RegisterComponent(IConfigurable $component);
	public function Locate($name,IConfigurable $exclude = null);
	public function Instance($name);
	public function Application();
}

interface IApplication extends IConfigurable {
/**
 * gets current request URI
 * @return string
 */
	public function Uri();
	
/**
 * gets host name
 * @return string
 */
	public function Host();
	
/**
 * gets path to directory where application is placed
 * @return string
 */
	public function AppPath();
	
/**
 * gets path to application private directory
 * @return string
 */
	public function PrivateDir();
	
/**
 * gets path to directory where compiled IOC configuration scripts are placed
 * @return string
 */
	public function IocTypesCompileDir();
	
/**
 * gets path to directory where compiled configuration scripts are placed
 * @return string
 */
	public function ConfCompileDir();
	
/**
 * gets path to directory where overriding configuration files are placed
 * @return string
 */
	public function OverrideDir();
	
/**
 * gets whether SEF urls are enabled
 * @return bool
 */
	public function Sef();
	
/**
 * get application web shared folder
 * @return string
 */
	public function Shared();
	
/**
 * gets application root URL
 * @return string
 */
	public function Root();
	
/**
 * gets output buffer
 * @return IOutputBuffer
 */
	public function Buffer();
	
/**
 * get session handler
 * @return ISessionHandler
 */
	public function Session();
	
/**
 * gets path where jawyl is placed 
 * @return string
 */
	public function SysPath();
	
/**
 * transforms a path to valid absolute path, relative to application root 
 * @param string $path
 * @return string
 */	
	
	public function AdjustPath($path);
	
/**
 * Gets a specified class Reflection object, if class does not exist tries to import it
 * @param string $classname a valid class name (with namespaces)
 * @return ReflectionClass
 */
	public function GetClass($classname);
	
/**
 * Instantiates a specified class, if class does not exist tries to import it
 * @param string $classname a valid class name (with namespaces)
 * @param array $arguments optional an array of arguments that should be passed to constructor
 * @return object
 * @see TApplication::GetClass()
 */
	public function Instantiate($classname,array $arguments = array());
	
/**
 * Gets a service with a specified name
 * @param string $name service name
 * @return TService
 */
	public function GetService($name);
	
/**
 * Makes a 302 redirect to a specified URL
 * @param string $url url where to redirect
 */
	public function Redirect($url);
	
/**
 * Gets current request URI, which is a base64 encoded serialization result of THttpRequest object
 * @return string
 * @see THttpRequest
 */
	public function RequestURI();
	
/**
 * Get current request object copy
 * @return TRequest
 */
	public function Request();
	
/**
 * Gets current service
 * @return TService
 * @see TService
 */
	public function CurrentService();
	
/**
 * @param string $service service name
 * @param array $parameters request parameters array
 * @return string
 */
	public function Url($service = null,array $parameters = array(),$base = null, $force_absolute = false);
}



interface IOutputBuffer {
	/**
	 * starts buffering.
	 */
	public function StartRecording();
	/**
	 * stops buffering and returns buffer contents.
	 * @return string
	 */
	public function StopRecording();
	
	public function Clean();
	
	public function Flush();
}

interface IClassLoader {
/**
 * @param string $classname
 * @return bool
 */	
	public function LoadClass($classname);
}
