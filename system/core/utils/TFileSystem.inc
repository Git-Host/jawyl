<?php
class TFileSystem {
	public static function ForceDir($dir){
		if (!file_exists($dir)){
			mkdir($dir,0777,true);	
		}
	}
	
	public static function IsAbsolute($path){
		return preg_match('/^([\\\\\\/]|[a-zA-Z]\:).*$/', trim($path));
	} 
}