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
TApplication::import("widgets::areas::TAreasNestings");


class TAreasTableColumn {
	public $Caption;
	public $Property;
	public $Editable = false;
	public $Type;
	public function __construct(array $rec){
		$cnt = count($rec);
		if ($cnt > 0)
			$this->Caption = _i($rec[0]);
		if ($cnt > 1)
			$this->Property = $rec[1];
		if ($cnt > 2){
			$this->Type = $rec[2];
			$this->Editable = true;	
		}
		if ($cnt > 3)
			$this->Editable = TConvertions::ConvertToBoolean($rec[3]);
	}
} 

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * nested table widget
 */
class TAreasNestedTable extends TAreasNestings {
/**
 * @var string
 */
	private $_columns_ = null;
/**
 * @var string
 */	
	public $ShowItem;
/**
 * @var boolean
 */	
	protected $allowOpen = true;
	
	private $_wysiwyg_settings_ = array();
	
/**
 * selects item
 * @param string $id item id 
 */	
	public function http_SelectItem($id){
		$selected = $this->SelectedItem();
		if (!is_null($selected)){
			if ($selected->ItemId() != $id)
				$this->Source->SelectItem($this->extractWrappedId($id));
		}
		else if (!is_null($id))
			return $this->Source->SelectItem($this->extractWrappedId($id));
	}

	public function __set($nm,$value){
		switch ($nm){
			case "WYSIWYGSettings":$this->_wysiwyg_settings_ = TConvertions::ConvertToArray($value,array(";",":"));break;
			case "AllowOpen": $this->allowOpen = TConvertions::ConvertToBoolean($value);break;
			case "Columns":{
				$this->_columns_ = $this->parseColumns(TConvertions::ConvertToArray($value,";"));
			}break;
			default:parent::__set($nm,$value);break;
		}
	}
	
	public function __get($nm){
		switch ($nm){
			case "WYSIWYGSettings":return $this->_wysiwyg_settings_;break;
			case "AllowOpen":return $this->allowOpen;break;
			case "Columns":return $this->_columns_;break;
		}
		return parent::__get($nm);
	}
	
	protected function parseColumns(array $columns){
		$result = array();
		foreach ($columns as $col)
			$result[] = new TAreasTableColumn(TConvertions::ConvertToArray($col,":"));
		return $result;
	}
		
	
	protected function onEditEvent(TItemEditEvent $event){
		$ei = $this->WrapItem($this->Source->Item($event->NodeId));
		$this->refreshArea($this->GetArea($ei,"Item",$this->GetArea($this,"Container",null)));
	}

/**
 * sets item attribute value
 * @param string $id item id
 * @param string $attrname name of the attribute
 * @param string $value value to assign
 */	
	protected function http_EditItem($attrname,$value){
		if (!($this->Source instanceof IItemsSource))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		$this->Source->EditCurrentItem($attrname,$value);
	}
	
	protected function renderContainer(TWidgetArea $area){
		$context = $area->Context();
		$items = $this->Items();
		if (!$this->Columns) return;
		?>
		<table id="<?php echo $area->Id(); ?>">
		<tr>
		<?php
		if (TConvertions::ConvertToBoolean($this->ShowItem)){ ?><th></th><?php }
		foreach ($this->Columns as $col){
		?><th id="<?php echo $this->Name(); ?>_<?php echo $col->Property; ?>"><?php
		__i($col->Caption);
		?></th><?php
		}
 		?>
		</tr>
		<?php
		if ($this->isSublevel()){
		?>
		<tr><td><div class="level-up" ondblclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"LevelUp",array()); ?>&quot;);">...</div></td></tr>
		<?php	
		}
		foreach ($items as $item)
			$this->RenderArea($this->GetArea($item,"Item",$area));
		?>
		</table>
		<?php
		$area->SkipWrapper = true;			
	}
	
	protected function renderItem(TWidgetArea $area){
		$context = $area->Context();
?>
<tr id="<?php echo $area->Id(); ?>" class="nested-table-row<?php echo $context->IsSelected()?" selected":""; ?>" onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $context->SelectLink(); ?>&quot;);" <?php if ($this->AllowOpen){ ?>ondblclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"OpenItem",array("id"=>$context->ItemId())); ?>&quot;);"<?php } ?>>
<?php
		if (TConvertions::ConvertToBoolean($this->ShowItem)){ ?><td><?php echo $context; ?></td><?php }

		foreach ($this->Columns as $col){
			?><td><?php
			$p = $context->Property($col->Property);
			if ($p){
				if (($col->Editable) && (!$p->ReadOnly()) && ($context->IsSelected()))
					$this->RenderArea($this->GetArea($p,"Value"));
				else
					if ($p->Type() == TItemPropertyType::PT_PASSWORD)
						echo "***";
					else
						echo $p->Value();
			}
			?></td><?php
		}
 ?>
</tr>
<?php 
		$area->SkipWrapper = true;			
	} 
	
	protected function http_Editor($name){
		$item = $this->Source->CurrentItem();
		if ($item)
			$this->refreshArea($this->GetArea($item->Property($name),"Editor"));
	}
	
	protected function renderValue(TWidgetArea $area){
		$context = $area->Context();
		?><span onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"Editor",array("name"=>$context->Name())); ?>&quot;);return false;"><?php
		if ($context->Type() == TItemPropertyType::PT_PASSWORD)
		echo "***";
		else
		echo $area->Context()->Value();
		?></span><?php
	}
	
	protected function javaScript(){
		?>	
		jQuery(document).one("ready",function(e){
			var wo = <?php echo json_encode($this->WYSIWYGSettings); ?>;wo.script_url = WYSIWYG_URL;
			jQuery("#<?php echo $this->Name(); ?>").data("wyswyg",wo);
		});
		<?php
	}	
	
	protected function renderEditor(TWidgetArea $area){
		$context = $area->Context();
		?>
		<form id="<?php echo $area->Id(); ?>_frm" method="post">
			<input type="hidden" name="controller" value="<?php echo $this->Name(); ?>" />
			<input type="hidden" name="method" value="EditItem" />
			<input type="hidden" name="attrname" value="<?php echo $context->Name(); ?>" />
		<?php
			TJQueryFormAspect::RenderField($area->Id()."_value",$area->Context(),"value",true,$this);
		?>
		</form>
		<script language="javascript">
		$("#<?php echo $area->Id(); ?>").areas_editor({poston:["change","blur"],wyswyg:jQuery("#<?php echo $this->Name(); ?>").data("wyswyg")});
		$("#<?php echo $area->Id(); ?>").areas_ajax_forms();
		</script>
		<?php		
	}

	public function AreaId(TWidgetArea $area){
		$context = $area->Context();
		if ($context instanceof IProperty)
			return $this->Name()."_".$context->Item()->ItemId()."_".$context->Name();
		return parent::AreaId($area);
	}
}
?>