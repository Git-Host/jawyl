<?php
/**
 * @package Components
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

/**
 * @package Components
 * @category System
 * installation error class
 */
final class TInstallationException extends TException {
	const ERR_NOT_INSTALLED = 301;
/**
 * @param int $msgcode
 */	
	protected function getMessageText($msgcode){
		switch ($msgcode){
			case self::ERR_NOT_INSTALLED:return 'Component not installed!';break;
		}
	}
}

/**
 * @package Components
 * @category System
 * interface for implementing installable components
 */
interface IInstallable {
/**
 * performs component installation
 */	
	public function Install();
/**
 * performs component uninstallation
 */	
	public function UnInstall();
/**
 * checks if component properly installed
 * @return boolean
 */	
	public function IsInstalled();
}

?>