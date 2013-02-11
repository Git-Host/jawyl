<?php
/**
 * @package Presenters
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/


require_once 'core/security/policy.inc';
/**
 * @ignore
 */
require_once 'common/mvp.inc';
/**
 * @ignore
 */
require_once 'common/presenters/interfaces.inc';

/**
 * @package Presenters
 * @category System
 * class of presenter interacting with current service to check permissions. 
 * Current service must implement ISecuredService interface. 
 * @see ISecuredService
 */
abstract class TSecuredPresenter extends TPresenter {
/**
 * gets presenter policy manager
 * @return IPolicyChecker
 */
	protected function getPolicyChecker(){
		if ($this->Application()->CurrentService() instanceof ISecuredService)
			return $this->Application()->CurrentService()->Policy();
		return null;
	}
	
/**
 * gets presenter policy provider
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
			if ($this->Application()->CurrentService() instanceof ISecuredService){
				$acl = $this->Application()->CurrentService()->ACL();
				if ($acl)
					return $policy_manager->CheckPolicy($acl->CurrentUser(),$policy);
			}
		}
		return false;
	}
}
?>