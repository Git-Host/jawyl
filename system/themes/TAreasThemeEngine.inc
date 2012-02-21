<?php
	TApplication::import("themes::TIntrinsicThemeEngine");
	TApplication::import("ajax::TAreasEngine");
	
	class TAreasThemeEngine extends TIntrinsicThemeEngine {
		private $_area_stack_ = array();
		
		private function _area_compile($matches){
			$context = "null";
			if (count($matches) > 1)
				$nm = str_replace('"','',$matches[1]);
			if (count($matches) > 2){
				$context = str_replace('"','',$matches[2]);
				if ($context == "") $context = "null"; else $context = "$".$context;
			}
			return '<?php $WIDGET->RenderArea($this->EnterArea($WIDGET->GetArea('.$context.',"'.$nm.'",$this->CurrentArea()))); $this->ExitArea(); ?>';
		}
		
		public function EnterArea(IArea $area){
			array_push($this->_area_stack_,$area);
			return $area;
		}
		
		public function ExitArea(){
			array_pop($this->_area_stack_);
		}
		
		public function CurrentArea(){
			$n = count($this->_area_stack_);
			if ($n > 0)
				return $this->_area_stack_[$n - 1];
			return null;
		}
		
		public function SkipWrapper(){
			$ca = $this->CurrentArea();
			if ($ac instanceof TWidgetArea)
				$ca->SkipWrapper = true;
		} 
		
		protected function preCompile($text){
			$text = preg_replace_callback('/<%area\\s+([^\\s]*|"[^"]*")(?:\\s+context\\s*=\\s*([^\\s]*|"[^"]*"))?\\s*%>/',array(&$this,"_area_compile"),$text);
			$text = preg_replace('/<%skipwrapper%>/','<?php $this->SkipWrapper(); ?>',$text);
			return $text;
		}
	}
?>