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
}


class TLink {
/**
 * @var string
 */	
	public $Text;
/**
 * @var string
 */	
	public $Link;
/**
 * @param string $text
 * @param string $link
 */	
	public function __construct($text,$link){
		$this->Text = $text;
		$this->Link = $link;
	}
}

class TMenuLink extends TLink {
/**
 * @var TMenuLink[]
 */	
	public $Children = array();
	public function AddChild(TMenuLink $link){
		$this->Children[] = $link;
	}		
}

interface ITemplatePage {
/**
 * @return string
 */	
	public function Title();
/**
 * @return string
 */	
	public function Description();
/**
 * @return string
 */	
	public function KeyWords();
/**
 * @return string
 */	
	public function ContentsPage();
/**
 * @return integer
 */	
	public function ContentsPagesCount();
/**
 * @return string[]
 */	
	public function ContentsPages();
/**
 * @param integer $index
 * @return string
 */	
	public function ContentsPageLink($index);
/**
 * @return string
 */	
	public function NextContentsPageLink();
/**
 * @return string
 */	
	public function PreviosContentsPageLink();
/**
 * @return TLink[]
 */	
	public function NextPageLink();
/**
 * @return TLink[]
 */	
	public function PreviousPageLink();
/**
 * @return TLink[]
 */	
	public function Breadcrumbs();
/**
 * @return TLink[]
 */	
	public function Children();
/**
 * @return TMenuLink[]
 */	
	public function Menu();
/**
 * @return string
 */	
	public function Contents();
/**
 * @return TWidget[]
 */	
	public function Widgets();
/**
 * @param string $name
 * @return TWidget
 */	
	public function Widget($name);
/**
 * @return string
 */	
	public function Meta();
}

/**
 * @package View
 * @subpackage Theming
 * @category System
 * abstract class to implement configurable theme engines
 * @see TConfigurable 
 */
abstract class TThemeEngine extends TConfigurable implements IThemeEngine {
/**
 * constructor.
 * @param string $name engine name
 */
	public function __construct($name){
		parent::__construct($name);
	}
}
?>