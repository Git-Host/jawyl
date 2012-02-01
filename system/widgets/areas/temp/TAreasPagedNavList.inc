<?php
	TApplication::import("widgets::TAreasNestings");

	class TAreasPagedNavList extends TAreasWidget {
/**
 * @see TJQueryWidget::jsCode()
 */	
		protected function jsCode(){
			
		}
/**
 * @see TJQueryWidget::linkCustomResources()
 */	
		protected function linkCustomResources(){
			$this->LoadWindow();
		}

		public function DeleteItem($delid){
			$this->Source->DeleteTheItem($delid);
		}
		
		public function BulkDelete(){
			
		}
		
		protected function defaultOutput(IThemeEngine $engine){
?>
<div id="<?php echo $this->Name(); ?>" class="contents-list">
	<table class="content-items">
<?php
	if ($this->isSublevel()){	
?>
	<tr><td><a href="<?php echo $this->response->Url($this->Name(),"LevelUp"); ?>">...</a></td></tr>
<?php		
	}
	$items = $this->Items();
	foreach ($items as $item){
?>
<tr id="<?php echo $item->ItemId(); ?>">
<td><?php echo $item->ClassName(); ?></td>
<td><?php echo $item; ?></td>
<td><a href="<?php echo $item->OpenLink(); ?>">contents</a></td>
<td><a href="<?php echo $this->response->ItemEditUrl($item->Item()); ?>">edit</a></td>
<td><a href="<?php echo $this->response->Url($this->Name(),"DeleteItem",array("delid"=>$item->Item()->ItemId())); ?>">delete</a></td>
<td><a href="javascript:jQuery(&quot;#<?php echo $this->Name();  ?>_move_dlg form input#moveitem&quot;).val(&quot;<?php echo $item->Item()->ItemId(); ?>&quot;);jQuery(&quot;#<?php echo $this->Name();  ?>_move_dlg&quot;).window().open(true);">move</a></td>
</tr>
<?php } ?>
</table>
<div id="<?php echo $this->Name(); ?>_move_dlg" style="display:none;width:200px;">
<div id="mt" style="height:170px;">
<?php
	$w = array_values($this->Widgets());
	if (count($w) > 0)
		$w[0]->Output($engine);
?>
</div>
<form action="<?php echo $this->response->Url($this->Name(),"Move"); ?>" method="post">
	<input type="hidden" id="moveitem" name="moveitem" value="" />
	<input type="submit" value="<?php __i("Move"); ?>" />
</form>
</div>
</div>
<?php	
			
		}

		public function Move(){
			$request = func_get_arg(func_num_args() - 1);
			if ($request instanceof THttpRequest){
				$w = array_values($this->Widgets());
				if (count($w) > 0){
					$target = $w[0]->Source->CurrentItem();
					if ($target){
						$this->Source->MoveItem($request->moveitem,$target->ItemId());
					} else $this->Source->MoveItem($request->moveitem,null);
				}
			}
		}
	} 
?>