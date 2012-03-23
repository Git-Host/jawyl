<?php
/**
 * @package Components
 * @subpackage Policy
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @ignore
 */
TApplication::import("core::security::policy");
TApplication::import("models::TMetaModel");

/**
 * @package Components
 * @subpackage Policy
 * @category System
 * policy component class
 */
class TPolicyModel extends TMetaModel implements IInstallable, IPolicyManager, IPolicyChecker {
	const POLICY_CLASS_POLICY = "Policy";
	
/**
 * @see TMetaModel::WrapItem()
 * @return TMetaItem returns null if passed instance is not of TPolicyModel::POLICY_CLASS_PERMISSION class.
 */	
	public function WrapItem(IInstance $item){
		if ($item->InstanceClass()->IsClass(self::POLICY_CLASS_POLICY))
			return parent::WrapItem($item);
		return null;		
	}	
	
/**
 * @see IInstallable::Install()
 */	
	public function Install(){
		$this->begin();
		try {	
			$this->dbDriver->Define(new TDBClass(self::POLICY_CLASS_POLICY,false,array(
				new TAttributeDefinition('subject', new TStringType(32),null, false, false, true),
				new TAttributeDefinition('object', new TStringType(32),null, false, false, true),
				new TAttributeDefinition('code', new TStringType(15),null, false, false, true)
			)));
			$this->dbDriver->commit();
		} catch (Exception $e){
			$this->rollback();
			throw $e;
		}
		return true;
	}
	
/**
 * @see IInstallable::IsInstalled()
 */
	public function IsInstalled(){
		return !is_null($this->dbDriver->getClass(self::POLICY_CLASS_POLICY));
	} 
/**
 * @see IInstallable::UnInstall()
 */	
	public function UnInstall(){
		$this->begin();
		try {
			$this->dbDriver->Undefine(self::POLICY_CLASS_POLICY);
			$this->dbDriver->commit();
		} catch (Exception $e){
			$this->rollback();
			throw $e;
		}
	} 
	
/**
 * grants policy
 * @return boolean
 * @see IPolicyManager::Grant()
 */	
	public function Grant(TPolicy $policy, ISecuritySubject $subject){
		$this->dbDriver->begin();
		$p = new TDBInstance(null,$this->dbDriver->getClass(self::POLICY_CLASS_POLICY));
		$p->subject = $subject->Guid();
		$p->object = null;
		if (!is_null($policy->SecurityObject()))
			$p->object = $policy->SecurityObject()->Soid();
		$p->code = $policy->Type();		
		$p = $p->Save();
		$this->commit();
		return true;
	}
/**
 * denies all permissions from security subject to security object
 * @return boolean
 */
	public function DenyAll(ISecuritySubject $subject, ISecurityObject $object){
		$this->dbDriver->begin();
		$c = $this->GetClass(self::POLICY_CLASS_POLICY);
		$ds = new TMetaClassDataSource($c, null);
		$ds->IsTarget = true;
		$ds->Filter = new TCondition(TCondition::C_EQUAL, array(new TMetaClassDataSourceAttribute("object",null,$ds),$object->Soid())); 
		$ds->Filter = new TCondition(TCondition::C_EQUAL, array(new TMetaClassDataSourceAttribute("subject",null,$ds),$subject->Guid()));
		$this->dbDriver->DeleteInstances($ds);
		$this->commit();
		return true;
	}
	
/**
 * Denies permission
 * @return boolean
 * @see IPolicyManager::Deny() 
 */	
	public function Deny(TPolicy $policy, ISecuritySubject $subject){
		$this->begin();
		try {
			$c = $this->GetClass(self::POLICY_CLASS_POLICY);
			$ds = new TMetaClassDataSource($c, null);
			$ds->IsTarget = true;
			$ds->Filter = new TCondition(TCondition::C_EQUAL, array(new TMetaClassDataSourceAttribute("code",null,$ds),$policy->Type()));
			$ds->Filter = new TCondition(TCondition::C_EQUAL, array(new TMetaClassDataSourceAttribute("object",null,$ds),is_null($policy->SecurityObject())?null:$policy->SecurityObject()->Soid())); 
			$ds->Filter = new TCondition(TCondition::C_EQUAL, array(new TMetaClassDataSourceAttribute("subject",null,$ds),$subject->Guid()));
			$this->dbDriver->DeleteInstances($ds);
			$this->commit();
		} catch (Exception $e){
			$this->rollback();
			throw $e;
		}
		return true;
	}

/**
 * checks policy
 * @return boolean
 * @see IPolicyChecker::CheckPolicy()
 */	
	public function CheckPolicy(ISecuritySubject $subject, TPolicy $policy){
		return $this->CheckSubjectsObjectsPolicies(array($subject), array(), array($policy));
	}
	
/**
 * @see IPolicyChecker::CheckSubjectsPolicy()
 * @return boolean
 */	
	public function CheckSubjectsPolicy(array $subjects, TPolicy $policy){
		return $this->CheckSubjectsObjectsPolicies($subjects, array(), array($policy));
	}
	
	private function _check_s_o(ISecurityObject $obj, array &$objs){
		$objs[] = $obj->Soid();
		if ($obj instanceof ISecurityChild){
			$parents = $obj->SecurityParents();
			foreach ($parents as $p) $objs[] = $p->Soid();
		}
	}
	
	private function _check_p_o(TPolicy $p, array &$objs, array &$pcodes){
		if ($p->SecurityObject())
			$this->_check_s_o($p->SecurityObject(),$objs);
		$pcodes[] = $p->Type();
		$syn = $p->Descendants();
		foreach ($syn as $p1)
			$this->_check_p_o($p1, $objs, $pcodes);	
	}

/**
 * @see IPolicyChecker::CheckSubjectsObjectsPolicies()
 * @return boolean
 */	
	public function CheckSubjectsObjectsPolicies(array $subjects, array $objects, array $policies){
		if (count($policies) == 0)
			return false;
			
		$c = $this->GetClass(self::POLICY_CLASS_POLICY);
		$ds = new TMetaClassDataSource($c);
		
		$code = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("code"),null,$ds);
		$subj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("subject"),null,$ds);
		$obj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("object"),null,$ds);
		
		$ds->Fetch = $code;  	
		
		$f = new TCondition(TCondition::C_IN, array($subj));
		foreach ($subjects as $s){
			$f->Operands = $subj->Guid();
			if ($subj instanceof ISecurityContext){
				$coops = $subj->Cooperators();
				foreach ($coops as $coop)
					$f->Operands = $coop->Guid();
			}
		}
		$ds->Filter = f;
		
		$objs = array();
		foreach ($objects as $obj) $this->_check_s_o($obj, $objs);
		$onull = new TCondition(TCondition::C_EQUAL, array($obj,null)); 
		if (empty($objs))
			$f = $onull;
		else {
			$f = new TCondition(TCondition::C_IN, array($obj,$objs));
			$f->Operands = array_unique($objs);
			$f = new TCondition(TCondition::C_OR,array($onull,$f));
		} 
		$ds->Filter = $f;
		
		$codes = array();
		foreach ($policies as $p)
			$this->_check_p_o($p, $objs, $codes);		 
		$f = new TCondition(TCondition::C_IN, array($code));
		$f->Operands = array_unique($codes);
		$ds->Filter = $f;
		
		$res = $this->dbDriver->FetchRecords($ds);
		$result = $res->Next();
		unset($res);
		return $result;
	}	

/**
 * @see IPolicyManager::SubjectPolicies()
 * @return array
 */	
	public function SubjectPolicies(ISecuritySubject $subject, ISecurityObject $object = null){
		$result = array();
		$c = $this->GetClass(self::POLICY_CLASS_POLICY);
		$ds = new TMetaClassDataSource($c);
		$code = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("code"),null,$ds);
		$subj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("subject"),null,$ds);
		$obj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("object"),null,$ds);
		$ds->Fetch = $code;
		$ds->Filter = new TCondition(TCondition::C_EQUAL, array($obj,($object?$object->Soid():null))); 
		$ds->Filter = new TCondition(TCondition::C_EQUAL, array($subj,$subject->Guid()));
		 
		$codes = $this->dbDriver->FetchRecords($ds);
		foreach ($codes as $code)
			$result[] = $code["code"];
		return $result;
	}
	
/**
 * gets explicit policies of subjects over object
 * @param array $subjects array of ISecuritySubject
 * @param ISecurityObject $object optional object, if not set global permissions are returned
 * @return array array of policy codes
 */	
	
	public function SubjectsPolicies(array $subjects, ISecurityObject $object = null){
		$result = array();		
		if (count($subjects) == 0){
			return $result;
		}
		$subjs = array();
		foreach ($subjects as $s) $subjs[] = $s->Guid();
		
		$c = $this->GetClass(self::POLICY_CLASS_POLICY);
		$ds = new TMetaClassDataSource($c);
		$code = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("code"),null,$ds);
		$subj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("subject"),null,$ds);
		$obj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("object"),null,$ds);
		$ds->Fetch = $code;
		$ds->Filter = new TCondition(TCondition::C_EQUAL, array($obj,($object?$object->Soid():null))); 
		$ds->Filter = new TCondition(TCondition::C_IN, array($subj,$subjs));
		
		$codes = $this->dbDriver->FetchRecords($ds);
		foreach ($codes as $code)
			$result[] = $code["code"];
		return $result;
	}
	
/**
 * gets security subjects that have any permissions over object
 * @return IIterator iterator of ISecuritySubject
 */	
	public function ObjectSubjects(ISecurityObject $object){
		$c = $this->GetClass(self::POLICY_CLASS_POLICY);
		$ds = new TMetaClassDataSource($c);
		$subj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("subject"),null,$ds);
		$obj = new TMetaClassDataSourceAttribute($c->GetAttributeDefinition("object"),null,$ds);
		$ds->Fetch = $subj;
		$ds->Filter = new TCondition(TCondition::C_EQUAL, array($obj,$object->Soid())); 
		return $this->dbDriver->FetchRecords($ds);
	}	
}
?>