<?php
class TFileSystem {
	public static function ForceDir($dir){
		if (!file_exists($dir)){
			mkdir($dir,0777,true);	
		}
	}
	
	public static function DeleteDir($dir){
		if (is_dir($dir)){
			$entries = scandir($dir);
			foreach ($entries as $entry)
                            if ($entry != '.' && $entry != '..'){ 
				if (is_dir($dir.'/'.$entry))
					self::DeleteDir($dir.'/'.$entry);
				else unlink($dir.'/'.$entry);
                            }
			rmdir($dir);		
		}
	}
	
	public function ListFiles($dir){
		$temp = array_filter(scandir($dir),create_function('$v', 'return $v != "." && $v != "..";'));
		array_walk($temp,create_function('&$v, $k, $d','$v = pathinfo($d."/".$v);'),$dir);
		return $temp;
	}
	
	public static function IsAbsolute($path){
		return preg_match('/^([\\\\\\/]|[a-zA-Z]\:|https?\:).*$/', trim($path));
	} 
}