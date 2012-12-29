<?php
class TLogger extends TConfigurable {
	private $_filename_;
	private $_format_;
	
	public function __set($nm,$value){
		switch ($nm){
			case 'LogFile':$this->_filename_ = TApplication::$Application->AdjustPath($value);break;
			case 'Format':$this->_format_ = $value;break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function Put(){
		$fn = $this->_filename_;
		if (!$fn)
			$fn = TApplication::$Application->PrivateDir.'/log/'.$this->Name().'.log';
	
		TFileSystem::ForceDir($fn);
		
		if ($this->_format_)
			$str = call_user_func_array('sprintf', array_merge(array($this->_format_),func_get_args()));
		else 
			$str = join("\t",func_get_args());
		
		$f = fopen($fn,'a');
		fwrite($f,$str."\n");
		fclose($f);	
	}
	
}