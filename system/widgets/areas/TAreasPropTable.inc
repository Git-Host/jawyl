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

/**
 * @ignore
 */
TApplication::import("widgets::areas::TAreasWidget");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * properties table widget, needs source to be IItemsSource
 * @see IItemsSource
 */
class TAreasPropTable extends TAreasWidget {
/**
 * @var string
 */	
	private $_hide_ = array();
	private $_show_ = array();
	private $_include_t_ = array();
	private $_exclude_t_ = array();

	public function __set($nm,$value){
		switch ($nm){
			case "ShowProperty":$this->_show_[] = $value;break;
			case "ShowProperties":$this->_show_ = TConvertions::ConvertToArray($value,";");break;
			case "HideProperty":$this->_hide_[] = $value;break;
			case "HideProperties":$this->_hide_ = TConvertions::ConvertToArray($value,";");break;
			case "ExcludeType":$this->_exclude_t_[] = $value;break;
			case "ExcludeTypes":$this->_exclude_t_ = TConvertions::ConvertToArray($value,";");break;
			case "IncludeType":$this->_include_t_[] = $value;break;
			case "IncludeTypes":$this->_include_t_ = TConvertions::ConvertToArray($value,";");break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case "ShowProperties":return $this->_show_;break;
			case "HideProperties":return $this->_hide_;break;
			case "ExcludeTypes":return $this->_exclude_t_;break;
			case "IncludeTypes":return $this->_include_t_;break;
		}
		return parent::__get($nm);
	}
	

/**
 * edits a property
 * @param string $name property name
 * @param string $value value to set
 */	
	protected function http_Edit($name,$value){
		if (!($this->Source instanceof IItemsSource))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		if ($value == "") 
			$value = null;
		else
			$value = html_entity_decode($value,ENT_QUOTES,"UTF-8");
		$this->Source->EditCurrentItem($name,$value);
	}

/**
 * gets list of properties of source current item
 * @return array
 */
	protected function getProperties($includedtypes = array(),$excludedtypes = array()){
		if (!($this->Source instanceof IItemsList))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		$item = $this->Source->CurrentItem();
		$result = array();
		if (!is_null($item)){
			 $temp = $item->Properties($this->IncludeTypes,$this->ExcludeTypes);
			 foreach ($temp as $p)
			 	if (!in_array($p->Name(),$this->HideProperties) && (empty($this->ShowProperties) || in_array($p->Name(),$this->ShowProperties)))
			 		$result[$p->Name()] = $p;
		}
		return $result;
	}
	
/**
 * gets properties of source current item, except of PT_HTML and PT_MULTILINE types
 */	
	public function Properties(){
		return $this->getProperties(array(),$this->_type_filter_);
	}
	
	protected function onSelectEvent(TItemSelectEvent $event){
		$this->refreshArea($this->GetArea($this,"Contents"));
	}
	
	protected function onEditEvent(TItemEditEvent $event){
		$curitem = $this->Source->CurrentItem();
		if ($curitem)				
			$prop = $curitem->Property($event->Attribute);
		if ($prop)
			$this->refreshArea($this->GetArea($prop,"Value"));
	}
	
/**
 * @see TJQueryWidget::linkCustomResources()
 */
	protected function linkCustomResources(){
		parent::linkCustomResources();
		self::LoadForms();
		self::LoadFileUpload();
	}
	
	protected function renderContents(TWidgetArea $area){
		$context = $area->Context();
?>
<table>
<tr><th><?php echo _i("Property"); ?></th><th><?php echo _i("Value"); ?></th></tr>
<?php

$props = $this->Properties();
foreach ($props as $p){ ?>
<tr>
<td valign="top" class="caption">
<?php echo htmlentities(_i($p->Caption()),ENT_QUOTES,"UTF-8"); ?>
</td>
<td valign="top">
<?php
$this->RenderArea($this->GetArea($p,"Property")); 
?>
</td>
</tr>
<?php } ?>
</table>
<?php		
	}
	
	protected function renderValue(TWidgetArea $area){
		$context = $area->Context();
		TJQueryFormAspect::RenderField($this->Name()."_".$context->Name()."_value",$context,"value",true,$this);
		?>
		<script language="javascript">$("#<?php echo $area->Id(); ?>").areas_editor({});</script>
		<?php
	}
	
	protected function renderProperty(TWidgetArea $area){
		$context = $area->Context();
		?>
<form id="<?php echo $this->Name(); ?>_<?php echo $context->Name(); ?>_frm" class="<?php echo $this->Name(); ?>_frm"  method="post">
<input type="hidden" name="controller" value="<?php echo $this->Name(); ?>" />
<input type="hidden" name="method" value="Edit" />
<input type="hidden" name="name" value="<?php echo $context->Name(); ?>" />
<?php
	$this->RenderArea($this->GetArea($context,"Value"));
?>
</form>
<script language="javascript">jQuery("#<?php echo $area->Id(); ?>").areas_ajax_forms();</script>
<?php		
	}
	
/**
 * @see TWidget::DefaultOutput()
 */	
	protected function defaultOutput(IThemeEngine $engine){
		?>
		<div id="<?php echo $this->Name(); ?>" class="propertiestable">
		<?php $this->RenderArea($this->GetArea($this,"Contents")); ?>
		</div>
		<?php
	}
}
?>