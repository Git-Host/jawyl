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
 * interface of security subject, against which permissions should be checked
 */
interface ISecuritySubject{
	/**
	 * get security subject unique id.
	 */
	public function Guid();
}

class TAnonymousUser implements ISecuritySubject {
	private static $_instance_;
	
	private function __construct(){
		self::$_instance_ = $this;
	}
	
	public static function Instance(){
		if (self::$_instance_)
			return self::$_instance_;
		return new TAnonymousUser();	
	}
	
	public function Guid(){
		return null;
	}
}

interface ISecurityContext extends ISecuritySubject {
/** 
 * @return ISecuritySubject[]
 */	
	public function Cooperators();
}

/**
 * @package CoreRuntimeClasses
 * @subpackage Security
 * @category Core
 * interface for ACL component implementation
 * ACL component should provide methods for user authentication
 */
interface IACLManager {
	/**
	 * checks whether specified login exists 
	 * @param string $login
	 * @return boolean
	 */
	public function UserExists($login);
	/**
	 * performs user authentication
	 * @param string $login user login
	 * @param string $pwd user password
	 * @return boolean
	 */
	public function Login($login,$pwd);
	/**
	 * performs user logout
	 */
	public function Logout();
	/**
	 * returns current logged user
	 * @return ISecuritySubject
	 */
	public function CurrentUser();
}

/**
 * @package CoreRuntimeClasses
 * @subpackage Security
 * @category Core
 * interface for ACL component implementation
 * ACL exception class
 */
final class TACLException extends TException {
	const ERR_ACL_NO_CONTEXT = 201;
	const ERR_ACL_USER_UNDEFINED = 202;
/**
 * @param int $msgcode
 * @return string
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_ACL_USER_UNDEFINED:return 'User "%login" undefined!';break;
			case self::ERR_ACL_NO_CONTEXT:return 'No security context is available!';break;
		}
	}
}
?>