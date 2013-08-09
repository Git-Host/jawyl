<?php
/**
 * @package View
 * @subpackage Widgets
 * @category System
 * @author Dan Krasilnikov <dkrasilnikov@gmail.com>
 * @copyright Copyright (c) 2009, Dan Krasilnikov
 * @license http://opensource.org/licenses/gpl-license.php GNU Public License
 * @version 0.0.1 alpha  
*/

require_once 'widgets/TSkinnableWidget.inc';
require_once 'common/widget/lists.inc';

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * widget using jquery framework
 */
abstract class TJQueryWidget extends TSkinnableWidget implements IFilePropertyPublisher {
/**
 * @var boolean
 */	
	public static $JQueryLoaded = false;
/**
 * @var boolean
 */	
	public static $JQueryAccordionLoaded = false;
/**
 * @var boolean
 */	
	public static $JQueryFileUploadLoaded = false;
/**
 * @var boolean
 */	
	public static $JQueryFormLoaded = false;
/**
 * @var boolean
 */	
	public static $JQueryTabControlLoaded = false;	
/**
 * @var boolean
 */	
	public static $JQueryWindowLoaded = false;
	
	public static $JQueryDragDropLoaded = false;
	
	protected $_conf_j_query_path_ = null;
	
	private $_theme_jquery_exists_ = false;

	public function JQueryPath(){
		if ($this->JQueryPath)
			return $this->JQueryPath;
		if ($this->currentEngine)
			if ($this->_theme_jquery_exists_ || file_exists($this->currentEngine->ThemeDir()."/js/jquery/")){
				$this->_theme_jquery_exists_ = true;
				return $this->currentEngine->ThemeUrl()."/js/jquery/"; 
			}
		return "/js/jquery/";
	}
	
	public function __set($nm, $value){
		switch ($nm){
			case 'JQueryPath':$this->_conf_j_query_path_ = $this->Application()->AdjustPath($value);break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	
/**
 * loads jquery accordion plugin
 */	
	protected function loadAccordion(){
		if (!self::$JQueryAccordionLoaded){
?>
<script type='text/javascript' src='<?php echo $this->Application()->AssetUrl("jquery/jquery.jawyl.accordion.js"); ?>'></script>
<?php			
			self::$JQueryAccordionLoaded = true;
		}
	}

/**
 * loads jquery fileupload plugin
 */	
	protected function loadFileUpload(){
		if (!self::$JQueryFileUploadLoaded){
?>
<script type='text/javascript' src='<?php echo $this->Application()->AssetUrl("jquery/jquery.jawyl.fileupload.js"); ?>'></script>
<?php			
			self::$JQueryFileUploadLoaded = true;
		}
	}
	
/**
 * loads jquery forms plugin
 */	
	protected function loadForms(){
		if (!self::$JQueryFormLoaded){
?>
<script type='text/javascript' src='<?php echo $this->Application()->AssetUrl("jquery/jquery.form.js"); ?>'></script>
<?php			
			self::$JQueryFormLoaded = true;
		}
	}
	
/**
 * loads jquery tabcontrol plugin
 */	
	protected function loadTabControl(){
		if (!self::$JQueryTabControlLoaded){
?>
<script type='text/javascript' src='<?php echo $this->Application()->AssetUrl("jquery/jquery.jawyl.tabcontrol.js"); ?>'></script>
<?php			
			self::$JQueryTabControlLoaded = true;
		}
	}

/**
 * loads jquery draggable plugin
 */	
	protected function loadDraggable(){
		if (!self::$JQueryDragDropLoaded){
?>
<script type='text/javascript' src='<?php echo $this->Application()->AssetUrl("jquery/jquery.jawyl.draggable.js"); ?>'></script>
<?php			
			self::$JQueryDragDropLoaded = true;
		}
	}	
		
/**
 * loads custom widget resources
 */	
	protected function linkCustomResources(){}
		
/**
 * 
 */	
	protected function resourceLoad(){
		if (!self::$JQueryLoaded){
		?>
<script type="text/javascript" src="<?php echo $this->JQueryPath(); ?>jquery-min.js"></script>
<script type="text/javascript" src="<?php echo $this->JQueryPath(); ?>jquery.cookie.js"></script>
<script type="text/javascript" src="<?php echo $this->JQueryPath(); ?>jquery-ui-min.js"></script>
		<?php  
			self::$JQueryLoaded = true;
		}
		$this->linkCustomResources();
		parent::resourceLoad();
	}

/**
 * gets value of source current item property of PT_FILE or PT_IMAGE type 
 */	
	public function http_File($id,$name){
		if (!($this->Source instanceof IItemAccessor))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		$item = $this->Source->Item($id);
		if ($item){
			$p = $item->Property($name);
			$f = $p->Value();
			if ($f instanceof TFile)
				TFilePropertyAspect::OutputFile($f);
		}
	}	
	
	public function FileLink($base){
		if ($base instanceof IProperty){
			$item = $base->Item();
			$f = $base->Value();
			if ($f instanceof TDBStoredFile)
				return $this->response->Url($this,"File",array("id"=>$item->ItemId(),"name"=>$base->Name()));
			return $f->GetLink();	
		} 
		if ($base instanceof TActionParameter){
			$f = $base->Value();
			if ($f instanceof TFile)
				return $f->GetLink();
			if (is_string($f))
				return $f;	
		}
		return $this->NoPicturePath;
	}
}
?>