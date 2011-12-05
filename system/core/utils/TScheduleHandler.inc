<?php
TApplication::import("core::security::acl");
TApplication::import("core::security::policy");
TApplication::import("core::TService");

class TJobPolicy extends TPolicy {
	const JOB_EXECUTE = "job.exec"; 
	
	public function __construct($type,ISecurityObject $so){
		parent::__construct($type,$so); 
	}
	
	protected function checkType($type){
		return $type == self::JOB_EXECUTE;
	}

	public function Descendants(){
		if ($this->type == self::JOB_EXECUTE)
			return array(new TServiceRole(TServiceRole::ADMIN));
		return array();
	}
} 

class TJobSecurityObject implements ISecurityObject, ISecurityChild {
	private $_handler_;
	private $_job_;
	
	public function __construct($jobname, ISecurityOBject $handler){
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


class TScheduleHandler {
	public static function ExecuteJob(ISecurityObject $object, IACLManager $acl, IPolicyChecker $policy, $jobname, array $parameters = array()){
		$rm = new ReflectionMethod(get_class($object), "schedule_".$jobname);
		if ($rm)
			if (!$rm->isPublic())
				if ($policy->CheckPolicy($acl->CurrentUser(),new TJobPolicy(TJobPolicy::JOB_EXECUTE,new TJobSecurityObject($jobname, $object)))) {
					$rm->invokeArgs($object, $parameters);
					return true;
				}
		return false;	
	}
}