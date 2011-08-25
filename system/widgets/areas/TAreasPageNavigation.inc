<?php
TApplicatuion::import("widgets::areas::TAreasWidget");
	
class TAreasPageNavigation extends TAreasWidget {
	protected function checkSource($src){
		return ($src instanceof IPagedItems) && parent::checkSource($src);
	}
		
/**
 * gets page count
 * @return int
 */	
	public function PageCount(){
		$pc = $this->Source->PageCount();
		if ($pc == 0) $pc = 1;
		return $pc;
	}
	
/**
 * gets current page number
 * @return int
 */	
	public function CurrentPage(){
		return $this->Source->CurrentPageNumber() + 1;
	}	
	
	protected function onReloadEvent(TDataReloadEvent $event){
		$this->refreshArea($this->GetArea($this,"Container"));
	}
		
	protected function renderContainer(TWidgetArea $area){
		$context = $area->Context();
		$cp = $this->CurrentPage();
		$pc = $this->PageCount();
		?>
			Page:
		<?php 
		if ($cp > 1){ 
		?>	
			<button id="<?php echo $area->Id(); ?>_btnfirst" class="ui-pv-firstpage" onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"OpenPage",array("pagenum"=>1)); ?>&quot;);"><span>|&lt;&lt;</span></button>
			<button id="<?php echo $area->Id(); ?>_btnprev" class="ui-pv-prevpage" onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"OpenPage",array("pagenum"=>$cp - 1)); ?>&quot;);"><span>&lt;&lt;</span></button>
		<?php 
		} 
		?>
			<input type="text" id="<?php echo $area->Id(); ?>_page_num" value="<?php echo $cp; ?>" class="ui-page-nav-pagenum" onchange="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"OpenPage",array()); ?>&quot;,{pagenum:this.value});" />/<span id="<?php echo $area->Id(); ?>_page_count" class="ui-page-nav-pagecount"><?php echo $pc; ?></span>
		<?php 
		if ($cp < $pc){ 
		?>	
			<button id="<?php echo $area->Id(); ?>_btnnext" class="ui-pv-nextpage" onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"OpenPage",array("pagenum"=>$cp + 1)); ?>&quot;);"><span>&gt;&gt;</span></button>
			<button id="<?php echo $area->Id(); ?>_btnlast" class="ui-pv-lastpage" onclick="javascript:jQuery.areas_AjaxCall(&quot;<?php echo $this->response->Url($this,"OpenPage",array("pagenum"=>$pc)); ?>&quot;);"><span>&gt;&gt;|</span></button>		
		<?php
		}
	}

/**
 * opens a page
 * @param int $pagenum
 */	
	protected function http_OpenPage($pagenum){
		if (!($this->Source instanceof IPagedItems))
			throw new TCoreException(TCoreException::ERR_NO_INTERFACE);
		$this->Source->OpenPage($pagenum - 1);
	}	
	
	protected function defaultOutput(IThemeEngine $engine){
		?>
		<div id="<?php echo $this->Name(); ?>" class="areas-page-navigation">
		<?php
		$this->RenderArea($this->GetArea($this,"Container"));
		?>
		</div>
		<?php
	}
}
?>