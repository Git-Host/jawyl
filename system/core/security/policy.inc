<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Security
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package CoreRuntimeClasses
 * @subpackage Security
 * @category Core
 * Interface for implementing security objects permissions over which are checked by policy
 */
interface ISecurityObject {
/**
 * gets security object unique id.  
 */	
	public function Soid();
}

interface ISecurityChild {
/**
 * @return ISecurityObject[] 
 */	
	public function SecurityParents();
}

/**
 * @package CoreRuntimeClasses
 * @subpackage Security
 * @category Core
 * Base class of policy permissions.
 */
abstract class TPolicy {

/**
 * @var string type of permission
 */	
	protected $type;
/**
 * @var ISecurityObject security object over which permission should be checked
 * @see ISecurityObject
 */	
	protected $sobj;

/**
 * gets permission type
 * @return string
 */	
	public function Type(){return $this->type;}
/**
 * gets permission security object
 * @return ISecurityObject
 * @see ISecurityObject
 */	
	public function SecurityObject(){return $this->sobj;}

/**
 * @param string $type
 * @return boolean
 */	
	protected abstract function checkType($type);
/**
 * permission constructor
 * @param string $type permission type
 * @param ISecurityObject $so optional security object
 */	
	public function __construct($type, ISecurityObject $so = null){
		$this->sobj = $so;
		if (!$this->checkType($type))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		$this->type = $type;
	}
/**
 * gets policies that inherit permissions
 * @return TPolicy[]
 */	
	public abstract function Descenders();
}

interface IPolicyChecker {
/**
 * checks policy of security subject
 * @param ISecuritySubject $subject subject to check
 * @param TPolicy $policy policy to check
 */	
	public function CheckPolicy(ISecuritySubject $subject, TPolicy $policy);
/**
 * checks policy of several security subjects
 * @param array $subjects array of ISecuritySubject
 * @param TPolicy $policy policy to check
 */	
	public function CheckSubjectsPolicy(array $subjects, TPolicy $policy);
/**
 * checks if any of specified policies are granted to security subjects over security objects
 * @param ISecuritySubject[] $subjects list of subjects
 * @param ISecurityObject[] $objects list of objects, if empty global policies are checked 
 * @param TPolicy[] $policies list of policies
 * @return boolean 
 */	
	public function CheckSubjectsObjectsPolicies(array $subjects, array $objects, array $policies);
}

/**
 * @package CoreRuntimeClasses
 * @subpackage Security
 * @category Core
 * Interface for implementing policy componets. 
 * Policy component should implement methods for permissions and role management 
 */
interface IPolicyManager {
/**
 * Grant policy to security subject. If policy security object is set to null, a global policy is granted.
 * @param TPolicy $policy policy to grant
 * @param ISecuritySubject $subject security subject to grant policy
 * @return boolean return true if policy successfully granted
 */	
	public function Grant(TPolicy $policy, ISecuritySubject $subject);
/**
 * Deny policy to security subject. If policy security object is set to null, a global policy is denied.
 * @param TPolicy $policy policy to deny
 * @param ISecuritySubject $subject security subject to deny permission
 * @return boolean return true if policy successfully denied
 */	
	public function Deny(TPolicy $policy, ISecuritySubject $subject);
/**
 * gets array of subject explicit policies granted over object
 * @param ISecuritySubject $subject security subject
 * @param ISecurityObject $object optional object, if not specified subject global policies are returned 
 * @return array list of policy codes
 */	
	public function SubjectPolicies(ISecuritySubject $subject, ISecurityObject $object = null);
}

interface IPolicyProvider {
/** 
 * tries to wrap passed object with a security object
 * @param object $object
 * @return ISecurityObject|null
 */	
	public function SecurityWrap($object);

/**
 * tries to get a policy object
 * @param ISecurityObject $object
 * @param string $policycode
 * @return TPolicy|null
 */	
	public function GetPolicy(ISecurityObject $object,$policycode);
}

?>