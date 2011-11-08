<?php
/**
 * @package Controllers
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


TApplication::import("core::security::policy");
/**
 * @ignore
 */
TApplication::import("common::mvc");
/**
 * @ignore
 */
TApplication::import("common::controllers::interfaces");

/**
 * @package Controllers
 * @category System
 * class of controller interacting with current service to check permissions. 
 * Current service must implement ISecuredService interface. 
 * @see ISecuredService
 */
abstract class TSecuredController extends TController {
/**
 * gets controller policy manager
 * @return IPolicyChecker
 */
	protected function getPolicyChecker(){
		if (TApplication::CurrentService() instanceof ISecuredService)
			return TApplication::CurrentService()->Policy();
		return null;
	}
	
/**
 * gets controller policy provider
 * @return IPolicyProvider
 */
	protected abstract function getPolicyProvider();
	
/**
 * checks premission
 * @param object $object
 * @param string $policycode
 * @return boolean 
 */	
	protected function checkPermission($object, $policycode){
		$provider = $this->getPolicyProvider();
		if (!($provider instanceof IPolicyProvider))
			return true;
		$so = $provider->SecurityWrap($object); 
		if (!($so instanceof ISecurityObject))
			return true;
		$policy = $provider->GetPolicy($so, $policycode);
		if (!policy)
			return false;
		$policy_manager = $this->getPolicyManager();
		if ($policy_manager){
			if (TApplication::CurrentService() instanceof ISecuredService){
				$acl = TApplication::CurrentService()->ACL();
				if ($acl)
					return $policy_manager->CheckPolicy($acl->CurrentUser(),$policy);
			}
		}
		return false;
	}
}
?>