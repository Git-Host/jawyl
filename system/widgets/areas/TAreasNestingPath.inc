<?php
	TApplication::import("common::widget::areas::TAreasList");

	class TAreasNestingPath extends TAreasList {
		public $RootText = "..";
		
		protected function getItems(){
			$item = $this->Source->CurrentContainer();
			if ($item)
				return $this->Source->ItemContainers($item->ItemId());
			$result = array();
			return new TArrayIterator($result);	
		}		
		
		protected function renderPath(TWidgetArea $area){
			$this->RenderArea($this->GetArea($this,"Root"));
			$conts = $this->getItems();
			while ($conts->Next())
				$this->RenderArea($this->GetArea($conts->Item,"Item"));	
			$item = $this->Source->CurrentContainer();
			if ($item)
				$this->RenderArea($this->GetArea($item,"Item"));
		}
		
		protected function renderRoot(TWidgetArea $area){
			?><a href="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"SelectItem",array("id"=>null)); ?>&quot;);">[<?php __i($this->RootText); ?>]</a>&nbsp;<?php
		}
		
		protected function renderItem(TWidgetArea $area){
			$item = $area->Context();
			?><a href="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $item->SelectLink(); ?>">[<?php echo $item->title; ?>]</a>&nbsp;<?php
		}
		
		protected function defaultOutput(IThemeEngine $engine){
			$this->RenderArea($this->GetArea($this,"Path"));
		}
		
		protected function checkSource($src){
			return ($src instanceof INestedItems) && parent::checkSource($src);
		}
		
		private function _on_refresh(TControllerEvent $event){
			$this->refreshArea($this->GetArea($this,"Path"));
		}
		
		protected function onEditEvent(TItemEditEvent $event){
			$this->_on_refresh($event);
		}
		
		protected function onOpenEvent(TItemOpenEvent $event){
			$this->_on_refresh($event);
		}
		
		public function SelectedItem(){
			return $this->Source->CurrentContainer();
		}
		public function http_SelectItem($id){
			$this->Source->OpenItem($this->extractWrappedId($id));
		}
		
		protected function linkCustomResources(){}		
	}
?>