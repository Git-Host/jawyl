<?php
	require_once 'widgets/TAreasEditForm.inc';

	class TAreasCreateForm extends TAreasEditForm {		
		public function Create(){
			$request = func_get_arg(func_num_args() - 1);
			if ($request instanceof THttpRequest){
				$cc = $this->Source->DataSource->GetItem($request->contentclass);
				$props = TContentModel::PropertiesByClass($cc->StorageClass);
				$attrvalues = array();
				
				foreach ($props as $p){
					if ($this->HideProperties){
						if (in_array($p->Name(),$this->HideProperties))
						continue;
					}
					$attrvalues[$p->Name()] = $request->__get($p->Name());
				}	
				$item = $this->Source->CreateContentItem($request->contentclass,$attrvalues);
				$this->Application()->Redirect($this->response->ItemEditUrl($item));
			} else throw new TCoreException(TCoreException::ERR_ACCESS_DENIED);
		}
		
		protected function defaultOutput(IThemeEngine $engine){
			if (!($this->response instanceof TContentManagerCreationPage)){
				?>
				<div class="error-msg">Widget cannot be displayed on page of this class!</div>
				<?php
				return;
			}
			
			$cc = $this->response->GetContentClass();
			if ($cc){
				?>
				<form action="<?php echo $this->response->Url($this->Name(),"Create"); ?>" method="post">
				<input type="hidden" name="contentclass" value="<?php echo $cc->ItemId(); ?>" />
				<table>
				<tr><th><?php echo _i("Property"); ?></th><th><?php echo _i("Value"); ?></th></tr>
				<?php
					$props = TContentModel::PropertiesByClass($cc->StorageClass);
					foreach ($props as $p){ $this->outProperty($p); } ?>
				</table>
				<input type="submit" value="<?php __i("Create"); ?>" />
				</form>
				<?php
			}
		}
	}
?>