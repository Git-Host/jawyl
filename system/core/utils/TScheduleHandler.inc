<?php
require_once 'core/security/acl.inc';
require_once 'core/security/policy.inc';
require_once 'core/TService.inc';

class TJobPolicy extends TPolicy {
	const JOB_EXECUTE = "job.exec"; 
	
	public function __construct(ISecurityObject $so){
		parent::__construct(self::JOB_EXECUTE,$so); 
	}
	
	protected function checkType($type){
		return $type == self::JOB_EXECUTE;
	}

	public function Descendants(){
		if (($this->type == self::JOB_EXECUTE) && ($this->sobj instanceof IConfigurable))
			return array(new TServiceRole(TServiceRole::ADMIN,$this->sobj->Application()->CurrentService()));
		return array();
	}
} 

class TJobSecurityObject implements ISecurityObject, ISecurityChild {
	private $_handler_;
	private $_job_;
	
	public function __construct($jobname, ISecurityObject $handler){
		$this->_job_ = $jobname;
		$this->_handler_ = $handler;
	}
	
	public function Soid(){
		return $this->_handler_->Soid()."::".$this->_job_;
	}
	
	public function SecurityParents(){
		return array($this->_handler_);
	}
}

interface IScheduleHandler extends ISecurityObject {
	public function ExecuteJob(IACLManager $acl, IPolicyChecker $policy, $jobname, array $parameters = array());
}

class TScheduleHandler extends TConfigurable implements IScheduleHandler {
	public function ExecuteJob(IACLManager $acl, IPolicyChecker $policy, $jobname, array $parameters = array()){
		$mn = "schedule_".$jobname;
		$rm = new ReflectionMethod(get_class($this), $mn);
		if ($rm)
			if (!$rm->isPublic())
				if ($policy->CheckPolicy($acl->CurrentUser(),new TJobPolicy(new TJobSecurityObject($jobname, $this)))) {
					call_user_func_array(array($this,$mn), $parameters);
					return true;
				}
		return false;	
	}
	
	public function Soid(){
		return $this->name;
	}
}
