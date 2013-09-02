<?php
require_once 'common/component/installable.inc';

class TPatcher extends TConfigurable {
	protected $_conf_private_dir_ = null;
	
	protected $_conf_app_dir = null;
	
	public function ApplyPatch(TFile $patch){
		$installed = false;
		$result = array();
		
		$pd = $this->PrivateDir?$this->PrivateDir:$this->Application()->PrivateDir();
		$ad = $this->AppDir?$this->AppDir:$this->Application()->AppPath();
		
		if ($patch->MimeType == 'application/zip' || pathinfo($patch->GetStorePath(),PATHINFO_EXTENSION) == "zip") {
			$d = new TDate();
			$extract_path = $pd.'/install/i_'.$d->TimeStamp();
			$backup_path = $pd.'/install/b_'.$d->TimeStamp();
			TFileSystem::ForceDir($extract_path);
			$zip = new ZipArchive();
			$zip->open($patch->GetStorePath());
			$zip->extractTo($extract_path);
			$zip->close();
			if (file_exists($extract_path.'/manifest.xml')){
				$pXml = new DOMDocument();
				$pXml->preserveWhiteSpace = false;
				$pXml->Load($extract_path.'/manifest.xml');
				$xpath = new DOMXPath($pXml);
				
				$cp = $xpath->query('copy',$pXml->firstChild);
				foreach ($cp as $node){
					$file = $xpath->query('file',$node);
					$dest = $xpath->query('destination',$node);
					$file = $file->item(0)->nodeValue;
					
					$bup = str_replace('{PrivateDir}', 'private', $dest->item(0)->nodeValue);
					$bup = str_replace('{AppDir}', '', $bup);					
					$dest = str_replace('{PrivateDir}', $pd, $dest->item(0)->nodeValue);
					$dest = str_replace('{AppDir}', $ad, $dest);
					TFileSystem::ForceDir($dest);
					if (file_exists($dest.'/'.$file)){
						TFileSystem::ForceDir($backup_path.'/'.$bup);
						copy($dest.'/'.$file,$backup_path.'/'.$bup.'/'.$file);
					}	
					copy($extract_path.'/'.$file, $dest.'/'.$file);
				}
				
				$install = $xpath->query('install',$pXml->firstChild);
				if ($install->length > 0){
					$this->Application()->Reload();
					foreach ($install as $i){
						if ($com = $this->Application()->Instance($i->nodeValue))
							if ($com instanceof IInstallable)
								$com->Install();
					}
				}

				$info = $xpath->query('info',$pXml->firstChild);
				foreach ($info as $i)
					$result[$i->getAttribute('name')] = $i->nodeValue;
			}
			TFileSystem::DeleteDir($extract_path);
			unlink($patch->GetStorePath());
			$installed = true;
		}
		if (!$installed)
			return false;	
		return $result;	
	}
}