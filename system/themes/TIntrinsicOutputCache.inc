<?php
require_once 'common/theme/engine.inc';

class TIntrinsicOutputCache extends TConfigurable implements IOutputCache {
	private $_cache_dir_;
	
	
/**
 * deletes cached template files on garbage collection, used as session gc handler 
 * @param int $lifetime
*/	
	protected function schedule_CleanCacheFiles(){
		$dh = opendir($this->CacheDir);
		while ($fn = readdir($dh)){
			$pi = pathinfo($fn);
			if (($fn != ".") && ($fn != "..") && ($pi["extension"] == "cache")){
				$txt = file_get_contents($dh."/".$fn);
				$test = preg_match('/\/\*\expires:(\d*);*\//', $txt);
				if ($test[1] < time())
					@unlink($dh."/".$fn);
			}
		}
		return true;
	}		
	
/** 
 * returns script cached output
 * @param string $name
 * @param int $lifetime
 * @return boolean
 */	
	public function CachedOutput($name,$lifetime = null){
		$fn = $this->CacheDir."/".$name."_".$this->Application()->RequestURI().".cache"; 
		if (file_exists($fn)){
			if (is_null($lifetime)){
				$cache = file_get_contents($fn);
				$expire = preg_match('/\/\*\expires:(\d*);*\//', $cache);
			} else $expire = filemtime($fn)+ $lifetime*60;
			
			if ($expire > time()){
				if ($cache) 
					echo $cache;
				else 
					include $fn;
				return true;
			}
		}
		return false;		
	}
/**
 * caches script output
 * @param string $name
 * @param string $output
 */	
	public function SaveCache($name,$output,$lifetime){
		if (!file_exists($this->CacheDir)) mkdir($this->CacheDir);
		file_put_contents($this->CacheDir."/".$name."_".$this->Application()->RequestURI().".cache",'<?php /*expires:'.(time()+$lifetime*60).';*/ ?>'.$output);
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "CacheDir":$this->_cache_dir_ = $value;break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case "CacheDir":return $this->_cache_dir_?$this->_cache_dir_:$this->Application()->AppPath().'/cache';break;
			default:return parent::__get($nm);break;
		}
	}
}