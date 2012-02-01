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
 * static output buffering wrapper. simplifies use of buffering.
 */
	class TOutputBuffer {
/**
 * starts buffering.
 */		
		public static function StartRecording(){
			   ob_start();
		}
/**
 * stops buffering and returns buffer contents.
 * @return string
 */		
		public static function StopRecording(){
			$result = ob_get_contents();
			ob_end_clean();   
			return $result;
		}
	}
?>