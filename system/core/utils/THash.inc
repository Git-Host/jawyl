<?php

class THash {
	const MD5 = "md5";
	const SHA1 = "sha1";
	const CRC32 = "crc32";
	const WHIRLPOOL = "whirlpool";
	
	public static $DefaultAlgorythm = self::MD5;
	
/**
 * @param string $data
 * @param string $algo
 * @return string
 */	
	public static function Apply($data,$algo = null){
		if (is_null($algo)) $algo = self::$DefaultAlgorythm;
		if (!self::CheckAlgorythm($algo))
			throw new TCoreException(TCoreException::ERR_BAD_VALUE);
		return hash($algo, $data);
	}

/**
 * @param string $algo
 * @return boolean
 */	
	public static function CheckAlgorythm($algo){
		return in_array($algo,hash_algos());
	}
}