<?php
	TApplication::import("widgets::TAreasWidget");

	class TAreasEditForm extends TAreasWidget {
		protected $hideProperties;
		
		public function __set($nm,$value){
			if ($nm == "HideProperties")
				$this->hideProperties  = TConvertions::ConvertToArray($value,";");
			else	
				parent::__set($nm,$value);
		}
		
		public function __get($nm){
			if ($nm == "HideProperties")
				return $this->hideProperties;
			return parent::__get($nm);
		}
		
		protected function outProperty(IPropertyMeta $p){
			if ($this->HideProperties){
				if (in_array($p->Name(),$this->HideProperties))
					return;
			}
				
				$v = "";
				if ($p instanceof IProperty)
					$v = $p->Value();
				?>
				<tr>
				<td valign="top" class="caption">
				<?php echo htmlentities(_i($p->Caption()),ENT_QUOTES,"UTF-8"); ?>
				</td>
				<td valign="top">
				<?php
					$sel = $p->Selection();
					if ((count($sel) > 0) || ($p->Type() == TItemPropertyType::PT_REFERENCE)){
						?><select name="<?php echo $p->Name(); ?>" class="<?php echo ($p->Type() == TItemPropertyType::PT_REFERENCE)?TItemPropertyType::PT_REFERENCE:"set";?>"><?php
						if ($p->Nullable()){
							?><option value="_null_"<?php echo is_null($v)?' selected':''; ?>><?php __i("None"); ?></option><?php
						}
			
						foreach ($sel as $key=>$value){
							?><option value="<?php echo $key; ?>"<?php echo (((string)$v == (string)$key)?' selected':''); ?>><?php __i($value); ?></option><?php
						}
						?></select><?php
					}
					else {
						switch ($p->Type()){
							case TItemPropertyType::PT_BOOL:{
								?><input name="<?php echo $p->Name(); ?>" type="checkbox" class="<?php echo TItemPropertyType::PT_BOOL; ?>" <?php echo ($v?'checked':''); ?>/><?php
							}break;
							case TItemPropertyType::PT_DATE:{
								?><input name="<?php echo $p->Name(); ?>" type="text" class="<?php echo TItemPropertyType::PT_DATE; ?>" value="<?php echo htmlentities($v); ?>" /><?php
							}break;
							case TItemPropertyType::PT_DEC:{
								?><input name="<?php echo $p->Name(); ?>" type="text" class="<?php echo TItemPropertyType::PT_DEC; ?>" value="<?php echo $v; ?>" /><?php
							}break;
							case TItemPropertyType::PT_IMAGE:{
								?><input name="<?php echo $p->Name(); ?>" type="file" class="<?php echo TItemPropertyType::PT_IMAGE; ?>" /><?php
							}break;
							case TItemPropertyType::PT_FILE:{
								?><input name="<?php echo $p->Name(); ?>" type="file" class="<?php echo TItemPropertyType::PT_FILE; ?>" /><?php
							}break;
							case TItemPropertyType::PT_HTML:{
								?><textarea name="<?php echo $p->Name(); ?>" class="<?php echo TItemPropertyType::PT_HTML; ?>"><?php echo htmlentities($v,ENT_QUOTES,"UTF-8"); ?></textarea><?php
							}break;
							case TItemPropertyType::PT_INT:{
								?><input name="<?php echo $p->Name(); ?>" type="text" class="<?php echo TItemPropertyType::PT_STRING; ?>" value="<?php echo $v; ?>" /><?php
							}break;
							case TItemPropertyType::PT_MULTILINE:{
								?><textarea name="<?php echo $p->Name(); ?>" class="<?php echo TItemPropertyType::PT_MULTILINE; ?>"><?php echo htmlentities($v,ENT_QUOTES,"UTF-8"); ?></textarea><?php
							}break;
							case TItemPropertyType::PT_PASSWORD:{
								?><input name="<?php echo $p->Name(); ?>" type="password" class="<?php echo TItemPropertyType::PT_PASSWORD; ?>" value="" /><?php
							}break;
							case TItemPropertyType::PT_STRING:{
								?><input name="<?php echo $p->Name(); ?>" type="text" class="<?php echo TItemPropertyType::PT_STRING; ?>" value="<?php echo htmlentities($v,ENT_QUOTES,"UTF-8"); ?>" /><?php
							}break;
					}
				}
				?>
				</td>
				</tr>
			<?php			
		}
		
		protected function defaultOutput(IThemeEngine $engine){
			$item = $this->Source->CurrentItem();
			if ($item){
				?>
				<form id="<?php echo $this->Name(); ?>" action="<?php echo $this->response->Url($this->Name(),"Edit"); ?>" method="post">
				<table>
				<tr><th><?php echo _i("Property"); ?></th><th><?php echo _i("Value"); ?></th></tr>
				<?php
					$props = $item->Properties();
					foreach ($props as $p) $this->outProperty($p); 
				?>
				</table>
				<input type="submit" value="<?php __i("Save"); ?>" />
				</form>
				<?php
			}
		}
		
		public function Edit(){
			$request = func_get_arg(func_num_args() - 1);
			if ($request instanceof THttpRequest){
				$item = $this->Source->CurrentItem();
				$props = $item->Properties();
				$attrvalues = array();
				foreach ($props as $p){
					if ($this->HideProperties){
						if (in_array($p->Name(),$this->HideProperties))
						continue;
					}
					$attrvalues[$p->Name()] = $request->__get($p->Name());
				}	
				$this->Source->ModifyCurrentItem($attrvalues);	
			}
		}
		
		protected function TAjaxEditFormInlineJavaScript(){
		?>
			jQuery(document).one("ready",function(e){jQuery("form#<?php echo $this->Name(); ?> input.date").datepicker({dateFormat:jQuery.racoon_date_format,constrainInput:true});});
		<?php
		}

		protected function linkCustomResources(){
		}	
	}
?>