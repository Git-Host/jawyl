<?php
final class TConsoleCall extends TRequest {
	
	protected $origin;
	
	private $_arguments_by_name_ = array();
	private $_arguments_by_index_ = array();
	
	public function __get($nm){
		if (isset($this->_arguments_by_name_[$nm]))
			return $this->_arguments_by_name_[$nm];
		return parent::__get($nm);
	}
	
	public function __set($nm,$value){
		$this->_arguments_by_name_[$nm] = $value;
	}
	
	public function __isset($nm){
		return isset($this->_arguments_by_name_[$nm]);
	}
	
	public function __unset($nm){
		unset($this->_arguments_by_name_[$nm]);
	}
	
	public function Parameters(){
		return $this->_arguments_by_name_;
	}	
	
	public function Arguments(){
		return $this->_arguments_by_index_; 
	}
	
	public function RequestURI(){
		return $this->origin;
	}	
	
	public function __construct($service = null){
		global $argc,$argv;
		for ($i = 1; $i < $argc; $i++){
			$temp = explode($argv[$i],'=');
			if (count($temp) > 1){
				if (is_null($service))
					if ($temp[0] == 'service')
						$service = $temp[1];
				$this->_arguments_by_name_[$temp[0]] = $temp[1];
				$this->_arguments_by_index_[] = $temp[1];
			} else 
				$this->_arguments_by_index_[] = $argv[$i]; 
		}
		
		$this->origin = base64_encode(serialize($argv));
		parent::__construct($service);
	}
}