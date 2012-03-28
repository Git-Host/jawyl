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
TApplication::import("common::presenter::basic");
/**
 * @ignore
 */
TApplication::import("widgets::areas::TAreasWidget");

TApplication::import("widgets::aspects::TJQueryFormAspect");

/**
 * @package View
 * @subpackage Widgets
 * @category System
 * action setup form display
 */
class TAreasActionProcessor extends TAreasWidget {
	public $BlockActions;
	
	private $_popup_ = false;
	private $_ajax_submit_ = false;
	private $_editors_ = array();

	public $SubmitText = "Ok";
	public $CancelText = "Cancel";
	public $Legend = "";
	
	private static $_actionprocessor_script_loaded_ = false;
	
/**
 * @ignore
 */	
	public function __get($nm){
		switch ($nm){
			case "actions":return $this->Actions();break;
			case "PopUp":return $this->_popup_;break;
			case "AjaxSubmit":return $this->_ajax_submit_;break;
		}
		return parent::__get($nm);
	}
	
	public function __set($nm,$value){
		switch ($nm){
			case "PopUp":$this->_popup_ = TConvertions::ConvertToBoolean($value);break;
			case "AjaxSubmit":$this->_ajax_submit_ = TConvertions::ConvertToBoolean($value);break;
			default:parent::__set($nm, $value);break;
		}
	}
	
	public function AddWidget(TWidget $widget){
		parent::AddWidget($widget);
		if ($widget instanceof IEditor)
			if ($widget->FieldName())
				$this->_editors_[$widget->FieldName()] = $widget;
	}
	
	protected function checkSource($src){
		return (($src instanceof IActionsList) || ($src instanceof IAction)) && parent::checkSource($src);
	}	
	
	protected function onActionStateEvent(TActionStateEvent $event){
		$this->refreshArea($this->GetArea($event->Action,"Form"));
	}
	
/**
 * @see TJQueryWidget::linkCustomResources()
 */
	protected function linkCustomResources(){
		parent::linkCustomResources();
		self::LoadForms();
		self::LoadFileUpload();
	}
		
	protected function areaCssClass(TWidgetArea $area){
		if ($area->Context() instanceof TAreasActionProcessor)
			if ($area->Skin() == "Form") 
				return "action-form";
		return null;
	}
	
	protected function areaStyle(TWidgetArea $area){
		if ($area->Context() instanceof TAreasActionProcessor)
			if ($area->Skin() == "Form") 
				return "display:none;";
		return null;
	}

	public function AreaId(TWidgetArea $area){
			return $this->Name();
	}
	
	protected function renderForm(TWidgetArea $area){
		$context = $area->Context();
		if ($context instanceof IAction){
		$params = $context->ParametersArray();
		
		$groups = array();
		foreach ($params as $p){
			if (!key_exists($p->Group(),$groups))
				$groups[$p->Group()] = array();
			$groups[$p->Group()][] = $p;	
		}

?>
	<div id="<?php echo $this->Name(); ?>_dlg">
	<form method="post">
		<input type="hidden" name="controller" value="<?php echo $context->Name(); ?>" />
		<input type="hidden" name="method" value="Execute" />
		<?php foreach ($groups as $g=>$params){ ?>
		<div class="param-group">
		<div class="param-group-title"><?php __i($g); ?></div>
		<table>
			<col width="100" />
				<?php foreach ($params as $p) { ?>
			<tr>
				<td><?php __i($p->Title()); ?><?php echo ($p->Optional()?"":"*"); ?></td>
				<td><?php
				if (key_exists($p->Name(), $this->_editors_)){
					$this->_editors_[$p->Name()]->SetupValue($p->Value());
					$this->_editors_[$p->Name()]->Output($this->currentEngine);
				}
				else 
				TJQueryFormAspect::RenderField($this->Name()."_".$p->Name(),$p,$p->Name(),false,null,5); 
				?></td>
			</tr>
			<?php } ?>
		</table>
		</div>
		<?php }
		if ($this->Legend){ 
		?>
		<div class="form-legend"><?php __i($this->Legend); ?></div>
		<?php } ?>
		<div class="dlg-btn-holder">
			<button onclick="javascript:$(this.form).submit();<?php if ($this->_popup_){ ?>$(&quot;#<?php echo $this->Name(); ?>_dlg&quot;).dialog('close');<?php } ?>return false;"><?php __i($this->SubmitText); ?></button>
			<?php if ($this->_popup_){ ?>
			<button onclick="javascript:$(&quot;#<?php echo $this->Name(); ?>_dlg&quot;).dialog('close');return false;"><?php __i($this->CancelText); ?></button>
			<?php } ?>
		</div>
	</form>
	</div>
	<script type="text/javascript" language="javascript">
	<?php 
	if (!$this->_popup_){
	?>
	$(document).one("ready",function(){
	<?php 
	}
	if ($this->_ajax_submit_){ ?>
		$("#<?php echo $this->Name(); ?>_dlg").areas_ajax_forms();
	<?php } ?>
		$("#<?php echo $this->Name(); ?>_dlg form").areas_editor({});
	<?php 
	if (!$this->_popup_){
		?>
	});
		<?php 
	}
	 if ($this->_popup_){ ?>
	$("#<?php echo $this->Name(); ?>_dlg").dialog({modal:true,height:300,width:400,show:'slide'});
	<?php } ?>	
	</script>
<?php	
		}		
	}
	
/**
 * @see TWidget::DefaultOutput()
 */	
	protected function defaultOutput(IThemeEngine $engine){
		if ($this->_popup_)
			$this->RenderArea($this->GetArea($this,"Form"));
		else		
			$this->RenderArea($this->GetArea($this->Source,"Form"));
	}
}
?>