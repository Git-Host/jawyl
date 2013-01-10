<?php
	require_once 'ajax/TAreasEngine.inc';
	require_once 'widgets/TJQueryWidget.inc';
	require_once 'common/theme/engine.inc';

	class TWidgetArea implements IArea {
/**
 * @var TAreasWidget
 */		
		private $_widget_;
		private $_context_;
/**
 * @var TWidgetArea
 */		
		private $_parent_;
		private $_skin_;
		
		public $SkipWrapper = false;

		
		public function __construct(TAreasWidget $widget, $context, $skin = null, TWidgetArea $parent = null){
			$this->_widget_ = $widget;
			$this->_context_ = $context;
			$this->_skin_ = $skin;
			$this->_parent_ = $parent;
		}
/**
 * @return string
 */	
		public function Id(){
			return $this->_widget_->AreaId($this);
		}	
		
		public function Render(){
			$this->_widget_->RenderArea($this);
		}
/**
 * @return IArea
 */
		public function ParentArea(){
			return $this->_parent_;
		}
/**
 * @return mixed
 */	
		public function Context(){
			return $this->_context_;
		}
/**
 * @return string
 */	
		public function Skin(){
			return $this->_skin_;
		}
	}

	abstract class TAreasWidget extends TJQueryWidget {
		public static $JQueryAreasLoaded = false;	
		
		protected function linkCustomResources(){
			if (!self::$JQueryAreasLoaded){
			$format =  TI18n::Locale()->DateFormat();
			$format = preg_replace('/d/','dd',$format);
			$format = preg_replace('/j/','d',$format);
			$format = preg_replace('/j/','d',$format);
			$format = preg_replace('/l/','DD',$format);
			$format = preg_replace('/F/','MM',$format);
			$format = preg_replace('/m/','mm',$format);
			$format = preg_replace('/n/','m',$format);
			$format = preg_replace('/n/','m',$format);
			$format = preg_replace('/Y/','yy',$format);
			$format = preg_replace('/y/','y',$format);
		?>
<script type='text/javascript' src='<?php echo self::JQueryPath(); ?>jquery.jawyl.areas.js'></script>
<script type='text/javascript'>
jQuery.areas_date_format = "<?php echo $format; ?>";
jQuery.areas_response_type = "<?php echo $this->response->AjaxContentType; ?>";
jQuery.areas_add_tag = "<?php echo TAreasEngine::AREA_ADD; ?>";
jQuery.areas_refresh_tag = "<?php echo TAreasEngine::AREA_REFERSH; ?>";
jQuery.areas_delete_tag = "<?php echo TAreasEngine::AREA_DELETE; ?>";
jQuery.areas_move_tag = "<?php echo TAreasEngine::AREA_MOVE; ?>";
jQuery.areas_OnDatePicker = function (inputs){
	inputs.datepicker({
		changeMonth:true,
		changeYear:true,
		constrainInput:true,
		dateFormat:$.areas_date_format,
		dayNames:<?php echo json_encode(TI18n::Locale()->WeekDayNames()); ?>,
		dayNamesMin:<?php echo json_encode(TI18n::Locale()->WeekDayNames(ILocale::NT_MIN)); ?>,
		dayNamesShort:<?php echo json_encode(TI18n::Locale()->WeekDayNames(ILocale::NT_SHORT)); ?>,
		firstDay:<?php echo TI18n::Locale()->FirstWeekDay(); ?>,
		monthNames:<?php echo json_encode(TI18n::Locale()->MonthNames(ILocale::NT_SHORT)); ?>,
		monthNamesShort:<?php echo json_encode(TI18n::Locale()->MonthNames(ILocale::NT_SHORT)); ?>,
		yearRange:"1930:2050"			
	});
}
</script>
		<?php
				self::$JQueryAreasLoaded = true;
			}
		}
		
		protected function refreshArea(TWidgetArea $area){
			if ($this->response->Request()->IsAjax)
				if ($this->response->AjaxEngine instanceof TAreasEngine)
					$this->response->AjaxEngine->RefreshArea($area);
		}
		
		protected function addArea(TWidgetArea $area){
			if ($this->response->Request()->IsAjax)
				if ($this->response->AjaxEngine instanceof TAreasEngine)
					$this->response->AjaxEngine->AddArea($area);
		}
		
		protected function deleteArea($id){
			if ($this->response->Request()->IsAjax)
				if ($this->response->AjaxEngine instanceof TAreasEngine)
					$this->response->AjaxEngine->DeleteArea($id);
		}
		
		protected function moveArea(TWidgetArea $area){
			if ($this->response->Request()->IsAjax)
				if ($this->response->AjaxEngine instanceof TAreasEngine)
					$this->response->AjaxEngine->MoveArea($area);
		}
		
		public function AreaId(TWidgetArea $area){
			$pref = "";
			if ($area->Skin())
				$pref = $area->Skin()."_";
			$context = $area->Context();
			$result = null;	
			if ($context === $this)
				return $pref.$this->Name();
			else if ($context instanceof IIdentity)
				$result =  $pref.$context->ItemId();	
			else if ($context instanceof IInstance)
				$result = $pref.$context->Id();	
			else if ($context instanceof IPropertyMeta)
				$result = $pref.$context->Name();	
			else if ($context instanceof IClass)
				$result = $pref.$context->Id();	
			else if ($context instanceof IAttributeDefinition)
				$result = $pref.$context->Id();
			else if ($context instanceof IAction)
				$result = $pref.$context->Name();
			if ($result) return $this->Name()."_".preg_replace('/[\\.:#$@]/','_',$result);
			return $result;	
		}
		
		protected function areaCssClass(TWidgetArea $area){
			$context = $area->Context();
			if (is_object($context))
				return get_class($context);
			return null;
		}
		
		protected function areaStyle(TWidgetArea $area){
			return null;
		}
		
		public function GetArea($context, $skin = null, TWidgetArea $parent = null){
			return new TWidgetArea($this,$context,$skin,$parent);
		}
		
		public function RenderArea(IArea $area){
				$this->currentEngine = $this->response->ThemeEngine;
				if ($this->currentEngine instanceof TAreasEngine)
					$this->currentEngine->EnterArea($area);
				$accs = $this->areaCssClass($area);
				$style = $this->areaStyle($area);
				$context = $area->Context();
				if (is_null($context)) $context = $this;

				$this->Application()->Buffer()->StartRecording();
				$skinfile = $this->Skin($this->currentEngine,$area->Skin());
				if ($skinfile) 
					$this->currentEngine->ProcessTemplate($context,$skinfile);
				else
					if (method_exists($this,"render".$area->Skin())){
						$skincache = $this->__get($area->Skin()."CacheLifeTime");
						$cachename = $this->Name()."_render".$area->Skin();
						$callmethod = true;
						if ($skincache > 0)
							$callmethod = !$this->startCache($this->currentEngine, $area->Skin(), $skincache);
						if ($callmethod)	
							call_user_func(array($this,"render".$area->Skin()),$area);
						if ($skincache > 0)
							$this->stopCache($this->currentEngine, $cachename, $skincache);	
					}
				$html = $this->Application()->Buffer()->StopRecording();
					
				$skipwrapper = false;
				if ($area instanceof TWidgetArea)
					$skipwrapper = $area->SkipWrapper;
				if (!$skipwrapper){ ?><div id="<?php echo $area->Id(); ?>" class="area-holder<?php echo $accs?(" ".$accs):""; ?>"<?php echo $style?' style="'.$style.'"':''; ?>><?php }
					echo $html;
				if (!$skipwrapper){ ?></div><?php }
				if ($this->currentEngine instanceof TAreasEngine)
					$this->currentEngine->ExitArea($area);				
		}
					
		public function __toString(){
			return $this->Name();
		}	
	}
?>