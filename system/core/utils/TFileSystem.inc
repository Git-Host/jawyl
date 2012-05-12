<?php
class TFileSystem {
	public static function ForceDir($dir){
		if (!file_exists($dir)){
			self::ForceDir(dirname($dir));
			mkdir($dir);	
		}
	}
	
	public static function IsAbsolute($path){
		return preg_match('/^([\\\\\\/]|[a-zA-Z]\:).*$/', trim($path));
	} 
}