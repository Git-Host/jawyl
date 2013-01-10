<?php
/**
 * @author dakrasilnikov
 * @property string $Service
 */

abstract class TRequest {
/**
 * @var string
 */	
	protected $service;
	
/**
 * @var IFileStorage
 */	
	protected $fileStorage;
	
	public function __construct($service = null, IFileStorage $file_storage = null){
		$this->service = $service;
		$this->fileStorage = $file_storage;
	}
	
	public function __get($nm){
		switch ($nm){
			case 'Service':return $this->service;break;
		}
		return null;
	}

/**
 * @ignore
 */		
	public function __isset($nm){
		if ($nm == "Service") 
			return !is_null($this->service);
		return false;
	}
	
/**
 * @return string
 */	
	public function GetService(){
		return $this->service;
	}
	
	public abstract function Parameters();

	public abstract function RequestURI();	
} 