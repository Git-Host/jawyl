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
TApplication::impoert("core::utils::TScheduleHandler");


class TCronService extends TService {
	public function Handle(THttpRequest $request){
		$result = false;
		
		if (!$this->ACL()->Login($request->user, $request->pwd)){
			throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
			return result;
		}
		
		$joblist = TApplication::$Application->AppPath."/".TApplication::$Application->PrivateDir."/jobs.xml";
		
		$dom = new DOMDocument();
		$dom->preserveWhiteSpace = false;
		$dom->Load($joblist);
		
		if (TConvertions::ConvertToBoolean($this->ValidateConfig))
			if (!$dom->schemaValidate(TApplication::$SystemPath."/schemas/joblist.xsd"))
				throw new TCoreException(TCoreException::ERR_CONF_INCORRECT); 
		
		$xpath = new DOMXPath($dom);
		
		$jobs = $xpath->query("/jobs/list[@name='".$request->list."']/job");
		
		foreach ($jobs as $job){
			$item = TApplication::Instance($job->getAttribute("component"));
			if ($item instanceof ISecurityObject){
				$parameters = array();
				$params = $xpath->query("parameter",$job);
				foreach ($params as $p)
					$parameters[$p->getAttribute("name")] = $p->nodeValue;
				$handled = TScheduleHandler::ExecuteJob($item, $this->ACL(), $this->Policy(), $job->getAttribute("name"), $parameters);
				if (!$result) $result = $handled; 
			}
		}
		return result;
	}
}