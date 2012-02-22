<?php
/**
 * @package Services
 * @subpackage SysServices
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("core::TService");
TApplication::import("common::service::interfaces");
TApplication::import("common::soap");


class TSOAPService extends TService {
	protected function checkRequest(TRequest $request){
		return $request instanceof THttpRequest;
	}
	
	protected function handleRequest(TRequest $request){
		$com = null;
		if ($request->component)
			$com = TApplication::Instance($request->component);
		if ($com instanceof ISoapServer){
			if ($request->wsdl){
				echo $com->WSDL();
				return true;
			} 
			$com->Handle();
			return true;
		}	
		return false;
	}
}
