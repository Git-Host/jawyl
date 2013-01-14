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
	class TOutputBuffer implements IOutputBuffer {
/**
 * starts buffering.
 */		
		public function StartRecording(){
			ob_start();
		}
/**
 * stops buffering and returns buffer contents.
 * @return string
 */		
		public function StopRecording(){
			$result = ob_get_contents();
			ob_end_clean();   
			return $result;
		}
		
		public function Clean(){
			while (ob_get_level() > 0)
				ob_end_clean();
		}
		
		public function Flush(){
			while (ob_get_level() > 0)
				ob_end_flush();
		}
	}
?>