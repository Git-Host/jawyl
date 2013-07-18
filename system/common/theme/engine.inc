<?php
/**
 * @package View
 * @subpackage Theming
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

interface IPreProcessor {
/**
 * parse template file and convert it to valid output
 * @param string $text
 * @return string
 */
	public function PreProcess($text);	
}

interface IOutputCache {
/** 
 * returns script cached output
 * @param string $name
 * @param int $lifetime
 * @return boolean
 */	
	public function CachedOutput($name,$lifetime = null);
/**
 * caches script output
 * @param string $name
 * @param string $output
 */	
	public function SaveCache($name,$output,$lifetime);
}

/**
 * @package View
 * @subpackage Theming
 * @category System
 * interface for implementing theme engines
 */
interface IThemeEngine {
/**
 * send output associated with response to client
 * @param TResponse $response 
 * @param string $template name name of template to use for reponse output
 */	
	public function RenderResponse(TResponse $response,$templatename);	
/**
 * gets local path to template files directory
 * @return string
 */	
	public function TemplateDir();

	public function ThemeDir();
	
	public function ThemeUrl();
/**
 * gets template file extension
 * @return string
 */	
	public function TemplateExtension();
/**
 * parse template file and convert it to valid output
 * @param mixed $context object to bind template to
 * @param string $filename template file name
 * @param ITemplateProcessor $preprocessor optional processor to preprocess a template 
 */
	public function ProcessTemplate($context, $filename, $cachelifetime = 0, IPreProcessor $preprocessor = null);

	public function OutputCache();
	
/**
 * @param string $themename
 * @return bool
 */	
	public function ThemeExists($themename);
}

/**
 * @package View
 * @subpackage Theming
 * @category System
 * abstract class to implement configurable theme engines
 * @see TConfigurable 
 */
abstract class TThemeEngine extends TConfigurable implements IThemeEngine {

}
?>