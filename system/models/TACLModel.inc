<?php
/**
 * @package Components
 * @subpackage ACL
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("models::TOrmModel");

/**
 * @package Components
 * @subpackage ACL
 * @category System
 * security subject implementation
 * @see ISecuritySubject
 * @see TOrmItem
 */
class TSecuritySubject extends TOrmItem implements ISecurityContext {
/**
 * constructor
 * @param IInstance $instance instance to wrap
 * @param TACLModel $com ACL component to interact with
 */	
	public function __construct(IInstance $instance, TACLModel $com){
		parent::__construct($instance,$com);
		unset($this->properties["guid"]);
	}
/**
 * object string representation. returns subject guid.
 * @return string
 */	
	public function __toString(){return $this->guid;}	
/**
 * gets security subject guid
 * @return string
 */	
	public function Guid(){return $this->guid;}
	
	public function Cooperators(){
		return $this->component->SubjectGroups($this);
	}
}

/**
 * @package Components
 * @subpackage ACL
 * @category System
 * group class
 * @see TSecuritySubject
 */
class TGroup extends TSecuritySubject {
/**
 * string representation. returns group name.
 * @return string
 */	
	public function __toString(){return $this->name;}
}

/**
 * @package Components
 * @subpackage ACL
 * @category System
 * user account class
 * @see TSecuritySubject
 */
class TUser extends TSecuritySubject {
/**
 * method for hashing values. uses md5 hashing algorithm.
 * @param string $value
 * @return string
 */	
    public static function Hash($value){return md5($value);}
/**
 * overriden method for setting attribute values. Hashes value for password attribute of user account.
 * @see IInstance::SetAttributeValue()
 */    
    public function SetAttributeValue($name,$value,$needsave = true){
    	if ($this->InstanceClass()->GetAttributeDefinition($name)->Type()->TypeCode == TAttributeType::PASSWORD)
    		$value = self::Hash($value);
    	parent::SetAttributeValue($name,$value,$needsave);
    }
/**
 * overriden method for getting attribute values. For password attribute returns empty string.
 * @see IInstance::SetAttributeValue()
 */    
    public function GetAttributeValue($name){
    	if ($this->InstanceClass()->GetAttributeDefinition($name)->Type() == TAttributeType::PASSWORD) return "";
    	return parent::GetAttributeValue($name);
    }
/**
 * string representation. returns value of login attribute.
 * @return string
 */    
    public function __toString(){return $this->login;}
}

/**
 * @package Components
 * @subpackage ACL
 * @category System
 * security items iterator. converts orm items to subsequent security item class. 
 * @see TIteratorAdapter
 */
class TSecurityItemIterator extends TIteratorAdapter {
	private $_component_;
/**
 * gets current iterator item
 * @return TUser|TGroup
 */	
	public function Item(){
		$dbi = $this->iterator->Item();
		$result =  $this->_component_->WrapItem($dbi);
		return $result;			
	}

/**
 * constructor.
 * @param IIterator $iterator orm items iterator
 * @param TACLModel $com acl component to interact with
 */	
	public function __construct(IIterator $iterator, TACLModel $com){
		parent::__construct($iterator);
		$this->_component_ = $com;
	}		
}

/**
 * @package Components
 * @subpackage ACL
 * @category System
 * acl component class 
 * @see TOrmModel
 * @see IInstallable
 * @see IACLManager
 */
class TACLModel extends TOrmModel implements IInstallable, IACLManager {
	const SECURITY_SUBJECT_CLASS = "SecuritySubject";
	const SECURITY_USER_CLASS = "User";
	const SECURITY_GROUP_CLASS = "Group";
	
	const SECURITY_RELATIONS = "GroupAssignment";

/**
 * @var int stores current user id
 */	
	protected $currentUID;
/**
 * installation method
 * @see IInstallable::Install()
 */	
	public function Install(){
		$this->begin();	
		$this->dbDriver->DefineClass(self::SECURITY_SUBJECT_CLASS,self::SECURITY_SUBJECT_CLASS);
		$this->dbDriver->DefineAttribute(self::SECURITY_SUBJECT_CLASS,"guid","guid", new TGuidType(true,false,true));
			
		$this->dbDriver->DefineClass(self::SECURITY_GROUP_CLASS,self::SECURITY_GROUP_CLASS,self::SECURITY_SUBJECT_CLASS);
		$this->dbDriver->DefineAttribute(self::SECURITY_GROUP_CLASS,"name","name", new TStringType(50,true,true,false));
			
		$this->dbDriver->DefineClass(self::SECURITY_USER_CLASS,self::SECURITY_USER_CLASS,self::SECURITY_SUBJECT_CLASS);
		$this->dbDriver->DefineAttribute(self::SECURITY_USER_CLASS,"login","login", new TStringType(20,true,true,false));
		$this->dbDriver->DefineAttribute(self::SECURITY_USER_CLASS,"password","pwd", new TPasswordType(32,false,true,true,""));
		$this->dbDriver->DefineAttribute(self::SECURITY_USER_CLASS,"disabled","disabled", new TBooleanType(false,false,0));
			
		$this->dbDriver->DefineStructure(self::SECURITY_RELATIONS,"Group assignments",TStructureType::ASSOCIATION,"GroupAssignments");
		$this->commit();
		return true;
	}

/**
 * @see IInstallable::IsInstalled()
 */
	public function IsInstalled(){
		$secsub = $this->dbDriver->getClass(self::SECURITY_SUBJECT_CLASS);
		$group = $this->dbDriver->getClass(self::SECURITY_GROUP_CLASS);
		$user = $this->dbDriver->getClass(self::SECURITY_USER_CLASS);
		$ga = $this->dbDriver->IsStructureDefined(self::SECURITY_RELATIONS,TStructureType::ASSOCIATION);
		return (!is_null($user) && !is_null($group) && !is_null($secsub) && $ga);
	} 
	
/**
 * uninstallation method
 * @see IInstallable::UnInstall()
 */	
	public function UnInstall(){
		$this->begin();
		$this->dbDriver->UnDefineStructure(self::SECURITY_RELATIONS);
		$this->dbDriver->UndefineClass(self::SECURITY_USER_CLASS);
		$this->dbDriver->UndefineClass(self::SECURITY_GROUP_CLASS);
		$this->dbDriver->UndefineClass(self::SECURITY_SUBJECT_CLASS);
		$this->commit();
	} 

/**
 * @see TOrmModel::CreateItem()
 */	
	public function CreateItem(IClass $c,array $parameters){
		if ($c->IsClass(self::SECURITY_USER_CLASS))
			$result = $this->CreateUser($parameters["login"],$parameters["pwd"]);
		else if ($c->IsClass(self::SECURITY_GROUP_CLASS))
			$result = $this->CreateGroup($parameters["name"]);
		else $result = parent::CreateItem($c,$parameters); 
		return $result;
	}

/**
 * creates new user account. Used in TACLModel::CreateItem()
 * @param string $login user login
 * @param string $pwd user password
 * @return TUser
 */	
	public function CreateUser($login,$pwd){
		$user = new TUser(new TDBInstance(null,$this->dbDriver->getClass(self::SECURITY_USER_CLASS)),$this);
		$user->login = $login;
		$user->pwd = $pwd;
		$user->guid = TSecuritySubject::GuidGen();
		$user = $this->WrapNCache($user->Save());
		return $user;
	}
	
/**
 * creates new group. Used in TACLModel::CreateItem()
 * @param string $name group name
 * @return TGroup
 */	
	public function CreateGroup($name){
		$group = new TGroup(new TDBInstance(null,$this->dbDriver->getClass(self::SECURITY_GROUP_CLASS)),$this);
		$group->guid = TSecuritySubject::GuidGen();
		$group->name = $name;
		$group = $this->WrapNCache($group->Save());
		return $group;
	}

/**
 * @see TOrmModel::WrapItem()
 * @return TUser|TGroup
 */	
	public function WrapItem(IInstance $item){
		if ($item->InstanceClass()->IsClass(TACLModel::SECURITY_USER_CLASS))
			return new TUser($item,$this);
		else if ($item->InstanceClass()->IsClass(TACLModel::SECURITY_GROUP_CLASS))
			return new TGroup($item,$this);
		/*else
		    $item = new TSecuritySubject($dbi,$this);*/
		return null;		
	}

/**
 * @see IACLManager::UserExists()
 * @param string $login user login
 * @return boolean
 */	
	public function UserExists($login){
		$fo = new TFetchOptions();
		$c = $this->GetClass(self::SECURITY_USER_CLASS);
		$fo->AddCondition(new TCondition(TCondition::C_EQUAL,array($c->GetAttributeDefinition("login"),$login)));
		$res = $this->dbDriver->GetClassInstances(self::SECURITY_USER_CLASS,false,$fo);
		return $res->ItemCount() > 0;
	}
		
/**
 * @see IACLManager::Login()
 * @param string $login user login
 * @param string $pwd user password
 */	
	public function Login($login,$pwd){
		$id = $this->doLogin($login,$pwd);
		if ($id){
			$this->currentUID = $id;
			if ($this->Persistance){
				TApplicationSession::ResetId();
				TApplicationSession::Set("current_uid",$this->currentUID);
			}
		}
		else {
			$this->currentUID = null;
			if ($this->Persistance)
				TApplicationSession::Set("current_uid",null);
		}
	}
	
/**
 * @see IACLManager::Logout()
 */	
	public function Logout(){
		$this->currentUID = null;
		if ($this->Persistance)
			TApplicationSession::Set("current_uid",null);
	}
	
/**
 * @see IACLManager::CurrentUser()
 * @return TUser
 */	
	public function CurrentUser(){
		if (is_null($this->currentUID))
			if ($this->Persistance)
				$this->currentUID = TApplicationSession::Get("current_uid");
		if ($this->currentUID)		
			return $this->GetItem($this->currentUID);
		return null;
	}	

/**
 * gets groups specified security subject belongs to.
 * @param TSecuritySubject $subject security subject
 * @param boolean $onlydirect optional, defaults to false. When set to true only explicit groups are returned.
 * @return TSecuritySubject[]
 * @see TSecuritySubject
 */		
	protected function subjGroups(TSecuritySubject $subject, $onlydirect = false){
		$result = array();
		$temp = new TSecurityItemIterator($this->dbDriver->Subjects(self::SECURITY_RELATIONS,$subject),$this);
		while ($temp->Next()){
			$result[$temp->Item()->Guid()] = $temp->Item();
			if (!$onlydirect){
				$tresult = $this->subjGroups($temp->Item(),false);
				$result = array_merge($result,$tresult);
			}
		}
		return $result;
	}
	
/**
 * gets groups specified security subject belongs to.
 * @param TSecuritySubject $subject security subject
 * @param boolean $onlydirect optional, defaults to false. When set to true only explicit groups are returned.
 * @return IIterator iterator of TSecuritySubject
 * @see TSecuritySubject
 */		
	public function SubjectGroups(TSecuritySubject $subject, $onlydirect = false){
		return $this->subjGroups($subject,$onlydirect);
	}
	
/**
 * adds security subject to group
 * @param TSecuritySubject $subject security subject
 * @param TGroup $group group
 * @return boolean
 */	
	public function AddToGroup(TSecuritySubject $subject, TGroup $group){
		$groups = $this->subjGroups($subject);
		if ($subject instanceof TGroup)
			$groups[$subject->Guid()] = $subject;
		if (!isset($groups[$group->Guid()]))
			return $this->dbDriver->Associate(self::SECURITY_RELATIONS,$group,$subject);
		else
			throw new TACLException(TACLException::ERR_ACL_GROUPCYCL);
		return false;
	}
	
/**
 * removes security subject from group
 * @param TSecuritySubject $subject security subject
 * @param TGroup $group group
 * @return boolean
 */	
	public function RemoveFromGroup(TSecuritySubject $subject, TGroup $group){
		return $this->dbDriver->Deassociate(self::SECURITY_RELATIONS,$group,$subject);
	}
	
/**
 * gets user account id by login and password. if account not found returns false.
 * @param string $login
 * @param string $pwd
 * @return int|false
 */	
	
	protected function doLogin($login,$pwd){			
		/*if (strlen($pwd) == 0) 
			$password = null;
		else*/
			$password = TUser::Hash($pwd);
		$fo = new TFetchOptions();
		
		$c = $this->GetClass(self::SECURITY_USER_CLASS);
		
		$fo->AddCondition(new TCondition(TCondition::C_EQUAL,array($c->GetAttributeDefinition("login"),$login)));
		$fo->AddCondition(new TCondition(TCondition::C_EQUAL,array($c->GetAttributeDefinition("pwd"),$password)));
		
		$res = $this->dbDriver->GetClassInstances(self::SECURITY_USER_CLASS,false,$fo);
		if ($res->Next()){
			$temp = $res->Item();
			return $temp->Id();
		} 
		throw new TACLException(TACLException::ERR_ACL_USER_UNDEFINED,array("login"=>$login));
		return false;
	}
}

?>