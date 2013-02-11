<?php
/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package CoreRuntimeClasses
 * @subpackage Utilities
 * @category Core
 * Static class for performing type convertions. There is only one method available yet. 
 */
	class TConvertions {
/**
 * convert any value to boolean type. 
 * For strings values "on","true","yes" are evaluated to true and values "off","false","no" are evalueated to false.
 * For numbers positive values are evaluated to true, zero and negative are evaluated to false.
 * @param mixed $v any value
 * @return boolean
 */		
	  	public static function ConvertToBoolean($v){
  		    if (is_null($v))
  		    	return false;
			if (is_numeric($v))	
				return $v > 0;
			else if (is_bool($v))
				return $v;
			else if (is_string($v)){
			    switch (strtolower($v)){
			    	case "on":return true;break;
			    	case "true":return true;break;
			       	case "yes":return true;break;
			    	case "off":return false;break;
			    	case "false":return false;break;
			    	case "no":return false;break;
			    	default:return ord($v) > 0;break;
			    }
			}
			else throw new TCoreException(TCoreException::ERR_BAD_VALUE);				
	  	}
	  	
	  	public static function CheckInteger($v){
			return (integer)$v == $v;
	  	}
	  	
	  	public static function CheckSequentialArray(array &$a){
	  		for (reset($a), $base = 0;key($a) === $base++;next($a));
	  		$result = is_null(key($a));
	  		reset($a);
	  		return $result;
	  	}

	  	private static function _str_array_parse($v,$separator, array $separators, array $keyval){
	  		if (isset($separators[$separator]))
	  			$temp = explode($separators[$separator],$v);
	  		else 
	  			return $v;		
	  			
	  		array_walk($temp, create_function('&$item', '$item = trim($item);'));
	  		$temp = array_filter($temp,create_function('$item','return $item !== "";'));
	  		$result = array();
	  		if ($separator < count($separators) - 1){
	  			foreach ($temp as $t){
	  				if (isset($keyval[$separator + 1]) && $keyval[$separator + 1]){
	  					$t1 = explode($separators[$separator + 1],$t);
	  					if (is_array($t1) && (count($t1) > 1))
	  						$result[$t1[0]] = self::_str_array_parse($t1[1], $separator + 2, $separators, $keyval);
	  					else
	  						$result[] = self::_str_array_parse($t1[0], $separator + 2, $separators, $keyval);
	  				} else 
	  					$result[] = self::_str_array_parse($t, $separator + 1, $separators, $keyval);
	  			}
	  		}
	  		else $result = $temp;	
	  		return $result; 
	  	}
	  	
	  	public static function ConvertToArray($v, $separator = ";",$iskeyvalue = false){
	  		if (is_array($v)) return $v;
	  		if (is_string($v)){
	  			if (!is_array($separator)) $separator = array($separator);
	  			if (!is_array($iskeyvalue)) $iskeyvalue = array($iskeyvalue);
	  			if (count($separator) != count($iskeyvalue))
	  				throw new TCoreException(TCoreException::ERR_BAD_VALUE);
	  				
	  			return self::_str_array_parse($v,0,$separator,$iskeyvalue);
	  		}
	  		if (is_object($v)) return array_values((array)$v);
	  		return (array)$v;
	  	} 
	}
?>