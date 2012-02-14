<?php
	final class TFilePropertyAspect {
		protected static function outputDBFile(TDBStoredFile $file){
			$dom = new DOMDocument();
			$dom->loadXML($file);
			$xpath = new DOMXPath($dom);
			$fn = $xpath->query("/file/header/name");
			$mime = "";
			if ($fn->length > 0){
				if ($fn->item(0)->nodeValue != "") 
					$mime = mime_content_type($fn->item(0)->nodeValue);
			}
			if ($mime == ""){
				$mt = $xpath->query("/file/header/mime");
				if ($mt->length > 0) 
					$mime = $mt->item(0)->nodeValue;
			}
			header("Content-Type:".$mime);
			$content = $xpath->query("/file/content");
			if ($content->length > 0) {
				echo $content->item(0)->nodeValue;
			}
			die;
		}

		public static function OutputFile(TFile $file){
			if ($file instanceof TDBStoredFile)
				self::outputDBFile($file);
			header("Content-Type:".$file->MimeType);
			echo $file->getContents();
			die;
		}
	}
?>