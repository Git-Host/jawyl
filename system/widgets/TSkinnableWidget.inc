<?php

TApplication::import("common::widget::TSourcedWidget");

abstract class TSkinnableWidget extends TSourcedWidget {
	public function http_Scripts(){
		$file = $this->getAsset("js","override.js","dir");
		if (!is_null($file))
			include $file;
		else	
		if (method_exists($this,"JavaScript"))
			$this->JavaScript();
		$file = $this->getAsset("js","extend.js","dir");
		if (!is_null($file))
			include $file;
		die;
	}
	
	public function http_Styles(){
		header("Content-Type:text/".TResponse::TYPE_CSS);
		$file = $this->getAsset("css","override.css","dir");
		if (!is_null($file))
			include $file;
		else
		if (method_exists($this,"Css"))
			$this->Css();
		$file = $this->getAsset("css","extend.css","dir");
		if (!is_null($file))
			include $file;
		die;
	}
	
	protected function getAsset($folder,$extension,$format = "url"){
		$file = false;
		$dir = $this->response->ThemeEngine->ThemeDir()."/".$folder."/";
		if ($this->SkinName)
			if (file_exists($dir.$this->SkinName.".".$extension))
				$file = $dir.$this->SkinName.".".$extension;
		if (!$file)		
			if (file_exists($dir.$this->Name().".".$extension))
				$file = $dir.$this->Name().".".$extension;	
		if (!$file)
			if (file_exists($dir.get_class($this).".".$extension))
				$file = $dir.get_class($this).".".$extension;
		if (!$file) return null;
		if ($format == "url")	
			return str_ireplace($this->response->ThemeEngine->ThemeDir(),$this->response->ThemeEngine->ThemeUrl(),$file);
		return 	$file;
	}
	
	protected function externalCss(){
		return $this->getAsset("css","css");
	}
	
	protected function externalJS(){
		return $this->getAsset("js","js");
	}
	
	protected function checkCss(){
		$file = $this->getAsset("css","extend.css","dir");
		if (!is_null($file)) return true;
		$file = $this->getAsset("css","override.css","dir");
		if (!is_null($file)) return true;
		return method_exists($this,"css");
	}
	
	protected function hasCss(){
		return $this->checkCss();
	}
	
	protected function checkJS(){
		$file = $this->getAsset("js","extend.js","dir");
		if (!is_null($file)) return true;
		$file = $this->getAsset("js","override.js","dir");
		if (!is_null($file)) return true;
		return method_exists($this,"javaScript");
	}
	
	protected function hasJS(){
		return $this->checkJS();
	}
	
	protected function checkIJS(){
		$file = $this->getAsset("js","inline.extend.js","dir");
		if (!is_null($file)) return true;
		$file = $this->getAsset("js","inline.override.js","dir");
		if (!is_null($file)) return true;
		return method_exists($this,"inlineJavaScript");
	}
	
	protected function hasIJS(){
		return $this->checkIJS();
	}
	
	protected function iJS(){
		$file = $this->getAsset("js","inline.override.js","dir");
		if (!is_null($file))
			include $file;
		else 
			if (method_exists($this,"inlineJavaScript")) 
				$this->inlineJavaScript();
		$file = $this->getAsset("js","inline.extend.js","dir");
		if (!is_null($file))
			include $file;
	}
	
	public function LoadResources(){
		$cssfile = $this->externalCss();
		if ($cssfile){
?>
<link href="<?php echo $cssfile; ?>" rel="stylesheet" type="text/css" />
<?php				
		}
		if ($this->hasCss()){	
?>
<link href="<?php echo $this->response->Url($this->Name(),"Styles"); ?>" rel="stylesheet" type="text/css" />
<?php				
		}	
		$jsfile = $this->externalJS();
		if ($jsfile){
?>
<script type='text/javascript' src='<?php echo $jsfile; ?>'></script>
<?php			
		}
		if ($this->hasJS()){
?>
<script type='text/javascript' src='<?php echo $this->response->Url($this->Name(),"Scripts"); ?>'></script>
<?php			
		} 
		
	if ($this->hasIJS()){
		?>		
<script type='text/javascript'>
<?php $this->IJS(); ?>
</script>		
		<?php
		}
	}
	
}
?>